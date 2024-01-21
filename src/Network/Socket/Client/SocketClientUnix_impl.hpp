
#include "Network/Socket/Client/SocketClient.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Needed for ::close (closing sockets)!
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

namespace claid
{

	Network::SocketClient::SocketClient() : sock(-1), connected(false)
	{

	}

	Network::SocketClient::SocketClient(int sock) : sock(sock), connected(true)
	{

	}

	int connectWithTimeout(int sock, sockaddr_in& addr, size_t timeOutInSeconds)
	{
		int res; 
		long arg; 
		fd_set myset; 
		struct timeval tv; 
		int valopt; 
		socklen_t lon; 

		// Set non-blocking 
		if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) { 
			Logger::logInfo("Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
			return -1;
		} 
		arg |= O_NONBLOCK; 
		if( fcntl(sock, F_SETFL, arg) < 0) { 
			Logger::logInfo("Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
			return -1;
		} 
		// Trying to connect with timeout 
		res = connect(sock, (struct sockaddr *)&addr, sizeof(addr)); 
		if (res < 0) { 
			if (errno == EINPROGRESS) { 
				
				do { 
				tv.tv_sec = timeOutInSeconds; 
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(sock, &myset); 
				res = select(sock+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					Logger::logInfo("Error connecting %d - %s\n", errno, strerror(errno)); 
					return -1;
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						Logger::logInfo("Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						Logger::logInfo("Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
						return -1;
					} 
					break; 
				} 
				else { 
					Logger::logInfo("Timeout in select() - Cancelling!\n"); 
					return -1;
				} 
				} while (1); 
			} 
			else { 
				Logger::logInfo("Error connecting %d - %s\n", errno, strerror(errno)); 
				return -1;
			} 
		} 
		// Set to blocking mode again... 
		if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) { 
			Logger::logInfo("Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
			return -1;
		} 
		arg &= (~O_NONBLOCK); 
		if( fcntl(sock, F_SETFL, arg) < 0) { 
			Logger::logInfo("Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
			return -1;
		} 
		return res;
	}

	bool Network::SocketClient::connectTo(std::string address, int port, size_t timeoutInMs)
	{
		struct sockaddr_in serv_addr;
		struct hostent server;

		bzero(&serv_addr, sizeof(serv_addr));

		char buffer[256];
		Logger::logInfo("Connect to 1");
		
		this->sock = socket(AF_INET, SOCK_STREAM, 0);
		Logger::logInfo("Opened Socket %d", this->sock);
		if (this->sock < 0) 
		{
			Logger::logInfo("Failed to open socket.\n");
			return false;
		}
		// int status = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

		// 	if (status == -1){
		// 	Logger::logInfo("error calling fcntl");
		// 	// handle the error.  By the way, I've never seen fcntl fail in this way
		// 	}
				Logger::logInfo("Connect to 2");

		

		// 		Logger::logInfo("Connect to 3");

		// if (server == NULL) 
		// {
		// 			Logger::logInfo("Connect to 4");

		// 	Logger::logInfo("Error invalid IP address.\n");
		// 	return false;
		// }
		// 		Logger::logInfo("Connect to 5");

		//bzero((char *) &serv_addr, sizeof(serv_addr));
		Logger::logInfo("Connect to 6");

		serv_addr.sin_addr.s_addr = inet_addr(address.c_str()); // sets IP of server
		serv_addr.sin_family = AF_INET; // uses internet address domain
		serv_addr.sin_port = htons(port); // sets PORT on server
		// serv_addr.sin_family = AF_INET;
		// bcopy((char *)server->h_addr, 
		// 	(char *)&serv_addr.sin_addr.s_addr,
		// 	server->h_length);

		Logger::logInfo("Connect to 7");

		
				Logger::logInfo("Connect to 8");

		// Usually, when we try to connect to a server and the port is not opened,
		// connect fails immediately. However, if the port is opened but connection with server application
		// is not possible for other reasons (e.g. blocked by firewall), then connect blocks and never returns.
		// https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
	
		// try to connect for 3 seconds, otherwise cancel.

		int res = connectWithTimeout(this->sock, serv_addr, timeoutInMs);//connect(this->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		if (res < 0) 
		{
			Logger::logInfo("Could not connect to %s:%d %d %s %d", address.c_str(), port, res, strerror(errno), errno);
			::close(this->sock);
			return false;
		}
		Logger::logInfo("connected %d %s %d",  res, strerror(errno), errno);

				Logger::logInfo("Connect to 9");

		this->connected = true;
		return true;
	}

	bool Network::SocketClient::writeBytes(std::vector<char>& byteBuffer)
	{
		Logger::logInfo("Low level socketclient %d: writing  %ul bytes", this->sock, byteBuffer.size());
		int result = send(this->sock, byteBuffer.data(), byteBuffer.size(), MSG_NOSIGNAL);
		if (result < 0)
		{
			SocketClientError error;
			error.errorType = SocketClientErrorType::ERROR_WRITE_FAILED;
			this->lastError = error;

			this->close();
			return false;
		}
		Logger::logInfo("low level socketclient %d: writing result %d", this->sock, result);
		return true;
	}

	bool Network::SocketClient::readBytes(std::vector<char>& byteBuffer, size_t numBytes)
	{
		byteBuffer = std::vector<char>(numBytes);
		int bytesReceived = 0;

		bytesReceived = recv(this->sock, byteBuffer.data(), numBytes, MSG_WAITALL);
		Logger::logInfo("Bytes received %d %d", bytesReceived, numBytes);
		if (bytesReceived < 1)
		{
			Logger::logInfo("SocketClient: read failed on socket %d: %s (%d)\n", this->sock, strerror(errno), errno);
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
		Logger::logInfo("SocketClient::close %d", this->connected);
		// Don't call close when socket was closed already or is not connected.
		// This will lead a funny "fdsan: attempted to close file descriptor 89, expected to be unowned, actually owned by ..."
		if(!this->connected)
			return;
		this->connected = false;

		long arg; 
		fd_set myset; 
		struct timeval tv; 
		int valopt; 
		socklen_t lon; 

		// Set non-blocking 
		if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) { 
			Logger::logInfo("Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		} 
		arg |= O_NONBLOCK; 
		if( fcntl(sock, F_SETFL, arg) < 0) { 
			Logger::logInfo("Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
	
		} 
		shutdown(sock, SHUT_RDWR);
		::close(this->sock);
		Logger::logInfo("SocketClient::close done");

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