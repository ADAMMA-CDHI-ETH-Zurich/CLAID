#pragma once

#include "TaggedData.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#define MAX_CHANNEL_BUFFER_SIZE 100

namespace portaible
{
    template<typename T>
    class ChannelBuffer
    {
        TaggedData<T> channelData[MAX_CHANNEL_BUFFER_SIZE];
        
        size_t currentIndex = 0;
        size_t numElements = 0;

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
        public:
            ChannelBuffer()
            {
                this->currentIndex = 0;
                this->numElements = 0;
            }

            void insert(TaggedData<T>& data)
            {

                this->lockMutex();



                this->channelData[this->currentIndex] = data;
                this->currentIndex++;

                if(this->currentIndex == MAX_CHANNEL_BUFFER_SIZE)
                {
                    this->currentIndex = 0;
                }

                if(numElements < MAX_CHANNEL_BUFFER_SIZE)
                {
                    numElements++;
                }
                this->unlockMutex();

            }

            bool getLatest(TaggedData<T>& latest)
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


                    latest = this->channelData[tmpIndex];
                    this->unlockMutex();
                    return true;
                }
            }
            // TODO Hier hab ich bestimmt verkackt.
            bool getClosest(const Time& timestamp, TaggedData<T>& closest)
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
                    TaggedData<T>& TaggedData = this->channelData[i];
                                        // std::cout << "Time stamps " << TaggedData.timestamp.toString() << " " << timestamp.toString() << "\n";

                 

                    // uint64_t a = x.count();
                    // uint64_t = minimalDifference.getNanoSeconds();

                    if(TaggedData.timestamp.toUnixNS() > timestamp.toUnixNS())
                    {
                        continue;
                    }

                    int64_t diff = abs(static_cast<long>(TaggedData.timestamp.toUnixNS() - timestamp.toUnixNS()));
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
                    closest = this->channelData[index];
                    this->unlockMutex();
                    return true;
                }
                this->unlockMutex();

                return false;

            }



            void getDataIntervall(const Time& min, const Time& max, std::vector<TaggedData<T> >& TaggedDataIntervall)
            {
                TaggedDataIntervall.clear();
                this->lockMutex(); 
                // Iterate from oldest to newest.
                for(int i = 0; i < this->numElements; i++)
                {
                    TaggedData<T>& TaggedData = this->channelData[relativeIndex(i)];
                    // if(TaggedData.timestamp < min)
                    // {
                    //     continue;
                    // }

                    // if(TaggedData.timestamp > max)
                    // {
                    //     // As the data is sorted from oldest to newest, if our timestamp is higher then max,
                    //     // this means that timestamps of all following data will be higher then max.
                    //     break;
                    // }
                  
                    TaggedDataIntervall.push_back(TaggedData);
                   
                }

                this->unlockMutex(); 
            }
        
    };
}

