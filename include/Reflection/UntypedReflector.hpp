#pragma once

namespace claid
{
	class UntypedReflector
	{
		public:
			virtual void invoke(void* reflector, void* obj) = 0;

			template<typename Reflector, typename Class>
			void invoke(Reflector& reflector, Class& object)
			{
				return invoke(static_cast<void*>(reflector), static_cast<void*>(object));
			}

			virtual void invokeMember(const char* memberName, void* reflector, void* obj) = 0;

			template<typename Reflector, typename Class>
			void invokeMember(const char* memberName, Reflector& reflector, Class& object)
			{
				return invokeMember(memberName, static_cast<void*>(reflector), static_cast<void*>(object));
			}
	};
}