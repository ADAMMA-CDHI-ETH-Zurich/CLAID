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

   
            // Use new template parameter U to allow to remove this
            // function for Untyped channels.
            // It would not be possible to use T, because we need to write
            // template<typename> in order to use std::enable_if
            
            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(TaggedData<T>& data)
            {
                verifyWriteAccess();
                this->typedChannel->post(data);
            }

            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(U& data)
            {
                this->post(TaggedData<U>(data));
            }

            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(std::shared_ptr<U> data)
            {
                this->post(TaggedData<U>(data));
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
    
    // TODO: Add mutex when subscribing / unsubscribing
    template<typename T>
    class TypedChannel : public ChannelBase
    {

        private:
       

            void signalNewDataToSubscribers()
            {
                for(ChannelSubscriberBase*& channelSubscriber : channelSubscribers)
                {
                    channelSubscriber->signalNewDataIsAvailable();
                } 
            }

        public:
      
            
            TypedChannel(const std::string& channelID) : ChannelBase(channelID)
            {
                // If T is Untyped, ChannelBuffer will be Untyped automatically.
                this->channelBuffer = new ChannelBuffer<T>();
            }

            virtual ~TypedChannel()
            {

            }


            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(const U& data) 
            {
                this->post(TaggedData<U>(data));

            }

            // Use new template parameter U to allow to remove this
            // function for Untyped channels.
            // It would not be possible to use T, because we need to write
            // template<typename> in order to use std::enable_if
            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(std::shared_ptr<T> data)
            {
                this->post(TaggedData<T>(data));
            }

            // Use new template parameter U to allow to remove this
            // function for Untyped channels.
            // It would not be possible to use T, because we need to write
            // template<typename> in order to use std::enable_if
            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(TaggedData<T> data)
            {
                this->castBuffer()->insert(data);
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

        
            ChannelBuffer<T>* castBuffer()
            {
                if(this->channelBuffer->isTyped())
                {
                    // What if T is Untyped although the buffer is already typed.. ? 
                    // We should be able to cast ChannelBufferBase* to ChannelBuffer<Untyped>
                    // anyways, EVEN if ChannelBufferBase* is not a ChannelBuffer<Untyped>, i.e.
                    // it was not created with channelBuffer = new ChannelBuffer<Untyped> but for example
                    // channelBuffer = new ChannelBuffer<int>.

                    // Why does it work? Because ChannelBuffer<T> and ChannelBuffer<Untyped> do not contain ANY
                    // additional variables compared to ChannelBufferBase.
                    // Thus, ChannelBuffer<T> and ChannelBuffer<Untyped> function as a "view" on the underlying data
                    // of ChannelDataBase.
                    return static_cast<ChannelBuffer<T>*>(this->channelBuffer);

                }
                else
                {
                    // if T is NOT untyped, this should be an exception. How can we do a typed access
                    // while the buffer is untyped ? ChannelManager must have messed up in casting the channel.

                    if(!std::is_same<T, Untyped>::value)
                    {
                        PORTAIBLE_THROW(Exception, "Error, ChannelBuffer is untyped, however we got a typed Channel object"
                        "that tried to acess data. This should not happen, as normally when we get the typed Channel, the ChannelBuffer should have been converted"
                        "to a typed one. MakeTyped Channel was not executed on Channel?");
                    }

                    return static_cast<ChannelBuffer<T>*>(this->channelBuffer);

                    //return static_cast<ChannelBuffer<Untyped>*>(this->channelBuffer);
                }
                // If T is Untyped -> then ? 
                // 
            }
            
            ChannelData<T> read()
            {
                ChannelData<T> latest;
                if(castBuffer()->getLatest(latest))
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
                if(!castBuffer()->getClosest(timestamp, closest))
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
                                const Duration& searchInterval)
            {
                Time newestTimeStamp = this->read()->timestamp;

                std::vector<ChannelData<T>> channelDataInterval;

                this->getChannelDataInterval(newestTimeStamp - searchInterval, newestTimeStamp, channelDataInterval);

                for(ChannelData<T>& channelData : channelDataInterval)
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
                ChannelSubscriber<T>* typedSubscriber = new ChannelSubscriber<T>(channelSubscriber);
                typedSubscriber->setChannel(this);

                this->channelSubscribers.push_back(static_cast<ChannelSubscriberBase*>(typedSubscriber));
                
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