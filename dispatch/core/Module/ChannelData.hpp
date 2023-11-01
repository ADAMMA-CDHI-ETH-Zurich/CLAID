#pragma once

#include "Utilities/Time.hpp"

namespace claid{
    
    template<typename T>
    struct ChannelData
    {
    private:
            std::shared_ptr<T> data;
            const Time timestamp;
            const std::string userId;

    public:

        ChannelData(std::shared_ptr<T> data, 
            const Time& timestamp, const std::string& userId) : data(data), timestamp(timestamp), userId(userId)
        {

        }

        static ChannelData fromCopy(const T& data, const Time& timestamp, const std::string& userId)
        {
            std::shared_ptr<T> sharedDataCopy = std::make_shared<T>(data);

            return ChannelData(data, timestamp, userId);
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

    };
}