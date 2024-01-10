#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stoi;
using std::to_string;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<char*>& filterParams, Mat image):FilterParamsValidator(filterParams, image)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	if (!ValidateIntegerParams(0, 4)) {
		msg_logger_->LogError("ERROR: Invalid format for CROP filter parameters.");
		return false;
	}

	short cropTopLeftCornerX = stoi(filter_params_.at(0));
	short cropTopLeftCornerY = stoi(filter_params_.at(1));
	short targetWidth = stoi(filter_params_.at(2));
	short targetHeight = stoi(filter_params_.at(3));

	if (targetWidth <= 0 || targetHeight <= 0) {
		msg_logger_->LogError("ERROR: Invalid target dimension values for CROP filter parameters.");
		return false;
	}

	if (IsCoordinateOutsideImage(cropTopLeftCornerX, cropTopLeftCornerY)) {
		msg_logger_->LogError("ERROR: Given coordinate lies outside the image. Coordinate: ("
			+ to_string(cropTopLeftCornerX) + "," + to_string(cropTopLeftCornerY) + ") | Image dimensions: "
			+ to_string(image_.cols) + "x" + to_string(image_.rows));

		return false;
	}

	return true;
}

bool CropFilterParamsValidator::IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > image_.cols || y > image_.rows;
}
