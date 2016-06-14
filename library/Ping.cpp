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
				LOG("핑을 보내지 않는 소켓(%05d)을 강제 종료합니다.", session->_sock->_sock);
				session->Close(); // 네트워크 비정상 종료로 핑을 보내지 않는 클라이언트를 강제 종료시킨다.
			}
		}
	}

	return 0;
}

void Ping::Stop()
{
	SetEvent(_event);
}
