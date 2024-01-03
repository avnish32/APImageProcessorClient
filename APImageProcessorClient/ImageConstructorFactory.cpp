#include "ImageConstructorFactory.h"
#include "OneChannelImageConstructor.h"
#include "TwoChannelImageConstructor.h"
#include "ThreeChannelImageConstructor.h"
#include "FourChannelImageConstructor.h"

ImageConstructor* ImageConstructorFactory::GetImageConstructor(const short& numOfChannels,
	const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
{
	switch (numOfChannels) {
	case 1:
		return new OneChannelImageConstructor(imageDataMap, imageDimensions);
	case 2:
		return new TwoChannelImageConstructor(imageDataMap, imageDimensions);
	case 3:
		return new ThreeChannelImageConstructor(imageDataMap, imageDimensions);
	case 4:
		return new FourChannelImageConstructor(imageDataMap, imageDimensions);
	default:
		return nullptr;
	}
	return nullptr;
}
