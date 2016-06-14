// LobbyServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <Windows.h>
#include <conio.h>
#include "Proactor.h"
#include "Server.h"
#include "Singleton.h"
#include "Dispatcher.h"
#include "Dump.h"
#include "Protocol.h"
#include "Ping.h"
#include "Log.h"
#include "Path.h"
#include "Session.h"
#include "Acceptor.h"
#include "Socket.h"
#include "Initialization.h"
#include "Schema.h"
#include "Database.h"
#include "MemoryLeak.h"

//////////////////////////////////////////////////////////////////////////

class Monitor : public Thread<Monitor>
{
public:
	Server* _server;
	HANDLE _event;

	Monitor(Server* server) : _server(server), _event(NULL) {}
	~Monitor()
	{
		if (_event)
		{
			CloseHandle(_event);
		}
	}

	virtual int Run()
	{
		_event = CreateEvent(NULL, FALSE, TRUE, NULL);
		while (_server->IsRun())
		{
			WaitForSingleObject(_event, 1000);

			Server::Lock lock(_server);

			LOG("'q'=종료/'c'=접속/'d'=끊기/연결=%s/접속=%d/소켓풀=%d", _server->IsConnected() ? "접속" : "끊김", _server->_connection.size(), _server->_acceptor->_pool.size());

			if (_kbhit())
			{
				switch (_getch())
				{
				case 'q':
					_server->Stop();
					break;
				case 'c':
					_server->Connect("127.0.0.1", LOGIN_PORT);
					break;
				case 'd':
					_server->Disconnect();
					break;
				}
			}
		}

		return 0;
	}

	void Stop()
	{
		SetEvent(_event);
	}
};

//////////////////////////////////////////////////////////////////////////

class LobbyServer : public Server
{
public:
	LobbyServer(USHORT port, ServerDispatcher* dispatcher, int pool) : Server(port, dispatcher, pool)
	{
		REGISTER_PACKET(dispatcher, &LobbyServer::OnCrypt);
	}

	bool OnCrypt(PACKET_S_CRYPT& crypt, Session& session)
	{
		session._key = crypt._key;

		std::string ip = _acceptor->_listener->GetLocalAddress().c_str();
		USHORT port = _acceptor->_port;
		LOG("채널 등록 %s:%d", ip.c_str(), port);

		Channel ch;
		ch.ip = inet_addr(ip.c_str());
		ch.port = port;
		session.Send(PACKET_S_REGISTER_CHANNEL(ch));
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////

class LobbyDb : public Database
{
};

//////////////////////////////////////////////////////////////////////////

int _tmain(int, _TCHAR* [])
{
	// ini, 로그, 메모리 누수 파일 초기화
	std::string module = Path::GetModuleName();
	Initialization ini(Path::RenameExtension(module, "ini"));
	TheLog::Instance()->AddOutput(Log::LOG_ALL, Path::RenameExtension(module, "log"));

#ifdef _DEBUG
	theMemoryLeak.OpenReportFile(Path::RenameExtension(module, "mem"));
#endif

	// 데이터베이스 접속
	typedef Singleton<LobbyDb> TheLobbyDb;
	TheLobbyDb::Instance()->Open("sa", "1111", "mydb", "127.0.0.1", 1433);

	// 데이터베이스 스키마 패치
	int version = ini["Database"]["Version"];
	Schema schema;
	int patch = schema.Patch(TheLobbyDb::Instance(), version);
	ini["Database"]["Version"] = patch;

	LOG("로비 서버의 포트 번호를 입력해주세요.");

	USHORT port;
	if (std::cin >> port)
	{
		LOG("로비 서버(%d)가 시작되었습니다 . . . ", port);
	}
	else
	{
		THROW_EXCEPTION();
	}

	// 서버, 서버 모니터, 핑 체크 시작
	typedef Singleton<ServerDispatcher> TheServerDispatcher;
	int pool = ini["SocketPool"]["Size"];
	LobbyServer server(port, TheServerDispatcher::Instance(), pool);
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

	LOG("로비 서버가 종료되었습니다 . . . ");

	return 0;
}
