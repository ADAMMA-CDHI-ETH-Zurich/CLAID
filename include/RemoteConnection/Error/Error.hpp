#pragma once

#include <memory>
#include "RemoteConnection/Error/ErrorType.hpp"

namespace portaible
{
    namespace RemoteConnection
    {
        class Error
        {
            private:
                std::shared_ptr<ErrorType> error;

            public:     

                template<typename T>
                void set()
                {
                    this->error = std::shared_ptr<T>(new T);
                }

                void set(std::shared_ptr<ErrorType> error)
                {
                    this->error = error;
                }

                template<typename T>
                bool is() const
                {
                    ErrorType* ptr = error.get();
                    
                    T* castError = dynamic_cast<T*>(ptr);

                    return castError != nullptr;
                }


        };
    }
}