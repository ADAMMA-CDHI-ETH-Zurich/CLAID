#pragma once

#include <memory>
#include "TaggedDataBase.hpp"
#include <unistd.h>
#include <iostream>
namespace portaible
{
    template<typename T>
    class TaggedData : public TaggedDataBase
    {
        private:
            std::shared_ptr<T> data;
       
         

        public:

            TaggedData() {}

            TaggedData(const T& data) : TaggedDataBase(Time::now(), 0)
            {
                this->data = std::shared_ptr<T>(new T(data));
            }
    
            TaggedData(const T& data, const Time& timestamp, uint32_t sequenceID = 0) : TaggedDataBase(timestamp, sequenceID)
            {
                // Copy is necessary.
                this->data = std::shared_ptr<T>(new T(data));
            }

            TaggedData(std::shared_ptr<T>& data) : TaggedDataBase(Time::now(), 0)
            {
                this->data = data;
            }

            TaggedData(std::shared_ptr<T>& data, const Time& timestamp, uint32_t sequenceID = 0) : TaggedDataBase(timestamp, sequenceID)
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

            void toBinary()
            {

            }

            TaggedDataBase getHeader()
            {
                return *static_cast<TaggedDataBase*>(this);
            }
    };
}