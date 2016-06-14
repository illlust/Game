#include <conio.h>
#include <utility>
#include "Acceptor.h"
#include "Socket.h"
#include "Session.h"
#include "Overlapped.h"
#include "Server.h"

//////////////////////////////////////////////////////////////////////////

Server::Server(USHORT port, ServerDispatcher* dispatcher, int pool) : _dispatcher(dispatcher), _stop(false), _connector(NULL), _id(0), i(0), j(0)
{
	_acceptor = new Acceptor(this, port, pool);
	_acceptor->Start();
}

Server::~Server()
{
	_acceptor->PostQuitStatus();
	_acceptor->Join();

	LOG("[���� ���� ���] ��������=%d/����=%d/����Ǯ=%d", i, _connection.size(), _acceptor->_pool.size());

	delete _acceptor;

	puts("��� �Ϸ��� �ƹ� Ű�� �����ʽÿ� . . . ");
	_getch();
}

void Server::Connect(std::string ip, USHORT port)
{
	Lock lock(this);

	if (_connector)
	{
		LOG_ERROR("�̹� ����Ǿ���");
		return;
	}

	Link* link = new Link(this, ip, port);
	link->Post();
}

void Server::Disconnect()
{
	Lock lock(this);

	if (!_connector)
	{
		LOG_ERROR("���ӵ��� �ʾҴ�");
		return;
	}

	_connector->Disconnect();
}

Session* Server::FindConnection(int id)
{
	Lock lock(this);

	Connection::iterator it = _connection.find(id);
	if (it == _connection.end())
		return NULL;

	return it->second;
}

Session* Server::AddConnection(SOCKET sock)
{
	Lock lock(this);

	if (_stop)
	{
		//LOG("������ �������̶� ������ �߰��� �� ����.");
		return NULL;
	}

	//LOG("����(%d)�� ���ӵǾ���.", (int)sock);

	++_id;

	Session* session = new Session(_id, this, _dispatcher, sock);
	_connection.insert(std::make_pair(_id, session));

	return session;
}

void Server::RemoveConnection(int id)
{
	Lock lock(this);

	Session* session = FindConnection(id);
	if (!session)
		LOG("������ ����(%d)�� ã�� ���ߴ�.", id);

	if (_connector && _connector->GetId() == session->GetId())
	{
		_connector->Close();
		_connector = NULL;
	}
	else
	{
		SOCKET sock = session->_sock->Detach();
		if (sock == INVALID_SOCKET)
		{
			_acceptor->AllocateSocket();
			//LOGERROR("���� ����� ����(%d)�� ���ܼ� ����Ϳ� �� ������ �Ҵ��ߴ�.", id);
			//LOG("[���� ����(%d)] %d\r\n", id, ++i);
			++i;
		}
		else
		{
			++j;
			_acceptor->PushSocket(sock);
			LOG("�������� ����Ϳ� �Ҵ���� ����(%d)�� %d��°�� ��ȯ�Ͽ���.", (int)sock, j);
		}
	}

	delete session;

	if (_connection.erase(id) != 1)
		LOG("erase ���� ���� %d ��° ����(%d)�� ������ ���ߴ�.", i, id);

	if (_stop && _connection.empty())
	{
		PostQuitStatus();
		return;
	}
}

void Server::Stop()
{
	Lock lock(this);

	_stop = true;

	if (_connection.empty())
	{
		PostQuitStatus();
		return;
	}

	for (Connection::iterator it = _connection.begin(); it != _connection.end(); ++it)
	{
		Session* session = it->second;
		session->Close();
	}
}

bool Server::IsConnected()
{
	Lock lock(this);
	return _connector != NULL;
}
