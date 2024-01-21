#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/core/shared_queue.hh"

#include <sstream>
#include <stdarg.h>
#include <cstdio>
#include <iostream>
#include <ctime>

#include <string>
#include <stdexcept>
#include <mutex>
std::string claid::Logger::logTag = "CLAID C++";
std::string claid::Logger::lastLogMessage = "";
bool claid::Logger::loggingToFileEnabled = false;
std::string claid::Logger::logStoragePath;

// Severity level determing what log messages are print to the console
// and stored to the log file.
LogMessageSeverityLevel claid::Logger::minSeverityLevelToPrintAndStore = LogMessageSeverityLevel::INFO;
// Severity level determining what log messages are forward to the log sink host.
LogMessageSeverityLevel claid::Logger::minSeverityLevelToForwardToLogSinkHost = LogMessageSeverityLevel::WARNING;
std::unique_ptr<std::ofstream> claid::Logger::logFile = nullptr;
std::shared_ptr<claid::SharedQueue<LogMessage>> claid::Logger::logMessageQueue = nullptr;

std::mutex claid::Logger::loggerMutex;

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
void claid::Logger::logInfo(const char *format, ...)
{
	va_list args, args_copy ;
    va_start( args, format ) ;
    va_copy( args_copy, args ) ;

    const auto sz = std::vsnprintf( nullptr, 0, format, args ) + 1 ;

    try
    {
        std::string result( sz, ' ' ) ;
        std::vsnprintf( &result.front(), sz, format, args_copy ) ;

        va_end(args_copy) ;
        va_end(args) ;

		
		claid::Logger::log(LogMessageSeverityLevel::INFO, result, LogMessageEntityType::MIDDLEWARE, "");
    }

    catch( const std::bad_alloc& )
    {
        va_end(args_copy) ;
        va_end(args) ;
        throw ;
    }
}

void claid::Logger::log(const LogMessageSeverityLevel severityLevel, 
    const std::string& message, const LogMessageEntityType entityType, 
    const std::string entityName)
{

	std::stringstream ss;
	std::string timeString = claid::Logger::getTimeString();

	ss << "[" << timeString << " | " << logTag << " " << LogMessageSeverityLevel_Name(severityLevel) << " | " << LogMessageEntityType_Name(entityType);

    if(entityName != "")
    {
        ss << " (" << entityName << ")";
    }

	ss << "] " << message;

    {
        std::unique_lock<std::mutex> lock(loggerMutex);
        if(severityLevel >= Logger::minSeverityLevelToPrintAndStore)
        {
            #ifdef __ANDROID__
                LOGCAT(ss.str().c_str(), __LINE__);
            #else
                std::cout << ss.str().c_str() << "\n" << std::flush;
            #endif
        }
    }

    
	

    std::shared_ptr<LogMessage> logMessage = std::make_shared<LogMessage>();
    logMessage->set_log_message(message);
    logMessage->set_severity_level(severityLevel);
    logMessage->set_unix_timestamp_in_ms(Time::now().toUnixTimestampMilliseconds());
    logMessage->set_entity_type(entityType);
    logMessage->set_entity_name(entityName);
    logMessage->set_runtime(Runtime::RUNTIME_CPP);

    if(severityLevel >= Logger::minSeverityLevelToForwardToLogSinkHost)
    {
        if(Logger::logMessageQueue != nullptr)
        {
            std::unique_lock<std::mutex> lock(claid::Logger::loggerMutex);

            Logger::logMessageQueue->push_back(logMessage);
        }
    }
}

void claid::Logger::log(const LogMessageSeverityLevel severityLevel, const char* format, ...)
{
	va_list args, args_copy ;
    va_start( args, format ) ;
    va_copy( args_copy, args ) ;
	
    const auto sz = std::vsnprintf( nullptr, 0, format, args ) + 1 ;

    try
    {
        std::string result( sz, ' ' ) ;
        std::vsnprintf( &result.front(), sz, format, args_copy ) ;

		claid::Logger::log(severityLevel, result, LogMessageEntityType::MIDDLEWARE, "");

		// This is not thread safe....
		// Leads to segfault if multiple threads access it at the same time.
		//claid::Logger::lastLogMessage = ss.str().c_str();

        va_end(args_copy) ;
        va_end(args) ;		
    }

    catch( const std::bad_alloc& )
    {
        va_end(args_copy) ;
        va_end(args) ;
        throw ;
    }
}

void claid::Logger::logWarning(const char* format, ...)
{
	va_list args, args_copy ;
    va_start( args, format ) ;
    va_copy( args_copy, args ) ;
	
    const auto sz = std::vsnprintf( nullptr, 0, format, args ) + 1 ;

    try
    {
        std::string result( sz, ' ' ) ;
        std::vsnprintf( &result.front(), sz, format, args_copy ) ;

		claid::Logger::log(LogMessageSeverityLevel::WARNING, result, LogMessageEntityType::MIDDLEWARE, "");

        va_end(args_copy) ;
        va_end(args) ;
    }

    catch( const std::bad_alloc& )
    {
        va_end(args_copy) ;
        va_end(args) ;
        throw ;
    }
}

void claid::Logger::logError(const char* format, ...)
{
	va_list args, args_copy ;
    va_start( args, format ) ;
    va_copy( args_copy, args ) ;
	
    const auto sz = std::vsnprintf( nullptr, 0, format, args ) + 1 ;

    try
    {
        std::string result( sz, ' ' ) ;
        std::vsnprintf( &result.front(), sz, format, args_copy ) ;

		claid::Logger::log(LogMessageSeverityLevel::ERROR, result, LogMessageEntityType::MIDDLEWARE, "");

        va_end(args_copy) ;
        va_end(args) ;
    }

    catch( const std::bad_alloc& )
    {
        va_end(args_copy) ;
        va_end(args) ;
        throw ;
    }
}

void claid::Logger::logFatal(const char* format, ...)
{
	va_list args, args_copy ;
    va_start( args, format ) ;
    va_copy( args_copy, args ) ;
	
    const auto sz = std::vsnprintf( nullptr, 0, format, args ) + 1 ;

    try
    {
        std::string result( sz, ' ' ) ;
        std::vsnprintf( &result.front(), sz, format, args_copy ) ;

		claid::Logger::log(LogMessageSeverityLevel::FATAL, result, LogMessageEntityType::MIDDLEWARE, "");

        va_end(args_copy) ;
        va_end(args) ;
    }

    catch( const std::bad_alloc& )
    {
        va_end(args_copy) ;
        va_end(args) ;
        throw ;
    }
}


void claid::Logger::println(const std::string& msg) {
	claid::Logger::logInfo("%s", msg.c_str());
}

/**
 * @brief In-class helper function to get a string containing the current time.
 *
 * Class intern helper function. It provides a string containingprintfln(
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

	ss << now->tm_mday << "." << now->tm_mon + 1 << "." << now->tm_year + 1900 << " ";

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

// /**
//  * @brief sets the logTag of the logger
//  *
//  * The logTag is a prefix which will be printed when using printfln
//  * after the time string.
//  *
//  * @param std::string logTag string containing the logTag
//  *
//  * @return void
//  */
// void claid::Logger::setLogTag(std::string logTag)
// {
// 	claid::Logger::logTag = logTag;
// }

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

void claid::Logger::attachToMiddleware(std::shared_ptr<SharedQueue<LogMessage>> logMessageQueue)
{
    std::unique_lock<std::mutex> lock(loggerMutex);
    claid::Logger::logMessageQueue = logMessageQueue;
}
void claid::Logger::setMinimimSeverityLevelToPrint(const LogMessageSeverityLevel minSeverityLevel)
{
    std::unique_lock<std::mutex> lock(loggerMutex);
    claid::Logger::minSeverityLevelToPrintAndStore = minSeverityLevel;
}


// bool claid::Logger::enableLoggingToFile(const std::string& path)
// {
// 	// If logging is already enabled, first disable it (i.e., closing current log file).
// 	if(claid::Logger::file != nullptr)
// 	{
// 		claid::Logger::disableLoggingToFile();
// 	}

// 	claid::Logger::file = new std::ofstream(path, std::ios::app);

// 	if(!file->is_open())
// 	{
// 		delete file;
// 		return false;
// 	}

// 	claid::Logger::loggingToFileEnabled = true;
// 	return true;
// }

// void claid::Logger::disableLoggingToFile()
// {
// 	if(claid::Logger::file != nullptr)
// 	{
// 		file->close();
// 		delete file;
// 	}

// 	claid::Logger::loggingToFileEnabled = false;
// }