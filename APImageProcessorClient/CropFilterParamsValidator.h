#pragma once
#include "FilterParamsValidator.h"

class CropFilterParamsValidator : public FilterParamsValidator
{
private:
	bool IsCoordinateOutsideImage(short, short);
public:
	CropFilterParamsValidator(const vector<char*>&, Mat);
	bool ValidateFilterParams();
};

