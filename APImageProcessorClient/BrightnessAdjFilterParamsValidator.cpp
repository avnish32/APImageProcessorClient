#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stof;

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	if (!ValidateFloatParams(0, 1)) {
		msg_logger_->LogError("ERROR: Invalid format for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}

	float brightnessAdjFactor = stof(filter_params_.at(0));
	if (brightnessAdjFactor < 0) {
		msg_logger_->LogError("ERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}
	return true;
}
