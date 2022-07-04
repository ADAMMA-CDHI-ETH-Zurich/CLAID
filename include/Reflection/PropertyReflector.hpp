#pragma once


#include "Reflection/RecursiveReflector.hpp"

namespace portaible
{
    class PropertyReflector : public RecursiveReflector<PropertyReflector>
    {


    public:

   

        template<typename T>
        void callFloatOrDouble(const char* property, T& member)
        {
            if (this->defaultValueCurrentlySet())
            {
                member = this->getCurrentDefaultValue<T>();
            }
        }

        // Also includes any variants of signed, unsigned, short, long, long long, ...
        template<typename T>
        void callInt(const char* property, T& member)
        {
            if (this->defaultValueCurrentlySet())
            {
                member = this->getCurrentDefaultValue<T>();
            }
        }

        void callBool(const char* property, bool& member)
        {
            if (this->defaultValueCurrentlySet())
            {
                member = this->getCurrentDefaultValue<bool>();
            }        
        }

        template<typename T>
        void callString(const char* property, T& member)
        {
            if (this->defaultValueCurrentlySet())
            {
                member = this->getCurrentDefaultValue<T>();
            }
        }

        template<typename T>
        void callBeginClass(const char* property, T& member)
        {
            
        }

        template<typename T>
        void callEndClass(const char* property, T& member)
        {
        }

        template<typename T>
        void callPointer(const char* property, T*& member)
        {
            if (this->defaultValueCurrentlySet())
            {
                member = this->getCurrentDefaultValue<T>();
            }
        }


        template<typename T>
        void reflect(std::string name, T& obj)
        {
            this->callOnObject(name.c_str(), obj);
        }
      
    };
}