#include "UDPClient.h"

#include<iostream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;
using namespace cv;

UDPClient::UDPClient()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1) {
		cout << "\nWSAStartup failed.";
		_thisSocket = INVALID_SOCKET;
		return;
	}
	_thisSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (_thisSocket == INVALID_SOCKET) {
		cout << "\nError while creating client socket. Error code: "<<WSAGetLastError();
	}
	else {
		cout << "\nClient socket created successfully.";
	}
}

UDPClient::~UDPClient()
{
	WSACleanup();
	closesocket(_thisSocket);
}

int UDPClient::sendImage(cv::String imageAddress, std::string serverIp, long serverPort)
{
	if (_thisSocket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid client socket.";
		return 0;
	}

	Mat imageToSend = imread(imageAddress, IMREAD_COLOR);
	if (imageToSend.empty()) {
		cout << "\nERROR: Cannot send empty image.";
	}
	
	sockaddr_in serverAddress;
	inet_pton(AF_INET, serverIp.c_str(), &(serverAddress.sin_addr));
	//serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = serverPort;
	serverAddress.sin_family = AF_INET;
	
	cv::String windowName = "clientImageBeforeSending";
	namedWindow(windowName, WINDOW_NORMAL);
	imshow(windowName, imageToSend);

	waitKey(0);
	destroyWindow(windowName);

	//cout << "\n\nImage data before resizing: " << imageToSend;
	//Below snippet taken from https://stackoverflow.com/a/20321262
	//Challenge: To send image to server in array form
	imageToSend = imageToSend.reshape(0, 1);
	//cout << "\n\nImage data after resizing: " << imageToSend;

	long imageSize = imageToSend.total() * imageToSend.elemSize();
	cout << "\nImage size before sending: " << imageSize;
	auto imagePtr = imageToSend.data;
	long bytesSent = 0, bytesLeftToSend = imageSize;
	while (bytesSent < imageSize) {
		long bytesSentThisIteration;

		cout << "\nBytes left to send: " << bytesLeftToSend;

		if (bytesLeftToSend >= 60000l) {
			bytesSentThisIteration = sendto(_thisSocket, (char*)imagePtr + bytesSent, 60000l, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
		}
		else {
			bytesSentThisIteration = sendto(_thisSocket, (char*)imagePtr + bytesSent, bytesLeftToSend, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
		}
		
		if (bytesSentThisIteration <= 0) {
			cout << "\nError while sending image. Error code: " << WSAGetLastError();
			return 0;
		}
		bytesSent += bytesSentThisIteration;
		bytesLeftToSend -= bytesSentThisIteration;
		//imagePtr += bytesSentThisIteration;
		cout << "\nBytes sent this iteration: " << bytesSentThisIteration;
	}
	
	cout << "\nImage sent to server.";

	

	return bytesSent;
}
