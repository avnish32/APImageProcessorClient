#include "ThreeChannelImageConstructor.h"
#include "Constants.h"

#include<string>

using std::to_string;
using cv::Vec3b;

ThreeChannelImageConstructor::ThreeChannelImageConstructor(const map<unsigned short, string>& image_data_map, const Size& image_dimensions)
	:ImageConstructor(image_data_map, image_dimensions)
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
	unsigned short no_of_image_fragments = image_data_map_.size(), current_image_fragment = 1;
	int current_image_fragment_byte = 0;
	const char* current_image_fragment_data = &(image_data_map_[current_image_fragment][0]);

	for (int i = 0; i < image_dimensions_.height; i++) {
		for (int j = 0; j < image_dimensions_.width; j++) {
			if (current_image_fragment_byte >= MAX_IMG_DATA_BYTES_IN_PAYLOAD) {
				current_image_fragment++;
				current_image_fragment_data = &(image_data_map_[current_image_fragment][0]);
				current_image_fragment_byte = 0;
			}

			image.at<Vec3b>(i, j) = Vec3b(*(current_image_fragment_data + current_image_fragment_byte),
				*(current_image_fragment_data + current_image_fragment_byte + 1),
				*(current_image_fragment_data + current_image_fragment_byte + 2));
			current_image_fragment_byte += 3;
		}
	}

	msg_logger_->LogDebug("Image constructed.");
	return image;
}
