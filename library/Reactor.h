#pragma once

#include <Windows.h>
#include "Lock.h"
#include "Thread.h"
#include "Dispatcher.h"

//////////////////////////////////////////////////////////////////////////

class PacketQueue;

class Reactor : public Thread<Reactor>
{
public:
	SOCKET _sock;
	WSAEVENT _wsaevent;
	PacketQueue* _q;
	char* _buf;
	char* _head;
	char* _tail;
	bool _pending;
	ClientDispatcher* _dispatcher;
	char _key;

	Reactor(ClientDispatcher* dispatcher);
	virtual ~Reactor();

	virtual int Run();

	void Connect(ULONG ip, USHORT port);
	void Connect(std::string ip, USHORT port);
	void Disconnect();

	void OnConnect();
	void OnDisconnect();
	void OnReceive();
	void OnSend();

	template<class T>
	void Send(const T& object)
	{
		Archive<1024> ar;
		ar << object;
		SendPacket(ar.Peek(), ar.Size());
	}

	void Close();

private:
	void Send(char* buf, int len);
	void SendPacket(char* buf, int len);
	void Shutdown(int how);
};
