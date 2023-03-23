#pragma once

#include "ClassFactoryBase.hpp"

namespace claid
{
	template<typename T>
	class ClassFactoryTyped : public ClassFactoryBase
	{
		public:
			void* getInstanceUntyped()
			{
				return static_cast<void*>(new T);
			}

			std::shared_ptr<void> getInstanceUntypedAsSharedPtr()
			{
				return std::static_pointer_cast<void>(std::make_shared<T>());
			}

			T* getInstance()
			{
				return new T;
			}

	};
}