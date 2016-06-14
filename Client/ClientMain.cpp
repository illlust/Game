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
		puts("��� �Ϸ��� �ƹ� Ű�� �����ʽÿ� . . .");
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
		LOG("������ �����ؼ� ��ȣŰ(%d)�� �޾ҽ��ϴ�.", crypt._key);
		client._key = crypt._key;

		LOG("'l'=�α���/esc=���");
		if (_getch() == 'l')
		{
			//std::cin.get();
			//std::string id;
			//std::string pw;
			//std::cout << "�� �� �� : ";
			//getline(std::cin, id);
			//std::cout << "�н����� : ";
			//getline(std::cin, pw);

			std::string id = "m1";
			std::string pw = "1111";

			PACKET_C_LOGIN_REQ req(id, pw);
			LOG("������ ���̵�(%s) �н�����(%s)�� �α����մϴ�.", req._id.c_str(), req._pw.c_str());
			client.Send(req);
		}
	}

	void OnLoginFailed(PACKET_C_LOGIN_NAK&, Reactor&)
	{
		LOG("�α��ο� �����Ͽ����ϴ�.");
		LOG("�� �׷����� ��й�ȣ�� Ʋ�ȴٰų� �����϶�ų� �˷�����ϰ����� ������ ���� �׽�Ʈ ���̴� �׳� ��������� �׾��.");
	}

	void OnLogin(PACKET_C_LOGIN_ACK&, Reactor& client)
	{
		LOG("�α��ο� �����Ͽ����ϴ�.");

		LOG("'c'=ä�θ���Ʈ/esc=���");
		if (_getch() == 'c')
		{
			client.Send(PACKET_C_CHANNEL_LIST_REQ());
		}
	}

	void OnChannelList(PACKET_C_CHANNEL_LIST_ACK& ack, Reactor& client)
	{
		if (ack._ch.empty())
			return;

		LOG("%d���� ä�� ����Ʈ�� �޾ҽ��ϴ�.", ack._ch.size());
		//std::copy
		//copy(ack._ch.begin(), ack._ch.end(), std::ostream_iterator<std::string>(std::cout, "\r\n"));

		LOG("'1'~'%d'=ä������/esc=���", ack._ch.size());
		int sel = _getch();
		if (!isdigit(sel))
			return;

		PACKET_C_CHANNEL_LIST_ACK::ChannelList::iterator it = ack._ch.begin();
		std::advance(it, sel - '1');
		Channel ch = *it;
		std::cout << (std::string)ch << " ������ �����մϴ�." << std::endl;

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

	LOG("���� Ŭ���̾�Ʈ�� ������ �Է����ּ���.");

	int dummy;
	if (std::cin >> dummy)
	{
		LOG("%d���� ���� Ŭ���̾�Ʈ�� ���۵Ǿ����ϴ� . . . ", dummy);
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

	LOG("���� Ŭ���̾�Ʈ�� ����Ǿ����ϴ� . . . ");

	return 0;
}
