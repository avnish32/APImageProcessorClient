#include<iostream>
#include<regex>
#include<opencv2/opencv.hpp>

#include "InputProcessor.h"
#include "Constants.h"
#include "ImageFilterEnums.h"
#include "FilterParamsValidator.h"
#include "FilterParamsValidatorFactory.h"

using std::stoi;
using std::stol;
using std::stof;
using std::invalid_argument;
using std::stringstream;
using std::regex;
using std::basic_regex;
using std::regex_search;

using cv::Mat;

/*
This function extract the filter parameters as a vector of character pointers from the command line arguments 
by iterating over the arguments starting from currentIndex until numberOfParams are obtained.
*/
vector<char*> InputProcessor::GetFilterParamsRaw(const ushort& current_index, const ushort& no_of_params)
{
	vector<char*> filter_params;

	for (int i = 1; i <= no_of_params; i++) {
		filter_params.push_back(*(arg_values_ + current_index + i));
	}
	msg_logger_->LogDebug("Raw filter params obtained.");
	return filter_params;
}

/*
This function extract the filter parameters as a vector of floating-point numbers from the command line arguments 
by iterating over the arguments starting from currentIndex until numberOfParams are obtained.
*/
vector<float> InputProcessor::GetFilterParams(const ushort& current_index, const ushort& no_of_params)
{
	vector<float> filter_params;

	for (int i = 1; i <= no_of_params; i++) {
		filter_params.push_back(stof(*(arg_values_ + current_index + i)));
	}
	return filter_params;
}

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> InputProcessor::SplitString(char* input_string, char delimiter) {
	std::string current_word = EMPTY_STRING;
	vector<std::string> output_vector;
	int i = 0;

	msg_logger_->LogDebug("Starting to split string.");

	while (*(input_string + i) != STRING_TERMINATING_CHAR) {
		char current_char = *(input_string + i);
		if (current_char == delimiter) {
			output_vector.push_back(current_word);
			current_word = EMPTY_STRING;
		}
		else {
			current_word += current_char;
		}
		++i;
	}
	if (current_word.length() > 0) {
		output_vector.push_back(current_word);
	}

	std::string log_string_after_splitting = "String after splitting: ";

	auto iter = output_vector.begin();
	while (iter != output_vector.end()) {
		log_string_after_splitting.append(*iter).append(" | ");
		iter++;
	}

	msg_logger_->LogDebug(log_string_after_splitting);
	msg_logger_->LogDebug("Returning output vector.");

	return output_vector;
}

InputProcessor::InputProcessor(int arg_count, char** arg_values)
{
	arg_count_ = arg_count;
	arg_values_ = arg_values;
}

/*
Validates the command line arguments. A valid argument string should follow the below format:
<server ip:port> <absolute path of image> <filter name> <filter params...>
This function checks whether the server IP and port are in correct format, if the image at the given path
is valid, if the filter name is correct and the filter parameters corresponding to the given filter are correct.
*/
bool InputProcessor::ValidateInput()
{
	if (arg_count_ < MIN_CMD_LINE_ARGS) {
		msg_logger_->LogError("ERROR: Too few arguments.");
		return false;
	}

	regex server_url_regex = basic_regex(SERVER_URL_VALIDATION_REGEX);
	if (!regex_search(*(arg_values_ + 1), server_url_regex)) {
		msg_logger_->LogError("Invalid server URL.");
		return false;
	}

	FilterParamsValidator* filter_params_validator = new FilterParamsValidator();
	ushort i = 2;
	while (*(arg_values_ + i) != nullptr) {

		Mat image = cv::imread(*(arg_values_ + i));
		if (image.empty()) {
			msg_logger_->LogError("ERROR: Image path contains empty image.");
			return false;
		}

		msg_logger_->LogDebug("Image read successfully.");
		i++;

		ImageFilterTypesEnum filter_type = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(arg_values_ + i));
		if (filter_type == INVALID_FILTER_TYPE) {
			stringstream s_stream;
			s_stream << *(arg_values_ + i);
			msg_logger_->LogError("ERROR: Invalid filter name: " + s_stream.str() + " in input.");
			return false;
		}
			
		filter_params_validator = FilterParamsValidatorFactory::GetFilterParamsValidator(filter_type, arg_values_, i, image);

		if (filter_params_validator == nullptr || !filter_params_validator->ValidateFilterParams()) {
			msg_logger_->LogError("Filter parameter validation failed.");
			delete filter_params_validator;
			return false;
		}
	}

	delete filter_params_validator;
	return true;
}

/*
This function initializes objects of ImageRequest class based on the given command line arguments.
*/
vector<ImageRequest> InputProcessor::InitializeImageRequests()
{
	vector<ImageRequest> image_requests;

	char* server_details = *(arg_values_ + 1);
	vector<string> server_ip_and_port = SplitString(server_details, ':');
	ushort server_port = stoi(server_ip_and_port.at(1));

	int i = 2;
	while (*(arg_values_ + i) != nullptr) {
		cv::String image_path = *(arg_values_ + i++);

		ImageFilterTypesEnum filter_type_enum = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(arg_values_ + i));
		vector<float> filter_params;
		stringstream s_stream;

		switch (filter_type_enum)
		{
		case ImageFilterTypesEnum::INVALID_FILTER_TYPE:
			s_stream << *(arg_values_ + i);
			msg_logger_->LogError("ERROR: Invalid filter name: " + s_stream.str() + " in input.");
			break;
		case RESIZE:
			filter_params = GetFilterParams(i, 2);
			i += 3;
			break;
		case ROTATE:
			filter_params.push_back(ImageFilterEnums::GetRotationDirectionEnumFromString(*(arg_values_ + i + 1)));
			filter_params.push_back(stof(*(arg_values_ + i + 2)));
			i += 3;
			break;
		case BRIGHTNESS_ADJ:
			filter_params = GetFilterParams(i, 1);
			i += 2;
			break;
		case FLIP:
			filter_params.push_back(ImageFilterEnums::GetFlipDirectionEnumFromString(*(arg_values_ + i + 1)));
			i += 2;
			break;
		case CROP:
			filter_params = GetFilterParams(i, 4);
			i += 5;
			break;
		case RGB_TO_GRAYSCALE:
			i += 1;
			break;
		default:
			msg_logger_->LogError("ERROR: Invalid filter type.");
		}

		ImageRequest image_request(server_ip_and_port.at(0), server_port, image_path, filter_type_enum, filter_params);
		image_requests.push_back(image_request);
	}

	return image_requests;
}
