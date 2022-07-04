#pragma once

namespace portaible
{
    class Runnable
    {
        private:
            bool valid = true;
        public:
            virtual void run() = 0;
            bool deleteAfterRun = false;
            bool isValid()
            {
                return this->valid;
            }
    };
}