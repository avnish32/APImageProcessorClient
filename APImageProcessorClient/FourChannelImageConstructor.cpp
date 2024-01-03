#include "FourChannelImageConstructor.h"
#include "Constants.h"

#include<string>

using std::to_string;
using cv::Vec4b;

FourChannelImageConstructor::FourChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
	:ImageConstructor(imageDataMap, imageDimensions)
{
}

FourChannelImageConstructor::~FourChannelImageConstructor()
{
	_msgLogger->LogDebug("Destroying object of FourChannelImageConstructor");
}

/*
This function constructs a quadruple channel image having the given imageDimensions
from the imageDataMap.
*/
Mat FourChannelImageConstructor::ConstructImage()
{
	_msgLogger->LogDebug("Constructing four channel image. Image data map size: " + to_string((ushort)_imageDataMap.size()));

	Mat image = Mat(_imageDimensions, CV_8UC4);
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

			image.at<Vec4b>(i, j) = Vec4b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2),
				*(currentImageFragmentData + currentImageFragmentByte + 3));
			currentImageFragmentByte += 4;
		}
	}

	_msgLogger->LogDebug("Image constructed.");
	return image;
}
