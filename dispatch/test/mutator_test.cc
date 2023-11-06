#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <map>

using namespace claid;


// Tets if all available Mutators can be implemented using the template specialization.
TEST(MutatorTestSuite, MutatorTest) {
    // numberVal
    auto mut = TypeMapping::getMutator<int>();

    // numberArray
    auto mut2 = TypeMapping::getMutator<std::vector<int>>();

    // stringArray
    auto mut3 = TypeMapping::getMutator<std::vector<std::string>>();

    // numberMap
    auto mut4 = TypeMapping::getMutator<std::map<std::string, double>>();

    // stringMap
    auto mut5 = TypeMapping::getMutator<std::map<std::string, std::string>>();

    // TODO: PROTO
}
