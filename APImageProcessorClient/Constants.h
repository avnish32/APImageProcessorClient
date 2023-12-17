#pragma once
#include<iostream>

using namespace std;

const int MAX_RETRY_COUNT = 3;
const string SERVER_IP_ADDRESS = "127.0.0.1";
const long SERVER_PORT = 8080;

const short RESPONSE_SUCCESS = 0;
const short RESPONSE_FAILURE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;

const string SIZE_PAYLOAD_PREFIX = "Size ";