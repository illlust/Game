#pragma once

#include <unordered_map>
#include <memory>
#include <utility>
#include <type_traits>
#include <Windows.h>
#include "Serialization.h"
#include "MemoryPool.h"
#include "Log.h"
#include "Lock.h"

//////////////////////////////////////////////////////////////////////////

template<class Event = int, class Peer = Null, class Result = void>
class Dispatcher : public Lockable<Dispatcher<Event, Peer, Result>>
{
public:

	class Null {};

	template<class Caller, class Result>
	class AbstractCommand
	{
	public:
		typedef Caller Caller;

		AbstractCommand() {}
		virtual ~AbstractCommand() = 0 {} // 추상 클래스

		virtual Result operator()(char*, int)
		{
			THROW_EXCEPTION();
		}

		virtual Result operator()(char*, int, Caller)
		{
			THROW_EXCEPTION();
		}
	};

	// 특화시킬 최대 인자 갯수
	template<class = void, class = void, class = void, class = void, class = void, class = void, class = void, class = void, class = void, class = void>
	class Command {};

	// C_ARG0
	template<class Callee>
	class Command<Result (Callee::*)()> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)();
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char*, int)
		{
			return (_callee->*_callback)();
		}
	};

	// C_ARG1
	template<class Callee, class Argument1>
	class Command<Result (Callee::*)(Argument1)> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			Archive<> sr(buf, len);
			sr >> arg1;
			return (_callee->*_callback)(arg1);
		}
	};

	// C_ARG2
	template<class Callee, class Argument1, class Argument2>
	class Command<Result (Callee::*)(Argument1, Argument2)> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1, Argument2);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			std::tr1::remove_reference<Argument2>::type arg2;
			Archive<> sr(buf, len);
			sr >> arg1 >> arg2;
			return (_callee->*_callback)(arg1, arg2);
		}
	};

	// C_ARG3
	template<class Callee, class Argument1, class Argument2, class Argument3>
	class Command<Result (Callee::*)(Argument1, Argument2, Argument3)> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1, Argument2, Argument3);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			std::tr1::remove_reference<Argument2>::type arg2;
			std::tr1::remove_reference<Argument3>::type arg3;
			Archive<> sr(buf, len);
			sr >> arg1 >> arg2 >> arg3;
			return (_callee->*_callback)(arg1, arg2, arg3);
		}
	};

	// C_ARG0_PEER
	template<class Callee>
	class Command<Result (Callee::*)(Peer), Peer> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Caller);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char*, int, Caller caller)
		{
			return (_callee->*_callback)(caller);
		}
	};

	// C_ARG1_PEER
	template<class Callee, class Argument1>
	class Command<Result (Callee::*)(Argument1, Peer), Peer> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1, Caller);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len, Caller caller)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			Archive<> sr(buf, len);
			sr >> arg1;
			return (_callee->*_callback)(arg1, caller);
		}
	};

	// C_ARG2_PEER
	template<class Callee, class Argument1, class Argument2>
	class Command<Result (Callee::*)(Argument1, Argument2, Peer), Peer> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1, Argument2, Caller);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len, Caller caller)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			std::tr1::remove_reference<Argument2>::type arg2;
			Archive<> sr(buf, len);
			sr >> arg1 >> arg2;
			return (_callee->*_callback)(arg1, arg2, caller);
		}
	};

	// C_ARG3_PEER
	template<class Callee, class Argument1, class Argument2, class Argument3>
	class Command<Result (Callee::*)(Argument1, Argument2, Argument3, Peer), Peer> : public AbstractCommand<Peer, Result>
	{
	public:
		typedef Result (Callee::*Callback)(Argument1, Argument2, Argument3, Caller);
		Callback _callback;
		Callee* _callee;

		Command(Callback callback, Callee* callee) : _callback(callback), _callee(callee) {}

		virtual Result operator()(char* buf, int len, Caller caller)
		{
			std::tr1::remove_reference<Argument1>::type arg1;
			std::tr1::remove_reference<Argument2>::type arg2;
			std::tr1::remove_reference<Argument3>::type arg3;
			Archive<> sr(buf, len);
			sr >> arg1 >> arg2 >> arg3;
			return (_callee->*_callback)(arg1, arg2, arg3, caller);
		}
	};

public:
	typedef std::tr1::shared_ptr<AbstractCommand<Peer, Result>> CommandPtr;
	typedef std::tr1::unordered_map<Event, CommandPtr> Invoker;

	Invoker _invoker;

	void Register(Event event, CommandPtr action)
	{
		Lock lock(this);
		_invoker.insert(std::make_pair(event, action));
	}

	Result Invoke(Event event, char* buf, int len)
	{
		Lock lock(this);

		Invoker::iterator it = _invoker.find(event);
		if (it == _invoker.end())
		{
			*TheLog::Instance() << "등록되지 않은 이벤트(" << event << ")를 디스패치하려고 했다." << endl;
			THROW_EXCEPTION();
		}

		CommandPtr command = it->second;
		return (*command)(buf, len);
	}

	Result Invoke(Event event, char* buf, int len, Peer peer)
	{
		Lock lock(this);

		Invoker::iterator it = _invoker.find(event);
		if (it == _invoker.end())
		{
			*TheLog::Instance() << "등록되지 않은 이벤트(" << event << ")를 디스패치하려고 했다." << endl;
			THROW_EXCEPTION();
		}

		CommandPtr command = it->second;
		return (*command)(buf, len, peer);
	}

	template<class Callee>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(), Callee* callee)
	{
		typedef Result (Callee::*Callback)();
		CommandPtr action(new Command<Callback>(callback, callee));
		dispatcher->Register(event, action);
	};

	template<class Callee, class Argument1>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1);
		CommandPtr action(new Command<Callback>(callback, callee));
		dispatcher->Register(event, action);
	};

	template<class Callee, class Argument1, class Argument2>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1, Argument2), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1, Argument2);
		CommandPtr action(new Command<Callback>(callback, callee));
		dispatcher->Register(event, action);
	};

	template<class Callee, class Argument1, class Argument2, class Argument3>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1, Argument2, Argument3), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1, Argument2, Argument3);
		CommandPtr action(new Command<Callback>(callback, callee));
		dispatcher->Register(event, action);
	};

	template<class Callee>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Peer), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Peer);
		CommandPtr action(new Command<Callback, Peer>(callback, callee));
		dispatcher->Register(event, action);
	};

	template<class Callee, class Argument1>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1, Peer), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1, Peer);
		CommandPtr command(new Command<Callback, Peer>(callback, callee));
		dispatcher->Register(event, command);
	};

	template<class Callee, class Argument1, class Argument2>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1, Argument2, Peer), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1, Argument2, Peer);
		CommandPtr command(new Command<Callback, Peer>(callback, callee));
		dispatcher->Register(event, command);
	};

	template<class Callee, class Argument1, class Argument2, class Argument3>
	friend void RegisterCallback(Dispatcher* dispatcher, Event event, Result (Callee::*callback)(Argument1, Argument2, Argument3, Peer), Callee* callee)
	{
		typedef Result (Callee::*Callback)(Argument1, Argument2, Argument3, Peer);
		CommandPtr command(new Command<Callback, Peer>(callback, callee));
		dispatcher->Register(event, command);
	};

	template<class Callee, class Packet>
	friend void RegisterPacket(Dispatcher* dispatcher, Result (Callee::*callback)(Packet, Peer), Callee* callee)
	{
		Event event = std::tr1::remove_reference<Packet>::type::EVENT;
		typedef Result (Callee::*Callback)(Packet, Peer);
		CommandPtr command(new Command<Callback, Peer>(callback, callee));
		dispatcher->Register(event, command);
	};
};

//////////////////////////////////////////////////////////////////////////

#define REGISTER_PACKET(dispatcher, callback) RegisterPacket(dispatcher, callback, this)

//////////////////////////////////////////////////////////////////////////

class Session;
typedef Dispatcher<int, Session&, bool> ServerDispatcher;

class Reactor;
typedef Dispatcher<int, Reactor&, void> ClientDispatcher;
