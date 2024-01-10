#pragma once
#include<iostream>

using std::string;

const short MIN_CMD_LINE_ARGS = 4;
const short MIN_IMG_METADATA_PARAMS_FROM_SERVER = 4;
const short NUM_OF_IMG_DATA_PARAMS = 5;

const string DEFAULT_SERVER_IP_ADDRESS = "127.0.0.1";
const long DEFAULT_SERVER_PORT = 8080;
const string SERVER_URL_VALIDATION_REGEX = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+:[0-9]+";

const short SUCCESS_RESPONSE = 0;
const short FAILURE_RESPONSE = -1;
const short SERVER_POSITIVE_ACK = 10;
const short SERVER_NEGATIVE_ACK = 11;
const short CLIENT_POSITIVE_ACK = 20;
const short CLIENT_NEGATIVE_ACK = 21;

const string SIZE_PAYLOAD_KEY = "SIZE";
const string SEQUENCE_PAYLOAD_KEY = "SEQ";
const string RESPONSE_PAYLOAD_KEY = "RES";

const unsigned short MAX_IMG_DATA_BYTES_IN_PAYLOAD = 60000;
const unsigned short MAX_SERVER_PAYLOAD_SIZE_BYTES = 60025;

const string CLIENT_MSG_DELIMITER = " ";
const char SERVER_RESPONSE_DELIMITER = ' ';
const char STRING_TERMINATING_CHAR = '\0';
const string TERMINATING_CHAR_AS_STRING = "\0";
const string EMPTY_STRING = "";

const short IMG_PAYLOAD_RECV_TIMEOUT_MILLIS = 2000;
const short SERVER_MSG_RECV_TIMEOUT_MILLIS = 5000;
const string ORIGINAL_IMAGE_WINDOW_NAME = "Original Image";
const string FILTERED_IMAGE_WINDOW_NAME = "Image After Applying Filter";
const string MODIFIED_SUFFIX = "_modified";
const string UNDERSCORE = "_";