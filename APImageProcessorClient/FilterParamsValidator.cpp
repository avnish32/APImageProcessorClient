#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper integers.
*/
bool FilterParamsValidator::ValidateIntegerParams(int start_index, int no_of_params)
{
	int current_index = start_index;
	for (int i = 0; i < no_of_params; i++) {
		char* current_arg_value = filter_params_.at(current_index);
		if (current_arg_value == nullptr) {
			return false;
		}

		try {
			std::stoi(current_arg_value);
		}
		catch (std::invalid_argument) {
			return false;
		}
		current_index++;
	}
	return true;
}

/*
This function checks whether the parameters starting from startIndex upto numberOfParams in the _filterParams
vector are proper floating-point numbers.
*/
bool FilterParamsValidator::ValidateFloatParams(int start_index, int no_of_params)
{
	int current_index = start_index;
	for (int i = 0; i < no_of_params; i++) {
		char* current_arg_value = filter_params_.at(i);
		if (current_arg_value == nullptr) {
			return false;
		}

		try {
			std::stof(current_arg_value);
		}
		catch (std::invalid_argument) {
			return false;
		}
		current_index++;
	}
	return true;
}

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filter_params)
{
	filter_params_ = filter_params;
}

FilterParamsValidator::FilterParamsValidator(const vector<char*>& filter_params, const Mat& image)
{
	filter_params_ = filter_params;
	image_ = image;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
