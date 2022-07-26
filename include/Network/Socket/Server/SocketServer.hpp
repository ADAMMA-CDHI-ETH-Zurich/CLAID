#pragma once


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <functional>

#include "Network/Socket/Client/SocketClient.hpp"

//#include <thread.h>
namespace portaible
{
	namespace Network
	{
		class SocketServer
		{
			public:
				enum SocketServerErrorType
				{
					SOCKET_CREATION_FAILED, // socket(...) failed
					SET_SOCK_OPT_FAILED, // setsockopt(...) failed
					BIND_FAILED, // bind(...) failed
					LISTEN_FAILED, // listen(...) failed
				};

				struct SocketServerError
				{
					SocketServerErrorType errorType;
					int additionalErrorID; // Error ID of the function that throws the error, e.g. WSAStartup fails with WSA_INVALID_PARAMETER
				
					SocketServerError()
					{

					}

					SocketServerError(SocketServerErrorType errorType, int additionalErrorID = -1) : errorType(errorType), additionalErrorID(additionalErrorID)
					{

					}
				};

				std::vector<SocketClient*> acceptedClients;

			private:
				int serverSocket;
				
				bool connected = false;
				int port;

				struct sockaddr_in address; 

				SocketServerError lastError;



			public:
				
			//	void close();
				bool bindTo(size_t port);
				bool accept(SocketClient& client);

				SocketServerError getLastError();


				SocketServer();
				~SocketServer();
		};


	}
}