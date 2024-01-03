#include "FilterParamsValidatorFactory.h"
#include "BrightnessAdjFilterParamsValidator.h"
#include "CropFilterParamsValidator.h"
#include "FlipFilterParamsValidator.h"
#include "ResizeFilterParamsValidator.h"
#include "RotateFilterParamsValidator.h"

/*
This function extract the filter parameters as a vector of character pointers from the command line arguments
by iterating over argValues starting from currentIndex until numberOfParams are obtained.
*/
vector<char*> FilterParamsValidatorFactory::_GetFilterParamsRaw(char** argValues, ushort& currentIndex, const ushort& numberOfParams)
{
	vector<char*> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(*(argValues + currentIndex + i));
	}
	currentIndex += numberOfParams + 1;
	return filterParams;
}

/*
This function checks filterType and instantiates an object of the corresponding concrete derived class of FilterParamsValidator
with filterParams and imageDimensions.
*/
FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filterType,
	char** argValues, ushort& currentIndex, const cv::Mat& image)
{
	switch (filterType) {
	case ImageFilterTypesEnum::INVALID_FILTER_TYPE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 2));
	case ROTATE:
		return new RotateFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 2));
	case FLIP:
		return new FlipFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 1));
	case CROP:
		return new CropFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 4), image);
	case RGB_TO_GRAYSCALE:
		currentIndex += 1;
		return new FilterParamsValidator();
	case BRIGHTNESS_ADJ:
		return new BrightnessAdjFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 1));
	default:
		return nullptr;
	}
	return nullptr;
}
