#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string>

#include<opencv2/opencv.hpp>

#pragma once
class UDPClient
{
	private:
		SOCKET _thisSocket;
public:
	UDPClient();
	~UDPClient();
	int sendImage(cv::String imageAddress, std::string serverIp, long serverPort); //TODO use abstract class here : Client -> UDPClient -> ImageSendingClient
};

