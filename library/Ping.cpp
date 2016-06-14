#include <WinSock2.h>
#include <Windows.h>
#include "Server.h"
#include "Protocol.h"
#include "Overlapped.h"
#include "Socket.h"
#include "Ping.h"
#include "Session.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////////

Ping::Ping(Server* server, int second) : _server(server), _second(second)
{

}

Ping::~Ping()
{
	if (_event)
	{
		CloseHandle(_event);
	}
}

int Ping::Run()
{
	_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	while (_server->IsRun())
	{
		WaitForSingleObject(_event, _second * 1000);

		Server::Lock lock(_server);

		for (Server::Connection::iterator it = _server->_connection.begin(); it != _server->_connection.end(); ++it)
		{
			Session* session = it->second;

			if (session->_ping)
			{
				session->_ping = false;
			}
			else
			{
				LOG("���� ������ �ʴ� ����(%05d)�� ���� �����մϴ�.", session->_sock->_sock);
				session->Close(); // ��Ʈ��ũ ������ ����� ���� ������ �ʴ� Ŭ���̾�Ʈ�� ���� �����Ų��.
			}
		}
	}

	return 0;
}

void Ping::Stop()
{
	SetEvent(_event);
}
