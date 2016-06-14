#pragma once

#include <Windows.h>
#include "Lock.h"

//////////////////////////////////////////////////////////////////////////

namespace Private
{
	typedef void (*AtExit)();
	void PushAtExit(int priority, AtExit callback);
}

//////////////////////////////////////////////////////////////////////////

template<class T, int PRIORITY = 0, template<class> class SingletonLock = StaticLockable>
class Singleton : public T, public SingletonLock<Singleton<T>>
{
	using SingletonLock<Singleton<T>>::Lock;

private:
	static bool _destroy;
	static Singleton* _instance;

	virtual ~Singleton()
	{
		_destroy = true;
		_instance = NULL;
	}

	static void Destroy()
	{
		delete _instance;
		_instance = NULL;
	}

public:
	static T* Instance()
	{
		if (!_instance)
		{
			Lock lock(NULL);
			if (!_instance)
			{
				if (_destroy)
				{
					RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
					return NULL;
				}

				_instance = new Singleton;
				Private::PushAtExit(PRIORITY, Destroy);
			}
		}

		return _instance;
	}
};

template<class T, int PRIORITY, template<class> class SingletonLock>
Singleton<T, PRIORITY, SingletonLock>* Singleton<T, PRIORITY, SingletonLock>::_instance;

template<class T, int PRIORITY, template<class> class SingletonLock>
bool Singleton<T, PRIORITY, SingletonLock>::_destroy;
