#pragma once

#include <string>
#include <stdint.h>

#include "ChannelBufferBase.hpp"
namespace claid
{
    class ChannelBase
    {
        public:
            virtual intptr_t getChannelDataTypeUniqueIdentifier() = 0;
            virtual std::string getChannelDataTypeName() = 0;

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

            virtual size_t getNumPublishers() = 0;
            virtual size_t getNumSubscribers() = 0;

            void writeBinaryData(const BinaryData& data);
            void readBinaryData(BinaryData& data);

            bool isTyped() const
            {
                return this->channelBuffer->isTyped();
            }


        protected:
            std::vector<ChannelSubscriberBase*> channelSubscribers;
            std::string channelID;        

            size_t numPublishers = 0;
            size_t numSubscribers = 0;

            ChannelBufferBase* channelBuffer;
    };
}