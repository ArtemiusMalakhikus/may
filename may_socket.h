/*
* The MIT License (MIT)
*
* Copyright (c) 2023 Malakhov Artyom
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand
* associated documentation files(the “Software”), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS
* OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* This is a single file library for encapsulation Berkeley sockets.
* For "Windows", you need to link the Ws2_32.lib
*/

#ifndef AILERON_SOCKET_H
#define AILERON_SOCKET_H

#if defined WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define GET_LAST_ERROR WSAGetLastError()
#define SOCKET_WOULDBLOCK WSAEWOULDBLOCK
typedef SOCKET SocketID;
typedef int SocketLength;
int (*CloseSocket)(SocketID) = closesocket;
#elif defined LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define GET_LAST_ERROR errno
#define SOCKET_WOULDBLOCK EAGAIN
typedef int SocketID;
typedef socklen_t SocketLength;
int (*CloseSocket)(SocketID) = close;
#endif

#include <algorithm>
#include <memory.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace may
{

/*!
* \brief Mandatory call before use.
*/
inline void EnableLibrary()
{
#if defined WINDOWS
	WSADATA wsaData;
	if (WSAStartup(514, &wsaData) != 0)
		std::cout << "library not enabled, error: " << WSAGetLastError() << std::endl;
#endif // WINDOWS
}

/*!
* \brief Mandatory call after use.
*/
inline void DisableLibrary()
{
#if defined WINDOWS
	if (WSACleanup() != 0)
		std::cout << "library in not disabled, error: " << WSAGetLastError() << std::endl;
#endif // !WINDOWS
}

enum class AddressFamily {
	IPV4 = AF_INET,
	IPV6 = AF_INET6
};

struct AddressInfo
{
	int flags;      //AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
	int family;     //PF_xxx
	int socktype;   //SOCK_xxx
	int protocol;   //0 or IPPROTO_xxx for IPv4 and IPv6
	size_t addrlen; //length of ai_addr
	sockaddr addr;  //binary address
};

int GetAddressInfo(std::string& domianName, std::string& portName, const addrinfo* hint, std::vector<AddressInfo>& addressInfos)
{
	addrinfo* addrinfoResult = nullptr;
	int result = getaddrinfo(domianName.c_str(), portName.c_str(), hint, &addrinfoResult);
	if (result == 0)
	{
		for (addrinfo* addressInfo = addrinfoResult; addressInfo != nullptr; addressInfo = addressInfo->ai_next)
			addressInfos.push_back({ addressInfo->ai_flags, addressInfo->ai_family, addressInfo->ai_socktype,
				addressInfo->ai_protocol, addressInfo->ai_addrlen, *addressInfo->ai_addr });

		freeaddrinfo(addrinfoResult);
	}

	return result;
}

class SocketAddress
{
public:
	SocketAddress()
	{
		size = sizeof(sockaddr);
	}

	SocketAddress(const sockaddr& _address)
	{
		size = sizeof(sockaddr);
		address = _address;
	}

	SocketAddress(const std::string& socketAddressStr)
	{
		size = sizeof(sockaddr);
		InitSocketAddressIPv4(socketAddressStr);
	}

	/*!
	* \brief Socket address initialization.
	* \param [in] socketAddressStr Socket address in the string. Format: "00.00.00.00:00"
	*/
	void InitSocketAddressIPv4(const std::string& socketAddressStr)
	{
		std::string addrStr1;
		std::string addrStr2;
		std::string addrStr3;
		std::string addrStr4;
		std::string portStr;

		unsigned int pos = socketAddressStr.find("localhost");
		if (pos != -1)
		{
			addrStr1 = "127";
			addrStr2 = "0";
			addrStr3 = "0";
			addrStr4 = "1";

			pos = socketAddressStr.find(":");
			if (pos != -1)
			{
				++pos;
				portStr = socketAddressStr.substr(pos, socketAddressStr.size() - pos);
			}
			else
			{
				portStr = "80";
			}
		}
		else
		{
			unsigned int i = 0;

			for (; i < socketAddressStr.size(); ++i)
			{
				if (socketAddressStr[i] != '.')
				{
					addrStr1 += socketAddressStr[i];
				}
				else
				{
					++i;
					break;
				}
			}

			for (; i < socketAddressStr.size(); ++i)
			{
				if (socketAddressStr[i] != '.')
				{
					addrStr2 += socketAddressStr[i];
				}
				else
				{
					++i;
					break;
				}
			}

			for (; i < socketAddressStr.size(); ++i)
			{
				if (socketAddressStr[i] != '.')
				{
					addrStr3 += socketAddressStr[i];
				}
				else
				{
					++i;
					break;
				}
			}

			for (; i < socketAddressStr.size(); ++i)
			{
				if (socketAddressStr[i] != ':')
				{
					addrStr4 += socketAddressStr[i];
				}
				else
				{
					++i;
					break;
				}
			}

			for (; i < socketAddressStr.size(); ++i)
			{
				portStr += socketAddressStr[i];
			}
		}

		unsigned char addr1 = atoi(addrStr1.c_str());
		unsigned char addr2 = atoi(addrStr2.c_str());
		unsigned char addr3 = atoi(addrStr3.c_str());
		unsigned char addr4 = atoi(addrStr4.c_str());
		unsigned short int port = htons(atoi(portStr.c_str()));

		reinterpret_cast<sockaddr_in*>(&address)->sin_family = AF_INET;
		reinterpret_cast<sockaddr_in*>(&address)->sin_port = port;

#if defined WINDOWS
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_addr = addr1 | (addr2 << 8) | (addr3 << 16) | (addr4 << 24);
#elif defined LINUX
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.s_addr = addr1 | (addr2 << 8) | (addr3 << 16) | (addr4 << 24);
#endif
	}

	bool operator==(SocketAddress& a)
	{
#if defined WINDOWS
		return reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_addr == reinterpret_cast<sockaddr_in*>(&a)->sin_addr.S_un.S_addr;
#elif defined LINUX
		return reinterpret_cast<sockaddr_in*>(&address)->sin_addr.s_addr == reinterpret_cast<sockaddr_in*>(&a)->sin_addr.s_addr;
#endif
	}

	bool operator!=(SocketAddress& a)
	{
		return !(*this == a);
	}

	sockaddr address;
	SocketLength size;
};

#ifdef UDP_SOCKET
class UDPSocket
{
public:
	UDPSocket()
	{
		socketID = -1;
		result = 0;
		error = 0;
		nonBlockingMode = false;
	}

	void CreateSocket(AddressFamily af)
	{
		socketID = socket(af, SOCK_DGRAM, 0);
		error = 0;

		if (socketID == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not created, error: " + oss.str();
		}
	}

	void Close()
	{
		if (socketID != -1)
		{
			result = CloseSocket(socketID);
			error = 0;
			socketID = -1;

			if (result == -1)
			{
				error = GET_LAST_ERROR;
				std::ostringstream oss;
				oss << error << std::endl;
				errorStr = "socket not closed, error: " + oss.str();
			}
		}
	}

	void SetSocketOptions(const int& level, const int& name, const char* data, const int& size)
	{
		result = setsockopt(socketID, level, name, data, size);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not closed, error: " + oss.str();
		}
	}

	/*!
	* \brief Bind socket to address
	*/
	void Bind(SocketAddress& address)
	{
		result = bind(socketID, &address.address, address.size);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not bind, error: " + oss.str();
		}
	}

	void SetNonBlockingMode()
	{
#if defined WINDOWS
		unsigned long arg = 1;
		result = ioctlsocket(socketID, FIONBIO, &arg);
#elif defined LINUX
		result = fcntl(socketID, F_GETFL, 0);
		if (result != -1)
			result = fcntl(socketID, F_SETFL, result | O_NONBLOCK);
#endif // WINDONS

		error = 0;
		nonBlockingMode = true;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "blocking mode is not set, error: " + oss.str();
		}
	}

	/*!
	* \param [in] buffer Pointer to the data.
	* \param [in] size Data size in bytes.
	* \param [in] address Link to the recipient's address.
	*/
	void SendTo(char* buffer, const int& size, SocketAddress& address)
	{
		result = sendto(socketID, buffer, size, 0, &address.address, address.size);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	/*!
	* \param [out] buffer Pointer to the data.
	* \param [out] size Data size in bytes.
	* \param [out] address Link to the sender's address.
	*/
	void ReceiveFrom(char* buffer, int size, SocketAddress& address)
	{
		result = recvfrom(socketID, buffer, size, 0, &address.address, &address.size);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	SocketID socketID;
	std::string errorStr; //error
	int result;           //last result
	int error;            //last error
	bool nonBlockingMode;
};
#endif // UDP_SOCKET

#ifdef TCP_SOCKET
class TCPSocket
{
public:
	TCPSocket()
	{
		socketID = -1;
		result = 0;
		error = 0;
		nonBlockingMode = false;
	}

	void CreateSocket(AddressFamily af)
	{
		socketID = socket(static_cast<int>(af), SOCK_STREAM, 0);
		error = 0;

		if (socketID == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not created, error: " + oss.str();
		}
	}

	void Close()
	{
		if (socketID != -1)
		{
			result = CloseSocket(socketID);
			error = 0;
			socketID = -1;

			if (result == -1)
			{
				error = GET_LAST_ERROR;
				std::ostringstream oss;
				oss << error << std::endl;
				errorStr = "socket not closed, error: " + oss.str();
			}
		}
	}

	/*!
	* \brief Bind socket to address
	*/
	void Bind(SocketAddress& address)
	{
		result = bind(socketID, &address.address, address.size);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not bind, error: " + oss.str();
		}
	}
	
	void SetNonBlockingMode()
	{
#if defined WINDOWS
		unsigned long arg = 1;
		result = ioctlsocket(socketID, FIONBIO, &arg);
#elif defined LINUX
		result = fcntl(socketID, F_GETFL, 0);
		if (result != -1)
			result = fcntl(socketID, F_SETFL, result | O_NONBLOCK);
#endif // WINDONS

		error = 0;
		nonBlockingMode = true;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "blocking mode is not set, error: " + oss.str();
		}
	}

	/*!
	* \brief Places a socket in a state in which it is listening for an incoming connection.
	*/
	void Listen()
	{
		result = listen(socketID, 128);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "listen error: " + oss.str();
		}
	}

	/*!
	* \brief Permits an incoming connection attempt on a socket.
	* \param [in] address Link for the incoming connection address.
	*/
	SocketID Accept(SocketAddress& address)
	{
		return accept(socketID, &address.address, &address.size);
	}

	/*!
	* \brief Establishes a connection.
	* \param [in] address Link to the connection address.
	*/
	void Connect(const SocketAddress& address)
	{
		result = connect(socketID, &address.address, address.size);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;

#if defined WINDOWS
			if (error == SOCKET_WOULDBLOCK && nonBlockingMode)
				result = 0;
#elif defined LINUX
			if (error == EINPROGRESS && nonBlockingMode)
				result = 0;
#endif
		}
	}

	/*!
	* \param [in] buffer Pointer to the data.
	* \param [in] size size Data size in bytes.
	*/
	void Send(const char* buffer, const int& size)
	{
		result = send(socketID, buffer, size, 0);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	/*!
	* \param [out] buffer Pointer to the data.
	* \param [out] size Data size in bytes.
	*/
	void Receive(char* buffer, const int& size)
	{
		result = recv(socketID, buffer, size, 0);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	SocketID socketID;
	std::string errorStr; //error
	int result;           //last result
	int error;            //last error
	bool nonBlockingMode;
};
#endif // TCP_SOCKET

}

#endif // !AILERON_SOCKET_H