#ifndef PROTO_UTIL_HH_
#define PROTO_UTIL_HH_

#include <string>

#include <google/protobuf/text_format.h>

namespace claid {
  // Returns a string representation of the protobuf messsage.
  std::string messageToString(const google::protobuf::Message& msg);

}  // namespace claid

#endif  // PROTO_UTIL_HH_
