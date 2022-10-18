// Taken from https://www.theimpossiblecode.com/blog/c11-generic-singleton-pattern/

#pragma once
#include "Logger/Logger.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"

template <typename T>
class Singleton
{
public:


    static T *getInstance()
    {
        if(mpInstance == nullptr)
        {
            mpInstance = new T();
        }
        if(claid::TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() == "claid::RunTime")
        {
           // claid::Logger::printfln("%s", claid::TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>().c_str());
        }
        
        return mpInstance;
    }

protected:
    Singleton()
    {
       
    }


private:
    static T* mpInstance;
};


template <typename T>
T* Singleton<T>::mpInstance = nullptr;