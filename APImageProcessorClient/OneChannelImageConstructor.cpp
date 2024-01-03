#include "OneChannelImageConstructor.h"
#include "Constants.h"

#include<string>

using std::to_string;

OneChannelImageConstructor::OneChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
	:ImageConstructor(imageDataMap, imageDimensions)
{
}

OneChannelImageConstructor::~OneChannelImageConstructor()
{
	_msgLogger->LogDebug("Destroying object of OneChannelImageConstructor");
}

/*
This function constructs a single channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat OneChannelImageConstructor::ConstructImage()
{
	_msgLogger->LogDebug("Constructing one channel image. Image data map size: " + to_string((ushort)_imageDataMap.size()));

	Mat image = Mat(_imageDimensions, CV_8UC1);
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

			image.at<uchar>(i, j) = *(currentImageFragmentData + currentImageFragmentByte);
			currentImageFragmentByte += 1;
		}
	}

	_msgLogger->LogDebug("Image constructed.");
	return image;
}
