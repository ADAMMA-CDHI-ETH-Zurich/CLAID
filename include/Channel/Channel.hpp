#pragma once

#include "TaggedData.hpp"
#include "ChannelBuffer.hpp"
#include "ChannelSubscriber.hpp"
#include "ChannelBase.hpp"
#include "ChannelAccessRights.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Exception/Exception.hpp"

namespace portaible
{
    template<typename T>
    class Channel;

    class ChannelManager;
}
#include "Channel/ChannelManager.hpp"

#include <vector>

namespace portaible
{

    template<typename T>
    class Channel
    {
        private:
            TypedChannel<T>* typedChannel;
            const ChannelSubscriberBase* subscriberPtr;

            // Stored as ptr, to make sure that if the channel object was copied 
            // and unsubcribe or unpublish was called later, all copied instances 
            // lose their access rights.
            std::shared_ptr<ChannelAccessRights> channelAccessRights;


            void verifyReadAccess()
            {
                if(*this->channelAccessRights.get() != ChannelAccessRights::READ && *this->channelAccessRights.get() != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot read from channel with ID " << typedChannel->getChannelID() << " as it was only published, not subscribed to.");
                }
            }

            void verifyWriteAccess()
            {
                if(*this->channelAccessRights.get() != ChannelAccessRights::WRITE && *this->channelAccessRights.get() != ChannelAccessRights::READ_WRITE)
                {
                    PORTAIBLE_THROW(Exception, "Cannot post to channel with ID " << typedChannel->getChannelID().c_str() << " as it was only subscribed but not published.");
                }
            }
        public:



            Channel() : subscriberPtr(nullptr)
            {
                this->channelAccessRights = std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::NONE));
            }

            Channel(TypedChannel<T>* typedChannel, std::shared_ptr<ChannelAccessRights> channelAccessRights) : typedChannel(typedChannel), channelAccessRights(channelAccessRights), subscriberPtr(nullptr)
            {

            }

            Channel(TypedChannel<T>* typedChannel, std::shared_ptr<ChannelAccessRights> channelAccessRights, ChannelSubscriberBase* subscriberPtr) : typedChannel(typedChannel), channelAccessRights(channelAccessRights), subscriberPtr(subscriberPtr)
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
                TaggedData<U> taggedData(data);
                this->post(taggedData);
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

            void unsubscribe()
            {
                this->typedChannel->callUnsubscribe(*this);
                *this->channelAccessRights.get() = ChannelAccessRights::NONE;
            }

            void unpublish()
            {
                this->typedChannel->callUnpublish(*this);
                *this->channelAccessRights.get() = ChannelAccessRights::NONE;
            }

            const ChannelSubscriberBase* getSubcriberHandle()
            {
                return this->subscriberPtr;
            }


    };
    // TODO: Add mutex when subscribing / unsubscribing
    template<typename T>
    class TypedChannel : public ChannelBase
    {

        private:
            // The channelManager, that created this channel.
            ChannelManager* channelManager;

            void signalNewDataToSubscribers()
            {
                for(ChannelSubscriberBase*& channelSubscriber : channelSubscribers)
                {
                    channelSubscriber->signalNewDataIsAvailable();
                } 
            }

        public:
      
            
            TypedChannel(ChannelManager* channelManager, const std::string& channelID) : ChannelBase(channelID), channelManager(channelManager)
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
                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::READ)));
            }

            Channel<T> subscribe(ChannelSubscriber<T> channelSubscriber)
            {
                ChannelSubscriber<T>* typedSubscriber = new ChannelSubscriber<T>(channelSubscriber);
                typedSubscriber->setChannel(this);

                ChannelSubscriberBase* untypedSubscriber = static_cast<ChannelSubscriberBase*>(typedSubscriber); 
                this->channelSubscribers.push_back(static_cast<ChannelSubscriberBase*>(typedSubscriber));
                
                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::READ)), untypedSubscriber);
            }

            Channel<T> publish()
            {
                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::WRITE)));
            }

            void getChannelDataInterval(const Time& min, const Time& max, std::vector<ChannelData<T> >& channelDataInterval)
            {
                channelDataInterval.clear();
                this->channelBuffer.getDataInterval(min, max, channelDataInterval);

           
            }

            // Unsubscribe and unpublish are a bit ugly.. I know
            // Because: Usually, a Channel<T> is created using ChannelManager.publish or subscribe, which are called by a Module.
            // But to unsubscribe or unpublish, it does not make sense to call them via the module.
            // Rather, the Channel<T> object itself should now how to unsubscribe/unpublish (i.e., revoking it's access rights).
            // So normally, neither the ChannelManager not corresponding Module need to care about it.
            // But, the ChannelManager should be notified when a channel was unsubscribed/unpublished from.
            // In TypedChannel<T>.unsubscribe, we could call ChannelManager.unsubscribe (or publish respectively).
            // However, then the order is reversed compared to publish/subscrube (ChannelManager.subscribe -> TypedChannel.subscribe vs.
            // TypedChannel.subscribe -> ChannelManager.subscribe).
            // Thus, we implement this callUnsubscribe and callPublish functions, which calls the corresponding functions of the ChannelManager,
            // which afterwards calls the corresponding function of the TypedChannel.
            // It's suboptimal, but luckily the user never needs to care about this implementation.
            void callUnsubscribe(Channel<T>& channel)
            {
                this->channelManager->unsubscribe(channel);
            }

            void callUnpublish(Channel<T>& channel)
            {
                this->channelManager->unpublish(channel);
            }

            void unsubscribe(Channel<T>& channel)
            {
                const ChannelSubscriberBase* subscriber = channel.getSubcriberHandle();

                if(subscriber != nullptr)
                {
                    auto it = std::find(this->channelSubscribers.begin(), this->channelSubscribers.end(), subscriber);

                    if(it != this->channelSubscribers.end())
                    {
                        this->channelSubscribers.erase(it);
                    }
                }

                

            }

            void unpublish(Channel<T>& channel)
            {
                // What to do?

            }


            // ChannelData<T> read(const Time& timestamp, SlotQueryMode mode = NEAREST_SLOT,
            //                     const Duration& tolerance = Duration::infinity())
            //                     {

            //                     }
    };




}