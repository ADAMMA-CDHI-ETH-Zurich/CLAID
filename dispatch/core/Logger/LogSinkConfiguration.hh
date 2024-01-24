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
        std::string logSinkLogStoragePath;
        std::shared_ptr<SharedQueue<LogMessage>> logSinkQueue = nullptr;
        // Severity level determining what log messages are forward to the log sink host.
        LogSinkTransferMode transferMode = LogSinkTransferMode::STORE_AND_UPLOAD;
    };
}