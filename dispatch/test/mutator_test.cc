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

#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

#include <string>
#include <vector>
#include <map>

using namespace claid;


// Tests if all available Mutators can be implemented using the template specialization.
TEST(MutatorTestSuite, MutatorTest) {
    // numberVal
    auto mut = TypeMapping::getMutator<unsigned short>();
    auto mut1 = TypeMapping::getMutator<signed short>();
    auto mut2 = TypeMapping::getMutator<unsigned int>();
    auto mut3 = TypeMapping::getMutator<signed int>();
    auto mut4 = TypeMapping::getMutator<unsigned long>();
    auto mut5 = TypeMapping::getMutator<signed long>();
    auto mut6 = TypeMapping::getMutator<unsigned long long>();
    auto mut7 = TypeMapping::getMutator<signed long long>();
    auto mut8 = TypeMapping::getMutator<float>();
    auto mut9 = TypeMapping::getMutator<double>();
    
    auto mut10 = TypeMapping::getMutator<std::string>();
    auto mut11 = TypeMapping::getMutator<bool>();

    // numberArray
    auto mut12 = TypeMapping::getMutator<std::vector<unsigned short>>();
    auto mut13 = TypeMapping::getMutator<std::vector<signed short>>();
    auto mut14 = TypeMapping::getMutator<std::vector<unsigned int>>();
    auto mut15 = TypeMapping::getMutator<std::vector<signed int>>();
    auto mut16 = TypeMapping::getMutator<std::vector<unsigned long>>();
    auto mut17 = TypeMapping::getMutator<std::vector<signed long>>();
    auto mut18 = TypeMapping::getMutator<std::vector<unsigned long long>>();
    auto mut19 = TypeMapping::getMutator<std::vector<signed long long>>();
    auto mut20 = TypeMapping::getMutator<std::vector<float>>();
    auto mut21 = TypeMapping::getMutator<std::vector<double>>();


    // stringArray
    auto mut22 = TypeMapping::getMutator<std::vector<std::string>>();


    // numberMap
    auto mut23 = TypeMapping::getMutator<std::map<std::string, unsigned short>>();
    auto mut24 = TypeMapping::getMutator<std::map<std::string, signed short>>();
    auto mut25 = TypeMapping::getMutator<std::map<std::string, unsigned int>>();
    auto mut26 = TypeMapping::getMutator<std::map<std::string, signed int>>();
    auto mut27 = TypeMapping::getMutator<std::map<std::string, unsigned long>>();
    auto mut28 = TypeMapping::getMutator<std::map<std::string, signed long>>();
    auto mut29 = TypeMapping::getMutator<std::map<std::string, unsigned long long>>();
    auto mut30 = TypeMapping::getMutator<std::map<std::string, signed long long>>();
    auto mut31 = TypeMapping::getMutator<std::map<std::string, float>>();
    auto mut32 = TypeMapping::getMutator<std::map<std::string, double>>();
    
    // stringMap
    auto mut33 = TypeMapping::getMutator<std::map<std::string, std::string>>();

    auto mut34 = TypeMapping::getMutator<NumberMap>();

    auto mut35 = TypeMapping::getMutator<AnyProtoType>();

}


TEST(MutatorTestSuite, ProtoMutateTest) {
    auto mutator = TypeMapping::getMutator<NumberArray>();

    DataPackage package;

    // NumberArray is a protobuf type.
    // Normally, you wouldn't use NumberArray directly, because the Mutator
    // automatically converts between vector and NumberArray.
    // If using NumberArray directly, the Mutator will serialize it as proto message using ProtoCodec.

    NumberArray protoTestVal;
    protoTestVal.add_val(42);
    protoTestVal.add_val(1337);

    mutator.setPackagePayload(package, protoTestVal);

    NumberArray returnVal;
    mutator.getPackagePayload(package, returnVal);

    ASSERT_EQ(returnVal.val_size(), 2) << "Expected 2 elements in deserialized NumberArray";

    ASSERT_EQ(returnVal.val(0), 42) << "Expected value 42";
    ASSERT_EQ(returnVal.val(1), 1337) << "Expected value 1337";


    
}

TEST(MutatorTestSuite, AnyProtoTest) {
    auto mutator = TypeMapping::getMutator<AnyProtoType>();

    DataPackage package;

    // NumberArray is a protobuf type.
    // Normally, you wouldn't use NumberArray directly, because the Mutator
    // automatically converts between vector and NumberArray.
    // If using NumberArray directly, the Mutator will serialize it as proto message using ProtoCodec.

    std::shared_ptr<NumberArray> protoTestVal(new NumberArray);
    protoTestVal->add_val(42);
    protoTestVal->add_val(1337);

    mutator.setPackagePayload(package, AnyProtoType(protoTestVal));

    AnyProtoType returnVal;
    mutator.getPackagePayload(package, returnVal);


    auto message = std::static_pointer_cast<const NumberArray>(returnVal.getMessage());


    ASSERT_EQ(message->val_size(), 2) << "Expected 2 elements in deserialized NumberArray";

    ASSERT_EQ(message->val(0), 42) << "Expected value 42";
    ASSERT_EQ(message->val(1), 1337) << "Expected value 1337";



}