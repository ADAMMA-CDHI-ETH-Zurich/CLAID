#include "Channel/ChannelBufferElement.hpp"
#include "Binary/BinaryDeserializer.hpp"
#include "Binary/BinarySerializer.hpp"


namespace portaible
{
    template<typename T>
    class ChannelBufferElementTyped : public ChannelBufferElement
    {
        private:
            TaggedData<T> typedData;

            // If T has no reflect function, then we cannot binary deserialize. However, this is only a problem if
            // data is posted to an Untyped channel by insertBinaryData AND someone else subscribed to that Channel in a typed manner.
            // In other words: If Untyped publish and typed subscribe happens at runtime, then it's a problem.
            // Thus, we throw an exception if the function is called with a type that has no reflect function (see other template specialization below).
            template <typename U = T>
            typename std::enable_if<has_mem_reflect<U>::value || has_non_member_function_reflect<BinaryDeserializer&, U&>::value>::type
            deserializeBinaryDataToTypedData()
            {

                if(!this->dataAvailable)
                {
                    PORTAIBLE_THROW(Exception, "Tried to deserialize binary data of ChannelBufferElement to type \"" << getDataTypeRTTIString<T>() << "\", however no binary data was set.");
                }

                BinaryDeserializer deserializer;
                BinaryDataReader reader(&this->binaryData.value());

                // We create the shared_ptr ourselves, to save a copy operation.
                // If we just used TaggedData<T> (data), with data being of type T,
                // then TaggedData would create the shared_ptr and making a copy of data.

                std::shared_ptr<T> dataPtr(new T);
                deserializer.deserialize(*dataPtr.get(), reader);

                this->typedData = TaggedData<T> (dataPtr, 
                            this->header.timestamp, 
                            this->header.sequenceID);


                           

            }

       
            // See comment above.
            template <typename U = T>
            typename std::enable_if<!has_mem_reflect<U>::value && !has_non_member_function_reflect<BinaryDeserializer&, U&>::value>::type
            deserializeBinaryDataToTypedData()
            {
                PORTAIBLE_THROW(Exception, "Error! Binary data posted to a channel with data type \"" << getDataTypeRTTIString<T>() << "\", that has a typed ChannelBuffer." 
                << "I.e., there is a typed subscriber or publisher available, but untyped data was posted. Cannot deserialize untyped binary data to typed data, " <<
                "as the mentioned data type does not contain a reflect function. Please add reflect function for the data type.");
            }

        public:

            ChannelBufferElementTyped(TaggedData<T> typedData)
            {
                this->typedData = typedData;
                this->header = typedData.getHeader();
            }

            ChannelBufferElementTyped(TaggedData<BinaryData> binaryData) : ChannelBufferElement(binaryData)
            {
                this->deserializeBinaryDataToTypedData();
            }

            TaggedData<T>& getTypedData()
            {
                return this->typedData;
            }

            virtual void serializeIfTyped()
            {
                
                // std::unique_lock<std::mutex> (this->mutex);
                // // TaggedData internally uses shared_ptr.
                // // Thus, we already create it, so we save a copy operation
                // // when creating the TaggedData object below.
                // std::shared_ptr<BinaryData> binaryData(new BinaryData);
                // BinarySerializer serializer;
                // //serializer.serialize(this->data.value(), binaryData.get());

                // // Store the binary data in ChannelBufferElemnt, so that it
                // // is available, if someone else wants to use it.
                // // Therefore, we do not need to serialize it multiple times.
                // this->channelBufferElement->binaryData = 
                //     new TaggedData<BinaryData>(binaryData, 
                //     this->getTimestamp(), 
                //     this->getSequenceID());
            }

    };
}