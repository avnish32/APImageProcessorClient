#include "Constants.h"
#include "UDPClient.h"
#include "ImageProcessor.h"

#include<iostream>

using std::string;
using std::thread;
using std::vector;
using std::map;
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
	InitializeSocket();
	MakeServerAddress(DEFAULT_SERVER_IP_ADDRESS, DEFAULT_SERVER_PORT);
	should_keep_listening_ = true;
}

UDPClient::UDPClient(const string& server_ip, const USHORT& server_port)
{
	InitializeSocket();
	MakeServerAddress(server_ip, server_port);

	mtx_.lock();
	should_keep_listening_ = true;
	mtx_.unlock();
}

UDPClient::~UDPClient()
{
	msg_logger_->LogDebug("Destroying UDP Client...");
	mtx_.lock();
	should_keep_listening_ = false;
	mtx_.unlock();

	WSACleanup();
	closesocket(socket_);
	msg_logger_->LogDebug("Destroyed UDP Client.");
}

/*
-------------------
Utility functions
-------------------
*/

/*
This function initializes the client socket and sets it to non-blocking mode.
*/
void UDPClient::InitializeSocket() {
	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1) {
		msg_logger_->LogError("WSAStartup failed.");
		socket_ = INVALID_SOCKET;
		return;
	}
	socket_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("Error while creating client socket. Error code: " + to_string(WSAGetLastError()));
		return;
	}
	else {
		msg_logger_->LogError("Client socket created successfully.");

		// Below block to print ip and port of the client and to bind socket.
		// Since multiple sockets are created in parellel threads, 
		// it attempts to bind multiple sockets to same port which is not allowed.
		// Hence, this is commented to allow multiple client threads to run in parallel,
		// to demonstrate multi-client handling capability of the server.
		

		//sockaddr_in this_socket;
		//int thisSocketSize = sizeof(this_socket);
		//getsockname(socket_, (sockaddr*)&this_socket, &thisSocketSize);

		//msg_logger_->LogDebug("Socket IP: " + to_string(this_socket.sin_addr.s_addr) + " | Socket port: " + to_string(this_socket.sin_port)
		//	+ " | Family: " + to_string(this_socket.sin_family));
		///*cout << "\nSocket IP: " << this_socket.sin_addr.s_addr << " | Socket port: " << this_socket.sin_port
		//	<< " | Family: " << this_socket.sin_family;*/

		//this_socket.sin_addr.s_addr = INADDR_ANY;
		//this_socket.sin_family = AF_INET;
		//if (bind(socket_, (sockaddr*)&this_socket, thisSocketSize) == SOCKET_ERROR) {
		//	msg_logger_->LogError("Error while binding client socket.Error code : " + to_string(WSAGetLastError()));
		//	socket_ = INVALID_SOCKET;
		//}
		//else {
		//	msg_logger_->LogError("Socket bound successfully.");
		//}

		u_long NON_BLOCKING_MODE_TRUE = 1;
		if (ioctlsocket(socket_, FIONBIO, &NON_BLOCKING_MODE_TRUE) == SOCKET_ERROR) {
			msg_logger_->LogError("Error while setting socket to non-blocking mode.");
			socket_ = INVALID_SOCKET;
		}
	}
}

/*
This functions checks in a thread-safe manner if the client should keep listening for server messages.
*/
bool UDPClient::ShouldListenThreadSafe()
{
	//Below code to wrap shared loop variables in a thread-safe function referred from https://stackoverflow.com/a/5927246

	bool result;
	mtx_.lock();
	result = should_keep_listening_;
	mtx_.unlock();
	return result;
}

/*
This functions checks if _receivedServerMsgsQueue is empty in a thread-safe manner.
*/
bool UDPClient::IsQueueEmptyThreadSafe()
{
	//Below code to wrap shared loop variables in a thread-safe function referred from https://stackoverflow.com/a/5927246
	bool result;

	mtx_.lock();
	result = received_server_msgs_queue_.empty();
	mtx_.unlock();

	return result;
}

/*
Drains the queue of this particular client and keeps appending queue messages to msgInQueue
until the queue becomes empty or a message is encountered in the queue having '\0'
as its last character, whichever occurs earlier.
*/
ushort UDPClient::DrainQueue(std::string& msg_in_q)
{
	long bytes_recd = 0;

	while (!IsQueueEmptyThreadSafe() && !msg_in_q.ends_with(STRING_TERMINATING_CHAR)) {
		msg_logger_->LogDebug("DrainQueue::Before popping from queue.");

		mtx_.lock();
		msg_in_q += received_server_msgs_queue_.front();
		received_server_msgs_queue_.pop();
		mtx_.unlock();

		bytes_recd += msg_in_q.length();
		msg_logger_->LogDebug("Msg in queue: " + msg_in_q);
	}
	return bytes_recd;
}

/*
Initializes the _serverAddress object using serverIp and serverPort.
*/
void UDPClient::MakeServerAddress(const std::string& server_ip, const USHORT& server_port) {
	inet_pton(AF_INET, server_ip.c_str(), &(server_address_.sin_addr));
	server_address_.sin_family = AF_INET;
	server_address_.sin_port = server_port;
}

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> UDPClient::SplitString(char* input_string, char delimiter) {
	std::string current_word = EMPTY_STRING;
	vector<std::string> output_vector;
	char* char_ptr = input_string;

	msg_logger_->LogDebug("Starting to split the server response.");

	while (*(char_ptr) != STRING_TERMINATING_CHAR) {
		char current_char = *(char_ptr);
		if (current_char == delimiter) {
			output_vector.push_back(current_word);
			current_word = EMPTY_STRING;
		}
		else {
			current_word += current_char;
		}
		++char_ptr;
	}
	if (current_word.length() > 0) {
		output_vector.push_back(current_word);
	}

	string log_string_after_splitting = "String after splitting: ";
	auto iter = output_vector.begin();
	while (iter != output_vector.end()) {
		log_string_after_splitting.append(*iter).append(" | ");
		iter++;
	}
	msg_logger_->LogDebug(log_string_after_splitting);
	msg_logger_->LogDebug("Returning output vector.");

	return output_vector;
}

/*
Splits inputString by the delimiter character until the splits reach numberOfSplits,
or the whole length of the string given by inputStringLength has been traversed,
whichever occurs earlier.
*/
const vector<string> UDPClient::SplitString(char* input_string, const char& delimiter, const int& no_of_splits, const int& input_string_length) {
	std::string current_word = EMPTY_STRING;
	int no_of_current_splits = 1;
	vector<string> output_vector;

	for (int i = 0; i < input_string_length; i++) {
		char current_char = *(input_string + i);
		if (current_char == delimiter && no_of_current_splits < no_of_splits) {
			output_vector.push_back(current_word);
			current_word = EMPTY_STRING;
			no_of_current_splits++;
		}
		else {
			current_word += current_char;
		}
	}

	if (current_word.length() > 0) {
		output_vector.push_back(current_word);
	}

	if (output_vector.size() < no_of_splits) {
		stringstream s_stream;
		s_stream << input_string;
		msg_logger_->LogError("Unexpected split. Original string: " + s_stream.str());
	}
	return output_vector;
}

/*
Fragments image into payloads of 60,000 bytes and builds a map of the image data
in the format "<sequence_number>" -> "SEQ <sequence number> SIZE <payload size in bytes> <image data>" without the quotes.
This increases reliability by keeping track of which payloads were successfully received by the server.
It also helps retrieve the data of the payloads not received by the server so they can be re-sent.
*/
void UDPClient::BuildImageDataPayloadMap(Mat image, map<u_short, string>& image_data_payload_map,
	map<u_short, u_short>& seq_no_to_payload_size_map, vector<u_short>& seq_nums)
{
	u_int image_bytes_left = image.elemSize() * image.total();
	u_int image_bytes_processed = 0;
	string payload;
	u_short payload_size;
	u_short payload_seq_no = 1;
	auto image_ptr = image.data;

	while (image_bytes_left > 0) {

		u_short image_bytes_processed_this_iteration = 0;
		if (image_bytes_left >= MAX_IMG_DATA_BYTES_IN_PAYLOAD) {

			payload = string(SEQUENCE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
				.append(to_string(payload_seq_no)).append(CLIENT_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
				.append(to_string(MAX_IMG_DATA_BYTES_IN_PAYLOAD)).append(CLIENT_MSG_DELIMITER);

			payload_size = payload.length();
			string image_data = string((char*)image_ptr + image_bytes_processed, MAX_IMG_DATA_BYTES_IN_PAYLOAD);
			payload += image_data;

			image_bytes_processed_this_iteration = MAX_IMG_DATA_BYTES_IN_PAYLOAD;
		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
				.append(to_string(payload_seq_no)).append(CLIENT_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
				.append(to_string(image_bytes_left)).append(CLIENT_MSG_DELIMITER);

			payload_size = payload.length();
			string image_data = string((char*)image_ptr + image_bytes_processed, image_bytes_left);
			payload += image_data;

			image_bytes_processed_this_iteration = image_bytes_left;
		}

		image_data_payload_map[payload_seq_no] = payload;

		payload_size += image_bytes_processed_this_iteration;
		seq_no_to_payload_size_map[payload_seq_no] = payload_size;

		seq_nums.push_back(payload_seq_no);
		image_bytes_processed += image_bytes_processed_this_iteration;
		image_bytes_left -= image_bytes_processed_this_iteration;

		payload_seq_no++;	
		msg_logger_->LogDebug("Bytes processed: " + to_string(image_bytes_processed));
	}
}

/*
Checks if timeoutDuration (in milliseconds) has passed between now and when the last message was received, given by lastMsgRecdTime.
*/
bool UDPClient::HasRequestTimedOut(const high_resolution_clock::time_point& last_msg_recd_time, const ushort& timeout_duration)
{
	//Below snippet to calculate elapsed time taken from https://stackoverflow.com/a/31657669
	auto now = high_resolution_clock::now();
	auto time_elapsed_since_last_msg_recd = duration_cast<milliseconds>(now - last_msg_recd_time);

	if (time_elapsed_since_last_msg_recd.count() >= timeout_duration) {
		return true;
	}

	return false;
}

/*
Returns the sequence numbers present in the series 0 to expectedNumberOfPayloads
but not in the key set of receivedPayloadsMap. Basically returns the sequence numbers
that are not yet received from the server.
*/
vector<u_short> UDPClient::GetMissingPayloadSeqNumbers(const map<u_short, string>& recd_payloads_map, u_short expected_no_of_payloads)
{
	vector<u_short> missing_seq_nums;

	for (u_short i = 1; i <= expected_no_of_payloads; i++) {
		if (recd_payloads_map.count(i) == 0) {
			missing_seq_nums.push_back(i);
		}
	}
	return missing_seq_nums;
}

bool UDPClient::IsValid()
{
	mtx_.lock();
	if (socket_ != INVALID_SOCKET) {
		mtx_.unlock();
		return true;
	}
	mtx_.unlock();
	return false;
}

/*
This function tells the client that it should keep listening to messages from server
from this point onwards. When the client is ready to receive messages, this is the method
that is called in a new thread so the receiving and processing logic remain independent of
each other and that one does not interfere with the CPU time of the other.
*/
void UDPClient::StartListeningForServerMsgs()
{
	mtx_.lock();
	should_keep_listening_ = true;
	mtx_.unlock();

	ReceiveServerMsgs();
}

/*
This function tells the client that the image processing is completed and
it should stop listening to messages from server now.
*/
void UDPClient::StopListeningForServerMsgs()
{
	mtx_.lock();
	should_keep_listening_ = false;
	mtx_.unlock();
}

/*
---------------------
Validation functions
---------------------
*/


/*
* This function validates the general server response.
* The expected format for this kind of response is "RES <responseCode>" without the quotes.
* Dependng on the circumstances, this response can have more parameters, but never less.
*/
short UDPClient::ValidateServerResponse(std::vector<std::string>& server_response_split, short& server_response_code)
{
	msg_logger_->LogDebug("Validating server response.");

	if (server_response_split.size() < 2 || server_response_split.at(0) != RESPONSE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Server response not in expected format. Split size: " + to_string((ushort)server_response_split.size())
			+ " | First element: " + server_response_split.at(0));

		return FAILURE_RESPONSE;
	}

	try {
		server_response_code = stoi(server_response_split.at(1));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Response code not a number. Recd response code: " + server_response_split.at(1));
		return FAILURE_RESPONSE;
	}
	msg_logger_->LogDebug("Server response validation successful.");

	return SUCCESS_RESPONSE;
}

/*
This function validates the image metadata.
A valid image metadata has the following format:
"SIZE <image width> <image height> <image size in bytes>"
without the quotes.
*/
short UDPClient::ValidateImageMetadataFromServer(std::vector<cv::String>& server_msg_split, cv::Size& image_dimensions, uint& image_file_size)
{
	msg_logger_->LogError("Validating image metadata sent by server...");

	if (server_msg_split.at(0) != SIZE_PAYLOAD_KEY || server_msg_split.size() < MIN_IMG_METADATA_PARAMS_FROM_SERVER) {
		msg_logger_->LogError("ERROR: Server sent image meta data in wrong format.");
		return FAILURE_RESPONSE;
	}
	try {
		image_dimensions = cv::Size(stoi(server_msg_split.at(1)), stoi(server_msg_split.at(2)));
		image_file_size = stoi(server_msg_split.at(3));
	}
	catch (invalid_argument iaExp) {
		msg_logger_->LogError("ERROR: Invalid image size values received.");

		return FAILURE_RESPONSE;
	}

	msg_logger_->LogError("Image metadata validation successful.");
	return SUCCESS_RESPONSE;
}

/*
* Validates the image data payload sent by the server.
* The expected format for this kind of message is: "SEQ <payload sequence number> SIZE <size of the image data in payload> <image data>"
* without the quotes.
*/
short UDPClient::ValidateImageDataPayload(const std::vector<cv::String>& split_image_data_payload, u_int& payload_seq_no, u_int& payload_size)
{
	if (split_image_data_payload.size() != NUM_OF_IMG_DATA_PARAMS || split_image_data_payload.at(0) != SEQUENCE_PAYLOAD_KEY
		|| split_image_data_payload.at(2) != SIZE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Image data payload in incorrect format. First word: " + split_image_data_payload.at(0));
		return FAILURE_RESPONSE;
	}

	try {
		payload_seq_no = stoi(split_image_data_payload.at(1));
		payload_size = stoi(split_image_data_payload.at(3));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Image data payload sequence num or size not a number. Seq num: " + split_image_data_payload.at(1)
			+ " | Size:" + split_image_data_payload.at(3));
		return FAILURE_RESPONSE;
	}

	return SUCCESS_RESPONSE;
}

/*
---------------------------------
Functions to send data to server
---------------------------------
*/

/*
This function fetches the image data payload corresponding to the numbers in payloadSeqNumbersToSend from imageDataPayloadMap
and sends them to the server.
*/
short UDPClient::SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& image_data_payload_map,
	map<u_short, u_short>& seq_no_to_payload_size_map, const vector<u_short>& payload_seq_nums_to_send)
{
	for (u_short payload_seq_num_to_send : payload_seq_nums_to_send) {
		char* payload_to_send = &(image_data_payload_map[payload_seq_num_to_send][0]);

		int send_result = sendto(socket_, payload_to_send, seq_no_to_payload_size_map[payload_seq_num_to_send], 0,
			(const sockaddr*)&server_address_, sizeof(server_address_));
		if (send_result == SOCKET_ERROR) {
			msg_logger_->LogError("ERROR while sending image payload to server. Error code " + to_string(WSAGetLastError()));

			return FAILURE_RESPONSE;
		}
		msg_logger_->LogDebug("Sent payload #" + to_string(payload_seq_num_to_send));
	}
	msg_logger_->LogDebug("Image payloads sent to server.");

	return SUCCESS_RESPONSE;
}

/*
This function constructs a vector of the payload sequence numbers that the client did not receive
from the sesrver before the last timeout occurred. It then sends these sequence numbers along with
a negative response code to the server so server can re-send the missing payloads.
*/
short UDPClient::SendMissingSeqNumbersToServer(map<u_short, std::string>& image_payload_seq_map, const u_short& expected_no_of_payloads
	, vector<u_short>& missing_seq_nums_in_last_timeout, short& server_inactive_count)
{
	short response_code;

	vector<u_short> missing_seq_nums_in_this_timeout = GetMissingPayloadSeqNumbers(image_payload_seq_map, expected_no_of_payloads);
	if (missing_seq_nums_in_this_timeout.size() > 0) {
		if (missing_seq_nums_in_last_timeout == missing_seq_nums_in_this_timeout) {

			//Server did not send any more payloads since last timeout or all paylads were lost. Incrementing inactive count.
			msg_logger_->LogError("Server is inactive or all packets were lost.");
			server_inactive_count++;

			if (server_inactive_count == SERVER_INACTIVE_LIMIT) {
				msg_logger_->LogError("Terminating connection with server as it seems to be inactive.");
				return FAILURE_RESPONSE;
			}
		}
		else {
			server_inactive_count = 0;
		}
		missing_seq_nums_in_last_timeout = missing_seq_nums_in_this_timeout;

		msg_logger_->LogError("Incomplete image data. Requesting missing data from server...");
		response_code = SendClientResponseToServer(CLIENT_NEGATIVE_ACK, &missing_seq_nums_in_this_timeout);
	}
	else {
		response_code = SUCCESS_RESPONSE;
	}

	if (response_code == FAILURE_RESPONSE) {
		msg_logger_->LogError("ERROR: Could not send response to server.");
		return FAILURE_RESPONSE;
	}

	return SUCCESS_RESPONSE;
}

/*
* Sends response to the server in the format "RES <response code> <missing payload sequence numbers (if any)>"
* without the quotes.
* This response helps the server to ensure that client has received the message/s succuessfully.
*/
short UDPClient::SendClientResponseToServer(const short& client_response_code, const vector<u_short>* missing_seq_nums)
{
	msg_logger_->LogDebug("Sending client response...");

	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid socket.");
		return FAILURE_RESPONSE;
	}

	string missing_seq_nums_string = EMPTY_STRING;
	if (missing_seq_nums != nullptr) {
		for (const u_short& missing_seq_num : *missing_seq_nums) {
			missing_seq_nums_string.append(to_string(missing_seq_num)).append(CLIENT_MSG_DELIMITER);
		}
	}
	string client_response_payload = string(RESPONSE_PAYLOAD_KEY).append(CLIENT_MSG_DELIMITER)
		.append(to_string(client_response_code)).append(CLIENT_MSG_DELIMITER)
		.append(missing_seq_nums_string).append(EMPTY_STRING + STRING_TERMINATING_CHAR);

	msg_logger_->LogDebug("Client response string: " + client_response_payload
		+ " | String length: " + to_string((ushort)client_response_payload.length()));

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	short bytes_sent = sendto(socket_, &client_response_payload[0], client_response_payload.length() + 1, 0,
		(const sockaddr*)&server_address_, sizeof(server_address_));

	if (bytes_sent <= 0) {
		msg_logger_->LogError("Error while sending response to server. Error code: " + to_string(WSAGetLastError()));
		return FAILURE_RESPONSE;
	}

	msg_logger_->LogError("Client response sent successfully.");
	return SUCCESS_RESPONSE;
}

/*
Sends the image metadata string to the server.
*/
short UDPClient::SendImageMetadataToServer(std::string image_metadata_payload)
{
	msg_logger_->LogError("Sending image metadata to server.");

	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid client socket.");

		return FAILURE_RESPONSE;
	}

	ushort payload_size = image_metadata_payload.length() + 1;

	msg_logger_->LogError("Image metadata payload before sending: " + image_metadata_payload + " | Size: " + to_string(payload_size));

	int bytes_sent = 0;

	while (bytes_sent < payload_size) {
		int bytes_sent_this_iteration = sendto(socket_, &image_metadata_payload[0] + bytes_sent, payload_size - bytes_sent,
			0, (sockaddr*)&server_address_, sizeof(server_address_));
		if (bytes_sent_this_iteration <= 0) {
			msg_logger_->LogError("Error while sending image size. Error code: " + to_string(WSAGetLastError()));
			return FAILURE_RESPONSE;
		}
		bytes_sent += bytes_sent_this_iteration;
	}
	msg_logger_->LogError("Image metadata successfully sent to server.");
	return SUCCESS_RESPONSE;
}

/*
This function builds the payload map for imageToSend and sends all image payloads
to the server. It also listens for server response after sending each batch of payloads
to ascertain the ones lost in transit. These payloads are then re-sent in the next iteration,
and the process is repeated until a positive acknowledgment is received from the server, or
the sesrver disconnects, whichever occurs earlier.
*/
short UDPClient::SendImage(const Mat image_to_send)
{
	msg_logger_->LogError("Sending image to server...");

	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid client socket.");

		return FAILURE_RESPONSE;
	}

	map<u_short, string> image_data_payload_map;
	map<u_short, u_short> seq_no_to_payload_size_map;
	vector<u_short> payload_seq_nums_to_send;
	short server_response_code = SERVER_NEGATIVE_ACK;

	BuildImageDataPayloadMap(image_to_send, image_data_payload_map, seq_no_to_payload_size_map, payload_seq_nums_to_send);

	while (server_response_code != SERVER_POSITIVE_ACK) {

		short response_code = SendImageDataPayloadsBySequenceNumbers(image_data_payload_map, seq_no_to_payload_size_map,
			payload_seq_nums_to_send);
		if (response_code == FAILURE_RESPONSE) {
			msg_logger_->LogError("Could not send image payloads to server.");
			return FAILURE_RESPONSE;
		}

		string server_response = EMPTY_STRING;
		response_code = ConsumeServerMsgFromQueue(server_response);
		if (response_code == FAILURE_RESPONSE) {
			msg_logger_->LogError("ERROR: Could not receive response from server.");
			return FAILURE_RESPONSE;
		}

		vector<string> server_response_split = SplitString(&server_response[0], SERVER_RESPONSE_DELIMITER);

		response_code = ValidateServerResponse(server_response_split, server_response_code);
		if (response_code == FAILURE_RESPONSE) {
			msg_logger_->LogError("ERROR: Validation failed for server response.");
			return FAILURE_RESPONSE;
		}

		if (server_response_code == SERVER_NEGATIVE_ACK) {
			payload_seq_nums_to_send.clear();
			for (int i = 2; i < server_response_split.size(); i++) {
				try {
					payload_seq_nums_to_send.push_back(stoi(server_response_split.at(i)));
				}
				catch (invalid_argument) {
					msg_logger_->LogError("ERROR: Sequence number sent by server not a number. Received sequence number: "
						+ server_response_split.at(i));
				}
			}
		}
	}

	msg_logger_->LogError("All image payloads received by server.");
	return SUCCESS_RESPONSE;
}

/*
---------------------------------------
Functions to receive data from server
--------------------------------------
*/

/*
This functions is responsible solely for receiving messages from the server and pushing them
into the queue of this particular UDPClient object. It should be run in its own separate thread using the 
StartListeningForServerMsgs function. This is to minimise packet loss as having the receiving logic
run asynchronously w.r.t. the processing logic will ensure that application listens for messages without
being hampered by payload processing time.
*/
short UDPClient::ReceiveServerMsgs()
{
	int server_address_size = sizeof(server_address_);

	char* server_response = new char[MAX_SERVER_PAYLOAD_SIZE_BYTES];
	int bytes_recd = 0;

	msg_logger_->LogError("Started listening to server msgs.");

	while (ShouldListenThreadSafe()) {

		mtx_.lock();
		bytes_recd = recvfrom(socket_, server_response, MAX_SERVER_PAYLOAD_SIZE_BYTES, 0, (sockaddr*)&server_address_, &server_address_size);
		mtx_.unlock();

		if (bytes_recd == SOCKET_ERROR) {
			int last_error = WSAGetLastError();

			if (last_error != WSAEWOULDBLOCK) {
				msg_logger_->LogError("Error while receving data from server. Error code: " + to_string(WSAGetLastError()));
				return FAILURE_RESPONSE;
			}
		}
		else {
			string server_msg = string(server_response, bytes_recd);

			mtx_.lock();
			received_server_msgs_queue_.push(server_msg);
			mtx_.unlock();

			msg_logger_->LogDebug("Server msg pushed into queue.");
		}
	}
	msg_logger_->LogError("Stopped listening to server msgs.");
	return SUCCESS_RESPONSE;
}

/*
This functions drains the queue and keeps appending to serverMsg
until a message is found having '\0' as its last character.
While consuming from the queue, it checks for timeout and returns if a timeout does occur,
assuming that the server is not active anymore.
*/
short UDPClient::ConsumeServerMsgFromQueue(std::string& server_msg)
{
	string server_msg_in_q = EMPTY_STRING;
	auto last_msg_consumed_time = high_resolution_clock::now();

	while (!server_msg_in_q.ends_with(STRING_TERMINATING_CHAR)) {

		ushort bytes_recd_this_iteration = DrainQueue(server_msg_in_q);
		if (bytes_recd_this_iteration > 0) {
			last_msg_consumed_time = high_resolution_clock::now();
			continue;
		}
		if (HasRequestTimedOut(last_msg_consumed_time, SERVER_MSG_RECV_TIMEOUT_MILLIS)) {
			//Server is inactive.
			msg_logger_->LogError("ERROR: Client timed out while waiting for server response.");
			return FAILURE_RESPONSE;
		}
	}
	server_msg += server_msg_in_q;
	return SUCCESS_RESPONSE;
}

/*
This function consumes the image data from the queue of this client object until no more bytes are left to receive. 
The bytes left to receive are calculated from an earlier server message. While popping from the queue, the function 
also checks if timeout has occurred every time it finds the queue empty. If a timeout does occur, it assumes that 
the server is not active anymore and returns.
Each raw image payload is then split by 'space' character and validated, before updating the imagePayloadSeqMap with the newly
consumed payload.
*/
short UDPClient::ConsumeImageDataFromQueue(const cv::Size& image_dimensions, const uint& image_file_size, ImageProcessor& image_processor)
{
	msg_logger_->LogError("Receiving image data...");
	long image_bytes_processed = 0, image_bytes_left_to_process = image_file_size;
	short response_code;

	u_short expected_no_of_payloads = image_bytes_left_to_process / MAX_IMG_DATA_BYTES_IN_PAYLOAD;
	if (image_bytes_left_to_process % MAX_IMG_DATA_BYTES_IN_PAYLOAD > 0) {
		expected_no_of_payloads++;
	}

	string image_data_from_server;
	map<u_short, string> image_payload_seq_map;
	vector<u_short> missing_payload_seq_nums;
	short server_inactive_count = 0;

	int server_address_size = (sizeof(server_address_));

	auto last_image_payload_recd_time = high_resolution_clock::now();

	while (image_bytes_left_to_process > 0) {

		if (received_server_msgs_queue_.empty()) {
			if (HasRequestTimedOut(last_image_payload_recd_time, IMG_PAYLOAD_RECV_TIMEOUT_MILLIS)) {
				response_code = SendMissingSeqNumbersToServer(image_payload_seq_map, expected_no_of_payloads, 
					missing_payload_seq_nums, server_inactive_count);
				last_image_payload_recd_time = high_resolution_clock::now();
				if (response_code == FAILURE_RESPONSE) {
					msg_logger_->LogError("Error while sending response to server on timeout.");
					return FAILURE_RESPONSE;
				}
			}
			continue;
		}

		mtx_.lock();
		image_data_from_server = received_server_msgs_queue_.front();
		received_server_msgs_queue_.pop();
		mtx_.unlock();

		vector<string> split_image_data_payload = SplitString(&image_data_from_server[0], SERVER_RESPONSE_DELIMITER, NUM_OF_IMG_DATA_PARAMS,
			image_data_from_server.length());

		msg_logger_->LogDebug("Split image payload size: " + to_string((ushort)split_image_data_payload.size()));

		u_int payload_seq_num = 0, image_payload_size = 0;

		response_code = ValidateImageDataPayload(split_image_data_payload, payload_seq_num, image_payload_size);
		if (response_code == FAILURE_RESPONSE) {
			msg_logger_->LogError("ERROR: Validation failed for image data payload from server.");
			return response_code;
		}

		msg_logger_->LogDebug("Image data after splitting: " + split_image_data_payload.at(0) + " | "
			+ split_image_data_payload.at(1) + " | " + split_image_data_payload.at(2) + " | "
			+ split_image_data_payload.at(3) + " | Length of image data: " + to_string((ushort)split_image_data_payload.at(4).length()));

		//Add to payload map
		image_payload_seq_map[payload_seq_num] = split_image_data_payload.at(4);

		image_bytes_processed += image_payload_size;
		image_bytes_left_to_process -= image_payload_size;

		msg_logger_->LogDebug("Image bytes recd: " + to_string(image_bytes_processed)
			+ " | Image bytes left to receive: " + to_string(image_bytes_left_to_process));

		last_image_payload_recd_time = high_resolution_clock::now();
	}

	ImageProcessor modified_image_processor(image_payload_seq_map, image_dimensions, image_file_size);
	image_processor = modified_image_processor;

	return SUCCESS_RESPONSE;
}

/*
This functions consumes a server message from the queue, splits the raw message by 'space'
character and sends the resulting vector for validation.
*/
short UDPClient::ReceiveAndValidateServerResponse(short& server_response_code)
{
	string server_response = EMPTY_STRING;

	short response_code = ConsumeServerMsgFromQueue(server_response);
	if (response_code == FAILURE_RESPONSE) {
		msg_logger_->LogError("ERROR: Did not receive response from server.");
		return FAILURE_RESPONSE;
	}

	msg_logger_->LogError("Received server response: " + server_response);

	vector<string> server_response_split = SplitString(&server_response[0], SERVER_RESPONSE_DELIMITER);
	return ValidateServerResponse(server_response_split, server_response_code);
}

/*
Consumes the raw image metadata string received from server, splits it by 'space' character
and forwards the resulting vector for validation.
*/
short UDPClient::ReceiveAndValidateImageMetadata(cv::Size& image_dimensions, uint& image_file_size)
{
	string server_msg = EMPTY_STRING;

	short response_code = ConsumeServerMsgFromQueue(server_msg);

	if (response_code == FAILURE_RESPONSE) {
		msg_logger_->LogError("ERROR: Could not receive image metadata from server.");
		return FAILURE_RESPONSE;
	}

	msg_logger_->LogError("Received image metadata from server: " + server_msg);

	vector<string> server_msg_split = SplitString(&server_msg[0], SERVER_RESPONSE_DELIMITER);
	return ValidateImageMetadataFromServer(server_msg_split, image_dimensions, image_file_size);
}