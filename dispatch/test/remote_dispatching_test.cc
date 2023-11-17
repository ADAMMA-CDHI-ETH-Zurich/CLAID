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
    std::cout << "=== TEST BEGIN ===\n";
    // const std::string address = "localhost:1337";
    // claid::RemoteDispatcherServer server(address);
    // absl::Status status = server.start();

    // SharedQueue<DataPackage> inQueue;
    // SharedQueue<DataPackage> outQueue;
    // const std::string userToken = "User01";
    // const std::string deviceID = "13:37";
    
    // claid::RemoteDispatcherClient client(address, userToken, deviceID, inQueue, outQueue);

    // std::cout << "Checkpoint 1\n";

    // client.registerAtServerAndStartStreaming();
    // std::cout << "Checkpoint 2\n";
    // ASSERT_TRUE(status.ok()) << status;
    // client.shutdown();
    // std::cout << "Checkpoint 3\n";

    // server.shutdown();
}