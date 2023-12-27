#include<vector>
#include<opencv2/opencv.hpp>

#pragma once

using std::vector;
using cv::Mat;

class FilterParamsValidator
{
protected:
	vector<char*> _filterParams;
	Mat _image;
	bool _ValidateIntegerParams(int startIndex, int numberOfParams);
	bool _ValidateFloatParams(int startIndex, int numberOfParams);

public:
	FilterParamsValidator();
	FilterParamsValidator(const vector<char*>& filterParams);
	FilterParamsValidator(const vector<char*>& filterParams, const Mat& image);
	virtual bool ValidateFilterParams();
};
