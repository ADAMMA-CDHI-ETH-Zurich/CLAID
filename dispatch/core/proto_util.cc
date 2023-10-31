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

std::string claid::dataPackagePayloadCaseToString(const DataPackage& dataPackage)
{
    const google::protobuf::Descriptor* descriptor = dataPackage.GetDescriptor();
    const google::protobuf::Reflection* reflection = dataPackage.GetReflection();
    const google::protobuf::OneofDescriptor* oneofDescriptor = descriptor->FindOneofByName("payload_oneof");

    for (int i = 0; i < oneofDescriptor->field_count(); i++) {
        const google::protobuf::FieldDescriptor* fieldDescriptor = oneofDescriptor->field(i);
        if (reflection->HasField(dataPackage, fieldDescriptor)) {
            const std::string& fieldName = fieldDescriptor->name();
            return fieldName;
        }
    }
    return "";
}
 