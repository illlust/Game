#pragma once

#include <Windows.h>
#include <string>
#include "Lock.h"
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////////

class Socket : public Lockable<Socket>, public MemoryPoolObject<Socket>
{
	using Lockable<Socket>::Lock;

public:
	enum { AddressLength = sizeof(SOCKADDR_IN) + 16 };

	explicit Socket(SOCKET sock = INVALID_SOCKET);
	virtual ~Socket();

	void Open();
	void Bind(DWORD ip, WORD port);
	void Listen();
	SOCKET Accept(SOCKADDR_IN* addr);
	void Close();
	void Connect(ULONG ip, USHORT port);
	void Connect(std::string ip, USHORT port);
	bool Connect(ULONG ip, USHORT port, OVERLAPPED* overlapped);
	bool Connect(std::string ip, USHORT port, OVERLAPPED* overlapped);
	bool Accept(SOCKET listener, char* buf, int len, OVERLAPPED* overlapped);
	bool Receive(char* buf, int len, OVERLAPPED* overlapped);
	bool Send(char* buf, int len, OVERLAPPED* overlapped);
	int Receive(char* buf, int len);
	int Send(char* buf, int len);
	bool Disconnect(OVERLAPPED* overlapped);

	SOCKET Detach();
	std::string GetLocalAddress();
	void Shutdown(int how);

public:
	SOCKET _sock;
};
