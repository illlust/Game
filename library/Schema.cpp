#include "Database.h"
#include "Schema.h"

//////////////////////////////////////////////////////////////////////////

int Schema::Patch(Database* db, int version)
{
	for (int i = version; i < UPPER_BOUND; ++i)
	{
		switch (i)
		{
		case CREATE_MEMBER_TABLE:
			{
				const char sql[] = "create table member             \
									(                               \
										member_id varchar(32),      \
										member_password varchar(32) \
									)";

				db->Execute(sql);
			}
			break;

		case PATCH2:
			db->Insert("insert member values ('m1', '1111')");
			break;

		case PATCH3:
			db->Insert("insert member values ('m2', '1111')");
			break;
		}
	}

	return UPPER_BOUND;
}
