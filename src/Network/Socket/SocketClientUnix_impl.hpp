
#include "Network/Socket/SocketClient.hpp"
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
		struct hostent *server;

		char buffer[256];
		
		this->sock = socket(AF_INET, SOCK_STREAM, 0);
		if (this->sock < 0) 
		{
			Logger::printfln("Failed to open socket.\n");
			return false;
		}
		server = gethostbyname(address.c_str());
		if (server == NULL) 
		{
			Logger::printfln("Error invalid IP address.\n");
			return false;
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);

		serv_addr.sin_port = htons(port);
		if (connect(this->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		{
			Logger::printfln("Could not connect to %s:%d", address.c_str(), port);
			return false;
		}
		this->connected = true;
		return true;
	}

	bool Network::SocketClient::writeBytes(std::vector<char>& byteBuffer)
	{
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
		this->close();
	}


}