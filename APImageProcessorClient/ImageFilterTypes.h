#include<iostream>

#pragma once

using std::string;

/*
Enum to store various filter types supported by the application.
*/
enum ImageFilterTypesEnum
{
	NONE, RESIZE, ROTATE, CROP, FLIP, RGB_TO_GRAYSCALE, BRIGHTNESS_ADJ
};

class ImageFilterTypes
{
public:
	static ImageFilterTypesEnum GetImageFilterTypeEnumFromString(string filterTypeString);
};

