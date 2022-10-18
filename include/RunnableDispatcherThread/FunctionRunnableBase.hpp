#pragma once

#include "Runnable.hpp"

namespace claid
{
    class FunctionRunnableBase : public Runnable
    {
        virtual void run() = 0;
    };
}