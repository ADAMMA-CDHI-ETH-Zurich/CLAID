
// Taken from https://www.theimpossiblecode.com/blog/c11-generic-singleton-pattern/

#pragma once
#include "dispatch/core/Logger/Logger.hh"

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