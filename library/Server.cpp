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

	LOG("[서버 종료 결과] 강제종료=%d/접속=%d/소켓풀=%d", i, _connection.size(), _acceptor->_pool.size());

	delete _acceptor;

	puts("계속 하려면 아무 키나 누르십시오 . . . ");
	_getch();
}

void Server::Connect(std::string ip, USHORT port)
{
	Lock lock(this);

	if (_connector)
	{
		LOG_ERROR("이미 연결되었다");
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
		LOG_ERROR("접속되지 않았다");
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
		//LOG("서버가 종료중이라 세션을 추가할 수 없다.");
		return NULL;
	}

	//LOG("세션(%d)이 접속되었다.", (int)sock);

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
		LOG("종료할 세션(%d)을 찾지 못했다.", id);

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
			//LOGERROR("강제 종료된 세션(%d)이 생겨서 억셉터에 새 소켓을 할당했다.", id);
			//LOG("[강제 종료(%d)] %d\r\n", id, ++i);
			++i;
		}
		else
		{
			++j;
			_acceptor->PushSocket(sock);
			LOG("서버에서 억셉터에 할당받은 소켓(%d)을 %d번째로 반환하였다.", (int)sock, j);
		}
	}

	delete session;

	if (_connection.erase(id) != 1)
		LOG("erase 강제 종료 %d 번째 세션(%d)을 지우지 못했다.", i, id);

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
