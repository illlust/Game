// LoginServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <string>
#include <iostream>
#include <functional>
#include <Shlwapi.h>
#include <vector>
#include <conio.h>
#include "Protocol.h"
#include "Dispatcher.h"
#include "WSALoader.h"
#include "Serialization.h"
#include "PacketQueue.h"
#include "Dump.h"
#include "Database.h"
#include "Initialization.h"
#include "Thread.h"
#include "Server.h"
#include "Singleton.h"
#include "Dispatcher.h"
#include "Ping.h"
#include "Socket.h"
#include "Session.h"
#include "Log.h"
#include "Acceptor.h"
#include "Path.h"
#include "Overlapped.h"
#include "MemoryLeak.h"

//////////////////////////////////////////////////////////////////////////

class Monitor : public Thread<Monitor>
{
public:
	HANDLE _event;
	Server* _server;

	Monitor(Server* server) : _server(server), _event(NULL)
	{
	}

	~Monitor()
	{
		if (_event)
		{
			CloseHandle(_event);
		}
	}

	void Stop()
	{
		SetEvent(_event);
	}

	virtual int Run()
	{
		_event = CreateEvent(NULL, FALSE, TRUE, NULL);

		while (_server->IsRun())
		{
			WaitForSingleObject(_event, 1000);

			Server::Lock lock(_server);

			LOG("'q'=종료/접속=%d/소켓풀=%d", _server->_connection.size(), _server->_acceptor->_pool.size());

			if (_kbhit())
			{
				switch (_getch())
				{
				case 'q':
					LOG("[서버 종료 시작] 강제종료=%d/접속=%d/소켓풀=%d", _server->i, _server->_connection.size(), _server->_acceptor->_pool.size());
					_server->Stop();
					break;
				}
			}
		}

		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////

class LoginDb : public Database
{
public:
	bool IsMember(std::string id, std::string password)
	{
		const char sql[] = "select * from t1 where member_id = '%s' and member_password = '%s'";
		Records rs = Select(sql, id.c_str(), password.c_str());
		return rs != NULL;
	}
};

//////////////////////////////////////////////////////////////////////////

class LoginServer : public Server
{
public:
	LoginDb* _db;
	PACKET_C_CHANNEL_LIST_ACK _ch;

public:
	LoginServer(USHORT port, ServerDispatcher* dispatcher, int pool, LoginDb* db) : Server(port, dispatcher, pool), _db(db)
	{
		REGISTER_PACKET(dispatcher, &LoginServer::OnRegisterChannel);
		REGISTER_PACKET(dispatcher, &LoginServer::OnLogin);
		REGISTER_PACKET(dispatcher, &LoginServer::OnChannelList);

		RegisterCallback(dispatcher, C_HELLO_WORLD_REQ, &LoginServer::OnHelloWorld, this);
		RegisterCallback(dispatcher, C_ARG0, &LoginServer::OnArg0, this);
		RegisterCallback(dispatcher, C_ARG1, &LoginServer::OnArg1, this);
		RegisterCallback(dispatcher, C_ARG2, &LoginServer::OnArg2, this);
		RegisterCallback(dispatcher, C_ARG3, &LoginServer::OnArg3, this);
		RegisterCallback(dispatcher, C_ARG0_PEER, &LoginServer::OnArg0Caller, this);
		RegisterCallback(dispatcher, C_ARG1_PEER, &LoginServer::OnArg1Caller, this);
		RegisterCallback(dispatcher, C_ARG2_PEER, &LoginServer::OnArg2Caller, this);
		RegisterCallback(dispatcher, C_ARG3_PEER, &LoginServer::OnArg3Caller, this);
	}

	bool OnRegisterChannel(PACKET_S_REGISTER_CHANNEL& req, Session&)
	{
		//std::string ip = session._sock->GetRemoteAddress().c_str();
		//USHORT port = session._sock->GetRemotePort();
		//_ch.AddChannel(ip.c_str(), port);
		_ch._ch.insert(req._ch);
		return true;
	}

	bool OnLogin(PACKET_C_LOGIN_REQ& req, Session& session)
	{
		if (_db->IsMember(req._id, req._pw))
		{
			session.Send(PACKET_C_LOGIN_ACK());
		}
		else
		{
			session.Send(PACKET_C_LOGIN_NAK());
			session.Disconnect();
		}

		return true;
	}

	bool OnChannelList(PACKET_C_CHANNEL_LIST_REQ&, Session& session)
	{
		session.Send(_ch);	
		return true;
	}

	bool OnHelloWorld(PACKET_C_HELLO_WORLD_REQ&, Session& session)
	{
		session.Send(PACKET_C_HELLO_WORLD_ACK());
		return true;
	}

	bool OnArg0Caller(Session&)
	{
		return true;
	}

	bool OnArg1Caller(int, Session&)
	{
		return true;
	}

	bool OnArg2Caller(int*, int, Session&)
	{
		return true;
	}

	bool OnArg3Caller(int&, int*, int, Session&)
	{
		return true;
	}

	bool OnArg0()
	{
		return true;
	}

	bool OnArg1(int)
	{
		return false;
	}

	bool OnArg2(int*, int)
	{
		return false;
	}

	bool OnArg3(int&, int*, int)
	{
		return false;
	}
};

//////////////////////////////////////////////////////////////////////////

int _tmain(int, _TCHAR* [])
{
	std::string module = Path::GetModuleName();
	Initialization ini(Path::RenameExtension(module, "ini"));
	int pool = ini["SocketPool"]["Size"];
	TheLog::Instance()->AddOutput(Log::LOG_ALL, Path::RenameExtension(module, "log"));

#ifdef _DEBUG
	theMemoryLeak.OpenReportFile(Path::RenameExtension(module, "mem"));
#endif

	LOG("로그인 서버가 시작되었습니다 . . . ");

	typedef Singleton<ServerDispatcher> TheServerDispatcher;
	typedef Singleton<LoginDb> TheLoginDb;

	TheLoginDb::Instance()->Open("sa", "1111", "mydb", "127.0.0.1", 1433);

	LoginServer server(LOGIN_PORT, TheServerDispatcher::Instance(), pool, TheLoginDb::Instance());
	Monitor monitor(&server);
	Ping ping(&server);

	server.Start(Proactor::Cpu());
	monitor.Start();
	ping.Start();

	server.Join();

	monitor.Stop();
	monitor.Join();
	ping.Stop();
	ping.Join();

	LOG("로그인 서버가 종료되었습니다 . . . ");

	return 0;
}
