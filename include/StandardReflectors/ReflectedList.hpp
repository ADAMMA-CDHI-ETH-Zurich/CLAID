#pragma once

#include "Reflection/Reflect.hpp"
#include <initializer_list>

#include "CLAID.hpp"

namespace claid
{
    // Can be used to easily implement lists using a custom item identifier.
    // (Note: list type, e.g. vector, must have a public value_type, that allows to retrieve the type 
    // of the elements in the list).
    // In contrast to ListReflector, this class holds the data itself and reflects it.
    // Hence, it can be used to define custom vectors like typedef ReflectedList<std::vector<int>> IntVector.  
    template<typename T>
    class ReflectedList
    {   
        private:
            typedef typename T::value_type ValueType;
            T val;
            std::string identifier;

        public:

            ReflectedList() : identifier("item")
            {

            }

            ReflectedList(std::string identifier, T& val) : identifier(identifier), val(val)
            {
            }

            ReflectedList(T& val) : identifier("item"), val(val)
            {

            }

            Reflect(ReflectedList<T>,
                reflectFunction(getList);
                splitReflectInType(reflector, *this);
            )


            #ifdef __PYTHON_WRAPPERS__
            CustomReflectForReflector(ReflectedList<T>, claid::PythonWrapper::PybindWrapperGenerator<ReflectedList<T>>,
                reflectFunction(getList);
            )
            #endif

            
       

            template<typename Reflector>
            void reflectRead(Reflector& r)
            {
                r.beginSequence();
                int32_t count;
                // Gets count from the current data.
                r.count(this->identifier, count);

                this->val = T(count);

                for(size_t i = 0; i < count; i++)
                {
                    r.itemIndex(i);
                    r.member(this->identifier.c_str(), this->val[i], "");
                }
                r.endSequence();
            }

            template<typename Reflector>
            void reflectWrite(Reflector& r)
            {
                r.beginSequence();
                int32_t count = this->val.size();
                r.count(this->identifier, count);

                for(size_t i = 0; i < this->val.size(); i++)
                {
                    ValueType& value = this->val[i];
                    r.itemIndex(i);
                    r.member(this->identifier.c_str(), value, "");
                }
                r.endSequence();
            }   

            const T& operator[](size_t index) const 
            {
                return this->val[index];
            }

            T& operator[](size_t index) 
            {
                return this->val[index];
            }
            
            size_t size() const
            {
                return this->val.size();
            }

            void push_back(ValueType& value)
            {
                this->val.push_back(value);
            }

            void push_back(const ValueType& value)
            {
                this->val.push_back(value);
            }


            void clear()
            {
                this->val.clear();
            }

            T getList()
            {
                return this->val;
            }
    };
}

