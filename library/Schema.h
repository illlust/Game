#pragma once

//////////////////////////////////////////////////////////////////////////

class Database;

class Schema
{
public:
	int Patch(Database* db, int version);

	enum Version
	{
		LOWER_BOUND = 0,

		CREATE_MEMBER_TABLE,
		PATCH2,
		PATCH3,

		UPPER_BOUND,
	};
};
