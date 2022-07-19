#include "Utilities/Time.hpp"

#include "RunTime/RunTime.hpp"
#include "Binary/BinaryData.hpp"
#include "Network/Message.hpp"

namespace portaible
{
    namespace Network
    {
        class NetworkModule : public Module
        {
            protected:
                virtual void initialize() = 0;
                virtual void onError() = 0;

            public: 
          
                virtual ~NetworkModule()
                {

                }         
        
        };
    }

    
}