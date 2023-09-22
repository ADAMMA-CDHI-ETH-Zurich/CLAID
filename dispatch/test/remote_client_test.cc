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
    
    claid::RemoteDispatcherClient client(address, host, userToken, deviceID, inQueue, outQueue);

    std::cout << "Checkpoint 1\n";

    absl::Status status = client.registerAtServerAndStartStreaming();
    std::cout << "Checkpoint 2\n";
    ASSERT_TRUE(status.ok()) << status;
    client.shutdown();
    std::cout << "Checkpoint 3\n";

}