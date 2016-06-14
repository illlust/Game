// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <winsock2.h>
#include <string>
#include <algorithm>
#include <functional>
#include <list>
#include <set>
#include "Protocol.h"
#include "WSALoader.h"
#include "Serialization.h"
#include "Dump.h"
#include "Reactor.h"
#include "Dispatcher.h"
#include "Thread.h"
#include "Lock.h"
#include "Log.h"
#include "Path.h"
#include "MemoryLeak.h"

//////////////////////////////////////////////////////////////////////////

class Reactor;

class Game : public WSALoader, public Thread<Game>, public Lockable<Game>
{
public:
	typedef std::list<Reactor*> Connection;
	Connection _connection;

	int _dummy;

	Game(int dummy) : _dummy(dummy) {}

	virtual ~Game()
	{
		puts("계속 하려면 아무 키나 누르십시오 . . .");
		_getch();
	}

	void Stop()
	{
		std::for_each(_connection.begin(), _connection.end(), std::tr1::mem_fn(&Reactor::Disconnect));
	}

	virtual int Run()
	{
		for (int i = 0; i < _dummy; ++i)
		{
			ClientDispatcher* dispatcher = new ClientDispatcher;

			REGISTER_PACKET(dispatcher, &Game::OnCrypt);
			REGISTER_PACKET(dispatcher, &Game::OnLogin);
			REGISTER_PACKET(dispatcher, &Game::OnLoginFailed);
			REGISTER_PACKET(dispatcher, &Game::OnChannelList);
			REGISTER_PACKET(dispatcher, &Game::OnHelloWorld);

			Reactor* client = new Reactor(dispatcher);
			_connection.push_back(client);

			client->Connect("127.0.0.1", LOGIN_PORT);
			client->Start();
		}

		for (Connection::iterator it = _connection.begin(); it != _connection.end(); ++it)
		{
			Reactor* client = *it;
			client->Join();
			delete client->_dispatcher;
			delete client;
		}

		return 0;
	}

	void OnCrypt(PACKET_S_CRYPT& crypt, Reactor& client)
	{
		LOG("서버에 접속해서 암호키(%d)를 받았습니다.", crypt._key);
		client._key = crypt._key;

		LOG("'l'=로그인/esc=취소");
		if (_getch() == 'l')
		{
			//std::cin.get();
			//std::string id;
			//std::string pw;
			//std::cout << "아 이 디 : ";
			//getline(std::cin, id);
			//std::cout << "패스워드 : ";
			//getline(std::cin, pw);

			std::string id = "m1";
			std::string pw = "1111";

			PACKET_C_LOGIN_REQ req(id, pw);
			LOG("서버에 아이디(%s) 패스워드(%s)로 로그인합니다.", req._id.c_str(), req._pw.c_str());
			client.Send(req);
		}
	}

	void OnLoginFailed(PACKET_C_LOGIN_NAK&, Reactor&)
	{
		LOG("로그인에 실패하였습니다.");
		LOG("왜 그랬는지 비밀번호가 틀렸다거나 가입하라거나 알려줘야하겠지만 지금은 더미 테스트 중이니 그냥 강제종료로 죽어라.");
	}

	void OnLogin(PACKET_C_LOGIN_ACK&, Reactor& client)
	{
		LOG("로그인에 성공하였습니다.");

		LOG("'c'=채널리스트/esc=취소");
		if (_getch() == 'c')
		{
			client.Send(PACKET_C_CHANNEL_LIST_REQ());
		}
	}

	void OnChannelList(PACKET_C_CHANNEL_LIST_ACK& ack, Reactor& client)
	{
		if (ack._ch.empty())
			return;

		LOG("%d개의 채널 리스트를 받았습니다.", ack._ch.size());
		//std::copy
		//copy(ack._ch.begin(), ack._ch.end(), std::ostream_iterator<std::string>(std::cout, "\r\n"));

		LOG("'1'~'%d'=채널접속/esc=취소", ack._ch.size());
		int sel = _getch();
		if (!isdigit(sel))
			return;

		PACKET_C_CHANNEL_LIST_ACK::ChannelList::iterator it = ack._ch.begin();
		std::advance(it, sel - '1');
		Channel ch = *it;
		std::cout << (std::string)ch << " 서버에 접속합니다." << std::endl;

		client.Close();
		client.Connect(ch.ip, ch.port);
	}

	void OnHelloWorld(PACKET_C_HELLO_WORLD_ACK& ack, Reactor& client)
	{
		LOG("%s", ack._message.c_str());
		Sleep(rand() % 1000);
		client.Send(PACKET_C_HELLO_WORLD_ACK());
	}
};

//////////////////////////////////////////////////////////////////////////

class Monitor : public Thread<Monitor>
{
public:
	Game* _game;

	Monitor(Game* game) : _game(game) {}

	virtual int Run()
	{
		while (_game->IsRun())
		{
			if (_kbhit())
			{
				switch(_getch())
				{
				case 'q':
				default:
					_game->Stop();
					break;
				}
			}

			Sleep(0);
		}

		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////

int _tmain(int, _TCHAR* [])
{
	std::string module = Path::GetModuleName();
	TheLog::Instance()->AddOutput(Log::LOG_ALL, Path::RenameExtension(module, "log"));

#ifdef _DEBUG
	theMemoryLeak.OpenReportFile(Path::RenameExtension(module, "mem"));
#endif

	LOG("더미 클라이언트의 개수를 입력해주세요.");

	int dummy;
	if (std::cin >> dummy)
	{
		LOG("%d개의 더미 클라이언트가 시작되었습니다 . . . ", dummy);
	}
	else
	{
		THROW_EXCEPTION();
	}

	Game game(dummy);
	Monitor monitor(&game);

	game.Start();
	monitor.Start();

	game.Join();
	monitor.Join();

	LOG("더미 클라이언트가 종료되었습니다 . . . ");

	return 0;
}
