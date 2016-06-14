#pragma once

#include <Windows.h>
#include <DbgHelp.h>

//////////////////////////////////////////////////////////////////////////

class Dump
{
private:
	PTOP_LEVEL_EXCEPTION_FILTER _filter;
	static LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* info);

public:
	Dump();
	~Dump();
};
