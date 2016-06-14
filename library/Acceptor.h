#pragma once

#include <Windows.h>
#include <vector>
#include <list>
#include "Proactor.h"

//////////////////////////////////////////////////////////////////////////

class Socket;
class Accept;
class Server;

class Acceptor : public Proactor, public Lockable<Acceptor>
{
public:
	enum { BACKLOG = 20 };

	//Server* _server;
	Socket* _listener;
	typedef std::list<SOCKET> SocketPool;
	SocketPool _pool;
	typedef std::vector<Accept*> AcceptPool;
	AcceptPool _accept;
	USHORT _port;

	void AllocateSocket();
	void PushSocket(SOCKET sock);
	SOCKET PopSocket();

	Acceptor(Server* server, USHORT port, int pool);
	virtual ~Acceptor();
};
