#pragma once

#include <Windows.h>
#include <set>
#include "Thread.h"

//////////////////////////////////////////////////////////////////////////

class Proactor : public Thread<Proactor>
{
public:
	HANDLE _iocp;

	typedef std::set<SOCKET> Association;
	Association _association;

public:
	Proactor();
	virtual ~Proactor();

	virtual int Run();
	bool Register(SOCKET sock);
	bool PostStatus(OVERLAPPED* overlapped);
	void PostQuitStatus();

	static int Cpu();
};
