
#pragma once

#include "dispatch/core/Module/ModuleFactory/ModuleFactoryBase.hh"



namespace claid
{


	template <typename T>
	struct has_annotate_module_function {
		template <typename U, typename = decltype(&U::annotateModule)>
		static std::true_type test(int);

		template <typename>
		static std::false_type test(...);

		static constexpr bool value = decltype(test<T>(0))::value;
	};


	template<typename T, class Enable = void>
	struct __AnnotateModuleFunctionInvoker 
	{
		static bool call(ModuleAnnotator& annotator) 
		{
			return false;
		}

	}; 

	template<class T>
	struct __AnnotateModuleFunctionInvoker<T, typename std::enable_if<has_annotate_module_function<T>::value>::type> 
	{
		static bool call(ModuleAnnotator& annotator) 
		{
			T::annotateModule(annotator);
			return true;
		}
	};    


	template<typename T>
	class ModuleFactoryTyped : public ModuleFactoryBase
	{
		public:
			Module* getInstanceUntyped() override final
			{
				return static_cast<Module*>(new T);
			}

			T* getInstance()
			{
				return new T;
			}

			bool getModuleAnnotation(ModuleAnnotator& annotator) override final
			{
				return __AnnotateModuleFunctionInvoker<T>::call(annotator);
			}

	};
}
