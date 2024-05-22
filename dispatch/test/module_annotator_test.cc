/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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

    void initialize(Properties properties)
    {

    }
};
REGISTER_MODULE(TestModuleWithAnnotationFunction, TestModuleWithAnnotationFunction);

class TestModuleWithoutAnnotationFunction : public Module
{
    void initialize(Properties properties)
    {

    }
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