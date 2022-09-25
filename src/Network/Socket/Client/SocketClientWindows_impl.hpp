#include "Network/Socket/Client/SocketClient.hpp"


#include <Ws2tcpip.h>
#include <winsock2.h>

#include <windows.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace portaible
{
	namespace Network
	{
		SocketClient::SocketClient() : sock(INVALID_SOCKET), connected(false)
		{

		}

		bool SocketClient::connectTo(std::string address, int port, size_t timeoutInMs)
		{
			this->sock = INVALID_SOCKET;
			SOCKADDR_IN addr;
			WSADATA wsa;
			int result = WSAStartup(MAKEWORD(2, 0), &wsa);

			SocketClientError error;

			if (result != 0)
			{
				error.errorType = SocketClientErrorType::ERROR_WSASTARTUP_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;
				return false;
			}

			this->sock = socket(AF_INET, SOCK_STREAM, 0);

			if (this->sock == INVALID_SOCKET)
			{
				error.errorType = SocketClientErrorType::ERROR_SOCKET_CREATION_FAILED;
				error.additionalErrorID = result;
				this->lastError = error;
				return false;
			}

			memset(&addr, 0, sizeof(SOCKADDR_IN));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);

			result = inet_pton(AF_INET, address.c_str(), &(addr.sin_addr));

			if (result == 0)
			{
				error.errorType = SocketClientErrorType::ERROR_INETPTON_INVALID_IP_ADRESS;
				error.additionalErrorID = 0;
				this->lastError = error;
				return false;
			}
			else if (result < 0)
			{
				error.errorType = SocketClientErrorType::ERROR_INETPTON_FAILED;
				error.additionalErrorID = WSAGetLastError();
				this->lastError = error;
				return false;
			}

			result = connect(this->sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));

			if (result != 0)
			{
				error.errorType = SocketClientErrorType::ERROR_CONNECT_FAILED;
				error.additionalErrorID = WSAGetLastError();
				this->lastError = error;
				return false;
			}

			this->connected = true;
			return true;
		}

		bool SocketClient::writeBytes(std::vector<char>& byteBuffer)
		{


			int result = send(this->sock, byteBuffer.data(), byteBuffer.size(), 0);
			if (result == SOCKET_ERROR)
			{
				SocketClientError error;
				error.errorType = SocketClientErrorType::ERROR_WRITE_FAILED;
				error.additionalErrorID = WSAGetLastError();
				this->lastError = error;

				this->close();
				return false;
			}


			return true;
		}

	
		bool SocketClient::readBytes(std::vector<char>& byteBuffer, size_t numBytes)
		{
			if (numBytes == 0)
			{
				SocketClientError error;
				error.additionalErrorID = WSAGetLastError();
				error.errorType = SocketClientErrorType::ERROR_READ_FAILED;

				this->lastError = error;

				return false;
			}
			int bytesReceived = 0;
			
			byteBuffer = std::vector<char>(numBytes);
			bytesReceived = recv(this->sock, byteBuffer.data(), numBytes, 0);
			
			if (bytesReceived < 1)
			{
				SocketClientError error;
				error.additionalErrorID = WSAGetLastError();
				error.errorType = SocketClientErrorType::ERROR_READ_FAILED;

				this->lastError = error;

				return false;
			}



			return true;
		}

		bool SocketClient::isConnected()
		{
			return this->connected;
		}

		void SocketClient::close()
		{
			closesocket(this->sock);
			WSACleanup();
			this->connected = false;
		}

		SocketClient::SocketClientError SocketClient::getLastError()
		{
			return this->lastError;
		}

		bool SocketClient::isValidIP4Adress(const std::string& string)
		{
			struct sockaddr_in sa;
			return inet_pton(AF_INET, string.c_str(), &(sa.sin_addr)) != 0;
		}

		SocketClient::~SocketClient()
		{

		}
	}
}