#include "ImageConstructor.h"

ImageConstructor::ImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
{
	_imageDataMap = imageDataMap;
	_imageDimensions = imageDimensions;
}

ImageConstructor::~ImageConstructor()
{
	_msgLogger->LogDebug("Destroyed imageConstructor object.");
}
