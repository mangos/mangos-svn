#ifndef SQLSTORAGE_H
#define SQLSTORAGE_H

#include "Common.h"
#include "Database/DatabaseEnv.h"

class SQLStorage
{
public:

	SQLStorage(const char*fmt,const char * sqlname)
	{
		format=fmt;
		table=sqlname;
		data=NULL;
		pIndex=NULL;
		iNumFields =strlen(fmt);
	}

	char** pIndex;
	uint32 iNumRecords;
	uint32 iNumFields;
	void Load();
	void Free();
private:

	char *data;
	char *pOldData;
	char **pOldIndex;
	uint32 iOldNumRecords;
	const char *format;
	const char *table;
	//bool HasString;
};
#endif
