#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"
#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <map>

using namespace claid;


// Tets if all available Mutators can be implemented using the template specialization.
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

    // TODO: PROTO
}
