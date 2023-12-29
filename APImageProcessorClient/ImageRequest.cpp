#include "ImageRequest.h"
#include "Constants.h"

using std::to_string;
using std::cout;

std::string ImageRequest::_ConvertFilterParamsToString()
{
	if (_filterParams.size() == 0) {
		cout << "\nFilter params is empty.";
		return "";
	}

	std::string filterParamsString = CLIENT_MSG_DELIMITER;
	auto iter = _filterParams.begin();
	
	while (iter != _filterParams.end()) {
		//TODO truncate float to 2 decimal places
		filterParamsString.append(to_string(*iter)).append(CLIENT_MSG_DELIMITER);
		iter++;
	}
	return filterParamsString;
}

ImageRequest::ImageRequest(std::string serverIp, ushort serverPort, cv::String imageAbsolutePath,
	ImageFilterTypesEnum filterTypeEnum, vector<float> filterParams)
{
	cout << "\nInside ImageRequest constructor. Server IP: " << serverIp
		<< " | server port: " << serverPort
		<< " | Image path: " << imageAbsolutePath
		<< " | Filter type: " << filterTypeEnum
		<< " | Filter params: ";

	auto iter = filterParams.begin();
	while (iter != filterParams.end()) {
		cout << *iter << " ";
		iter++;
	}

	_serverIp = serverIp;
	_serverPort = serverPort;
	_imageAbsolutePath = imageAbsolutePath;
	_filterTypeEnum = filterTypeEnum;
	_filterParams = filterParams;
	_image = cv::imread(imageAbsolutePath, cv::IMREAD_COLOR);

	cout << "\nExiting constructor. Image empty: " << _image.empty() << " | Image size: " << _image.total() * _image.elemSize()
		<< " | filter params: ";
	iter = _filterParams.begin();
	while (iter != _filterParams.end()) {
		cout << *iter << " ";
		iter++;
	}
}

ImageRequest::~ImageRequest()
{
	cout << "\nImageRequest destroyed.";
}

std::string ImageRequest::GetImageMetadataPayload()
{
	std::string payload = SIZE_PAYLOAD_KEY;
	payload.append(CLIENT_MSG_DELIMITER)
		.append(to_string(_image.cols)).append(CLIENT_MSG_DELIMITER)
		.append(to_string(_image.rows)).append(CLIENT_MSG_DELIMITER)
		.append(to_string(_image.total() * _image.elemSize())).append(CLIENT_MSG_DELIMITER)
		.append(to_string(_filterTypeEnum)).append(_ConvertFilterParamsToString()).append("\0");

	return payload;
}

Mat ImageRequest::GetImage()
{
	return _image;
}

const std::string& ImageRequest::GetServerIp()
{
	return _serverIp;
}

const ushort& ImageRequest::GetServerPort()
{
	return _serverPort;
}
