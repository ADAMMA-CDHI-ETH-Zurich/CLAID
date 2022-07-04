#pragma once
#define _PORTAIBLE_RUNTIME_H_

#include <iostream>

#include <stdio.h>
#include "Utilities/Singleton.hpp"
#include "Channel/ChannelManager.hpp"

#include "Authority/Authority.hpp"
#include "Loader/LoaderManager.hpp"

#define PORTAIBLE_RUNTIME portaible::RunTime::getInstance()

namespace portaible
{

    class RunTime : public Singleton<RunTime> 
    {
        private:
            std::vector<Authority*> authorities;


        public:
            ChannelManager channelManager;
            Loader::LoaderManager loader;

   
        void test()
        {
            printf("Test\n");
        }

        void start()
        {
            this->startAuthorities();
        }


        public:
            void startAuthorities();

            void addAuthority(Authority* authority);

            size_t getNumAuthorities();
            size_t getNumChannels();
            const std::string& getChannelNameByIndex(size_t id);
    };
}

#include "XML/XMLDeserializer.hpp"

#include "Authority/Authority_impl.hpp"
#include "Loader/Loader_impl.hpp"