#include "Network/Socket/Client/SocketClient.hpp"
namespace portaible
{
bool Network::SocketClient::write(BinaryData& data)
{
    Logger::printfln("Calling write");
    // First, we write the size (number of bytes) of the data.
    size_t size = data.getNumBytes();
    std::vector<char> tmp(sizeof(size_t));
    
    *reinterpret_cast<size_t*>(tmp.data()) = size;

    if(!writeBytes(tmp))
        return false;

    // Now write the data itself.
    if(!writeBytes(data.getData()))
        return false;

    return true;
}

bool Network::SocketClient::read(BinaryData& data)
{
    // We read the number of bytes first
    size_t* dataNumBytes;
    std::vector<char> byteBuffer;
    // Read data size

    if(!this->readBytes(byteBuffer, sizeof(size_t)))
    {
        return false;
    }

    dataNumBytes = reinterpret_cast<size_t*>(byteBuffer.data());

    // Now read the data
    data.resize(*dataNumBytes);
    
    if(!this->readBytes(data.getData(), *dataNumBytes))
    {
        return false;
    }

    return true;
}
}

#ifdef __WIN32
    #include "SocketClientWindows_impl.hpp"
#else
    #include "SocketClientUnix_impl.hpp"
#endif