#include "Channel/ChannelBufferElement.hpp"
#include "Binary/BinaryDeserializer.hpp"
#include "Binary/BinarySerializer.hpp"

#include "XML/XMLSerializer.hpp"

namespace claid
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
            typename std::enable_if<has_mem_reflect<U>::value || has_non_member_function_reflect<BinaryDeserializer&, U&>::value || std::is_arithmetic<U>::value>::type
            deserializeBinaryDataToTypedData()
            {
                std::unique_lock<std::mutex> (this->mutex);

                if(!this->dataAvailable)
                {
                    CLAID_THROW(Exception, "Tried to deserialize binary data of ChannelBufferElement to type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\", however no binary data was set.");
                }

                BinaryDeserializer deserializer;
                std::shared_ptr<BinaryDataReader> reader = std::make_shared<BinaryDataReader>(this->binaryData.value());

                // We create the shared_ptr ourselves, to save a copy operation.
                // If we just used TaggedData<T> (data), with data being of type T,
                // then TaggedData would create the shared_ptr and making a copy of data.

                std::shared_ptr<T> dataPtr(new T());
                deserializer.deserialize(*dataPtr.get(), reader);

                this->typedData = TaggedData<T> (dataPtr, 
                            this->header.timestamp, 
                            this->header.sequenceID);     
            }

       
            // See comment above.
            template <typename U = T>
            typename std::enable_if<!has_mem_reflect<U>::value && !has_non_member_function_reflect<BinaryDeserializer&, U&>::value && !std::is_arithmetic<U>::value>::type
            deserializeBinaryDataToTypedData()
            {
                CLAID_THROW(Exception, "Error! Cannot deserialize untyped binary data posted to a channel with data type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\"" <<
                " as no reflect function is available for the mentioned data type. Please add a reflect function to the datatype.");
            }

            // If T has no reflect function, then we cannot serialize typed data to binary. However, this is only a problem if
            // data is posted to a typed channel, and subscribed to in untyped manner, and binary data is read from the untyped subscriber.
            // If the untyped subscriber never wants to access the binary data, then we don't have a problem.
            // Thus, we throw an exception if the function is called with a type that has no reflect function (see other template specialization below).
            template <typename U = T>
            typename std::enable_if<has_mem_reflect<U>::value || has_non_member_function_reflect<BinaryDeserializer&, U&>::value || std::is_arithmetic<U>::value>::type
            serializeTypedDataToBinaryData()
            {

                // Serialize data if possible.
                std::unique_lock<std::mutex> (this->mutex);
                // TaggedData internally uses shared_ptr.
                // Thus, we already create it, so we save a copy operation
                // when creating the TaggedData object below.
                std::shared_ptr<BinaryData> binaryData(new BinaryData());
                BinarySerializer serializer;
                serializer.serialize(this->typedData.value(), binaryData.get());

                // Store the binary data in ChannelBufferElemnt, so that it
                // is available, if someone else wants to use it.
                // Therefore, we do not need to serialize it multiple times.
                this->binaryData = 
                    TaggedData<BinaryData>(binaryData, 
                    this->header.timestamp, 
                    this->header.sequenceID); 

            }

       
            // See comment above.
            template <typename U = T>
            typename std::enable_if<!has_mem_reflect<U>::value && !has_non_member_function_reflect<BinaryDeserializer&, U&>::value && !std::is_arithmetic<U>::value>::type
            serializeTypedDataToBinaryData()
            {
                CLAID_THROW(Exception, "Error! Cannot serialize typed data to binary data. A subscriber tried to get binary data from a channel of type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\", " <<
                "but typed data could not be serialized, as no reflect function is available for the mentioned data type. Please add a reflect function to the data type.");
            }

            template<typename U = T>
            typename std::enable_if<has_mem_reflect<U>::value || 
                    has_non_member_function_reflect<BinaryDeserializer&, U&>::value ||
                    std::is_arithmetic<U>::value, std::shared_ptr<XMLNode>>::type
            serializeToXML()
            {
                std::unique_lock<std::mutex> (this->mutex);
                XMLSerializer serializer;                
                serializer.serialize("value", this->typedData.value());
                return serializer.getXMLNode();
            }

            template<typename U = T>
            typename std::enable_if<!has_mem_reflect<U>::value && 
                    !has_non_member_function_reflect<BinaryDeserializer&, U&>::value &&
                    !std::is_arithmetic<U>::value, std::shared_ptr<XMLNode>>::type
            serializeToXML()
            {
                CLAID_THROW(Exception, "Error! Cannot serialize typed data to XML. A subscriber tried to get XML data from a channel of type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\", " <<
                "but typed data could not be serialized, as no reflect function is available for the mentioned data type. Please add a reflect function to the data type.");
            }

            

        public:

            ChannelBufferElementTyped() : ChannelBufferElement("")
            {
            }

            ChannelBufferElementTyped(TaggedData<T> typedData) : ChannelBufferElement("")
            {
                this->typedData = typedData;
                this->header = typedData.getHeader();
                this->dataAvailable = true;
                this->binaryDataAvailable = false;

                this->dataTypeName = ClassFactory::getInstance()->getClassNameOfObject(this->typedData.value());
            }

            ChannelBufferElementTyped(TaggedData<BinaryData> binaryData) : ChannelBufferElement(binaryData, "")
            {
                this->deserializeBinaryDataToTypedData();
                this->dataAvailable = true;
                this->binaryDataAvailable = true;
                this->dataTypeName = ClassFactory::getInstance()->getClassNameOfObject(this->typedData.value());
            }

            TaggedData<T> getTypedData()
            {
                return this->typedData;
            }

            TaggedData<BinaryData> getBinaryData()
            {
                std::unique_lock<std::mutex> (this->mutex);
                if(!this->dataAvailable)
                {
                    CLAID_THROW(Exception, "Error! Tried to get binary data from ChannelBufferElement<T> (type \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\"), but no data was never set (no data available).");
                }

                if(!this->binaryDataAvailable)
                {
                    serializeTypedDataToBinaryData();
                    this->binaryDataAvailable = true;
                }
           

                return this->binaryData;
            }

            bool canSerializeToXML() const
            {
                // Can serialize to XML, because 
                // we are a typed ChannelBufferElement.
                return true;
            }

            
            virtual std::shared_ptr<XMLNode> toXML()
            {
                return this->serializeToXML();
            }

            virtual bool applySerializerToData(std::shared_ptr<AbstractSerializer> serializer, bool addHeader = false)
            {
                const T& data = typedData.value();
                std::string dataTypeName = ClassFactory::getInstance()->getClassNameOfObject(data);
                std::string reflectorName = serializer->getReflectorName();

                UntypedReflector* untypedReflector;
                if (!ReflectionManager::getInstance()->getReflectorForClass(dataTypeName, reflectorName, untypedReflector))
                {
                    return false;
                }

                T* nonConstData = const_cast<T*>(&data);
                untypedReflector->invoke(static_cast<void*>(serializer.get()), reinterpret_cast<void*>(nonConstData));               

                if(addHeader)
                {
                    this->addDataHeaderAsMemberUsingSerializer(serializer);
                }

                return true;
            }

   

    };
}