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


#pragma once

namespace claid
{
    class Runnable
    {
        private:
            bool valid = true;
            // If true, the RunnableDispatcherThread catches any exception thrown in the run() function.
            // The exception message is saved.
            bool exceptionThrown = false;
            std::string exceptionMessage;
        public:
            virtual ~Runnable() 
            {
                this->invalidate();
            }

            virtual void run() = 0;
            bool wasExecuted = false;
            bool catchExceptions = false;
            bool stopDispatcherAfterThisRunnable = false;
            
            void invalidate()
            {
                this->valid = false;
            }

            bool isValid() const
            {
                return this->valid;
            }

            bool wasExceptionThrown() const
            {
                return this->exceptionThrown;
            }

            const std::string& getExceptionMessage() const
            {
                return this->exceptionMessage;
            }

            void setException(const std::string& exceptionMessage)
            {
                this->exceptionMessage = exceptionMessage;
                this->exceptionThrown = true;
            }
            
    };
}