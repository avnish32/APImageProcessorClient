#include "ImageRequest.h"
#include "Constants.h"

using std::to_string;

/*
This function converts _filterParams from a float vector to string to be used suitably
as a payload before sending to the server.
*/
std::string ImageRequest::ConvertFilterParamsToString()
{
	if (filter_params_.size() == 0) {
		msg_logger_->LogDebug("Filter params is empty.");
		return EMPTY_STRING;
	}

	std::string filterParamsString = CLIENT_MSG_DELIMITER;
	auto iter = filter_params_.begin();
	
	while (iter != filter_params_.end()) {
		float roundedToTwoDecimals = std::roundf((*iter) * 100) / 100;
		filterParamsString.append(std::format("{:.2f}", roundedToTwoDecimals)).append(CLIENT_MSG_DELIMITER);
		iter++;
	}
	return filterParamsString;
}

/*
Parameterized constructor; it initializes all the members which are needed to make a request to the server.
*/
ImageRequest::ImageRequest(std::string serverIp, ushort serverPort, cv::String imageAbsolutePath,
	ImageFilterTypesEnum filterTypeEnum, vector<float> filterParams)
{
	string imageRequestParamsLogMsg = string("Inside ImageRequest constructor. Server IP: ").append(serverIp)
		.append(" | server port: ").append(to_string(serverPort))
		.append(" | Image path: ").append(imageAbsolutePath)
		.append(" | Filter type: ").append(to_string(filterTypeEnum))
		.append(" | Filter params: ");

	auto iter = filterParams.begin();
	while (iter != filterParams.end()) {
		imageRequestParamsLogMsg.append(to_string(*iter)).append(" ");
		iter++;
	}

	msg_logger_->LogDebug(imageRequestParamsLogMsg);

	server_ip_ = serverIp;
	server_port_ = serverPort;
	image_absolute_path_ = imageAbsolutePath;
	filter_type_enum_ = filterTypeEnum;
	filter_params_ = filterParams;
	image_ = cv::imread(imageAbsolutePath, cv::IMREAD_COLOR);
}

ImageRequest::~ImageRequest()
{
	msg_logger_->LogDebug("ImageRequest destroyed.");
}

/*
This function constructs a string payload from its member variables to be sent to the server.
*/
std::string ImageRequest::GetImageMetadataPayload()
{
	std::string payload = SIZE_PAYLOAD_KEY;
	payload.append(CLIENT_MSG_DELIMITER)
		.append(to_string(image_.cols)).append(CLIENT_MSG_DELIMITER)
		.append(to_string(image_.rows)).append(CLIENT_MSG_DELIMITER)
		.append(to_string(image_.total() * image_.elemSize())).append(CLIENT_MSG_DELIMITER)
		.append(to_string(filter_type_enum_)).append(ConvertFilterParamsToString()).append(EMPTY_STRING + STRING_TERMINATING_CHAR);

	return payload;
}

Mat ImageRequest::GetImage()
{
	return image_;
}

const std::string& ImageRequest::GetServerIp()
{
	return server_ip_;
}

const ushort& ImageRequest::GetServerPort()
{
	return server_port_;
}

const cv::String ImageRequest::GetImagePath()
{
	return image_absolute_path_;;
}
