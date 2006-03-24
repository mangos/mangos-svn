/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#ifndef DBCSTORES_H
#define DBCSTORES_H

#include "Common.h"
//#include "DataStore.h"
#include "dbcfile.h"
#include "DBCStructure.h"


struct bidentry
{
    uint32 AuctionID;
    uint32 amt;
};

struct AuctionEntry
{
    uint32 auctioneer;
    uint32 item;
    uint32 owner;
    uint32 bid;
    uint32 buyout;
    time_t time;
    uint32 bidder;
    uint32 Id;
};


float GetRadius(SpellRadius *radius);
uint32 GetCastTime(SpellCastTime *time);
float GetMinRange(SpellRange *range);
float GetMaxRange(SpellRange *range);
uint32 GetDuration(SpellDuration *dur);

template<class T>
class DBCStorage
{
public:
	DBCStorage(const char *f){data = NULL;fmt=f;}
	~DBCStorage(){if(data) delete [] data;};
	
	inline
	T* LookupEntry(uint32 id)
	{
		return (id>nCount)?NULL:data[id];
		
	}
	inline
	unsigned int GetNumRows()
	{
		return nCount;
	}


	void Load(char* fn)
	{
		
		dbc = new DBCFile; 
		dbc->Load(fn);
		data=(T **) dbc->AutoProduceData(fmt,&nCount);
		delete dbc;
	}

	T** data;
	uint32 nCount;

private:
	DBCFile * dbc;
	const char * fmt;
};


extern DBCStorage <TalentEntry> sTalentStore;
extern DBCStorage <SpellRadius> sSpellRadius; 
extern DBCStorage <AreaTableEntry> sAreaStore;
extern DBCStorage <FactionTemplateEntry> sFactionTemplateStore;

extern DBCStorage <SpellRange>  sSpellRange;
extern DBCStorage <emoteentry> sEmoteStore;

extern DBCStorage <SpellEntry> sSpellStore;
extern DBCStorage <SpellCastTime> sCastTime;
extern DBCStorage <SpellDuration> sSpellDuration;
extern DBCStorage <FactionEntry> sFactionStore;
extern DBCStorage <ItemSetEntry> sItemSetStore;
extern DBCStorage <ItemDisplayTemplateEntry> sItemDisplayTemplateStore;
#endif
