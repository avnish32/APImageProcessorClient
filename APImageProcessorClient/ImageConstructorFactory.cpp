#include "ImageConstructorFactory.h"
#include "OneChannelImageConstructor.h"
#include "TwoChannelImageConstructor.h"
#include "ThreeChannelImageConstructor.h"
#include "FourChannelImageConstructor.h"

ImageConstructor* ImageConstructorFactory::GetImageConstructor(const short& no_of_channels,
	const map<unsigned short, string>& image_data_map, const Size& image_dimensions)
{
	switch (no_of_channels) {
	case 1:
		return new OneChannelImageConstructor(image_data_map, image_dimensions);
	case 2:
		return new TwoChannelImageConstructor(image_data_map, image_dimensions);
	case 3:
		return new ThreeChannelImageConstructor(image_data_map, image_dimensions);
	case 4:
		return new FourChannelImageConstructor(image_data_map, image_dimensions);
	default:
		return nullptr;
	}
	return nullptr;
}
