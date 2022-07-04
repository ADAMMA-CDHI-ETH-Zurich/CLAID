#pragma once
#include <vector>

// Only difference between the two is, basically, that reflectWrite needs to resize the vector.
// Read means read from data -> for example Serialization (NOT read such as "read from XML").
template<typename Reflector, typename T>
void reflectRead(Reflector& r, std::vector<T>& vector)
{
    r.beginSequence();
    size_t count = vector.size();
    r.count("item", count);

    for(T& val : vector)
    {
        r.member("item", val, "");
    }
    r.endSequence();
}

// Write means write to data -> for example Deserialization (NOT write such as "write to XML").
template<typename Reflector, typename T>
void reflectWrite(Reflector& r, std::vector<T>& vector)
{
    r.beginSequence();
    size_t count;
    // Gets count from the current data.
    r.count("item", count);

    vector = std::vector<T>(count);

    for(size_t i = 0; i < count; i++)
    {
        r.member("item", vector[i], "");
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

