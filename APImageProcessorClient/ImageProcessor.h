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
	Mat image_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	std::string GetAddressToSaveModifiedImage(std::string);

public:
	ImageProcessor();
	ImageProcessor(Mat);
	ImageProcessor(map<unsigned short, std::string>, const Size&, const uint&);
	~ImageProcessor();
	void DisplayImage(cv::String);
	void DisplayOriginalAndFilteredImage(const Mat&, const Mat&);
	void SaveImage(std::string);
	Mat GetImage();
};

