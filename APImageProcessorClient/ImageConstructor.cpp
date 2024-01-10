#include "ImageConstructor.h"

ImageConstructor::ImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions)
{
	image_data_map_ = imageDataMap;
	image_dimensions_ = imageDimensions;
}

ImageConstructor::~ImageConstructor()
{
	msg_logger_->LogDebug("Destroyed imageConstructor object.");
}
