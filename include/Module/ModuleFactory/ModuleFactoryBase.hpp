#pragma once

#include "RunTime/RunTime.hpp"

namespace claid
{
	namespace ModuleFactory
	{
		class ModuleFactoryBase
		{
			public:
				virtual BaseModule* getInstanceUntyped() = 0;
		};
	}
}