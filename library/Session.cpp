#include <WinSock2.h>
#include "Server.h"
#include "Socket.h"
#include "Overlapped.h"
#include "Packet.h"
#include "Session.h"

//////////////////////////////////////////////////////////////////////////

Session::Session(int id, Server* server, ServerDispatcher* dispatcher, SOCKET sock) :
_id(id), _server(server), _dispatcher(dispatcher), _sock(NULL),
_receipt(NULL), _key(0), _ping(sock != INVALID_SOCKET)
{
	_sock = new Socket(sock);
	_receipt = new Receipt(server, this, _sock);
}

Session::~Session()
{
	delete _sock;
	_sock = NULL;
}

void Session::Receive()
{
	_receipt->Post();
}

void Session::Send(char* buf, int len)
{
	//printf("[%s][socket=%d][crypt=%d]\n", __FUNCTION__, _sock->_sock, _crypt);
	Transmit* transmit = new Transmit(_sock, buf, len, _key);
	transmit->Post();
}

bool Session::OnReceive(Packet* packet)
{
	//printf("[%s][socket=%d][crypt=%d]\n", __FUNCTION__, _sock->_sock, _crypt);
	_ping = true;
	return _dispatcher->Invoke(*packet->event, packet->buf, packet->len, *this);
}

void Session::Close()
{
	_sock->Close();
}

void Session::Disconnect()
{
	_sock->Shutdown(SD_BOTH);
}

int Session::GetId()
{
	return _id;
}
