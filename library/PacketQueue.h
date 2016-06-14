#pragma once

//////////////////////////////////////////////////////////////////////////

class Packet;

class PacketQueue
{
public:
	char* _buf;
	char* _head;
	char* _tail;
	int _capacity;
	int _overfull;

	PacketQueue(int size = 1024, int num = 128, int overfull = 32);
	virtual ~PacketQueue();

	int Remain();
	char* Tail();
	void Push(char* buf, int len);
	bool HasPacket();
	void Pop(Packet* packet);
};
