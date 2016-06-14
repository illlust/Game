#include "MemoryLeak.h"

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////

MemoryLeak theMemoryLeak;

//////////////////////////////////////////////////////////////////////////

MemoryLeak::MemoryLeak() : _state(NULL), _file(INVALID_HANDLE_VALUE),
_flags(0), _warnmode(0), _errormode(0), _assertmode(0),
_warn(_CRTDBG_INVALID_HFILE), _error(_CRTDBG_INVALID_HFILE), _assert(_CRTDBG_INVALID_HFILE)
{
	_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);

	_warnmode = _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_errormode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
	_assertmode = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
	_warn = _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_error = _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_assert = _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

	_state = new _CrtMemState;
	_CrtMemCheckpoint(_state);
}

MemoryLeak::~MemoryLeak()
{
	_CrtMemState state;
	_CrtMemCheckpoint(&state);

	_CrtMemState diff;
	if (_CrtMemDifference(&diff, _state, &state))
	{
		__asm int 3;
		_CrtMemDumpStatistics(&diff);
		_CrtMemDumpAllObjectsSince(_state);
	}

	delete _state;

	if (_file != INVALID_HANDLE_VALUE)
		CloseHandle(_file);

	_CrtSetDbgFlag(_flags);

	_CrtSetReportMode(_CRT_WARN, _warnmode);
	_CrtSetReportMode(_CRT_ERROR, _errormode);
	_CrtSetReportMode(_CRT_ASSERT, _assertmode);
	_CrtSetReportFile(_CRT_WARN, _warn);
	_CrtSetReportFile(_CRT_ERROR, _error);
	_CrtSetReportFile(_CRT_ASSERT, _assert);
}

void MemoryLeak::OpenReportFile(std::string path, bool append)
{
	if (_file != INVALID_HANDLE_VALUE)
		return;

	_file = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(_file, 0, NULL, FILE_END);

	_CrtSetReportFile(_CRT_WARN, _file);
	_CrtSetReportFile(_CRT_ERROR, _file);
	_CrtSetReportFile(_CRT_ASSERT, _file);
}

//void MemoryLeak::SetReportFile(HANDLE file)
//{
//	if (_file != INVALID_HANDLE_VALUE)
//	{
//		CloseHandle(_file);
//		_file = INVALID_HANDLE_VALUE;
//		// 파일을 닫을 책임은 핸들 제공자에게 있으므로 핸들을 저장하지 않는다.
//	}
//
//	_CrtSetReportFile(_CRT_WARN, file);
//	_CrtSetReportFile(_CRT_ERROR, file);
//	_CrtSetReportFile(_CRT_ASSERT, file);
//}

void MemoryLeak::Break(long block)
{
	_CrtSetBreakAlloc(block);
	_crtBreakAlloc = block;
}

//////////////////////////////////////////////////////////////////////////

#endif // _DEBUG
