#pragma once
#include <vector>
#include "Logger/Logger.hpp"

// Only difference between the two is, basically, that reflectWrite needs to resize the vector.
template<typename Reflector, typename T>
void reflectRead(Reflector& r, std::vector<T>& vector)
{
    r.beginSequence();
    size_t count;
    // Gets count from the current data.
    r.count("item", count);

    vector = std::vector<T>(count);

    printf("vector start\n");
    for(size_t i = 0; i < count; i++)
    {
        r.itemIndex(i);
        r.member("item", vector[i], "");
    }
    printf("vector end");
    r.endSequence();
}

template<typename Reflector, typename T>
void reflectWrite(Reflector& r, std::vector<T>& vector)
{
    r.beginSequence();
    size_t count = vector.size();
    r.count("item", count);

    for(size_t i = 0; i < vector.size(); i++)
    {
        T& val = vector[i];
        r.itemIndex(i);
        r.member("item", val, "");
    }
    r.endSequence();
}

// SplitReflect needs to be included after (!!) defining reflectRead and reflectWrite for the type ! 
#include "Reflection/SplitReflect.hpp"

template<typename Reflector, typename T>
void reflect(Reflector& r, std::vector<T>& vector)
{
    splitReflect(r, vector);
}

