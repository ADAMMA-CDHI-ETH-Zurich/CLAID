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

package adamma.c4dhi.claid.Logger;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import adamma.c4dhi.claid.LogMessageSeverityLevel;
import adamma.c4dhi.claid.LogMessageEntityType;
import adamma.c4dhi.claid.LogMessage;
import adamma.c4dhi.claid.Runtime;

import adamma.c4dhi.claid_platform_impl.CLAID;

public class Logger {

    public static void log(LogMessageSeverityLevel level, final String message)
    {
        Logger.log(level, message, LogMessageEntityType.MIDDLEWARE, "JAVA_RUNTIME");
    }

    public static void log(LogMessageSeverityLevel level, final String message, LogMessageEntityType entityType, String entityName)
    {
        String output = "[" + getTimeString() + " | CLAID Java - " + level.toString() + "] " + message + "\n";

        if(level == LogMessageSeverityLevel.ERROR || level == LogMessageSeverityLevel.FATAL)
        {
            System.err.println(output);
        }
        else
        {
            System.out.println(output);
        }

        LogMessage.Builder logMessageBuilder = LogMessage.newBuilder();
/*
        if(level.compareTo(CLAID.getLogSinkSeverityLevel()) >= 0)
        {
            // Set values for the LogMessage fields
            logMessageBuilder.setLogMessage(message);
            logMessageBuilder.setSeverityLevel(level);
            logMessageBuilder.setUnixTimestampInMs(System.currentTimeMillis());
            logMessageBuilder.setEntityType(entityType);
            logMessageBuilder.setEntityName(entityName);
            logMessageBuilder.setRuntime(Runtime.RUNTIME_JAVA);

            // Build the LogMessage
            LogMessage logMessage = logMessageBuilder.build();
            CLAID.postLogMessage(logMessage);
        }*/
    }


    public static void logInfo(final String message)
    {
        log(LogMessageSeverityLevel.INFO, message);
    }

    public static void logWarning(final String message)
    {
        log(LogMessageSeverityLevel.WARNING, message);
    }

    public static void logError(final String message)
    {
        log(LogMessageSeverityLevel.ERROR, message);
    }

    public static void logFatal(final String message)
    {
        log(LogMessageSeverityLevel.FATAL, message);
    }


    public static String getTimeString()
    {
         // Get the current date and time
        LocalDateTime currentDateTime = LocalDateTime.now();

        // Define the desired format
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("dd.MM.yyyy - HH:mm:ss");

        // Format the current date and time using the formatter
        String formattedDateTime = currentDateTime.format(formatter);
        return formattedDateTime;
    }
}
