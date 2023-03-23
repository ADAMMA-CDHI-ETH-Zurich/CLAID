#pragma once

#include <memory>

namespace claid
{
	class ClassFactoryBase
	{
		public:
			virtual void* getInstanceUntyped() = 0;
			virtual std::shared_ptr<void> getInstanceUntypedAsSharedPtr() = 0;
	};
}