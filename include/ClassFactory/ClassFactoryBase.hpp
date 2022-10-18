#pragma once

namespace claid
{
	namespace ClassFactory
	{
		class ClassFactoryBase
		{
			public:
				virtual void* getInstanceUntyped() = 0;
		};
	}
}