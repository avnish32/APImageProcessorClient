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
	ushort DrainQueue(std::string&);
	void MakeServerAddress(const std::string&, const USHORT&);
	const vector<std::string> SplitString(char*, char);
	const vector<string> SplitString(char*, const char&, const int&, const int&);
	void BuildImageDataPayloadMap(Mat, map<u_short, string>&,
		map<u_short, u_short>&, vector<u_short>&);
	bool HasRequestTimedOut(const high_resolution_clock::time_point&, const ushort&);
	vector<u_short> GetMissingPayloadSeqNumbers(const map<u_short, string>&, u_short);
	
	//Validation functions
	short ValidateServerResponse(std::vector<cv::String>&, short&);
	short ValidateImageMetadataFromServer(std::vector<cv::String>&, cv::Size&, uint&);
	short ValidateImageDataPayload(const std::vector<cv::String>&, u_int&, u_int&);

	//Functions to send data to server
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>&, map<u_short, u_short>&,
		const vector<u_short>&);
	short SendMissingSeqNumbersToServer(map<u_short, std::string>&, const u_short&
		, vector<u_short>&, short&);

	//Functions to receive data from server
	short ReceiveServerMsgs();
	short ConsumeServerMsgFromQueue(std::string&);

public:
	UDPClient();
	UDPClient(const string&, const USHORT&);
	~UDPClient();
	
	//Utility functions
	bool IsValid();
	void StartListeningForServerMsgs();
	void StopListeningForServerMsgs();

	//Functions to send data to server
	short SendClientResponseToServer(const short&, const vector<u_short>*);
	short SendImageMetadataToServer(std::string);
	short SendImage(const Mat);

	//Functions to receive data from server
	short ConsumeImageDataFromQueue(const cv::Size&, const uint&, ImageProcessor&);
	short ReceiveAndValidateServerResponse(short&);
	short ReceiveAndValidateImageMetadata(cv::Size&, uint&);
};

