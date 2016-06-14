#include <WinSock2.h>
#include <algorithm>
#include <functional> 
#include <utility> 
#include "Socket.h"
#include "Overlapped.h"
#include "Log.h"
#include "Server.h"
#include "Acceptor.h"

//////////////////////////////////////////////////////////////////////////

Acceptor::Acceptor(Server* server, USHORT port, int pool) : _port(port)
{
	_listener = new Socket;
	_listener->Open();
	_listener->Bind(INADDR_ANY, port);

	BOOL on = TRUE;
	if (setsockopt(_listener->_sock, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(on)))
		LOG_LAST_ERROR("SO_CONDITIONAL_ACCEPT");

	_listener->Listen();

	Register(_listener->_sock);

	for (int i = 0; i < pool + BACKLOG; ++i)
	{
		AllocateSocket();
	}

	for (int i = 0; i < BACKLOG; ++i)
	{
		Accept* accept = new Accept(this, server);
		_accept.push_back(accept);
		accept->Post();
	}
}

Acceptor::~Acceptor()
{
	LOG("[����� ���� ����] ����Ǯ=%d", _pool.size());

	std::for_each(_accept.begin(), _accept.end(), std::tr1::mem_fn(&Accept::Destroy));
	std::for_each(_pool.begin(), _pool.end(), closesocket);
	_listener->Close();
	delete _listener;

	LOG("[����� ���� ���] ����Ǯ=%d", _pool.size());
}

void Acceptor::PushSocket(SOCKET sock)
{
	Lock lock(this);

	if (sock == INVALID_SOCKET)
		THROW_EXCEPTION("��ȿ���� ���� ������ �߰��Ϸ��� �ߴ�.");

	_pool.push_back(sock);
	//LOG("������ ����� ����(%d)�� Ǯ�� �߰��ߴ�.", sock);
}

SOCKET Acceptor::PopSocket()
{
	Lock lock(this);

	if (_pool.empty())
	{
		AllocateSocket();
		LOG_ERROR("����Ǯ�� �� ���ο� ����(%d)�� �Ҵ��ߴ�.", _pool.back());
	}

	SOCKET sock = _pool.back();
	_pool.pop_back();
	return sock;
}

void Acceptor::AllocateSocket()
{
	Lock lock(this);

	Socket sock;
	sock.Open();
	_pool.push_back(sock.Detach());
}
