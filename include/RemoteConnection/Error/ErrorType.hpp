#pragma once

namespace claid
{
    namespace RemoteConnection
    {
        struct ErrorType
        {
            virtual ~ErrorType() {}
            virtual std::string getDescription() {return "No description specified.";}
        };
    }
}