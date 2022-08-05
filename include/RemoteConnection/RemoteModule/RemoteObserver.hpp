#pragma once

#include "RunTime/RunTime.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Message/MessageHeader/MessageHeaderChannelUpdate.hpp"
#include "RemoteConnection/Message/MessageData/MessageDataString.hpp"
#include "Utilities/VariadicTemplateHelpers.hpp"
namespace portaible
{
    namespace RemoteConnection
    {
        // Observes everything we do locally (i.e. channels 
        // that were (un)published or (un)subscribed within the RunTime (global ChannelManager)).
        class RemoteObserver : public SubModule
        {
            private:
                void onMessageReceived(ChannelData<Message> message);
                void onChannelUpdate(const MessageHeaderChannelUpdate& header, const MessageDataString& data);
                void onChannelPublished(const std::string& channelID);

            public:
                ChannelSubscriber<Message> getSubscriberForReceptionOfMessages();

     

                template<typename Header, typename Data, typename Class, typename... Args>
                bool callFunctionIfSignatureMatches(const Message& message, void (Class::*f)(const Header& header, const Data& data, Args...), Class* obj, Args... args)
                {
                    const MessageHeader& messageHeader = *message.header.get();
                    const MessageData& messageData = *message.data.get();

                    if(messageHeader.is<Header>())
                    {
                        const Header& typedHeader = *messageHeader.as<Header>();

                        if(messageData.is<Data>())
                        {
                            const Data& typedData = *messageData.as<Data>();
                            std::function<void (const Header&, const Data&, Args...)> function = VariadicTemplateHelpers::bind_with_variadic_placeholders<Class, const Header&, const Data&, Args...>(f, obj);

                            function(typedHeader, typedData, args...);
                            return true;
                        }
                    }
                    return false;
                }

                template<typename Header, typename Data, typename Class, typename... Args>
                bool callFunctionIfSignatureMatchesThrowExceptionIfWrongData(const Message& message, void (Class::*f)(const Header& header, const Data& data, Args...), Class* obj, Args... args)
                {
                    const MessageHeader& messageHeader = *message.header.get();
                    const MessageData& messageData = *message.data.get();

                    if(messageHeader.is<Header>())
                    {
                        const Header& typedHeader = *messageHeader.as<Header>();

                        if(messageData.is<Data>())
                        {
                            const Data& typedData = *messageData.as<Data>();
                            std::function<void (const Header&, const Data&, Args...)> function = VariadicTemplateHelpers::bind_with_variadic_placeholders<Class, const Header&, const Data&, Args...>(f, obj);

                            function(typedHeader, typedData, args...);
                            return true;
                        }
                        else
                        {
                            PORTAIBLE_THROW(Exception, "Error! Received Message with header type \"" << messageHeader.getClassName() << "\" and expected data "
                            << "to be of type \"" << Data::staticGetClassName() << "\", but got data with type \"" << messageData.getClassName() << "\".");
                        }
                    }
                    return false;
                }

        };
    }
}