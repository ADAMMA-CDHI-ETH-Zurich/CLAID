#include "PropertyNode.hpp"

namespace claid
{
    template<typename T>
    class TypedPropertyNode : public PropertyNode
    {
        T* data;

        virtual void set(void* data)
        {
            T* t = static_cast<T*>(data);
            this->setTyped(*t);
        }

        virtual void* get()
        {
            T& t = this->getTyped();
            return static_cast<void>*(&t);
        }

        virtual void setTyped(T& data)
        {
            *this->data = data;
        }

        virtual T& getTyped()
        {
            return *this->data;
        }
    };
}