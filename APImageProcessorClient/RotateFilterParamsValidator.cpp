#include "RotateFilterParamsValidator.h"
#include "ImageFilterEnums.h"

#include<iostream>
#include<string>

using std::stoi;
using std::stringstream;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<char*>& filter_params):FilterParamsValidator(filter_params)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	RotationDirection rotation_direction = ImageFilterEnums::GetRotationDirectionEnumFromString(filter_params_.at(0));

	switch (rotation_direction) {
	case INVALID_ROTATION_DIRECTION:
		stringstream s_stream;
		s_stream << filter_params_.at(0);
		msg_logger_->LogError("ERROR: Invalid direction given for rotation: " + s_stream.str());
		return false;
	}

	if (!ValidateIntegerParams(1, 1)) {
		msg_logger_->LogError("ERROR: Incorrect format for ROTATE filter parameters.");
		return false;
	}

	short num_of_turns = stoi(filter_params_.at(1));

	if (num_of_turns < 0) {
		msg_logger_->LogError("ERROR: Invalid number of turns given for rotation.");
		return false;
	}
	return true;
}
