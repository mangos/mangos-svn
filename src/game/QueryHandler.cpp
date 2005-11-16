/* QueryHandler.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NAME_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNameQueryOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;

    recv_data >> guid;

    uint32 race = 0, gender = 0, cl = 0;
    std::string name = "ERROR_NO_NAME_FOR_GUID";

#ifndef ENABLE_GRID_SYSTEM
    Player *pChar = objmgr.GetObject<Player>(guid);
#else
    Player *pChar = ObjectAccessor::Instance().FindPlayer(guid);
#endif
    if (pChar == NULL)
    {
        if (!objmgr.GetPlayerNameByGUID(guid, name))
            Log::getSingleton( ).outError( "No player name found for this guid" );

        // TODO: load race, class, sex, etc from db
    }
    else
    {
        race = pChar->getRace();
        gender = pChar->getGender();
        cl = pChar->getClass();
        name = pChar->GetName();
    }

    Log::getSingleton( ).outDebug( "Recieved CMSG_NAME_QUERY for: %s", name.c_str() );

    data.Initialize( SMSG_NAME_QUERY_RESPONSE );

    data << guid;
    data << name;
    data << race << gender << cl;

    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_QUERY_TIME:
//////////////////////////////////////////////////////////////
void WorldSession::HandleQueryTimeOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );

    data << (uint32)time(NULL);
    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_CREATURE_QUERY:
//////////////////////////////////////////////////////////////
// UQ1: Defaults...
extern uint32 default_trainer_guids[12];

void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recv_data )
{// UQ1: Think I have this correct now.. :)
    WorldPacket data;
    uint32 entry;
    uint64 guid;
    CreatureInfo *ci;

    recv_data >> entry;
    recv_data >> guid;

    //ci = objmgr.GetCreatureName(entry);
    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

	if (unit == NULL)
	{
		Log::getSingleton( ).outDebug( "WORLD: HandleCreatureQueryOpcode - (%u) NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
		return;
	}

	ci = objmgr.GetCreatureName(unit->GetNameID());

	if (!ci)
	{
		Log::getSingleton( ).outDebug( "WORLD: HandleCreatureQueryOpcode - (%u) NO CREATUREINFO! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
		return;
	}

	Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u - GUID: %u.", ci->Name.c_str(), entry, guid);

	Trainerspell *strainer = objmgr.GetTrainerspell(unit->GetNameID());

	uint32 npcflags = unit->GetUInt32Value(UNIT_NPC_FLAGS);

/*
	if ((npcflags & UNIT_NPC_FLAG_VENDOR) > 0)
    {
		Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY %s is a vendor.", ci->Name.c_str());
        //data << uint32(UNIT_NPC_FLAG_VENDOR);       // Flags1
		unit->SetUInt32Value(UNIT_NPC_FLAGS, npcflags);
    }
	else if ((npcflags & UNIT_NPC_FLAG_TRAINER) > 0)
    {
		Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY %s is a trainer.", ci->Name.c_str());
        //data << uint32(UNIT_NPC_FLAG_TRAINER);       // Flags1
		unit->SetUInt32Value(UNIT_NPC_FLAGS, npcflags);
    }
    else
    {
        //data << (uint32)npcflags;//ci->flags1;        // Flags1
    }
*/

    data.Initialize( SMSG_CREATURE_QUERY_RESPONSE );
    data << (uint32)entry;
    data << ci->Name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << ci->SubName.c_str();    // Subname (Guild Name)
    
	data << (uint32)ci->flags1;        // Flags1

	// UQ1: This is how wowwow does it.. Don't know what the "7" means...
    
	if ((ci->Type & 2) > 0)
    {
        data << uint32(7);
    }
	else
    {
        data << uint32(0);
    }

/*
#define CREATURE_TYPE_BEAST				1
#define CREATURE_TYPE_DRAGON			2
#define CREATURE_TYPE_DEMON				3
#define CREATURE_TYPE_ELEMENTAL			4
#define CREATURE_TYPE_GIANT				5
#define CREATURE_TYPE_UNDEAD			6
#define CREATURE_TYPE_HUMANOID			7
#define CREATURE_TYPE_CRITTER			8
#define CREATURE_TYPE_MECHANICAL		9
#define CREATURE_TYPE_UNKNOWN			10
*/

    data << ci->Type;            // Creature Type

	// UQ1: Set these values for elite field for:
	// 1		= Elite
	// 2		= Rare Elite
	// 3		= World Boss
	// 4		= Rare Boss (Not sure if this will show though)
	// 5		= %d mana
	// 6		= %d rage
	// 7		= %d focus
	// 8		= %d energy
	// 10		= 100% mana
	// 11		= 100% rage
	// 12		= 100% focus
	// 13		= 100% energy
	//data << (uint32)0; // Elite
	if (ci->level >= 16 && ci->level < 32)
		data << (uint32)CREATURE_ELITE_ELITE; // Elite
	else if (ci->level >= 32 && ci->level < 48)
		data << (uint32)CREATURE_ELITE_RAREELITE; // Elite
	else if (ci->level >= 48 && ci->level < 59)
		data << (uint32)CREATURE_ELITE_WORLDBOSS; // Boss
	else if (ci->level >= 60)
		data << (uint32)CREATURE_ELITE_RARE; // Boss
	else
		data << (uint32)CREATURE_ELITE_NORMAL; // Standard

/*
#define CREATURE_FAMILY_WOLF			1
#define CREATURE_FAMILY_CAT				2
#define CREATURE_FAMILY_SPIDER			3
#define CREATURE_FAMILY_BEAR			4
#define CREATURE_FAMILY_BOAR			5
#define CREATURE_FAMILY_CROCILISK		6
#define CREATURE_FAMILY_CARRION_BIRD	7
#define CREATURE_FAMILY_CRAB			8
#define CREATURE_FAMILY_GORILLA			9
#define CREATURE_FAMILY_RAPTOR			11
#define CREATURE_FAMILY_TALLSTRIDER		12
#define CREATURE_FAMILY_FELHUNTER		15
#define CREATURE_FAMILY_VOIDWALKER		16
#define CREATURE_FAMILY_SUCCUBUS		17
#define CREATURE_FAMILY_DOOMGUARD		19
#define CREATURE_FAMILY_SCORPID			20
#define CREATURE_FAMILY_TURTLE			21
#define CREATURE_FAMILY_IMP				23
#define CREATURE_FAMILY_BAT				24
#define CREATURE_FAMILY_HYENA			25
#define CREATURE_FAMILY_OWL				26
#define CREATURE_FAMILY_WIND_SERPENT	27
*/
	data << (uint32)ci->family;    // Family

    data << (uint32)0;            // Unknown
    data << ci->DisplayID;        // DisplayID

	// UQ1: Add some padding data... I'm positive we can send more here!
	data << (uint32)0;
	data << (uint32)0;
	data << (uint32)0;
	data << (uint32)0;

    SendPacket( &data );
}

void WorldSession::SendTestCreatureQueryOpcode( uint32 entry, uint64 guid, uint32 testvalue )
{// UQ1: For testing values... use testvalue and ".npcinfoset <value>" ingame
    WorldPacket data;
    CreatureInfo *ci;

    ci = objmgr.GetCreatureName(entry);
	Log::getSingleton( ).outDetail("WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u - GUID: %u.", ci->Name.c_str(), entry, guid);

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    Trainerspell *strainer = objmgr.GetTrainerspell(entry/*unit->GetNameID()*/);

	if ((ci->flags1 & UNIT_NPC_FLAG_TRAINER) && !strainer)
	{// Use Defaults...
		strainer = objmgr.GetTrainerspell(default_trainer_guids[ci->classNum]);
	}

    data.Initialize( SMSG_CREATURE_QUERY_RESPONSE );
    data << (uint32)entry;
    data << ci->Name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << ci->SubName.c_str();    // Subname (Guild Name)
    data << (uint32)ci->flags1;        // Flags1

    if ((ci->Type & 2) > 0)
    {
        data << uint32(7);
    }
    else
    {
        data << uint32(0);
    }

    //data << ci->Type;            // Creature Type
	data << (uint32)testvalue;

	// UQ1: Set these values for elite field for:
	// 1		= Elite
	// 2		= Rare Elite
	// 3		= World Boss
	// 4		= Rare Boss (Not sure if this will show though)
	// 5		= %d mana
	// 6		= %d rage
	// 7		= %d focus
	// 8		= %d energy
	// 10		= 100% mana
	// 11		= 100% rage
	// 12		= 100% focus
	// 13		= 100% energy
	//data << (uint32)0; // Elite
	if (ci->level >= 16 && ci->level < 32)
		data << (uint32)CREATURE_ELITE_ELITE; // Elite
	else if (ci->level >= 32 && ci->level < 48)
		data << (uint32)CREATURE_ELITE_RAREELITE; // Elite
	else if (ci->level >= 48 && ci->level < 59)
		data << (uint32)CREATURE_ELITE_WORLDBOSS; // Boss
	else if (ci->level >= 60)
		data << (uint32)CREATURE_ELITE_RARE; // Boss
	else
		data << (uint32)CREATURE_ELITE_NORMAL; // Standard

	data << (uint32)ci->family;    // Family

    data << (uint32)0;            // Unknown (move before or after unknowns 3 and 4) don't know where exactly
    data << ci->DisplayID;        // DisplayID

	data << (uint32)0;
	data << (uint32)0;
	data << (uint32)0;
	data << (uint32)0;

    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
/*
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;    

    Log::getSingleton( ).outDetail("WORLD: CMSG_GAMEOBJECT_QUERY '%u'", guid);
    const GameObjectInfo *info = objmgr.GetGameObjectInfo(entryID);
    if( info == NULL )
    {
    Log::getSingleton( ).outDebug( "Missing game object info for entry %d", entryID);    
    return; // no luck..
    }

    data << (uint32)(entryID);
    data << (uint32)info->type;
    data << (uint32)info->displayId;
    data << info->name.c_str();
    data << uint8(0) << uint8(0) << uint8(0);
    data << uint32(info->sound0);
    data << uint32(info->sound1);
    data << uint32(info->sound2);
    data << uint32(info->sound3);
    data << uint32(info->sound4);
    data << uint32(info->sound5);
    data << uint32(info->sound6);
    data << uint32(info->sound7);
    data << uint32(info->sound8);
    data << uint32(info->sound9);
    data << uint16(0);
    data << uint8(0);
    SendPacket( &data );
}
*/
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{

    WorldPacket data;
    data.Initialize( SMSG_GAMEOBJECT_QUERY_RESPONSE );
    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;

    Log::getSingleton( ).outDetail("WORLD: CMSG_GAMEOBJECT_QUERY '%u'", guid);

    const GameObjectInfo *info = objmgr.GetGameObjectInfo(entryID);
    if( info == NULL )
    {
        Log::getSingleton( ).outDebug( "Missing game object info for entry %d", entryID);    
        return; // no luck..
    }

/*
    Converter.ToBytes(obj1.Id, this.tempBuff, ref num1);
    Converter.ToBytes(obj1.ObjectType, this.tempBuff, ref num1);
    Converter.ToBytes(obj1.Model, this.tempBuff, ref num1);
    Converter.ToBytes(obj1.Name, this.tempBuff, ref num1);
    Converter.ToBytes(0, this.tempBuff, ref num1);
    for (int num2 = 0; num2 < obj1.Sound.Length; num2++)
    {
        Converter.ToBytes(obj1.Sound[num2], this.tempBuff, ref num1);
    }
    for (int num3 = obj1.Sound.Length; num3 < 0x10; num3++)
    {
        Converter.ToBytes(0, this.tempBuff, ref num1);
    }
*/
   
    data << (uint32)(entryID);
    data << (uint32)info->type;
    data << (uint32)info->displayId;
    data << info->name.c_str();
    data << uint32(0);
    
    data << uint32(info->sound0);
    data << uint32(info->sound1);
    data << uint32(info->sound2);
    data << uint32(info->sound3);
    data << uint32(info->sound4);
    data << uint32(info->sound5);
    data << uint32(info->sound6);
    data << uint32(info->sound7);
    data << uint32(info->sound8);
    data << uint32(info->sound9);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    /*data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);*/

    data << uint16(0);
    data << uint8(0);

    SendPacket( &data );
}



//////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
    Log::getSingleton().outDetail("WORLD: Received MSG_CORPSE_QUERY");

    Corpse *pCorpse = NULL;
    WorldPacket data;
#ifndef ENABLE_GRID_SYSTEM
    pCorpse = objmgr.GetCorpseByOwner(GetPlayer());
#else
    pCorpse = ObjectAccessor::Instance().GetCorpse(*GetPlayer(), GetPlayer()->GetGUID());
#endif
    if(pCorpse)
    {
        data.Initialize(MSG_CORPSE_QUERY);
        data << uint8(0x01);
        data << uint32(0x00000001);
        data << pCorpse->GetPositionX();
        data << pCorpse->GetPositionY();
        data << pCorpse->GetPositionZ();
        data << uint32(0x00000001);
        SendPacket(&data);
    }
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NPC_TEXT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 textID;
    uint32 uField0, uField1;
    GossipText *pGossip;
	std::string GossipStr;

    recv_data >> textID;
    Log::getSingleton( ).outDetail("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID );

    recv_data >> uField0 >> uField1;
    GetPlayer()->SetUInt32Value(UNIT_FIELD_TARGET, uField0);
    GetPlayer()->SetUInt32Value(UNIT_FIELD_TARGET + 1, uField1);

	pGossip = objmgr.GetGossipText(textID);
	
    data.Initialize( SMSG_NPC_TEXT_UPDATE );
    data << textID;

	if (!pGossip)
	{
		data << uint32( 0 );
		data << "Greetings $N";
		data << "Greetings $N";
	} else

	for (int i=0; i<8; i++)
	{
		data << pGossip->Options[i].Probability;
		data << pGossip->Options[i].Text_0;

		if ( pGossip->Options[i].Text_1 == "" )
			data << pGossip->Options[i].Text_0; else
			data << pGossip->Options[i].Text_1;

		data << pGossip->Options[i].Language;

		data << pGossip->Options[i].Emotes[0]._Delay;
		data << pGossip->Options[i].Emotes[0]._Emote;

		data << pGossip->Options[i].Emotes[1]._Delay;
		data << pGossip->Options[i].Emotes[1]._Emote;

		data << pGossip->Options[i].Emotes[2]._Delay;
		data << pGossip->Options[i].Emotes[2]._Emote;
	}

    SendPacket( &data );

    Log::getSingleton( ).outString( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_PAGE_TEXT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandlePageQueryOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 pageID;

    recv_data >> pageID;
    Log::getSingleton( ).outDetail("WORLD: Received CMSG_PAGE_TEXT_QUERY for pageID '%u'", pageID);
	
	while (pageID)
	{
		ItemPage *pPage = objmgr.RetreiveItemPageText( pageID );
		data.Initialize( SMSG_PAGE_TEXT_QUERY_RESPONSE );
		data << pageID;

		if (!pPage)
		{
			data << "The selected item page is missing!$b$bPlease report to Ludmilla Server Developers.";
			data << uint32(0);

			pageID = 0;
		} else
		{
			data << pPage->PageText;
			data << uint32(pPage->Next_Page);

			pageID = pPage->Next_Page;
		}

		SendPacket( &data );

		Log::getSingleton( ).outString( "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE " );
	}
}


