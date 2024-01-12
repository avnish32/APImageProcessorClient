#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stoi;
using std::to_string;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<char*>& filter_params, Mat image):FilterParamsValidator(filter_params, image)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	if (!ValidateIntegerParams(0, 4)) {
		msg_logger_->LogError("ERROR: Invalid format for CROP filter parameters.");
		return false;
	}

	short top_left_corner_x = stoi(filter_params_.at(0));
	short top_left_corner_y = stoi(filter_params_.at(1));
	short target_width = stoi(filter_params_.at(2));
	short target_height = stoi(filter_params_.at(3));

	if (target_width <= 0 || target_height <= 0) {
		msg_logger_->LogError("ERROR: Invalid target dimension values for CROP filter parameters.");
		return false;
	}

	if (IsCoordinateOutsideImage(top_left_corner_x, top_left_corner_y)) {
		msg_logger_->LogError("ERROR: Given coordinate lies outside the image. Coordinate: ("
			+ to_string(top_left_corner_x) + "," + to_string(top_left_corner_y) + ") | Image dimensions: "
			+ to_string(image_.cols) + "x" + to_string(image_.rows));

		return false;
	}

	return true;
}

bool CropFilterParamsValidator::IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > image_.cols || y > image_.rows;
}
