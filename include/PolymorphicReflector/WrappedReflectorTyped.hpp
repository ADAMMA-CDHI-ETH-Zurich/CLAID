#pragma once
#include "WrappedReflectorBase.hpp"


namespace claid
{
	namespace PolymorphicReflector
	{
		template<typename ObjectType, typename Reflector>
		class WrappedReflectorTyped : public WrappedReflectorBase<Reflector>
		{

			virtual void invoke(Reflector& reflector, void* obj) 
			{
				this->invokeTyped(reflector, *static_cast<ObjectType*>(obj));
			}

			virtual void invokeTyped(Reflector& r, ObjectType& obj)
			{
				obj.reflect(r);
			}
		};
	}
}