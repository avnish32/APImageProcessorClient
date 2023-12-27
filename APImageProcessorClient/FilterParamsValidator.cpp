#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

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
