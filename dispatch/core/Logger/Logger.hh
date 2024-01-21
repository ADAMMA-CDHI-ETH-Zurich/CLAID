#pragma once
#include <string>
#include <fstream>

#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"


using claidservice::LogMessageSeverityLevel;
using claidservice::LogMessageEntityType;
using claidservice::LogMessage;
using claidservice::Runtime;

namespace claid {

template<class T>
class SharedQueue;
}

namespace claid
{
	/**
	 * @brief  Logger providing Desktop Linux <-> Android platform independent logging
	 *
	 * While compiling, it's checked if the library is built for Android
	 * or any other architecture. The logger class provides platform indepent
	 * logging by using preprocessor directives to only compile those
	 * functions that are needed on the given architecture.
	 * On Linux for example, the logger prints to stdout.
	 * On Android however, the logging library is used and the logger
	 * prints to the logcat output console.
	 */
	class Logger
	{
		private:
				/**
				 * A tag to display in every print.
				 * Every use of printfln looks like this:
				 * [00:38:12 - tag]: Logger print test.
				 * Notice that every print is prefixed with the current time
				 * for convenience.
				 */
				static std::string logTag;

				static std::string lastLogMessage;

				static std::string getTimeString();

				static bool loggingToFileEnabled;
				static std::string logStoragePath;

				// Severity level determing what log messages are print to the console
				// and stored to the log file.
				static LogMessageSeverityLevel minSeverityLevelToPrintAndStore;
				// Severity level determining what log messages are forward to the log sink host.
				static LogMessageSeverityLevel minSeverityLevelToForwardToLogSinkHost;

				static std::unique_ptr<std::ofstream> logFile;

				static std::shared_ptr<SharedQueue<LogMessage>> logMessageQueue;

				static std::mutex loggerMutex;

		public:
			static void println(const std::string& msg);
			// static void setLogTag(std::string logTag);
			static std::string getLastLogMessage();

			static void log(const LogMessageSeverityLevel severityLevel, const std::string& message, const LogMessageEntityType entityType, const std::string entityName);
			static void log(const LogMessageSeverityLevel severityLevel, const char* format, ...);
			static void logInfo(const char* format, ...);
			static void logWarning(const char* format, ...);
			static void logError(const char* format, ...);
			static void logFatal(const char* format, ...);

			// All log messages are forward to the middleware, which can then process them separately.
			// Useful to send messages to the log_sink host or to allow individual runtimes to receive all log messages.
			static void attachToMiddleware(std::shared_ptr<SharedQueue<LogMessage>> logMessageQueue);
			static void setMinimimSeverityLevelToPrint(const LogMessageSeverityLevel minSeverityLevel);

			// // Returns true if log file was created successfully.
			// static bool enableLoggingToFile(const std::string& path);
			// static void disableLoggingToFile();
	};
}