package adamma.c4dhi.claid.Logger;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import adamma.c4dhi.claid.LogMessageSeverityLevel;
import adamma.c4dhi.claid.LogMessageEntityType;

public class Logger {

    public static void log(LogMessageSeverityLevel level, final String message)
    {
        Logger.log(level, message, LogMessageEntityType.MIDDLEWARE, "JAVA_RUNTIME");
    }

    public static void log(LogMessageSeverityLevel level, final String message, LogMessageEntityType entityType, String entityName)
    {
        String output = "[" + getTimeString() + " | CLAID - " + level.toString() + "] " + message + "\n";

        if(level == LogMessageSeverityLevel.ERROR || level == LogMessageSeverityLevel.FATAL)
        {
            System.err.println(output);
        }
        else
        {
            System.out.println(output);
        }

        LogMessage.Builder logMessageBuilder = LogMessage.newBuilder();

        if(level >= CLAID.logSinkSeverityLevel())
        {
            // Set values for the LogMessage fields
            logMessageBuilder.setLogMessage(message);
            logMessageBuilder.setSeverityLevel(level);
            logMessageBuilder.setUnixTimestampInMs(System.currentTimeMillis());
            logMessageBuilder.setEntityType(entityType);
            logMessageBuilder.setEntityName(entityName);
            logMessageBuilder.setRuntime(LogMessage.Runtime.RUNTIME_JAVA);

            // Build the LogMessage
            LogMessage logMessage = logMessageBuilder.build();
            CLAID.postLogMessage(logMessage);
        }
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
