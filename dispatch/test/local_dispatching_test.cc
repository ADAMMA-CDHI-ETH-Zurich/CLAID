#include "dispatch/core/local_dispatching.hh"
#include "gtest/gtest.h"

using namespace std; 
using namespace claid; 

TEST(LocalDispatcherTestSuit, SocketBasedDispatcherTest) {
    // TODO: remove this from being hard coded
    string addr = "/tmp/claidtest/grpc.socket";
    
    // TODO: The module table needs to be populated. 
    ModuleTable modTable; 
    DispatcherServer server(addr, modTable);

    // TODO: complete this test ! 
}
