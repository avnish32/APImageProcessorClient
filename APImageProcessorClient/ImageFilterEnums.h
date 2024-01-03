#include<iostream>

#pragma once

using std::string;

/*
Enum to store various filter types supported by the application.
*/
enum ImageFilterTypesEnum
{
	INVALID_FILTER_TYPE, RESIZE, ROTATE, CROP, FLIP, RGB_TO_GRAYSCALE, BRIGHTNESS_ADJ
};

/*
Enum to store possible rotation directions.
*/
enum RotationDirection {
	CLOCKWISE, ANTI_CLOCKWISE, INVALID_ROTATION_DIRECTION
};

/*
Enum to store possible flipping directions.
*/
enum FlipDirection {
	HORIZONTAL, VERTICAL, INVALID_FLIP_DIRECTION
};

class ImageFilterEnums
{
public:
	static ImageFilterTypesEnum GetImageFilterTypeEnumFromString(string filterTypeString);
	static RotationDirection GetRotationDirectionEnumFromString(string rotationDirectionString);
	static FlipDirection GetFlipDirectionEnumFromString(string flipDirectionString);
};

