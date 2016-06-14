#include <Windows.h>
#include "Lock.h"

//////////////////////////////////////////////////////////////////////////

CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&_cs);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&_cs);
}

void CriticalSection::Lock()
{
	EnterCriticalSection(&_cs);
}

void CriticalSection::Unlock()
{
	LeaveCriticalSection(&_cs);
}
