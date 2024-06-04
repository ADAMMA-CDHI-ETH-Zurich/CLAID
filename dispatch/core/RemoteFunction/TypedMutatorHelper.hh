#pragma once

#include "AbstractMutatorHelper.hh"
#include "TypeMapping.hh"

namespace claid {

    template<typename T>
    class TypedMutatorHelper : public AbstractMutatorHelper
    {
        void setPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr)
        {
            Mutator<T> mutator = TypeMapping::getMutator<T>();
            T* data = reinterpret_cast<T*>(dataPtr);

            mutator.setPackagePayload(dataPackage, *data);
        }

        void getPackagePayloadUntyped(DataPackage& dataPackage, void* dataPtr)
        {
            Mutator<T> mutator = TypeMapping::getMutator<T>();
            T* data = reinterpret_cast<T*>(dataPtr);

            mutator.getPackagePayload(dataPackage, *data);
        }

        bool isSameTypeInternal(const std::type_info& type)
        {
            return type == typeid(T);
        }

        std::string TypedMutatorHelper::getTypeName() const
        {
            return typeid(T).name();
        }
    };

}