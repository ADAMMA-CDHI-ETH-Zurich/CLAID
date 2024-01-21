#include "Network/Socket/Client/SocketClient.hpp"
#include "Utilities/Time.hpp"
#include "Logger/Logger.hpp"
#include <Ws2tcpip.h>
#include <winsock2.h>

#include <windows.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace claid
{
	namespace Network
	{
		SocketClient::SocketClient() : sock(INVALID_SOCKET), connected(false)
		{

		}

		SocketClient::SocketClient(int sock) : sock(sock), connected(true)
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
				closesocket(this->sock);
				WSACleanup();
				return false;
			}

			this->connected = true;
			return true;
		}

		bool SocketClient::writeBytes(std::vector<char>& byteBuffer)
		{
			Logger::logInfo("Write bytes 1\n");
			int result = send(this->sock, byteBuffer.data(), byteBuffer.size(), 0);
			Logger::logInfo("Write bytes 2 %d %d %u %d\n", result, byteBuffer.size(), this, this->sock);

			if (result == SOCKET_ERROR)
			{
			Logger::logInfo("Write bytes 3\n");
				SocketClientError error;
				error.errorType = SocketClientErrorType::ERROR_WRITE_FAILED;
				error.additionalErrorID = WSAGetLastError();
				this->lastError = error;

				this->close();
				return false;
			}

			Logger::logInfo("Write bytes 4\n");


			return true;
		}

	
		bool SocketClient::readBytes(std::vector<char>& byteBuffer, size_t numBytes)
		{
			Logger::logInfo("read bytes 1\n");

			if (numBytes == 0)
			{
				Logger::logInfo("read bytes 2\n");

				SocketClientError error;
				error.additionalErrorID = WSAGetLastError();
				error.errorType = SocketClientErrorType::ERROR_READ_FAILED;

				this->lastError = error;

				return false;
			}
			int bytesReceived = 0;
			Logger::logInfo("read bytes 3\n");
		
			byteBuffer = std::vector<char>(numBytes);
			bytesReceived = recv(this->sock, byteBuffer.data(), numBytes, MSG_WAITALL);
			Logger::logInfo("read bytes 4\n");
			
			if (bytesReceived < 1)
			{
							Logger::logInfo("read bytes 5\n");

				SocketClientError error;
				error.additionalErrorID = WSAGetLastError();
				error.errorType = SocketClientErrorType::ERROR_READ_FAILED;

				this->lastError = error;

				return false;
			}


			Logger::logInfo("read bytes 6\n");

			return true;
		}

		bool SocketClient::isConnected()
		{
			return this->connected;
		}

		void SocketClient::close()
		{
			Logger::logInfo("SocketClient::close %d", this->connected);
			// Don't call close when socket was closed already or is not connected.
			// This will lead a funny "fdsan: attempted to close file descriptor 89, expected to be unowned, actually owned by ..."
			if(!this->connected)
			{
				return;
			}
			
			closesocket(this->sock);
			WSACleanup();
			shutdown(this->sock, SD_BOTH);
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