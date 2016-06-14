#include <Windows.h>
#include <assert.h>
#include "Packet.h"
#include "Protocol.h"
#include "PacketQueue.h"

//////////////////////////////////////////////////////////////////////////

PacketQueue::PacketQueue(int size, int num, int overfull)
{
	_capacity = size * num;
	_overfull = overfull;

	_buf = new char[_capacity];
	_head = _buf;
	_tail = _buf;
}

PacketQueue::~PacketQueue()
{
	delete[] _buf;
}

int PacketQueue::Remain()
{
	return _capacity - (_tail - _buf);
}

char* PacketQueue::Tail()
{
	return _tail;
}

void PacketQueue::Push(char* buf, int len)
{
	assert(Remain() > len);

	CopyMemory(_tail, buf, len);
	_tail += len;
}

bool PacketQueue::HasPacket()
{
	int size = _tail - _head;
	if (size <= HEADERSIZE)
		return false;

	short header;
	CopyMemory(&header, _head, HEADERSIZE);
	return size >= HEADERSIZE + header;
}

void PacketQueue::Pop(Packet* packet)
{
	if (_tail - _buf > _overfull)
	{
		int size = _tail - _head;
		MoveMemory(_buf, _head, size);
		_head = _buf;
		_tail = _buf + size;
	}

	short len;
	CopyMemory(&len, _head, HEADERSIZE);

	packet->header = len;
	packet->event = reinterpret_cast<int*>(_head + HEADERSIZE);
	packet->buf = _head + HEADERSIZE + EVENTSIZE;
	packet->len = len - EVENTSIZE - FOOTERSIZE;
	packet->bcc = packet->buf[packet->len];

	_head += HEADERSIZE;
	_head += len;
}
