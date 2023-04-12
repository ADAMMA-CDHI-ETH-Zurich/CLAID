#pragma once
#include "UntypedReflector.hpp"


namespace claid
{
	template<typename ObjectType, typename Reflector>
	class TypedReflector : public UntypedReflector
	{

		virtual void invoke(void* reflector, void* obj) 
		{
			this->invokeTyped(*static_cast<Reflector*>(reflector), *static_cast<ObjectType*>(obj));
		}

		virtual void invokeTyped(Reflector& r, ObjectType& obj)
		{
			r.invokeReflectorOnClassThatHasReflectFunction(obj);
		}

		virtual void invokeMember(const char* memberName, void* reflector, void* obj)
		{
			return this->invokeMemberTyped(memberName, *static_cast<Reflector*>(reflector), *static_cast<ObjectType*>(obj));
		}

		virtual void invokeMemberTyped(const char* memberName, Reflector& r, ObjectType& obj)
		{
			r.member(memberName, obj, "");
		}

		virtual void invokeMemberWithDefaultValue(const char* memberName, void* reflector, void* obj, void* defaultValue)
		{
			return this->invokeMemberWithDefaultValueTyped(memberName, *static_cast<Reflector*>(reflector), *static_cast<ObjectType*>(obj), *static_cast<ObjectType*>(defaultValue));
		}

		virtual void invokeMemberWithDefaultValueTyped(const char* memberName, Reflector& r, ObjectType& obj, ObjectType& defaultValue)
		{
			r.member(memberName, obj, "", defaultValue);
		}

	};
}