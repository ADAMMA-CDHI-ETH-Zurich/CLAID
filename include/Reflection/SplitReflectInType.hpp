#pragma once

#include <type_traits>
// Invoke reflectWrite
template<typename Reflector, typename Type, typename Enable = void>
struct ClassReflectReadWriteInvoker 
{
	static void call(Reflector& r, Type& obj) 
	{
		// Cannot put this here, because windows MSCV is ... not so smart.
		// Somehow it evaluates the default type for all the types, meaning it would complain, for example,
		// about the BinarySerializer not having a "read" function, as used in reflectRead(...) of some types that use splitReflect() (e.g., BinaryData).
		// The BinarySerializer is a Serializer (obv.), ths isReadOnly is true. Therefore, reflectWrite would be called.
		// However, if we put reflectRead here, MSCV would still evaluate it..
		// clang and gcc, as usual, do not complain.
		// obj.reflectRead<Reflector>(r);
		obj.reflectRead<Reflector>(r);
	}
}; 

// Invoke reflectRead
template<typename Reflector, typename Type>
struct ClassReflectReadWriteInvoker<Reflector, Type, typename std::enable_if<Reflector::isReadOnly>::type> 
{
	static void call(Reflector& r, Type& obj) 
	{
		obj.reflectWrite<Reflector>(r);
	}
};

// Invoke reflectRead
template<typename Reflector, typename Type>
struct ClassReflectReadWriteInvoker<Reflector, Type, typename std::enable_if<!Reflector::isReadOnly>::type> 
{
	static void call(Reflector& r, Type& obj) 
	{
		obj.reflectRead<Reflector>(r);
	}
};


template<typename Reflector, typename Type>
inline void splitReflectInType(Reflector& r, Type& obj)
{
	ClassReflectReadWriteInvoker<Reflector, Type>::call(r, obj);
}