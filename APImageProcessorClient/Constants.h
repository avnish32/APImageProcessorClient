#pragma once
#include<iostream>

using std::string;

const int MAX_RETRY_COUNT = 3;
const string SERVER_IP_ADDRESS = "127.0.0.1";
const long SERVER_PORT = 8080;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;

const string SIZE_PAYLOAD_KEY = "SIZE";
const string SEQUENCE_PAYLOAD_KEY = "SEQ";
const string RESPONSE_PAYLOAD_KEY = "RES";

const short MAX_SERVER_RESPONSE_PAYLOAD_SIZE_BYTES = 5000;
const char SERVER_RESPONSE_DELIMITER = ' ';