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

            bool start(ChannelSubscriberPublisher* subscriberPublisher, const std::map<std::string, std::string>& properties) override final;

    };
}
