#include "ImageConstructor.h"

#pragma once
class ImageConstructorFactory
{
public:
	static ImageConstructor* GetImageConstructor(const short& numOfChannels,
		const map<unsigned short, string>& imageDataMap, const Size& imageDimensions);
};

