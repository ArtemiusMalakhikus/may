#ifndef AILERON_SOCKET_H
#define AILERON_SOCKET_H

/*
* Библиотека инкапсуляции сокетов Беркли.
* Для Windows не забудь подключить библиотеку Ws2_32.lib 
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
* Функция включает библиотеку сокетов.
* Обязательный однократный вызов перед использованием сокетов.
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
* Функция выключает библиотеку сокетов.
* Обязательный однократный вызов после использования сокетов.
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
	* Функция устанавливает адрес сокета.
	* \param [in] socketAddressStr Строка адреса сокета, формата 00.00.00.00:00
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

	/*!
	* Функция создаёт UDP сокет.
	* \param [in] af Семейство адресов.
	* \return ложь, если сокет создан, иначе истину
	*/
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

	/*!
	* Функция закрывает сокет.
	*/
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
	* Функция устанавливает дополнительные параметры сокета
	*/
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
	* Функция связывает сокет с адресом.
	* \param [in] address Ссылка на адрес.
	* \return ложь, если сокет связан, иначе истину.
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
	* Функция устанавливает сокет в неблокирующий режим.
	* \return ложь, если режим установлен, иначе истину.
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
	* Функция отправляет данные по адресу.
	* \param [in] buffer Указатель на данные.
	* \param [in] size Ссылка на размер данных в байтах.
	* \param [in] address Ссылка на адрес получателя.
	*/
	void SendTo(char* buffer, const int& size, SocketAddress& address)
	{
		result = sendto(sock, buffer, size, 0, &address.address, address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* Функция получает данные от адреса.
	* \param [out] buffer Указатель на данные.
	* \param [out] size Ссылка на размер данных в байтах.
	* \param [out] address Ссылка на адрес отправителя.
	*/
	void ReceiveFrom(char* buffer, int size, SocketAddress& address)
	{
		result = recvfrom(sock, buffer, size, 0, &address.address, &address.size);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //строка с ошибкой
	int result;           //результат по последней вызванной функции
	int error;            //ошибка последней вызванной функции
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
	* Функция создаёт TCP сокет.
	* \param [in] af Семейство адресов.
	* \return ложь, если сокет создан, иначе истину
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

	/*!
	* Функция закрывает сокет.
	*/
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
	* Функция связывает сокет с адресом.
	* \param [in] address Ссылка на адрес.
	* \return ложь, если сокет связан, иначе истину.
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
	* Функция устанавливает сокет в неблокирующий режим.
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
	* Функция переводит сокет в прослушивающее состояние.
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
	* Функция разрешает попытку входящего подключения к сокету.
	* \param [out] address Адрес удалённого узла.
	* \return сокет.
	*/
	SOCKET Accept(SocketAddress& address)
	{
		return accept(sock, &address.address, &address.size);
	}

	/*!
	* Функция устанавливает соединение с сервером.
	* \param [in] address Ссылка на адрес сервера.
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
	* Функция отправляет данные по адресу.
	* \param [in] buffer Указатель на данные.
	* \param [in] size Ссылка на размер данных в байтах.
	*/
	void Send(char* buffer, const int& size)
	{
		result = send(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	/*!
	* Функция получает данные от адреса.
	* \param [out] buffer Указатель на данные.
	* \param [out] size Ссылка на размер данных в байтах.
	*/
	void Receive(char* buffer, const int& size)
	{
		result = recv(sock, buffer, size, 0);

		if (result == SOCKET_ERROR)
			error = WSAGetLastError();
	}

	SOCKET sock;
	std::string errorStr; //строка с ошибкой
	int result;           //результат по последней вызванной функции
	int error;            //ошибка последней вызванной функции
};
#endif // TCP_SOCKET

}

#endif // !AILERON_SOCKET_H