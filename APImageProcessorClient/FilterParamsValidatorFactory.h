#include "FilterParamsValidator.h"
#include "Constants.h"
#include "ImageFilterEnums.h"

#include<opencv2/opencv.hpp>
#include<vector>

#pragma once
class FilterParamsValidatorFactory
{
private:
	static vector<char*> _GetFilterParamsRaw(char** argValues, ushort& currentIndex, const ushort& numberOfParams);
public:
	static FilterParamsValidator* GetFilterParamsValidator(const ImageFilterTypesEnum& filterType,
		char** argValues, ushort& currentIndex, const cv::Mat& image);
};