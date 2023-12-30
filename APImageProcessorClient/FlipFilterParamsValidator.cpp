#include "FlipFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<char*>& filterParams):FilterParamsValidator(filterParams)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateIntegerParams(0, 1)) {
		//cout << "\nERROR: Incorrect format for FLIP filter parameters.";
		_msgLogger->LogError("ERROR: Incorrect format for FLIP filter parameters.");
		return false;
	}

	short direction = stoi(_filterParams.at(0));

	//TODO use enum for this
	if (direction != 0 && direction != 1) {
		//cout << "\nERROR: Invalid direction given for flipping.";
		_msgLogger->LogError("ERROR: Invalid direction given for flipping.");
		return false;
	}
	return true;
}
