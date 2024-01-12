#include<opencv2/opencv.hpp>
#include<string>

#include "ImageFilterEnums.h"
#include "MsgLogger.h"

using std::vector;
using cv::Mat;

#pragma once

/*
This class represents a single request that the client makes to the server for image processing.
*/

class ImageRequest
{
private:
	std::string server_ip_;
	ushort server_port_;
	Mat image_;
	cv::String image_absolute_path_;
	ImageFilterTypesEnum filter_type_enum_;
	vector<float> filter_params_;
	
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	std::string ConvertFilterParamsToString();
	
public:
	ImageRequest(std::string, ushort, cv::String, 
		ImageFilterTypesEnum, vector<float>);
	~ImageRequest();
	std::string GetImageMetadataPayload();
	Mat GetImage();
	const std::string& GetServerIp();
	const ushort& GetServerPort();
	const cv::String GetImagePath();
};

