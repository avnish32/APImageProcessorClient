#include "Constants.h"
#include "UDPClient.h"

#include<iostream>

using namespace std;

#pragma comment (lib, "ws2_32.lib")

using namespace std;
using namespace cv;

UDPClient::UDPClient()
{
	initializeSocket();
}

UDPClient::~UDPClient()
{
	WSACleanup();
	closesocket(_socket);
}

//---------------------- Utility functions

const sockaddr_in makeServerAddress(const std::string& serverIp, const long& serverPort) {
	sockaddr_in serverAddress;
	inet_pton(AF_INET, serverIp.c_str(), &(serverAddress.sin_addr));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = serverPort;

	return serverAddress;
}

//----------------Member functions

bool UDPClient::isValid()
{
	return _socket != INVALID_SOCKET;
}

short UDPClient::sendImageSize(cv::String imageAddress, std::string serverIp, long serverPort)
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid client socket.";
		return RESPONSE_FAILURE;
	}

	Mat imageToSend = imread(imageAddress, IMREAD_COLOR);
	if (imageToSend.empty()) {
		cout << "\nERROR: Cannot send size of empty image.";
		return RESPONSE_FAILURE;
	}

	std::string payload = SIZE_PAYLOAD_PREFIX;
	payload.append(to_string(imageToSend.cols)).append(" ").append(to_string(imageToSend.rows)).append("\0");
	short payloadSize = strlen(payload.c_str());
	cout << "\nImage size payload before sending: " << payload<<" | Size: "<<payloadSize;

	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);

	int bytesSent = 0, retryCount = 0;
	char payloadPtr[15];
	cout << "\nBefore strcpy.";
	strcpy_s(payloadPtr, payload.c_str());
	cout << "\nAfter strcpy.";

	while (bytesSent < payloadSize) {
		int bytesSentThisIteration = sendto(_socket, payloadPtr+bytesSent, sizeof(payloadPtr), 0, (sockaddr*)&serverAddress, sizeof(serverAddress));
		if (bytesSentThisIteration <= 0) {
			cout << "\nError while sending image size. Error code: " << WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}
	
	
	cout << "\nImage size successfully sent to server.";
	return RESPONSE_SUCCESS;
}

short UDPClient::sendImage(cv::String imageAddress, std::string serverIp, long serverPort)
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid client socket.";
		return RESPONSE_FAILURE;
	}

	Mat imageToSend = imread(imageAddress, IMREAD_COLOR);
	if (imageToSend.empty()) {
		cout << "\nERROR: Cannot send empty image.";
		return RESPONSE_FAILURE;
	}
	
	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);
	
	cv::String windowName = "Client Image Before Sending";
	namedWindow(windowName, WINDOW_NORMAL);
	
	/*imshow(windowName, imageToSend);
	waitKey(0);
	destroyWindow(windowName);*/

	long imageSize = imageToSend.total() * imageToSend.elemSize();
	long bytesSent = fragmentAndSendImageData(imageToSend, imageSize, serverAddress);

	return RESPONSE_SUCCESS;
}

short UDPClient::receiveMsgFromServer(std::string serverIp, long serverPort)
{
	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);
	int serverAddressSize = sizeof(serverAddress);
	short serverResponseCode = SERVER_NEGATIVE_ACK;
	int bytesRecd = 0;
	while (bytesRecd < sizeof(serverResponseCode)) {
		int bytesRecdThisIteration = recvfrom(_socket, (char*)&serverResponseCode, sizeof(serverResponseCode), 0, (sockaddr*)&serverAddress, &serverAddressSize);
		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError while receving data from server. Error code: " << WSAGetLastError();
				return RESPONSE_FAILURE;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
		else {
			bytesRecd += bytesRecdThisIteration;
		}
	}
	
	cout << "\nResponse from server: " << serverResponseCode;
	
	return serverResponseCode;
}

long UDPClient::fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize, const sockaddr_in& serverAddress)
{
	//cout << "\n\nImage data before resizing: " << imageToSend;
	//Below snippet taken from https://stackoverflow.com/a/20321262
	//Challenge: To send image to server in array form
	imageToSend = imageToSend.reshape(0, 1);
	//cout << "\n\nImage data after resizing: " << imageToSend;

	
	cout << "\nImage size before sending: " << imageSize;
	auto imagePtr = imageToSend.data;
	long bytesSent = 0, bytesLeftToSend = imageSize;
	string payload;
	u_short payloadLength;
	u_int payloadSequenceNum = 1;

	while (bytesSent < imageSize) {
		long bytesSentThisIteration;

		cout << "\nBytes left to send: " << bytesLeftToSend;

		if (bytesLeftToSend >= 60000l) {
			payload = "Seq " + to_string(payloadSequenceNum) + " Size " + to_string(60000) + " ";
			payloadLength = payload.length();
			string imageData = string((char*)imagePtr + bytesSent, 60000);
			
			//cout << "\nPayload strlen: " << strlen(payload.c_str()) << " | imageData strlen: " << strlen(imageData.c_str()) << " strlen imageptr: "<<strlen((const char*)imagePtr);
			payload += imageData;
			
			bytesSentThisIteration = sendto(_socket, &payload[0], 60000 + payloadLength, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
		}
		else {
			payload = "Seq " + to_string(payloadSequenceNum) + " Size " + to_string(bytesLeftToSend) + " ";
			payloadLength = payload.length();
			string imageData = string((char*)imagePtr + bytesSent, bytesLeftToSend);
			payload += imageData;

			bytesSentThisIteration = sendto(_socket, &payload[0], bytesLeftToSend + payloadLength, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
		}

		if (bytesSentThisIteration <= 0) {
			cout << "\nError while sending image. Error code: " << WSAGetLastError();
			return bytesSent;
		}
		bytesSent = bytesSent + (bytesSentThisIteration - payloadLength);
		bytesLeftToSend = bytesLeftToSend - (bytesSentThisIteration - payloadLength);
		//imagePtr += bytesSentThisIteration;
		cout << "\nBytes sent this iteration: " << bytesSentThisIteration;
		payloadSequenceNum++;

		this_thread::sleep_for(chrono::milliseconds(50));
	}

	cout << "\nImage sent to server.";
	return bytesSent;
}

void UDPClient::initializeSocket() {
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1) {
		cout << "\nWSAStartup failed.";
		_socket = INVALID_SOCKET;
		return;
	}
	_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (_socket == INVALID_SOCKET) {
		cout << "\nError while creating client socket. Error code: " << WSAGetLastError();
		return;
	}
	else {
		cout << "\nClient socket created successfully.";
		u_long nonBlockingModeTrue = 1;
		if (ioctlsocket(_socket, FIONBIO, &nonBlockingModeTrue) == SOCKET_ERROR) {
			cout << "\nError while setting socket to non-blocking mode.";
			_socket = INVALID_SOCKET;
		}
	}
}