#pragma once

#include "TaggedData.hpp"
#include "ChannelBuffer.hpp"
#include "Untyped.hpp"
#include "Binary/BinaryData.hpp"
namespace portaible
{
    // Forward declaration
    template<typename T>
    class ChannelBuffer;

    // Not a DataBase, its just the base class for ChannelData ;)
    class ChannelDataBase
    {
        protected:
            bool valid;

            bool isValid() const
            {
                return this->valid;
            }

            ChannelDataBase(bool valid) : valid(valid)
            {

            }
    };

    template<typename T>
    class ChannelData;

    template<>
    class ChannelData<Untyped> : public ChannelDataBase
    {

        private:
            TaggedData<BinaryData> taggedBinaryData;
        // We hold ? 
        // Binary data ? 

        public:

            static ChannelData InvalidChannelData()
            {
                ChannelData channelData;
                channelData.valid = false;
                return channelData;
            }

            ChannelData() : ChannelDataBase(false)
            {

            }

            ChannelData(TaggedData<BinaryData> taggedBinaryData) : ChannelDataBase(true), taggedBinaryData(taggedBinaryData)
            {
            }


        // ChannelData(ChannelBuffer<T>* holderBuffer, TaggedData<T>& data) : ChannelDataBase(true), holderBuffer(holderBuffer), data(data)
        // {
        //     holderBuffer->serialize();
        // }
    };

    // Template specialization
    template<typename T>
    class ChannelData : public ChannelDataBase
    {
        private:
            TaggedData<T> data;
            const TaggedData<T>& internalValue() const
            {
                return data;
            }



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

            ChannelData() : ChannelDataBase(false)
            {
            }


            ChannelData(ChannelBuffer<T>* holderBuffer, TaggedData<T>& data) : ChannelDataBase(true), holderBuffer(holderBuffer), data(data)
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

            
    };
}