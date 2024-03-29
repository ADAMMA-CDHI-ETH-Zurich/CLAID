
#pragma once

#include "Runnable.hh"

namespace claid
{
    class FunctionRunnableBase : public Runnable
    {
        virtual void run() = 0;
    };
}