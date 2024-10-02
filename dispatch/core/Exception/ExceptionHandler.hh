/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

#include <exception>
#include <cstdlib>
#include <iostream>
#include "dispatch/core/Logger/Logger.hh"

namespace claid {

class ExceptionHandler {
public:
    // Constructor that sets this class as the terminate handler
    static void install() {
        std::set_terminate(ExceptionHandler::handleTerminate);
    }

    // Terminate handler that logs the exception and terminates the program
    static void handleTerminate() {
        std::exception_ptr exptr = std::current_exception();

        if (exptr) {
            try {
                // Re-throw the current exception to get its type
                std::rethrow_exception(exptr);
            } catch (const std::exception& e) {
                // If it's a standard exception, log its message
                Logger::logFatal(e.what());
            } catch (...) {
                // For non-standard exceptions, log a generic message
                Logger::logFatal("Unknown exception caught.");
            }
        } else {
            Logger::logFatal("Terminate called without active exception.");
        }

        // Terminate the program after logging
        std::abort();
    }
};

}