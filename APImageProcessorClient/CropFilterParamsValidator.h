#pragma once
#include "FilterParamsValidator.h"

class CropFilterParamsValidator : public FilterParamsValidator
{
private:
	bool _IsCoordinateOutsideImage(short cropTopLeftCornerX, short cropTopLeftCornerY);
public:
	CropFilterParamsValidator(const vector<char*>& filterParams, Mat image);
	bool ValidateFilterParams();
};

