// APImageProcessorClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <thread>

#include "UDPClient.h"
#include "Constants.h"

using cv::Size;
using cv::Mat;
using cv::imread;
using cv::IMREAD_COLOR;
using cv::INTER_LINEAR;
using cv::String;

using std::string;
using std::cout;
using std::endl;
using std::thread;
using std::vector;

void sendImageToServer(cv::String& imageWriteAddress);

int main(int argc, char** argv)
{
	// Read the image file
	Mat image = imread("G:/Wallpapers/wp2003072-firewatch-wallpapers.jpg", IMREAD_COLOR);
	//Mat image(Size(1024, 768), CV_8UC3, Scalar(10, 80, 45));

	if (image.empty()) // Check for failure
	{
		cout << "Could not open or find the image" << endl;
		system("pause"); //wait for any key press
		return -1;
	}

	//String windowName = "My HelloWorld Window"; //Name of the window
	String savedImgWindowName = "Saved Image";

	//namedWindow(windowName, WINDOW_NORMAL); // Create a window
	//imshow(windowName, image); // Show our image inside the created window.

	String imageWriteAddress[] = { "./Resources/1.jpg", "./Resources/2.jpg", "./Resources/3.jpg", "./Resources/4.jpg",
	"./Resources/5.jpg" , "./Resources/6.jpg" , "./Resources/7.jpg" , "./Resources/8.jpg" };
	Mat resizedImage;
	resize(image, resizedImage, Size(1024, 768), INTER_LINEAR);
	bool wasImageWritten = imwrite(imageWriteAddress[0], resizedImage);
	if (!wasImageWritten) {
		cout << "\nError while writing the image.";
	}
	else {
		//namedWindow(savedImgWindowName, WINDOW_NORMAL);
		Mat savedImage = imread(imageWriteAddress[0]);
		//imshow(savedImgWindowName, savedImage);
	}

	//#######Sending to server

	bool retFlag;

	vector<thread> threadVector;
	for (int i = 0; i < 8; i++) {
		//thread t(&sendImageToServer, ref(imageWriteAddress[i]));
		threadVector.push_back(thread(&sendImageToServer, ref(imageWriteAddress[i])));
	}

	for (thread &t : threadVector) {
		t.join();
	}
	//short retVal = sendImageToServer(imageWriteAddress, retFlag);
	
	/*if (retVal == RESPONSE_FAILURE) {
		cout << "\nError while sending image to server.";
	}
	else {
		cout << "\nSuccessfully sent image to server.";
	}*/

	//cout << "\nResized Image data before reshaping: Rows: "<<resizedImage.rows<<" | Cols: "<<resizedImage.cols <<endl<< resizedImage;
	//resizedImage = resizedImage.reshape(0, 1);
	//cout << "\nResized Image data after reshaping: Rows: " << resizedImage.rows << " | Cols: " << resizedImage.cols << endl << resizedImage;
	//cout << "\nAt (0,8): " << resizedImage.at<Vec3b>(0,8);

	//waitKey(0); // Wait for any keystroke in the window

	//destroyWindow(windowName);
	//destroyWindow(savedImgWindowName);//destroy the created window

	return 0;
}

void sendImageToServer(cv::String& imageWriteAddress)
{
	cout << "\nThread initialized to send image to server.";
	UDPClient udpClient;
	if (!udpClient.isValid()) {
		cout << "\nSocket could not be created. Application will now exit.";
		//return RESPONSE_FAILURE;
	}

	int responseCode = udpClient.sendImageSize(imageWriteAddress, SERVER_IP_ADDRESS, SERVER_PORT);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nSending image size to server failed. Application will now exit.";
		//return RESPONSE_FAILURE;
	}

	short serverResponseCode;
	responseCode = udpClient.receiveAndValidateServerResponse(SERVER_IP_ADDRESS, SERVER_PORT, serverResponseCode);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nReceving/validating response from server failed. Application will now exit.";
		return;
		//return RESPONSE_FAILURE;
	}

	if (serverResponseCode == SERVER_NEGATIVE_ACK) {
		cout << "\nServer sent negative acknowldgement. Application will now exit.";
		return;
		//return RESPONSE_FAILURE;
	}

	responseCode = udpClient.sendImage(imageWriteAddress, SERVER_IP_ADDRESS, SERVER_PORT);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nSending image to server failed. Application will now exit.";
		//return RESPONSE_FAILURE;
	}
}
