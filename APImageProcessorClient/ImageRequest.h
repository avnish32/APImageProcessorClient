#include<opencv2/opencv.hpp>
#include<string>

#include "ImageFilterTypes.h"
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
	std::string _serverIp;
	ushort _serverPort;
	Mat _image;
	cv::String _imageAbsolutePath;
	ImageFilterTypesEnum _filterTypeEnum;
	vector<float> _filterParams;
	
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	std::string _ConvertFilterParamsToString();
	
public:
	ImageRequest(std::string serverIp, ushort serverPort, cv::String imageAbsolutePath, 
		ImageFilterTypesEnum filterTypeEnum, vector<float> filterParams);
	~ImageRequest();
	std::string GetImageMetadataPayload();
	Mat GetImage();
	const std::string& GetServerIp();
	const ushort& GetServerPort();
	const cv::String GetImagePath();
};

