#pragma once

#include "ChannelData.hpp"
#include "Binary/BinaryData.hpp"

namespace portaible
{
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


        // Store it as pointers? Or others..
        // ChannelData uses TaggedData internally, which stores
        // data as a shared_ptr. Thus, we do not need to care about
        // copying data at this point here.
        // ChannelBuffer provides a view on the data: 
        // ChannelBuffer can either be ChannelBuffer<Untyped> or
        // ChannelBuffer<T>. The untyped one does not know how to 
        // cast ChannelDataBase* into the typed data.
        // Thus, we leave it as pointer here. Only when a channel has been typed
        // (ChannelDataBuffer<Untyped> was replaced by a ChannelBuffer<T>), we can access
        // the typed data, the ChannelBuffer<T> will cast it accordingly.
        // Alternative: Derive from ChannelBufferElement, i.e. 
        // template<typename T>
        // class ChannelBufferElementTyped : public ChannelBufferElement
        // And store typed ChannelBufferElements in the buffer of ChannelBuffer.
        
        ChannelDataBase* channelData = nullptr; // Typed data
        ChannelData<Untyped>* untypedData = nullptr;  // Untyped data
        TaggedData<BinaryData>* binaryData = nullptr;

        void clear()
        {
            deleteIfNotNull(channelData);
            deleteIfNotNull(untypedData);
            deleteIfNotNull(binaryData);
        }

        private:
            template<typename T>
            void deleteIfNotNull(T*& ptr)
            {
                if(ptr != nullptr)
                {
                    delete ptr;
                }
                ptr = nullptr;
            }

    };
}
