#include <sstream>
#include <cstdlib>
#include "Log.h"

//////////////////////////////////////////////////////////////////////////

int Log::_reference;

//////////////////////////////////////////////////////////////////////////

Log::Log(int output, std::string path, bool append) :
_logoutput(false), _logfile(false), _logconsole(false), _logtime(false),
_file(NULL), _console(NULL),_level(LEVEL_DEFAULT)
{
	AddOutput(output, path, append);
}

Log::~Log()
{
	CloseFile();
	CloseConsole();
}

void Log::AddOutput(int output, std::string path, bool append)
{
	Lock lock(this);

	if (output & LOG_OUTPUT)
	{
		_logoutput = true;
	}

	if (output & LOG_FILE)
	{
		OpenFile(path, append);
	}

	if (output & LOG_CONSOLE)
	{
		OpenConsole();
	}

	if (output & LOG_TIME)
	{
		_logtime = true;
	}
}

void Log::RemoveOutput(int output)
{
	Lock lock(this);

	if (output & LOG_OUTPUT)
	{
		_logoutput = false;
	}

	if (output & LOG_FILE)
	{
		CloseFile();
	}

	if (output & LOG_CONSOLE)
	{
		CloseConsole();
	}

	if (output & LOG_TIME)
	{
		_logtime = false;
	}
}

void Log::Print(const char* format, ...)
{
	va_list valist;
	va_start(valist, format);
	DoPrint(LEVEL_DEFAULT, format, valist);
	va_end(valist);
}

void Log::Print(Level level, const char* format, ...)
{
	if (level < _level)
		return;
	
	va_list valist;
	va_start(valist, format);
	DoPrint(level, format, valist);
	va_end(valist);
}

Log& Log::DoPrint(Level level, const char* format, va_list valist)
{
	Lock lock(this);

	if (!format)
		return *this;

	if (valist)
	{
		char logbuf[1024];
		_vsnprintf_s(logbuf, sizeof(logbuf), format, valist);
		strcat_s(logbuf, "\r\n");

		if (_logtime)
		{
			SYSTEMTIME time;
			GetLocalTime(&time);
			char timebuf[32];
			sprintf_s(timebuf, "[%04d/%02d/%02d %02d:%02d:%02d] ",
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

			char levelbuf[16];
			switch (level)
			{
			case LEVEL_DEFAULT:
				ZeroMemory(levelbuf, sizeof(levelbuf));
				break;
			case LEVEL_WARNING:
				strcpy_s(levelbuf, "< WARNING!! >  ");
				break;
			case LEVEL_ERROR:
				strcpy_s(levelbuf, "<  ERROR!!  >  ");
				break;
			}

			char linebuf[1024 + 32 + 16];
			sprintf_s(linebuf, "%s%s%s", timebuf, levelbuf, logbuf);

			DoPrintLine(linebuf);
		}
		else
		{
			DoPrintLine(logbuf);
		}
	}
	else
	{
		DoPrintLine(format);
	}

	return *this;
}

void Log::DoPrintLine(const char* line) 
{
	if (_logoutput)
	{
		OutputDebugStringA(line);
	}

	if (_logfile && _file)
	{
		DWORD written;
		WriteFile(_file, line, strlen(line), &written, NULL);
	}

	if (_logconsole && _console)
	{
		DWORD written;
		WriteConsoleA(_console, line, strlen(line), &written, NULL); 
	}
}

Log::operator bool()
{
	Lock lock(this);
	return _logfile && _file;
}

void Log::OpenFile(std::string path, bool append)
{
	Lock lock(this);

	if (!append)
	{
		std::string bak = path + ".bak";
		MoveFileExA(path.c_str(), bak.c_str(), MOVEFILE_REPLACE_EXISTING);
	}

	CloseFile();

	_file = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE)
	{
		_logfile = false;
		return;
	}

	SetFilePointer(_file, 0, NULL, FILE_END);

	_logfile = true;
}

void Log::CloseFile()
{
	Lock lock(this);

	if (_file)
	{
		_logfile = false;

		CloseHandle(_file);
		_file = NULL;
	}
}

void Log::OpenConsole()
{
	Lock lock(this);

	_logconsole = true;

	if (!_console)
	{
		if (_reference++ == 0)
		{
#ifndef _CONSOLE
			if (!AllocConsole())
				LOG_LAST_ERROR(GetLastError());
#endif // _CONSOLE
			_console = GetStdHandle(STD_OUTPUT_HANDLE);
		}
	}
}

void Log::CloseConsole()
{
	Lock lock(this);

	_logconsole = false;

	if (_console)
	{
		if (--_reference == 0)
		{
#ifndef _CONSOLE
			FreeConsole();
#endif // _CONSOLE
		}

		_console = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

std::string FormatLastError(DWORD error)
{
	std::string result;

	HLOCAL buf = NULL;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL,
		error, MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), (char*)&buf, 0, NULL);

	if (buf)
	{
		std::ostringstream stream;
		stream << "[" << error << "] " << (char*)buf;
		result = stream.str();
		LocalFree(buf);
	}

	return result;
}
