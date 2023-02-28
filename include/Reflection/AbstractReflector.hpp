#pragma once

#include "MemberDetector.h"
#include "NonMemberDetector.h"
#include "NonIntrusiveReflectors/all.hpp"

HAS_MEM_DETECTOR(reflect)
HAS_MEM_DETECTOR(classFactoryRegistrar)
HAS_NON_MEMBER_FUNCTION_WITH_TWO_ARGUMENTS_DETECTOR(void, reflect)


#include "ReflectorInvoker.hpp"

namespace claid
{
    class AbstractReflector
    {
    };
}