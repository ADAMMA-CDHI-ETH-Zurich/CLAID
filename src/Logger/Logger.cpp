#include "Logger/Logger.hpp"

#include <sstream>
#include <stdarg.h>
#include <cstdio>
#include <iostream>
#include <ctime>

#include <string>
#include <stdexcept>
#include <mutex>
std::string claid::Logger::logTag = "claid";
std::string claid::Logger::lastLogMessage = "";
bool claid::Logger::loggingToFileEnabled = false;
std::ofstream* claid::Logger::file = nullptr;
std::mutex fileAccessMutex;

#ifdef __ANDROID__
	#include <android/log.h>
	#define  LOGCAT(...)  __android_log_print(ANDROID_LOG_INFO,claid::Logger::logTag.c_str(),__VA_ARGS__)
#endif

/**
 * @brief prints a new line to the console
 *
 * Takes a format c-string and a variable amount of parameters
 * and formats an string accordingly.
 * This string will be printed prefixed with the current time
 * and the specified logTag.
 *
 * @param const char* format c-string containing the string which
 * shall be printed. May contain conversion specifiers like %d, %f etc.
 *
 * @param ... variable of amount of parameters used to format the
 * conversion specifiers.
 */
void claid::Logger::printfln(const char *format, ...)
{
	va_list vargs;

	va_start(vargs, format);


	// Size 500 was too small.
	const int BUFF_SIZE = 10240;
    char buffer[BUFF_SIZE] = "";
	// If number of character is > BUFF_SIZE, the remaining characters will not be written
	// by using vsnprintf (in contrast to vsprintf). Therefore, should not yield a segfault.
	std::vsnprintf(buffer, BUFF_SIZE, format, vargs);


	va_end(vargs);
	std::stringstream ss;
	std::string timeString = claid::Logger::getTimeString();

	ss << "[" << claid::Logger::logTag << " | "
			<< timeString << "] " << buffer;

	// This is not thread safe....
	// Leads to segfault if multiple threads access it at the same time.
	//claid::Logger::lastLogMessage = ss.str().c_str();
	
	#ifdef __ANDROID__
		LOGCAT(ss.str().c_str(), __LINE__);
	#else
		std::cout << ss.str().c_str() << "\n" << std::flush;
	#endif

	if(claid::Logger::loggingToFileEnabled && claid::Logger::file != nullptr)
	{
		fileAccessMutex.lock();
		(*file) << ss.str() << "\n";
		file->flush();
		fileAccessMutex.unlock();
	}
}

/**
 * @brief In-class helper function to get a string containing the current time.
 *
 * Class intern helper function. It provides a string containing
 * the current time as follows: hh:mm::ss
 * hh being the hour
 * mm being the minute
 * ss being the second
 *
 * @param std::string *timeStr pointer to a string to wich the
 * time string will be written to.
 *
 * @return void.
 */
std::string claid::Logger::getTimeString()
{
	time_t t = time(0);
	struct tm * now = localtime(&t);

	std::stringstream ss;

	if (now->tm_hour < 10)
		ss << "0" << now->tm_hour << ":";
	else
		ss << now->tm_hour << ":";

	if (now->tm_min < 10)
		ss << "0" << now->tm_min << ":";
	else
		ss << now->tm_min << ":";

	if (now->tm_sec < 10)
		ss << "0" << now->tm_sec;
	else
		ss << now->tm_sec;

	return ss.str();
}

/**
 * @brief sets the logTag of the logger
 *
 * The logTag is a prefix which will be printed when using printfln
 * after the time string.
 *
 * @param std::string logTag string containing the logTag
 *
 * @return void
 */
void claid::Logger::setLogTag(std::string logTag)
{
	claid::Logger::logTag = logTag;
}

/**
 * @brief Returns last log mesage.
 *
 * Returns the last message that was printed.
 * 
 * @return std::string Last log message that was printed.
 */
std::string claid::Logger::getLastLogMessage()
{
	return claid::Logger::lastLogMessage;
}


bool claid::Logger::enableLoggingToFile(const std::string& path)
{
	// If logging is already enabled, first disable it (i.e., closing current log file).
	if(claid::Logger::file != nullptr)
	{
		claid::Logger::disableLoggingToFile();
	}

	claid::Logger::file = new std::ofstream(path);

	if(!file->is_open())
	{
		delete file;
		return false;
	}
	
	claid::Logger::loggingToFileEnabled = true;
	return true;
}

void claid::Logger::disableLoggingToFile()
{
	if(claid::Logger::file != nullptr)
	{
		file->close();
		delete file;
	}

	claid::Logger::loggingToFileEnabled = false;
}