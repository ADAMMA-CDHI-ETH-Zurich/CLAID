#pragma once

#include <typeinfo>
#include "dispatch/proto/claidservice.pb.h"

using claidservice::DataPackage;

namespace claid {

    class AbstractMutatorHelper
    {
        private:
            virtual void setPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr) = 0;
            virtual void getPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr) = 0;

        public: 

            virtual ~AbstractMutatorHelper()
            {
                
            }

            template<typename T>
            void setPackagePayload(DataPackage& dataPackage, T& data)
            {
                this->setPackagePayloadUntyped(dataPackage, reinterpret_cast<void*>(&data));
            }

            template<typename T>
            void getPackagePayload(DataPackage& dataPackage, T& data)
            {
                this->getPackagePayloadUntyped(dataPackage, reinterpret_cast<void*>(&data));
            }

            template<typename T>
            bool isSameType()
            {
                Mutator<T> tmpMutator = TypeMapping::getMutator<T>();
                return tmpMutator.getMessageTypeName() == this->getTypeName();
            }

            virtual std::string getTypeName() const = 0;
    };
}