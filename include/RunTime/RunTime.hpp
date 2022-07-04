#pragma once
#define _PORTAIBLE_RUNTIME_H_

#include <iostream>

#include <stdio.h>
#include "Utilities/Singleton.hpp"
#include "Channel/ChannelManager.hpp"

#include "Module/Module.hpp"
#include "Loader/LoaderManager.hpp"

#define PORTAIBLE_RUNTIME portaible::RunTime::getInstance()

namespace portaible
{

    class RunTime : public Singleton<RunTime> 
    {
        private:
            std::vector<Module*> modules;


        public:
            ChannelManager channelManager;
            Loader::LoaderManager loader;

   
        void test()
        {
            printf("Test\n");
        }

        void start()
        {
            this->startModules();
        }


        public:
            void startModules();

            void addModule(Module* module);

            size_t getNumModules();
            size_t getNumChannels();
            const std::string& getChannelNameByIndex(size_t id);
    };
}

#include "XML/XMLDeserializer.hpp"

#include "Module/Module_impl.hpp"
#include "Loader/Loader_impl.hpp"