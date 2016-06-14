#pragma warning(push)
#pragma warning(disable:4996)

#include <cstdlib> 
#include <algorithm>
#include "Singleton.h"

//////////////////////////////////////////////////////////////////////////

namespace Private
{
	class Priority
	{
	public:
		int _priority;
		AtExit _callback;

		Priority(int priority, AtExit callback) : _priority(priority), _callback(callback) {}

		friend bool PriorityCompare(const Priority* lhs, const Priority* rhs)
		{
			return lhs->_priority < rhs->_priority;
		}
	};

	int size;
	Priority** q;

	void PopAtExit()
	{
		Priority* p = q[size - 1];
		--size;

		q = (Priority**)realloc(q, sizeof(Priority*) * size);

		(*p->_callback)();
		delete p;
	}

	void PushAtExit(int priority, AtExit callback)
	{
		Priority* p = new Priority(priority, callback);
		++size;

		q = (Priority**)realloc(q, sizeof(Priority*) * size);
		Priority** it = std::upper_bound(&q[0], &q[size - 1], p, PriorityCompare);
		std::copy_backward(it, &q[size - 1], &q[size]);
		*it = p;

		std::atexit(PopAtExit);
	};
}

//////////////////////////////////////////////////////////////////////////

#pragma warning(pop)
