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
	msg_logger_->LogDebug("Destroying object of FourChannelImageConstructor");
}

/*
This function constructs a quadruple channel image having the given imageDimensions
from the imageDataMap.
*/
Mat FourChannelImageConstructor::ConstructImage()
{
	msg_logger_->LogDebug("Constructing four channel image. Image data map size: " + to_string((ushort)image_data_map_.size()));

	Mat image = Mat(image_dimensions_, CV_8UC4);
	unsigned short numberOfImageFragments = image_data_map_.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(image_data_map_[currentImageFragment][0]);

	for (int i = 0; i < image_dimensions_.height; i++) {
		for (int j = 0; j < image_dimensions_.width; j++) {
			if (currentImageFragmentByte >= MAX_IMG_DATA_BYTES_IN_PAYLOAD) {

				currentImageFragment++;
				currentImageFragmentData = &(image_data_map_[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			image.at<Vec4b>(i, j) = Vec4b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2),
				*(currentImageFragmentData + currentImageFragmentByte + 3));
			currentImageFragmentByte += 4;
		}
	}

	msg_logger_->LogDebug("Image constructed.");
	return image;
}
