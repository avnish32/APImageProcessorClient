#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::cout;
using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	cout << "\nInside ResizeFilterParamsValidator::ValidateFilterParams(). Filter params size: "<<_filterParams.size();

	if (!_ValidateIntegerParams(0, 2)) {
		cout << "\nERROR: Incorrect format for RESIZE filter parameters.";
		return false;
	}

	short targetWidth = stoi(_filterParams.at(0));
	short targetHeight = stoi(_filterParams.at(1));

	if (targetWidth <= 0 || targetHeight <= 0) {
		cout << "\nERROR: Width and height of resized image cannot be zero or negative.";
		return false;
	}

	cout << "\nInside ResizeFilterParamsValidator::ValidateFilterParams(). Validation successful.";
	return true;
}
