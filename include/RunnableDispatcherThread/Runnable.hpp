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
            virtual ~Runnable() {}

            virtual void run() = 0;
            bool wasExecuted = false;
            bool catchExceptions = false;
            bool stopDispatcherAfterThisRunnable = false;
            
            bool isValid()
            {
                return this->valid;
            }

            bool wasExceptionThrown()
            {
                return this->exceptionThrown;
            }

            const std::string& getExceptionMessage()
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