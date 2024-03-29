/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#pragma once
#include <string>
#include <fstream>

#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Logger/LogSinkConfiguration.hh"


using claidservice::LogSinkTransferMode;
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
			
				static std::mutex loggerMutex;
				// Severity level determing what log messages are print to the console
				// and stored to the log file.
				static LogMessageSeverityLevel minSeverityLevelToPrintAndStore;
				static std::unique_ptr<std::ofstream> logFile;

				static LogSinkConfiguration logSinkConfiguration;
				static void forwardLogMessageToLogSink(std::shared_ptr<LogMessage> logMessage);
				

		public:
			static void println(const std::string& msg);
			// static void setLogTag(std::string logTag);
			static std::string getLastLogMessage();

			static void log(const LogMessageSeverityLevel severityLevel, const std::string& message, const LogMessageEntityType entityType, const std::string entityName);

			static void log(const LogMessageSeverityLevel severityLevel, const char* format, ...);
			static void logDebug(const char* format, ...);
			static void logInfo(const char* format, ...);
			static void logWarning(const char* format, ...);
			static void logError(const char* format, ...);
			static void logFatal(const char* format, ...);

			// All log messages are forward to the middleware, which can then process them separately.
			// Useful to send messages to the log_sink host or to allow individual runtimes to receive all log messages.
			static void setMinimumSeverityLevelToPrint(const LogMessageSeverityLevel minSeverityLevel);
			static LogMessageSeverityLevel getMinimumSeverityLevelToPrint();
			static void setLogSinkConfiguration(const LogSinkConfiguration& logSinkConfiguration);

			// void enableLogSinkTransferModeStoreAndUpload(const std::string& storagePath, LogMessageSeverityLevel minSeverityLevel);
			// void enableLogSinkTransferModeStream(std::shared_ptr<SharedQueue<LogMessage>> logMessageQueue, LogMessageSeverityLevel minSeverityLevel);

			// // Returns true if log file was created successfully.
			// static bool enableLoggingToFile(const std::string& path);
			// static void disableLoggingToFile();
	};
}