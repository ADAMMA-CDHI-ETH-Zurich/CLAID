#include "Network/Socket/Server/SocketServer.hpp"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
namespace claid
{
	namespace Network
	{
        bool SocketServer::bindTo(size_t port)
        {
            this->port = port;
            WSADATA wsaData;
            struct addrinfo* addr = NULL;
            struct addrinfo hints;
            int result;

            SocketServerError error;

            // Initialize Winsock
            result = WSAStartup(MAKEWORD(2,2), &wsaData);
            if (result != 0) 
            {
                error.errorType = SocketServerErrorType::ERROR_WSASTARTUP_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;
				return false;
            }

            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            // Resolve the server address and port
            result = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &addr);
            if ( result != 0 ) 
            {
                error.errorType = SocketServerErrorType::GET_ADDRESS_INFO_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;
				return false;
            }

            // Create a SOCKET for the server to listen for client connections.
            this->serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (this->serverSocket == INVALID_SOCKET) 
            {

                error.errorType = SocketServerErrorType::SOCKET_CREATION_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;
                freeaddrinfo(addr);
                WSACleanup();
				return false;
            }

            // Setup the TCP listening socket
            result = bind( this->serverSocket, addr->ai_addr, (int)addr->ai_addrlen);
            if (result == SOCKET_ERROR) 
            {
                error.errorType = SocketServerErrorType::BIND_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;                
                freeaddrinfo(addr);
                closesocket(this->serverSocket);
                WSACleanup();
                return 1;
            }

            freeaddrinfo(addr);

            result = listen(this->serverSocket, SOMAXCONN);
            if (result == SOCKET_ERROR) 
            {
                error.errorType = SocketServerErrorType::LISTEN_FAILED;
                error.additionalErrorID = result;
                this->lastError = error;   
                closesocket(this->serverSocket);
                WSACleanup();
                return 1;
            }

            
            return true;
        }

        bool SocketServer::accept(SocketClient& client)
        {
            int clientSocket;
            struct sockaddr_in clientAddress; 
            // CRUCIAL ON ANDROID! IF NOT ZEROING, WEIRD STUFF CAN HAPPEN !
            // E.G. ACCEPT FAILS BECAUSE OF "invalid_argument".
            bzero((char *) &clientAddress, sizeof(clientAddress));
            
            socklen_t clientAddressLength = sizeof(clientAddress);
            clientSocket = ::accept(this->serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
            if (clientSocket < 0) 
            { 
                Logger::printfln("SocketServer:ClientHandler failed to accept client. Is Server down??\n");
                Logger::printfln("recv: %s (%d)\n", strerror(errno), errno);
                this->lastError.errorString = SocketServerErrorType::ACCEPT_FAILED;
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