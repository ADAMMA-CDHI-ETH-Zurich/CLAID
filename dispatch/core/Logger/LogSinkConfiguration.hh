#pragma once
#include <string>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"

using claidservice::LogSinkTransferMode;
using claidservice::LogMessageSeverityLevel;
using claidservice::LogMessageEntityType;
using claidservice::LogMessage;
using claidservice::Runtime;

namespace claid
{
    struct LogSinkConfiguration
    {
        std::string logSinkHost;
        std::string logSinkLogStoragePath;

        // Severity level determining what log messages are forward to the log sink host.
        LogMessageSeverityLevel logSinkSeverityLevel = LogMessageSeverityLevel::INFO;
        LogSinkTransferMode logSinkTransferMode = LogSinkTransferMode::STORE_AND_UPLOAD;

        // To be filled out by the middleware.
        std::shared_ptr<SharedQueue<LogMessage>> logSinkQueue = nullptr;

        bool loggingToLogSinkEnabled() const
        {
            return logSinkHost != "";
        }
    };
}