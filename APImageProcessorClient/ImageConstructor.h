#include<map>
#include<string>
#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

using std::map;
using std::string;

using cv::Size;
using cv::Mat;

#pragma once

/*
This is an abstract class to construct an image of _imageDimensions from the given map.
Its implementation depends on the number of channel the image is supposed to have.
*/
class ImageConstructor
{
protected:
	map<unsigned short, string> image_data_map_;
	Size image_dimensions_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();
public:
	ImageConstructor(const map<unsigned short, string>&, const Size&);
	~ImageConstructor();
	virtual Mat ConstructImage() = 0;
};