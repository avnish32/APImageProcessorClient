#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<char*>& filterParams, Mat image):FilterParamsValidator(filterParams, image)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateIntegerParams(0, 4)) {
		cout << "\nERROR: Invalid format for CROP filter parameters.";
		return false;
	}

	short cropTopLeftCornerX = stoi(_filterParams.at(0));
	short cropTopLeftCornerY = stoi(_filterParams.at(1));
	short targetWidth = stoi(_filterParams.at(2));
	short targetHeight = stoi(_filterParams.at(3));

	if (cropTopLeftCornerX < 0 || cropTopLeftCornerY < 0 || targetWidth <= 0 || targetHeight <= 0) {
		cout << "\nERROR: Invalid values for CROP filter parameters.";
		return false;
	}

	if (cropTopLeftCornerX < _image.cols || cropTopLeftCornerY < _image.rows) {
		cout << "\nERROR: Given coordinate lies outside the image.";
		return false;
	}

	return true;
}
