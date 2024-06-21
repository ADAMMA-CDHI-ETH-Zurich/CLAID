/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

import 'package:claid/generated/claidservice.pb.dart';
import 'dart:io';

class Logger {

    static void _logInternal(LogMessageSeverityLevel level, final String message)
    {
        Logger.log(level, message, LogMessageEntityType.MIDDLEWARE, "DART_RUNTIME");
    }

    static void log(LogMessageSeverityLevel level, final String message, LogMessageEntityType entityType, String entityName)
    {
        String output = "[" + getTimeString() + " | CLAID Dart - " + level.toString() + "] " + message + "\n";

        if(level == LogMessageSeverityLevel.ERROR || level == LogMessageSeverityLevel.FATAL)
        {
            print(output);
        }
        else
        {
            stderr.write(output);
        }

        LogMessage logMessage;
/*
        if(level.compareTo(CLAID.getLogSinkSeverityLevel()) >= 0)
        {
            // Set values for the LogMessage fields
            logMessageBuilder.setLogMessage(message);
            logMessageBuilder.setSeverityLevel(level);
            logMessageBuilder.setUnixTimestampInMs(System.currentTimeMillis());
            logMessageBuilder.setEntityType(entityType);
            logMessageBuilder.setEntityName(entityName);
            logMessageBuilder.setRuntime(Runtime.RUNTIME_DART);

            // Build the LogMessage
            LogMessage logMessage = logMessageBuilder.build();
            CLAID.postLogMessage(logMessage);
        }*/
    }


    static void logInfo(final String message)
    {
        _logInternal(LogMessageSeverityLevel.INFO, message);
    }

    static void logWarning(final String message)
    {
        _logInternal(LogMessageSeverityLevel.WARNING, message);
    }

    static void logError(final String message)
    {
        _logInternal(LogMessageSeverityLevel.ERROR, message);
    }

    static void logFatal(final String message)
    {
        _logInternal(LogMessageSeverityLevel.FATAL, message);
        throw Exception(message);
    }


    static String getTimeString()
    {
        // Get the current date and time
        DateTime currentDateTime = DateTime.now();

        // Format the current date and time
        String formattedDateTime = 
            '${currentDateTime.day.toString().padLeft(2, '0')}.'
            '${currentDateTime.month.toString().padLeft(2, '0')}.'
            '${currentDateTime.year} - '
            '${currentDateTime.hour.toString().padLeft(2, '0')}:'
            '${currentDateTime.minute.toString().padLeft(2, '0')}:'
            '${currentDateTime.second.toString().padLeft(2, '0')}';

        return formattedDateTime;
    }
}
