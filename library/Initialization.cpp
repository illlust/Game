#include "Initialization.h"
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

//////////////////////////////////////////////////////////////////////////

Initialization::Section::ValueProxy::ValueProxy(std::string& value) : _value(value)
{
	
}

Initialization::Section::ValueProxy::operator std::string()
{
	return _value;
}

Initialization::Section::ValueProxy::operator int()
{
	return atoi(_value.c_str());
}

//Initialize::Section::ValueProxy::operator CRect()
//{
//	std::string strLeft, strTop, strWidth, strHeight;
//	AfxExtractSubString(strLeft,   m_refValue, 0, ',');
//	AfxExtractSubString(strTop,    m_refValue, 1, ',');
//	AfxExtractSubString(strWidth,  m_refValue, 2, ',');
//	AfxExtractSubString(strHeight, m_refValue, 3, ',');
//
//	return CRect(atoi(strLeft), atoi(strTop), atoi(strWidth), atoi(strHeight));
//}

Initialization::Section::ValueProxy& Initialization::Section::ValueProxy::operator =(std::string value)
{
	_value = value;
	return *this;
}

Initialization::Section::ValueProxy& Initialization::Section::ValueProxy::operator =(int value)
{
	char buf[32];
	_itoa_s(value, buf, 10);
	_value = buf;
	return *this;
}

//Initialize::Section::ValueProxy& Initialize::Section::ValueProxy::operator =(CRect rcValue)
//{
//	std::string strValue;
//	strValue.Format("%d,%d,%d,%d", rcValue.left, rcValue.top, rcValue.right, rcValue.bottom);
//	m_refValue = strValue;
//	return *this;
//}

//////////////////////////////////////////////////////////////////////////

Initialization::Section::Section(PairMap& pair) : _pair(pair)
{

}

Initialization::Section::ValueProxy Initialization::Section::operator [](std::string key)
{
	return _pair[key];
}

//////////////////////////////////////////////////////////////////////

Initialization::Initialization(std::string path) : _path(path)
{
	Load(_path);
}

Initialization::~Initialization()
{
	Save(_path);
}

Initialization::Section Initialization::operator [](std::string section)
{
	return _section[section];
}

std::string Initialization::GetSectionString(std::string section, std::string key)
{
	return (*this)[section][key];
}

int Initialization::GetSectionInt(std::string section, std::string key)
{
	return (*this)[section][key];
}

void Initialization::SetSectionString(std::string section, std::string key, std::string value)
{
	(*this)[section][key] = value;
}

void Initialization::SetSectionInt(std::string section, std::string key, int value)
{
	(*this)[section][key] = value;
}

void Initialization::SetPath(std::string path)
{
	_path = path;
}

void Initialization::Load(std::string path)
{
	if (!PathFileExistsA(path.c_str()))
		return;

	char sectionNamesBuffer[1024];
	GetPrivateProfileSectionNamesA(sectionNamesBuffer, sizeof(sectionNamesBuffer), path.c_str());
	const char* currentSectionName = sectionNamesBuffer;

	while (*currentSectionName)
	{
		char sectionBuffer[1024];
		GetPrivateProfileSectionA(currentSectionName, sectionBuffer, sizeof(sectionBuffer), path.c_str());
		const char* currentPair = sectionBuffer;

		Section::PairMap pair;
		Section section(pair);

		while (*currentPair)
		{
			std::string current = currentPair;
			std::string key = current.substr(0, current.find('='));
			std::string value = current.substr(current.find('=') + 1);

			if (key != "")
			{
				section._pair[key] = value;
			}

			currentPair += strlen(currentPair) + 1;
		}

		_section[currentSectionName] = section._pair;

		currentSectionName += strlen(currentSectionName) + 1;
	}
}

void Initialization::Save(std::string path)
{
	if (path == "")
		return;

	HANDLE file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file ==INVALID_HANDLE_VALUE)
		return;

	for (SectionMap::iterator it = _section.begin(); it != _section.end(); ++it)
	{
		std::string section = "[" + it->first + "]\r\n";
		DWORD written;
		WriteFile(file, section.c_str(), section.size(), &written, NULL);

		for (Section::PairMap::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			std::string pair = it2->first + "=" + it2->second + "\r\n";
			DWORD write;
			WriteFile(file, pair.c_str(), pair.size(), &write, NULL);
		}
	}
}
