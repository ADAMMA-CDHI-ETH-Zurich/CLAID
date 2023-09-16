#pragma once

#include "dispatch/core/shared_queue.hh"

using claidservice::DataPackage;

namespace claid
{
    class RemoteGRPCClient
    {
        private:
            SharedQueue<DataPackage>& inputQueue;
            SharedQueue<DataPackage>& outputQueue;

            const std::string addressToConnectTo;

        public: 
            RemoteGRPCClient(
                    SharedQueue<DataPackage>& inputQueue, 
                    SharedQueue<DataPackage>& outputQueue,
                    const std::string& addressToConnectTo) :  inputQueue(inputQueue), 
                                                            outputQueue(outputQueue), 
                                                            onnectToAddress(addressToConnectTo)
            {

            }
            
            
    };
}