#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;
using std::stoi;
using std::to_string;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<char*>& filterParams, Mat image):FilterParamsValidator(filterParams, image)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	if (!_ValidateIntegerParams(0, 4)) {
		//cout << "\nERROR: Invalid format for CROP filter parameters.";
		_msgLogger->LogError("ERROR: Invalid format for CROP filter parameters.");
		return false;
	}

	short cropTopLeftCornerX = stoi(_filterParams.at(0));
	short cropTopLeftCornerY = stoi(_filterParams.at(1));
	short targetWidth = stoi(_filterParams.at(2));
	short targetHeight = stoi(_filterParams.at(3));

	if (targetWidth <= 0 || targetHeight <= 0) {
		//cout << "\nERROR: Invalid target dimension values for CROP filter parameters.";
		_msgLogger->LogError("ERROR: Invalid target dimension values for CROP filter parameters.");
		return false;
	}

	if (_IsCoordinateOutsideImage(cropTopLeftCornerX, cropTopLeftCornerY)) {
		/*cout << "\nERROR: Given coordinate lies outside the image. Coordinate: ("
			<<cropTopLeftCornerX<<","<<cropTopLeftCornerY<<") | Image dimensions: "
			<<_image.cols<<"x"<<_image.rows;*/
		_msgLogger->LogError("ERROR: Given coordinate lies outside the image. Coordinate: ("
			+ to_string(cropTopLeftCornerX) + "," + to_string(cropTopLeftCornerY) + ") | Image dimensions: "
			+ to_string(_image.cols) + "x" + to_string(_image.rows));

		return false;
	}

	return true;
}

bool CropFilterParamsValidator::_IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > _image.cols || y > _image.rows;
}
