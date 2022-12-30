#pragma once

#include "TaggedData.hpp"
#include "Untyped.hpp"
#include "Binary/BinaryData.hpp"
#include "Channel/ChannelBufferElement.hpp"



#include "Binary/BinarySerializer.hpp"
#include "XML/XMLSerializer.hpp"
#include "Reflection/Reflect.hpp"

namespace claid
{
    // Forward declaration
 
    struct ChannelBufferElement;

    // Not a DataBase, its just the base class for ChannelData ;)
    class ChannelDataBase
    {
        protected:
            bool valid;

            

            ChannelDataBase(bool valid) : valid(valid)
            {

            }

        public:
            virtual ~ChannelDataBase()
            {
                
            }

            bool isValid() const
            {
                return this->valid;
            }

            virtual TaggedData<BinaryData> getBinaryData() = 0;

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

            // The ChannelBufferElement that our data belongs to.
            // This makes sure that the ChannelBufferElement is not deleted as long as an instance
            // of ChannelData still holds it.
            // Can also be used to retrieve binaryData.
            std::shared_ptr<ChannelBufferElement> channelBufferElement;   

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

            ChannelData(TaggedDataBase header, 
                std::shared_ptr<ChannelBufferElement> channelBufferElement) : ChannelDataBase(true), header(header), channelBufferElement(channelBufferElement)
            {
            }

            const Time& getTimestamp() const
            {
                return this->header.timestamp;
            }

            const uint64_t getSequenceID()
            {
                return this->header.sequenceID;
            }

            TaggedData<BinaryData> getBinaryData() 
            {
                return channelBufferElement->getBinaryData();
            }

            virtual TaggedDataBase getHeader()
            {
                return this->header;
            }

     
        // ChannelData(ChannelBuffer<T>* holderBuffer, TaggedData<T>& taggedData) : ChannelDataBase(true), holderBuffer(holderBuffer), taggedData(taggedData)
        // {
        //     holderBuffer->serialize();
        // }
    };

    // Template specialization
    template<typename T>
    class ChannelData : public ChannelDataBase
    {
        private:
            TaggedData<T> taggedData;
            const TaggedData<T>& internalValue() const
            {
                return taggedData;
            }

            // The ChannelBufferElement that our data belongs to.
            // This makes sure that the ChannelBufferElement is not deleted as long as an instance
            // of ChannelData still holds it.
            // Can also be used to retrieve binaryData.
            std::shared_ptr<ChannelBufferElement> channelBufferElement;   

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


            ChannelData(TaggedData<T>& taggedData, 
                std::shared_ptr<ChannelBufferElement> channelBufferElement) : ChannelDataBase(true), taggedData(taggedData), channelBufferElement(channelBufferElement)
            {
            }

            Reflect(ChannelData,
                reflectMember(taggedData);
            )


            TaggedData<BinaryData> getBinaryData() 
            {
                return channelBufferElement->getBinaryData();
            }

            TaggedDataBase getHeader()
            {
                return *static_cast<TaggedDataBase*>(&taggedData);
            }

            const Time& getTimestamp() const
            {
                return this->taggedData.timestamp;
            }

            const uint64_t getSequenceID()
            {
                return this->taggedData.sequenceID;
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

            std::shared_ptr<XMLNode> toXML()
            {
                if(!this->valid)
                {
                    CLAID_THROW(Exception, "Error, cannot serialize ChannelData to XML, data not valid");
                }
                XMLSerializer serializer;
                serializer.serialize(*this);

                return serializer.getXMLNode();
            }


            
    };
}