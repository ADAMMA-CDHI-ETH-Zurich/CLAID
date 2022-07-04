#pragma once

#include "TaggedData.hpp"
#include "ChannelBuffer.hpp"
#include "ChannelSubscriber.hpp"
#include "ChannelBase.hpp"
#include "ChannelAccessRights.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Exception/Exception.hpp"
#include <vector>

namespace portaible
{

    template<typename T>
    class Channel
    {
        private:
            GlobalChannel<T>* globalChannel;
            ChannelAccessRights channelAccessRights;

            void verifyReadAccess()
            {
                if(this->channelAccessRights != ChannelAccessRights::READ && this->channelAccessRights != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot read from channel with ID " << globalChannel->getChannelID() << " as it was only published, not subscribed to.");
                }
            }

            void verifyWriteAccess()
            {
                if(this->channelAccessRights != ChannelAccessRights::WRITE && this->channelAccessRights != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot post to channel with ID " << globalChannel->getChannelID().c_str() << " as it was only subscribed but not published.");
                }
            }
        public:

            Channel()
            {
            }

            Channel(GlobalChannel<T>* globalChannel, ChannelAccessRights channelAccessRights) : globalChannel(globalChannel), channelAccessRights(channelAccessRights)
            {

            }

            ChannelRead<T> read()
            {
                verifyReadAccess();
                return globalChannel->read();
            }

  
            ChannelRead<T> read(const Time& timestamp,
                                const Duration& tolerance = Duration::infinity())
            {
                verifyReadAccess();
                return globalChannel->read(timestamp, tolerance);
            }

             ChannelRead<T> read(const uint32_t sequenceID,
                                const Duration& searchIntervall = Duration::seconds(1))
            {
                verifyReadAccess();
                return globalChannel->read(sequenceID, searchIntervall);
            }

            void post(TaggedData<T> data)
            {
                verifyWriteAccess();
                this->globalChannel->post(data);
            }

            void post(T& data)
            {
                this->post(TaggedData<T>(data));
            }

            void post(std::shared_ptr<T> data)
            {
                this->post(TaggedData<T>(data));
            }

            void getChannelReadIntervall(const Time& min, const Time& max, std::vector<ChannelRead<T>>& channelReadIntervall)
            {
                return this->globalChannel->getChannelReadIntervall(min, max, channelReadIntervall);
            }

            const std::string& getChannelID() const
            {
                return this->globalChannel->getChannelID();
            }

    };
    

    template<typename T>
    class GlobalChannel : public GlobalChannelBase
    {

        private:
            ChannelBuffer<T> channelBuffer;
            std::vector<ChannelSubscriber<T> > channelSubscribers;
            std::string channelID;

            void signalNewDataToSubscribers()
            {
                for(ChannelSubscriber<T>& channelSubscriber : channelSubscribers)
                {
                    channelSubscriber.signalNewDataIsAvailable();
                } 
            }

        public:
      
            
            GlobalChannel(const std::string & channelID) : channelID(channelID)
            {

            }

            void post(const T& data)
            {
                this->post(TaggedData<T>(data));
            }

            void post(std::shared_ptr<T> data)
            {
                this->post(TaggedData<T>(data));
            }

            void post(TaggedData<T> data)
            {
                this->channelBuffer.insert(data);
                this->signalNewDataToSubscribers();
            }

            

            intptr_t getChannelDataTypeUniqueIdentifier()
            {
                return getDataTypeUniqueIdentifier<T>();
            }

            std::string getChannelDataTypeUniqueIdentifierRTTIString()
            {
                return getDataTypeRTTIString<T>();
            }
            
            ChannelRead<T> read()
            {
                TaggedData<T> latest;
                if(this->channelBuffer.getLatest(latest))
                {
                    return ChannelRead<T>(latest);
                }
                else
                {
                    return ChannelRead<T>::InvalidChannelRead();
                }
            }

            ChannelRead<T> read(const Time& timestamp,
                                const Duration& tolerance = Duration::infinity())
            {
                
                TaggedData<T> closest;
                if(!this->channelBuffer.getClosest(timestamp, closest))
                {
                    return ChannelRead<T>::InvalidChannelRead();
                }
                else
                {

                    int64_t difference = closest.timestamp.toUnixNS() - timestamp.toUnixNS();
                    // TODO are data types correct ? Is time/duration implementation correct ? 
                    if(abs(difference) <= tolerance.getNanoSeconds())
                    {
                        return ChannelRead<T>(closest);
                    }
                    else
                    {
                        return ChannelRead<T>::InvalidChannelRead();
                    }
                }
            }

            ChannelRead<T> read(const uint32_t sequenceID,
                                const Duration& searchIntervall)
            {
                Time newestTimeStamp = this->read()->timestamp;

                std::vector<ChannelRead<T>> channelReadIntervall;

                this->getChannelReadIntervall(newestTimeStamp - searchIntervall, newestTimeStamp, channelReadIntervall);

                for(ChannelRead<T>& channelRead : channelReadIntervall)
                {
                    if(channelRead->sequenceID == sequenceID)
                    {
                        return channelRead;
                    }
                }

                // No data with given sequence ID found.
                return ChannelRead<T>::InvalidChannelRead();
            }
           
            Channel<T> subscribe()
            {
                return Channel<T>(this, ChannelAccessRights::READ);
            }

   

            Channel<T> subscribe(ChannelSubscriber<T> channelSubscriber)
            {
                channelSubscriber.setChannel(this);
                this->channelSubscribers.push_back(channelSubscriber);
                
                return subscribe();
            }

            Channel<T> publish()
            {
                return Channel<T>(this, ChannelAccessRights::WRITE);
            }

            void getChannelReadIntervall(const Time& min, const Time& max, std::vector<ChannelRead<T> >& channelReadIntervall)
            {
                channelReadIntervall.clear();
                std::vector<TaggedData<T> > TaggedDataIntervall;
                this->channelBuffer.getDataIntervall(min, max, TaggedDataIntervall);

                for(TaggedData<T>& TaggedData : TaggedDataIntervall)
                {
                    channelReadIntervall.push_back(ChannelRead<T>(TaggedData));
                }
            }

            const std::string& getChannelID() const
            {
                return this->channelID;
            }

            // ChannelRead<T> read(const Time& timestamp, SlotQueryMode mode = NEAREST_SLOT,
            //                     const Duration& tolerance = Duration::infinity())
            //                     {

            //                     }
    };


}