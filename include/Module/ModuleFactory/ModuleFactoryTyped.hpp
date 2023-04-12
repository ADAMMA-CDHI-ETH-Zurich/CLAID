#pragma once

#include "Module/ModuleFactory/ModuleFactoryBase.hpp"

namespace claid
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