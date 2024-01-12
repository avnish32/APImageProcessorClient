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

	std::string filter_params_string = CLIENT_MSG_DELIMITER;
	auto iter = filter_params_.begin();
	
	while (iter != filter_params_.end()) {
		float rounded_to_two_decimals = std::roundf((*iter) * 100) / 100;
		filter_params_string.append(std::format("{:.2f}", rounded_to_two_decimals)).append(CLIENT_MSG_DELIMITER);
		iter++;
	}
	return filter_params_string;
}

/*
Parameterized constructor; it initializes all the members which are needed to make a request to the server.
*/
ImageRequest::ImageRequest(std::string server_ip, ushort server_port, cv::String image_absolute_path,
	ImageFilterTypesEnum filter_type_enum, vector<float> filter_params)
{
	string log_image_req_params = string("Inside ImageRequest constructor. Server IP: ").append(server_ip)
		.append(" | server port: ").append(to_string(server_port))
		.append(" | Image path: ").append(image_absolute_path)
		.append(" | Filter type: ").append(to_string(filter_type_enum))
		.append(" | Filter params: ");

	auto iter = filter_params.begin();
	while (iter != filter_params.end()) {
		log_image_req_params.append(to_string(*iter)).append(" ");
		iter++;
	}

	msg_logger_->LogDebug(log_image_req_params);

	server_ip_ = server_ip;
	server_port_ = server_port;
	image_absolute_path_ = image_absolute_path;
	filter_type_enum_ = filter_type_enum;
	filter_params_ = filter_params;
	image_ = cv::imread(image_absolute_path, cv::IMREAD_COLOR);
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
	return image_absolute_path_;
}
