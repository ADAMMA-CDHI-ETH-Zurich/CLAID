#pragma once

#include "ModuleFactoryBase.hpp"

namespace claid
{
	namespace ModuleFactory
	{
		template<typename T>
		class ModuleFactoryTyped : public ModuleFactoryBase
		{
			public:
				BaseModule* getInstanceUntyped()
				{
					return static_cast<BaseModule*>(new T);
				}

				T* getInstance()
				{
					return new T;
				}
		};
	}
}