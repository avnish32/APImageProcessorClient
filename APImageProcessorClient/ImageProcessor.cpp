#include "ImageProcessor.h"

#include<iostream>

using cv::imwrite;
using cv::imshow;
using cv::Vec3b;

using std::cout;

ImageProcessor::ImageProcessor()
{
	cout << "\nImage processor default constructor.";
	_image = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	_image = image;
}

ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	_image = Mat(imageDimensions, CV_8UC3);
	cout << "\nConstructing image. Image data map size: " << imageDataMap.size();
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
