//#include <exception>
#include <WinSock2.h>
#include "Overlapped.h"
#include "Proactor.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////////

Proactor::Proactor()
{
	_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (_iocp == INVALID_HANDLE_VALUE)
		THROW_EXCEPTION("포트 만들기가 실패하였다");
}

Proactor::~Proactor()
{
	CloseHandle(_iocp);
}

int Proactor::Run()
{
	volatile bool run = true;
	while (run)
	{
		DWORD transferred;
		ULONG_PTR key;
		OVERLAPPED* overlapped;

		if (GetQueuedCompletionStatus(_iocp, &transferred, &key, (OVERLAPPED**)&overlapped, INFINITE))
		{
			if (overlapped)
			{
				static_cast<Overlapped*>(overlapped)->Complete(transferred);
			}
			else
			{
				LOG("프로액터 쓰레드를 종료한다.");
				run = false;
			}
		}
		else
		{
			//printf("[%s][tranferred=%d][error=%d][overlapped=%p][GQCS() == FALSE]\n", __FUNCTION__, transferred, GetLastError(), overlapped);

			// 1225=ERROR_CONNECTION_REFUSED // Link
			// 64=ERROR_NETNAME_DELETED // Receipt
			// 995=ERROR_OPERATION_ABORTED // Receipt

			if (overlapped)
			{
				static_cast<Overlapped*>(overlapped)->Error();
			}
			else
			{
				THROW_EXCEPTION("IOCP 에서 불러오기에 실패하였다.");
			}
		}
	}

	return 0;
}

bool Proactor::Register(SOCKET sock)
{
	if (!_association.insert(sock).second)
	{
		//LOG_ERROR("이미 등록된 소켓이다.");
		return false;
	}

	if (!CreateIoCompletionPort((HANDLE)sock, _iocp, NULL, 0))
		THROW_EXCEPTION("포트에 등록을 실패하였다.");

	return true;
}

bool Proactor::PostStatus(OVERLAPPED* overlapped)
{
	return PostQueuedCompletionStatus(_iocp, 0, NULL, overlapped) != FALSE;
}

void Proactor::PostQuitStatus()
{
//	printf("[%s]\n", __FUNCTION__);
//	PostStatus(NULL);
	for (int i = 0; i < _num; ++i)
		PostStatus(NULL);
}

int Proactor::Cpu()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	//printf("[%s][cpu=%d]\n", __FUNCTION__, info.dwNumberOfProcessors * 2 + 2);
	return info.dwNumberOfProcessors * 2 + 2;
}
