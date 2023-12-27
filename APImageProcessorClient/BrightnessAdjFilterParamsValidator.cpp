#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateFloatParams(0, 1)) {
		cout << "\nERROR: Invalid format for BRIGHTNESS ADJUSTMENT filter parameters.";
		return false;
	}

	short brightnessAdjFactor = stoi(_filterParams.at(0));
	if (brightnessAdjFactor < 0) {
		cout << "\nERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.";
		return false;
	}
	return true;
}
