#include "ImageConstructor.h"

#pragma once
class ImageConstructorFactory
{
public:
	static ImageConstructor* GetImageConstructor(const short&,
		const map<unsigned short, string>&, const Size&);
};

