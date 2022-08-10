
#include "Network/Socket/Client/SocketClient.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

namespace portaible
{

	Network::SocketClient::SocketClient() : sock(-1), connected(false)
	{

	}

	Network::SocketClient::SocketClient(int sock) : sock(sock), connected(true)
	{

	}

	bool Network::SocketClient::connectTo(std::string address, int port)
	{
		struct sockaddr_in serv_addr;
		struct hostent server;

		bzero(&serv_addr, sizeof(serv_addr));

		char buffer[256];
		Logger::printfln("Connect to 1");
		
		this->sock = socket(AF_INET, SOCK_STREAM, 0);
		if (this->sock < 0) 
		{
			Logger::printfln("Failed to open socket.\n");
			return false;
		}
				Logger::printfln("Connect to 2");

		

		// 		Logger::printfln("Connect to 3");

		// if (server == NULL) 
		// {
		// 			Logger::printfln("Connect to 4");

		// 	Logger::printfln("Error invalid IP address.\n");
		// 	return false;
		// }
		// 		Logger::printfln("Connect to 5");

		//bzero((char *) &serv_addr, sizeof(serv_addr));
		Logger::printfln("Connect to 6");

		serv_addr.sin_addr.s_addr = inet_addr(address.c_str()); // sets IP of server
		serv_addr.sin_family = AF_INET; // uses internet address domain
		serv_addr.sin_port = htons(port); // sets PORT on server
		// serv_addr.sin_family = AF_INET;
		// bcopy((char *)server->h_addr, 
		// 	(char *)&serv_addr.sin_addr.s_addr,
		// 	server->h_length);

		Logger::printfln("Connect to 7");

		
				Logger::printfln("Connect to 8");

		if (connect(this->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		{
			Logger::printfln("Could not connect to %s:%d", address.c_str(), port);
			return false;
		}
				Logger::printfln("Connect to 9");

		this->connected = true;
		return true;
	}

	bool Network::SocketClient::writeBytes(std::vector<char>& byteBuffer)
	{
		Logger::printfln("Low level socketclient: writing %ul bytes", byteBuffer.size());
		int result = send(this->sock, byteBuffer.data(), byteBuffer.size(), 0);
		if (result < 0)
		{
			SocketClientError error;
			error.errorType = SocketClientErrorType::ERROR_WRITE_FAILED;
			this->lastError = error;

			this->close();
			return false;
		}

		return true;
	}

	bool Network::SocketClient::readBytes(std::vector<char>& byteBuffer, size_t numBytes)
	{
		byteBuffer = std::vector<char>(numBytes);
		int bytesReceived = 0;

		bytesReceived = recv(this->sock, byteBuffer.data(), numBytes, MSG_WAITALL);
		Logger::printfln("Bytes received %d %d", bytesReceived, numBytes);
		if (bytesReceived < 1)
		{
			Logger::printfln("SocketClient: read failed on socket %d: %s (%d)\n", this->sock, strerror(errno), errno);
			SocketClientError error;
			error.errorType = SocketClientErrorType::ERROR_READ_FAILED;
				
			this->lastError = error;

			return false;
		}

		return true;
	}




	bool Network::SocketClient::isConnected()
	{
		return this->connected;
	}

	void Network::SocketClient::close()
	{
		::close(this->sock);
		this->connected = false;
	}

	Network::SocketClient::SocketClientError Network::SocketClient::getLastError()
	{
		return this->lastError;
	}

	bool Network::SocketClient::isValidIP4Adress(const std::string& string)
	{
		struct sockaddr_in sa;
		return inet_pton(AF_INET, string.c_str(), &(sa.sin_addr)) != 0;
	}

	Network::SocketClient::~SocketClient()
	{

	}


}