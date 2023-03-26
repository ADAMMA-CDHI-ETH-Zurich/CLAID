#pragma once

#include "ClassFactoryBase.hpp"
#include <type_traits>
#include "Exception/Exception.hpp"
namespace claid
{
	template<typename T>
	class ClassFactoryTyped : public ClassFactoryBase
	{
		private:
			template<typename U = T>
			typename std::enable_if<!std::is_abstract<U>::value, void*>::type
				getInstanceUntypedHelper()
			{
				return static_cast<void*>(new T);
			}

			template<typename U = T>
			typename std::enable_if<std::is_abstract<U>::value, void*>::type
				getInstanceUntypedHelper()
			{
				CLAID_THROW(Exception, "Error, cannot create instance of class \"" << ClassFactory::getInstance()->getClassName<U>() << "\" at runtime using ClassFactory."
					<< "This class is an abstract class.");
			}

			template<typename U = T>
			typename std::enable_if<!std::is_abstract<U>::value, std::shared_ptr<void>>::type
				getInstanceUntypedAsSharedPtrHelper()
			{
				return std::static_pointer_cast<void>(std::make_shared<T>());
			}

			template<typename U = T>
			typename std::enable_if<std::is_abstract<U>::value, std::shared_ptr<void>>::type
				getInstanceUntypedAsSharedPtrHelper()
			{
				CLAID_THROW(Exception, "Error, cannot create instance of class \"" << ClassFactory::getInstance()->getClassName<U>() << "\" at runtime using ClassFactory."
					<< "This class is an abstract class.");
			}

		public:

			void* getInstanceUntyped()
			{
				return getInstanceUntypedHelper<T>();
			}


			std::shared_ptr<void> getInstanceUntypedAsSharedPtr()
			{
				return getInstanceUntypedAsSharedPtrHelper<T>();
			}

			T* getInstance()
			{
				return new T;
			}

	};
}