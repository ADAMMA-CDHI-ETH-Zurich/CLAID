#include "gtest/gtest.h"

#include "dispatch/core/Module/Properties.hh"
#include "dispatch/core/proto_util.hh"

#include <fstream>
#include <string>

#include "dispatch/core/Module/Properties.hh"

bool loadFileToString(const std::string& filePath, std::string& buffer);


using namespace claid;
using namespace claidservice;

TEST(PropertiesTestSuite, PropertiesTest)  
{
    std::string json;
    loadFileToString("dispatch/test/properties_test.json", json);

    google::protobuf::Struct val;

    google::protobuf::util::JsonParseOptions options2;
    absl::Status status = JsonStringToMessage(json, &val, options2);
    ASSERT_TRUE(status.ok()) << status;

    Properties properties(val);

    int myInt;
    int myIntAsString;
    double myDouble;
    bool myBool;
    bool myBoolAsString;
    std::string myString;

    ASSERT_TRUE(properties.getNumberProperty("myInt", myInt));
    ASSERT_TRUE(properties.getNumberProperty("myIntAsString", myIntAsString));
    ASSERT_TRUE(properties.getNumberProperty("myDouble", myDouble));
    ASSERT_TRUE(properties.getBoolProperty("myBool", myBool));
    ASSERT_TRUE(properties.getBoolProperty("myBoolAsString", myBoolAsString));

    ASSERT_TRUE(properties.getStringProperty("myString", myString));

    ASSERT_EQ(myInt, 42);
    ASSERT_EQ(myIntAsString, 420);
    ASSERT_EQ(myDouble, 13.37);
    ASSERT_EQ(myBool, true);
    ASSERT_EQ(myBoolAsString, true);
    ASSERT_EQ(myString, "This is a test.");

    PowerSavingStrategyList strategyList;

    ASSERT_TRUE(properties.getObjectProperty("myPowerSavingStrategy", strategyList));

    ASSERT_EQ(strategyList.strategies().size(), 1);
    PowerSavingStrategy strategy = strategyList.strategies()[0];

    ASSERT_EQ(strategy.battery_threshold(), 70.0);
    ASSERT_EQ(strategy.active_modules().size(), 2);
    ASSERT_EQ(strategy.active_modules()[0], "Accelerometer");
    ASSERT_EQ(strategy.active_modules()[1], "Gyroscope");
    ASSERT_EQ(strategy.paused_modules().size(), 1);
    ASSERT_EQ(strategy.paused_modules()[0], "AudioRecorder");

    ASSERT_EQ(strategy.power_profiles().size(), 1);
    ASSERT_NE(strategy.power_profiles().find("Accelerometer"), strategy.power_profiles().end());

    auto it = strategy.power_profiles().find("Accelerometer");
    const PowerProfile& profile = it->second;
    ASSERT_EQ(profile.power_profile_type(), PowerProfileType::POWER_SAVING_MODE);
    ASSERT_EQ(profile.frequency(), 5);

    std::string jsonOutput;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    status = MessageToJsonString(val, &jsonOutput, options);
    ASSERT_TRUE(status.ok()) << status;

}

bool loadFileToString(const std::string& filePath, std::string& buffer)
{
    std::ifstream file(filePath);

    if(!file.is_open())
    {
        std::cout << "Failed to open file\n";
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    buffer = std::string(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    return true;
}