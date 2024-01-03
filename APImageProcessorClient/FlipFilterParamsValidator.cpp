#include "FlipFilterParamsValidator.h"
#include "ImageFilterEnums.h"

#include<iostream>
#include<string>

using std::stoi;
using std::stringstream;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	FlipDirection flipDirection = ImageFilterEnums::GetFlipDirectionEnumFromString(_filterParams.at(0));

	switch (flipDirection) {
	case INVALID_FLIP_DIRECTION:
		stringstream sStream;
		sStream << _filterParams.at(0);
		_msgLogger->LogError("ERROR: Invalid direction given for flipping: "+ sStream.str());
		return false;
	}
	return true;
}
