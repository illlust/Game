#pragma once

#include <Windows.h>
#include <cstdio>
#include "Uncopyable.h"

//////////////////////////////////////////////////////////////////////////

class CriticalSection : private Uncopyable
{
public:
	CRITICAL_SECTION _cs;
	CriticalSection();
	~CriticalSection();
	void Lock();
	void Unlock();
};

//////////////////////////////////////////////////////////////////////////

class Lock
{
public:
	CriticalSection* _cs;

	Lock(CriticalSection* cs) : _cs(cs)
	{
		_cs->Lock();
	}
	~Lock()
	{
		_cs->Unlock();
	}
};

//////////////////////////////////////////////////////////////////////////

template<class T>
class Unlockable
{
	friend class Lock;

public:
	Unlockable() {}
	virtual ~Unlockable() {}

	class Lock
	{
	public:
		Lock(T*) {}
		~Lock() {}
	};

	T* This()
	{
		return this;
	}
};

//////////////////////////////////////////////////////////////////////////

template<class T>
class StaticLockable
{
	friend class Lock;

private:
	static CriticalSection _static;

public:
	StaticLockable() {}
	virtual ~StaticLockable() {}

	class Lock
	{
	public:
		Lock(T*)
		{
			StaticLockable<T>::_static.Lock();
		}
		~Lock()
		{
			StaticLockable<T>::_static.Unlock();
		}
	};

	T* This()
	{
		return NULL;
	}
};

template<class T>
CriticalSection StaticLockable<T>::_static;

//////////////////////////////////////////////////////////////////////////

template<class T>
class Lockable
{
	friend class Lock;

private:
	CriticalSection _cs;

public:
	Lockable() {}
	virtual ~Lockable() {}

	class Lock
	{
	private:
		T* _object;

	public:
		Lock(T* object) : _object(object)
		{
			_object->_cs.Lock();
		}
		~Lock()
		{
			_object->_cs.Unlock();
		}
	};

	T* This()
	{
		return this;
	}
};
