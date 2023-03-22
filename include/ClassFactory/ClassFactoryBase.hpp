#pragma once

namespace claid
{
	class ClassFactoryBase
	{
		public:
			virtual void* getInstanceUntyped() = 0;
	};
}