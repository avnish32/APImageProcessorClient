#include<opencv2/opencv.hpp>
#include<string>

#include "ImageFilterTypes.h"
#include "MsgLogger.h"

using std::vector;
using cv::Mat;

#pragma once
class ImageRequest
{
private:
	std::string _serverIp;
	ushort _serverPort;
	cv::String _imageAbsolutePath;
	ImageFilterTypesEnum _filterTypeEnum;
	vector<float> _filterParams;
	Mat _image;
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
};

