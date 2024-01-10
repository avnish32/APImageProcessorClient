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
	mtx_.lock();
	log_file_.open("logFile.txt", std::ios_base::app);
	mtx_.unlock();
}

MsgLogger::~MsgLogger()
{
	mtx_.lock();
	log_file_.close();
	mtx_.unlock();
}

/*
Static method to enforce the singleton pattern.
*/
MsgLogger* MsgLogger::GetInstance()
{
	if (logger_instance_ == nullptr) {
		logger_instance_ = new MsgLogger();
	}
	return logger_instance_;
}

/*
Function to log a message on debug level.
It writes only to the external log file.
*/
void MsgLogger::LogDebug(const string& msg)
{
	//Below snippet to convert clock time to string taken from https://stackoverflow.com/a/52729233
	std::string timestamp = format("{:%Y-%m-%d %H:%M:%S}", system_clock::now());
	mtx_.lock();
	log_file_ << timestamp << " | Thread ID: " << get_id() << " | " << msg << endl;
	mtx_.unlock();
}

/*
Function to log a message on error level.
It writes to the external log file as well as the console.
*/
void MsgLogger::LogError(const string& msg)
{
	//Below snippet to convert clock time to string taken from https://stackoverflow.com/a/52729233
	std::string timestamp = format("{:%Y-%m-%d %H:%M:%S}", system_clock::now());
	mtx_.lock();
	log_file_ << timestamp << " | Thread ID: " << get_id() << " | " << msg << endl;
	std::cout << "\n" << msg;
	mtx_.unlock();
}

void MsgLogger::ReleaseInstance()
{
	if (logger_instance_ != nullptr) {
		delete logger_instance_;
		logger_instance_ = nullptr;
	}
}
