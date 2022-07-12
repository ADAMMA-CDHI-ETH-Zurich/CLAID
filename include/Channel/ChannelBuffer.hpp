#pragma once

#include "Untyped.hpp"
#include "TaggedData.hpp"
#include "ChannelData.hpp"
#include "ChannelBufferBase.hpp"
#include "Binary/BinaryData.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#define MAX_CHANNEL_BUFFER_SIZE 20

namespace portaible
{
    
    template<typename T>
    class ChannelBuffer;

    template<>
    class ChannelBuffer<Untyped> : public ChannelBufferBase
    {
        ChannelBuffer() : ChannelBufferBase()
        {
            this->typed = false;
        }

        virtual std::string getDataTypeName()
        {
            return "Untyped";
        }

        ChannelData<Untyped>& getDataByIndex(size_t index)
        {
            return *static_cast<ChannelData<Untyped>*>(this->getElement(index).binaryData);
        }
    };

    template<typename T>
    class ChannelBuffer : public ChannelBufferBase
    {
        private:

         

            ChannelBuffer() : ChannelBufferBase()
            {
                this->typed = true;
            }

            virtual std::string getDataTypeName()
            {
                return getDataTypeRTTIString<T>();
            }

            
            

        public:

            ChannelData<T>& getDataByIndex(size_t index)
            {
                return *static_cast<ChannelData<T>*>(this->getElement(index).channelData);
            }
            

            void insert(TaggedData<T>& data)
            {

                this->lockMutex();

                // Is this thread safe?
                // Yes.. as long as we create copies in getLatest, getClosest and getDataInterval.
                if(this->getElement(this->currentIndex).channelData != nullptr)
                {
                    delete this->getElement(this->currentIndex).channelData;
                }
                this->getElement(this->currentIndex).channelData = new ChannelData<T>(data);

                // Do not convert to binary data / serialize. It might not be needed.

                increaseIndex();
                this->unlockMutex();

            }

            bool getLatest(ChannelData<T>& latest)
            {
                return ChannelBufferBase::getLatest<T>(this, latest);
            }
            // TODO Very likely this contains a bug
            bool getClosest(const Time& timestamp, ChannelData<T>& closest)
            {
                return ChannelBufferBase::getClosest<T>(this, timestamp, closest);
                
            }



            void getDataInterval(const Time& min, const Time& max, std::vector<ChannelData<T> >& channelDataInterval)
            {
                ChannelBufferBase::getDataInterval<T>(this, min, max, channelDataInterval);
            }

            void serialize()
            {

            }
        
    };
}

