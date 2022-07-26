#include "Network/Socket/Server/SocketServer.hpp"
#include <arpa/inet.h>
namespace portaible
{
namespace Network
{
    bool SocketServer::bindTo(size_t port)
    {
        this->port = port;
        if ((this->serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        {
            this->lastError = SocketServerErrorType::SOCKET_CREATION_FAILED;
            return false;
        }

        int opt;
        if (setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                    &opt, sizeof(opt))) 
        { 
            this->lastError = SocketServerErrorType::SET_SOCK_OPT_FAILED;
            return false;
        } 
        
        this->address.sin_family = AF_INET; 
        this->address.sin_addr.s_addr = INADDR_ANY; 
        this->address.sin_port = htons(port); 
        
        // Forcefully attaching socket to the port 
        if (bind(this->serverSocket, (struct sockaddr *)&this->address,  
                                    sizeof(this->address))<0) 
        { 
            this->lastError = SocketServerErrorType::BIND_FAILED;
            return false;
        } 


        if (listen(this->serverSocket, 3) < 0) 
        { 
            this->lastError = SocketServerErrorType::LISTEN_FAILED;
            return false;
        } 
        return true;
    }

    bool SocketServer::accept(SocketClient& client)
    {
        int clientSocket;
        struct sockaddr_in clientAddress; 
        
        clientSocket = ::accept(this->serverSocket, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddress);
        if (clientSocket < 0) 
        { 
            Logger::printfln("SocketServer:ClientHandler failed to accept client. Is Server down??\n");
            Logger::printfln("recv: %s (%d)\n", strerror(errno), errno);
            return false;
        } 
        else
        {
            struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientAddress;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            char str[INET_ADDRSTRLEN];
            ::inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
            Logger::printfln("SocketServer:ClientHandler successfully accepted client with address %s %d\n", str, clientSocket);
            
            client = SocketClient(clientSocket);
            return true;
        }
    }


    // void Socket::SocketServer::Server::close()
    // {

    // }

    SocketServer::SocketServerError SocketServer::getLastError()
    {
        return this->lastError;
    }


    SocketServer::SocketServer()
    {

    }

    SocketServer::~SocketServer()
    {

    }
}
}
