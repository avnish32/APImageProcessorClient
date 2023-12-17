#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string>

#include<opencv2/opencv.hpp>

#pragma once
class UDPClient
{
private:
	SOCKET _socket = INVALID_SOCKET;

	void initializeSocket();
	long fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize, const sockaddr_in& serverAddress);

public:
	UDPClient();
	~UDPClient();
	
	bool isValid();
	short sendImageSize(cv::String imageAddress, std::string serverIp, long serverPort);
	short sendImage(cv::String imageAddress, std::string serverIp, long serverPort); //TODO use abstract class here : Client -> UDPClient -> ImageSendingClient
	short receiveMsgFromServer(std::string serverIp, long serverPort);
};

