#pragma once

#include <string>

//////////////////////////////////////////////////////////////////////////

class Path
{
public:
	static std::string GetModuleName();
	static std::string GetModulePath();
	static std::string RenameExtension(std::string path, std::string extension);
};
