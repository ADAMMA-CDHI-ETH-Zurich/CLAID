#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/RemoteFunction/RemoteFunction.hh"

using claidservice::DataPackage;

#include <string>
#include <vector>
#include <map>

using namespace claid;


// Tests if all available Mutators can be implemented using the template specialization.
TEST(RemoteFunctionTestSuite, RemoteFunctionTest) 
{   
    FutureHandler futuresHandler;
    SharedQueue<DataPackage> queue;
    RemoteFunctionIdentifier identifier;

   RemoteFunction<T, int, int> function(futuresHandler, queue, identifier);
}