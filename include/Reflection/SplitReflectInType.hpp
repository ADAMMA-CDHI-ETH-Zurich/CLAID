#pragma once

#include <type_traits>
// Invoke reflectWrite
template<typename Reflector, typename Type, class Enable = void>
struct ClassReflectReadWriteInvoker 
{
	static void call(Reflector& r, Type& obj) 
	{
		obj.reflectRead(r);
	}
}; 

// Invoke reflectRead
template<typename Reflector, typename Type>
struct ClassReflectReadWriteInvoker<Reflector, Type, typename std::enable_if<Reflector::isReadOnly>::type> 
{
	static void call(Reflector& r, Type& obj) 
	{
		obj.reflectWrite(r);
	}
};


template<typename Reflector, typename Type>
inline void splitReflectInType(Reflector& r, Type& obj)
{
	ClassReflectReadWriteInvoker<Reflector, Type>::call(r, obj);
}