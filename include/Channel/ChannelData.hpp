#pragma once

#include "TaggedData.hpp"
#include "ChannelBuffer.hpp"
namespace portaible
{
    template<typename T>
    class ChannelBuffer;

    template<typename T>
    class ChannelData
    {
        private:
            TaggedData<T> data;
            const TaggedData<T>& internalValue() const
            {
                return data;
            }

            bool valid;


            // The channel buffer that created us.
            // More specifically: We are an entry in the ChannelData[MAX_CHANNEL_BUFFER_SIZE] channelBuffer
            // array of the associated ChannelBuffer.
            ChannelBuffer<T>* holderBuffer;

        public:

            static ChannelData InvalidChannelData()
            {
                ChannelData channelData;
                channelData.valid = false;
                return channelData;
            }

            ChannelData() : valid(false)
            {

            }


            ChannelData(ChannelBuffer<T>* holderBuffer, TaggedData<T>& data) : holderBuffer(holderBuffer), data(data), valid(true)
            {
                holderBuffer->serialize();
            }


            operator const TaggedData<T>&() const 
            { 
                return this->internalValue(); 
            }


            const TaggedData<T>& operator*() const 
            { 
                return this->internalValue(); 
            }


            const TaggedData<T>* operator->() const 
            { 
                return &this->internalValue();
            }

            const Time& getTimestamp() const
            {
                return this->data.timestamp;
            }

            bool isValid() const
            {
                return this->valid;
            }
    };
}