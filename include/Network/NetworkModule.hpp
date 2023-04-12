#pragma once

#include "Utilities/Time.hpp"

#include "RunTime/RunTime.hpp"
#include "Binary/BinaryData.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Error/Error.hpp"
#include "Network/Error/NetworkErrors.hpp"
#include "Network/NetworkStateChangeRequest.hpp"
#include "Reflection/Reflect.hpp"
namespace claid
{
    namespace Network
    {
        class NetworkModule : public Module
        {    
            protected:
                Channel<NetworkStateChangeRequest> networkStateChangeRequestChannel;
                std::string listenForNetworkChangeRequestsOn;

                virtual void initialize()
                {
                    if(this->listenForNetworkChangeRequestsOn != "")
                    {
                        this->networkStateChangeRequestChannel = 
                            this->subscribe<NetworkStateChangeRequest>(
                                    this->listenForNetworkChangeRequestsOn, &NetworkModule::onNetworkStateChangeRequestedCallback, this);
                    }
                }

                virtual void onNetworkStateChangeRequested(const NetworkStateChangeRequest& networkStateChangeRequest)
                {
                    
                }

            private:
                void onNetworkStateChangeRequestedCallback(ChannelData<NetworkStateChangeRequest> data)
                {
                    this->onNetworkStateChangeRequested(data->value());
                }

            
                

            public: 

                Reflect(NetworkModule,
                    reflectMemberWithDefaultValue(listenForNetworkChangeRequestsOn, std::string(""));
                )
          
                virtual ~NetworkModule()
                {

                }         
        
        };
    }

    
}