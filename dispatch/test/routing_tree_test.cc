
#include "gtest/gtest.h"
#include "dispatch/core/Router/RoutingTree.hh"
#include "dispatch/core/Router/RoutingTreeParser.hh"
#include "dispatch/core/Configuration/Configuration.hh"
#include "tools/cpp/runfiles/runfiles.h"

using namespace claid;

inline HostDescription makeHostDescription(const std::string& hostname, bool isServer, const std::string& hostServerAddress, const std::string& connectTo)
{
    HostDescription description;
    description.hostname = hostname;
    description.isServer = isServer;
    description.hostServerAddress = hostServerAddress;
    description.connectTo = connectTo;
    return description;
}

inline void addHostDescription(HostDescriptionMap& map, const std::string& hostname, bool isServer, const std::string& hostServerAddress, const std::string& connectTo)
{
    map.insert(std::make_pair(hostname, makeHostDescription(hostname, isServer, hostServerAddress, connectTo)));
}

TEST(RoutingTreeTestSuite, RoutingTreeTest)
{
    RoutingTree routingTree;
    RoutingTreeParser parser;  

    HostDescriptionMap hostDescriptions;
    addHostDescription(hostDescriptions, "Host1", true, "", "");
    
    absl::Status status = parser.buildRoutingTree(hostDescriptions, routingTree);
    ASSERT_EQ(status.ok(), true) << status;
} 
