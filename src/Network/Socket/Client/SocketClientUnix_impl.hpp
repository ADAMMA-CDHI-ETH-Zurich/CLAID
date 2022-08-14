
#include "Network/Socket/Client/SocketClient.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>


namespace portaible
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
			fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
			exit(0); 
		} 
		arg |= O_NONBLOCK; 
		if( fcntl(sock, F_SETFL, arg) < 0) { 
			fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
			exit(0); 
		} 
		// Trying to connect with timeout 
		res = connect(sock, (struct sockaddr *)&addr, sizeof(addr)); 
		if (res < 0) { 
			if (errno == EINPROGRESS) { 
				fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
				do { 
				tv.tv_sec = timeOutInSeconds; 
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(sock, &myset); 
				res = select(sock+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
					exit(0); 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						exit(0); 
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
						exit(0); 
					} 
					break; 
				} 
				else { 
					fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
					exit(0); 
				} 
				} while (1); 
			} 
			else { 
				fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
				exit(0); 
			} 
		} 
		// Set to blocking mode again... 
		if( (arg = fcntl(sock, F_GETFL, NULL)) < 0) { 
			fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
			exit(0); 
		} 
		arg &= (~O_NONBLOCK); 
		if( fcntl(sock, F_SETFL, arg) < 0) { 
			fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
			exit(0); 
		} 
		return res;
	}

	bool Network::SocketClient::connectTo(std::string address, int port, size_t timeoutInMs)
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
		// int status = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

		// 	if (status == -1){
		// 	Logger::printfln("error calling fcntl");
		// 	// handle the error.  By the way, I've never seen fcntl fail in this way
		// 	}
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

		// Usually, when we try to connect to a server and the port is not opened,
		// connect fails immediately. However, if the port is opened but connection with server application
		// is not possible for other reasons (e.g. blocked by firewall), then connect blocks and never returns.
		// https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
	
		// try to connect for 3 seconds, otherwise cancel.

		int res = connectWithTimeout(this->sock, serv_addr, timeoutInMs);//connect(this->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		if (res < 0) 
		{
			Logger::printfln("Could not connect to %s:%d %d %s %d", address.c_str(), port, res, strerror(errno), errno);
			return false;
		}
		Logger::printfln("connected %d %s %d",  res, strerror(errno), errno);

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