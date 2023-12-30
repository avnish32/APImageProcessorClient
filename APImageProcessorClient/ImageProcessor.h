#include<opencv2/opencv.hpp>
#include<map>

#include "MsgLogger.h"

#pragma once

using cv::Mat;
using cv::Size;

using std::map;

class ImageProcessor
{
private:
	Mat _image;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	cv::String _GetAddressToSaveImage();
	void _ConstructOneChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions);
	void _ConstructTwoChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions);
	void _ConstructThreeChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions);
	void _ConstructFourChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions);

public:
	ImageProcessor();
	ImageProcessor(Mat image);
	ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize);
	~ImageProcessor();
	void DisplayImage(cv::String windowName);
	void SaveImage();
	void SaveImage(cv::String saveAddress);
	Mat GetImage();
};

