#include "Constants.h"
#include "UDPClient.h"
#include "ImageProcessor.h"

#include<iostream>

using std::string;
using std::cout;
using std::thread;
using std::vector;
using std::to_string;
using std::stoi;
using std::invalid_argument;
using std::stringstream;
using std::this_thread::get_id;
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
	_MakeServerAddress("127.0.0.1", 8080);
	_shouldKeepListening = true;
}

UDPClient::UDPClient(const string& serverIp, const USHORT& serverPort)
{
	initializeSocket();
	_MakeServerAddress(serverIp, serverPort);
	_shouldKeepListening = true;
}

UDPClient::~UDPClient()
{
	_shouldKeepListening = false;
	WSACleanup();
	closesocket(_socket);
}

void UDPClient::_MakeServerAddress(const std::string& serverIp, const USHORT& serverPort) {
	inet_pton(AF_INET, serverIp.c_str(), &(_serverAddress.sin_addr));
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = serverPort;
}

const vector<std::string> UDPClient::SplitString(char* inputString, char delimiter) {
	std::string currentWord = "";
	vector<std::string> outputVector;
	char* charPtr = inputString;

	//cout << "\nStarting to split the server response.";
	_msgLogger->LogDebug("Starting to split the server response.");


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

	string stringAfterSplittingLogMsg = "String after splitting: ";
	//cout << "\nString after splitting: ";
	auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		//cout << *iter << "|";
		stringAfterSplittingLogMsg.append(*iter).append(" | ");
		iter++;
	}
	_msgLogger->LogDebug(stringAfterSplittingLogMsg);

	//cout << "\nReturning output vector. ";
	_msgLogger->LogDebug("Returning output vector.");

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
		//cout << "\nUnexpected split. Original string: " << inputString;
		stringstream sStream;
		sStream << inputString;
		_msgLogger->LogError("Unexpected split. Original string: " + sStream.str());
	}
	return outputVector;
}

void UDPClient::BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
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
		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(" ").append(to_string(payloadSequenceNum)).append(" ")
				.append(SIZE_PAYLOAD_KEY).append(" ").append(to_string(imageBytesLeft)).append(" ");
			
			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, imageBytesLeft);
			payload += imageData;

			imageBytesProcessedThisIteration = imageBytesLeft;
		}

		imageDataPayloadMap[payloadSequenceNum] = payload;

		payloadSize += imageBytesProcessedThisIteration;
		sequenceNumToPayloadSizeMap[payloadSequenceNum] = payloadSize;

		sequenceNumbers.push_back(payloadSequenceNum);
		imageBytesProcessed += imageBytesProcessedThisIteration;
		imageBytesLeft -= imageBytesProcessedThisIteration;
		
		payloadSequenceNum++;

		//cout << "\nBytes processd: " << imageBytesProcessed;		
		_msgLogger->LogDebug("Bytes processd: " + to_string(imageBytesProcessed));
	}
}

bool UDPClient::isValid()
{
	return _socket != INVALID_SOCKET;
}

short UDPClient::SendImageMetadata(std::string imageMetadataPayload)
{
	_msgLogger->LogError("Sending image metadata to server.");

	if (_socket == INVALID_SOCKET) {
		//cout << "\nERROR: Invalid client socket.";
		_msgLogger->LogError("ERROR: Invalid client socket.");

		return RESPONSE_FAILURE;
	}

	ushort payloadSize = imageMetadataPayload.length() + 1;
	
	//cout << "\nImage metadata payload before sending: " << imageMetadataPayload << " | Size: " << payloadSize;
	_msgLogger->LogError("Image metadata payload before sending: " + imageMetadataPayload + " | Size: " + to_string(payloadSize));

	int bytesSent = 0;

	while (bytesSent < payloadSize) {
		int bytesSentThisIteration = sendto(_socket, &imageMetadataPayload[0] + bytesSent, payloadSize - bytesSent, 
			0, (sockaddr*)&_serverAddress, sizeof(_serverAddress));
		if (bytesSentThisIteration <= 0) {
			//cout << "\nError while sending image size. Error code: " << WSAGetLastError();
			_msgLogger->LogError("Error while sending image size. Error code: " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}

	//cout << "\nImage metadata successfully sent to server.";
	_msgLogger->LogError("Image metadata successfully sent to server.");
	return RESPONSE_SUCCESS;
}

short UDPClient::SendClientResponseToServer(const short& clientResponseCode, const vector<u_short>* missingSeqNumbers)
{
	//cout << "\nEntered sendClientResponse.";
	_msgLogger->LogDebug("Sending client response...");

	if (_socket == INVALID_SOCKET) {
		//cout << "\nERROR: Invalid socket.";
		_msgLogger->LogError("ERROR: Invalid socket.");
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

	//cout << "\nClient response string: " << clientResponsePayload << " | string length: " << clientResponsePayload.length();
	_msgLogger->LogDebug("Client response string: " + clientResponsePayload 
		+ " | String length: " + to_string((ushort)clientResponsePayload.length()));

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	short bytesSent = sendto(_socket, &clientResponsePayload[0], clientResponsePayload.length() + 1, 0, 
		(const sockaddr*)&_serverAddress, sizeof(_serverAddress));

	if (bytesSent <= 0) {
		//cout << "\nError while sending acknowldgement to server. Error code: " << WSAGetLastError();
		_msgLogger->LogError("Error while sending response to server. Error code: " + to_string(WSAGetLastError()));
		return RESPONSE_FAILURE;
	}

	_msgLogger->LogError("Client response sent successfully.");
	return RESPONSE_SUCCESS;
}

short UDPClient::sendImageSize(cv::String imageAddress)
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

	int bytesSent = 0, retryCount = 0;
	char payloadPtr[15];
	cout << "\nBefore strcpy.";
	strcpy_s(payloadPtr, payload.c_str());
	cout << "\nAfter strcpy.";

	while (bytesSent < payloadSize) {
		int bytesSentThisIteration = sendto(_socket, payloadPtr+bytesSent, sizeof(payloadPtr), 0, (sockaddr*)&_serverAddress, sizeof(_serverAddress));
		if (bytesSentThisIteration <= 0) {
			cout << "\nError while sending image size. Error code: " << WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}
	
	
	cout << "\nImage size successfully sent to server.";
	return RESPONSE_SUCCESS;
}

short UDPClient::SendImage(const Mat imageToSend)
{
	_msgLogger->LogError("Sending image to server.");

	if (_socket == INVALID_SOCKET) {
		//cout << "\nERROR: Invalid client socket.";
		_msgLogger->LogError("ERROR: Invalid client socket.");

		return RESPONSE_FAILURE;
	}

	map<u_short, string> imageDataPayloadMap;
	map<u_short, u_short> sequenceNumToPayloadSizeMap;
	vector<u_short> payloadSeqNumbersToSend;
	short serverResponseCode = SERVER_NEGATIVE_ACK;

	BuildImageDataPayloadMap(imageToSend, imageDataPayloadMap, sequenceNumToPayloadSizeMap, payloadSeqNumbersToSend);
	
	while (serverResponseCode != SERVER_POSITIVE_ACK) {

		short responseCode = SendImageDataPayloadsBySequenceNumbers(imageDataPayloadMap, sequenceNumToPayloadSizeMap, 
			payloadSeqNumbersToSend);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nCould not send image payloads to server.";
			_msgLogger->LogError("Could not send image payloads to server.");
			return RESPONSE_FAILURE;
		}

		string serverResponse = "";
		responseCode = _ConsumeServerMsgFromQueue(serverResponse);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nCould not receive response from server.";
			_msgLogger->LogError("ERROR: Could not receive response from server.");

			return RESPONSE_FAILURE;
		}

		vector<string> serverResponseSplit = SplitString(&serverResponse[0], SERVER_RESPONSE_DELIMITER);

		responseCode = ValidateServerResponse(serverResponseSplit, serverResponseCode);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nERROR: Validation failed for server response.";
			_msgLogger->LogError("ERROR: Validation failed for server response.");

			return RESPONSE_FAILURE;
		}

		if (serverResponseCode == SERVER_NEGATIVE_ACK) {
			payloadSeqNumbersToSend.clear();
			for (int i = 2; i < serverResponseSplit.size(); i++) {
				try {
					payloadSeqNumbersToSend.push_back(stoi(serverResponseSplit.at(i)));
				}
				catch (invalid_argument) {
					//cout << "\nERROR: Sequence number sent by server not a number. Received sequence number: " << serverResponseSplit.at(i);
					_msgLogger->LogError("ERROR: Sequence number sent by server not a number. Received sequence number: " 
						+ serverResponseSplit.at(i));
				}
			}
		}
	}

	//cout << "\nAll image payloads received by server.";
	_msgLogger->LogError("All image payloads received by server.");

	return RESPONSE_SUCCESS;
}

short UDPClient::ConsumeImageDataFromQueue(const cv::Size& imageDimensions, const uint& imageFileSize, Mat& filteredImage)
{
	_msgLogger->LogError("Receiving image data...");
	long imageBytesProcessed = 0, imageBytesLeftToProcess = imageFileSize;
	short responseCode;

	u_short expectedNumberOfPayloads = imageBytesLeftToProcess / 60000;
	if (imageBytesLeftToProcess % 60000 > 0) {
		expectedNumberOfPayloads++;
	}
	
	string imageDataFromServer;
	map<u_short, string> imagePayloadSeqMap;

	int serverAddressSize = (sizeof(_serverAddress));

	auto lastImagePayloadRecdTime = high_resolution_clock::now();
	while (imageBytesLeftToProcess > 0) {

		if (_receivedServerMsgsQueue.empty()) {
			responseCode = _CheckForTimeout(lastImagePayloadRecdTime, imagePayloadSeqMap, expectedNumberOfPayloads);
			if (responseCode == RESPONSE_FAILURE) {
				//cout << "\nError when sending response to server on timeout.";
				_msgLogger->LogError("Error while sending response to server on timeout.");
				return RESPONSE_FAILURE;
			}
			continue;
		}
		
		_mtx.lock();
		imageDataFromServer = _receivedServerMsgsQueue.front();
		_receivedServerMsgsQueue.pop();
		_mtx.unlock();

		//add to payload map
		vector<string> splitImageDataPayload = SplitString(&imageDataFromServer[0], ' ', 5, imageDataFromServer.length());
		
		//cout << "\nSplit image payload size: " << splitImageDataPayload.size();
		_msgLogger->LogDebug("Split image payload size: " + to_string((ushort)splitImageDataPayload.size()));

		u_int payloadSeqNum = 0, imagePayloadSize = 0;

		responseCode = _ValidateImageDataPayload(splitImageDataPayload, payloadSeqNum, imagePayloadSize);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nValidation failed for image data payload from server.";
			_msgLogger->LogError("ERROR: Validation failed for image data payload from server.");
			return responseCode;
		}

		imagePayloadSeqMap[payloadSeqNum] = splitImageDataPayload.at(4);
		
		/*cout << "\nImage data after splitting: " << splitImageDataPayload.at(0) << " | " << splitImageDataPayload.at(1) << " | "
			<< splitImageDataPayload.at(2) << " | " << splitImageDataPayload.at(3) << " | Length of image data: " << splitImageDataPayload.at(4).length();*/
		_msgLogger->LogDebug("Image data after splitting: " + splitImageDataPayload.at(0) + " | " 
			+ splitImageDataPayload.at(1) + " | " + splitImageDataPayload.at(2) + " | " 
			+ splitImageDataPayload.at(3) + " | Length of image data: " + to_string((ushort)splitImageDataPayload.at(4).length()));

		imageBytesProcessed += imagePayloadSize;
		imageBytesLeftToProcess -= imagePayloadSize;

		//cout << "\nImage bytes recd: " << imageBytesProcessed << " | image bytes left to receive: " << imageBytesLeftToProcess;
		_msgLogger->LogDebug("Image bytes recd: " + to_string(imageBytesProcessed)
			+ " | Image bytes left to receive: " + to_string(imageBytesLeftToProcess));

		lastImagePayloadRecdTime = high_resolution_clock::now();
	}

	//cout << "\nAll image data received. Sending positive ACK to server.";
	_msgLogger->LogError("All image data received. Sending positive ACK to server.");

	responseCode = SendClientResponseToServer(CLIENT_POSITIVE_ACK, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nCould not send ACK to server.";
		_msgLogger->LogError("ERROR: Could not send ACK to server.");
		return RESPONSE_FAILURE;
	}

	ImageProcessor imageProcessor(imagePayloadSeqMap, imageDimensions, imageFileSize);
	imageProcessor.SaveImage();
	filteredImage = imageProcessor.GetImage();

	return RESPONSE_SUCCESS;
}

short UDPClient::_ValidateImageDataPayload(const std::vector<cv::String>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize)
{
	//TODO shift hardcoded values to constants
	if (splitImageDataPayload.size() != 5 || splitImageDataPayload.at(0) != SEQUENCE_PAYLOAD_KEY || splitImageDataPayload.at(2) != SIZE_PAYLOAD_KEY) {
		//cout << "\nERROR: Image data payload in incorrect format. First word: " << splitImageDataPayload.at(0);
		_msgLogger->LogError("ERROR: Image data payload in incorrect format. First word: " + splitImageDataPayload.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		payloadSeqNum = stoi(splitImageDataPayload.at(1));
		payloadSize = stoi(splitImageDataPayload.at(3));
	}
	catch (invalid_argument) {
		/*cout << "\nERROR: Image data payload sequence num or size not a number. Seq num: " << splitImageDataPayload.at(1)
			<< " | Size:" << splitImageDataPayload.at(3);*/
		_msgLogger->LogError("ERROR: Image data payload sequence num or size not a number. Seq num: " + splitImageDataPayload.at(1)
			+ " | Size:" + splitImageDataPayload.at(3));
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

short UDPClient::_CheckForTimeout(std::chrono::steady_clock::time_point& lastImagePayloadRecdTime,
	std::map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads)
{
	short responseCode;

	//Below snippet to calculate elapsed time taken from https://stackoverflow.com/a/31657669
	auto now = high_resolution_clock::now();
	auto timeElapsedSinceLastImagePayloadRecd = std::chrono::duration_cast<milliseconds>(now - lastImagePayloadRecdTime);

	// cout << "\ntimeElapsedSinceLastImagePayloadRecd: " << timeElapsedSinceLastImagePayloadRecd.count();

	if (timeElapsedSinceLastImagePayloadRecd.count() < IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS) {
		return RESPONSE_SUCCESS;
	}

	vector<u_short> missingSeqNumbers = _CalculateMissingPayloadSeqNumbers(imagePayloadSeqMap, expectedNumberOfPayloads);
	if (missingSeqNumbers.size() > 0) {
		responseCode = SendClientResponseToServer(CLIENT_NEGATIVE_ACK, &missingSeqNumbers);
	}
	else {
		responseCode = RESPONSE_SUCCESS;
	}

	lastImagePayloadRecdTime = high_resolution_clock::now();

	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nERROR: Could not send response to server.";
		_msgLogger->LogError("ERROR: Could not send response to server.");
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

vector<u_short> UDPClient::_CalculateMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads)
{
	vector<u_short> missingSeqNumbers;

	for (u_short i = 1; i <= expectedNumberOfPayloads; i++) {
		if (receivedPayloadsMap.count(i) == 0) {
			missingSeqNumbers.push_back(i);
		}
	}
	return missingSeqNumbers;
}

short UDPClient::ValidateServerResponse(std::vector<std::string>& serverResponseSplit, short& serverResponseCode)
{
	//cout << "\nValidating server response.";
	_msgLogger->LogDebug("Validating server response.");

	if (serverResponseSplit.size() < 2 || serverResponseSplit.at(0) != RESPONSE_PAYLOAD_KEY) {
		/*cout << "\nERROR: Server response not in expected format. Split size: " << serverResponseSplit.size();
		cout<<"\nFirst element: " << serverResponseSplit.at(0);*/
		_msgLogger->LogError("ERROR: Server response not in expected format. Split size: " + to_string((ushort)serverResponseSplit.size())
			+ " | First element: " + serverResponseSplit.at(0));

		return RESPONSE_FAILURE;
	}

	try {
		serverResponseCode = stoi(serverResponseSplit.at(1));
	}
	catch (invalid_argument) {
		//cout << "\nERROR: Response code not a number. Recd response code: " << serverResponseSplit.at(1);
		_msgLogger->LogError("ERROR: Response code not a number. Recd response code: " + serverResponseSplit.at(1));
		return RESPONSE_FAILURE;
	}
	
	//cout << "\nServer response validation successful.";
	_msgLogger->LogDebug("Server response validation successful.");

	return RESPONSE_SUCCESS;
}

short UDPClient::_ValidateImageMetadataFromServer(std::vector<cv::String>& serverMsgSplit, cv::Size& imageDimensions, uint& imageFileSize)
{
	_msgLogger->LogError("Validating image metadata sent by server...");

	if (serverMsgSplit.at(0) != SIZE_PAYLOAD_KEY || serverMsgSplit.size() < MIN_IMAGE_METADATA_PARAMS_FROM_SERVER) {
		//cout << "\nServer sent image meta data in wrong format.";
		_msgLogger->LogError("ERROR: Server sent image meta data in wrong format.");
		return RESPONSE_FAILURE;
	}
	try {
		imageDimensions = cv::Size(stoi(serverMsgSplit.at(1)), stoi(serverMsgSplit.at(2)));
		imageFileSize = stoi(serverMsgSplit.at(3));
	}
	catch (invalid_argument iaExp) {
		//cout << "\nInvalid image size values received.";
		_msgLogger->LogError("ERROR: Invalid image size values received.");

		return RESPONSE_FAILURE;
	}

	_msgLogger->LogError("Image metadata validation successful.");
	return RESPONSE_SUCCESS;
}

bool UDPClient::_ShouldListenThreadSafe()
{
	//Below code to wrap shared loop variables in a thread-safe function referred from https://stackoverflow.com/a/5927246

	bool result;
	_mtx.lock();
	result = _shouldKeepListening;
	_mtx.unlock();
	return result;
}

bool UDPClient::_IsQueueEmptyThreadSafe()
{
	//Below code to wrap shared loop variables in a thread-safe function referred from https://stackoverflow.com/a/5927246
	bool result;

	_mtx.lock();
	result = _receivedServerMsgsQueue.empty();
	_mtx.unlock();

	return result;
}

short UDPClient::SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, 
	map<u_short, u_short>& sequenceNumToPayloadSizeMap, const vector<u_short>& payloadSeqNumbersToSend)
{
	for (u_short payloadSeqNumberToSend : payloadSeqNumbersToSend) {
		char* payloadToSend = &(imageDataPayloadMap[payloadSeqNumberToSend][0]);

		int sendResult = sendto(_socket, payloadToSend, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend], 0, 
			(const sockaddr*) &_serverAddress, sizeof(_serverAddress));
		if (sendResult == SOCKET_ERROR) {
			//cout << "\nERROR while sending image payload to server. Error code "<<WSAGetLastError();
			_msgLogger->LogError("ERROR while sending image payload to server. Error code " + to_string(WSAGetLastError()));

			return RESPONSE_FAILURE;
		}
		//cout << "\nSent payload #" << payloadSeqNumberToSend;
		_msgLogger->LogDebug("Sent payload #" + to_string(payloadSeqNumberToSend));
	}
	
	//cout << "\nImage payloads sent to server.";
	_msgLogger->LogDebug("Image payloads sent to server.");

	return RESPONSE_SUCCESS;
}

short UDPClient::ReceiveServerMsgs()
{
	int serverAddressSize = sizeof(_serverAddress);

	char* serverResponse = new char[MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES];
	int bytesRecd = 0;

	//cout << "\nThread ID: "<<get_id()<<" | Started listening to server msgs.";
	_msgLogger->LogError("Started listening to server msgs.");

	while (_ShouldListenThreadSafe()) {

		_mtx.lock();
		bytesRecd = recvfrom(_socket, serverResponse, MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES, 0, (sockaddr*)&_serverAddress, &serverAddressSize);
		_mtx.unlock();

		if (bytesRecd == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				//cout << "\nError while receving data from server. Error code: " << WSAGetLastError();
				_msgLogger->LogError("Error while receving data from server. Error code: " + to_string(WSAGetLastError()));
				return RESPONSE_FAILURE;
			}
			//sleep_for(milliseconds(50));
		}
		else {
			string serverMsg = string(serverResponse, bytesRecd);

			_mtx.lock();
			_receivedServerMsgsQueue.push(serverMsg);
			_mtx.unlock();

			//cout << "\nThread ID: "<<get_id()<<" | Server msg pushed into queue.";
			_msgLogger->LogDebug("Server msg pushed into queue.");
			//break;
		}
	}

	//cout << "\nThread ID: " << get_id() << " | Stopped listening to server msgs.";
	_msgLogger->LogError("Stopped listening to server msgs.");

	return RESPONSE_SUCCESS;
}

int UDPClient::_DrainQueue(std::string& msgInQueue)
{
	long bytesRecd = 0;

	while (!_IsQueueEmptyThreadSafe()) {

		//cout << "\nDrainQueue::Before popping from queue.";
		_msgLogger->LogDebug("DrainQueue::Before popping from queue.");

		_mtx.lock();
		msgInQueue += _receivedServerMsgsQueue.front();
		_receivedServerMsgsQueue.pop();
		_mtx.unlock();

		//cout << "\nmsgInQueue: " << msgInQueue;
		bytesRecd += msgInQueue.length();

		//cout << "\nQueue msg string at iteration end: " << msgInQueue;
		_msgLogger->LogDebug("Msg in queue: " + msgInQueue);
	}
	return bytesRecd;
}

short UDPClient::ReceiveAndValidateServerResponse(short& serverResponseCode)
{
	string serverResponse = "";

	short responseCode = _ConsumeServerMsgFromQueue(serverResponse);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nDid not receive response from server.";
		_msgLogger->LogError("ERROR: Did not receive response from server.");

		return RESPONSE_FAILURE;
	}
	
	_msgLogger->LogError("Received server response: " + serverResponse);

	vector<string> serverResponseSplit = SplitString(&serverResponse[0], SERVER_RESPONSE_DELIMITER);
	return ValidateServerResponse(serverResponseSplit, serverResponseCode);
}

short UDPClient::_ConsumeServerMsgFromQueue(std::string& serverMsg)
{
	string serverMsgInQueue = "";
	while (!serverMsgInQueue.ends_with('\0')) {
		//TODO if lastMsgConsumedTime exceeds timeout limit, assume server has died and return response_failure.
		long bytesRecd = _DrainQueue(serverMsgInQueue);
		//TODO if bytesRecd > 0, update lastMsgConsumedTime.
	}
	serverMsg += serverMsgInQueue;
	return RESPONSE_SUCCESS;
}

short UDPClient::ReceiveAndValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize)
{
	string serverMsg = "";

	short responseCode = _ConsumeServerMsgFromQueue(serverMsg);
	
	_msgLogger->LogError("Received image metadata from server: " + serverMsg);

	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nDid not receive image metadata from server.";
		_msgLogger->LogError("ERROR: Could not receive image metadata from server.");
		return RESPONSE_FAILURE;
	}

	vector<string> serverMsgSplit = SplitString(&serverMsg[0], SERVER_RESPONSE_DELIMITER);
	return _ValidateImageMetadataFromServer(serverMsgSplit, imageDimensions, imageFileSize);
}

void UDPClient::StartListeningForServerMsgs()
{
	_mtx.lock();
	_shouldKeepListening = true;
	_mtx.unlock();

	ReceiveServerMsgs();
}

void UDPClient::StopListeningForServerMsgs()
{
	_mtx.lock();
	_shouldKeepListening = false;
	_mtx.unlock();
}

short UDPClient::fragmentAndSendImageData(cv::Mat& imageToSend, const long& imageSize)
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
			
			bytesSentThisIteration = sendto(_socket, &payload[0], 60000 + payloadLength, 0, (const sockaddr*)&_serverAddress, sizeof(_serverAddress));
		}
		else {
			payload = "Seq " + to_string(payloadSequenceNum) + " Size " + to_string(bytesLeftToSend) + " ";
			payloadLength = payload.length();
			string imageData = string((char*)imagePtr + bytesSent, bytesLeftToSend);
			payload += imageData;

			bytesSentThisIteration = sendto(_socket, &payload[0], bytesLeftToSend + payloadLength, 0, (const sockaddr*)&_serverAddress, sizeof(_serverAddress));
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

		//sleep_for(milliseconds(50));
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

		cout << "\nSocket IP: " << thisSocket.sin_addr.s_addr << " | Socket port: " << thisSocket.sin_port
			<<" | Family: "<<thisSocket.sin_family;

		/*thisSocket.sin_addr.s_addr = INADDR_ANY;
		thisSocket.sin_family = AF_INET;
		if (bind(_socket, (sockaddr*)&thisSocket, thisSocketSize) == SOCKET_ERROR) {
			cout << "\nError while binding client socket. Error code: " << WSAGetLastError();
			_socket = INVALID_SOCKET;
		}
		else {
			cout << "\nSocket bound successfully.";
		}*/

		u_long nonBlockingModeTrue = 1;
		if (ioctlsocket(_socket, FIONBIO, &nonBlockingModeTrue) == SOCKET_ERROR) {
			cout << "\nError while setting socket to non-blocking mode.";
			_socket = INVALID_SOCKET;
		}
	}
}