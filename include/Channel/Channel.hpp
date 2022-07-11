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
            TypedChannel<T>* typedChannel;
            ChannelAccessRights channelAccessRights;

            void verifyReadAccess()
            {
                if(this->channelAccessRights != ChannelAccessRights::READ && this->channelAccessRights != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot read from channel with ID " << typedChannel->getChannelID() << " as it was only published, not subscribed to.");
                }
            }

            void verifyWriteAccess()
            {
                if(this->channelAccessRights != ChannelAccessRights::WRITE && this->channelAccessRights != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot post to channel with ID " << typedChannel->getChannelID().c_str() << " as it was only subscribed but not published.");
                }
            }
        public:

            Channel()
            {
            }

            Channel(TypedChannel<T>* typedChannel, ChannelAccessRights channelAccessRights) : typedChannel(typedChannel), channelAccessRights(channelAccessRights)
            {

            }

            ChannelData<T> read()
            {
                verifyReadAccess();
                return typedChannel->read();
            }

  
            ChannelData<T> read(const Time& timestamp,
                                const Duration& tolerance = Duration::infinity())
            {
                verifyReadAccess();
                return typedChannel->read(timestamp, tolerance);
            }

             ChannelData<T> read(const uint32_t sequenceID,
                                const Duration& searchIntervall = Duration::seconds(1))
            {
                verifyReadAccess();
                return typedChannel->read(sequenceID, searchIntervall);
            }

            void post(TaggedData<T> data)
            {
                verifyWriteAccess();
                this->typedChannel->post(data);
            }

            void post(T& data)
            {
                this->post(TaggedData<T>(data));
            }

            void post(std::shared_ptr<T> data)
            {
                this->post(TaggedData<T>(data));
            }

            void getChannelDataIntervall(const Time& min, const Time& max, std::vector<ChannelData<T>>& channelDataIntervall)
            {
                return this->typedChannel->getChannelDataIntervall(min, max, channelDataIntervall);
            }

            const std::string& getChannelID() const
            {
                return this->typedChannel->getChannelID();
            }

    };
    

    template<typename T>
    class TypedChannel : public ChannelBase
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
      
            
            TypedChannel(const std::string& channelID) : channelID(channelID)
            {

            }

            virtual ~TypedChannel()
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
            
            ChannelData<T> read()
            {
                ChannelData<T> latest;
                if(this->channelBuffer.getLatest(latest))
                {
                    return latest;
                }
                else
                {
                    return ChannelData<T>::InvalidChannelData();
                }
            }

            ChannelData<T> read(const Time& timestamp,
                                const Duration& tolerance = Duration::infinity())
            {
                ChannelData<T> closest;
                if(!this->channelBuffer.getClosest(timestamp, closest))
                {
                    return ChannelData<T>::InvalidChannelData();
                }
                else
                {

                    int64_t difference = closest.timestamp.toUnixNS() - timestamp.toUnixNS();
                    // TODO are data types correct ? Is time/duration implementation correct ? 
                    if(abs(difference) <= tolerance.getNanoSeconds())
                    {
                        return closest;
                    }
                    else
                    {
                        return ChannelData<T>::InvalidChannelData();
                    }
                }
            }

            ChannelData<T> read(const uint32_t sequenceID,
                                const Duration& searchIntervall)
            {
                Time newestTimeStamp = this->read()->timestamp;

                std::vector<ChannelData<T>> channelDataIntervall;

                this->getChannelDataIntervall(newestTimeStamp - searchIntervall, newestTimeStamp, channelDataIntervall);

                for(ChannelData<T>& channelData : channelDataIntervall)
                {
                    if(channelData->sequenceID == sequenceID)
                    {
                        return channelData;
                    }
                }

                // No data with given sequence ID found.
                return ChannelData<T>::InvalidChannelData();
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

            void getChannelDataInterval(const Time& min, const Time& max, std::vector<ChannelData<T> >& channelDataInterval)
            {
                channelDataInterval.clear();
                this->channelBuffer.getDataInterval(min, max, channelDataInterval);

           
            }

            const std::string& getChannelID() const
            {
                return this->channelID;
            }

            // ChannelData<T> read(const Time& timestamp, SlotQueryMode mode = NEAREST_SLOT,
            //                     const Duration& tolerance = Duration::infinity())
            //                     {

            //                     }
    };

    template<typename T>
    class LocalTypedChannel : public TypedChannel<T>
    {
        public:
            LocalTypedChannel() : TypedChannel<T>("LocalChannel")
            {

            }
    };


}