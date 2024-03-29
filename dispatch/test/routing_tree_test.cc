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
   absl::Status status = map.insert(std::make_pair(hostname, makeHostDescription(hostname, isServer, hostServerAddress, connectTo)));
   ASSERT_EQ(status.ok(), true) << status;
}

inline void addClientsToServer(HostDescriptionMap& map, const std::string& server, const std::vector<std::string>& clients)
{
    for(const std::string& client : clients)
    {
        addHostDescription(map, client, false, "", server);
    }
}

inline void assertAllChildsMatch(RoutingTree& tree, const std::string& host, const std::vector<std::string>& expectedChilds)
{
    RoutingNode* node = tree.lookupHost(host);
    ASSERT_NE(node, nullptr) << "Did not find host \"" << host << "\" in routing tree.";
    RoutingTree subTree(node);

    std::vector<std::string> actualChilds;
    subTree.getChildHostRecursively(actualChilds);

    ASSERT_EQ(actualChilds.size(), expectedChilds.size()) << 
        "Number of children expected for host \"" << 
        host << "\" did not match actual number of children (expected: " << expectedChilds.size() << ", actual: " << actualChilds.size() << ")";

    std::vector<std::string> expectedChildsCopy = expectedChilds;

    std::sort(actualChilds.begin(), actualChilds.end());
    std::sort(expectedChildsCopy.begin(), expectedChildsCopy.end());

    for(int i = 0; i < actualChilds.size(); i++)
    {
        ASSERT_EQ(actualChilds[i], expectedChildsCopy[i]) << "Found unexpected child \"" << actualChilds[i] << "\" for host \"" << host << "\"";
    }
}

inline void assertVectorMatchExactly(const std::vector<std::string>& first, const std::vector<std::string>& second)
{
    ASSERT_EQ(first.size(), second.size()) << "Vectors do not match, different number of elements: " << first.size() << " vs. " << second.size();
    
    for(size_t i = 0; i < first.size(); i++)
    {
        ASSERT_EQ(first[i], second[i]) << "Vectors do not match, elements at index " << i << " differ: " << first[i] << " vs. " << second[i];
    }
}

std::vector<std::string> mergeVectors(const std::vector<vector<std::string>>& in)
{
    std::vector<std::string> out;

    for(const std::vector<std::string>& tmp : in)
    {
        for(const std::string& str : tmp)
        {
            out.push_back(str);
        }
    }

    return out;
}

void printChilds(RoutingTree& routingTree, const std::string& targetHost)
{
    RoutingNode* node = routingTree.lookupHost(targetHost);
    ASSERT_NE(node, nullptr) << "Did not find host \"" << targetHost << "\" in routing tree.";
    RoutingTree subTree(node);
 
    std::string out; 
    subTree.toString(out);
    std::cout << out << "\n";
}

template<typename T>
void printVector(const std::vector<T> vector)
{
    for(const T& val : vector)
    {
        std::cout << val << "\n";
    }
}

// Tests whether the routing three is build correctly when specifying multiple hosts
TEST(RoutingTreeTestSuite, RoutingTreeTest)
{
    RoutingTree routingTree;
    RoutingTreeParser parser;  

    HostDescriptionMap hostDescriptions;
    addHostDescription(hostDescriptions, "Server1", true, "claid1.ethz.ch", "");
    addHostDescription(hostDescriptions, "Server2", true, "claid2.ethz.ch", "Server1");
    addHostDescription(hostDescriptions, "Server3", true, "claid3.claid.ethz.ch", "Server2");
    addHostDescription(hostDescriptions, "Server4", true, "claid4.claid.ethz.ch", "Server2");
    addHostDescription(hostDescriptions, "Server5", true, "claid5.claid.ethz.ch", "Server4");

    std::vector<std::string> server1Childs = {"Client1", "Client2", "Client3"};
    std::vector<std::string> server2Childs = {"Client4", "Client5"};
    std::vector<std::string> server3Childs = {"Client6"};
    std::vector<std::string> server4Childs = {"Client7", "Client8", "Client9", "Client10"};
    std::vector<std::string> server5Childs = {"Client11", "Client12", "Client13", "Client14"};

    addClientsToServer(hostDescriptions, "Server1", server1Childs);
    addClientsToServer(hostDescriptions, "Server2", server2Childs);
    addClientsToServer(hostDescriptions, "Server3", server3Childs);
    addClientsToServer(hostDescriptions, "Server4", server4Childs);
    addClientsToServer(hostDescriptions, "Server5", server5Childs);

    absl::Status status = parser.buildRoutingTree(hostDescriptions, routingTree);

    ASSERT_EQ(status.ok(), true) << status;

    assertAllChildsMatch(routingTree, "Server5", server5Childs);
    assertAllChildsMatch(routingTree, "Server4", mergeVectors({{"Server5"}, server4Childs, server5Childs}));
    assertAllChildsMatch(routingTree, "Server3", server3Childs);
    assertAllChildsMatch(routingTree, "Server2", mergeVectors({{"Server3", "Server4", "Server5"}, server2Childs, server3Childs, server4Childs, server5Childs}));
    assertAllChildsMatch(routingTree, "Server1", mergeVectors({{"Server2", "Server3", "Server4", "Server5"}, server1Childs, server2Childs, server3Childs, server4Childs, server5Childs}));

    std::vector<std::string> path;
    RoutingDirection routingDirection;
    bool success = routingTree.getRouteFromHostToHost("Client6", "Client11", path, routingDirection);
    ASSERT_TRUE(success) << "Failed to get path from Client6 to Client11";
    assertVectorMatchExactly(path, {"Server3", "Server2", "Server4", "Server5", "Client11"});

    success = routingTree.getRouteFromHostToHost("Server1", "Client14", path, routingDirection);
    ASSERT_TRUE(success) << "Failed to get path from Server1 to Client14";
    assertVectorMatchExactly(path, {"Server2", "Server4", "Server5", "Client14"});

    success = routingTree.getRouteFromHostToHost("Client8", "Client3", path, routingDirection);
    ASSERT_TRUE(success) << "Failed to get path from Client8 to Client3";
    assertVectorMatchExactly(path, {"Server4", "Server2", "Server1", "Client3"});

    success = routingTree.getRouteFromHostToHost("Client1", "Client3", path, routingDirection);
    ASSERT_TRUE(success) << "Failed to get path from Client1 to Client3";
    assertVectorMatchExactly(path, {"Server1", "Client3"});
} 

// Tests that no two hosts can become the master host (root of the tree).
// If a host is declared as server and does not have a connectTo address specified,
// it has to be the root of the tree as it does not connect to another server.
// This is only allowed for ONE host. Only the root server is allowed to not connect to other servers.
TEST(RoutingTreeTestSuite, NoTwoMasterHostsTest)
{
    RoutingTree routingTree;
    RoutingTreeParser parser;  

    HostDescriptionMap hostDescriptions;
    addHostDescription(hostDescriptions, "Host1", true, "claid.ethz.ch", "");
    addHostDescription(hostDescriptions, "Host2", true, "other.claid.ethz.ch", "");
    addHostDescription(hostDescriptions, "Host3", false, "", "claid.ethz.ch");
    addHostDescription(hostDescriptions, "Host4", false, "", "other.claid.ethz.ch");
    addHostDescription(hostDescriptions, "Host5", false, "", "other.claid.ethz.ch");
    addHostDescription(hostDescriptions, "Host6", false, "", "claid.ethz.ch");
    

    absl::Status status = parser.buildRoutingTree(hostDescriptions, routingTree);
    ASSERT_NE(status.ok(), true) << status;
} 

// Verifies that it is not allowed to add two Hosts with the same name to the HostDescriptionMap
TEST(RoutingTreeTestSuite, UniqueHostNameTest)
{
    HostDescriptionMap hostDescriptions;
    absl::Status status = hostDescriptions.insert(std::make_pair("Host1", HostDescription("Host1", true, "claid.ethz.ch", "")));
    status.Update(hostDescriptions.insert(std::make_pair("Host2", HostDescription("Host2", true, "1.claid.ethz.ch", "claid.ethz.ch"))));
    status.Update(hostDescriptions.insert(std::make_pair("Host2", HostDescription("Host2", true, "claid.ethz.ch", "claid.ethz.ch"))));

    ASSERT_FALSE(status.ok()) << "Adding two hosts to the HostDescriptionMap did not yield an error.";
} 

// Verifies that the RoutingTreeParser rejects hosts that are a server with the same address.
TEST(RoutingTreeTestSuite, UniqueAddressTest)
{
    HostDescriptionMap hostDescriptions;
    absl::Status status = hostDescriptions.insert(std::make_pair("Host1", HostDescription("Host1", true, "claid.ethz.ch", "")));
    status.Update(hostDescriptions.insert(std::make_pair("Host2", HostDescription("Host2", true, "1.claid.ethz.ch", "claid.ethz.ch"))));
    status.Update(hostDescriptions.insert(std::make_pair("Host3", HostDescription("Host3", true, "claid.ethz.ch", "claid.ethz.ch"))));

    RoutingTree routingTree;
    RoutingTreeParser parser;  
    status.Update(parser.buildRoutingTree(hostDescriptions, routingTree));

    ASSERT_FALSE(status.ok()) << "RoutingTreeParser accepted to build a routing tree containing two hosts with the same address.";
} 

// Verifies that the RoutingTreeParser rejects hosts that connect to themselves.
TEST(RoutingTreeTestSuite, NoSelfLoopTest)
{
    HostDescriptionMap hostDescriptions;
    absl::Status status = hostDescriptions.insert(std::make_pair("Host1", HostDescription("Host1", true, "claid.ethz.ch", "Host1")));

    RoutingTree routingTree;
    RoutingTreeParser parser;  
    status.Update(parser.buildRoutingTree(hostDescriptions, routingTree));

    ASSERT_FALSE(status.ok()) << "A host that connects to itself should be rejected by the RoutingTreeParser, but it was not.";
}