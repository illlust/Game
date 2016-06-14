#pragma once

#include "Thread.h"

//////////////////////////////////////////////////////////////////////////

class Server;

class Ping : public Thread<Ping>
{
public:
	Server* _server;
	int _second;
	HANDLE _event;

	Ping(Server* server, int second = 60);
	~Ping();

	virtual int Run();
	void Stop();
};
