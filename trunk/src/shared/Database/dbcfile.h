#ifndef DBCFILE_H
#define DBCFILE_H
#include "Platform/Define.h"
#include <cassert>



enum {
FT_NA='x',//not used or unknown
FT_STRING='s',//char*
FT_FLOAT='f',//float
FT_INT='i',//uint32
FT_SORT='d',//sorted by this field, field is not included
FT_IND='n'//the same,but parsed to data
};








class DBCFile
{
public:
	DBCFile();
	~DBCFile();

	bool Load(const char *filename);

	
	

	class Record
	{
	public:
		float getFloat(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<float*>(offset+field*4);
		}
		uint32 getUInt(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<uint32*>(offset+field*4);
		}
			
		
		const char *getString(size_t field) const
		{
			assert(field < file.fieldCount);
			size_t stringOffset = getUInt(field);
			assert(stringOffset < file.stringSize);
			return reinterpret_cast<char*>(file.stringTable + stringOffset);
		}
	private:
		Record(DBCFile &file, unsigned char *offset): file(file), offset(offset) {}
		unsigned char *offset;
		DBCFile &file;

		friend class DBCFile;

	};
	

	// Get record by id
	Record getRecord(size_t id);
	/// Get begin iterator over records

	uint32 GetNumRows() const { return recordCount;}
	uint32 GetCols() const { return fieldCount; }
	bool IsLoaded() {return (data!=NULL);}
	void *AutoProduceData(const char*,uint32 *);
private:
	
	uint32 recordSize;
	uint32 recordCount;
	uint32 fieldCount;
	uint32 stringSize;
	unsigned char *data;
	unsigned char *stringTable;
};

#endif
