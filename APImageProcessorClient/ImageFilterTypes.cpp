#include "ImageFilterTypes.h"

/*
This function returns the enum object based on filterTypeString.
*/
ImageFilterTypesEnum ImageFilterTypes::GetImageFilterTypeEnumFromString(string filterTypeString)
{
    if (filterTypeString == "Resize") {
        return RESIZE;
    }

    if (filterTypeString == "Rotate") {
        return ROTATE;
    }

    if (filterTypeString == "Crop") {
        return CROP;
    }

    if (filterTypeString == "Flip") {
        return FLIP;
    }

    if (filterTypeString == "Grayscale") {
        return RGB_TO_GRAYSCALE;
    }

    if (filterTypeString == "Brightness") {
        return BRIGHTNESS_ADJ;
    }

    return NONE;
}
