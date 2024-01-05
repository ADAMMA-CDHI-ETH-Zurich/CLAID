#include "gtest/gtest.h"

#include "dispatch/core/CLAID.hh"

using claidservice::DataPackage;

using namespace claid;

class TestModuleWithExpectedProperties : public Module
{
    public:
    static void defineExpectedProperties(std::map<std::string, std::string>& properties)
    {
        std::cout << "Testcall\n" << std::flush;
        properties["val1"] = "42";
        properties["otherVal"] = "1337";
    }
};
REGISTER_MODULE(TestModuleWithExpectedProperties, TestModuleWithExpectedProperties);

class TestModuleWithoutExpectedProperties : public Module
{
    
};
REGISTER_MODULE(TestModuleWithoutExpectedProperties, TestModuleWithoutExpectedProperties);

// Tests if all available Mutators can be implemented using the template specialization.
TEST(ExpectedPropertiesTestSuite, ExpectedPropertiesTest) {

    std::map<std::string, std::string> expectedProperties;
    ASSERT_FALSE(ModuleFactory::getInstance()->getExpectedPropertiesOfModule("TestModuleWithoutExpectedProperties", expectedProperties));
    ASSERT_EQ(expectedProperties.size(), 0);

    ASSERT_TRUE(ModuleFactory::getInstance()->getExpectedPropertiesOfModule("TestModuleWithExpectedProperties", expectedProperties));
    ASSERT_EQ(expectedProperties.size(), 2);

    ASSERT_EQ(expectedProperties["val1"], "42");
    ASSERT_EQ(expectedProperties["otherVal"], "1337");
}