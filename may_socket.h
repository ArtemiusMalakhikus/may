#ifndef AILERON_SOCKET_H
#define AILERON_SOCKET_H

/*
* The MIT License (MIT)
*
* Copyright (c) 2023 Malakhov Artyom
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand
* associated documentation files(the �Software�), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS
* OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* ���������� ������������ ������� ������.
* ��� Windows �� ������ ���������� ���������� Ws2_32.lib 
*/

#ifdef WINDOWS
#include <WinSock2.h>
#elif UNIX
#include <>
#endif

#include <iostream>
#include <string>
#include <sstream>

namespace may_socket
{

#ifdef WINDOWS
/*!
* ������� �������� ���������� �������.
* ������������ ����������� ����� ����� �������������� �������.
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
* ������� ��������� ���������� �������.
* ������������ ����������� ����� ����� ������������� �������.
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
	* ������� ������������� ����� ������.
	* \param [in] socketAddressStr ������ ������ ������, ������� 00.00.00.00:00
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
		closesocket(sock);
	}

	/*!
	* ������� ������ UDP �����.
	* \param [in] af ��������� �������.
	* \return ����, ���� ����� ������, ����� ������
	*/
	void CreateSocket(AddressFamily af)
	{
		sock = socket(af, SOCK_DGRAM, 0);

		if (sock == INVALID_SOCKET)
		{
			error = WSAGetLastError();
			errorStr = "socket not created, error: " + std::to_string(error);
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* ������� ��������� ����� � �������.
	* \param [in] address ������ �� �����.
	* \return ����, ���� ����� ������, ����� ������.
	*/
	void Bind(SocketAddress& address)
	{
		result = bind(sock, &address.address, address.size);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			errorStr = "socket not bind, error: " + std::to_string(error);
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* ������� ������������� ����� � ������������� �����.
	* \return ����, ���� ����� ����������, ����� ������.
	*/
	bool SetNonBlockingMode()
	{
		unsigned long arg = 1;

		result = ioctlsocket(sock, FIONBIO, &arg);

		if (result == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			errorStr = "blocking mode is not set, error: " + std::to_string(error);
		}
		else
		{
			error = 0;
		}
	}

	/*!
	* ������� ���������� ������ �� ������.
	* \param [in] buffer ��������� �� ������.
	* \param [in] size ������ �� ������ ������ � ������.
	* \param [in] address ������ �� ����� ����������.
	*/
	void SendTo(char* buffer, const int& size, SocketAddress& address)
	{
		result = sendto(sock, buffer, size, 0, &address.address, address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* ������� �������� ������ �� ������.
	* \param [out] buffer ��������� �� ������.
	* \param [out] size ������ �� ������ ������ � ������.
	* \param [out] address ������ �� ����� �����������.
	*/
	void ReceiveFrom(char* buffer, int size, SocketAddress& address)
	{
		result = recvfrom(sock, buffer, size, 0, &address.address, &address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //������ � �������
	int result;           //��������� �� ��������� ��������� �������
	int error;            //������ ��������� ��������� �������
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

	/*!
	* ������� ������ TCP �����.
	* \param [in] af ��������� �������.
	* \return ����, ���� ����� ������, ����� ������
	*/
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
	* ������� ��������� ����� � �������.
	* \param [in] address ������ �� �����.
	* \return ����, ���� ����� ������, ����� ������.
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
	
	/*!
	* ������� ������������� ����� � ������������� �����.
	*/
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
	* ������� ��������� ����� � �������������� ���������.
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
	* ������� ��������� ������� ��������� ����������� � ������.
	* \param [out] address ����� ��������� ����.
	* \return �����.
	*/
	SOCKET Accept(SocketAddress& address)
	{
		return accept(sock, &address.address, &address.size);
	}

	/*!
	* ������� ������������� ���������� � ��������.
	* \param [in] address ������ �� ����� �������.
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
	* ������� ���������� ������ �� ������.
	* \param [in] buffer ��������� �� ������.
	* \param [in] size ������ �� ������ ������ � ������.
	*/
	void Send(char* buffer, const int& size)
	{
		result = send(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* ������� �������� ������ �� ������.
	* \param [out] buffer ��������� �� ������.
	* \param [out] size ������ �� ������ ������ � ������.
	*/
	void Receive(char* buffer, const int& size)
	{
		result = recv(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //������ � �������
	int result;           //��������� �� ��������� ��������� �������
	int error;            //������ ��������� ��������� �������
};
#endif // TCP_SOCKET

}

#endif // !AILERON_SOCKET_H