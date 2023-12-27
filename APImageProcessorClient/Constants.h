#pragma once
#include<iostream>

using std::string;

const short MIN_CMD_LINE_ARGS = 5;

const int MAX_RETRY_COUNT = 3;
const string SERVER_IP_ADDRESS = "127.0.0.1";
const long SERVER_PORT = 8080;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;
const short CLIENT_POSITIVE_ACK = 20;
const short CLIENT_NEGATIVE_ACK = 21;

const string SIZE_PAYLOAD_KEY = "SIZE";
const string SEQUENCE_PAYLOAD_KEY = "SEQ";
const string RESPONSE_PAYLOAD_KEY = "RES";

const unsigned short MAX_SERVER_MSG_PAYLOAD_SIZE_BYTES = 60025;
const string CLIENT_MSG_DELIMITER = " ";
const char SERVER_RESPONSE_DELIMITER = ' ';

const short IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS = 2000;