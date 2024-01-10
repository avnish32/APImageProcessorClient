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
vector<char*> InputProcessor::GetFilterParamsRaw(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<char*> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(*(arg_values_ + currentIndex + i));
	}
	msg_logger_->LogDebug("Raw filter params obtained.");
	return filterParams;
}

/*
This function extract the filter parameters as a vector of floating-point numbers from the command line arguments 
by iterating over the arguments starting from currentIndex until numberOfParams are obtained.
*/
vector<float> InputProcessor::GetFilterParams(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<float> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(stof(*(arg_values_ + currentIndex + i)));
	}
	return filterParams;
}

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> InputProcessor::SplitString(char* inputString, char delimiter) {
	std::string currentWord = EMPTY_STRING;
	vector<std::string> outputVector;
	int i = 0;

	msg_logger_->LogDebug("Starting to split string.");

	while (*(inputString + i) != STRING_TERMINATING_CHAR) {
		char currentChar = *(inputString + i);
		if (currentChar == delimiter) {
			outputVector.push_back(currentWord);
			currentWord = EMPTY_STRING;
		}
		else {
			currentWord += currentChar;
		}
		++i;
	}
	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	std::string stringAfterSplittingLogMsg = "String after splitting: ";

	auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		stringAfterSplittingLogMsg.append(*iter).append(" | ");
		iter++;
	}

	msg_logger_->LogDebug(stringAfterSplittingLogMsg);
	msg_logger_->LogDebug("Returning output vector.");

	return outputVector;
}

InputProcessor::InputProcessor(int argCount, char** argValues)
{
	arg_count_ = argCount;
	arg_values_ = argValues;
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

	regex serverURLRegex = basic_regex(SERVER_URL_VALIDATION_REGEX);
	if (!regex_search(*(arg_values_ + 1), serverURLRegex)) {
		msg_logger_->LogError("Invalid server URL.");
		return false;
	}

	FilterParamsValidator* filterParamsValidator = new FilterParamsValidator();
	ushort i = 2;
	while (*(arg_values_ + i) != nullptr) {

		Mat image = cv::imread(*(arg_values_ + i));
		if (image.empty()) {
			msg_logger_->LogError("ERROR: Image path contains empty image.");
			return false;
		}

		msg_logger_->LogDebug("Image read successfully.");
		i++;

		ImageFilterTypesEnum filterType = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(arg_values_ + i));
		if (filterType == INVALID_FILTER_TYPE) {
			stringstream sStream;
			sStream << *(arg_values_ + i);
			msg_logger_->LogError("ERROR: Invalid filter name: " + sStream.str() + " in input.");
			return false;
		}
			
		filterParamsValidator = FilterParamsValidatorFactory::GetFilterParamsValidator(filterType, arg_values_, i, image);

		if (filterParamsValidator == nullptr || !filterParamsValidator->ValidateFilterParams()) {
			msg_logger_->LogError("Filter parameter validation failed.");
			delete filterParamsValidator;
			return false;
		}
	}

	delete filterParamsValidator;
	return true;
}

/*
This function initializes objects of ImageRequest class based on the given command line arguments.
*/
vector<ImageRequest> InputProcessor::InitializeImageRequests()
{
	vector<ImageRequest> imageRequests;

	char* serverDetails = *(arg_values_ + 1);
	vector<string> serverIpAndPort = SplitString(serverDetails, ':');
	ushort serverPort = stoi(serverIpAndPort.at(1));

	int i = 2;
	while (*(arg_values_ + i) != nullptr) {
		cv::String imagePath = *(arg_values_ + i++);

		ImageFilterTypesEnum filterTypeEnum = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(arg_values_ + i));
		vector<float> filterParams;
		stringstream sStream;

		switch (filterTypeEnum)
		{
		case ImageFilterTypesEnum::INVALID_FILTER_TYPE:
			sStream << *(arg_values_ + i);
			msg_logger_->LogError("ERROR: Invalid filter name: " + sStream.str() + " in input.");
			break;
		case RESIZE:
			filterParams = GetFilterParams(i, 2);
			i += 3;
			break;
		case ROTATE:
			filterParams.push_back(ImageFilterEnums::GetRotationDirectionEnumFromString(*(arg_values_ + i + 1)));
			filterParams.push_back(stof(*(arg_values_ + i + 2)));
			i += 3;
			break;
		case BRIGHTNESS_ADJ:
			filterParams = GetFilterParams(i, 1);
			i += 2;
			break;
		case FLIP:
			filterParams.push_back(ImageFilterEnums::GetFlipDirectionEnumFromString(*(arg_values_ + i + 1)));
			i += 2;
			break;
		case CROP:
			filterParams = GetFilterParams(i, 4);
			i += 5;
			break;
		case RGB_TO_GRAYSCALE:
			i += 1;
			break;
		default:
			msg_logger_->LogError("ERROR: Invalid filter type.");
		}

		ImageRequest imageRequest(serverIpAndPort.at(0), serverPort, imagePath, filterTypeEnum, filterParams);
		imageRequests.push_back(imageRequest);
	}

	return imageRequests;
}
