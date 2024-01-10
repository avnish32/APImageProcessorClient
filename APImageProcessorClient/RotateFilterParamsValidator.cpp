#include "RotateFilterParamsValidator.h"
#include "ImageFilterEnums.h"

#include<iostream>
#include<string>

using std::stoi;
using std::stringstream;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	RotationDirection rotationDirection = ImageFilterEnums::GetRotationDirectionEnumFromString(filter_params_.at(0));

	switch (rotationDirection) {
	case INVALID_ROTATION_DIRECTION:
		stringstream sStream;
		sStream << filter_params_.at(0);
		msg_logger_->LogError("ERROR: Invalid direction given for rotation: " + sStream.str());
		return false;
	}

	if (!ValidateIntegerParams(1, 1)) {
		msg_logger_->LogError("ERROR: Incorrect format for ROTATE filter parameters.");
		return false;
	}

	short numOfTurns = stoi(filter_params_.at(1));

	if (numOfTurns < 0) {
		msg_logger_->LogError("ERROR: Invalid number of turns given for rotation.");
		return false;
	}
	return true;
}
