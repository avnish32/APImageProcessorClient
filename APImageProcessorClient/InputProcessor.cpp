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
vector<char*> InputProcessor::_GetFilterParamsRaw(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<char*> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(*(_argValues + currentIndex + i));
	}
	_msgLogger->LogDebug("Raw filter params obtained.");
	return filterParams;
}

/*
This function extract the filter parameters as a vector of floating-point numbers from the command line arguments 
by iterating over the arguments starting from currentIndex until numberOfParams are obtained.
*/
vector<float> InputProcessor::_GetFilterParams(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<float> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(stof(*(_argValues + currentIndex + i)));
	}
	return filterParams;
}

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> InputProcessor::_SplitString(char* inputString, char delimiter) {
	std::string currentWord = EMPTY_STRING;
	vector<std::string> outputVector;
	int i = 0;

	_msgLogger->LogDebug("Starting to split string.");

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

	_msgLogger->LogDebug(stringAfterSplittingLogMsg);
	_msgLogger->LogDebug("Returning output vector.");

	return outputVector;
}

InputProcessor::InputProcessor(int argCount, char** argValues)
{
	_argCount = argCount;
	_argValues = argValues;
}

/*
Validates the command line arguments. A valid argument string should follow the below format:
<server ip:port> <absolute path of image> <filter name> <filter params...>
This function checks whether the server IP and port are in correct format, if the image at the given path
is valid, if the filter name is correct and the filter parameters corresponding to the given filter are correct.
*/
bool InputProcessor::ValidateInput()
{
	if (_argCount < MIN_CMD_LINE_ARGS) {
		_msgLogger->LogError("ERROR: Too few arguments.");
		return false;
	}

	regex serverURLRegex = basic_regex(SERVER_URL_VALIDATION_REGEX);
	if (!regex_search(*(_argValues + 1), serverURLRegex)) {
		_msgLogger->LogError("Invalid server URL.");
		return false;
	}

	FilterParamsValidator* filterParamsValidator = new FilterParamsValidator();
	ushort i = 2;
	while (*(_argValues + i) != nullptr) {

		Mat image = cv::imread(*(_argValues + i));
		if (image.empty()) {
			_msgLogger->LogError("ERROR: Image path contains empty image.");
			return false;
		}

		_msgLogger->LogDebug("Image read successfully.");
		i++;

		ImageFilterTypesEnum filterType = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(_argValues + i));
		if (filterType == INVALID_FILTER_TYPE) {
			stringstream sStream;
			sStream << *(_argValues + i);
			_msgLogger->LogError("ERROR: Invalid filter name: " + sStream.str() + " in input.");
			return false;
		}
			
		filterParamsValidator = FilterParamsValidatorFactory::GetFilterParamsValidator(filterType, _argValues, i, image);

		if (filterParamsValidator == nullptr || !filterParamsValidator->ValidateFilterParams()) {
			_msgLogger->LogError("Filter parameter validation failed.");
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

	char* serverDetails = *(_argValues + 1);
	vector<string> serverIpAndPort = _SplitString(serverDetails, ':');
	ushort serverPort = stoi(serverIpAndPort.at(1));

	int i = 2;
	while (*(_argValues + i) != nullptr) {
		cv::String imagePath = *(_argValues + i++);

		ImageFilterTypesEnum filterTypeEnum = ImageFilterEnums::GetImageFilterTypeEnumFromString(*(_argValues + i));
		vector<float> filterParams;
		stringstream sStream;

		switch (filterTypeEnum)
		{
		case ImageFilterTypesEnum::INVALID_FILTER_TYPE:
			sStream << *(_argValues + i);
			_msgLogger->LogError("ERROR: Invalid filter name: " + sStream.str() + " in input.");
			break;
		case RESIZE:
			filterParams = _GetFilterParams(i, 2);
			i += 3;
			break;
		case ROTATE:
			filterParams.push_back(ImageFilterEnums::GetRotationDirectionEnumFromString(*(_argValues + i + 1)));
			filterParams.push_back(stof(*(_argValues + i + 2)));
			i += 3;
			break;
		case BRIGHTNESS_ADJ:
			filterParams = _GetFilterParams(i, 1);
			i += 2;
			break;
		case FLIP:
			filterParams.push_back(ImageFilterEnums::GetFlipDirectionEnumFromString(*(_argValues + i + 1)));
			i += 2;
			break;
		case CROP:
			filterParams = _GetFilterParams(i, 4);
			i += 5;
			break;
		case RGB_TO_GRAYSCALE:
			i += 1;
			break;
		default:
			_msgLogger->LogError("ERROR: Invalid filter type.");
		}

		ImageRequest imageRequest(serverIpAndPort.at(0), serverPort, imagePath, filterTypeEnum, filterParams);
		imageRequests.push_back(imageRequest);
	}

	return imageRequests;
}
