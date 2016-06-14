#pragma once

#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <Mmsystem.h>
#include <time.h>
#include "Lock.h"

//////////////////////////////////////////////////////////////////////////

template<class T>
unsigned WINAPI StartAddress(void* object);

//////////////////////////////////////////////////////////////////////////

template<class T>
class Thread
{
	friend unsigned WINAPI StartAddress<Thread<T>>(void* object);

protected:
	HANDLE _thread[MAXIMUM_WAIT_OBJECTS];
	int _num;

protected:
	virtual int Run() = 0;

public:

	Thread() : _num(0)
	{
		ZeroMemory(_thread, sizeof(_thread));
	}

	virtual ~Thread() {}

	void Spawn(void*) {}

	void Start(int num = 1)
	{
		_num = (std::min)(num, MAXIMUM_WAIT_OBJECTS);

		for (int i = 0; i < _num; ++i)
		{
			// 더미 클라이언트의 부하 때문에 쓰레드당 스택을 임시로 128로 한다.
			HANDLE thread = (HANDLE)_beginthreadex(NULL, 128, StartAddress<Thread<T>>, this, 0, NULL);
			_thread[i] = thread;
		}
	}

	bool Join()
	{
		DWORD result = WaitForMultipleObjects(_num, _thread, TRUE, INFINITE);
		_num = 0;
		ZeroMemory(_thread, sizeof(_thread));
		return result != WAIT_FAILED;
	}

	bool IsRun()
	{
		return WaitForMultipleObjects(_num, _thread, FALSE, 0) == WAIT_TIMEOUT;
	}
};

//////////////////////////////////////////////////////////////////////////

class ThreadContext
{
public:
	class Timer
	{
	public:
		Timer();
		unsigned int Tick();
	};

	class Random
	{
	public:
		Random(unsigned int seed);
	};

	ThreadContext();
};

//////////////////////////////////////////////////////////////////////////

template<class T>
unsigned WINAPI StartAddress(void* object)
{
	ThreadContext context;
	return static_cast<T*>(object)->Run();
}
