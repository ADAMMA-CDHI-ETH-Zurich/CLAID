#pragma once

#include "Reflection/Reflect.hpp"


namespace claid
{
    // Can be used to easily implement reflection for lists using a custom item identifier.
    // (Note: list type, e.g. vector, must have a public value_type, that allows to retrieve the type 
    // of the elements in the list).
    // Example:
    /*
        class MyClass
        {
            private: 
                std::vector<int> myValues;

            public:
                template<typename Reflector>
                void reflect(Reflector& r)
                {
                    ListReflector("Value", myValues).reflect(r);
                }
        }

        ...
        in XML:

        <root>
            <MyClass>
                <Value>10</Value>
                <Value>42</Value>
            </MyClass>
        </root>

        (otherwise, would need to use "item" instead of "Value", as this is the default implementation for vector.)
    */
    template<typename T>
    class ListReflector
    {   
        private:
            typedef typename T::value_type ValueType;
            T& ref;
            std::string identifier;

        public:

            ListReflector(std::string identifier, T& ref) : identifier(identifier), ref(ref)
            {

            }


            template<typename Reflector>
            void reflect(Reflector& r)
            {
                splitReflectInType(r, *this);
            }

            template<typename Reflector>
            void reflectRead(Reflector& r)
            {
                r.beginSequence();
                int32_t count;
                // Gets count from the current data.
                r.count(this->identifier, count);

                this->ref = T(count);

                for(size_t i = 0; i < count; i++)
                {
                    r.itemIndex(i);
                    r.member(this->identifier.c_str(), this->ref[i], "");
                }
                r.endSequence();
            }

            template<typename Reflector>
            void reflectWrite(Reflector& r)
            {
                r.beginSequence();
                int32_t count = this->ref.size();
                r.count(this->identifier, count);

                for(size_t i = 0; i < this->ref.size(); i++)
                {
                    ValueType& val = this->ref[i];
                    r.itemIndex(i);
                    r.member(this->identifier.c_str(), val, "");
                }
                r.endSequence();
            }   
    };
}

