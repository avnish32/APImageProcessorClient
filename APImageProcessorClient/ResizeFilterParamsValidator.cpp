#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateIntegerParams(0, 2)) {
		_msgLogger->LogError("ERROR: Incorrect format for RESIZE filter parameters.");
		return false;
	}

	short targetWidth = stoi(_filterParams.at(0));
	short targetHeight = stoi(_filterParams.at(1));

	if (targetWidth <= 0 || targetHeight <= 0) {
		_msgLogger->LogError("ERROR: Width and height of resized image cannot be zero or negative.");
		return false;
	}
	return true;
}
;