#include<vector>

#include "ImageRequest.h"

#pragma once

using std::vector;

class InputProcessor
{
private:
	int _argCount;
	char** _argValues;

	vector<char*> _GetFilterParamsRaw(const ushort& currentIndex, const ushort& numberOfParams);
	vector<float> _GetFilterParams(const ushort& currentIndex, const ushort& numberOfParams);
	const vector<std::string> _SplitString(char* inputString, char delimiter);

public:
	InputProcessor(int argCount, char** argValues);
	bool ValidateInput();
	vector<ImageRequest> InitializeImageRequests();


};

