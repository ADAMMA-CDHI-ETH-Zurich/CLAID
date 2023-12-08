package adamma.c4dhi.claid.Logger;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Logger {
    public static void log(SeverityLevel level, final String message)
    {
        String output = "[" + getTimeString() + " | CLAID - " + level.toString() + "] " + message + "\n";

        if(level == SeverityLevel.ERROR || level == SeverityLevel.FATAL)
        {
            System.err.println(output);
        }
        else
        {
            System.out.println(output);
        }
    }

    public static void logInfo(final String message)
    {
        log(SeverityLevel.INFO, message);
    }

    public static void logWarning(final String message)
    {
        log(SeverityLevel.WARNING, message);
    }

    public static void logError(final String message)
    {
        log(SeverityLevel.ERROR, message);
    }

    public static void logFatal(final String message)
    {
        log(SeverityLevel.FATAL, message);
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
