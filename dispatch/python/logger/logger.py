from logger.severity_level import SeverityLevel

from datetime import datetime
import sys


class Logger:
    @staticmethod
    def log(level, message):
        output = f"[{Logger.get_time_string()} | CLAID - {level.value}] {message}\n"

        if level in {SeverityLevel.ERROR, SeverityLevel.FATAL}:
            print(output, file=sys.stderr)
        else:
            print(output)

    @staticmethod
    def log_info(message):
        Logger.log(SeverityLevel.INFO, message)

    @staticmethod
    def log_warning(message):
        Logger.log(SeverityLevel.WARNING, message)

    @staticmethod
    def log_error(message):
        Logger.log(SeverityLevel.ERROR, message)

    @staticmethod
    def log_fatal(message):
        Logger.log(SeverityLevel.FATAL, message)

    @staticmethod
    def get_time_string():
        # Get the current date and time
        current_datetime = datetime.now()

        # Define the desired format
        formatter = "%d.%m.%Y - %H:%M:%S"

        # Format the current date and time using the formatter
        formatted_datetime = current_datetime.strftime(formatter)
        return formatted_datetime
