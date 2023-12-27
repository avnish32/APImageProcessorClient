#pragma once
#include "FilterParamsValidator.h"
class RotateFilterParamsValidator : public FilterParamsValidator
{
public:
	RotateFilterParamsValidator(const vector<char*>& filterParams);
	bool ValidateFilterParams();
};

