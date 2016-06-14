#include <WinSock2.h>
#include <MSWSock.h>
#include "Log.h"
#include "Socket.h"

//////////////////////////////////////////////////////////////////////////

Socket::Socket(SOCKET sock) : _sock(sock)
{
//	ZeroMemory(&_local, sizeof(_local));
//	ZeroMemory(&_remote, sizeof(_remote));
}

Socket::~Socket()
{

}

void Socket::Open()
{
	Lock lock(this);

	_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_sock == INVALID_SOCKET)
		THROW_EXCEPTION("소켓 만들기에 실패하였다");

	//int zero = 0;
	//if (setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero)) == SOL_SOCKET)
	//	OUTPUTLASTERROR("SO_SNDBUF");
	//if (setsockopt(_sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero)) == SOL_SOCKET)
	//	OUTPUTLASTERROR("SO_RCVBUF");

	//LINGER linger = { 0, };
	//linger.l_onoff = 0;
	//linger.l_linger = 0;
	//if (setsockopt(_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)) == SOL_SOCKET)
	//	OUTPUTLASTERROR("SO_LINGER");

	//BOOL keepalive = TRUE;
	//if (setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE,(char*)&keepalive, sizeof(keepalive)) == SOL_SOCKET)
	//	OUTPUTLASTERROR("SO_KEEPALIVE");
}

void Socket::Bind(DWORD ip, WORD port)
{
	Lock lock(this);

	SOCKADDR_IN local = { 0, };
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(ip);
	local.sin_port = htons(port);

	if (bind(_sock, (SOCKADDR*)&local, sizeof(local)) == SOCKET_ERROR)
		LOG_LAST_ERROR("바인딩에 실패했다");
}

void Socket::Listen()
{
	Lock lock(this);

	if (listen(_sock, SOMAXCONN) == SOCKET_ERROR)
		THROW_EXCEPTION("SOCKET_ERROR");
}

SOCKET Socket::Accept(SOCKADDR_IN* addr)
{
	Lock lock(this);

	int addrlen = sizeof(SOCKADDR_IN);
	return accept(_sock, (SOCKADDR*)addr, &addrlen);
}

void Socket::Close()
{
	Lock lock(this);

	if (_sock == INVALID_SOCKET)
		return;

	//HasOverlappedIoCompleted 
	//if (!CancelIo((HANDLE)_sock))
	//	THROWEXCEPTION(GetLastError());

	//if (shutdown(_sock, SD_BOTH) == SOCKET_ERROR)
		//THROWEXCEPTION(WSAGetLastError()); // 10057=WSAENOTCONN

	//if (!CancelIo((HANDLE)_sock))
	//	OUTPUTLASTERROR("CancelIo");

	Shutdown(SD_BOTH);

	if (closesocket(_sock) == SOCKET_ERROR)
		LOG_LAST_ERROR("closesocket");

	_sock = INVALID_SOCKET;
}

bool Socket::Accept(SOCKET listener, char* buf, int len, OVERLAPPED* overlapped)
{
	Lock lock(this);

	LPFN_ACCEPTEX AcceptEx;
	GUID guid = WSAID_ACCEPTEX;
	DWORD bytes;
	if (WSAIoctl(listener, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
		&AcceptEx, sizeof(AcceptEx), &bytes, NULL, NULL) == SOCKET_ERROR)
	{
		THROW_EXCEPTION("AcceptEx 불러오기에 실패하였다.");
	}

	DWORD received;
	if (!AcceptEx(listener, _sock, buf, len, AddressLength, AddressLength, &received, overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			LOG_LAST_ERROR("AcceptEx");
			return false;
		}
	}

	return true;
}

bool Socket::Receive(char* buf, int len, OVERLAPPED* overlapped)
{
	Lock lock(this);

	WSABUF wsabuf;
	wsabuf.buf = buf;
	wsabuf.len = len;
	DWORD received;
	DWORD flags = 0;

	if (WSARecv(_sock, &wsabuf, 1, &received, &flags, overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LOG_LAST_ERROR("WSARecv");
			return false;
		}
	}

	return true;
}

bool Socket::Send(char* buf, int len, OVERLAPPED* overlapped)
{
	Lock lock(this);

	WSABUF wsabuf;
	wsabuf.buf = buf;
	wsabuf.len = len;
	DWORD sent = 0;

	if (WSASend(_sock, &wsabuf, 1, &sent, 0, overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LOG_LAST_ERROR("WSASend");
			return false;
		}
	}

	return true;
}

int Socket::Receive(char* buf, int len)
{
	Lock lock(this);
	return recv(_sock, buf, len, 0);
}

int Socket::Send(char* buf, int len)
{
	Lock lock(this);
	return send(_sock, buf, len, 0);
}

void Socket::Connect(ULONG ip, USHORT port)
{
	Lock lock(this);

	//SOCKADDR_IN local = { 0, };
	//int namelen = sizeof(local);
	//getsockname(_sock, (SOCKADDR*)&local, &namelen);

	SOCKADDR_IN remote = { 0, };
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = ip;

	if (connect(_sock, (SOCKADDR*)&remote, sizeof(remote)) == SOCKET_ERROR)
		THROW_EXCEPTION("연결을 실패하였다");
}

void Socket::Connect(std::string ip, USHORT port)
{
	Lock lock(this);
	Connect(inet_addr(ip.c_str()), port);
}

bool Socket::Connect(ULONG ip, USHORT port, OVERLAPPED* overlapped)
{
	Lock lock(this);

	//int namelen = sizeof(local);
	//getsockname(_sock, (SOCKADDR*)&local, &namelen);

	LPFN_CONNECTEX ConnectEx;
	GUID guid = WSAID_CONNECTEX;
	DWORD bytes;
	if (WSAIoctl(_sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
		&ConnectEx, sizeof(ConnectEx), &bytes, NULL, NULL) == SOCKET_ERROR)
	{
		THROW_EXCEPTION("ConnectEx 불러오기에 실패하였다.");
	}

	SOCKADDR_IN remote = { 0, };
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = ip;

	DWORD sent;

	if (!ConnectEx(_sock, (SOCKADDR*)&remote, sizeof(remote), NULL, 0, &sent, overlapped))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			THROW_EXCEPTION("연결을 실패하였다");
	}

	return true;
}

bool Socket::Connect(std::string ip, USHORT port, OVERLAPPED* overlapped)
{
	Lock lock(this);
	return Connect(inet_addr(ip.c_str()), port, overlapped);
}

SOCKET Socket::Detach()
{
	Lock lock(this);
	SOCKET sock = _sock;
	_sock = INVALID_SOCKET;
	return sock;
}

std::string Socket::GetLocalAddress()
{
	char name[256];
	if (gethostname(name, sizeof(name)) == SOCKET_ERROR)
	{
		LOG_LAST_ERROR("gethostname");
		return "";
	}

	HOSTENT* host = gethostbyname(name);
	if (!host)
	{
		LOG_LAST_ERROR("gethostbyname");
		return "";
	}

	return inet_ntoa(*reinterpret_cast<IN_ADDR*>(host->h_addr_list[0]));
}

bool Socket::Disconnect(OVERLAPPED* overlapped)
{
	Lock lock(this);

	if (_sock == INVALID_SOCKET)
		return false;

	LPFN_DISCONNECTEX DisconnectEx;
	GUID guid = WSAID_DISCONNECTEX;
	DWORD bytes;
	if (WSAIoctl(_sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
		&DisconnectEx, sizeof(DisconnectEx), &bytes, NULL, NULL) == SOCKET_ERROR)
	{
		THROW_EXCEPTION("DisconnectEx 불러오기에 실패하였다.");
	}

	if (!DisconnectEx(_sock, overlapped, TF_REUSE_SOCKET, 0))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
			THROW_EXCEPTION("DisconnectEx");
	}

	return true;
}

void Socket::Shutdown(int how)
{
	Lock lock(this);

	if (_sock == INVALID_SOCKET)
		return;

	if (shutdown(_sock, how) == SOCKET_ERROR)
	{
		DWORD error = WSAGetLastError();

		int second;
		int secondlen = sizeof(second);
		if (getsockopt(_sock, SOL_SOCKET, SO_CONNECT_TIME, (char*)&second, &secondlen) == SOL_SOCKET)
			LOG_LAST_ERROR("SO_CONNECT_TIME");

		if (second == 0xFFFFFFFF) // 연결되지 않은 소켓
			return;

		BOOL listen;
		int listenlen = sizeof(listen);
		if (getsockopt(_sock, SOL_SOCKET, SO_ACCEPTCONN, (char*)&listen, &listenlen) == SOCKET_ERROR)
			LOG_LAST_ERROR("SO_ACCEPTCONN");

		if (listen && WSAGetLastError() == WSAENOTCONN) // 리슨 소켓
			return;

		LOG_ERROR("소켓 셧다운 에러(%d)", error);
	}
};
