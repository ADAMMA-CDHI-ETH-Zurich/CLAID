#pragma once

#ifdef _WIN32

#include <Ws2tcpip.h>
#include <winsock2.h>

#include <windows.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#else
#endif

#pragma once

#include "Logger/Logger.hpp"
#include "Binary/BinaryData.hpp"

#include <stdlib.h>

#include <stdio.h>

#include <string.h>
#include <iostream>
#include <vector>

//#include <thread.h>

namespace portaible
{
	namespace Network
	{
		class SocketClient
		{
			public:
				enum SocketClientErrorType
				{
					ERROR_WSASTARTUP_FAILED, // If WSAStartup(...) fails
					ERROR_CONNECT_FAILED, // If connect(...) fails
					ERROR_INETPTON_INVALID_IP_ADRESS, // If given IP is not a valid IPv4 dotted-decimal string or a valid IPv6 address string
					ERROR_INETPTON_FAILED, // If inet_pton(...) fails with specific error..
					ERROR_SOCKET_CREATION_FAILED, // If socket() function fails
					ERROR_WRITE_FAILED,
					ERROR_READ_FAILED,
				};

				struct SocketClientError
				{
					SocketClientErrorType errorType;
					int additionalErrorID; // Error ID of the function that throws the error, e.g. WSAStartup fails with WSA_INVALID_PARAMETER
				};

			private:
				
				bool connected = false;


				SocketClientError lastError;

				


			public:
				int sock;

				void close();

				bool connectTo(std::string address, int port, size_t timeoutInMs = 3);

				// See SocketClient.cpp for implementation of these two functions,
				// not the __imple.hpp files.
				bool write(BinaryData& data);
				bool read(BinaryData& data);

				bool writeBytes(std::vector<char>& byteBuffer);
				bool readBytes(std::vector<char>& byteBuffer, size_t numBytes);

		
		
				bool isConnected();

				SocketClientError getLastError();

				static bool isValidIP4Adress(const std::string& string);

				SocketClient();
				SocketClient(int sock);
				~SocketClient();
		};
	}
}
