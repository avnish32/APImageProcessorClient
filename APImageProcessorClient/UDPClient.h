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
	short fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize, const sockaddr_in& serverAddress);
	short sendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);
	short validateServerResponse(std::vector<cv::String>& serverResponseSplit, short& serverResponseCode);

public:
	UDPClient();
	~UDPClient();
	
	bool isValid();
	short SendImageMetadata(std::string imageMetadataPayload, std::string serverIp, long serverPort);
	short sendImageSize(cv::String imageAddress, std::string serverIp, long serverPort);
	short sendImage(const Mat imageToSend, const std::string& serverIp, const long& serverPort); //TODO use abstract class here : Client -> UDPClient -> ImageSendingClient
	short receiveServerResponse(std::string serverIp, long serverPort, vector<string>& splitServerResponse);
	short receiveAndValidateServerResponse(std::string serverIp, long serverPort, short& serverResponseCode);
};

