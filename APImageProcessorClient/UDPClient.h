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
class UDPClient
{
private:
	SOCKET _socket = INVALID_SOCKET;
	sockaddr_in _serverAddress;
	queue<std::string> _receivedServerMsgsQueue;
	bool _shouldKeepListening;
	std::mutex _mtx;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	void initializeSocket();
	void _MakeServerAddress(const std::string& serverIp, const USHORT& serverPort);
	const vector<std::string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	void BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);
	bool _HasRequestTimedOut(const high_resolution_clock::time_point& lastMsgRecdTime, const ushort& timeoutDuration);
	short _SendMissingSeqNumbersToServer(map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads
		, vector<u_short>& missingSeqNumbersInLastTimeout);
	vector<u_short> _CalculateMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	short fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize);
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend);
	short ValidateServerResponse(std::vector<cv::String>& serverResponseSplit, short& serverResponseCode);
	short _ValidateImageMetadataFromServer(std::vector<cv::String>& serverMsgSplit, cv::Size& imageDimensions, uint& imageFileSize);
	short _ValidateImageDataPayload(const std::vector<cv::String>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize);
	bool _ShouldListenThreadSafe();
	bool _IsQueueEmptyThreadSafe();
	short _ConsumeServerMsgFromQueue(std::string& serverResponse);
	ushort _DrainQueue(std::string& msgInQueue);

public:
	UDPClient();
	UDPClient(const string& serverIp, const USHORT& serverPort);
	~UDPClient();
	
	bool isValid();
	short SendClientResponseToServer(const short& clientResponseCode, const vector<u_short>* missingSeqNumbers);
	short SendImageMetadata(std::string imageMetadataPayload);
	short sendImageSize(cv::String imageAddress);
	short SendImage(const Mat imageToSend); //TODO use abstract class here : Client -> UDPClient -> ImageSendingClient
	short ReceiveAndValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize);
	short ConsumeImageDataFromQueue(const cv::Size& imageDimensions, const uint& imageFileSize, ImageProcessor& imageProcessor);
	short ReceiveServerMsgs();
	short ReceiveAndValidateServerResponse(short& serverResponseCode);
	void StartListeningForServerMsgs();
	void StopListeningForServerMsgs();
};

