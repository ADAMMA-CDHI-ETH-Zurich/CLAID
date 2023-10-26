#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

using namespace claid;

void test()
{
    Mutator<int> mut = TypeMapping::getMutator<int>();
}