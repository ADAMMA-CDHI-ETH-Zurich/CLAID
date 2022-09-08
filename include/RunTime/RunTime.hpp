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


#include "Module/Module_impl.hpp"
#include "XMLLoader/XMLLoader_impl.hpp"