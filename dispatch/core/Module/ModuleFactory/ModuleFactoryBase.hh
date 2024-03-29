
#pragma once

#include "dispatch/core/Module/Module.hh"


#include "dispatch/core/Module/ModuleAnnotator/ModuleAnnotator.hh"

namespace claid
{
	class ModuleFactoryBase
	{
		public:
			virtual Module* getInstanceUntyped() = 0;
			virtual bool getModuleAnnotation(ModuleAnnotator& annotator) = 0;
	};
}