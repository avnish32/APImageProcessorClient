#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper integers.
*/
bool FilterParamsValidator::_ValidateIntegerParams(int startIndex, int numberOfParams)
{
	for (int i = startIndex; i < numberOfParams; i++) {
		char* currentArgValue = _filterParams.at(i);
		if (currentArgValue == nullptr) {
			return false;
		}

		try {
			std::stoi(currentArgValue);
		}
		catch (std::invalid_argument) {
			return false;
		}
	}
	return true;
}

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper floating-point numbers.
*/
bool FilterParamsValidator::_ValidateFloatParams(int startIndex, int numberOfParams)
{
	for (int i = startIndex; i < numberOfParams; i++) {
		char* currentArgValue = _filterParams.at(i);
		if (currentArgValue == nullptr) {
			return false;
		}

		try {
			std::stof(currentArgValue);
		}
		catch (std::invalid_argument) {
			return false;
		}
	}
	return true;
}

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filterParams)
{
	_filterParams = filterParams;
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filterParams, const Mat& image)
{
	_filterParams = filterParams;
	_image = image;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
