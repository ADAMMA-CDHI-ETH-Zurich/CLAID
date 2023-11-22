#pragma once

#include "dispatch/core/Utilities/Time.hh"

namespace claid{
    
    template<typename T>
    struct ChannelData
    {
    private:
            std::shared_ptr<T> data;
            Time timestamp;
            std::string userId;

            bool valid = false;
    public:

        ChannelData() : valid(false)
        {

        }

        ChannelData(std::shared_ptr<T> data, 
            const Time& timestamp, const std::string& userId) : data(data), timestamp(timestamp), userId(userId), valid(true)
        {

        }

        static ChannelData fromCopy(const T& data, const Time& timestamp, const std::string& userId)
        {
            std::shared_ptr<T> sharedDataCopy = std::make_shared<T>(data);

            return ChannelData(sharedDataCopy, timestamp, userId);
        }


        const T& getData()
        {
            return *this->data.get();
        }

        const Time& getTimestamp()
        {
            return this->timestamp;
        }

        const std::string& getUserId()
        {
            return this->userId;
        }

        bool isValid() const
        {
            return this->valid;
        }

    };
}