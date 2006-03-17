#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbcfile.h"


DBCFile::DBCFile()
{
	data = NULL;
}


bool DBCFile::Load(const char *filename)
{

	uint32 header;
	if(data)
	{
		delete [] data;
		data=NULL;	
	}
	FILE * f=fopen(filename,"rb");
	if(!f)return false;

	fread(&header,4,1,f); // Number of records
	if(header!=0x43424457)
	{
		//printf("not dbc file");
		return false;//'WDBC'
	}
	fread(&recordCount,4,1,f); // Number of records
	fread(&fieldCount,4,1,f); // Number of fields
	fread(&recordSize,4,1,f); // Size of a record
	fread(&stringSize,4,1,f); // String size
	
	if(fieldCount*4 != recordSize) return false;

	data = new unsigned char[recordSize*recordCount+stringSize];
	stringTable = data + recordSize*recordCount;
	fread(data,recordSize*recordCount+stringSize,1,f);
	fclose(f);
	return true;
}

DBCFile::~DBCFile()
{
	if(data)
		delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
	assert(data);
	return Record(*this, data + id*recordSize);
}


void * DBCFile::AutoProduceData(const char * format,uint32 * records)
{
/*
format STRING, NA, FLOAT,NA,INT <=>
struct{
char* field0,
float field1,
int field2
}entry;

this func will generate  entry[rows] data;
*/
	typedef char * ptr;
	char * _data;
	ptr* table;
	int i=-1;
	
	uint32 recordsize=0;
	uint32 offset=0;

	if(strlen(format)!=fieldCount)
	{
		printf("Error, probably dbc file format was updated (%d fields are in DBC).\n",fieldCount);
		return NULL;
	}
	//get struct size
	for(int x=0;x<fieldCount;x++)
	switch(format[x])
	{
		case FT_FLOAT:
		case FT_INT:
		recordsize+=4;
		break;
		case FT_STRING:
		recordsize+=sizeof(char*);
		break;
		case FT_SORT:
		i=x;
        break;
		case FT_IND:
		i=x;
		recordsize+=4;
        break;
	}
	
	
	if(i>=0)
	{	
		uint32 maxi=0;
		//find max index
		for(uint32 y=0;y<recordCount;y++)
		{
			uint32 ind=getRecord(y).getUInt (i);
			if(ind>maxi)maxi=ind;
		}
		
		*records=maxi;
		maxi++;
		table=new ptr[maxi];
		memset(table,0,maxi*sizeof(ptr));
	}else 
	{
		*records = recordCount;
		table = new ptr [recordCount];
	}

		_data= new char[recordCount *recordsize];
		
		for(uint32 y =0;y<recordCount;y++)
		{
			//offset=0;
			if(i>=0)
			{
				table[getRecord(y).getUInt (i)]=&_data[offset];
			}else table[y]=&_data[offset];

			for(uint32 x=0;x<fieldCount;x++)
			switch(format[x])
			{
				case FT_FLOAT:
					*((float*)(&_data[offset]))=getRecord(y).getFloat (x);
					offset+=4;
					break;
				case FT_IND:
				case FT_INT:
					*((uint32*)(&_data[offset]))=getRecord(y).getUInt (x);
					offset+=4;
					break;
				break;
				case FT_STRING:
					uint32 l=strlen(getRecord(y).getString  (x))+1;
					char * st=new char[l];
					memcpy(st,getRecord(y).getString  (x),l);
					*((char**)(&_data[offset]))=st;
					offset+=sizeof(char*);
					break;
			}
		}
	
	
return table;
}
