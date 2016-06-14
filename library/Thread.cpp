#include "Thread.h"

#pragma comment(lib, "winmm.lib")

//////////////////////////////////////////////////////////////////////////

ThreadContext::ThreadContext()
{
	Timer timer;
	Random random(timer.Tick());
}

//////////////////////////////////////////////////////////////////////////

ThreadContext::Timer::Timer()
{
	TIMECAPS cap;
	timeGetDevCaps(&cap, sizeof(cap));
	UINT resolution = (std::max)(cap.wPeriodMin, (UINT)1); 
	timeBeginPeriod(resolution);
}

unsigned int ThreadContext::Timer::Tick()
{
	return timeGetTime();
}

//////////////////////////////////////////////////////////////////////////

ThreadContext::Random::Random(unsigned int seed)
{
	//printf("[seed=%u][thread=%d]\n", seed, GetCurrentThreadId());
	srand(seed);
}
