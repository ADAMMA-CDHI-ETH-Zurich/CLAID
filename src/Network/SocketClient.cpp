#include "Network/SocketClient.hpp"
namespace portaible
{
bool Network::SocketClient::write(BinaryData& data)
{
    size_t size = data.getNumBytes();
    std::vector<char> tmp(sizeof(size_t));
    
    *reinterpret_cast<size_t*>(tmp.data()) = size;

    writeBytes(tmp);
    writeBytes(data.getData());
}

bool Network::SocketClient::read(BinaryData& data)
{
    size_t* dataNumBytes;
    std::vector<char> byteBuffer;
    // Read data size

    if(!this->readBytes(byteBuffer, sizeof(size_t)))
    {
        return false;
    }

    dataNumBytes = reinterpret_cast<size_t*>(byteBuffer.data());

    data.resize(*dataNumBytes);
    
    return this->readBytes(data.getData(), *dataNumBytes);

}
}

#ifdef __WIN32
    #include "SocketClientWindows_impl.hpp"
#else
    #include "SocketClientUnix_impl.hpp"
#endif