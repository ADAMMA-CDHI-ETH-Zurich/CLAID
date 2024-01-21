#include "Network/Socket/Client/SocketClient.hpp"


#ifdef _WIN32
    #include "SocketClientWindows_impl.hpp"
#else
    #include "SocketClientUnix_impl.hpp"
#endif

namespace claid
{
    bool Network::SocketClient::write(BinaryData& data)
    {
        // First, we write the size (number of bytes) of the data.
        int32_t size = data.getNumBytes();

        std::vector<char> tmp(sizeof(int32_t));

        *reinterpret_cast<int32_t*>(tmp.data()) = size;

        Logger::logInfo("Writing header %u", size);
        if (!writeBytes(tmp))
            return false;

        Logger::logInfo("Writing data %u", size);
        // Now write the data itself.
        if (!writeBytes(data.getData()))
            return false;

        return true;
    }

    bool Network::SocketClient::read(BinaryData& data)
    {
        // We read the number of bytes first
        int32_t* dataNumBytes;
        std::vector<char> byteBuffer;
        // Read data size

        if (!this->readBytes(byteBuffer, sizeof(int32_t)))
        {
            return false;
        }

        dataNumBytes = reinterpret_cast<int32_t*>(byteBuffer.data());
        Logger::logInfo("Read header, continuing to read %d bytes", *dataNumBytes);

        // Now read the data
        data.resize(*dataNumBytes);

        if (!this->readBytes(data.getData(), *dataNumBytes))
        {
            return false;
        }

        return true;
        }
    }