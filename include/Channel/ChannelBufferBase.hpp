#pragma once

#include "TaggedData.hpp"
#include "Binary/BinaryData.hpp"
#include "Exception/Exception.hpp"
#include "ChannelBufferElement.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#define MAX_CHANNEL_BUFFER_SIZE 20

namespace portaible
{
    class ChannelBufferBase
    {
        public:
            ChannelBufferBase()
            {
                this->currentIndex = 0;
                this->numElements = 0;
                this->typed = false;

                for(size_t i = 0; i < MAX_CHANNEL_BUFFER_SIZE; i++)
                {
                    this->channelBufferElements[i].channelData = nullptr;
                    this->channelBufferElements[i].untypedData = nullptr;
                    this->channelBufferElements[i].binaryData = nullptr;
                }
            }

            bool isTyped()
            {
                return this->typed;
            }

        protected:
            ChannelBufferElement channelBufferElements[MAX_CHANNEL_BUFFER_SIZE];
            size_t currentIndex = 0;
            size_t numElements = 0;
            bool typed = false;

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

           

            
            virtual void insertBinaryData(TaggedData<BinaryData>& binaryData)
            {
                this->lockMutex();

                // Is this thread safe?
                // Yes.. as long as we create copies in getLatest, getClosest and getDataInterval.
                if(this->channelBufferElements[this->currentIndex].binaryData != nullptr)
                {
                    delete this->channelBufferElements[this->currentIndex].binaryData;
                }
                this->channelBufferElements[this->currentIndex].binaryData = new TaggedData<BinaryData>(binaryData);

                // TODO: if typed, deserialize
                // Do we have to ? 
                // What if no one wants to access typed data.....
                // Deserialize on typed read ? e.g. in getDataByIndex ? 
                if(this->isTyped())
                {
                    // deserialize
                }

                increaseIndex();
                this->unlockMutex();
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

            ChannelBufferElement& getElement(size_t index)
            {
                if(index < 0 || index >= MAX_CHANNEL_BUFFER_SIZE)
                {
                    PORTAIBLE_THROW(Exception, "Error, tried to get element from channel with index " << index << " which is out of bounds [0, " << MAX_CHANNEL_BUFFER_SIZE << ").");
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

                    printf("get 1\n");
                    latest = derived->getDataByIndex(tmpIndex);
                    printf("get 2\n");
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
                    ChannelData<T>& taggedData = derived->getDataByIndex(i);
                                        // std::cout << "Time stamps " << TaggedData.timestamp.toString() << " " << timestamp.toString() << "\n";

                 

                    // uint64_t a = x.count();
                    // uint64_t = minimalDifference.getNanoSeconds();

                    if(taggedData.getTimestamp().toUnixNS() > timestamp.toUnixNS())
                    {
                        continue;
                    }

                    int64_t diff = abs(static_cast<long>(taggedData.getTimestamp().toUnixNS() - timestamp.toUnixNS()));
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
                    ChannelData<T>& channelData = derived->getDataByIndex(relativeIndex(i));


                    // TODO: THIS PROBABLY NEEDS A FIX? 
                    // WAS COMMENTED WHEN TESTING CHANNELSYNCHRONIZER, I UNCOMMENTED IT WITHOUT CHECKING FOR BUGS.
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



            virtual std::string getDataTypeName() = 0;
    };

}