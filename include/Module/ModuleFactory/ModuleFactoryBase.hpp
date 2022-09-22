#pragma once

#include "RunTime/RunTime.hpp"

namespace portaible
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