#include "ImageFilterEnums.h"

/*
This function returns the enum object based on filterTypeString.
*/
ImageFilterTypesEnum ImageFilterEnums::GetImageFilterTypeEnumFromString(string filter_type_string)
{
    if (filter_type_string == "Resize") {
        return RESIZE;
    }

    if (filter_type_string == "Rotate") {
        return ROTATE;
    }

    if (filter_type_string == "Crop") {
        return CROP;
    }

    if (filter_type_string == "Flip") {
        return FLIP;
    }

    if (filter_type_string == "Grayscale") {
        return RGB_TO_GRAYSCALE;
    }

    if (filter_type_string == "Brightness") {
        return BRIGHTNESS_ADJ;
    }

    return INVALID_FILTER_TYPE;
}

RotationDirection ImageFilterEnums::GetRotationDirectionEnumFromString(string rotation_direcion_string)
{
    if (rotation_direcion_string == "Clockwise" || rotation_direcion_string == "clockwise") {
        return CLOCKWISE;
    }

    if (rotation_direcion_string == "Anti-Clockwise"
        || rotation_direcion_string == "Anti-clockwise"
        || rotation_direcion_string == "anti-clockwise") {
        return ANTI_CLOCKWISE;
    }
    
    return INVALID_ROTATION_DIRECTION;
}

FlipDirection ImageFilterEnums::GetFlipDirectionEnumFromString(string flip_direction_string)
{
    if (flip_direction_string == "Horizontal" || flip_direction_string == "horizontal") {
        return HORIZONTAL;
    }

    if (flip_direction_string == "Vertical" || flip_direction_string == "vertical") {
        return VERTICAL;
    }

    return INVALID_FLIP_DIRECTION;
}
