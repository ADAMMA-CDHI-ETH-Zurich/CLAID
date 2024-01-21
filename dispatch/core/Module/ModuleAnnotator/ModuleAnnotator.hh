#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"




using claidservice::DataPackage;
using claidservice::CtrlType;
using claidservice::ModuleAnnotation;


namespace claid
{
    class ModuleAnnotator
    {
        private:
            ModuleAnnotation annotation;

            std::string moduleType;

            template <typename T>
            DataPackage prepareExamplePackage(const std::string& moduleId, const std::string& channelName, bool isPublisher) 
            {
                DataPackage dataPackage;

                if(isPublisher)
                {
                    // Only set module Id, host will be added by Middleware later.
                    dataPackage.set_source_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
                }
                else
                {
                    // Only set module Id, host will be added by Middleware later.
                    dataPackage.set_target_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
                }
                dataPackage.set_channel(channelName);

                Mutator<T> mutator = TypeMapping::getMutator<T>();
                T exampleInstance = T();
                mutator.setPackagePayload(dataPackage, exampleInstance);

                return dataPackage;   
            }


        public:



            ModuleAnnotator(const std::string& moduleType);

            void setModuleDescription(const std::string& moduleDescription);
            void describeProperty(const std::string& propertyName, const std::string& propertyDescription);

            template<typename T>
            void describePublishChannel(const std::string& channelName, const std::string& channelDescription)
            {
                DataPackage examplePackage = prepareExamplePackage<T>(this->moduleType, channelName, true);
                this->annotation.add_channel_definition()->CopyFrom(examplePackage);
                this->annotation.add_channel_description(channelDescription);
            }

            template<typename T>
            void describeSubscribeChannel(const std::string& channelName, const std::string& channelDescription)
            {
                DataPackage examplePackage = prepareExamplePackage<T>(this->moduleType, channelName, false);
                this->annotation.add_channel_definition()->CopyFrom(examplePackage);
                this->annotation.add_channel_description(channelDescription);
            }

            const ModuleAnnotation& getAnnotation() const;

            
    };
}