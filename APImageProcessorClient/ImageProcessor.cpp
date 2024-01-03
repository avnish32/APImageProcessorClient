#include "ImageProcessor.h"
#include "Constants.h"
#include "ImageConstructor.h"
#include "ImageConstructorFactory.h"

#include<string>
#include<thread>
#include<format>
#include<chrono>

using std::stringstream;
using std::this_thread::get_id;
using std::format;
using std::chrono::system_clock;

using cv::imwrite;
using cv::imshow;
using cv::Vec2b;
using cv::Vec3b;
using cv::Vec4b;
using cv::waitKey;
using cv::destroyWindow;
using cv::namedWindow;
using cv::WINDOW_KEEPRATIO;

using std::to_string;

ImageProcessor::ImageProcessor()
{
	_msgLogger->LogDebug("Image processor default constructor.");
	_image = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	_image = image;
}

/*
Parameterized constructor; constructs an image having imageDimensions from the given imageDataMap.
*/
ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize)
{
	short numOfChannels = imageFileSize / (imageDimensions.width * imageDimensions.height);
	_msgLogger->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(numOfChannels));

	ImageConstructor* imageConstructor = ImageConstructorFactory::GetImageConstructor(numOfChannels, imageDataMap, imageDimensions);
	if (imageConstructor == nullptr) {
		_msgLogger->LogError("ERROR: Constructing image with " + to_string(numOfChannels) + "is currently not supported.");
		return;
	}

	_image = imageConstructor->ConstructImage();
}

ImageProcessor::~ImageProcessor()
{
	_msgLogger->LogDebug("Image processor destructor.");
}

void ImageProcessor::DisplayImage(cv::String windowName)
{
	namedWindow(windowName, WINDOW_KEEPRATIO);
	imshow(windowName, _image);

	cv::waitKey(0);
	cv::destroyWindow(windowName);
}

/*
This functions saves the modified image at the location of the original image.
*/
void ImageProcessor::SaveImage(std::string originalImageAddress)
{
	bool wasImageWritten = imwrite(_GetAddressToSaveModifiedImage(originalImageAddress), _image);
	if (!wasImageWritten) {
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	_msgLogger->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return _image;
}

/*
This function constructs the address to save the modified image.
The new address is the same as that of original image, with the suffix '_modified_<timestamp>' added.
*/
std::string ImageProcessor::_GetAddressToSaveModifiedImage(std::string originalImageAddress) 
{
	//Below snippet to convert chrono::time_point to string taken from https://stackoverflow.com/a/46240575
	string currentTimeString = format("{:%H%M%S}", system_clock::now());

	ushort dotIndex = originalImageAddress.find_last_of('.');
	string modifiedImageSaveAddress = string(originalImageAddress, 0, dotIndex)
		.append(MODIFIED_SUFFIX).append(UNDERSCORE).append(currentTimeString)
		.append(originalImageAddress, dotIndex, originalImageAddress.length());

	return modifiedImageSaveAddress;
}

void ImageProcessor::DisplayOriginalAndFilteredImage(const Mat& originalImage, const Mat& filteredImage)
{
	//Below snippet to convert thread id to string taken from https://stackoverflow.com/a/19255203
	stringstream sStream;
	sStream << get_id();

	string originalImageWindowName = ORIGINAL_IMAGE_WINDOW_NAME + sStream.str();
	string filteredImageWindowName = FILTERED_IMAGE_WINDOW_NAME + sStream.str();

	namedWindow(originalImageWindowName, cv::WINDOW_AUTOSIZE);
	imshow(originalImageWindowName, originalImage);

	namedWindow(filteredImageWindowName, cv::WINDOW_AUTOSIZE);
	imshow(filteredImageWindowName, filteredImage);

	waitKey(0);
	destroyWindow(originalImageWindowName);
	destroyWindow(filteredImageWindowName);
}

