#include "ImageFilterEnums.h"

/*
This function returns the enum object based on filterTypeString.
*/
ImageFilterTypesEnum ImageFilterEnums::GetImageFilterTypeEnumFromString(string filterTypeString)
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

    return INVALID_FILTER_TYPE;
}

RotationDirection ImageFilterEnums::GetRotationDirectionEnumFromString(string rotationDirectionString)
{
    if (rotationDirectionString == "Clockwise" || rotationDirectionString == "clockwise") {
        return CLOCKWISE;
    }

    if (rotationDirectionString == "Anti-Clockwise"
        || rotationDirectionString == "Anti-clockwise"
        || rotationDirectionString == "anti-clockwise") {
        return ANTI_CLOCKWISE;
    }
    
    return INVALID_ROTATION_DIRECTION;
}

FlipDirection ImageFilterEnums::GetFlipDirectionEnumFromString(string flipDirectionString)
{
    if (flipDirectionString == "Horizontal" || flipDirectionString == "horizontal") {
        return HORIZONTAL;
    }

    if (flipDirectionString == "Vertical" || flipDirectionString == "vertical") {
        return VERTICAL;
    }

    return INVALID_FLIP_DIRECTION;
}
