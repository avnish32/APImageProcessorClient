#pragma once
#include "FilterParamsValidator.h"
class FlipFilterParamsValidator : public FilterParamsValidator
{
public:
	FlipFilterParamsValidator(const vector<char*>&);
	bool ValidateFilterParams();
};

