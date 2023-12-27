#include<iostream>
#include<opencv2/opencv.hpp>

#include "InputProcessor.h"
#include "Constants.h"
#include "ImageFilterTypes.h"
#include "FilterParamsValidator.h"
#include "ResizeFilterParamsValidator.h"
#include "RotateFilterParamsValidator.h"
#include "CropFilterParamsValidator.h"
#include "FlipFilterParamsValidator.h"
#include "BrightnessAdjFilterParamsValidator.h"

using std::cout;
using std::stoi;
using std::stol;
using std::stof;
using std::invalid_argument;
using cv::Mat;

vector<char*> InputProcessor::_GetFilterParamsRaw(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<char*> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(*(_argValues + currentIndex + i));
	}
	cout << "\nRaw filter params obtained.";
	return filterParams;
}

vector<float> InputProcessor::_GetFilterParams(const ushort& currentIndex, const ushort& numberOfParams)
{
	vector<float> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(stof(*(_argValues + currentIndex + i)));
	}
	return filterParams;
}

const vector<std::string> InputProcessor::_SplitString(char* inputString, char delimiter) {
	std::string currentWord = "";
	vector<std::string> outputVector;
	int i = 0;
	cout << "\nStarting to split string.";
	//cout << "\nString array at beginning: " << *stringArray;


	while (*(inputString + i) != '\0') {
		char currentChar = *(inputString + i);
		//cout << "\nCurrent word: " << currentWord<<" | current char = "<<currentChar;
		if (currentChar == delimiter) {
			outputVector.push_back(currentWord);
			currentWord = "";
		}
		else {
			currentWord += currentChar;
		}
		++i;
	}
	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	cout << "\nString after splitting: ";
	auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		cout << *iter << "|";
		iter++;
	}
	cout << "\nReturning output vector. ";
	return outputVector;
}

InputProcessor::InputProcessor(int argCount, char** argValues)
{
	_argCount = argCount;
	_argValues = argValues;
}

bool InputProcessor::ValidateInput()
{
	//Cmd line args format: <server ip:port><space><absolute path of image><space><filter name><space><filter params...>
	if (_argCount < MIN_CMD_LINE_ARGS) {
		cout << "\nERROR: Too few arguments.";
		return false;
	}

	//TODO Regex to check server IP and port

	FilterParamsValidator* filterParamsValidator = new FilterParamsValidator();
	int i = 2;
	while (*(_argValues + i) != nullptr) {

		Mat image = cv::imread(*(_argValues + i++));
		if (image.empty()) {
			cout << "\nERROR: Image path contains empty image.";
			return false;
		}

		cout << "\nImage read successfully.";

		switch (ImageFilterTypes::GetImageFilterTypeEnumFromString(*(_argValues+i)))
		{
		case NONE:
			cout << "\nERROR: Invalid filter name: " << *(_argValues + i) << " in input.";
			return false;
		case RESIZE:
			filterParamsValidator = new ResizeFilterParamsValidator(_GetFilterParamsRaw(i, 2));
			i += 3;
			break;
		case ROTATE:
			//TODO can consider taking direction input as string instead of numbers
			filterParamsValidator = new RotateFilterParamsValidator(_GetFilterParamsRaw(i, 2));
			i += 3;
			break;
		case FLIP:
			//TODO can consider taking direction input as string instead of numbers
			filterParamsValidator = new FlipFilterParamsValidator(_GetFilterParamsRaw(i, 1));
			i += 2;
			break;
		case CROP:
			filterParamsValidator = new CropFilterParamsValidator(_GetFilterParamsRaw(i, 4), image);
			i += 5;
			break;
		case RGB_TO_GRAYSCALE:
			//filterParamsValidator = new FilterParamsValidator();
			i += 1;
			break;
		case BRIGHTNESS_ADJ:
			filterParamsValidator = new BrightnessAdjFilterParamsValidator(_GetFilterParamsRaw(i, 1));
			i += 2;
			break;
		default:
			cout << "\nERROR: Invalid filter type.";
		}

		if (!filterParamsValidator->ValidateFilterParams()) {
			cout << "\nFilter parameter validation failed.";
			return false;
		}
	}

	return true;
}

vector<ImageRequest> InputProcessor::InitializeImageRequests()
{
	vector<ImageRequest> imageRequests;

	char* serverDetails = *(_argValues + 1);
	vector<string> serverIpAndPort = _SplitString(serverDetails, ':');
	long serverPort = stol(serverIpAndPort.at(1));

	int i = 2;
	while (*(_argValues + i) != nullptr) {
		cv::String imagePath = *(_argValues + i++);

		ImageFilterTypesEnum filterTypeEnum = ImageFilterTypes::GetImageFilterTypeEnumFromString(*(_argValues + i));
		vector<float> filterParams;
		switch (filterTypeEnum)
		{
		case NONE:
			cout << "\nERROR: Invalid filter name: " << *(_argValues + i) << " in input.";
			break;
		case RESIZE:
		case ROTATE:
			//TODO can consider taking direction input as string instead of numbers
			filterParams = _GetFilterParams(i, 2);
			i += 3;
			break;
		case BRIGHTNESS_ADJ:
		case FLIP:
			//TODO can consider taking direction input as string instead of numbers
			filterParams = _GetFilterParams(i, 1);
			i += 2;
			break;
		case CROP:
			filterParams = _GetFilterParams(i, 4);
			i += 5;
			break;
		case RGB_TO_GRAYSCALE:
			//filterParamsValidator = new FilterParamsValidator();
			i += 1;
			break;
		default:
			cout << "\nERROR: Invalid filter type.";
		}

		ImageRequest imageRequest(serverIpAndPort.at(0), serverPort, imagePath, filterTypeEnum, filterParams);
		imageRequests.push_back(imageRequest);
	}

	return imageRequests;
}
