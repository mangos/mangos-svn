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


#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "EventSystem.h"



void HandleCorpsesErase(void*)
{
    sLog.outBasic("Global Event (corpses/bones removal)");

    QueryResult *result = sDatabase.PQuery("SELECT * FROM corpses WHERE UNIX_TIMESTAMP()-UNIX_TIMESTAMP(time) > 1200;");

    if(!result)
	{
	DEBUG_LOG("No corpses to erase");
	delete result;
	}
    else {
	DEBUG_LOG("We have corpses to erase");
	Field *fields = result->Fetch();

	uint64 guid = fields[0].GetUInt64();
	uint32 flag = fields[10].GetUInt32();

	// TODO check flag and time,
	// destroy object,delete corpse from DB

	//WorldPacket data;
	//data.Initialize( SMSG_DESTROY_OBJECT );
	//data << guid;
	//SendMessageToSet(&data,true);

	delete result;
	}

}


    // global event to erase corpses/bones
    uint32 m_CorpsesEventID = AddEvent(&HandleCorpsesErase,NULL,60000,false,true);
