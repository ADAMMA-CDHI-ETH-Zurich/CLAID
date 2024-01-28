from dispatch.proto.claidservice_pb2 import LogMessageSeverityLevel

from datetime import datetime
import sys


class Logger:
    @staticmethod
    def log(level, message):
        output = f"[{Logger.get_time_string()} | CLAID - {LogMessageSeverityLevel.Name(level)}] {message}\n"

        if level in {LogMessageSeverityLevel.ERROR, LogMessageSeverityLevel.FATAL}:
            print(output, file=sys.stderr)
        else:
            print(output)

    @staticmethod
    def log_debug(message):
        Logger.log(LogMessageSeverityLevel.DEBUG, message)

    @staticmethod
    def log_info(message):
        Logger.log(LogMessageSeverityLevel.INFO, message)

    @staticmethod
    def log_warning(message):
        Logger.log(LogMessageSeverityLevel.WARNING, message)

    @staticmethod
    def log_error(message):
        Logger.log(LogMessageSeverityLevel.ERROR, message)

    @staticmethod
    def log_fatal(message):
        Logger.log(LogMessageSeverityLevel.FATAL, message)

    @staticmethod
    def get_time_string():
        # Get the current date and time
        current_datetime = datetime.now()

        # Define the desired format
        formatter = "%d.%m.%Y - %H:%M:%S"

        # Format the current date and time using the formatter
        formatted_datetime = current_datetime.strftime(formatter)
        return formatted_datetime
