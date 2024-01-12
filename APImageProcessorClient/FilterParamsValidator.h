#include<vector>
#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

#pragma once

using std::vector;
using cv::Mat;

/*
This is an abstract class having a virtual function ValidateFilterParams().
The children of this class contain validation logic for filter parameters of
each filter that the application supports.
*/

class FilterParamsValidator
{
protected:
	vector<char*> filter_params_;
	Mat image_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	bool ValidateIntegerParams(int, int);
	bool ValidateFloatParams(int, int);

public:
	FilterParamsValidator();
	FilterParamsValidator(const vector<char*>&);
	FilterParamsValidator(const vector<char*>&, const Mat&);
	virtual bool ValidateFilterParams();
};

