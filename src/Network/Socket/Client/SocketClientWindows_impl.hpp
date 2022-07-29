#include "Network/SocketClient.hpp"

Socket::SocketClient::SocketClient() : sock(INVALID_SOCKET), connected(false)
{

}

bool Socket::SocketClient::connectTo(std::string address, int port)
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
		error.additionalErrorID = 0;
		this->lastError = error;
		return false;
	}

	memset(&addr, 0, sizeof(SOCKADDR_IN)); // zuerst alles auf 0 setzten 
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

bool Socket::SocketClient::write(const char* data, int numBytes)
{
	

	int result = send(this->sock, data, numBytes, 0);
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

bool Socket::SocketClient::write(const std::vector<char>& bytes)
{
	return this->write(bytes.data(), bytes.size());
}

bool Socket::SocketClient::read(std::vector<char>* byteBuffer)
{
	byteBuffer->clear();
	char buffer[1] = "";
	int bytesReceived = 0;
	int s = strlen(buffer);

		
	while ((bytesReceived = recv(this->sock, buffer, 1, 0)) >= 0)
	{
		byteBuffer->push_back(buffer[0]);
	}

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

bool Socket::SocketClient::isConnected()
{
	return this->connected;
}

void Socket::SocketClient::close()
{
	closesocket(this->sock);
	WSACleanup();
	this->connected = false;
}

Socket::SocketClient::SocketClientError Socket::SocketClient::getLastError()
{
	return this->lastError;
}

bool Socket::SocketClient::isValidIP4Adress(const std::string& string)
{
	struct sockaddr_in sa;
	return inet_pton(AF_INET, string.c_str(), &(sa.sin_addr)) != 0;
}

Socket::SocketClient::~SocketClient()
{

}