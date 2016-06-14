#include <WinSock2.h>
#include <cstdio>
#include "Packet.h"
#include "PacketQueue.h"
#include "Crypt.h"
#include "Log.h"
#include "Reactor.h"

//////////////////////////////////////////////////////////////////////////

Reactor::Reactor(ClientDispatcher* dispatcher) : _dispatcher(dispatcher), _key(0)
{
	_q = new PacketQueue(1024, 128, 32);
	_buf = new char[_q->_capacity];
	_head = _buf;
	_tail = _buf;
	_pending = false;
	_sock = INVALID_SOCKET;

	_wsaevent = WSACreateEvent();
	if (_wsaevent == WSA_INVALID_EVENT)
		THROW_EXCEPTION("WSA_INVALID_EVENT");
}

Reactor::~Reactor()
{
	if (_sock != INVALID_SOCKET)
		LOG_LAST_ERROR("_sock != INVALID_SOCKET");

	delete _q;
	delete[] _buf;
	WSACloseEvent(_wsaevent);
}

int Reactor::Run()
{
	volatile bool run = true;

	while (run)
	{
		if (WSAWaitForMultipleEvents(1, &_wsaevent, FALSE, WSA_INFINITE, FALSE) == WSA_WAIT_FAILED)
			THROW_EXCEPTION("WSAWaitForMultipleEvents");

		WSANETWORKEVENTS events;
		if (WSAEnumNetworkEvents(_sock, _wsaevent, &events) == SOCKET_ERROR)
			THROW_EXCEPTION("WSAEnumNetworkEvents");

		if (events.lNetworkEvents & FD_CONNECT)
		{
			if (events.iErrorCode[FD_CONNECT_BIT])
			{
				//LOG_ERROR("[FD_CONNECT_BIT=%d]", events.iErrorCode[FD_CONNECT_BIT]);

				Close();
				run = false;
			}
			else
			{
				OnConnect();
			}
		}

		if (events.lNetworkEvents & FD_CLOSE)
		{
			if (events.iErrorCode[FD_CLOSE_BIT])
			{
				//LOG_ERROR("[FD_CLOSE_BIT=%d]\n", events.iErrorCode[FD_CLOSE_BIT]);

				OnDisconnect();
				run = false;
			}
			else
			{
				OnDisconnect();
				run = false;
			}
		}

		if (events.lNetworkEvents & FD_READ)
		{
			if (events.iErrorCode[FD_READ_BIT])
			{
				//LOG_ERROR("[FD_READ_BIT=%d]\n", events.iErrorCode[FD_READ_BIT]);

				OnDisconnect();
				run = false;
			}
			else
			{
				OnReceive();
			}
		}

		if (events.lNetworkEvents & FD_WRITE)
		{
			if (events.iErrorCode[FD_WRITE_BIT])
			{
				//LOG_ERROR("[FD_WRITE_BIT=%d]\n", events.iErrorCode[FD_WRITE_BIT]);

				OnDisconnect();
				run = false;
			}
			else
			{
				OnSend();
			}
		}
	}

	return 0;
}

void Reactor::Disconnect()
{
	Shutdown(SD_SEND);
}

void Reactor::Connect(ULONG ip, USHORT port)
{
	if (_sock != INVALID_SOCKET)
		THROW_EXCEPTION("_sock != INVALID_SOCKET");

	_head = _buf;
	_tail = _buf;
	_pending = false;

	_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_sock == INVALID_SOCKET)
		THROW_EXCEPTION("INVALID_SOCKET");

	if (WSAEventSelect(_sock, _wsaevent, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE) == SOCKET_ERROR)
		THROW_EXCEPTION("WSAEventSelect");

	SOCKADDR_IN addr = { 0, };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip;

	if (WSAConnect(_sock, (sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			THROW_EXCEPTION("WSAConnect");
	}
}

void Reactor::Connect(std::string ip, USHORT port)
{
	Connect(inet_addr(ip.c_str()), port);
}

void Reactor::OnConnect()
{
//	LOG("[%s][socket=%d]", __FUNCTION__, _sock);
}

void Reactor::OnDisconnect()
{
//	LOG("[%s][socket=%d]", __FUNCTION__, _sock);
	Shutdown(SD_BOTH);
	Close();
}

void Reactor::OnReceive()
{
	char buf[1024];
	WSABUF wsabuf;
	wsabuf.buf = buf;
	wsabuf.len = sizeof(buf);
	DWORD received;
	DWORD flags = 0;

	if (WSARecv(_sock, &wsabuf, 1, &received, &flags, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			Sleep(10);
			OnReceive();
		}

		if (WSAGetLastError() == WSAECONNRESET)
		{
			LOG_LAST_ERROR("WSAECONNRESET");
			return;
		}

		if (WSAGetLastError() == WSAESHUTDOWN)
		{
			LOG_LAST_ERROR("WSAESHUTDOWN");
			return;
		}

		if (WSAGetLastError() == WSAENOTSOCK)
		{
			LOG_LAST_ERROR("WSAESHUTDOWN");
			return;
		}

		if (WSAGetLastError() != WSAEWOULDBLOCK)
			THROW_EXCEPTION("WSARecv");
	}

	_q->Push(buf, received);

	while (_q->HasPacket())
	{
		Packet packet;
		_q->Pop(&packet);

		Crypt crypt;
		char bcc = crypt.Decrypt(_key, packet.buf - EVENTSIZE, packet.buf - EVENTSIZE, packet.header - FOOTERSIZE);
		if (packet.bcc != bcc)
			THROW_EXCEPTION("%d", _key);

		_dispatcher->Invoke(*packet.event, packet.buf, packet.len, *this);
	}
}

void Reactor::OnSend()
{
	WSABUF wsabuf;
	wsabuf.buf = _head;
	wsabuf.len = _tail - _head;
	DWORD sent = 0;

	if (WSASend(_sock, &wsabuf, 1, &sent, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			Sleep(10);
			OnSend();
			return;
		}

		if (WSAGetLastError() == WSAENOTSOCK)
		{
			LOG_LAST_ERROR("WSAENOTSOCK");
			return;
		}

		if (WSAGetLastError() == WSAECONNABORTED)
		{
			LOG_LAST_ERROR("WSAECONNABORTED");
			return;
		}

		if (WSAGetLastError() == WSAECONNRESET)
		{
			LOG_LAST_ERROR("WSAECONNRESET");
			return;
		}

		if (WSAGetLastError() == WSAESHUTDOWN)
		{
			LOG_LAST_ERROR("WSAESHUTDOWN");
			return;
		}

		if (WSAGetLastError() != WSAEWOULDBLOCK)
			THROW_EXCEPTION("WSASend");
	}

	int size = _tail - _head;
	MoveMemory(_buf, _head + sent, size - sent);
	_head = _buf;
	_tail = _buf + size - sent;

	_pending = false;
}

void Reactor::Send(char* buf, int len)
{
	if (_pending)
	{
		if (_tail - _head + len >= _q->_capacity)
		{
			return;
		}

		CopyMemory(_tail, buf, len);
		_tail += len;
		Sleep(0);
		return;
	}

	WSABUF wsabuf;
	wsabuf.buf = buf;
	wsabuf.len = len;
	DWORD sent = 0;

	if (WSASend(_sock, &wsabuf, 1, &sent, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			_pending = true;

			if (_tail - _head + len >= _q->_capacity)
				THROW_EXCEPTION("WSASend");

			CopyMemory(_tail, buf, len);
			_tail += len;
			return;
		}

		if (WSAGetLastError() == WSAECONNRESET)
		{
			LOG_LAST_ERROR("WSAECONNRESET");
			return;
		}

		if (WSAGetLastError() == WSAESHUTDOWN)
		{
			LOG_LAST_ERROR("WSAESHUTDOWN");
			return;
		}

		if (WSAGetLastError() != WSAEWOULDBLOCK)
			THROW_EXCEPTION("WSASend");
	}

	if (sent != static_cast<DWORD>(len))
	{
		CopyMemory(_tail, buf + sent, len - sent);
		_tail += len - sent;
	}
}

void Reactor::SendPacket(char* buf, int len)
{
	char packet[1024];
	short header = static_cast<short>(len + FOOTERSIZE);
	CopyMemory(packet, &header, HEADERSIZE);

	Crypt crypt;
	char bcc = crypt.Encrypt(_key, packet + HEADERSIZE, buf, len);
	packet[HEADERSIZE + len] = bcc;

	Send(packet, HEADERSIZE + len + FOOTERSIZE);
}

void Reactor::Shutdown(int how)
{
	if (_sock == INVALID_SOCKET)
		return;

	if (shutdown(_sock, how) == SOCKET_ERROR)
	{
		//LOG_LAST_ERROR("shutdown");
	}
}

void Reactor::Close()
{
	if (_sock == INVALID_SOCKET)
		return;

	Shutdown(SD_BOTH);

	if (closesocket(_sock) == SOCKET_ERROR)
		THROW_EXCEPTION("closesocket");

	_sock = INVALID_SOCKET;

	_key = 0;
}
