/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"
#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"

#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"

#include <chrono>
#include <thread>




using namespace std;
using namespace claid;
using namespace claidservice;

using namespace std::chrono_literals;

using google::protobuf::Message;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::MessageDifferencer;
using google::protobuf::io::StringOutputStream;

/*
RemoteDispatcherClient(const std::string& addressToConnectTo,
                    const std::string& userToken,
                    const std::string& deviceID,
                    SharedQueue<DataPackage>& incomingQueue, 
                    SharedQueue<DataPackage>& outgoingQueue);
*/

TEST(RemoteDispatcherTestSuite, ServerTest) 
{
    std::cout << "=== CLIENT TEST BEGIN ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const std::string address = "localhost:1337";
    
    SharedQueue<DataPackage> inQueue;
    SharedQueue<DataPackage> outQueue;

    const std::string host = "alex_host";
    const std::string userToken = "User01";
    const std::string deviceID = "13:37";
    
    ClientTable clientTable;

    claid::RemoteDispatcherClient client(address, host, userToken, deviceID, clientTable);

    std::cout << "Checkpoint 1\n";

    absl::Status status = client.start();
    std::cout << "Checkpoint 2\n";
    ASSERT_TRUE(status.ok()) << status;
    client.shutdown();
    std::cout << "Checkpoint 3\n";

}