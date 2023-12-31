#include "MsgLogger.h"

#include<iostream>
#include<format>
#include<chrono>

using std::endl;
using std::format;
using std::chrono::system_clock;
using std::this_thread::get_id;

MsgLogger::MsgLogger()
{
	_mtx.lock();
	_logFile.open("logFile.txt", std::ios_base::app);
	_mtx.unlock();
}

MsgLogger::~MsgLogger()
{
	_mtx.lock();
	_logFile.close();
	_mtx.unlock();
}

MsgLogger* MsgLogger::GetInstance()
{
	if (_loggerInstance == nullptr) {
		_loggerInstance = new MsgLogger();
	}
	return _loggerInstance;
}

void MsgLogger::LogDebug(const string& msg)
{
	//Below snippet to convert clock time to string taken from https://stackoverflow.com/a/52729233
	std::string timestamp = format("{:%Y-%m-%d %H:%M:%S}", system_clock::now());
	_mtx.lock();
	_logFile << timestamp << " | Thread ID: " << get_id() << " | " << msg << endl;
	_mtx.unlock();
}

void MsgLogger::LogError(const string& msg)
{
	//Below snippet to convert clock time to string taken from https://stackoverflow.com/a/52729233
	std::string timestamp = format("{:%Y-%m-%d %H:%M:%S}", system_clock::now());
	_mtx.lock();
	_logFile << timestamp << " | Thread ID: " << get_id() << " | " << msg << endl;
	std::cout << "\n" << msg;
	_mtx.unlock();
}

void MsgLogger::ReleaseInstance()
{
	if (_loggerInstance != nullptr) {
		delete _loggerInstance;
		_loggerInstance = nullptr;
	}
}
