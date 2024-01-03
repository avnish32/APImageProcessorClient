#include "RotateFilterParamsValidator.h"
#include "ImageFilterEnums.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;
using std::stringstream;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	RotationDirection rotationDirection = ImageFilterEnums::GetRotationDirectionEnumFromString(_filterParams.at(0));

	switch (rotationDirection) {
	case INVALID_ROTATION_DIRECTION:
		stringstream sStream;
		sStream << _filterParams.at(0);
		_msgLogger->LogError("ERROR: Invalid direction given for rotation: " + sStream.str());
		return false;
	}

	if (!_ValidateIntegerParams(1, 1)) {
		//cout << "\nERROR: Incorrect format for ROTATE filter parameters.";
		_msgLogger->LogError("ERROR: Incorrect format for ROTATE filter parameters.");
		return false;
	}

	//short direction = stoi(_filterParams.at(0));
	short numOfTurns = stoi(_filterParams.at(1));

	//TODO use enum for this
	//if (direction != 0 && direction != 1) {
	//	//cout << "\nERROR: Invalid direction given for rotation.";
	//	_msgLogger->LogError("ERROR: Invalid direction given for rotation.");
	//	return false;
	//}

	if (numOfTurns < 0) {
		//cout << "\nERROR: Invalid number of turns given for rotation.";
		_msgLogger->LogError("ERROR: Invalid number of turns given for rotation.");
		return false;
	}
	
	return true;
}
