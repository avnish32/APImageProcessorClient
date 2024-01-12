#include "FlipFilterParamsValidator.h"
#include "ImageFilterEnums.h"

#include<iostream>
#include<string>

using std::stoi;
using std::stringstream;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<char*>& filter_params):FilterParamsValidator(filter_params)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	FlipDirection flip_direction = ImageFilterEnums::GetFlipDirectionEnumFromString(filter_params_.at(0));

	switch (flip_direction) {
	case INVALID_FLIP_DIRECTION:
		stringstream s_stream;
		s_stream << filter_params_.at(0);
		msg_logger_->LogError("ERROR: Invalid direction given for flipping: "+ s_stream.str());
		return false;
	}
	return true;
}
