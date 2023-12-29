#pragma once
#include "FilterParamsValidator.h"
class ResizeFilterParamsValidator : public FilterParamsValidator
{
public:
	ResizeFilterParamsValidator(const vector<char*>& filterParams);
	bool ValidateFilterParams();
};