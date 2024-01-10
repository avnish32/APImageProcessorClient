#include<vector>

#include "ImageRequest.h"
#include "MsgLogger.h"

#pragma once

using std::vector;

/*
This class handles the operations to be performed on input submitted by the user
through the command line.
*/
class InputProcessor
{
private:
	int arg_count_;
	char** arg_values_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	vector<char*> GetFilterParamsRaw(const ushort& currentIndex, const ushort& numberOfParams);
	vector<float> GetFilterParams(const ushort& currentIndex, const ushort& numberOfParams);
	const vector<std::string> SplitString(char* inputString, char delimiter);

public:
	InputProcessor(int argCount, char** argValues);
	bool ValidateInput();
	vector<ImageRequest> InitializeImageRequests();
};

