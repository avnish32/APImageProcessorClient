// APImageProcessorClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>

#include <iostream>
#include<fstream>
#include <string>
#include <thread>

#include "UDPClient.h"
#include "Constants.h"
#include "InputProcessor.h"
#include "ImageRequest.h"
#include "MsgLogger.h"

using cv::Size;
using cv::Mat;
using cv::imread;
using cv::imshow;
using cv::waitKey;
using cv::namedWindow;
using cv::destroyAllWindows;
using cv::IMREAD_COLOR;
using cv::INTER_LINEAR;
using cv::String;
using cv::Vec3b;

using std::string;
using std::cout;
using std::endl;
using std::thread;
using std::vector;
using std::bind;
using std::stringstream;
using std::to_string;

typedef unsigned short u_short;

MsgLogger* MsgLogger::logger_instance_ = nullptr;

void SendImageRequestToServer(ImageRequest&);

void CleanUpClient(UDPClient&, std::thread&);

int main(int argc, char** argv)
{
	MsgLogger* msg_logger = MsgLogger::GetInstance();

	std::string arg_values = "Command line arguments: ";

	for (int i = 0; i < argc; i++) {
		stringstream s_stream;
		s_stream << *(argv + i);
		arg_values.append(s_stream.str()).append(" | ");
	}

	msg_logger->LogError(arg_values);

	InputProcessor input_processor(argc, argv);
	if (!input_processor.ValidateInput()) {
		msg_logger->LogError("ERROR: Invalid input.");
		return EXIT_FAILURE;
	}

	msg_logger->LogError("Validation of command line arguments successful.");

	vector<ImageRequest> image_requests = input_processor.InitializeImageRequests();
	vector<thread> thread_vector;

	for (ImageRequest& image_request : image_requests) {
		//Spawning a new client thread for each request to be sent to the server.
		thread_vector.push_back(thread(&SendImageRequestToServer, std::ref(image_request)));
	}

	for (thread &t : thread_vector) {
		t.join();
	}
	return 0;
}

/*
This is the main orchestrator function that calls all the other function in order of the steps to perform
for processing the image. For each image processing request, this is the function that is called in a separate thred.
*/
void SendImageRequestToServer(ImageRequest& image_request)
{
	MsgLogger* msg_logger = MsgLogger::GetInstance();

	cout << "\n\n";
	msg_logger->LogError("Thread initialized to communicate with server.");

	//Create a new client object.
	UDPClient udp_client(image_request.GetServerIp(), image_request.GetServerPort());
	if (!udp_client.IsValid()) {
		msg_logger->LogError("ERROR: Socket could not be created.");
		return;
	}

	//Send image information such as its dimensions, size, filter type and parameters to the server.
	int response_code = udp_client.SendImageMetadataToServer(image_request.GetImageMetadataPayload());
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Sending image size to server failed.");
		return;
	}

	//Below call spawns a new thread here to push messages received from server in queue.
	//This current thread will consume these messages from the queue.
	//This was done to minimise packet loss while client processes the received payload.
	thread server_msg_receiving_thread(bind(&(UDPClient::StartListeningForServerMsgs), &udp_client));

	
	//Listen for server acknowledgment after sending image metadata.
	short server_response_code;
	response_code = udp_client.ReceiveAndValidateServerResponse(server_response_code);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Receving/validating response from server failed.");
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	if (server_response_code == SERVER_NEGATIVE_ACK) {
		msg_logger->LogError("Server sent negative acknowldgement.");
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	//Send image data to server.
	response_code = udp_client.SendImage(image_request.GetImage());
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Sending image to server failed.");
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	//Receive filtered image dimensions
	cv::Size processed_image_dimensions;
	uint processed_image_file_size;

	short client_response_code = CLIENT_POSITIVE_ACK;
	response_code = udp_client.ReceiveAndValidateImageMetadata(processed_image_dimensions, processed_image_file_size);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Error while receiving/validating dimensions of processed image from server.");
		client_response_code = CLIENT_NEGATIVE_ACK;
	}

	//Send acknowledgment based on image dimensions received.
	msg_logger->LogError("Sending response to server. Response code: " + to_string(client_response_code));
	response_code = udp_client.SendClientResponseToServer(client_response_code, nullptr);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Error while sending response to server.");
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	if (client_response_code == CLIENT_NEGATIVE_ACK) {
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	//Receive filtered image data
	ImageProcessor modified_image_processor;
	response_code = udp_client.ConsumeImageDataFromQueue(processed_image_dimensions, processed_image_file_size, modified_image_processor);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("Error while receiving processed image from server.");
		CleanUpClient(udp_client, server_msg_receiving_thread);
		return;
	}

	//Send positive acknowledgment if all payloads received successfully.
	msg_logger->LogError("All image data received. Sending positive ACK to server.");

	response_code = udp_client.SendClientResponseToServer(CLIENT_POSITIVE_ACK, nullptr);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger->LogError("ERROR: Could not send ACK to server.");
		// Not returning from here as no further communication is required with server,
		// and application can proceed with saving the received image.
	}

	//Stop communicating with the server now.
	CleanUpClient(udp_client, server_msg_receiving_thread);

	//Save modified image in the location of the original image
	modified_image_processor.SaveImage(image_request.GetImagePath());

	//Display both images
	modified_image_processor.DisplayOriginalAndFilteredImage(image_request.GetImage(), modified_image_processor.GetImage());
}

/*
This function stops any communication with the server.
*/
void CleanUpClient(UDPClient& udp_client, std::thread& server_msg_receiving_thread)
{
	udp_client.StopListeningForServerMsgs();
	server_msg_receiving_thread.join();
}