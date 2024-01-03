#include "TwoChannelImageConstructor.h"
#include "Constants.h"

#include<string>

using std::to_string;
using cv::Vec2b;

TwoChannelImageConstructor::TwoChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
	:ImageConstructor(imageDataMap, imageDimensions)
{
}

TwoChannelImageConstructor::~TwoChannelImageConstructor()
{
	_msgLogger->LogDebug("Destroying object of TwoChannelImageConstructor");
}

/*
This function constructs a double channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat TwoChannelImageConstructor::ConstructImage()
{
	_msgLogger->LogDebug("Constructing two channel image. Image data map size: " + to_string((ushort)_imageDataMap.size()));

	Mat image = Mat(_imageDimensions, CV_8UC2);
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

			image.at<Vec2b>(i, j) = Vec2b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1));
			currentImageFragmentByte += 2;
		}
	}

	_msgLogger->LogDebug("Image constructed.");
	return image;
}
