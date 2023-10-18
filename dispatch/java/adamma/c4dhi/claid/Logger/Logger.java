package adamma.c4dhi.claid.Logger;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Logger {
    public static void log(SeverityLevel level, final String message)
    {
        String output = "[" + getTimeString() + " | CLAID - " + level.toString() + "] " + message;
        System.out.println(output);
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
