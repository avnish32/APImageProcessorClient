#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	if (!ValidateIntegerParams(0, 2)) {
		msg_logger_->LogError("ERROR: Incorrect format for RESIZE filter parameters.");
		return false;
	}

	short targetWidth = stoi(filter_params_.at(0));
	short targetHeight = stoi(filter_params_.at(1));

	if (targetWidth <= 0 || targetHeight <= 0) {
		msg_logger_->LogError("ERROR: Width and height of resized image cannot be zero or negative.");
		return false;
	}
	return true;
}
;