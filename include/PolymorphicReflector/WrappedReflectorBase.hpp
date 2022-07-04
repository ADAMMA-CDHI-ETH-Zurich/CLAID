#pragma once

namespace portaible
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