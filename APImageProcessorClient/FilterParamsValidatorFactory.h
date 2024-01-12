#include "FilterParamsValidator.h"
#include "Constants.h"
#include "ImageFilterEnums.h"

#include<opencv2/opencv.hpp>
#include<vector>

#pragma once
class FilterParamsValidatorFactory
{
private:
	static vector<char*> GetFilterParamsRaw(char**, ushort&, const ushort&);
public:
	static FilterParamsValidator* GetFilterParamsValidator(const ImageFilterTypesEnum&,
		char**, ushort&, const cv::Mat&);
};