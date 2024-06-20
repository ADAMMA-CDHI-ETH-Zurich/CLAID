###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

from dispatch.proto.claidservice_pb2 import LogMessageSeverityLevel, LogMessage, LogMessageEntityType, Runtime

from datetime import datetime
import sys


class Logger:

    # Workaround for that there can be multiple CLAID instances in Python/C++ but only one Logger instance.
    # For now we assume that users will only use one instance of CLAID at a time.
    # For the future, the Logging class needs a redesign.
    claid_instance = None

    log_severity_level_to_print = LogMessageSeverityLevel.INFO

    @staticmethod
    def log(level, message, log_message_entity_type = LogMessageEntityType.MIDDLEWARE, log_message_entity = "PYTHON_RUNTIME"):
        output = f"[{Logger.get_time_string()} | CLAID Python {LogMessageSeverityLevel.Name(level)}] {message}\n"

        if not isinstance(message, str):
            try:
                message = str(message)
            except:
                return

        if level >= Logger.log_severity_level_to_print:

            if level in {LogMessageSeverityLevel.ERROR, LogMessageSeverityLevel.FATAL}:
                print(output, file=sys.stderr)
            else:
                print(output)


        if Logger.claid_instance != None:

            severity_level = Logger.claid_instance.get_log_sink_severity_level()

            if level >= severity_level:
                log_message = LogMessage()

                # Set values for the LogMessage fields
                log_message.log_message = message
                log_message.severity_level = LogMessageSeverityLevel.INFO
                log_message.unix_timestamp_in_ms = int(datetime.now().timestamp() * 1000)
                log_message.entity_type = log_message_entity_type
                log_message.entity_name = log_message_entity
                log_message.runtime = Runtime.RUNTIME_PYTHON


                # Forward the LogMessage to your log sink function
               # Logger.claid_instance.post_log_message(log_message)


    @staticmethod
    def log_debug(message):
        Logger.log(LogMessageSeverityLevel.DEBUG_VERBOSE, message)

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
    

