#include "Constants.h"
#include "UDPClient.h"

#include<iostream>

using std::string;
using std::cout;
using std::thread;
using std::vector;
using std::to_string;
using std::stoi;
using std::invalid_argument;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;

using cv::Size;
using cv::Mat;
using cv::imread;
using cv::IMREAD_COLOR;
using cv::namedWindow;
using cv::WINDOW_NORMAL;

#pragma comment (lib, "ws2_32.lib")

UDPClient::UDPClient()
{
	initializeSocket();
}

UDPClient::~UDPClient()
{
	WSACleanup();
	closesocket(_socket);
}

const sockaddr_in UDPClient::makeServerAddress(const std::string& serverIp, const long& serverPort) {
	sockaddr_in serverAddress;
	inet_pton(AF_INET, serverIp.c_str(), &(serverAddress.sin_addr));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = serverPort;

	return serverAddress;
}

const vector<std::string> UDPClient::SplitString(char* inputString, char delimiter) {
	std::string currentWord = "";
	vector<std::string> outputVector;
	char* charPtr = inputString;
	cout << "\nStarting to split the server response.";
	//cout << "\nString array at beginning: " << *stringArray;


	while (*(charPtr) != '\0') {
		char currentChar = *(charPtr);
		//cout << "\nCurrent word: " << currentWord<<" | current char = "<<currentChar;
		if (currentChar == delimiter) {
			outputVector.push_back(currentWord);
			currentWord = "";
		}
		else {
			currentWord += currentChar;
		}
		++charPtr;
	}
	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	cout << "\nString after splitting: ";
	auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		cout << *iter << "|";
		iter++;
	}
	cout << "\nReturning output vector. ";
	return outputVector;
}

const vector<string> UDPClient::SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength) {
	std::string currentWord = "";
	int numberOfCurrentSplits = 1;
	vector<string> outputVector;
	//char* charPtr = inputString;
	//cout << "\nString array at beginning: " << *stringArray;

	for (int i = 0; i < inputStringLength; i++) {
		char currentChar = *(inputString + i);
		//cout << "\nCurrent word: " << currentWord<<" | current char = "<<currentChar;
		if (currentChar == delimiter && numberOfCurrentSplits < numberOfSplits) {
			outputVector.push_back(currentWord);
			currentWord = "";
			numberOfCurrentSplits++;
		}
		else {
			currentWord += currentChar;
		}
		//++charPtr;
	}

	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	//cout << "\nString after splitting: ";
	/*auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		cout << *iter << "|";
		iter++;
	}*/
	//cout << "\nReturning output vector. ";
	if (outputVector.size() < numberOfSplits) {
		cout << "\nUnexpected split. Original string: " << inputString;
	}
	return outputVector;
}

void UDPClient::buildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
	map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers) 
{
	u_int imageBytesLeft = image.elemSize() * image.total();
	u_int imageBytesProcessed = 0;
	string payload;
	u_short payloadSize;
	u_short payloadSequenceNum = 1;
	auto imagePtr = image.data;

	while (imageBytesLeft > 0) {
		
		u_short imageBytesProcessedThisIteration = 0;
		if (imageBytesLeft >= 60000) {
			
			payload = string(SEQUENCE_PAYLOAD_KEY).append(" ").append(to_string(payloadSequenceNum)).append(" ")
				.append(SIZE_PAYLOAD_KEY).append(" ").append(to_string(60000)).append(" ");
			
			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, 60000);
			payload += imageData;

			imageBytesProcessedThisIteration = 60000;

			//bytesSentThisIteration = sendto(_socket, &payload[0], 60000 + payloadLength, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));

		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(" ").append(to_string(payloadSequenceNum)).append(" ")
				.append(SIZE_PAYLOAD_KEY).append(" ").append(to_string(imageBytesLeft)).append(" ");
			
			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, imageBytesLeft);
			payload += imageData;

			imageBytesProcessedThisIteration = imageBytesLeft;

			//bytesSentThisIteration = sendto(_socket, &payload[0], bytesLeftToSend + payloadLength, 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
		}


		imageDataPayloadMap[payloadSequenceNum] = payload;

		payloadSize += imageBytesProcessedThisIteration;
		sequenceNumToPayloadSizeMap[payloadSequenceNum] = payloadSize;

		sequenceNumbers.push_back(payloadSequenceNum);
		imageBytesProcessed += imageBytesProcessedThisIteration;
		imageBytesLeft -= imageBytesProcessedThisIteration;
		
		payloadSequenceNum++;

		cout << "\nBytes processd: " << imageBytesProcessed;		
	}
}

bool UDPClient::isValid()
{
	return _socket != INVALID_SOCKET;
}

short UDPClient::SendImageMetadata(std::string imageMetadataPayload, std::string serverIp, long serverPort)
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid client socket.";
		return RESPONSE_FAILURE;
	}

	ushort payloadSize = imageMetadataPayload.length();
	cout << "\nImage metadata payload before sending: " << imageMetadataPayload << " | Size: " << payloadSize;

	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);

	int bytesSent = 0;

	while (bytesSent < payloadSize) {
		int bytesSentThisIteration = sendto(_socket, &imageMetadataPayload[0] + bytesSent, imageMetadataPayload.length()+1-bytesSent, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));
		if (bytesSentThisIteration <= 0) {
			cout << "\nError while sending image size. Error code: " << WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}

	cout << "\nImage metadata successfully sent to server.";
	return RESPONSE_SUCCESS;
}

short UDPClient::sendClientResponse(const short& clientResponseCode, std::string serverIp, long serverPort, const vector<u_short>* missingSeqNumbers)
{
	cout << "\nEntered sendClientResponse.";
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		return RESPONSE_FAILURE;
	}

	string missingSeqNumbersString = "";
	if (missingSeqNumbers != nullptr) {
		for (const u_short& missingSeqNumber : *missingSeqNumbers) {
			missingSeqNumbersString.append(to_string(missingSeqNumber)).append(" ");
		}
	}
	string clientResponsePayload = string(RESPONSE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
		.append(to_string(clientResponseCode)).append(CLIENT_MSG_DELIMITER)
		.append(missingSeqNumbersString).append("\0");
	cout << "\nClient response string: " << clientResponsePayload << " | string length: " << clientResponsePayload.length();

	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	short bytesSent = sendto(_socket, &clientResponsePayload[0], clientResponsePayload.length() + 1, 0, 
		(const sockaddr*)&serverAddress, sizeof(serverAddress));

	if (bytesSent <= 0) {
		cout << "\nError while sending acknowldgement to server. Error code: " << WSAGetLastError();
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
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

	std::string payload = SIZE_PAYLOAD_KEY;
	payload.append(" ").append(to_string(imageToSend.cols)).append(" ").append(to_string(imageToSend.rows)).append("\0");
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

short UDPClient::sendImage(const Mat imageToSend, const std::string& serverIp, const long& serverPort)
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid client socket.";
		return RESPONSE_FAILURE;
	}
	
	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);
	
	cv::String windowName = "Client Image Before Sending";
	namedWindow(windowName, WINDOW_NORMAL);
	
	/*imshow(windowName, imageToSend);
	waitKey(0);
	destroyWindow(windowName);*/

	//long imageSize = imageToSend.total() * imageToSend.elemSize();

	map<u_short, string> imageDataPayloadMap;
	map<u_short, u_short> sequenceNumToPayloadSizeMap;
	vector<u_short> payloadSeqNumbersToSend;
	short serverResponseCode = SERVER_NEGATIVE_ACK;

	buildImageDataPayloadMap(imageToSend, imageDataPayloadMap, sequenceNumToPayloadSizeMap, payloadSeqNumbersToSend);
	
	while (serverResponseCode != SERVER_POSITIVE_ACK) {

		short responseCode = sendImageDataPayloadsBySequenceNumbers(imageDataPayloadMap, sequenceNumToPayloadSizeMap, 
			payloadSeqNumbersToSend, serverAddress);
		if (responseCode == RESPONSE_FAILURE) {
			cout << "\nCould not send image payloads to server.";
			return RESPONSE_FAILURE;
		}

		vector<string> serverResponseSplit;
		responseCode = receiveServerMsg(serverIp, serverPort, serverResponseSplit);
		if (responseCode == RESPONSE_FAILURE) {
			cout << "\nCould not receive response from server.";
			return RESPONSE_FAILURE;
		}

		responseCode = validateServerResponse(serverResponseSplit, serverResponseCode);
		if (responseCode == RESPONSE_FAILURE) {
			cout << "\nERROR: Validation failed for server response.";
			return RESPONSE_FAILURE;
		}

		if (serverResponseCode == SERVER_NEGATIVE_ACK) {
			payloadSeqNumbersToSend.clear();
			for (int i = 2; i < serverResponseSplit.size(); i++) {
				try {
					payloadSeqNumbersToSend.push_back(stoi(serverResponseSplit.at(i)));
				}
				catch (invalid_argument) {
					cout << "\nERROR: Sequence number sent by server not a number. Received sequence number: " << serverResponseSplit.at(i);
				}
			}
		}
	}

	cout << "\nAll image payloads received by server.";
	//long bytesSent = fragmentAndSendImageData(imageToSend, imageSize, serverAddress);

	return RESPONSE_SUCCESS;
}

short UDPClient::ReceiveAndValidateImageMetadata(std::string serverIp, long serverPort, cv::Size& imageDimensions)
{
	vector <std::string> serverMsgSplit;
	short responseCode = receiveServerMsg(serverIp, serverPort, serverMsgSplit);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nERROR: Could not receive server message.";
		return RESPONSE_FAILURE;
	}

	return _ValidateImageDimensionsFromServer(serverMsgSplit, imageDimensions);
}

short UDPClient::receiveImage(const cv::Size& imageDimensions, const std::string& serverIp, const long& serverPort)
{
	long imageBytesRecd = 0, imageBytesLeftToReceive = imageDimensions.width * imageDimensions.height * 3;

	u_short expectedNumberOfPayloads = imageBytesLeftToReceive / 60000;
	if (imageBytesLeftToReceive % 60000 > 0) {
		expectedNumberOfPayloads++;
	}
	
	string imageDataString = "";
	char* imageDataFromServer = new char[MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES];
	map<u_short, string> imagePayloadSeqMap;

	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);
	int serverAddressSize = (sizeof(serverAddress));

	auto lastImagePayloadRecdTime = high_resolution_clock::now();
	while (imageBytesLeftToReceive > 0) {

		int bytesRecdThisIteration = recvfrom(_socket, imageDataFromServer, MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES, 0, 
			(sockaddr*)&serverAddress, &serverAddressSize);
		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError while receving data from server. Error code: " << WSAGetLastError();
				return RESPONSE_FAILURE;
			}
			
			else {
				//check for timeout
			}
		}
		else {
			//add to payload map
			//serverMsg = string(serverResponse, bytesRecd);
			break;
		}

		if (clientQueue.empty()) {
			responseCode = CheckForTimeout(lastImagePayloadRecdTime, imagePayloadSeqMap, expectedNumberOfPayloads, clientAddress);
			if (responseCode == RESPONSE_FAILURE) {
				cout << "\nError when sending response to client on timeout.";
				//TODO client cleanup
				return;
			}
			continue;
		}

		/*long lastPayloadSize = 0;
		try {
			lastPayloadSize = stol(clientQueue.front());
			clientQueue.pop();
		}
		catch (invalid_argument) {
			cout << "\nERROR::Invalid payload size value found in queue.";
		}

		cout << "\nLast payload size for image data: " << lastPayloadSize;*/

		if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image data.";
		}

		if (clientQueue.front().length() < 17) {
			cout << "\nUnexpected msg found in queue. Msg: " << clientQueue.front();
			clientQueue.pop();
		}
		cout << "\nQueue msg size before splitting: " << clientQueue.front().length();
		vector<string> splitImageDataPayload = SplitString(&(clientQueue.front()[0]), ' ', 5, clientQueue.front().length());
		cout << "\nSplit image payload size: " << splitImageDataPayload.size();

		//TODO shift hardcoded values to constants
		if (splitImageDataPayload.size() != 5 || splitImageDataPayload.at(0) != SEQUENCE_PAYLOAD_KEY || splitImageDataPayload.at(2) != SIZE_PAYLOAD_KEY) {
			cout << "\nERROR: Image data payload in incorrect format. First word: " << splitImageDataPayload.at(0);
			return;
		}

		u_int payloadSeqNum = 0, payloadSize = 0;
		try {
			payloadSeqNum = stoi(splitImageDataPayload.at(1));
			payloadSize = stoi(splitImageDataPayload.at(3));
		}
		catch (invalid_argument) {
			cout << "\nERROR: Image data payload sequence num or size not an int. Seq num: " << splitImageDataPayload.at(1)
				<< " | Size:" << splitImageDataPayload.at(3);
			return;
		}


		imagePayloadSeqMap[payloadSeqNum] = splitImageDataPayload.at(4);
		cout << "\nImage data after splitting: " << splitImageDataPayload.at(0) << " | " << splitImageDataPayload.at(1) << " | "
			<< splitImageDataPayload.at(2) << " | " << splitImageDataPayload.at(3) << " | Length of image data: " << splitImageDataPayload.at(4).length();
		//imageDataString += splitImageDataPayload.at(4);
		clientQueue.pop();

		cout << "\nAfter popping image data payload from queue. Queue size: " << clientQueue.size();

		imageBytesRecd += payloadSize;
		imageBytesLeftToReceive -= payloadSize;

		cout << "\nImage bytes recd: " << imageBytesRecd << " | image bytes left to receive: " << imageBytesLeftToReceive;
		lastImagePayloadRecdTime = chrono::high_resolution_clock::now();
	}

	//Check for data integrity - hash? - can match hash only after re-creating the image
	//Removed data integrity check as above loop is exited only when all bytes are received.

	cout << "\nAll image data received. Sending positive ACK to client.";
	responseCode = sendServerResponse(SERVER_POSITIVE_ACK, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send ACK to client.";
		return;
	}
}

short UDPClient::validateServerResponse(std::vector<std::string>& serverResponseSplit, short& serverResponseCode)
{
	cout << "\nValidating server response.";

	if (serverResponseSplit.size() < 2 || serverResponseSplit.at(0) != RESPONSE_PAYLOAD_KEY) {
		cout << "\nERROR: Server response not in expected format. Split size: " << serverResponseSplit.size();
		cout<<"\nFirst element: " << serverResponseSplit.at(0);
		return RESPONSE_FAILURE;
	}

	try {
		serverResponseCode = stoi(serverResponseSplit.at(1));
	}
	catch (invalid_argument) {
		cout << "\nERROR: Response code not a number. Recd response code: " << serverResponseSplit.at(1);
		return RESPONSE_FAILURE;
	}
	
	cout << "\nServer response validation successful.";
	return RESPONSE_SUCCESS;
}

short UDPClient::_ValidateImageDimensionsFromServer(std::vector<cv::String>& serverMsgSplit, cv::Size& imageDimensions)
{
	if (serverMsgSplit.at(0) != SIZE_PAYLOAD_KEY || serverMsgSplit.size() < 3) {
		cout << "\nServer sent image meta data in wrong format.";
		return RESPONSE_FAILURE;
	}
	try {
		imageDimensions = cv::Size(stoi(serverMsgSplit.at(1)), stoi(serverMsgSplit.at(2)));
	}
	catch (invalid_argument iaExp) {
		cout << "\nInvalid image size values received.";
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

short UDPClient::sendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap, 
	const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress)
{
	for (u_short payloadSeqNumberToSend : payloadSeqNumbersToSend) {
		char* payloadToSend = &(imageDataPayloadMap[payloadSeqNumberToSend][0]);

		int sendResult = sendto(_socket, payloadToSend, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend], 0, (const sockaddr*) &serverAddress, sizeof(serverAddress));
		if (sendResult == SOCKET_ERROR) {
			cout << "\nERROR while sending image payload to server. Error code "<<WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		cout << "\nSent payload #" << payloadSeqNumberToSend;
	}
	
	cout << "\nImage payloads sent to server.";
	return RESPONSE_SUCCESS;
}



short UDPClient::receiveServerMsg(std::string serverIp, long serverPort, string& serverMsg)
{
	//TODO make this a member variable
	const sockaddr_in serverAddress = makeServerAddress(serverIp, serverPort);
	int serverAddressSize = sizeof(serverAddress);

	char* serverResponse = new char[MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES];
	int bytesRecd = 0;

	while (true) {

		bytesRecd = recvfrom(_socket, serverResponse, MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES, 0, (sockaddr*)&serverAddress, &serverAddressSize);
		if (bytesRecd == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError while receving data from server. Error code: " << WSAGetLastError();
				return RESPONSE_FAILURE;
			}
			//sleep_for(milliseconds(50));
		}
		else {
			serverMsg = string(serverResponse, bytesRecd);
			break;
		}
	}

	return RESPONSE_SUCCESS;
}

short UDPClient::receiveAndValidateServerResponse(std::string serverIp, long serverPort, short& serverResponseCode)
{
	string serverResponse;
	short responseCode = receiveServerMsg(serverIp, serverPort, serverResponse);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nERROR: Could not receive server response.";
		return RESPONSE_FAILURE;
	}
	
	vector<string> serverResponseSplit = SplitString(&serverResponse[0], SERVER_RESPONSE_DELIMITER);
	return validateServerResponse(serverResponseSplit, serverResponseCode);
}

short UDPClient::receiveAndValidateImageDimensions(std::string serverIp, long serverPort, cv::Size& imageDimensions)
{
	string serverMsg;
	short responseCode = receiveServerMsg(serverIp, serverPort, serverMsg);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nERROR: Could not receive server response.";
		return RESPONSE_FAILURE;
	}

	vector<string> serverResponseSplit = SplitString(&serverMsg[0], SERVER_RESPONSE_DELIMITER);
	return _ValidateImageDimensionsFromServer(serverResponseSplit, imageDimensions);
}

short UDPClient::fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize, const sockaddr_in& serverAddress)
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

		sleep_for(milliseconds(50));
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

		sockaddr_in thisSocket;
		int thisSocketSize = sizeof(thisSocket);
		getsockname(_socket, (sockaddr*) & thisSocket, &thisSocketSize);

		cout << "\nSocket IP: " << thisSocket.sin_addr.s_addr << " | Socket port: " << thisSocket.sin_port;

		u_long nonBlockingModeTrue = 1;
		if (ioctlsocket(_socket, FIONBIO, &nonBlockingModeTrue) == SOCKET_ERROR) {
			cout << "\nError while setting socket to non-blocking mode.";
			_socket = INVALID_SOCKET;
		}
	}
}