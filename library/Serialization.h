#pragma once

#include <Windows.h>
#include <type_traits>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <utility>

//////////////////////////////////////////////////////////////////////////

class AbstractArchive
{
protected:
	template<bool> class Assert;
	template<> class Assert<true> {};

protected:
	const char* _head;
	const char* _tail;
	char* _index;

protected:
	~AbstractArchive() {}

public:
	AbstractArchive(const char* head, const char* tail, char* index) : _head(head), _tail(tail), _index(index) {}

	char* Peek()
	{
		return const_cast<char*>(_head);
	}

	std::size_t Size() const
	{
		return _index - _head;
	}

	std::size_t Remain() const
	{
		return _tail - _index;
	}

	std::size_t Capacity() const
	{
		return _tail - _head;
	}
};

//////////////////////////////////////////////////////////////////////////

template<std::size_t N = 0>
class Archive : public AbstractArchive
{
private:
	char _buf[N];

public:
	Archive() : AbstractArchive(_buf, _buf + N, _buf) {}

private:
	bool Overflow(std::size_t size) const
	{
		//if (Remain() < size) std::cerr << __FUNCTION__ << std::endl;
		return Remain() < size;
	}

	template<class T>
	void Copy(const T* buf, std::size_t size)
	{
		if (Overflow(size))
			return;

		CopyMemory(_index, buf, size);
		_index += size;
	}

	// POD 포인터형
	template<class T>
	void Store(const T& value, std::tr1::true_type, std::tr1::true_type)
	{
		using std::tr1::remove_pointer;
		typedef remove_pointer<T>::type type;

		using std::tr1::is_same;
		using std::tr1::remove_const;
		// 문자열 포인터는 저장할 수 없으므로 컴파일 에러로 처리한다.
		Assert<!is_same<char, remove_const<type>::type>::value>();
		// 타입 크기는 버퍼보다 작아야 한다.
		Assert<sizeof(type) < N>();

		bool null = (value == NULL);
		Copy(&null, sizeof(null));

		if (null)
			return;

		Copy(value, sizeof(type));
	}

	// POD 형
	template<class T>
	void Store(const T& value, std::tr1::true_type, std::tr1::false_type)
	{
		Assert<sizeof(value) < N>();
		Copy(&value, sizeof(value));
	}

	// POD 아닌 포인터형
	template<class T>
	void Store(const T& value, std::tr1::false_type, std::tr1::true_type)
	{
		bool null = (value == NULL);
		Copy(&null, sizeof(null));

		if (null)
			return;

		value->Serialize(*this);
	}

	// POD 아닌 형
	template<class T>
	void Store(const T& value, std::tr1::false_type, std::tr1::false_type)
	{
		value.Serialize(*this);
	}

public:
	template<class T>
	Archive& operator &(const T& value)
	{
		return operator <<(value);
	}

	// 실제로 쓸 함수에 분배해준다.
	template<class T>
	Archive& operator <<(const T& value)
	{
		using std::tr1::is_pod;
		using std::tr1::is_pointer;
		using std::tr1::remove_pointer;
		typedef remove_pointer<T>::type type;
		Store(value, is_pod<type>(), is_pointer<T>());
		return *this;
	}

	Archive& operator <<(const std::string& value)
	{
		*this << value.size();
		Copy(value.c_str(), value.size());
		return *this;
	}

	// map을 제외한 컨테이너 쓰기
	template<class Container>
	void Store(const Container& c)
	{
		*this << c.size();

		using std::for_each;
		using std::tr1::bind;
		using std::tr1::mem_fn;
		using std::tr1::placeholders::_1;
		typedef typename Container::value_type value_type;
		for_each(c.begin(), c.end(), bind(mem_fn(&Archive::operator &<value_type>), this, _1));
	}

	template<class T>
	Archive& operator <<(const std::vector<T>& container)
	{
		Store(container);
		return *this;
	}

	template<class T>
	Archive& operator <<(const std::list<T>& container)
	{
		Store(container);
		return *this;
	}

	template<class T>
	Archive& operator <<(const std::set<T>& container)
	{
		Store(container);
		return *this;
	}

	template<class K, class V>
	Archive& operator <<(const std::map<K, V>& container)
	{
		*this << container.size();

		typedef typename std::map<K, V>::const_iterator const_iterator;
		for (const_iterator it = container.begin(); it != container.end(); ++it)
		{
			*this << it->first << it->second;
		}

		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////

template<>
class Archive<0> : public AbstractArchive
{
public:
	Archive(char* buf, std::size_t size) : AbstractArchive(buf, buf + size, buf) {}

private:
	bool Underflow(std::size_t size)
	{
		return Capacity() < Size() + size;
	}

	template<class T>
	void Copy(T* buf, std::size_t size)
	{
		if (Underflow(size))
			return;

		CopyMemory(buf, _index, size);
		_index += size;
	}

	// POD 포인터형
	template<class T>
	void Load(T& value, std::tr1::true_type, std::tr1::true_type)
	{
		using std::tr1::remove_pointer;
		typedef remove_pointer<T>::type type;

		// 문자열 포인터는 불러올 수 없으므로 컴파일 에러로 처리한다.
		using std::tr1::is_same;
		using std::tr1::remove_const;
		Assert<!is_same<char, remove_const<type>::type>::value>();

		bool null;
		Copy(&null, sizeof(null));

		if (null)
		{
			value = NULL;
		}
		else
		{
			// 메모리를 할당하므로 관리에 주의한다.
			value = new type;
			Copy(value, sizeof(type));
		}
	}

	// POD 형
	template<class T>
	void Load(T& value, std::tr1::true_type, std::tr1::false_type)
	{
		Copy(&value, sizeof(value));
	}

	// POD 아닌 포인터형
	template<class T>
	void Load(T& value, std::tr1::false_type, std::tr1::true_type)
	{
		using std::tr1::remove_pointer;
		typedef remove_pointer<T>::type type;

		bool null;
		Copy(&null, sizeof(null));

		if (null)
		{
			value = NULL;
		}
		else
		{
			// 메모리를 할당하므로 관리에 주의한다.
			value = new type;
			value->Serialize(*this);
		}
	}

	// POD 아닌 형
	template<class T>
	void Load(T& value, std::tr1::false_type, std::tr1::false_type)
	{
		value.Serialize(*this);
	}

public:
	template<class T>
	Archive& operator &(T& value)
	{
		return operator >>(value);
	}

	template<class T>
	Archive& operator >>(T& value)
	{
		using std::tr1::is_pod;
		using std::tr1::is_pointer;
		using std::tr1::remove_pointer;
		typedef remove_pointer<T>::type type;
		Load(value, is_pod<type>(), is_pointer<T>());
		return *this;
	}

	Archive& operator >>(std::string& value)
	{
		using std::string;
		typedef string::size_type size_type;
		size_type size = LoadSize(value);

		value.assign(_index, size);
		_index += size;

		return *this;
	}

	template<class Container>
	typename Container::size_type LoadSize(const Container&)
	{
		typedef typename Container::size_type size_type;
		size_type size;
		*this >> size;

		if (Underflow(size))
			return 0;

		return size;
	}

	template<class T>
	Archive& operator >>(std::vector<T>& container)
	{
		using std::vector;
		typedef typename vector<T>::size_type size_type;
		size_type size = LoadSize(container);
		container.reserve(size);

		for (size_type i = 0; i < size; ++i)
		{
			T value;
			*this >> value;
			container.push_back(value);
		}

		return *this;
	}

	template<class T>
	Archive& operator >>(std::list<T>& container)
	{
		using std::list;
		typedef typename list<T>::size_type size_type;
		size_type size = LoadSize(container);

		for (size_type i = 0; i < size; ++i)
		{
			T value;
			*this >> value;
			container.push_back(value);
		}

		return *this;
	}

	template<class T>
	Archive& operator >>(std::set<T>& container)
	{
		using std::set;
		typedef typename set<T>::size_type size_type;
		size_type size = LoadSize(container);

		for (size_type i = 0; i < size; ++i)
		{
			T value;
			*this >> value;
			container.insert(value);
		}

		return *this;
	}

	template<class K, class V>
	Archive& operator >>(std::map<K, V>& container)
	{
		using std::map;
		typedef typename map<K, V>::size_type size_type;
		size_type size = LoadSize(container);

		for (size_type i = 0; i < size; ++i)
		{
			K key;
			V value;
			*this >> key >> value;
			container.insert(std::make_pair(key, value));
		}

		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////

#define SERIAL_CLASS(arg)                   \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar)              \
{                                           \
	ar << arg;                              \
}                                           \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar) const        \
{                                           \
	ar << arg;                              \
}                                           \
template<>                                  \
void Serialize<0>(Archive<0>& ar)           \
{                                           \
	ar >> arg;                              \
}                                           \
virtual void Serialize() { throw; } // is_pod 가 아니도록 만드는 더미 가상 함수

//////////////////////////////////////////////////////////////////////////

#define SERIAL_PACKET(event, arg)           \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar)              \
{                                           \
	ar << event << arg;                     \
}                                           \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar) const        \
{                                           \
	ar << event << arg;                     \
}                                           \
template<>                                  \
void Serialize<0>(Archive<0>& ar)           \
{                                           \
	ar >> arg;                              \
}                                           \
virtual void Serialize() { throw; }         \
enum { EVENT = event }; // 디스패처에서 알 수 있도록 고정적으로 정해진 패킷 이벤트명

//////////////////////////////////////////////////////////////////////////

#define SERIAL_EVENT(event)                 \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar)              \
{                                           \
	ar << event;                            \
}                                           \
template<std::size_t N>                     \
void Serialize(Archive<N>& ar) const        \
{                                           \
	ar << event;                            \
}                                           \
template<>                                  \
void Serialize<0>(Archive<0>&)              \
{                                           \
}                                           \
virtual void Serialize() { throw; }         \
enum { EVENT = event };
