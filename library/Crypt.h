#pragma once

#include "Lock.h"

//////////////////////////////////////////////////////////////////////////

class Packet;

class Crypt : StaticLockable<Crypt>
{
public:
	char Encrypt(char crypt, char* destination, const char* source, int len);
	char Decrypt(char crypt, char* destination, const char* source, int len);
	char Encrypt(char crypt, Packet* packet);
	char Decrypt(char crypt, Packet* packet);
};
