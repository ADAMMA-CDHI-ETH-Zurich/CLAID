#pragma once

namespace portaible
{
    namespace RemoteConnection
    {
        struct ErrorType
        {
            virtual ~ErrorType() {}
            virtual std::string getDescription() {return "No description specified."};
        };
    }
}