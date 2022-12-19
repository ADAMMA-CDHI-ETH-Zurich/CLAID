#pragma once

#include "TaggedData.hpp"
#include "ChannelBuffer.hpp"
#include "ChannelSubscriber.hpp"
#include "ChannelBase.hpp"
#include "ChannelAccessRights.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
#include "Exception/Exception.hpp"
#include <algorithm>
namespace claid
{
    template<typename T>
    class Channel;

    class ChannelManager;
}
#include "Channel/ChannelManager.hpp"

#include <vector>

namespace claid
{
    

    template<typename T>
    class Channel
    {
        private:
            TypedChannel<T>* typedChannel;
            const ChannelSubscriberBase* subscriberPtr;
            size_t publisherSubscriberUniqueIdentifier = {0};

            // Stored as ptr, to make sure that if the channel object was copied 
            // and unsubcribe or unpublish was called later, all copied instances 
            // lose their access rights.
            std::shared_ptr<ChannelAccessRights> channelAccessRights;


            void verifyReadAccess()
            {
                if(*this->channelAccessRights.get() != ChannelAccessRights::READ && *this->channelAccessRights.get() != ChannelAccessRights::READ_WRITE)
                {
                    CLAID_THROW(Exception, "Cannot read from channel with ID " << typedChannel->getChannelID() << " as it was either only published or has been unsubscribed.");
                }
            }

            void verifyWriteAccess()
            {
                if(*this->channelAccessRights.get() != ChannelAccessRights::WRITE && *this->channelAccessRights.get() != ChannelAccessRights::READ_WRITE)
                {
                    CLAID_THROW(Exception, "Cannot post to channel with ID " << typedChannel->getChannelID() << " as it was either only subscribed to or has been unpublished.");
                }
            }
        public:



            Channel() :  subscriberPtr(nullptr), publisherSubscriberUniqueIdentifier(0)
            {
                this->channelAccessRights = std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::NONE));
            }

            Channel(TypedChannel<T>* typedChannel, 
                std::shared_ptr<ChannelAccessRights> channelAccessRights, size_t publisherSubscriberUniqueIdentifier) : 
                typedChannel(typedChannel), channelAccessRights(channelAccessRights), 
                subscriberPtr(nullptr), publisherSubscriberUniqueIdentifier(publisherSubscriberUniqueIdentifier)
            {

            }

            Channel(TypedChannel<T>* typedChannel, 
                std::shared_ptr<ChannelAccessRights> channelAccessRights, ChannelSubscriberBase* subscriberPtr, size_t publisherSubscriberUniqueIdentifier) : 
                typedChannel(typedChannel), channelAccessRights(channelAccessRights), 
                subscriberPtr(subscriberPtr), publisherSubscriberUniqueIdentifier(publisherSubscriberUniqueIdentifier)
            {

            }

            ~Channel()
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

             ChannelData<T> read(const uint64_t sequenceID,
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
            post(const T& data, const Time timestamp = Time::now(), uint64_t sequenceID = 0)
            {
                TaggedData<U> taggedData(data, timestamp, sequenceID);
                this->post<U>(taggedData);
            }

            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(std::shared_ptr<T> data, const Time timestamp = Time::now(), uint64_t sequenceID = 0)
            {
                TaggedData<U> taggedData(data, timestamp, sequenceID);
                this->post<U>(taggedData);
            }

            void postBinaryData(TaggedData<BinaryData>& binaryData)
            {
                this->verifyWriteAccess();
                this->typedChannel->postBinaryData(binaryData);         
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
                // If the access rights are NONE already, possible unsubscribe has been called
                // on a copy of a Channel<T> object. Thus, we do not 
                // unsubscribe again. Also see comments in TypedChannel<T>::unsubscribe (and unpublish)
                if(*this->channelAccessRights.get() != ChannelAccessRights::NONE)
                {
                    this->typedChannel->callUnsubscribe(*this);
                    *this->channelAccessRights.get() = ChannelAccessRights::NONE;
                }
                else
                {
                    // TODO LOG WARNING
                }
            }

            void unpublish()
            {
                // If the access rights are NONE already, possible unpublish has been called
                // on a copy of a Channel<T> object. Thus, we do not 
                // unpublish again. Also see comments in TypedChannel<T>::unpublish (and unsubscribe)
                if(*this->channelAccessRights.get() != ChannelAccessRights::NONE)
                {
                    this->typedChannel->callUnpublish(*this);
                    *this->channelAccessRights.get() = ChannelAccessRights::NONE;
                }
                else
                {
                    // TODO LOG WARNING
                }
            }

            const ChannelSubscriberBase* getSubcriberHandle()
            {
                return this->subscriberPtr;
            }

            size_t getPublisherSubscriberUniqueIdentifier()
            {
                return this->publisherSubscriberUniqueIdentifier;
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

            // Templated helper functions to solve redundant dependencies:
            // ChannelManager needs TypedChannel<T>, but TypedChannel<T> needs ChannelManager to unsubscribe / unpublish
            // (i.e. it needs to call the corresponding functions of the ChannelManager the TypedChannel belongs to).
            // Therefore, at the beginning of this file, we provided the forward declaration, which is incomplete (i.e. does not define the
            // unsubscribe and unpublish functions of ChannelManager). Clang is smart enough to get it anyways.
            // Gcc too, but throws a lot of warnings which are annyoing.
            // Thus, by wrapping it into another templated function, we can suppress this warnings as this templated function is only evaluated
            // when it is used (i.e. channel.unsubscribe() or unpublish() is used), and by that time ChannelManager has been fully defined already.
            // Not the prettiest solution, but it works.
            template<typename A>
            void callUnsubscribeHelper(A& a, Channel<T>& channel)
            {
                a.unsubscribe(channel);
            }

            template<typename A>
            void callUnpublishHelper(A& a, Channel<T>& channel)
            {
                a.unpublish(channel);
            }

            

        public:
      
            
            TypedChannel(ChannelManager* channelManager, const std::string& channelID) : ChannelBase(channelID), channelManager(channelManager)
            {
                // If T is Untyped, ChannelBuffer will be Untyped automatically.
                this->channelBuffer = new ChannelBuffer<T>();
                this->numPublishers = 0;
                this->numSubscribers = 0;
            }

            // Used by type() to type channel (TypedChannel<Untyped> -> TypedChannel<T>)
            TypedChannel(std::vector<ChannelSubscriberBase*> channelSubscribers,
                        std::string channelID,
                        size_t numPublishers,
                        size_t numSubscribers,
                        ChannelManager* channelManager,
                        ChannelBufferBase* channelBuffer) 
            {
                this->channelSubscribers = channelSubscribers;
                this->channelID = channelID;
                this->numPublishers = numPublishers;
                this->numSubscribers = numSubscribers;
                this->channelManager = channelManager;
                this->channelBuffer = channelBuffer;

            }

            virtual ~TypedChannel()
            {

            }


            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(const U& data, const Time timestamp = Time::now(), uint64_t sequenceID = 0) 
            {
                this->post(TaggedData<U>(data, timestamp, sequenceID));
            }

            // Use new template parameter U to allow to remove this
            // function for Untyped channels.
            // It would not be possible to use T, because we need to write
            // template<typename> in order to use std::enable_if
            template <typename U = T>
            typename std::enable_if<!std::is_same<U, Untyped>::value>::type
            post(std::shared_ptr<T> data, const Time timestamp = Time::now(), uint64_t sequenceID = 0)
            {
                this->post(TaggedData<T>(data, timestamp, sequenceID));
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
                return this->channelBuffer->getDataTypeIdentifier();
            }

            std::string getChannelDataTypeName()
            {
                return this->channelBuffer->getDataTypeName();
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
                        CLAID_THROW(Exception, "Error, ChannelBuffer is untyped, however we got a typed Channel object"
                        "that tried to acess data. This should not happen, as normally when we get the typed Channel, the ChannelBuffer should have been converted"
                        "to a typed one. MakeTyped Channel was not executed on Channel?");
                    }

                    return static_cast<ChannelBuffer<T>*>(this->channelBuffer);

                    //return static_cast<ChannelBuffer<Untyped>*>(this->channelBuffer);
                }
                // If T is Untyped -> then ? 
                // 
            }

            void postBinaryData(TaggedData<BinaryData>& binaryData)
            {
                this->castBuffer()->insertBinaryData(binaryData);
                this->signalNewDataToSubscribers();
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

            ChannelData<T> read(const uint64_t sequenceID,
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
           
            Channel<T> subscribe(size_t publisherSubscriberUniqueIdentifier)
            {
                this->numSubscribers++;
                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::READ)), publisherSubscriberUniqueIdentifier);
            }

            Channel<T> subscribe(ChannelSubscriber<T> channelSubscriber, size_t publisherSubscriberUniqueIdentifier)
            {
                ChannelSubscriber<T>* typedSubscriber = new ChannelSubscriber<T>(channelSubscriber);
                typedSubscriber->setChannel(this);

                ChannelSubscriberBase* untypedSubscriber = static_cast<ChannelSubscriberBase*>(typedSubscriber); 
                this->channelSubscribers.push_back(static_cast<ChannelSubscriberBase*>(typedSubscriber));
                
                this->numSubscribers++;

                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::READ)), untypedSubscriber, publisherSubscriberUniqueIdentifier);
            }

            Channel<T> publish(size_t publisherSubscriberUniqueIdentifier)
            {
                this->numPublishers++;

                return Channel<T>(this, std::shared_ptr<ChannelAccessRights>(new ChannelAccessRights(ChannelAccessRights::WRITE)), publisherSubscriberUniqueIdentifier);
            }

            void getChannelDataInterval(const Time& min, const Time& max, std::vector<ChannelData<T> >& channelDataInterval)
            {
                channelDataInterval.clear();
                this->channelBuffer->getDataInterval(min, max, channelDataInterval);           
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
                //this->channelManager->unsubscribe(channel);
                callUnsubscribeHelper<ChannelManager>(*this->channelManager, channel);
            }


            void callUnpublish(Channel<T>& channel)
            {
                callUnpublishHelper<ChannelManager>(*this->channelManager, channel);
//                this->channelManager->unpublish(channel);
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

                if(this->numSubscribers == 0)
                {
                    CLAID_THROW(Exception, "Error! Unsubscribe was called for channel with id " << this->channelID << ". However, it's number of subscribers is already 0."
                    "There must be a bug somewhere that allows to unsubscribe a channel multiple times.");
                }

                numSubscribers--;

            }

            void unpublish(Channel<T>& channel)
            {
                // Could numPublishers be already 0, when unpublish is called?
                // Yes, theoretically.
                // Consider the following:
                /*
                    Channel<int> c1 = typedChannel.publish("IntChannel"); // typedChannel.numPublishers = 1
                    Channel<int> c2 = c1;

                    c1.unpublish(); // typedChannel.numPublishers = 0
                    c2.unpublish(); // typedChannel.numPublishers = -1 // theoretically numPublishers would be negative now (not possible, as numPublishers
                                       is size_t, therefore underflow).
                */
                // Copying channels is not forbidden (e.g. in order to allow to store multiple channels in a vector).
                // Thus, theoretically someone could create a channel, copy it, and call unpublish on both channels.
                // However, see implementation of Channel.unpublish: it checks the access rights and if it does not have write access anymore,
                // it assumes it has been unpublished already (complementary for unsubscribe).
                // Therefore, it would not call typedChannel.unpublish().
                // Thus, we throw an exception here if number of publishers would get negative.
            


                if(this->numPublishers == 0)
                {
                    CLAID_THROW(Exception, "Error! Unpublish was called for channel with id " << this->channelID << ". However, it's number of publishers is already 0."
                    "There must be a bug somewhere that allows to unpublish a channel multiple times.");
                }

                numPublishers--;
            }

            size_t getNumPublishers()
            {
                return this->numPublishers;
            }

            size_t getNumSubscribers()
            {
                return this->numSubscribers;
            }

            // ChannelData<T> read(const Time& timestamp, SlotQueryMode mode = NEAREST_SLOT,
            //                     const Duration& tolerance = Duration::infinity())
            //                     {

            //                     }

            template <typename NewType, typename U = T>
            typename std::enable_if<std::is_same<U, Untyped>::value && !std::is_same<NewType, Untyped>::value>::type
            type() 
            {   
                if(this->channelBuffer->isTyped())
                {
                    CLAID_THROW(Exception, "Error, tried to type a channel to type " << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<NewType>() << ", but the channel has already been typed "
                    << " to type " << this->channelBuffer->getDataTypeName());
                }
                ChannelBuffer<Untyped>* untypedBuffer = static_cast<ChannelBuffer<Untyped>*>(this->channelBuffer);
                ChannelBuffer<NewType>* typedBuffer = untypedBuffer->type<NewType>();
                this->channelBuffer = static_cast<ChannelBufferBase*>(typedBuffer);
                delete untypedBuffer;


            }
    };




}