#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<char*>& filter_params):FilterParamsValidator(filter_params)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	if (!ValidateIntegerParams(0, 2)) {
		msg_logger_->LogError("ERROR: Incorrect format for RESIZE filter parameters.");
		return false;
	}

	short target_width = stoi(filter_params_.at(0));
	short target_height = stoi(filter_params_.at(1));

	if (target_width <= 0 || target_height <= 0) {
		msg_logger_->LogError("ERROR: Width and height of resized image cannot be zero or negative.");
		return false;
	}
	return true;
}
;