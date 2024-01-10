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

void SendImageRequestToServer(ImageRequest& imageRequest);

void CleanUpClient(UDPClient& udpClient, std::thread& serverMsgReceivingThread);

int main(int argc, char** argv)
{
	MsgLogger* msgLogger = MsgLogger::GetInstance();

	std::string argValues = "Command line arguments: ";

	for (int i = 0; i < argc; i++) {
		stringstream sStream;
		sStream << *(argv + i);
		argValues.append(sStream.str()).append(" | ");
	}

	msgLogger->LogError(argValues);

	InputProcessor inputProcessor(argc, argv);
	if (!inputProcessor.ValidateInput()) {
		msgLogger->LogError("ERROR: Invalid input.");
		return EXIT_FAILURE;
	}

	msgLogger->LogError("Validation of command line arguments successful.");

	vector<ImageRequest> imageRequests = inputProcessor.InitializeImageRequests();
	vector<thread> threadVector;

	for (ImageRequest& imageRequest : imageRequests) {
		//Spawning a new client thread for each request to be sent to the server.
		threadVector.push_back(thread(&SendImageRequestToServer, std::ref(imageRequest)));
	}

	for (thread &t : threadVector) {
		t.join();
	}
	return 0;
}

/*
This is the main orchestrator function that calls all the other function in order of the steps to perform
for processing the image. For each image processing request, this is the function that is called in a separate thred.
*/
void SendImageRequestToServer(ImageRequest& imageRequest)
{
	MsgLogger* msgLogger = MsgLogger::GetInstance();

	cout << "\n\n";
	msgLogger->LogError("Thread initialized to communicate with server.");

	//Create a new client object.
	UDPClient udpClient(imageRequest.GetServerIp(), imageRequest.GetServerPort());
	if (!udpClient.IsValid()) {
		msgLogger->LogError("ERROR: Socket could not be created.");
		return;
	}

	//Send image information such as its dimensions, size, filter type and parameters to the server.
	int responseCode = udpClient.SendImageMetadataToServer(imageRequest.GetImageMetadataPayload());
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Sending image size to server failed.");
		return;
	}

	//Below call spawns a new thread here to push messages received from server in queue.
	//This current thread will consume these messages from the queue.
	//This was done to minimise packet loss while client processes the received payload.
	thread serverMsgReceivingThread(bind(&(UDPClient::StartListeningForServerMsgs), &udpClient));

	
	//Listen for server acknowledgment after sending image metadata.
	short serverResponseCode;
	responseCode = udpClient.ReceiveAndValidateServerResponse(serverResponseCode);
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Receving/validating response from server failed.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	if (serverResponseCode == SERVER_NEGATIVE_ACK) {
		msgLogger->LogError("Server sent negative acknowldgement.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Send image data to server.
	responseCode = udpClient.SendImage(imageRequest.GetImage());
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Sending image to server failed.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Receive filtered image dimensions
	cv::Size processedImageDimensions;
	uint processedImageFileSize;

	short clientResponseCode = CLIENT_POSITIVE_ACK;
	responseCode = udpClient.ReceiveAndValidateImageMetadata(processedImageDimensions, processedImageFileSize);
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Error while receiving/validating dimensions of processed image from server.");
		clientResponseCode = CLIENT_NEGATIVE_ACK;
	}

	//Send acknowledgment based on image dimensions received.
	msgLogger->LogError("Sending response to server. Response code: " + to_string(clientResponseCode));
	responseCode = udpClient.SendClientResponseToServer(clientResponseCode, nullptr);
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Error while sending response to server.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Receive filtered image data/
	ImageProcessor modifiedImageProcessor;
	responseCode = udpClient.ConsumeImageDataFromQueue(processedImageDimensions, processedImageFileSize, modifiedImageProcessor);
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("Error while receiving processed image from server.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Send positive acknowledgment if all payloads received successfully.
	msgLogger->LogError("All image data received. Sending positive ACK to server.");

	responseCode = udpClient.SendClientResponseToServer(CLIENT_POSITIVE_ACK, nullptr);
	if (responseCode == FAILURE_RESPONSE) {
		msgLogger->LogError("ERROR: Could not send ACK to server.");
		// Not returning from here as no further communication is required with server,
		// and application can proceed with saving the received image.
	}

	//Stop communicating with the server now.
	CleanUpClient(udpClient, serverMsgReceivingThread);

	//Save modified image in the location of the original image
	modifiedImageProcessor.SaveImage(imageRequest.GetImagePath());

	//Display both images
	modifiedImageProcessor.DisplayOriginalAndFilteredImage(imageRequest.GetImage(), modifiedImageProcessor.GetImage());
}

/*
This function stops any communication with the server.
*/
void CleanUpClient(UDPClient& udpClient, std::thread& serverMsgReceivingThread)
{
	udpClient.StopListeningForServerMsgs();
	serverMsgReceivingThread.join();
}