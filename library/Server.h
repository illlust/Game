#pragma once

#include <unordered_map>
#include "WSALoader.h"
#include "Proactor.h"
#include "Lock.h"
#include "Dispatcher.h"

//////////////////////////////////////////////////////////////////////////

class Database;
class Session;
class Acceptor;

class Server : public WSALoader, public Proactor, public Lockable<Server>
{
public:
	int i, j;
	int _id;

	Database* _db;
	ServerDispatcher* _dispatcher;
	Acceptor* _acceptor;
	Session* _connector;
	bool _stop;

	typedef std::tr1::unordered_map<int, Session*> Connection;
	Connection _connection;

	Server(USHORT port, ServerDispatcher* dispatcher, int pool = 0);
	virtual ~Server();

	void Connect(std::string ip, USHORT port);
	void Disconnect();
	Session* FindConnection(int id);
	Session* AddConnection(SOCKET sock);
	void RemoveConnection(int id);

	void Stop();
	bool IsConnected();
};
