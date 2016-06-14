#include <Windows.h>
#include "Packet.h"
#include "Crypt.h"

//////////////////////////////////////////////////////////////////////////

char Crypt::Encrypt(char crypt, char* destination, const char* source, int len)
{
	Lock lock(NULL);

	char bcc = 0; // Block Check Character

	for (int i = 0; i < len; ++i)
	{
		bcc += source[i];
		destination[i] = source[i] ^ crypt;
		//destination[i] = source[i];
	}

	return bcc;
}

char Crypt::Decrypt(char crypt, char* destination, const char* source, int len)
{
	Lock lock(NULL);

	char bcc = 0;

	for (int i = 0; i < len; ++i)
	{
		//destination[i] = source[i];
		destination[i] = source[i] ^ crypt;
		bcc += destination[i];
	}

	return bcc;
}

char Crypt::Encrypt(char crypt, Packet* packet)
{
	return Encrypt(crypt, reinterpret_cast<char*>(packet->event), reinterpret_cast<char*>(packet->event), packet->header - FOOTERSIZE);
}

char Crypt::Decrypt(char crypt, Packet* packet)
{
	return Encrypt(crypt, reinterpret_cast<char*>(packet->event), reinterpret_cast<char*>(packet->event), packet->header - FOOTERSIZE);
}
