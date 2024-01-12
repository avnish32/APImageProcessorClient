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
vector<char*> FilterParamsValidatorFactory::GetFilterParamsRaw(char** arg_values, ushort& current_index, const ushort& no_of_params)
{
	vector<char*> filter_params;

	for (int i = 1; i <= no_of_params; i++) {
		filter_params.push_back(*(arg_values + current_index + i));
	}
	current_index += no_of_params + 1;
	return filter_params;
}

/*
This function checks filterType and instantiates an object of the corresponding concrete derived class of FilterParamsValidator
with filterParams and imageDimensions.
*/
FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filter_type,
	char** arg_values, ushort& current_index, const cv::Mat& image)
{
	switch (filter_type) {
	case ImageFilterTypesEnum::INVALID_FILTER_TYPE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(GetFilterParamsRaw(arg_values, current_index, 2));
	case ROTATE:
		return new RotateFilterParamsValidator(GetFilterParamsRaw(arg_values, current_index, 2));
	case FLIP:
		return new FlipFilterParamsValidator(GetFilterParamsRaw(arg_values, current_index, 1));
	case CROP:
		return new CropFilterParamsValidator(GetFilterParamsRaw(arg_values, current_index, 4), image);
	case RGB_TO_GRAYSCALE:
		current_index += 1;
		return new FilterParamsValidator();
	case BRIGHTNESS_ADJ:
		return new BrightnessAdjFilterParamsValidator(GetFilterParamsRaw(arg_values, current_index, 1));
	default:
		return nullptr;
	}
	return nullptr;
}
