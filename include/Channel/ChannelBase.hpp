#pragma once

#include <string>
#include <unistd.h>

#include "ChannelBufferBase.hpp"

namespace portaible
{
    class ChannelBase
    {
        public:
            virtual intptr_t getChannelDataTypeUniqueIdentifier()
            {
                return 0;
            }

            virtual std::string getChannelDataTypeUniqueIdentifierRTTIString()
            {
                return "";
            }

            ChannelBase()
            {
                
            }

            ChannelBase(std::string channelID) : channelID(channelID)
            {

            }

            virtual ~ChannelBase()
            {

            }

            const std::string& getChannelID() const
            {
                return this->channelID;
            }


        protected:
            std::vector<ChannelSubscriberBase*> channelSubscribers;
            std::string channelID;

            ChannelBufferBase* channelBuffer;
    };
}