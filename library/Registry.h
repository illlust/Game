#pragma once

#include <WinReg.h>
#include <iostream>
#include <string>
//#include <exception>

//////////////////////////////////////////////////////////////////////////

class Registry
{
private:

	class Handle
	{
	public:
		explicit Handle(HKEY key) : _key(key) {}
		~Handle() { RegCloseKey(_key); }

		operator HKEY() { return _key; }

	private:
		Registry& operator =(const Registry&);
		HKEY _key;
	};

public:
	explicit Registry(HKEY root) : _root(root), _name(""), _parent(0) {}

	explicit Registry(const char* name, const Registry* parent)
		: _root(0), _name(name), _parent(parent) {}

	const Registry operator /(const char* name) const
	{
		return Registry(name, this);
	}

	const Registry& operator =(int value) const
	{
		RegSetValueExA(Open(), _name, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
		return *this;
	}

	const Registry& operator =(const char* value) const
	{
		RegSetValueExA(Open(), _name, 0, REG_SZ, (LPBYTE)value, strlen(value));
		return *this;
	}

	operator int() const
	{
		DWORD buf = 0;
		DWORD size = sizeof(buf);
		RegQueryValueExA(Open(), _name, 0, 0, (LPBYTE)&buf, &size);
		return buf;
	}

	operator std::string() const
	{
		char buf[MAX_PATH] = { 0, };
		DWORD size = sizeof(buf);
		RegQueryValueExA(Open(), _name, 0, 0, (LPBYTE)buf, &size);
		return buf;
	}

#ifdef _AFXDLL
	operator CString() const
	{
		return operator std::string().c_str();
	}
#endif

private:
 	Handle Create() const
 	{
		HKEY key = 0;
		RegCreateKeyExA(Root()->_root, Key().c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, 0);
		return Handle(key);
	}

	Handle Open() const
	{
		HKEY key = 0;
		if (RegOpenKeyExA(Root()->_root, Key().c_str(), 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
			return Create();

		return Handle(key);
	}

	std::string Key(const Registry* caller) const
	{
		if (_parent == 0) return "";
		if (caller == this) return _parent->Key(this);
		return _parent->Key(this) + "\\" + _name;
	}

public:
	DWORD Type() const
	{
		DWORD type = REG_NONE;
		if (RegQueryValueExA(Open(), _name, 0, &type, 0, 0) != ERROR_SUCCESS)
			return REG_NONE;

		return type;
	}

	std::string Path() const
	{
		return Key() + "\\" + _name;
	}

	std::string Key() const
	{
		return Key(this).c_str() + 1;
	}

	const Registry* Root() const
	{
		if (!_parent) return this;

		const Registry* root = _parent;
		while (root->_parent)
		{
			root = root->_parent;
		}

		return root;
	}

	void Erase() const
	{
		switch (Type())
		{
		case REG_DWORD:
		case REG_SZ:
			RegDeleteValueA(Open(), _name);
			break;

		case REG_NONE:
			{
				Registry name = (*this)/_name;
				Handle key = name.Open();
				if (key == 0) return;

				char buf[MAX_PATH] = { 0, };
				while (RegEnumKeyA(key, 0, buf, sizeof(buf)) != ERROR_NO_MORE_ITEMS)
				{
					std::string subkey = Path() + "\\" + buf;
					Registry reg = (*Root())/subkey.c_str();
					reg.Erase();
				}

				RegDeleteKeyA(Open(), _name);
			}
			break;

		//default:
		//	throw std::exception("Access only REG_SZ, REG_DWORD");
		}
	}

private:
	Registry(const Registry&);
	Registry& operator =(const Registry&);

	const HKEY _root;
	const char* _name;
	const Registry* _parent;
};

//////////////////////////////////////////////////////////////////////////

#define HKCR Registry(HKEY_CLASSES_ROOT)
#define HKCC Registry(HKEY_CURRENT_CONFIG)
#define HKCU Registry(HKEY_CURRENT_USER)
#define HKLM Registry(HKEY_LOCAL_MACHINE)
#define HKUS Registry(HKEY_USERS)

std::ostream& operator <<(std::ostream& stream, const Registry& p);
