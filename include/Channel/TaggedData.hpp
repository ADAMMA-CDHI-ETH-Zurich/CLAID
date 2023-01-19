#pragma once

#include <memory>
#include "TaggedDataBase.hpp"
#include "Reflection/ReflectBase.hpp"
#include <stdint.h>
#include <iostream>
#include <algorithm>
namespace claid
{
    template<typename T>
    class TaggedData : public TaggedDataBase
    {
        private:
            std::shared_ptr<T> data;
       
         

        public:

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                REFLECT_BASE(r, TaggedDataBase);
                r.member("Data", this->data, "");
            }

            TaggedData() {}

            TaggedData(const T& data) : TaggedDataBase(Time::now(), 0)
            {
                this->data = std::shared_ptr<T>(new T(data));
            }
    
            TaggedData(const T& data, const Time& timestamp, uint64_t sequenceID = 0) : TaggedDataBase(timestamp, sequenceID)
            {
                // Copy is necessary.
                this->data = std::shared_ptr<T>(new T(data));
            }

            TaggedData(std::shared_ptr<T>& data) : TaggedDataBase(Time::now(), 0)
            {
                this->data = data;
            }

            TaggedData(std::shared_ptr<T>& data, const Time& timestamp, uint64_t sequenceID = 0) : TaggedDataBase(timestamp, sequenceID)
            {
                this->data = data;
            }

            T& value()
            {
                return *this->data.get();
            }

            const T& value() const
            {
                return *this->data.get();
            }

            // void toBinary()
            // {

            // }

            // uint64_t getUniqueIdentifier() const
            // {
            //     return reinterpret_cast<uint64_t>(data.get());
            // }

            TaggedDataBase getHeader()
            {
                return *static_cast<TaggedDataBase*>(this);
            }
    };
}