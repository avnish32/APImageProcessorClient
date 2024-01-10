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
	msg_logger_->LogDebug("Destroying object of TwoChannelImageConstructor");
}

/*
This function constructs a double channel image having the given _imageDimensions
from the _imageDataMap.
*/
Mat TwoChannelImageConstructor::ConstructImage()
{
	msg_logger_->LogDebug("Constructing two channel image. Image data map size: " + to_string((ushort)image_data_map_.size()));

	Mat image = Mat(image_dimensions_, CV_8UC2);
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

			image.at<Vec2b>(i, j) = Vec2b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1));
			currentImageFragmentByte += 2;
		}
	}

	msg_logger_->LogDebug("Image constructed.");
	return image;
}
