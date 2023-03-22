#pragma once

#include "ClassFactoryBase.hpp"

namespace claid
{
	template<typename T>
	class ClassFactoryTyped : public ClassFactoryBase
	{
		public:
			void* getInstanceUntyped()
			{
				return static_cast<void*>(new T);
			}

			T* getInstance()
			{
				return new T;
			}

	};
}