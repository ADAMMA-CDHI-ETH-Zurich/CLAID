#pragma once

namespace portaible
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