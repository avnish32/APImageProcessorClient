#pragma once
#include "ImageConstructor.h"

/*
This is a concrete class of the abstract class ImageConstructor.
It constructs a single channel image.
*/
class OneChannelImageConstructor : public ImageConstructor
{
public:
	OneChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions);
	~OneChannelImageConstructor();
	Mat ConstructImage() override;
};