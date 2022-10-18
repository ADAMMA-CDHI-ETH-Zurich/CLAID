#pragma once

#include "ChannelData.hpp"
#include "Binary/BinaryData.hpp"
#include "Exception/Exception.hpp"

#include <mutex>


namespace claid
{
   
    class ChannelDataBase;
    template<typename T>
    class ChannelData;

    struct ChannelBufferElement
    {
        // ChannelData is used by typed channels,
        // BinaryData is used by untyped channels.
        // They can be converted to each other, as ChannelData
        // needs to be binary de-/serializable.
        // So, why store both in one struct then, isn't it overhead?
        // Typed channels also allow to read binary data. Whenever
        // binary data shall be read from a channel, the data at the given
        // timestamp needs to be serialized. Thus, if multiple different processes
        // want to read the binary data, it always would need to be serialized again.
        // Thus, whenever the corresponding data of the typed channel is serialized,
        // we store the serialized data aswell.

        protected:
            TaggedDataBase header;
            TaggedData<BinaryData> binaryData;

            // If data is available at all (either binary or typed)
            bool dataAvailable = false;

            // If binary data is available (either set directly or typed data has been serialized).
            bool binaryDataAvailable = false;
            std::mutex mutex;


        public:


            ChannelBufferElement() : dataAvailable(false)
            {

            }

            ChannelBufferElement(TaggedData<BinaryData> binaryData) : binaryData(binaryData), dataAvailable(true), binaryDataAvailable(true)
            {
                this->header = binaryData.getHeader();
            }
            
            virtual ~ChannelBufferElement()
            {
            }

            void lock()
            {
                this->mutex.lock();
            }

            void unlock()
            {
                this->mutex.unlock();
            }

            TaggedDataBase getHeader()
            {
                return this->header;
            }

            virtual TaggedData<BinaryData> getBinaryData()
            {
                if(!this->dataAvailable)
                {
                    CLAID_THROW(Exception, "Error! Tried to get binary data from ChannelBufferElement (untyped), but no data was never set (no data available).");
                }

                return this->binaryData;
            }

            bool isDataAvailable() const
            {
                return this->dataAvailable;
            }





    };
}
