#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string>
#include<map>

#include<opencv2/opencv.hpp>

using std::string;
using std::map;
using std::vector;

using cv::Mat;

#pragma once
class UDPClient
{
private:
	SOCKET _socket = INVALID_SOCKET;

	void initializeSocket();
	const sockaddr_in makeServerAddress(const std::string& serverIp, const long& serverPort);
	const vector<std::string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	void buildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);
	short fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize, const sockaddr_in& serverAddress);
	short sendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);
	short validateServerResponse(std::vector<cv::String>& serverResponseSplit, short& serverResponseCode);
	short _ValidateImageDimensionsFromServer(std::vector<cv::String>& serverMsgSplit, cv::Size& imageDimensions);

public:
	UDPClient();
	~UDPClient();
	
	bool isValid();
	short sendClientResponse(const short& clientResponseCode, std::string serverIp, long serverPort, const vector<u_short>* missingSeqNumbers);
	short SendImageMetadata(std::string imageMetadataPayload, std::string serverIp, long serverPort);
	short sendImageSize(cv::String imageAddress, std::string serverIp, long serverPort);
	short sendImage(const Mat imageToSend, const std::string& serverIp, const long& serverPort); //TODO use abstract class here : Client -> UDPClient -> ImageSendingClient
	short ReceiveAndValidateImageMetadata(std::string serverIp, long serverPort, cv::Size& imageDimensions);
	short receiveImage(const cv::Size& imageDimensions);
	short receiveServerMsg(std::string serverIp, long serverPort, string& serverMsg);
	short receiveAndValidateServerResponse(std::string serverIp, long serverPort, short& serverResponseCode);
	short receiveAndValidateImageDimensions(std::string serverIp, long serverPort, cv::Size& imageDimensions);
};

