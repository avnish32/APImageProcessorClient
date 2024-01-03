#include "ThreeChannelImageConstructor.h"
#include "Constants.h"

#include<string>

using std::to_string;
using cv::Vec3b;

ThreeChannelImageConstructor::ThreeChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
	:ImageConstructor(imageDataMap, imageDimensions)
{
}

ThreeChannelImageConstructor::~ThreeChannelImageConstructor()
{
	_msgLogger->LogDebug("Destroying object of ThreeChannelImageConstructor");
}

/*
This function constructs a triple channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat ThreeChannelImageConstructor::ConstructImage()
{
	_msgLogger->LogDebug("Constructing three channel image. Image data map size: " + to_string((ushort)_imageDataMap.size()));

	Mat image = Mat(_imageDimensions, CV_8UC3);
	unsigned short numberOfImageFragments = _imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(_imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < _imageDimensions.height; i++) {
		for (int j = 0; j < _imageDimensions.width; j++) {
			if (currentImageFragmentByte >= MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD) {
				currentImageFragment++;
				currentImageFragmentData = &(_imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			image.at<Vec3b>(i, j) = Vec3b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2));
			currentImageFragmentByte += 3;
		}
	}

	_msgLogger->LogDebug("Image constructed.");
	return image;
}
