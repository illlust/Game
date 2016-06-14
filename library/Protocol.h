#pragma once

#include <Mmsystem.h>
#include <string>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <set>
#include "Serialization.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////////

enum Port
{
	LOGIN_PORT = 9190,
};

enum Protocol
{
	C_HELLO_WORLD_REQ = 0,
	C_HELLO_WORLD_ACK,
	C_PING,
	S_CRYPT,
	C_LOGIN_REQ,
	C_LOGIN_ACK,
	C_LOGIN_NAK,
	S_REGISTER_CHANNEL,
	C_CHANNEL_LIST_REQ,
	C_CHANNEL_LIST_ACK,
	C_JOIN_CHANNEL_REQ,
	C_JOIN_CHANNEL_ACK,
	C_ARG0,
	C_ARG1,
	C_ARG2,
	C_ARG3,
	C_ARG0_PEER,
	C_ARG1_PEER,
	C_ARG2_PEER,
	C_ARG3_PEER,
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_HELLO_WORLD_REQ
{
	PACKET_C_HELLO_WORLD_REQ() : _message("Hello server~~~") {}
	std::string _message;
	SERIAL_PACKET(C_HELLO_WORLD_REQ, _message);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_HELLO_WORLD_ACK
{
	PACKET_C_HELLO_WORLD_ACK() : _message("Hello client~~~") {}
	std::string _message;
	SERIAL_PACKET(C_HELLO_WORLD_ACK, _message);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_PING
{
	SERIAL_EVENT(C_PING);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_S_CRYPT
{
	PACKET_S_CRYPT() : _key(0) {}
	void Crypt()
	{
		_key = (rand() % (std::numeric_limits<char>::max)()) + 1; // 1~127
	}

	char _key;
	SERIAL_PACKET(S_CRYPT, _key);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_LOGIN_REQ
{
	std::string _id;
	std::string _pw;
	PACKET_C_LOGIN_REQ(std::string id = "", std::string pw = "") : _id(id), _pw(pw) {}
	SERIAL_PACKET(C_LOGIN_REQ, _id & _pw);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_LOGIN_ACK
{
	SERIAL_EVENT(C_LOGIN_ACK);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_LOGIN_NAK
{
	SERIAL_EVENT(C_LOGIN_NAK);
};

//////////////////////////////////////////////////////////////////////////

struct Channel
{
	ULONG ip;
	USHORT port;

	operator std::string()
	{
		in_addr addr = { 0, };
		addr.s_addr = ip;
		std::ostringstream stream;

		stream << inet_ntoa(addr) << ":" << port;
		return static_cast<std::string>(stream.str());
	}

	bool operator <(const Channel& rhs) const
	{
		if (ip < rhs.ip)
			return true;
		if (ip > rhs.ip)
			return false;
		if (port < rhs.port)
			return true;
		if (port > rhs.port)
			return false;
		return false;
	}
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_S_REGISTER_CHANNEL
{
	Channel _ch;
	PACKET_S_REGISTER_CHANNEL() {}
	PACKET_S_REGISTER_CHANNEL(Channel ch) : _ch(ch) {}
	SERIAL_PACKET(S_REGISTER_CHANNEL, _ch);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_CHANNEL_LIST_REQ
{
	SERIAL_EVENT(C_CHANNEL_LIST_REQ);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_CHANNEL_LIST_ACK
{
	typedef std::set<Channel> ChannelList;
	ChannelList _ch;
	void AddChannel(std::string ip, USHORT port)
	{
		Channel ch;
		ch.ip = inet_addr(ip.c_str());
		ch.port = port;
		_ch.insert(ch);
	}
	SERIAL_PACKET(C_CHANNEL_LIST_ACK, _ch);
};

//////////////////////////////////////////////////////////////////////////

struct PACKET_C_JOIN_CHANNEL_REQ
{
	SERIAL_EVENT(C_JOIN_CHANNEL_REQ);
};
