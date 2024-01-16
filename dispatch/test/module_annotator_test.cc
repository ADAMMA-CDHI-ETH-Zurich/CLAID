#include "gtest/gtest.h"

#include "dispatch/core/CLAID.hh"

using claidservice::DataPackage;

using namespace claid;

class TestModuleWithAnnotationFunction : public Module
{
    public:
    static void annotateModule(ModuleAnnotator& annotator)
    {
        // std::cout << "Testcall\n" << std::flush;
        // properties["val1"] = "42";
        // properties["otherVal"] = "1337";
    }
};
REGISTER_MODULE(TestModuleWithAnnotationFunction, TestModuleWithAnnotationFunction);

class TestModuleWithoutAnnotationFunction : public Module
{
    
};
REGISTER_MODULE(TestModuleWithoutAnnotationFunction, TestModuleWithoutAnnotationFunction);

// Tests if all available Mutators can be implemented using the template specialization.
TEST(ExpectedPropertiesTestSuite, ExpectedPropertiesTest) {

    ModuleAnnotator annotator("TestModuleWithAnnotationFunction");
    ASSERT_TRUE(ModuleFactory::getInstance()->getModuleAnnotation("TestModuleWithAnnotationFunction", annotator));
   // ASSERT_EQ(expectedProperties.size(), 0);

    annotator = ModuleAnnotator("TestModuleWithoutAnnotationFunction");
    ASSERT_FALSE(ModuleFactory::getInstance()->getModuleAnnotation("TestModuleWithoutAnnotationFunction", annotator));
    //ASSERT_EQ(expectedProperties.size(), 2);

    // ASSERT_EQ(expectedProperties["val1"], "42");
    // ASSERT_EQ(expectedProperties["otherVal"], "1337");
}