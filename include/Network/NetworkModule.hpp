#pragma once

#include "Utilities/Time.hpp"

#include "RunTime/RunTime.hpp"
#include "Binary/BinaryData.hpp"
#include "RemoteConnection/Message/Message.hpp"
#include "RemoteConnection/Error/Error.hpp"
#include "Network/Error/NetworkErrors.hpp"


namespace claid
{
    namespace Network
    {
        class NetworkModule : public Module
        {
            protected:
                virtual void initialize() = 0;

            public: 
          
                virtual ~NetworkModule()
                {

                }         
        
        };
    }

    
}