#pragma once
#include "ImageConstructor.h"

/*
This is a concrete class of the abstract class ImageConstructor.
It constructs a quadruple channel image.
*/
class FourChannelImageConstructor : public ImageConstructor
{
public:
	FourChannelImageConstructor(const map<unsigned short, string>&, const Size&);
	~FourChannelImageConstructor();
	Mat ConstructImage() override;
};

