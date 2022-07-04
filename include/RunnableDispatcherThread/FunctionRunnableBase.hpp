#pragma once

#include "Runnable.hpp"

namespace portaible
{
    class FunctionRunnableBase : public Runnable
    {
        virtual void run() = 0;
    };
}