#pragma once

#include "dispatch/core/Module/Module.hh"

namespace claid
{
    class ManagerModule : public Module
    {

        protected:

            void pauseModuleById(const std::string& moduleId);
            void resumeModuleById(const std::string& moduleId);
            void adjustPowerProfileOnModuleById(const std::string& moduleId, PowerProfile powerProfile);

            bool start(ChannelSubscriberPublisher* subscriberPublisher, Properties properties) override final;

    };
}
