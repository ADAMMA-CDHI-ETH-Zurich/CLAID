#pragma once

namespace claid
{
    class Runnable
    {
        private:
            bool valid = true;
        public:
            virtual ~Runnable() {}

            virtual void run() = 0;
            bool deleteAfterRun = false;
            bool isValid()
            {
                return this->valid;
            }
    };
}