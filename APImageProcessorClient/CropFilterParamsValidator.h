#pragma once
#include "FilterParamsValidator.h"

class CropFilterParamsValidator : public FilterParamsValidator
{
public:
	CropFilterParamsValidator(const vector<char*>& filterParams, Mat image);
	bool ValidateFilterParams();
};

