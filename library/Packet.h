#pragma once

const int HEADERSIZE = sizeof(short);
const int EVENTSIZE = sizeof(int);
const int FOOTERSIZE = sizeof(char);

//#pragma pack(push, 1)

class Packet
{
public:
	short header;
	int* event;
	char* buf;
	int len;
	char bcc;
};

//#pragma pack(pop)
