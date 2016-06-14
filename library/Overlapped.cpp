#include <WinSock2.h>
#include <MSWSock.h>
#include "Proactor.h"
#include "Server.h"
#include "Dispatcher.h"
#include "Crypt.h"
#include "Log.h"
#include "Socket.h"
#include "Packet.h"
#include "PacketQueue.h"
#include "Crypt.h"
#include "Session.h"
#include "Protocol.h"
#include "Acceptor.h"
#include "Overlapped.h"

//////////////////////////////////////////////////////////////////////////

Overlapped::Overlapped()
{
	Initialize();
}

void Overlapped::Destroy()
{
	delete this;
}

void Overlapped::Post()
{
	Initialize();
	OnPost();
}

void Overlapped::Initialize()
{
	Internal = 0;
	InternalHigh = 0;
	Offset = 0;
	OffsetHigh = 0;
	hEvent = NULL;
}

void Overlapped::Error()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

Accept::Accept(Acceptor* acceptor, Server* server) :
_acceptor(acceptor), _server(server), _buf(NULL)
{
	_sock = new Socket;
	_buf = new char[Socket::AddressLength * 2];
}

Accept::~Accept()
{
	_acceptor->PushSocket(_sock->Detach());
	delete _sock;
	delete[] _buf;
}

void Accept::OnPost()
{
	_sock->_sock = _acceptor->PopSocket();
	if (!_sock->Accept(_acceptor->_listener->_sock, _buf, 0, this))
	{
		LOG("AcceptEx 호출에 실패하였다.");
		Error();
		THROW_EXCEPTION("AcceptEx 호출에 실패하였다.");
	}
}

void Accept::Complete(DWORD)
{
	if (_server->_stop)
	{
		LOG("서버가 종료중이라 억셉트를 취소한다.");
		Error();
		return;
	}

	if (setsockopt(_sock->_sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_acceptor->_listener->_sock, sizeof(_acceptor->_listener->_sock)) == SOCKET_ERROR)
		LOG_LAST_ERROR("SO_UPDATE_ACCEPT_CONTEXT");

	LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs;
	GUID guid = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD bytes;
	if (WSAIoctl(_sock->_sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
		&GetAcceptExSockaddrs, sizeof(GetAcceptExSockaddrs), &bytes, NULL, NULL) == SOCKET_ERROR)
		THROW_EXCEPTION("GetAcceptExSockaddrs 불러오기에 실패하였다.");

	//SOCKADDR* local = NULL;
	//SOCKADDR* remote = NULL;
	//int locallen = 0;
	//int remotelen = 0;
	//GetAcceptExSockaddrs(_buf, 0, Socket::AddressLength, Socket::AddressLength,
	//	&local, &locallen, &remote, &remotelen);
	//if (!local || !remote)
	//	THROW_EXCEPTION("GetAcceptExSockaddrs 호출이 실패하였다.");

	//_sock->_local = *reinterpret_cast<SOCKADDR_IN*>(local);
	//_sock->_remote = *reinterpret_cast<SOCKADDR_IN*>(remote);

	//printf("[%s][socket=%d][local=%s][remote=%s]\n", __FUNCTION__, _sock->_sock, _sock->GetLocalAddress().c_str(), _sock->GetRemoteAddress().c_str());

	Socket* sock = new Socket(_sock->Detach());
	Connect* connect = new Connect(_server, sock);
	connect->Post();

	Post();
}

void Accept::Error()
{
	// 서버 종료로 인한 에러이므로 아무 것도 하지 않고
	// 억셉터에서 소켓을 닫고 delete 해주길 기다린다
}

//////////////////////////////////////////////////////////////////////////

Receipt::Receipt(Server* server, Session* session, Socket* sock) :
_q(NULL), _key(0), _session(session), _server(server), _sock(sock)
{
	_q = new PacketQueue(1024, 128, 16);
}

Receipt::~Receipt()
{
	delete _q;
}

void Receipt::OnPost()
{
	if (!_sock->Receive(_q->Tail(), _q->Remain(), this))
		Error();
}

void Receipt::Complete(DWORD transferred)
{
	if (transferred == 0)
	{
		Error();
		return;
	}

	_q->_tail += transferred;

	while (_q->HasPacket())
	{
		Packet packet;
		_q->Pop(&packet);

		Crypt crypt;
		char bcc = crypt.Decrypt(_key, packet.buf - EVENTSIZE, packet.buf - EVENTSIZE, packet.header - FOOTERSIZE);
		if (packet.bcc != bcc)
			THROW_EXCEPTION("%d", _key);

		if (!_session->OnReceive(&packet))
		{
			Error();
			return;
		}
	}

	Post();
}

void Receipt::Error()
{
	Disconnect* disconnect = new Disconnect(_server, _session->GetId());
	disconnect->Post();

	Destroy();
}

//////////////////////////////////////////////////////////////////////////

Transmit::Transmit(Socket* sock, char* buf, int len, char key) :
_sock(sock), _buf(NULL), _len(HEADERSIZE + len + FOOTERSIZE)
{
	_buf = new char[HEADERSIZE + len + FOOTERSIZE];

	short header = static_cast<short>(len + FOOTERSIZE);
	CopyMemory(_buf, &header, HEADERSIZE);

	Crypt crypt;
	char bcc = crypt.Encrypt(key, _buf + HEADERSIZE, buf, len);
	int pos = _buf + HEADERSIZE + len - _buf;
	_buf[pos] = bcc;
}

Transmit::~Transmit()
{
	delete[] _buf;
}

void Transmit::OnPost()
{
	if (!_sock->Send(_buf, _len, this))
	{
		Error();
	}
}

void Transmit::Complete(DWORD transferred)
{
	if (transferred != static_cast<DWORD>(_len))
		THROW_EXCEPTION("IOCP에서 전송을 요청한 크기와 완료된 크기가 다르다.");

	Destroy();
}

void Transmit::Error()
{
	//LOG_ERROR("송신에 실패하였다.");
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

Connect::Connect(Server* server, Socket* sock) :
_server(server), _sock(sock)
{

}

Connect::~Connect()
{
	delete _sock;
}

void Connect::OnPost()
{
	if (!_server->PostStatus(this))
	{
		LOG_ERROR("접속 연결에 실패하였다.");
		Error();
	}
}

void Connect::Complete(DWORD)
{
	SOCKET sock = _sock->Detach();
	_server->Register(sock);

	Session* session = _server->AddConnection(sock);
	if (!session)
	{
		LOG("세션을 찾을 수 없어서 억셉터에 할당받은 소켓을 반환한다.");
		_server->_acceptor->PushSocket(sock);
		Error();
		return;
	}

	PACKET_S_CRYPT crypt;
	crypt.Crypt();
	session->Send(crypt);

	session->_key = crypt._key;
	session->_receipt->_key = crypt._key;
	session->Receive();

	Destroy();
}

void Connect::Error()
{
	//LOGERROR("커넥트에 실패하였다.");
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

Disconnect::Disconnect(Server* server, int id) :
_server(server), _id(id)
{

}

Disconnect::~Disconnect()
{

}

void Disconnect::OnPost()
{
	Session* session = _server->FindConnection(_id);
	if (!session)
	{
		Error();
		return;
	}

	if (!session->_sock->Disconnect(this))
	{
		_server->RemoveConnection(_id);
		Error();
		return;
	}
}

void Disconnect::Complete(DWORD)
{
	_server->RemoveConnection(_id);
	Destroy(); 
}

void Disconnect::Error()
{
	//LOGERROR("디스커넥스에 실패하였다.");
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

Link::Link(Server* server, std::string ip, USHORT port) :
_server(server), _ip(ip), _port(port), _sock(NULL)
{
	_sock = new Socket;
}

Link::~Link()
{
	delete _sock;
}

void Link::OnPost()
{
	_sock->Open();
	_sock->Bind(0, 0);
	_server->Register(_sock->_sock);
	_sock->Connect(_ip, _port, this);
}

void Link::Complete(DWORD)
{
	if (setsockopt(_sock->_sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) == SOCKET_ERROR)
		LOG_LAST_ERROR("SO_UPDATE_CONNECT_CONTEXT");

	//SOCKADDR_IN name;
	//int namelen = sizeof(name);
	//getsockname(_sock._sock, (SOCKADDR*)&name, &namelen);
	//_sock._local = name;

	SOCKET sock = _sock->Detach();
	_server->_connector = _server->AddConnection(sock);
	_server->_connector->Receive();

	Destroy();
}
