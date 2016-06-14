#pragma once

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Lock.h"
#include "Singleton.h"

//////////////////////////////////////////////////////////////////////////

template<bool> class Assert;
template<> class Assert<true> {};

//////////////////////////////////////////////////////////////////////////

class Log : public Lockable<Log>
{
public:
	enum Level { LEVEL_DEFAULT = 0, LEVEL_WARNING, LEVEL_ERROR };
	enum Output
	{
		LOG_ALL     = 0xFFFFFFFF,
		LOG_OUTPUT  = 0x00000001,
		LOG_FILE    = 0x00000002,
		LOG_CONSOLE = 0x00000004,
		LOG_TIME    = 0x00000008,
	};

private:
	Log& DoPrint(Level level, const char* format, va_list valist = NULL);
	void DoPrintLine(const char* line);
	void OpenFile(std::string path, bool append = false);
	void CloseFile();
	void OpenConsole();
	void CloseConsole();

public:
	Log(int output = ~LOG_FILE, std::string path = "", bool append = false);
	virtual ~Log();

	operator bool();

	void AddOutput(int output, std::string path = "", bool append = false);
	void RemoveOutput(int output);
	void Print(const char* format, ...);
	void Print(Level level, const char* format, ...);
	void Print(Level, ...) {}

	template<class T>
	Log& operator <<(const T& object)
	{
		std::ostringstream stream;
		stream << object;
		return DoPrint(LEVEL_DEFAULT, stream.str().c_str());
	}

public:
	typedef Log& (*Function)(Log&);

	Log& operator <<(Function f)
	{
		Lock lock(this);
		return f(*this);
	}

	friend Log& endl(Log& log)
	{
		return log << "\r\n";
	}

public:
	HANDLE _file;
	HANDLE _console;

private:
	bool _logoutput;
	bool _logfile;
	bool _logconsole;
	bool _logtime;
	std::string _path;
	Level _level;
	static int _reference;
};

//////////////////////////////////////////////////////////////////////////

typedef Singleton<Log> TheLog;

#define LOG TheLog::Instance()->Print
#define ASC(line) #line
#define ITOA(line) ASC(line)
#define __FILELINE__ __FILE__"("ITOA(__LINE__)") : "
#define TODO(todo) message(__FILELINE__##todo)

//////////////////////////////////////////////////////////////////////////

std::string FormatLastError(DWORD error);

inline void BreakPoint()
{
	__asm int 3;
}

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

#define LOG_DEBUG LOG

#define LOG_WARNING(...)                                         \
{                                                                \
	Log::Lock warninglock(TheLog::Instance());                   \
	*TheLog::Instance() << __FILELINE__ << __FUNCTION__ << endl; \
	LOG(Log::LEVEL_WARNING, __VA_ARGS__);                        \
}

#define LOG_ERROR(...)                                           \
{                                                                \
	Log::Lock errorlock(TheLog::Instance());                     \
	*TheLog::Instance() << __FILELINE__ << __FUNCTION__ << endl; \
	LOG(Log::LEVEL_ERROR, __VA_ARGS__);                          \
}

#define LOG_LAST_ERROR(...)                                      \
{                                                                \
	Log::Lock lasterrorlock(TheLog::Instance());                 \
	*TheLog::Instance() << __FILELINE__ << __FUNCTION__ << endl; \
	*TheLog::Instance() << FormatLastError(GetLastError());      \
	LOG(Log::LEVEL_ERROR, __VA_ARGS__);                          \
}

#define THROW_EXCEPTION(...)                                     \
{                                                                \
	Log::Lock exceptionlock(TheLog::Instance());                 \
	*TheLog::Instance() << __FILELINE__ << __FUNCTION__ << endl; \
	*TheLog::Instance() << FormatLastError(GetLastError());      \
	LOG(Log::LEVEL_ERROR, __VA_ARGS__);                          \
	BreakPoint();                                                \
	RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);            \
	throw;                                                       \
}

#define STATIC_ASSERT(...) {}

#else

#define LOG_DEBUG true ? __noop : __noop
#define LOG_WARNING(...) __noop;
#define LOG_ERROR(...) __noop;
#define LOG_LAST_ERROR(...) __noop;
#define THROW_EXCEPTION(...) { RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL); throw; }
#define STATIC_ASSERT(...) {}

#endif // _DEBUG
