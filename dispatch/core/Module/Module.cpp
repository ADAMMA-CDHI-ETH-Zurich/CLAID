/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
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
// For remaining implementations of Module, see Module_impl.hpp
// (was done in separate _impl file, in order to solve cycle dependencies..)

#include "RunTime/RunTime.hpp"

namespace claid
{
    // Somehow compiler does not like the constructor in the Module_imp .. soooo
    Module::Module()
    {
    }


    void Module::moduleError(const std::string& error) {
        std::string errorMsg = "Module \"" + id + "\": " + error;
        Logger::log(SeverityLevel::ERROR, errorMsg);
    }

    void Module::moduleWarning(const std::string& warning) {
        std::string warningMsg = "Module \"" + id + "\": " + warning;
        Logger::log(SeverityLevel::WARNING, warningMsg);
    }

    bool Module::start(ChannelSubscriberPublisher* subscriberPublisher, const std::map<std::string, std::string>& properties) 
    {
        if (isInitialized) {
            moduleError("Initialize called twice!");
            return false;
        }

        this->subscriberPublisher = subscriberPublisher;

        if (!runnableDispatcher.start()) {
            moduleError("Failed to start RunnableDispatcher.");
            return false;
        }

        isInitializing = true;

        runnableDispatcher.addRunnable(FunctionRunnable([this, properties] { initializeInternal(properties); }));

        while (!isInitialized) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        isInitializing = false;
        subscriberPublisher = nullptr;
        return true;
    }

    void Module::setId(const std::string& id) {
        this->id = id;
    }

    std::string Module::getId() {
        return id;
    }

    void Module::enqueueRunnable(const ScheduledRunnable& runnable) {
        runnableDispatcher.addRunnable(runnable);
    }
}

