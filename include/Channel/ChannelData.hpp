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

        public:
            virtual ~ChannelDataBase()
            {
                
            }

            virtual void getBinaryData(BinaryData& binaryData) = 0;

    };

    template<typename T>
    class ChannelData;

    template<>
    class ChannelData<Untyped> : public ChannelDataBase
    {

        private:
            // Why not store TaggedData<BinaryData> ?
            // Because we want to serialize only when we really need it.
            // If someone subscribed to a channel untyped, maybe he just wants
            // to retrieve information about individual elements but not necessarily
            // serialize them. If the BinaryData is required, it can be retrieved using
            // getBinaryData().
            TaggedDataBase header;         

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

            ChannelData(TaggedDataBase header) : ChannelDataBase(true), header(header)
            {
            }

            

            const Time& getTimestamp() const
            {
                return this->header.timestamp;
            }

            void getBinaryData(BinaryData& binaryData) 
            {
                
            }

            virtual TaggedDataBase getHeader()
            {
                return this->header;
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


            ChannelData(TaggedData<T>& data) : ChannelDataBase(true), data(data)
            {
                holderBuffer->serialize();
            }

            void getBinaryData(BinaryData& binaryData) 
            {
                
            }

            TaggedDataBase getHeader()
            {
                return *static_cast<TaggedDataBase*>(&data);
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