#pragma once

#include <crtdbg.h>
#include <vector>
#include <algorithm>
#include "Lock.h"
#include "Singleton.h"

#pragma push_macro("new")

#ifdef new
#undef new
#endif

//////////////////////////////////////////////////////////////////////////

template <typename T, int N>
class MemoryPool
{
public:
	MemoryPool() : _memory(NULL), _used(0)
	{
#ifdef _DEBUG
		_memory = reinterpret_cast<char*>(::operator new(sizeof(T) * N, _NORMAL_BLOCK, __FILE__, __LINE__));
#else
		_memory = reinterpret_cast<char*>(::operator new(sizeof(T) * N));
#endif
		_deallocate.reserve(N);
	}

	~MemoryPool()
	{
		std::for_each(_allocate.begin(), _allocate.end(), Delete);
#ifdef _DEBUG
		::operator delete(_memory, _NORMAL_BLOCK, __FILE__, __LINE__);
#else
		::operator delete(_memory);
#endif
	}

	T* Allocate()
	{
		T* memory;

		if (_deallocate.empty())
		{
			if (_used < N)
			{
				memory = reinterpret_cast<T*>(_memory) + _used;
				++_used;
			}
			else
			{
				memory = New();
				_allocate.push_back(memory);
			}
		}
		else
		{
			memory = _deallocate.back();
			_deallocate.pop_back();
		}

		return memory;
	}

	void Deallocate(T* memory)
	{
		_deallocate.push_back(memory);
	}

protected:
	static T* New()
	{
#ifdef _DEBUG
		return reinterpret_cast<T*>(::operator new(sizeof(T), _NORMAL_BLOCK, __FILE__, __LINE__));
#else
		return reinterpret_cast<T*>(::operator new(sizeof(T)));
#endif
	}

	static void Delete(T* memory)
	{
#ifdef _DEBUG
		::operator delete(memory, _NORMAL_BLOCK, __FILE__, __LINE__);
#else
		::operator delete(memory);
#endif
	}

protected:
	std::vector<T*> _allocate;
	std::vector<T*> _deallocate;
	char* _memory;
	unsigned int _used;
};

//////////////////////////////////////////////////////////////////////////

template <class T, int N = 128>
class MemoryPoolObject : public StaticLockable<T>
{
protected:
	typedef Singleton<MemoryPool<T, N>> TheMemoryPool;

public:
	MemoryPoolObject() {}
	virtual ~MemoryPoolObject() {}

	static void* operator new(std::size_t size)
	{
		Lock lock(NULL);
		if (size != sizeof(T))
		{
			__asm int 3;
			return ::operator new(size);
		}
		return TheMemoryPool::Instance()->Allocate();
	}

	static void operator delete(void* memory, std::size_t size)
	{
		Lock lock(NULL);
		if (!memory)
			return;
		if (size != sizeof(T))
		{
			__asm int 3;
			::operator delete(memory);
			return;
		}
		TheMemoryPool::Instance()->Deallocate(reinterpret_cast<T*>(memory));
	}
};

//////////////////////////////////////////////////////////////////////////

#pragma pop_macro("new")
