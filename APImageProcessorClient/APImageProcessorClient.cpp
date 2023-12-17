// APImageProcessorClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "stdafx.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

#include "UDPClient.h"
#include "Constants.h"

using namespace cv;
using namespace std;

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

	String windowName = "My HelloWorld Window"; //Name of the window
	String savedImgWindowName = "Saved Image";

	namedWindow(windowName, WINDOW_NORMAL); // Create a window
	imshow(windowName, image); // Show our image inside the created window.

	String imageWriteAddress = "./Resources/savedImage.jpg";
	Mat resizedImage;
	resize(image, resizedImage, Size(1024, 768), INTER_LINEAR);
	bool wasImageWritten = imwrite(imageWriteAddress, image);
	if (!wasImageWritten) {
		cout << "\nError while writing the image.";
	}
	else {
		namedWindow(savedImgWindowName, WINDOW_NORMAL);
		Mat savedImage = imread(imageWriteAddress);
		imshow(savedImgWindowName, savedImage);
	}

	//#######Sending to server

	UDPClient udpClient;
	if (!udpClient.isValid()) {
		cout << "\nSocket could not be created. Application will now exit.";
		return RESPONSE_FAILURE;
	}
	
	int responseCode = udpClient.sendImageSize(imageWriteAddress, SERVER_IP_ADDRESS, SERVER_PORT);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nSending image size to server failed. Application will now exit.";
		return RESPONSE_FAILURE;
	}
	
	responseCode = udpClient.receiveMsgFromServer(SERVER_IP_ADDRESS, SERVER_PORT);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nReceving acknowldegement from server failed. Application will now exit.";
		return RESPONSE_FAILURE;
	}
	else if (responseCode == SERVER_NEGATIVE_ACK) {
		cout << "\nServer sent negative acknowldgement. Application will now exit.";
		return RESPONSE_FAILURE;
	}
	
	responseCode = udpClient.sendImage(imageWriteAddress, SERVER_IP_ADDRESS, SERVER_PORT);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nSending image to server failed. Application will now exit.";
		return RESPONSE_FAILURE;
	}

	//cout << "\nResized Image data before reshaping: Rows: "<<resizedImage.rows<<" | Cols: "<<resizedImage.cols <<endl<< resizedImage;
	resizedImage = resizedImage.reshape(0, 1);
	//cout << "\nResized Image data after reshaping: Rows: " << resizedImage.rows << " | Cols: " << resizedImage.cols << endl << resizedImage;
	cout << "\nAt (0,8): " << resizedImage.at<Vec3b>(0,8);

	waitKey(0); // Wait for any keystroke in the window

	destroyWindow(windowName);
	destroyWindow(savedImgWindowName);//destroy the created window

	return 0;
}
