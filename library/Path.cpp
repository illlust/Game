#include <Shlwapi.h>
#include "Path.h"

#pragma comment(lib, "Shlwapi.lib")

//////////////////////////////////////////////////////////////////////////

std::string Path::GetModuleName()
{
	char buf[MAX_PATH];
	GetModuleFileNameA(NULL, buf, sizeof(buf));
	return buf;
}

std::string Path::GetModulePath()
{
	char buf[MAX_PATH];
	strcpy_s(buf, GetModuleName().c_str());
	PathRemoveFileSpecA(buf);
	PathAddBackslashA(buf);
	return buf;
}

std::string Path::RenameExtension(std::string path, std::string extension)
{
	char buf[MAX_PATH];
	strcpy_s(buf, path.c_str());
	//strcat_s(buf, ".");
	PathRenameExtensionA(buf, ("." + extension).c_str());
	return buf;
}
