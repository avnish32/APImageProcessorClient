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

//TODO remove after testing
#include "ImageFilterEnums.h"

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

MsgLogger* MsgLogger::_loggerInstance = nullptr;

void SendImageRequestToServer(ImageRequest& imageRequest);

string GetModifiedImageSaveLocation(const string& originalImageLocation);

void CleanUpClient(UDPClient& udpClient, std::thread& serverMsgReceivingThread);

void DisplayOriginalAndFilteredImage(const Mat& imageRequest, const Mat& filteredImage);

void TestImageFunctionalities(cv::String  imageWriteAddress[8], bool& retFlag);

int main(int argc, char** argv)
{
	MsgLogger* msgLogger = MsgLogger::GetInstance();

	std::string argValues = "Command line arguments: ";
	//cout << "\nArg values: ";

	for (int i = 0; i < argc; i++) {
		//cout << *(argv + i) << " | ";
		stringstream sStream;
		sStream << *(argv + i);
		argValues.append(sStream.str()).append(" | ");
	}

	msgLogger->LogError(argValues);

	InputProcessor inputProcessor(argc, argv);
	if (!inputProcessor.ValidateInput()) {
		//cout << "\nInvalid input.";
		msgLogger->LogError("ERROR: Invalid input.");
		return EXIT_FAILURE;
	}

	//cout << "\nValidation successful.";
	msgLogger->LogError("Validation of command line arguments successful.");

	vector<ImageRequest> imageRequests = inputProcessor.InitializeImageRequests();

	vector<thread> threadVector;

	//cout << "\nBefore looping over image requests.";
	//msgLogger->LogDebug("Before looping over image requests.");

	for (ImageRequest& imageRequest : imageRequests) {
		//thread t(&sendImageToServer, ref(imageWriteAddress[i]));

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

	//cout << "\nThread initialized to send image to server. Thread ID: "<<std::this_thread::get_id();
	cout << "\n\n";
	msgLogger->LogError("Thread initialized to communicate with server.");

	//Create a new client object.
	UDPClient udpClient(imageRequest.GetServerIp(), imageRequest.GetServerPort());
	if (!udpClient.IsValid()) {
		//cout << "\nSocket could not be created.";
		msgLogger->LogError("ERROR: Socket could not be created.");
		return;
	}

	//Send image information such as its dimensions, size, filter type and parameters to the server.
	int responseCode = udpClient.SendImageMetadataToServer(imageRequest.GetImageMetadataPayload());
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nSending image size to server failed.";
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
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nReceving/validating response from server failed.";
		msgLogger->LogError("Receving/validating response from server failed.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	if (serverResponseCode == SERVER_NEGATIVE_ACK) {
		//cout << "\nServer sent negative acknowldgement.";
		msgLogger->LogError("Server sent negative acknowldgement.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Send image data to server.
	responseCode = udpClient.SendImage(imageRequest.GetImage());
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nSending image to server failed.";
		msgLogger->LogError("Sending image to server failed.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
		//return RESPONSE_FAILURE;
	}

	//Receive filtered image dimensions
	cv::Size processedImageDimensions;
	uint processedImageFileSize;

	short clientResponseCode = CLIENT_POSITIVE_ACK;
	responseCode = udpClient.ReceiveAndValidateImageMetadata(processedImageDimensions, processedImageFileSize);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nError while receiving/validating dimensions of processed image from server.";
		msgLogger->LogError("Error while receiving/validating dimensions of processed image from server.");
		clientResponseCode = CLIENT_NEGATIVE_ACK;
	}

	//Send acknowledgment based on image dimensions received.
	msgLogger->LogError("Sending response to server. Response code: " + to_string(clientResponseCode));
	responseCode = udpClient.SendClientResponseToServer(clientResponseCode, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nCould not send response to server.";
		msgLogger->LogError("Error while sending response to server.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
		//return RESPONSE_FAILURE;
	}

	if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
	}

	//Receive filtered image data/
	ImageProcessor modifiedImageProcessor;
	responseCode = udpClient.ConsumeImageDataFromQueue(processedImageDimensions, processedImageFileSize, modifiedImageProcessor);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nError while receiving processed image from server.";
		msgLogger->LogError("Error while receiving processed image from server.");
		CleanUpClient(udpClient, serverMsgReceivingThread);
		return;
		//return RESPONSE_FAILURE;
	}

	//Send positive acknowledgment if all payloads received successfully.
	msgLogger->LogError("All image data received. Sending positive ACK to server.");

	responseCode = udpClient.SendClientResponseToServer(CLIENT_POSITIVE_ACK, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nCould not send ACK to server.";
		msgLogger->LogError("ERROR: Could not send ACK to server.");
		// Not returning from here as no further communication is required with server,
		// and application can proceed with saving the received image.
		//return RESPONSE_FAILURE;
	}

	//Stop communicating with the server now.
	CleanUpClient(udpClient, serverMsgReceivingThread);

	//Save modified image in the location of the original image
	string modifiedImageSaveLocation = GetModifiedImageSaveLocation(imageRequest.GetImagePath());
	msgLogger->LogDebug("Modified image location: " + modifiedImageSaveLocation);
	modifiedImageProcessor.SaveImage(modifiedImageSaveLocation);

	//Display both images
	DisplayOriginalAndFilteredImage(imageRequest.GetImage(), modifiedImageProcessor.GetImage());
}

/*
This function returns the location of the original image where the modified image should be saved
*/

//TODO move this to image processor
string GetModifiedImageSaveLocation(const string& originalImageAddress)
{
	ushort dotIndex = originalImageAddress.find_last_of('.');
	string modifiedImageSaveAddress = string(originalImageAddress, 0, dotIndex)
		.append(MODIFIED_SUFFIX).append(originalImageAddress, dotIndex, originalImageAddress.length());
	
	return modifiedImageSaveAddress;
}

/*
This function stops any communication with the server.
*/
void CleanUpClient(UDPClient& udpClient, std::thread& serverMsgReceivingThread)
{
	udpClient.StopListeningForServerMsgs();
	serverMsgReceivingThread.join();
}

void DisplayOriginalAndFilteredImage(const Mat& originalImage, const Mat& filteredImage)
{
	namedWindow(ORIGINAL_IMAGE_WINDOW_NAME, cv::WINDOW_AUTOSIZE);
	imshow(ORIGINAL_IMAGE_WINDOW_NAME, originalImage);

	namedWindow(FILTERED_IMAGE_WINDOW_NAME, cv::WINDOW_AUTOSIZE);
	imshow(FILTERED_IMAGE_WINDOW_NAME, filteredImage);

	waitKey(0);
	destroyAllWindows();
}

//TODO remove filter methods after testing
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

Mat RotateFilter(Mat sourceImage, u_short _numOfTurns, RotationDirection _direction)
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

void TestImageFunctionalities(cv::String  imageWriteAddress[8], bool& retFlag)
{
	retFlag = true;
	// Read the image file
	Mat image = imread("G:/Wallpapers/wp2003072-firewatch-wallpapers.jpg", IMREAD_COLOR);
	//Mat image(Size(1024, 768), CV_8UC3, Scalar(10, 80, 45));

	if (image.empty()) // Check for failure
	{
		cout << "Could not open or find the image" << endl;
		system("pause"); //wait for any key press
		return;
	}

	//String windowName = "My HelloWorld Window"; //Name of the window
	String savedImgWindowName = "Saved Image";

	//namedWindow(windowName, WINDOW_NORMAL); // Create a window
	//imshow(windowName, image); // Show our image inside the created window.


	Mat resizedImage;

	//resize(image, resizedImage, Size(1024, 768), INTER_LINEAR);
	resizedImage = ResizeFilter(image, 800, 1080);

	Mat rotatedImage = RotateFilter(image, 7, RotationDirection::ANTI_CLOCKWISE);

	int cropTopLeftX = 1700, cropTopLeftY = 10, croppedWidth = 200, croppedHeight = 200;
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
}