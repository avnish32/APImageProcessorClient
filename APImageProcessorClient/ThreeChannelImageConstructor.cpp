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
	msg_logger_->LogDebug("Destroying object of ThreeChannelImageConstructor");
}

/*
This function constructs a triple channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat ThreeChannelImageConstructor::ConstructImage()
{
	msg_logger_->LogDebug("Constructing three channel image. Image data map size: " + to_string((ushort)image_data_map_.size()));

	Mat image = Mat(image_dimensions_, CV_8UC3);
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

			image.at<Vec3b>(i, j) = Vec3b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2));
			currentImageFragmentByte += 3;
		}
	}

	msg_logger_->LogDebug("Image constructed.");
	return image;
}
