#pragma once

#include "TaggedData.hpp"
#include "Binary/BinaryData.hpp"
#include "Binary/BinaryDataReader.hpp"
#include "Binary/BinaryDeserializer.hpp"
#include "Exception/Exception.hpp"
#include "ChannelBufferElement.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#define MAX_CHANNEL_BUFFER_SIZE 5

namespace claid
{
    class ChannelBufferBase
    {
        public:
            ChannelBufferBase()
            {
                this->currentIndex = 0;
                this->numElements = 0;

                for(size_t i = 0; i < MAX_CHANNEL_BUFFER_SIZE; i++)
                {
                    this->channelBufferElements[i] = newEmptyChannelBufferElement();
                }
            }

            virtual bool isTyped() const = 0;

            // Might be overriden by typed ChannelBuffer.

            virtual std::shared_ptr<ChannelBufferElement> newEmptyChannelBufferElement()
            {
                std::shared_ptr<ChannelBufferElement> element = std::shared_ptr<ChannelBufferElement>(new ChannelBufferElement(this->dataTypeName));
     
                return element;
            }

            virtual std::shared_ptr<ChannelBufferElement> newChannelBufferElementFromBinaryData(TaggedData<BinaryData>& binaryData)
            {
                std::shared_ptr<ChannelBufferElement> element = std::shared_ptr<ChannelBufferElement>(new ChannelBufferElement(binaryData, this->dataTypeName));
     
                return element;
            }

            

            void getTypeNameFromBinaryData(const BinaryData& binaryData, std::string& typeName) const
            {
                // First bytes of the binary data are the RTTI string identifiying the type.
                BinaryDataReader reader(binaryData);
                reader.readString(typeName);
            }

            

            virtual intptr_t getDataTypeIdentifier() const = 0;

            std::string getDataTypeName() const
            {
                return this->dataTypeName;
            }

            virtual void insertBinaryData(TaggedData<BinaryData> binaryData)
            {
                this->lockMutex();

                Logger::printfln("Inserting binary data (ChannelBufferBase) at index %d %s", this->currentIndex, this->dataTypeName.c_str());
                // Check data types by typename string.
                std::string binaryDataTypeName;
                this->getTypeNameFromBinaryData(binaryData.value(), binaryDataTypeName);
                if(this->getDataTypeName() != "Untyped")
                {
                    if(binaryDataTypeName != this->getDataTypeName())
                    {
                        // ERROR!
                        CLAID_THROW(Exception, "Error, tried to insert binary data to channel buffer. Previously, data of type \"" <<
                        this->getDataTypeName() << "\" has been inserted into the buffer, but now it was tried to insert data of type \"" << binaryDataTypeName << "\"."
                        << "Only binary data of ONE type should ever be added to the buffer.")
                    }
                }
                else
                {
                    // data type name is untyped, which is only the case if never any
                    // binary data has been written to the buffer.
                    // Now that we want to insert some binary data, we can get it's data type name
                    // (which is serialized into the data), and set our data type name accordingly.
                    // This makes sure that all the binary data inserted has the same type.
                    // Otherwise, it would be possible to insert binary data of different types, e.g string, float, int.
                    // So, for the first time binary data is inserted, we store it's data type name.
                    // This does NOT mean, that we type the channel. It only means that we store the data type name of the untyped
                    // binary data for the future, so that we can make sure that the user does not insert data of different random types
                    // into the buffer. Even the buffer is untyped, only binary data of one type should be added to it.
                    // Otherwise, as soon as we type the channel/buffer (e.g. in ChannelManager), we definitely WILL get errors.
                    this->dataTypeName = binaryDataTypeName;
                }
                



                // Is this thread safe?
                // Yes.. as long as we create copies in getLatest, getClosest and getDataInterval.
                // If buffer is typed, binary data automatically get's deserialized in overriden function newChannelBufferElementFromBinaryData in ChannelBuffer<T>.
                // ChannelBuffer<T> uses ChannelBufferElementTyped<T>, which deserializes the data in it's constructor (the constructor that uses binary data).
                this->channelBufferElements[this->currentIndex] = newChannelBufferElementFromBinaryData(binaryData);


                increaseIndex();
                this->unlockMutex();
            }

        protected:
            std::shared_ptr<ChannelBufferElement> channelBufferElements[MAX_CHANNEL_BUFFER_SIZE];
            size_t currentIndex = 0;
            size_t numElements = 0;
            std::string dataTypeName;

            // GlobalIndex means index 0 is oldest element, MAX_CHANNEL_BUFFER_SIZE - 1 is highest element.
            int relativeIndex(int globalIndex)
            {
                // Oldest index is at currentIndex + 1 or 0, if numElements < MAX_CHANNEL_BUFFER_SIZE

                int relative;

                if(numElements < MAX_CHANNEL_BUFFER_SIZE)
                {
                    relative = 0;
                }
                else
                {
                    relative = currentIndex + 1;
                }

                if(relative >= MAX_CHANNEL_BUFFER_SIZE)
                {
                    relative -= MAX_CHANNEL_BUFFER_SIZE;
                }

                relative +=  globalIndex;

                if(relative >= MAX_CHANNEL_BUFFER_SIZE)
                {
                    relative -= MAX_CHANNEL_BUFFER_SIZE;
                }

                return relative;
            }


            std::mutex mutex;

            void lockMutex()
            {
                this->mutex.lock();
            }   

            void unlockMutex()
            {
                this->mutex.unlock();
            }

            
            

   
            void increaseIndex()
            {
                this->currentIndex++;

                if(this->currentIndex == MAX_CHANNEL_BUFFER_SIZE)
                {
                    this->currentIndex = 0;
                }

                if(numElements < MAX_CHANNEL_BUFFER_SIZE)
                {
                    numElements++;
                }
            }

            std::shared_ptr<ChannelBufferElement>& getElement(size_t index)
            {
                if(index < 0 || index >= MAX_CHANNEL_BUFFER_SIZE)
                {
                    CLAID_THROW(Exception, "Error, tried to get element from channel with index " << index << " which is out of bounds [0, " << MAX_CHANNEL_BUFFER_SIZE << ").");
                }    

                return this->channelBufferElements[index];
            }

            template<typename T, typename Derived>
            bool getLatest(Derived* derived, ChannelData<T>& latest)
            {
                this->lockMutex();
               

                if(numElements == 0)
                {
                    // No data available yet
                    this->unlockMutex();
                    return false;
                }
                else
                {

                    size_t tmpIndex;
                    if(currentIndex == 0)
                    {
                        tmpIndex = MAX_CHANNEL_BUFFER_SIZE - 1;
                    }
                    else
                    {
                        tmpIndex = currentIndex - 1;
                    }

                    latest = derived->getDataByIndex(tmpIndex);
                    this->unlockMutex();
                    return true;
                }
            }

            template<typename T, typename Derived>
            bool getClosest(Derived* derived, const Time& timestamp, ChannelData<T>& closest)
            {
                this->lockMutex();

                if(this->numElements == 0)
                {
                    this->unlockMutex();
                    return false;
                }

                int64_t minimalDifference = std::numeric_limits<int64_t>::max();
                int index = -1;
                for(int i = 0; i < this->numElements; i++)
                {
                    ChannelData<T> taggedData = derived->getDataByIndex(i);
                                        // std::cout << "Time stamps " << TaggedData.timestamp.toString() << " " << timestamp.toString() << "\n";

                 

                    // uint64_t a = x.count();
                    // uint64_t = minimalDifference.getNanoSeconds();

                    if(taggedData.getTimestamp().toUnixNanoseconds() > timestamp.toUnixNanoseconds())
                    {
                        continue;
                    }

                    int64_t diff = abs(static_cast<long>(taggedData.getTimestamp().toUnixNanoseconds() - timestamp.toUnixNanoseconds()));
                    if(diff <= minimalDifference)
                    {
                        minimalDifference = diff;
                        index = i;
                    }
                    // else
                    // {
                    //     // If the difference is greater then our previous difference, this means
                    //     // we are iterating away from the closest object, as the objects are sorted in ascending order (oldest time stamp to newest).
                    //     // Thus, we can abort and return the closest object, if we found any.

                    //     if(index != -1)
                    //     {
                    //         closest = this->channelData[index];
                    //         this->unlockMutex();
                    //         return true;
                    //     }
                    //     else
                    //     {
                    //         this->unlockMutex();
                    //         return false;
                    //     }

                    // }
                }

                if(index != -1)
                {
                    closest = derived->getDataByIndex(index);
                    this->unlockMutex();
                    return true;
                }
                this->unlockMutex();

                return false;

            }

            template<typename T, typename Derived>
            void getDataInterval(Derived* derived, const Time& min, const Time& max, std::vector<ChannelData<T> >& channelDataInterval)
            {
                channelDataInterval.clear();
                this->lockMutex(); 
                // Iterate from oldest to newest.
                for(int i = 0; i < this->numElements; i++)
                {
                    ChannelData<T> channelData = derived->getDataByIndex(relativeIndex(i));


                    // TODO: THIS PROBABLY NEEDS A FIX? 
                    if(channelData.getTimestamp() < min)
                    {
                        continue;
                    }

                    if(channelData.getTimestamp() > max)
                    {
                        // As the data is sorted from oldest to newest, if our timestamp is higher then max,
                        // this means that timestamps of all following data will be higher then max.
                        break;
                    }
                  
                    channelDataInterval.push_back(channelData);


                   
                }

                this->unlockMutex(); 
            }



            
    };

}