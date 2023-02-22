#pragma once

#include "RunTime/RunTime.hpp"

namespace claid
{
	class ModuleFactoryBase
	{
		public:
			virtual BaseModule* getInstanceUntyped() = 0;
	};
}