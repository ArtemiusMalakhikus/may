#ifndef AILERON_SOCKET_H
#define AILERON_SOCKET_H

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

#ifdef WINDOWS
#include <WinSock2.h>
#elif UNIX
#include <>
#endif

#include <iostream>
#include <string>
#include <sstream>

namespace may
{

#ifdef WINDOWS
/*!
* \brief Mandatory call before use.
*/
inline void EnableLibrary()
{
	WSADATA wsaData;
	if (WSAStartup(514, &wsaData) != 0)
	{
		std::cout << "library not enabled, error: " << WSAGetLastError() << std::endl;
	}
}

/*!
* \brief Mandatory call after use.
*/
inline void DisableLibrary()
{
	if (WSACleanup() != 0)
	{
		std::cout << "library in not disabled, error: " << WSAGetLastError() << std::endl;
	}
}
#endif // WINDOWS

enum AddressFamily {
	IPV4 = AF_INET,
	IPV6 = AF_INET6
};

class SocketAddress
{
public:
	SocketAddress()
	{
		size = sizeof(sockaddr);
	}

	/*!
	* \brief Socket address initialization.
	* \param [in] socketAddressStr Socket address in the string. Format: "00.00.00.00:00"
	*/
	void InitSocketAddressIPv4(std::string socketAddressStr)
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

		char addr1 = atoi(addrStr1.c_str());
		char addr2 = atoi(addrStr2.c_str());
		char addr3 = atoi(addrStr3.c_str());
		char addr4 = atoi(addrStr4.c_str());
		unsigned short int port = htons(atoi(portStr.c_str()));

		reinterpret_cast<sockaddr_in*>(&address)->sin_family = AF_INET;
		reinterpret_cast<sockaddr_in*>(&address)->sin_port = port;
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_un_b.s_b1 = addr1;
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_un_b.s_b2 = addr2;
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_un_b.s_b3 = addr3;
		reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_un_b.s_b4 = addr4;
	}

	bool operator==(SocketAddress& a)
	{
		return reinterpret_cast<sockaddr_in*>(&address)->sin_addr.S_un.S_addr == reinterpret_cast<sockaddr_in*>(&a)->sin_addr.S_un.S_addr;
	}

	bool operator!=(SocketAddress& a)
	{
		return !(*this == a);
	}

	sockaddr address;
	int size;
};

#ifdef UDP_SOCKET
class UDPSocket
{
public:
	UDPSocket()
	{
		sock = 0;
		result = 0;
		error = 0;
	}

	~UDPSocket()
	{
		//closesocket(sock);
	}

	void CreateSocket(AddressFamily af)
	{
		sock = socket(af, SOCK_DGRAM, 0);

		if (sock == INVALID_SOCKET)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not created, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	void Close()
	{
		result = closesocket(sock);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not closed, error: " + oss.str();
		}
		else
		{
			error = 0;
			sock = 0;
		}
	}

	void SetSocketOptions(const int& level, const int& name, const char* data, const int& size)
	{
		result = setsockopt(sock, level, name, data, size);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not closed, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* \brief Bind socket to address
	*/
	void Bind(SocketAddress& address)
	{
		result = bind(sock, &address.address, address.size);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not bind, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	void SetNonBlockingMode()
	{
		unsigned long arg = 1;

		result = ioctlsocket(sock, FIONBIO, &arg);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "blocking mode is not set, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* \param [in] buffer Pointer to the data.
	* \param [in] size Data size in bytes.
	* \param [in] address Link to the recipient's address.
	*/
	void SendTo(char* buffer, const int& size, SocketAddress& address)
	{
		result = sendto(sock, buffer, size, 0, &address.address, address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* \param [out] buffer Pointer to the data.
	* \param [out] size Data size in bytes.
	* \param [out] address Link to the sender's address.
	*/
	void ReceiveFrom(char* buffer, int size, SocketAddress& address)
	{
		result = recvfrom(sock, buffer, size, 0, &address.address, &address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //error
	int result;           //last result
	int error;            //last error
};
#endif // UDP_SOCKET

#ifdef TCP_SOCKET
class TCPSocket
{
public:
	TCPSocket()
	{
		sock = 0;
		result = 0;
		error = 0;
	}

	~TCPSocket()
	{
		//closesocket(sock);
	}

	void CreateSocket(AddressFamily af)
	{
		sock = socket(af, SOCK_STREAM, 0);

		if (sock == INVALID_SOCKET)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not created, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	void Close()
	{
		result = closesocket(sock);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not closed, error: " + oss.str();
		}
		else
		{
			error = 0;
			sock = 0;
		}
	}

	/*!
	* \brief Bind socket to address
	*/
	void Bind(SocketAddress& address)
	{
		result = bind(sock, &address.address, address.size);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "socket not bind, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}
	
	void SetNonBlockingMode()
	{
		unsigned long arg = 1;

		result = ioctlsocket(sock, FIONBIO, &arg);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "blocking mode is not set, error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* \brief Places a socket in a state in which it is listening for an incoming connection.
	*/
	void Listen()
	{
		result = listen(sock, SOMAXCONN);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			std::ostringstream oss;
			oss << error << std::endl;
			errorStr = "listen error: " + oss.str();
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* \brief Permits an incoming connection attempt on a socket.
	* \param [in] address Link for the incoming connection address.
	*/
	SOCKET Accept(SocketAddress& address)
	{
		return accept(sock, &address.address, &address.size);
	}

	/*!
	* \brief Establishes a connection.
	* \param [in] address Link to the connection address.
	*/
	void Connect(SocketAddress& address)
	{
		result = connect(sock, &address.address, address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
		else
			error = 0;
	}

	/*!
	* \param [in] buffer Pointer to the data.
	* \param [in] size size Data size in bytes.
	*/
	void Send(char* buffer, const int& size)
	{
		result = send(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* \param [out] buffer Pointer to the data.
	* \param [out] size Data size in bytes.
	*/
	void Receive(char* buffer, const int& size)
	{
		result = recv(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //error
	int result;           //last result
	int error;            //last error
};
#endif // TCP_SOCKET

}

#endif // !AILERON_SOCKET_H