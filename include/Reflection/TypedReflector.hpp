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
			r.invokeReflectOnObject(obj);
		}

		virtual void invokeMember(const char* memberName, void* reflector, void* obj)
		{
			return this->invokeMemberTyped(memberName, *static_cast<Reflector*>(reflector), *static_cast<ObjectType*>(obj));
		}

		virtual void invokeMemberTyped(const char* memberName, Reflector& r, ObjectType& obj)
		{
			r.member(memberName, obj, "");
		}

	};
}