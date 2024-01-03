#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stof;

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateFloatParams(0, 1)) {
		_msgLogger->LogError("ERROR: Invalid format for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}

	float brightnessAdjFactor = stof(_filterParams.at(0));
	if (brightnessAdjFactor < 0) {
		_msgLogger->LogError("ERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}
	return true;
}
