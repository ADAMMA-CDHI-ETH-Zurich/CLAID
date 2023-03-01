#pragma once

namespace claid
{
	namespace PolymorphicReflector
	{
		template<typename Reflector>
		class WrappedReflectorBase
		{
			public:
				virtual void invoke(Reflector& reflector, void* obj) = 0;
		};
	}
}