// APImageProcessorClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <thread>

#include "UDPClient.h"
#include "Constants.h"

typedef unsigned short u_short;

using cv::Size;
using cv::Mat;
using cv::imread;
using cv::IMREAD_COLOR;
using cv::INTER_LINEAR;
using cv::String;
using cv::Vec3b;

using std::string;
using std::cout;
using std::endl;
using std::thread;
using std::vector;

void sendImageToServer(cv::String& imageWriteAddress);

//TODO remove filter methods after testing
enum RotationMode { CLOCKWISE, ANTI_CLOCKWISE };

Mat ResizeFilter(Mat _sourceImage, int _targetWidth, int _targetHeight)
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	if (sourceWidth == _targetWidth && sourceHeight == _targetHeight) {
		//TODO return the original image
		return _sourceImage;
	}

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), _sourceImage.type());
	for (int i = 0; i < _targetHeight; i++) {
		for (int j = 0; j < _targetWidth; j++) {
			int sourceImageRow = round(((float)i / _targetHeight) * sourceHeight);
			int sourceImageCol = round(((float)j / _targetWidth) * sourceWidth);
			targetImage.at<cv::Vec3b>(i, j) = _sourceImage.at<cv::Vec3b>(sourceImageRow, sourceImageCol);

			//cout << "\n(" << i << ", " << j << ") <- (" << sourceImageRow << ", " << sourceImageCol << ")";
		}
	}

	return targetImage;
}

Mat RotateClockwiseOnce(Mat _sourceImage)
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), _sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = sourceHeight - j - 1;
			int sourceImageCol = i;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat RotateAntiClockwiseOnce(Mat _sourceImage)
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), _sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = j;
			int sourceImageCol = sourceWidth - i - 1;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat RotateTwice(Mat _sourceImage)
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	Mat targetImage = Mat(cv::Size(sourceWidth, sourceHeight), _sourceImage.type());

	for (int i = 0; i < sourceHeight; i++) {
		for (int j = 0; j < sourceWidth; j++) {
			int sourceImageRow = sourceHeight - i - 1;
			int sourceImageCol = sourceWidth - j - 1;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat RotateFilter(Mat sourceImage, u_short _numOfTurns, RotationMode _direction)
{
	_numOfTurns %= 4;

	switch (_numOfTurns) {
	case 1:
		switch (_direction)
		{
		case CLOCKWISE:
			return RotateClockwiseOnce(sourceImage);
		default:
			return RotateAntiClockwiseOnce(sourceImage);
		}
	case 2:
		//For 2 turns, direction doesn't matter.
		return RotateTwice(sourceImage);
	case 3:
		switch (_direction)
		{
		case CLOCKWISE:
			return RotateAntiClockwiseOnce(sourceImage);
		default:
			return RotateClockwiseOnce(sourceImage);
		}
	default:
		return sourceImage;
	}

	return Mat();
}

bool CanCropImage(int _cropTopLeftCornerX, int _cropTopLeftCornerY, Mat _sourceImage)
{
	return _cropTopLeftCornerX <= _sourceImage.cols
		&& _cropTopLeftCornerY <= _sourceImage.rows;
}

Mat Crop(int _cropTopLeftCornerX, int _cropTopLeftCornerY, int _targetWidth, int _targetHeight, Mat _sourceImage)
{
	//Initialize target dimensions
	int sourceImageColLimit = MIN(_cropTopLeftCornerX + _targetWidth, _sourceImage.cols);
	int sourceImageRowLimit = MIN(_cropTopLeftCornerY + _targetHeight, _sourceImage.rows);
	_targetWidth = sourceImageColLimit - _cropTopLeftCornerX;
	_targetHeight = sourceImageRowLimit - _cropTopLeftCornerY;

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), _sourceImage.type());

	int currentTargetX = -1, currentTargetY = -1;

	for (int i = _cropTopLeftCornerY; i < sourceImageRowLimit; i++) {
		currentTargetY++;
		currentTargetX = 0;
		for (int j = _cropTopLeftCornerX; j < sourceImageColLimit; j++) {
			targetImage.at<Vec3b>(currentTargetY, currentTargetX) = _sourceImage.at<Vec3b>(i, j);
			currentTargetX++;
		}
	}

	return targetImage;
}

Mat FlipHorizontally(Mat _sourceImage)
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(i, _sourceImage.cols - j - 1);
		}
	}

	return targetImage;
}

Mat FlipVertically(Mat _sourceImage)
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(_sourceImage.rows - i - 1, j);
		}
	}

	return targetImage;
}

Mat ConvertRGBToGrayscale(Mat _sourceImage)
{
	//Initialize target image. Since this is a grayscale image, target need have only one channel
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), CV_8UC1);

	Mat sourceChannelsSplit[3];
	split(_sourceImage, sourceChannelsSplit);

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			//grayscale = 0.3 * R + 0.59 * G + 0.11 * B}
			targetImage.at<uchar>(i, j) = (0.3f * sourceChannelsSplit[2].at<uchar>(i, j))
				+ (0.59f * sourceChannelsSplit[1].at<uchar>(i, j))
				+ (0.11f * sourceChannelsSplit[0].at<uchar>(i, j));
		}
	}
	return targetImage;
}

float clampPixelValue(float unClampedValue, const float minValue, const float maxValue)
{
	if (unClampedValue < minValue) {
		return minValue;
	}

	if (unClampedValue > maxValue) {
		return  maxValue;
	}

	return unClampedValue;
}

Mat AdjustBrigthness(Mat _sourceImage, float _brightnessAdjFactor)
{
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	Mat sourceBGRChannels[3];

	cv::split(_sourceImage, sourceBGRChannels);

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			uchar red = round(clampPixelValue(sourceBGRChannels[2].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar green = round(clampPixelValue(sourceBGRChannels[1].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar blue = round(clampPixelValue(sourceBGRChannels[0].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));

			targetImage.at<Vec3b>(i, j) = Vec3b(blue, green, red);
		}
	}

	return targetImage;
}

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

	//resize(image, resizedImage, Size(1024, 768), INTER_LINEAR);
	resizedImage = ResizeFilter(image, 800, 1080);

	Mat rotatedImage = RotateFilter(image, 7, ANTI_CLOCKWISE);

	int cropTopLeftX = 1700, cropTopLeftY=10, croppedWidth = 200, croppedHeight = 200;
	Mat croppedImage = image;
	if (CanCropImage(cropTopLeftX, cropTopLeftY, image)) {
		 croppedImage = Crop(cropTopLeftX, cropTopLeftY, croppedWidth, croppedHeight, image);
	}
	else {
		cout << "\nImage cannot be cropped with the given parameteres.";
	}
	
	Mat flippedImage = FlipVertically(image);

	Mat grayscaleImage = ConvertRGBToGrayscale(image);

	Mat brigthnessAdjustedImg = AdjustBrigthness(image, 0.25f);

	bool wasImageWritten = imwrite(imageWriteAddress[0], brigthnessAdjustedImg);
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
	for (int i = 0; i < 0; i++) {
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
