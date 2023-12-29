#include "RotateFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateIntegerParams(0, 2)) {
		cout << "\nERROR: Incorrect format for ROTATE filter parameters.";
		return false;
	}

	short direction = stoi(_filterParams.at(0));
	short numOfTurns = stoi(_filterParams.at(1));

	//TODO use enum for this
	if (direction != 0 && direction != 1) {
		cout << "\nERROR: Invalid direction given for rotation.";
		return false;
	}

	if (numOfTurns < 0) {
		cout << "\nERROR: Invalid number of turns given for rotation.";
		return false;
	}
	
	return true;
}
