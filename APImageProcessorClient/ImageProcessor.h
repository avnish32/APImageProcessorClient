#include<opencv2/opencv.hpp>
#include<map>

#include "MsgLogger.h"

#pragma once

using cv::Mat;
using cv::Size;

using std::map;

/*
This class handles various tasks related to the processing of image.
*/

class ImageProcessor
{
private:
	Mat _image;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	std::string _GetAddressToSaveModifiedImage(std::string originalImageAddress);

public:
	ImageProcessor();
	ImageProcessor(Mat image);
	ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize);
	~ImageProcessor();
	void DisplayImage(cv::String windowName);
	void DisplayOriginalAndFilteredImage(const Mat& originalImage, const Mat& filteredImage);
	void SaveImage(std::string originalImageAddress);
	Mat GetImage();
};

