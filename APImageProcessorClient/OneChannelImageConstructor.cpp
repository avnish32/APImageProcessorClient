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
	msg_logger_->LogDebug("Destroying object of OneChannelImageConstructor");
}

/*
This function constructs a single channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat OneChannelImageConstructor::ConstructImage()
{
	msg_logger_->LogDebug("Constructing one channel image. Image data map size: " + to_string((ushort)image_data_map_.size()));

	Mat image = Mat(image_dimensions_, CV_8UC1);
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

			image.at<uchar>(i, j) = *(currentImageFragmentData + currentImageFragmentByte);
			currentImageFragmentByte += 1;
		}
	}

	msg_logger_->LogDebug("Image constructed.");
	return image;
}
