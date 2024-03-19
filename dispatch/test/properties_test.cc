#include "gtest/gtest.h"

#include "dispatch/core/Module/Properties.hh"
#include "dispatch/core/proto_util.hh"

TEST(PropertiesTestSuite, PropertiesTest)  
{
    std::string json = "{\n" 
        "    \"hobbies\": [\"Flying\", \"Motorcycling\", \"Action Movies\"],\n" 
        "    \"isMarried\": true,\n" 
        "    \"contactDetails\": {\n" 
        "      \"phone\": \"555-1234\",\n" 
        "      \"address\": {\n" 
        "        \"street\": \"Hollywood Blvd\",\n" 
        "        \"city\": \"Los Angeles\",\n" 
        "        \"zipCode\": \"90028\"\n" 
        "      }\n" 
        "    }\n" 
        "}"; 

    google::protobuf::Value val;

    google::protobuf::util::JsonParseOptions options2;
    absl::Status status = JsonStringToMessage(json, &val, options2);

    ASSERT_TRUE(status.ok()) << status;

    std::string jsonOutput;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    status = MessageToJsonString(val, &jsonOutput, options);
        ASSERT_TRUE(status.ok()) << status;

    std::cout << "test \n" << jsonOutput << "\n";
}