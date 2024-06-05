#pragma once

#include <typeinfo>
#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid {

    class AbstractMutatorHelper
    {
        private:
            virtual void setPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr) = 0;
            virtual void getPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr) = 0;
            virtual bool isSameTypeInternal(const std::type_info& type) = 0;

        public: 

            virtual ~AbstractMutatorHelper()
            {
                
            }

            template<typename T>
            void setPackagePayload(DataPackage& dataPackage, T& data)
            {

            }

            template<typename T>
            void getPackagePayload(DataPackage& dataPackage, T& data)
            {

            }

            template<typename T>
            bool isSameType()
            {
                const std::type_info& x = typeid(T);
                return isSameTypeInternal(x);
            }

            virtual std::string getTypeName() const = 0;
    };
}