#pragma once

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <string>
#include <Windows.h>

#ifdef new
#undef new
#pragma message("MemoryLeak undef new")
#endif
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)

#ifdef malloc
#undef malloc
#pragma message("MemoryLeak undef malloc")
#endif
#define malloc(s) (_malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__))

//////////////////////////////////////////////////////////////////////////

class MemoryLeak
{
public:
	_CrtMemState* _state;
	HANDLE _file;
	int _flags;
	int _warnmode;
	int _errormode;
	int _assertmode;
	_HFILE _warn;
	_HFILE _error;
	_HFILE _assert;

	MemoryLeak();
	~MemoryLeak();

	void OpenReportFile(std::string path, bool append = false);
	//void SetReportFile(HANDLE file);
	void Break(long block);
};

//////////////////////////////////////////////////////////////////////////

extern MemoryLeak theMemoryLeak; // 정적 메모리까지 추적해야 하므로 싱글턴이 아닌 전역 객체이어야 한다.

//////////////////////////////////////////////////////////////////////////

#endif // _DEBUG
