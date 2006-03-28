/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SQLStorage.h" 
#include "../game/EventSystem.h"

const char ItemPrototypefmt[]="iiissssiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffffffiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiisiiiiiiiiiiiis";
const char GameObjectInfofmt[]="iiisiifiiiiiiiiiis";
const char CreatureInfofmt[]="iissiiiiiififfiiiiiiififiiffifiiiiiiiiiiiiiiiiiiiisss";

SQLStorage sItemStorage(ItemPrototypefmt,"itemstemplate");
SQLStorage sGOStorage(GameObjectInfofmt,"gameobjecttemplate");
SQLStorage sCreatureStorage(CreatureInfofmt,"creaturetemplate");

void FreeStorage(SQLStorage * p)
{
	p->Free();
}

void SQLStorage::Free ()
{
	uint32 offset=0;
	for(uint32 x=0;x<iNumFields;x++)
	if(format[x]==FT_STRING)
	{
		for(uint32 y=0;y<iOldNumRecords;y++)
			if(pOldIndex[y])
			delete [] ((char*)(pOldIndex[y])+offset);
		
		offset+=sizeof(char*);

	}else offset+=4;
	

	delete [] pOldIndex;
	delete [] pOldData;
	
}


void SQLStorage::Load ()
{	
	uint32 maxi;
	Field *fields;
	QueryResult *result  = sDatabase.PQuery("SELECT MAX(entry) FROM %s;",table); 
	if(!result)
	{
		printf("Error loading %s table\n",table);
		return;
	}

	maxi= (*result)[0].GetUInt32()+1;
	delete result;

	result = sDatabase.PQuery("SELECT COUNT(*) FROM %s;",table);	

	fields = result->Fetch();
	uint32 recordcount=fields[0].GetUInt32(); 
	delete result;

	result = sDatabase.PQuery("SELECT * from %s;",table);	

	if(!result)
	{
	    printf("%s table is empty!\n",table);
	    return;
	}
 

	uint32 recordsize=0;
	uint32 offset=0;
	

	if(iNumFields!=result->GetFieldCount())
	{	
		printf("Error, probably sql file format was updated (there should be %d fields in sql).\n",iNumFields);
		return;
	}
	

	if(sizeof(void*)==4)
		recordsize=4*iNumFields;
	else
	{
		//get struct size
		uint32 sc=0;
		for(uint32 x=0;x<iNumFields;x++)
		if(format[x]==FT_STRING)
			sc++;
		recordsize=(iNumFields-sc)*4+sc*sizeof(char*);
	}
	
	char** newIndex=new char*[maxi];
	memset(newIndex,0,maxi*sizeof(char*));
	
	char * _data= new char[recordcount *recordsize];
	uint32 count=0;
	do
	{
		fields = result->Fetch();
		char *p=(char*)&_data[recordsize*count];
		newIndex[fields[0].GetUInt32()]=p;

		offset=0;
		for(uint32 x=0;x<iNumFields;x++)
		switch(format[x])
		{
			case FT_INT:
			*((uint32*)(&p[offset]))=fields[x].GetUInt32();
			offset+=4;
			break;
			case FT_FLOAT:
			*((float*)(&p[offset]))=fields[x].GetFloat();
			offset+=4;
			break;
			case FT_STRING:
			char * tmp=(char*)fields[x].GetString();
			char* st;
			if(!tmp)
			{
				st=new char[1];
				*st=0;
			}else{
			uint32 l=strlen(tmp)+1;
			st=new char[l];
			memcpy(st,tmp,l);}
			*((char**)(&p[offset]))=st;
			offset+=sizeof(char*);
			break;
		}
	count++;
	}while( result->NextRow() );

	delete result; 


	if(data)//Reload table
		AddEvent((EventHandler)(&FreeStorage),this,20000);
	
	pOldData=data;
	pOldIndex=pIndex;
	iOldNumRecords=iNumRecords;
	
	pIndex =newIndex;
	iNumRecords=maxi;
	data=_data;

}







