#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

#include <vector>
#include <string>
#include <map>

using namespace claid;


void test()
{
    DataPackage myMessage;
    myMessage.set_string_val("blablablaba");
    myMessage.set_number_val(42);
    const google::protobuf::Descriptor* descriptor = myMessage.GetDescriptor();
    const google::protobuf::Reflection* reflection = myMessage.GetReflection();

        const google::protobuf::OneofDescriptor* oneofDescriptor = descriptor->FindOneofByName("payload_oneof");

        for (int i = 0; i < oneofDescriptor->field_count(); i++) {
            const google::protobuf::FieldDescriptor* fieldDescriptor = oneofDescriptor->field(i);
            if (reflection->HasField(myMessage, fieldDescriptor)) {
                const std::string& fieldName = fieldDescriptor->name();
                printf("FieldName %s\n", fieldName.c_str());
                // You now have the name of the chosen oneof field
                break;
            }
        }
    
}

int main()
{
    auto mut = TypeMapping::getMutator<int>();
    auto mut2 = TypeMapping::getMutator<std::vector<int>>();
    auto mut3 = TypeMapping::getMutator<std::vector<std::string>>();
    auto mut4 = TypeMapping::getMutator<std::map<std::string, double>>();
    auto mut5 = TypeMapping::getMutator<std::map<std::string, std::string>>();

    test();
    return 0;
}