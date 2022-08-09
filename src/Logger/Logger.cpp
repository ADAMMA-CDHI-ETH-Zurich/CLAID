#include "Logger/Logger.hpp"

#include <sstream>
#include <stdarg.h>
#include <cstdio>
#include <iostream>
#include <ctime>

#include <string>
#include <stdexcept>
std::string portaible::Logger::logTag = "portaible";
std::string portaible::Logger::lastLogMessage = "";

#ifdef __ANDROID__
	#include <android/log.h>
	#define  LOG_TAG    "portaible"
	#define  LOGCAT(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

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
void portaible::Logger::printfln(const char *format, ...)
{
	va_list vargs;

	va_start(vargs, format);


    char buffer[500] = "";
	std::vsprintf(buffer, format, vargs);


	va_end(vargs);
	std::stringstream ss;
	std::string timeString;
	portaible::Logger::getTimeString(&timeString);

	ss << "[" << portaible::Logger::logTag << " | "
			<< timeString << "] " << buffer;

	portaible::Logger::lastLogMessage = ss.str().c_str();
	#ifdef __ANDROID__

		LOGCAT(ss.str().c_str(), __LINE__);
	#else
		std::cout << ss.str().c_str() << "\n" << std::flush;
	#endif

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
void portaible::Logger::getTimeString(std::string *timeStr)
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

	*timeStr = ss.str();
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
void portaible::Logger::setLogTag(std::string logTag)
{
	portaible::Logger::logTag = logTag;
}

/**
 * @brief Returns last log mesage.
 *
 * Returns the last message that was printed.
 * 
 * @return std::string Last log message that was printed.
 */
std::string portaible::Logger::getLastLogMessage()
{
	return portaible::Logger::lastLogMessage;
}