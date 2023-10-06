#include "dispatch/core/proto_util.hh"

#include <google/protobuf/text_format.h>

std::string claid::messageToString(const google::protobuf::Message& msg) {
    std::string buf;
    if (google::protobuf::TextFormat::PrintToString(msg, &buf)) {
        return buf;
    }
    return "Message not valid (partial content: " +
            msg.ShortDebugString();
}
