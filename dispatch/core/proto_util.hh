#ifndef PROTO_UTIL_HH_
#define PROTO_UTIL_HH_

#include <string>

#include <google/protobuf/text_format.h>
#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;
namespace claid {
  // Returns a string representation of the protobuf messsage.
  std::string messageToString(const google::protobuf::Message& msg);

  std::string dataPackagePayloadCaseToString(const DataPackage& dataPackage);

}  // namespace claid

#endif  // PROTO_UTIL_HH_
