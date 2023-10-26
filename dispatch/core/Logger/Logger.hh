#pragma once
#include <string>
#include <fstream>
#include "dispatch/core/Logger/SeverityLevel.hh"

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
				static std::ofstream* file;
		public:
			static void printfln(const char *format, ...);
			static void println(const std::string& msg);
			static void setLogTag(std::string logTag);
			static std::string getLastLogMessage();

			static void log(const SeverityLevel SeverityLevel, const std::string& message);
			static void log(const SeverityLevel severityLevel, const char* format, ...);
			static void log(const char* format, ...);
			static void logInfo(const char* format, ...);
			static void logWarning(const char* format, ...);
			static void logError(const char* format, ...);
			static void logFatal(const char* format, ...);

			static std::string severityLevelToString(const SeverityLevel level);


			// // Returns true if log file was created successfully.
			// static bool enableLoggingToFile(const std::string& path);
			// static void disableLoggingToFile();
	};
}