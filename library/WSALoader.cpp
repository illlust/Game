#include <WinSock2.h>
//#include <exception>
#include "WSALoader.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

//////////////////////////////////////////////////////////////////////////

WSALoader::WSALoader()
{
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
}

WSALoader::~WSALoader()
{
	WSACleanup();
}
