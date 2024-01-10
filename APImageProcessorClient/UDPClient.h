#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string>
#include<map>
#include<queue>
#include<mutex>

#include<opencv2/opencv.hpp>

#include "MsgLogger.h"
#include "ImageProcessor.h"

using std::string;
using std::map;
using std::vector;
using std::queue;
using std::chrono::high_resolution_clock;

using cv::Mat;

#pragma once

/*
This class represents a UDP-based client that communicates with the server
using various functions provided.
*/
class UDPClient
{
private:
	SOCKET socket_ = INVALID_SOCKET;
	sockaddr_in server_address_;
	queue<std::string> received_server_msgs_queue_;
	bool should_keep_listening_;
	std::mutex mtx_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	//Utility functions
	void InitializeSocket();
	bool ShouldListenThreadSafe();
	bool IsQueueEmptyThreadSafe();
	ushort DrainQueue(std::string& msgInQueue);
	void MakeServerAddress(const std::string& serverIp, const USHORT& serverPort);
	const vector<std::string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	void BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);
	bool HasRequestTimedOut(const high_resolution_clock::time_point& lastMsgRecdTime, const ushort& timeoutDuration);
	vector<u_short> GetMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	
	//Validation functions
	short ValidateServerResponse(std::vector<cv::String>& serverResponseSplit, short& serverResponseCode);
	short ValidateImageMetadataFromServer(std::vector<cv::String>& serverMsgSplit, cv::Size& imageDimensions, uint& imageFileSize);
	short ValidateImageDataPayload(const std::vector<cv::String>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize);

	//Functions to send data to server
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend);
	short SendMissingSeqNumbersToServer(map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads
		, vector<u_short>& missingSeqNumbersInLastTimeout);

	//Functions to receive data from server
	short ReceiveServerMsgs();
	short ConsumeServerMsgFromQueue(std::string& serverResponse);

public:
	UDPClient();
	UDPClient(const string& serverIp, const USHORT& serverPort);
	~UDPClient();
	
	//Utility functions
	bool IsValid();
	void StartListeningForServerMsgs();
	void StopListeningForServerMsgs();

	//Functions to send data to server
	short SendClientResponseToServer(const short& clientResponseCode, const vector<u_short>* missingSeqNumbers);
	short SendImageMetadataToServer(std::string imageMetadataPayload);
	short SendImage(const Mat imageToSend);

	//Functions to receive data from server
	short ConsumeImageDataFromQueue(const cv::Size& imageDimensions, const uint& imageFileSize, ImageProcessor& imageProcessor);
	short ReceiveAndValidateServerResponse(short& serverResponseCode);
	short ReceiveAndValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize);
};

