#pragma once

#include "MemoryPool.h"
#include "Dispatcher.h"

//////////////////////////////////////////////////////////////////////////

class Server;
class Receipt;
class Packet;
class Socket;

class Session : public MemoryPoolObject<Session>
{
public:
	Server* _server;
	ServerDispatcher* _dispatcher;
	Socket* _sock;
	Receipt* _receipt;
	char _key;
	bool _ping;
	int _id;

	Session(int id = 0, Server* server = NULL, ServerDispatcher* dispatcher = NULL, SOCKET sock = INVALID_SOCKET);
	virtual ~Session();

	void Receive();
	bool OnReceive(Packet* packet);
	void Close();
	void Disconnect();
	int GetId();

	template<class T>
	void Send(const T& object)
	{
		Archive<1024> ar;
		ar << object;
		Send(ar.Peek(), ar.Size());
	}

private:
	void Send(char* buf, int len);
};
