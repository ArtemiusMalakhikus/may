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
*/

#ifndef AILERON_SOCKET_H
#define AILERON_SOCKET_H

#if defined WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define GET_LAST_ERROR WSAGetLastError()
#define SOCKET_WOULDBLOCK WSAEWOULDBLOCK
#elif defined UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define GET_LAST_ERROR errno
#define SOCKET_WOULDBLOCK EAGAIN
#endif

#include <algorithm>
#include <memory.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>

namespace may
{

#if defined WINDOWS
	typedef SOCKET SocketID;
	typedef int AddressLength;
	int (*CloseSocket)(may::SocketID) = closesocket;
#elif defined UNIX
	typedef int SocketID;
	typedef socklen_t AddressLength;
	int (*CloseSocket)(may::SocketID) = close;
#endif

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

int GetAddressInfo(const std::string& domianName, const std::string& portName, const addrinfo* hint, std::vector<may::AddressInfo>& addressInfos)
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

class SocketAddress;
typedef uint16_t EqualPort;
typedef uint32_t EqualIP;

class SocketAddress
{
public:
	SocketAddress()
	{
		size = sizeof(sockaddr_in);
		memset(&address, 0, size);
	}

	SocketAddress(const may::AddressFamily& family)
	{
		if (family == may::AddressFamily::IPV4)
		{
			size = sizeof(sockaddr_in);
			memset(&address, 0, size);
		}
		else if (family == may::AddressFamily::IPV6)
		{
			size = sizeof(sockaddr_in6);
			memset(&address, 0, size);
		}
	}

	SocketAddress(const sockaddr_storage& _address, const may::AddressLength& _size)
	{
		size = _size;
		address = _address;
	}

	SocketAddress(const sockaddr& _address, const may::AddressLength& _size)
	{
		size = _size;
		memcpy(&address, &_address, size);
	}

	SocketAddress(const sockaddr_in& ipv4)
	{
		size = sizeof(sockaddr_in);
		memcpy(&address, &ipv4, size);
	}

	SocketAddress(const sockaddr_in6& ipv6)
	{
		size = sizeof(sockaddr_in6);
		memcpy(&address, &ipv6, size);
	}

	/*!
	* \param [in] socketAddressStr Socket address in the string.
	* Format: ipv4 - 000.000.000.000:00000 (dec:dec) or ipv6 - [FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF]:00000 ([hex]:dec)
	*/
	SocketAddress(const std::string& socketAddressStr)
	{
		if (socketAddressStr.find("localhost") != -1)
		{
			std::string portStr("80");
			auto pos = socketAddressStr.find(":");
			if (pos != -1)
			{
				++pos;
				portStr = socketAddressStr.substr(pos, socketAddressStr.size() - pos);
			}

			std::array<uint8_t, 4> addr{
				std::stoi("127"),
				std::stoi("0"),
				std::stoi("0"),
				std::stoi("1")
			};

			uint16_t port = htons(std::stoi(portStr));

			sockaddr_in ipv4{};
			memset(&ipv4, 0, sizeof(sockaddr_in));
			ipv4.sin_family = static_cast<uint16_t>(may::AddressFamily::IPV4);
			ipv4.sin_port = port;
			memcpy(&ipv4.sin_addr, addr.data(), addr.size() * sizeof(uint8_t));

			size = sizeof(sockaddr_in);
			memcpy(&address, &ipv4, size);
		}
		else
		{
			if (socketAddressStr[0] == '[')
			{
				std::array<uint16_t, 8> addr;

				auto pos1 = socketAddressStr.find_first_of(':');
				addr[0] = htons(std::stoi(socketAddressStr.substr(1, pos1 - 1), nullptr, 16));
				++pos1;

				auto pos2 = socketAddressStr.find_first_of(':', pos1);
				addr[1] = htons(std::stoi(socketAddressStr.substr(pos1, pos2 - pos1), nullptr, 16));
				++pos2;

				auto pos3 = socketAddressStr.find_first_of(':', pos2);
				addr[2] = htons(std::stoi(socketAddressStr.substr(pos2, pos3 - pos2), nullptr, 16));
				++pos3;

				auto pos4 = socketAddressStr.find_first_of(':', pos3);
				addr[3] = htons(std::stoi(socketAddressStr.substr(pos3, pos4 - pos3), nullptr, 16));
				++pos4;

				auto pos5 = socketAddressStr.find_first_of(':', pos4);
				addr[4] = htons(std::stoi(socketAddressStr.substr(pos4, pos5 - pos4), nullptr, 16));
				++pos5;

				auto pos6 = socketAddressStr.find_first_of(':', pos5);
				addr[5] = htons(std::stoi(socketAddressStr.substr(pos5, pos6 - pos5), nullptr, 16));
				++pos6;

				auto pos7 = socketAddressStr.find_first_of(':', pos6);
				addr[6] = htons(std::stoi(socketAddressStr.substr(pos6, pos7 - pos6), nullptr, 16));
				++pos7;

				auto pos8 = socketAddressStr.find_first_of(']', pos7);
				addr[7] = htons(std::stoi(socketAddressStr.substr(pos7, pos8 - pos7), nullptr, 16));
				pos8 += 2;

				uint16_t port = htons(std::stoi(socketAddressStr.substr(pos8, socketAddressStr.size() - pos8)));

				sockaddr_in6 ipv6{};
				memset(&ipv6, 0, sizeof(sockaddr_in6));
				ipv6.sin6_family = static_cast<uint16_t>(may::AddressFamily::IPV6);
				ipv6.sin6_port = port;
				memcpy(&ipv6.sin6_addr, addr.data(), addr.size() * sizeof(uint16_t));

				size = sizeof(sockaddr_in6);
				memcpy(&address, &ipv6, size);
			}
			else
			{
				std::array<uint8_t, 4> addr;

				auto pos1 = socketAddressStr.find_first_of('.');
				addr[0] = std::stoi(socketAddressStr.substr(0, pos1));
				++pos1;

				auto pos2 = socketAddressStr.find_first_of('.', pos1);
				addr[1] = std::stoi(socketAddressStr.substr(pos1, pos2 - pos1));
				++pos2;

				auto pos3 = socketAddressStr.find_first_of('.', pos2);
				addr[2] = std::stoi(socketAddressStr.substr(pos2, pos3 - pos2));
				++pos3;

				auto pos4 = socketAddressStr.find_first_of(':', pos3);
				addr[3] = std::stoi(socketAddressStr.substr(pos3, pos4 - pos3));
				++pos4;

				uint16_t port = htons(std::stoi(socketAddressStr.substr(pos4, socketAddressStr.size() - pos4)));

				sockaddr_in ipv4{};
				memset(&ipv4, 0, sizeof(sockaddr_in));
				ipv4.sin_family = static_cast<uint16_t>(may::AddressFamily::IPV4);
				ipv4.sin_port = port;
				memcpy(&ipv4.sin_addr, addr.data(), addr.size() * sizeof(uint8_t));

				size = sizeof(sockaddr_in);
				memcpy(&address, &ipv4, size);
			}
		}
	}

	std::string_view GetIP() const
	{
		if (address.ss_family == static_cast<uint16_t>(may::AddressFamily::IPV4))
			return std::string_view{ (reinterpret_cast<const char*>(&address) + 4), 4 };
		else if (address.ss_family == static_cast<uint16_t>(may::AddressFamily::IPV6))
			return std::string_view{ (reinterpret_cast<const char*>(&address) + 8), 16 };
	}

	uint16_t GetPort()
	{
		return *(reinterpret_cast<uint16_t*>(&address) + 1);
	}

	bool operator==(const SocketAddress& _address)
	{
		if (size == _address.size)
			return memcmp(&address, &_address.address, size) == 0;
		return false;
	}

	bool operator!=(const SocketAddress& _address)
	{
		return !(*this == _address);
	}

	bool operator==(const may::EqualIP* _address)
	{
		const sockaddr_storage* addr = reinterpret_cast<const sockaddr_storage*>(_address);

		if (address.ss_family == static_cast<uint16_t>(may::AddressFamily::IPV4))
			return memcmp((reinterpret_cast<uint8_t*>(&address) + 4), (reinterpret_cast<const uint8_t*>(addr) + 4), 4) == 0;
		else if (address.ss_family == static_cast<uint16_t>(may::AddressFamily::IPV6))
			return memcmp((reinterpret_cast<uint8_t*>(&address) + 8), (reinterpret_cast<const uint8_t*>(addr) + 8), 16) == 0;
		return false;
	}

	bool operator!=(const may::EqualIP* _address)
	{
		return !(*this == _address);
	}

	bool operator==(const may::EqualPort* _address)
	{
		const sockaddr_storage* addr = reinterpret_cast<const sockaddr_storage*>(_address);
		return memcmp((reinterpret_cast<uint8_t*>(&address) + 2), (reinterpret_cast<const uint8_t*>(addr) + 2), 2) == 0;
	}

	bool operator!=(const may::EqualPort* _address)
	{
		return !(*this == _address);
	}

	sockaddr_storage address;
	may::AddressLength size;
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

	void CreateSocket(const may::AddressFamily& family)
	{
		socketID = socket(static_cast<int>(family), SOCK_DGRAM, 0);
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
			result = may::CloseSocket(socketID);
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
	void Bind(const may::SocketAddress& address)
	{
		result = bind(socketID, reinterpret_cast<const sockaddr*>(&address.address), address.size);
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
#elif defined UNIX
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
	void SendTo(char* buffer, const int& size, may::SocketAddress& address)
	{
		result = sendto(socketID, buffer, size, 0, reinterpret_cast<const sockaddr*>(&address.address), address.size);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	/*!
	* \param [out] buffer Pointer to the data.
	* \param [out] size Data size in bytes.
	* \param [out] address Link to the sender's address.
	*/
	void ReceiveFrom(char* buffer, int size, may::SocketAddress& address)
	{
		result = recvfrom(socketID, buffer, size, 0, reinterpret_cast<sockaddr*>(&address.address), &address.size);
		error = 0;

		if (result == -1)
			error = GET_LAST_ERROR;
	}

	may::SocketID socketID;
	std::string errorStr;
	int result;
	int error;
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

	void CreateSocket(const may::AddressFamily& family)
	{
		socketID = socket(static_cast<int>(family), SOCK_STREAM, 0);
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
			result = may::CloseSocket(socketID);
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
	void Bind(const may::SocketAddress& address)
	{
		result = bind(socketID, reinterpret_cast<const sockaddr*>(&address.address), address.size);
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
#elif defined UNIX
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
	may::SocketID Accept(may::SocketAddress& address)
	{
		return accept(socketID, reinterpret_cast<sockaddr*>(&address.address), &address.size);
	}

	/*!
	* \brief Establishes a connection.
	* \param [in] address Link to the connection address.
	*/
	void Connect(const may::SocketAddress& address)
	{
		result = connect(socketID, reinterpret_cast<const sockaddr*>(&address.address), address.size);
		error = 0;

		if (result == -1)
		{
			error = GET_LAST_ERROR;

#if defined WINDOWS
			if (error == SOCKET_WOULDBLOCK && nonBlockingMode)
				result = 0;
#elif defined UNIX
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

	may::SocketID socketID;
	std::string errorStr;
	int result;
	int error;
	bool nonBlockingMode;
};
#endif // TCP_SOCKET

}

#endif // !AILERON_SOCKET_H