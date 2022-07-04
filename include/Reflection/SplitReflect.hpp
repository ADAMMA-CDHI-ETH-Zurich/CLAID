#pragma once

#include <type_traits>
// Invoke reflectWrite
template<typename Reflector, typename Type, class Enable = void>
struct ReflectReadWriteInvoker 
{
	static void call(Reflector& r, Type& member) 
	{
		reflectWrite(r, member);
	}

}; 

// Invoke reflectRead
template<typename Reflector, typename Type>
struct ReflectReadWriteInvoker<Reflector, Type, typename std::enable_if<Reflector::isReadOnly>::type> 
{
	static void call(Reflector& r, Type& member) 
	{
		reflectRead(r, member);
	}
};


template<typename Reflector, typename Type>
inline void splitReflect(Reflector& r, Type& member)
{
	ReflectReadWriteInvoker<Reflector, Type>::call(r, member);
}