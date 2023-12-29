#include "ImageProcessor.h"

#include<iostream>

using cv::imwrite;
using cv::imshow;
using cv::Vec2b;
using cv::Vec3b;
using cv::Vec4b;

using std::cout;


void ImageProcessor::_ConstructOneChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	cout << "\nConstructing one channel image. Image data map size: " << imageDataMap.size();

	_image = Mat(imageDimensions, CV_8UC1);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			//cout << "\nInside reconstruction loop. i= " << i << " | j= " << j;
			if (currentImageFragmentByte >= 60000) {

				//cout << "\nCurrent fragment " << currentImageFragment << " completed. ";

				currentImageFragment++;
				currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			_image.at<uchar>(i, j) = *(currentImageFragmentData + currentImageFragmentByte);
			currentImageFragmentByte += 1;
		}
	}

	cout << "\nImage re-shaped.";
}

void ImageProcessor::_ConstructTwoChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	cout << "\nConstructing two channel image. Image data map size: " << imageDataMap.size();

	_image = Mat(imageDimensions, CV_8UC2);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			//cout << "\nInside reconstruction loop. i= " << i << " | j= " << j;
			if (currentImageFragmentByte >= 60000) {

				//cout << "\nCurrent fragment " << currentImageFragment << " completed. ";

				currentImageFragment++;
				currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			_image.at<Vec2b>(i, j) = Vec2b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1));
			currentImageFragmentByte += 2;
		}
	}

	cout << "\nImage re-shaped.";
}

void ImageProcessor::_ConstructThreeChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	cout << "\nConstructing three channel image. Image data map size: " << imageDataMap.size();

	_image = Mat(imageDimensions, CV_8UC3);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			//cout << "\nInside reconstruction loop. i= " << i << " | j= " << j;
			if (currentImageFragmentByte >= 60000) {

				//cout << "\nCurrent fragment " << currentImageFragment << " completed. ";

				currentImageFragment++;
				currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			_image.at<Vec3b>(i, j) = Vec3b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2));
			currentImageFragmentByte += 3;
		}
	}

	cout << "\nImage re-shaped.";
}

void ImageProcessor::_ConstructFourChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	cout << "\nConstructing four channel image. Image data map size: " << imageDataMap.size();

	_image = Mat(imageDimensions, CV_8UC4);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			//cout << "\nInside reconstruction loop. i= " << i << " | j= " << j;
			if (currentImageFragmentByte >= 60000) {

				//cout << "\nCurrent fragment " << currentImageFragment << " completed. ";

				currentImageFragment++;
				currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			_image.at<Vec4b>(i, j) = Vec4b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2),
				*(currentImageFragmentData + currentImageFragmentByte + 3));
			currentImageFragmentByte += 4;
		}
	}

	cout << "\nImage re-shaped.";
}

ImageProcessor::ImageProcessor()
{
	cout << "\nImage processor default constructor.";
	_image = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	_image = image;
}

ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize)
{
	short numOfChannels = imageFileSize / (imageDimensions.width * imageDimensions.height);
	cout << "\nInside ImageProcessor. Number of channels: " << numOfChannels;

	switch (numOfChannels) {
	case 1:
		_ConstructOneChannelImage(imageDataMap, imageDimensions);
		break;
	case 2:
		_ConstructTwoChannelImage(imageDataMap, imageDimensions);
		break;
	case 3:
		_ConstructThreeChannelImage(imageDataMap, imageDimensions);
		break;
	case 4:
		_ConstructFourChannelImage(imageDataMap, imageDimensions);
		break;
	default:
		_ConstructThreeChannelImage(imageDataMap, imageDimensions);
		break;
	}
}

ImageProcessor::~ImageProcessor()
{
	cout << "\nImage processor destructor.";
}

void ImageProcessor::DisplayImage(cv::String windowName)
{
	//namedWindow(windowName, WINDOW_KEEPRATIO);
	imshow(windowName, _image);

	cv::waitKey(0);
	cv::destroyWindow(windowName);
}

void ImageProcessor::SaveImage(cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, _image);
	if (!wasImageWritten) {
		cout << "\nImage could not be written to file.";
		return;
	}
	cout << "\nImage written to file successfully.";
}

void ImageProcessor::SaveImage()
{
	bool wasImageWritten = imwrite(_GetAddressToSaveImage(), _image);
	if (!wasImageWritten) {
		cout << "\nImage could not be written to file.";
		return;
	}
	cout << "\nImage written to file successfully.";
}

Mat ImageProcessor::GetImage()
{
	return _image;
}

cv::String ImageProcessor::_GetAddressToSaveImage() {

	//Below snippet to convert thread id to string taken from https://stackoverflow.com/a/19255203
	auto threadId = std::this_thread::get_id();
	std::stringstream sStream;
	sStream << threadId;

	//Below snippet to convert chrono::time_point to string taken from https://stackoverflow.com/a/46240575
	//using namespace std::chrono_literals;
	std::chrono::time_point tp = std::chrono::system_clock::now();
	std::string timestamp = std::format("{:%H%M%S}", tp);

	//string timestamp = std::format("{:%H%M%s}", nowTime);
	cv::String imageSaveAddress = "./Resources/savedImage_" + sStream.str() + "_" + timestamp + ".jpg";
	return imageSaveAddress;
}

