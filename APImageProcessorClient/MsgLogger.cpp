#include "MsgLogger.h"

#include<iostream>

using std::endl;
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
	_mtx.lock();
	_logFile << "Thread: " << get_id() << " | " << msg << endl;
	_mtx.unlock();
}

void MsgLogger::LogError(const string& msg)
{
	_mtx.lock();
	_logFile << "Thread: " << get_id() << " | " << msg << endl;
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
