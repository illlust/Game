#pragma once

#include <Windows.h>
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////////

class Overlapped : public OVERLAPPED
{
protected:
	virtual ~Overlapped() {}

private:
	virtual void OnPost() = 0;

public:
	Overlapped();
	void Initialize();
	void Destroy();
	void Post();
	virtual void Complete(DWORD transferred) = 0;
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class Acceptor;
class Server;
class Socket;

class Accept : public Overlapped, public MemoryPoolObject<Accept>
{
private:
	virtual ~Accept();
	virtual void OnPost();

public:
	Acceptor* _acceptor;
	Server* _server;
	Socket* _sock;
	char* _buf;

	Accept(Acceptor* acceptor, Server* server);

	virtual void Complete(DWORD transferred);
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class PacketQueue;
class Session;
class Socket;

class Receipt : public Overlapped, public MemoryPoolObject<Receipt>
{
private:
	virtual ~Receipt();
	virtual void OnPost();

public:
	PacketQueue* _q;
	Session* _session;
	Socket* _sock;
	char _key;
	Server* _server;

	Receipt(Server* server, Session* session, Socket* sock);

	virtual void Complete(DWORD transferred);
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class Socket;

class Transmit : public Overlapped, public MemoryPoolObject<Transmit>
{
private:
	virtual ~Transmit();
	virtual void OnPost();

public:
	Socket* _sock;
	char* _buf; // 메모리풀 적용해야함
	int _len;

	Transmit(Socket* sock, char* buf, int len, char key);

	virtual void Complete(DWORD transferred);
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class Server;
class Socket;

class Connect : public Overlapped, public MemoryPoolObject<Connect>
{
private:
	virtual ~Connect();
	virtual void OnPost();

public:
	Server* _server;
	Socket* _sock;

	Connect(Server* server, Socket* sock);

	virtual void Complete(DWORD transferred);
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class Server;
class Socket;

class Disconnect : public Overlapped, public MemoryPoolObject<Disconnect>
{
private:
	virtual ~Disconnect();
	virtual void OnPost();

public:
	Server* _server;
	int _id;

	Disconnect(Server* server, int id);

	virtual void Complete(DWORD transferred);
	virtual void Error();
};

//////////////////////////////////////////////////////////////////////////

class Server;
class Socket;

class Link : public Overlapped, public MemoryPoolObject<Link>
{
private:
	virtual ~Link();
	virtual void OnPost();

public:
	std::string _ip;
	USHORT _port;
	Server* _server;
	Socket* _sock;

	Link(Server* server, std::string ip, USHORT port);

	virtual void Complete(DWORD transferred);
};
