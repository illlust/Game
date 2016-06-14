#include <Windows.h>
#include "Registry.h"

//////////////////////////////////////////////////////////////////////////

std::ostream& operator <<(std::ostream& stream, const Registry& p)
{
	return stream << (std::string)p;
}
