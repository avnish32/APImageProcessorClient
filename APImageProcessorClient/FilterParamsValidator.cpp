#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper integers.
*/
bool FilterParamsValidator::ValidateIntegerParams(int startIndex, int numberOfParams)
{
	int currentIndex = startIndex;
	for (int i = 0; i < numberOfParams; i++) {
		char* currentArgValue = filter_params_.at(currentIndex);
		if (currentArgValue == nullptr) {
			return false;
		}

		try {
			std::stoi(currentArgValue);
		}
		catch (std::invalid_argument) {
			return false;
		}
		currentIndex++;
	}
	return true;
}

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper floating-point numbers.
*/
bool FilterParamsValidator::ValidateFloatParams(int startIndex, int numberOfParams)
{
	int currentIndex = startIndex;
	for (int i = 0; i < numberOfParams; i++) {
		char* currentArgValue = filter_params_.at(i);
		if (currentArgValue == nullptr) {
			return false;
		}

		try {
			std::stof(currentArgValue);
		}
		catch (std::invalid_argument) {
			return false;
		}
		currentIndex++;
	}
	return true;
}

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filterParams)
{
	filter_params_ = filterParams;
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filterParams, const Mat& image)
{
	filter_params_ = filterParams;
	image_ = image;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
