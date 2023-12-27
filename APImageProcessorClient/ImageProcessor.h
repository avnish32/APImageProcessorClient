#include<opencv2/opencv.hpp>
#include<map>

#pragma once

using cv::Mat;
using cv::Size;

using std::map;

class ImageProcessor
{
private:
	Mat _image;

	cv::String _GetAddressToSaveImage();

public:
	ImageProcessor();
	ImageProcessor(Mat image);
	ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions);
	~ImageProcessor();
	void DisplayImage(cv::String windowName);
	void SaveImage();
	void SaveImage(cv::String saveAddress);
	Mat GetImage();
};

