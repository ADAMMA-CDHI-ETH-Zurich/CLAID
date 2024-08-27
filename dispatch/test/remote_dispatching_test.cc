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
#include "dispatch/core/CLAID.hh"

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

class SenderModule : public claid::Module
{
    Channel<std::string> sendChannel;

    void initialize(Properties properties)
    {
        moduleInfo("Initialize called");
        sendChannel = this->publish<std::string>("TestChannel");

        registerPeriodicFunction("PeriodicFunction", &SenderModule::periodicFunction, this, Duration::milliseconds(1000));
    }

    void periodicFunction()
    {
        Logger::logInfo("Periodic function");
        sendChannel.post("TestData");
    }

    void onConnectedToRemoteServer()
    {
  
    }

    void onDisconnectedFromRemoteServer()
    {

    }
};

class ReceiverModule : public claid::Module
{
    Channel<std::string> receiveChannel;

    void initialize(Properties properties)
    {
        moduleInfo("Initialize!");
        receiveChannel = this->subscribe<std::string>("TestChannel", &ReceiverModule::onData, this);
    }

    void onData(ChannelData<std::string> data)
    {
        Logger::logInfo("Received data %s", data.getData().c_str());
    }
};

REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestSenderModule, SenderModule)
REGISTER_MODULE_FACTORY_CUSTOM_NAME(TestReceiverModule, ReceiverModule)

TEST(RemoteDispatcherTestSuite, ServerTest) 
{
    // unix domain socket for the client CLAID instance (not for remote communication, only for local dispatching!)
    const char* socket_path_local_1 = "unix:///tmp/test_grpc_client.sock";

    // unix domain socket for the server CLAID instance (not for remote communication, only for local dispatching!)
    const char* socket_path_local_2 = "unix:///tmp/test_grpc_server.sock";
    const char* config_file = "dispatch/test/remote_dispatching_test.json";
    const char* client_host_id = "test_client";
    const char* server_host_id = "test_server";
    const char* user_id = "user42";
    const char* device_id = "something_else";

    bool result;

    Logger::logInfo("===== STARTING CLIENT MIDDLEWARE ====");

    CLAID clientMiddleware;
    result = clientMiddleware.start(socket_path_local_1, config_file, client_host_id, user_id, device_id);
    ASSERT_TRUE(result) << "Failed to start client middleware";
   
    // We deliberately start the clientMiddleware first. This will cause the connection attempt
    // of the RemoteDispatcherClient to fail, because the server is not started yet.
    // The RemoteDispatcherClient should then try at a later time (every 2 seconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    Logger::logInfo("===== STARTING SERVER MIDDLEWARE ====");
    
    CLAID serverMiddleware;
    result = serverMiddleware.start(socket_path_local_2, config_file, server_host_id, user_id, device_id);
    ASSERT_TRUE(result) << "Failed to start server middleware";
   

    std::this_thread::sleep_for(std::chrono::milliseconds(4100));
    ASSERT_TRUE(clientMiddleware.isConnectedToRemoteServer()) << clientMiddleware.getRemoteClientStatus();

    Logger::logInfo("===== STOPPING SERVER MIDDLEWARE ====");

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    serverMiddleware.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    Logger::logInfo("===== STOPPING CLIENT MIDDLEWARE ====");

    clientMiddleware.shutdown();

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

}