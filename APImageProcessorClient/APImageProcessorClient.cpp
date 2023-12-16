// APImageProcessorClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "stdafx.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

#include "UDPClient.h"

using namespace cv;
using namespace std;

void displayImageValues(Mat image) {
	cout << endl;
	int rows = image.rows;
	int cols = image.cols;
	cout << "\nRows: " << rows << " | cols: " << cols;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			cout << image.at<ushort>(i,j) << " ";
		}
	}
	cout << endl;
}

int main(int argc, char** argv)
{
	// Read the image file
	Mat image = imread("F:/Amy Santiago/wp4758617.jpg", IMREAD_COLOR);
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
	bool wasImageWritten = imwrite(imageWriteAddress, resizedImage);
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
	udpClient.sendImage(imageWriteAddress, "127.0.0.1", 8080);

	//cout << "\nResized Image data before reshaping: Rows: "<<resizedImage.rows<<" | Cols: "<<resizedImage.cols <<endl<< resizedImage;
	resizedImage = resizedImage.reshape(0, 1);
	//cout << "\nResized Image data after reshaping: Rows: " << resizedImage.rows << " | Cols: " << resizedImage.cols << endl << resizedImage;
	cout << "\nAt (0,8): " << resizedImage.at<Vec3b>(0,8);

	waitKey(0); // Wait for any keystroke in the window

	destroyWindow(windowName);
	destroyWindow(savedImgWindowName);//destroy the created window

	return 0;
}
