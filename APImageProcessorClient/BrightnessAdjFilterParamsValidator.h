#pragma once
#include "FilterParamsValidator.h"
class BrightnessAdjFilterParamsValidator : public FilterParamsValidator
{
public:
    BrightnessAdjFilterParamsValidator(const vector<char*>& filterParams);
    bool ValidateFilterParams();
};

