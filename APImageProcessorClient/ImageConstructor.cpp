#include "ImageConstructor.h"

ImageConstructor::ImageConstructor(const map<unsigned short, string>& image_data_map, const Size& image_dimensions)
{
	image_data_map_ = image_data_map;
	image_dimensions_ = image_dimensions;
}

ImageConstructor::~ImageConstructor()
{
	msg_logger_->LogDebug("Destroyed imageConstructor object.");
}
