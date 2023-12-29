#include "FilterParamsValidatorFactory.h"
#include "BrightnessAdjFilterParamsValidator.h"
#include "CropFilterParamsValidator.h"
#include "FlipFilterParamsValidator.h"
#include "ResizeFilterParamsValidator.h"
#include "RotateFilterParamsValidator.h"

vector<char*> FilterParamsValidatorFactory::_GetFilterParamsRaw(char** argValues, ushort& currentIndex, const ushort& numberOfParams)
{
	vector<char*> filterParams;

	for (int i = 1; i <= numberOfParams; i++) {
		filterParams.push_back(*(argValues + currentIndex + i));
	}
	//cout << "\nRaw filter params obtained."
	currentIndex += numberOfParams + 1;
	return filterParams;
}

FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filterType,
	char** argValues, ushort& currentIndex, const cv::Mat& image)
{
	switch (filterType) {
	case NONE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 2));
	case ROTATE:
		//TODO can consider taking direction input as string instead of numbers
		return new RotateFilterParamsValidator(_GetFilterParamsRaw(argValues, currentIndex, 2));
	case FLIP:
		//TODO can consider taking direction input as string instead of numbers
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
