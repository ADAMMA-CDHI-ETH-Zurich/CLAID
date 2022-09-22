#pragma once
#define _PORTAIBLE_RUNTIME_H_

#include <iostream>

#include <stdio.h>
#include "Utilities/Singleton.hpp"
#include "Channel/ChannelManager.hpp"

#include "Module/Module.hpp"
#include "XMLLoader/XMLLoaderManager.hpp"

#define PORTAIBLE_RUNTIME portaible::RunTime::getInstance()

namespace portaible
{

    class RunTime : public Singleton<RunTime> 
    {
        private:
            std::vector<Module*> modules;
            bool started = false;


        public:
            ChannelManager channelManager;
            XMLLoader::XMLLoaderManager loader;

   
        void test()
        {
        }

        void start()
        {
            if(this->started)
            {
                PORTAIBLE_THROW(Exception, "Error in RunTime::start(), start was called twice !");
            }
            this->startModules();
            this->started = true;
        }


        public:
            void startModules();

            void addModule(Module* module);

            size_t getNumModules();
            size_t getNumChannels();
            const std::string& getChannelNameByIndex(size_t id);

            bool isStarted() const;
    };
}

// Yes, sadly, we need to define this here.
// In Module.hpp would be a cyclic dependency.
#include "Module/ModuleFactory/ModuleFactory.hpp"
#define DECLARE_MODULE(className)\
    DECLARE_SERIALIZATION(className)\
    DECLARE_MODULE_FACTORY(className)\

#define REGISTER_MODULE(className)\
    static_assert(std::is_base_of<portaible::BaseModule, className>::value, "Tried to register a Module (see above), however it seems you forgot to inherit from Module or BaseModule. "  ); \
    REGISTER_SERIALIZATION(className)\
    REGISTER_MODULE_FACTORY(className)\

#include "Module/Module_impl.hpp"
#include "XMLLoader/XMLLoader_impl.hpp"