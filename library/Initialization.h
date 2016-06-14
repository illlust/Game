#pragma once

#include <map>
#include <string>

//////////////////////////////////////////////////////////////////////////

class Initialization
{
public:

	class Section
	{
	public:

		class ValueProxy
		{
		public:
			ValueProxy(std::string& value);
			operator std::string();
			operator int();
			//operator CRect();
			ValueProxy& operator =(std::string value);
			ValueProxy& operator =(int value);
			//ValueProxy& operator =(CRect rcValue);
		private:
			std::string& _value;
		private:
			ValueProxy& operator =(const ValueProxy&);
		};

	public:
		typedef std::map<std::string, std::string> PairMap;
		PairMap& _pair;

	public:
		Section(PairMap& pairMap);
		ValueProxy operator [](std::string key);
	private:
		Section& operator =(const Section&);
	};

public:
	Initialization(std::string path = "");
	virtual ~Initialization();

	Section operator [](std::string section);
	std::string GetSectionString(std::string section, std::string key);
	int GetSectionInt(std::string section, std::string key);
	void SetSectionString(std::string section, std::string key, std::string value);
	void SetSectionInt(std::string section, std::string key, int value);
	void SetPath(std::string path);
	void Load(std::string path);
	void Save(std::string path);

public:
	typedef std::map<std::string, Section::PairMap> SectionMap;
	SectionMap _section;
	std::string _path;
};
