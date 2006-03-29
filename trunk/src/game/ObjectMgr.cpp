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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"

#include "Log.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "World.h"
#include "WorldSession.h"
#include "Group.h"
#include "Guild.h"
#include "ProgressBar.cpp"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

extern SQLStorage sItemStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sCreatureStorage;

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid = 1;
    m_hiCreatureGuid = 1;
    m_hiItemGuid = 1;
    m_hiGoGuid = 1;
    m_hiDoGuid = 1;
	m_hiCorpseGuid=1;
}


ObjectMgr::~ObjectMgr()
{

 

    for( QuestMap::iterator i = mQuests.begin( ); i != mQuests.end( ); ++ i )
    {
        delete i->second;
    }
    mQuests.clear( );

   
    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    for( AreaTriggerMap::iterator i = mAreaTriggerMap.begin( ); i != mAreaTriggerMap.end( ); ++ i )
    {
        delete i->second;
    }
    mAreaTriggerMap.clear( );

  
   
}






Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    GroupSet::const_iterator itr;
    for (itr = mGroupSet.begin(); itr != mGroupSet.end(); itr++)
    {
        if ((*itr)->GetLeaderGUID() == guid)
        {
            return *itr;
        }
    }

    return NULL;
}




Guild * ObjectMgr::GetGuildById(const uint32 GuildId) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetId() == GuildId)
        {
            return *itr;
        }
    }

    return NULL;
}




CreatureInfo *ObjectMgr::GetCreatureTemplate(uint32 id)
{
	return (sCreatureStorage.iNumRecords<=id)?NULL:(CreatureInfo*)sCreatureStorage.pIndex[id];
}






void ObjectMgr::LoadCreatureTemplates()
{

	sCreatureStorage.Load();

  sLog.outString( "" );
//	sLog.outString( ">> Loaded %d creature definitions", count );
//	sLog.outString( "" );
}

PlayerCreateInfo* ObjectMgr::GetPlayerCreateInfo(uint32 race, uint32 class_)
{
    uint32 createId;

    Field *player_fields, *items_fields, *spells_fields, *skills_fields, *actions_fields;
    PlayerCreateInfo *pPlayerCreateInfo;

    QueryResult *player_result = sDatabase.PQuery("SELECT * FROM playercreateinfo WHERE race = '%u' AND class = '%u';", race, class_);
    
    if( !player_result ) 
        return NULL;

    pPlayerCreateInfo = new PlayerCreateInfo;

    player_fields = player_result->Fetch();

    pPlayerCreateInfo->createId = player_fields[0].GetUInt8();
    createId = (uint32)pPlayerCreateInfo->createId;
    pPlayerCreateInfo->race = player_fields[1].GetUInt8();
    pPlayerCreateInfo->class_ = player_fields[2].GetUInt8();
    pPlayerCreateInfo->mapId = player_fields[3].GetUInt32();
    pPlayerCreateInfo->zoneId = player_fields[4].GetUInt32();
    pPlayerCreateInfo->positionX = player_fields[5].GetFloat();
    pPlayerCreateInfo->positionY = player_fields[6].GetFloat();
    pPlayerCreateInfo->positionZ = player_fields[7].GetFloat();
    pPlayerCreateInfo->displayId = player_fields[8].GetUInt16();
    pPlayerCreateInfo->strength = player_fields[9].GetUInt8();
    pPlayerCreateInfo->ability = player_fields[10].GetUInt8();
    pPlayerCreateInfo->stamina = player_fields[11].GetUInt8();
    pPlayerCreateInfo->intellect = player_fields[12].GetUInt8();
    pPlayerCreateInfo->spirit = player_fields[13].GetUInt8();
    pPlayerCreateInfo->basearmor = player_fields[14].GetUInt32();
    pPlayerCreateInfo->health = player_fields[15].GetUInt32();
    pPlayerCreateInfo->mana = player_fields[16].GetUInt32();
    pPlayerCreateInfo->rage = player_fields[17].GetUInt32();
    pPlayerCreateInfo->focus = player_fields[18].GetUInt32();
    pPlayerCreateInfo->energy = player_fields[19].GetUInt32();
    pPlayerCreateInfo->attackpower = player_fields[20].GetUInt32();
    pPlayerCreateInfo->mindmg = player_fields[21].GetFloat();
    pPlayerCreateInfo->maxdmg = player_fields[22].GetFloat();
    pPlayerCreateInfo->ranmindmg = player_fields[23].GetFloat();
    pPlayerCreateInfo->ranmaxdmg = player_fields[24].GetFloat();

    delete player_result;

    QueryResult *items_result = sDatabase.PQuery("SELECT * FROM playercreateinfo_items WHERE createId = '0' OR createId = '%u';", createId);

    do {
		 if(!items_result) break;
		 items_fields = items_result->Fetch();
		 pPlayerCreateInfo->item_id.push_back(items_fields[1].GetUInt32());
		 pPlayerCreateInfo->item_bagIndex.push_back(items_fields[2].GetUInt32());
		 pPlayerCreateInfo->item_slot.push_back(items_fields[3].GetUInt8());
		 pPlayerCreateInfo->item_amount.push_back(items_fields[4].GetUInt32());
	 } while (items_result->NextRow());

    delete items_result;
    
    QueryResult *spells_result = sDatabase.PQuery("SELECT * FROM playercreateinfo_spells WHERE createId = '0' OR createId = '%u';", createId);

    do 
    {
        if(!spells_result) break;
        spells_fields = spells_result->Fetch();
        pPlayerCreateInfo->spell.push_back(spells_fields[1].GetUInt16());

    } while( spells_result->NextRow() );

    delete spells_result;
    
    QueryResult *skills_result = sDatabase.PQuery("SELECT * FROM playercreateinfo_skills WHERE createId = '0' OR createId = '%u';", createId);

    do 
    {
        if(!skills_result) break;
        skills_fields = skills_result->Fetch();
        pPlayerCreateInfo->skill[0].push_back(skills_fields[1].GetUInt16());
        pPlayerCreateInfo->skill[1].push_back(skills_fields[2].GetUInt16());
        pPlayerCreateInfo->skill[2].push_back(skills_fields[3].GetUInt16());

    } while( skills_result->NextRow() );

    delete skills_result;

    QueryResult *actions_result = sDatabase.PQuery("SELECT * FROM playercreateinfo_actions WHERE createId = '0' OR createId = '%u';", createId);

    do 
    {
        if(!actions_result) break;
        actions_fields = actions_result->Fetch();
        pPlayerCreateInfo->action[0].push_back(actions_fields[1].GetUInt16());
        pPlayerCreateInfo->action[1].push_back(actions_fields[2].GetUInt16());
        pPlayerCreateInfo->action[2].push_back(actions_fields[3].GetUInt16());
        pPlayerCreateInfo->action[3].push_back(actions_fields[4].GetUInt16());

    } while( actions_result->NextRow() );

    delete actions_result;

    return pPlayerCreateInfo; 
}

uint64 ObjectMgr::GetPlayerGUIDByName(const char *name) const
{

    uint64 guid = 0;

    QueryResult *result = sDatabase.PQuery("SELECT guid FROM characters WHERE name = '%s';", name);

    if(result)
    {
        guid = (*result)[0].GetUInt32();

        delete result;
    }

    return guid;
}


bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{

    QueryResult *result = sDatabase.PQuery("SELECT name FROM characters WHERE guid = '%d';", GUID_LOPART(guid));

    if(result)
    {
        name = (*result)[0].GetString();
        delete result;
        return true;
    }

    return false;
}


void ObjectMgr::LoadAuctions()
{
    QueryResult *result = sDatabase.PQuery( "SELECT * FROM auctionhouse;" );

    if( !result )
        return;

    AuctionEntry *aItem;

    do
    {
        Field *fields = result->Fetch();

        aItem = new AuctionEntry;
        aItem->auctioneer = fields[0].GetUInt32();
        aItem->item = fields[1].GetUInt32();
        aItem->owner = fields[2].GetUInt32();
        aItem->buyout = fields[3].GetUInt32();
        aItem->time = fields[4].GetUInt32();
        aItem->bidder = fields[5].GetUInt32();
        aItem->bid = fields[6].GetUInt32();
        aItem->Id = fields[7].GetUInt32();
        AddAuction(aItem);
    } while (result->NextRow());
    delete result;

}



void ObjectMgr::LoadItemPrototypes()
{
	sItemStorage.Load ();
 	sLog.outString( ">> Loaded %u item prototypes", sItemStorage.iNumRecords);
}

void ObjectMgr::LoadAuctionItems()
{
    QueryResult *result = sDatabase.PQuery( "SELECT * FROM auctioned_items;" );

    if( !result )
        return;
    Field *fields;
    do
    {
        fields = result->Fetch();
        Item* item = new Item;
        item->LoadFromDB(fields[0].GetUInt32(), 2);
        AddAItem(item);
    }
    while( result->NextRow() );

    delete result;
}


void ObjectMgr::LoadMailedItems()
{
    QueryResult *result = sDatabase.PQuery( "SELECT * FROM mailed_items;" );

    if( !result )
        return;
    Field *fields;
    do
    {
        fields = result->Fetch();
        Item* item = new Item;
        item->LoadFromDB(fields[0].GetUInt32(), 3);
        AddMItem(item);
    }
    while( result->NextRow() );

    delete result;
}

void ObjectMgr::LoadGuilds()
{
	Guild *newguild;
	QueryResult *result = sDatabase.PQuery( "SELECT guildId FROM guilds;" );
	uint32 count = 0;
	
	if( !result ) 
	{
		
		barGoLink bar( 1 );
		
		bar.step();

		sLog.outString( "" );
		sLog.outString( ">> Loaded %d guild definitions", count );
		return;
	}
	
	
	barGoLink bar( result->GetRowCount() );
	
	do
	{
		Field *fields = result->Fetch();
		
		
		bar.step();
		count++;
		
		newguild = new Guild;
		newguild->LoadGuildFromDB(fields[0].GetUInt32());
		AddGuild(newguild);
		
	}while( result->NextRow() );

	sLog.outString( "" );
	sLog.outString( ">> Loaded %d guild definitions", count );
}

void ObjectMgr::LoadQuests()
{
	QueryResult *result = sDatabase.PQuery( "SELECT * FROM quests;" );

    if( !result ) return;

	
	barGoLink bar( result->GetRowCount() );

    Quest *pQuest;
	uint32 count = 0;
	int iCalc;
	int CiC;

    do {
        Field *fields = result->Fetch();

		
		bar.step();

        pQuest = new Quest;
		iCalc = 0;

		pQuest->m_qId        = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qCategory  = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qFlags     = fields[ iCalc++ ].GetUInt32();

		pQuest->m_qTitle         = fields[ iCalc++ ].GetString();
        pQuest->m_qDetails       = fields[ iCalc++ ].GetString();
        pQuest->m_qObjectives    = fields[ iCalc++ ].GetString();

		pQuest->m_qCompletionInfo  = fields[ iCalc++ ].GetString();
		pQuest->m_qIncompleteInfo  = fields[ iCalc++ ].GetString();
		pQuest->m_qEndInfo         = fields[ iCalc++ ].GetString();

		for (CiC = 0; CiC < QUEST_OBJECTIVES_COUNT; CiC++)
			pQuest->m_qObjectiveInfo[CiC]    = fields[ iCalc++ ].GetString();

		pQuest->m_qPlayerLevel      = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qComplexityLevel  = fields[ iCalc++ ].GetUInt32();

		pQuest->m_qRequiredQuestsCount = fields[ iCalc++ ].GetUInt32();
		for ( CiC = 0; CiC < QUEST_DEPLINK_COUNT; CiC++)
		{ pQuest->m_qRequiredQuests[CiC] = fields[ iCalc++ ].GetUInt32(); }

        pQuest->m_qRequiredAbsQuestsCount = fields[ iCalc++ ].GetUInt32();
		for ( CiC = 0; CiC < QUEST_DEPLINK_COUNT; CiC++)
		{ pQuest->m_qRequiredAbsQuests[CiC] = fields[ iCalc++ ].GetUInt32(); }

		pQuest->m_qLockerQuestsCount = fields[ iCalc++ ].GetUInt32();
		for ( CiC = 0; CiC < QUEST_DEPLINK_COUNT; CiC++)
		{ pQuest->m_qLockerQuests[CiC] = fields[ iCalc++ ].GetUInt32(); }

		for (CiC = 0; CiC < QUEST_OBJECTIVES_COUNT; CiC++)
			pQuest->m_qObjItemId[CiC]    = fields[ iCalc++ ].GetUInt32();

		for (CiC = 0; CiC < QUEST_OBJECTIVES_COUNT; CiC++)
			pQuest->m_qObjItemCount[CiC]    = fields[ iCalc++ ].GetUInt32();

		for (CiC = 0; CiC < QUEST_OBJECTIVES_COUNT; CiC++)
			pQuest->m_qObjMobId[CiC]    = fields[ iCalc++ ].GetUInt32();

		for (CiC = 0; CiC < QUEST_OBJECTIVES_COUNT; CiC++)
			pQuest->m_qObjMobCount[CiC]    = fields[ iCalc++ ].GetUInt32();


		pQuest->m_qRewChoicesCount = fields[ iCalc++ ].GetUInt32();
		for ( CiC = 0; CiC < QUEST_REWARD_CHOICES_COUNT; CiC++)
		{ pQuest->m_qRewChoicesItemId[CiC] = fields[ iCalc++ ].GetUInt32(); }

		for ( CiC = 0; CiC < QUEST_REWARD_CHOICES_COUNT; CiC++)
		{ pQuest->m_qRewChoicesItemCount[CiC] = fields[ iCalc++ ].GetUInt32(); }

		pQuest->m_qRewCount = fields[ iCalc++ ].GetUInt32();
		for ( CiC = 0; CiC < QUEST_REWARDS_COUNT; CiC++)
		{ pQuest->m_qRewItemId[CiC] = fields[ iCalc++ ].GetUInt32(); }

		for ( CiC = 0; CiC < QUEST_REWARDS_COUNT; CiC++)
		{ pQuest->m_qRewItemCount[CiC] = fields[ iCalc++ ].GetUInt32(); }


		pQuest->m_qRewMoney         = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qObjRepFaction_1  = fields[ iCalc++ ].GetUInt32();
        pQuest->m_qObjRepFaction_2  = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qObjRepValue_1    = fields[ iCalc++ ].GetUInt32();
        pQuest->m_qObjRepValue_2    = fields[ iCalc++ ].GetUInt32();

		pQuest->m_qQuestItem     = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qNextQuestId   = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qRewSpell      = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qObjTime       = fields[ iCalc++ ].GetUInt32();

		pQuest->m_qType          = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qRequiredRaces = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qRequiredClass = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qRequiredTradeskill   = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qSpecialFlags  = fields[ iCalc++ ].GetUInt32();

		pQuest->m_qPointId		 = fields[ iCalc++ ].GetUInt32();
		pQuest->m_qPointX    	 = fields[ iCalc++ ].GetFloat();
		pQuest->m_qPointY		 = fields[ iCalc++ ].GetFloat();
		pQuest->m_qPointOpt		 = fields[ iCalc++ ].GetUInt32();

		count++;
        AddQuest(pQuest);
    }
    while( result->NextRow() );

    delete result;
	
	// points all quests for their next quest
	for( QuestMap::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        i->second->m_qNextQuest = objmgr.GetQuest( i->second->m_qNextQuestId );
    }
	

	sLog.outString( "" );
	sLog.outString( ">> Loaded %d quest definitions", count );
}



void ObjectMgr::AddGossipText(GossipText *pGText)
{
	ASSERT( pGText->Text_ID );
    ASSERT( mGossipText.find(pGText->Text_ID) == mGossipText.end() );
    mGossipText[pGText->Text_ID] = pGText;
}


GossipText *ObjectMgr::GetGossipText(uint32 Text_ID)
{
    GossipTextMap::const_iterator itr;
    for (itr = mGossipText.begin(); itr != mGossipText.end(); itr++)
    {
        if(itr->second->Text_ID == Text_ID)
            return itr->second;
    }
    return NULL;
}


void ObjectMgr::LoadGossipText()
{
	GossipText *pGText;
	QueryResult *result = sDatabase.PQuery( "SELECT * FROM npc_text;" );

	int count = 0;
	if( !result ) return;
	int cic;

	
	barGoLink bar( result->GetRowCount() );

	do {
		count++;
		cic = 0;

		Field *fields = result->Fetch();

		
		bar.step();

		pGText = new GossipText;
		pGText->Text_ID    = fields[cic++].GetUInt32();

		for (int i=0; i< 8; i++)
		{
			pGText->Options[i].Text_0           = fields[cic++].GetString();
			pGText->Options[i].Text_1           = fields[cic++].GetString();

			pGText->Options[i].Language         = fields[cic++].GetUInt32();
			pGText->Options[i].Probability      = fields[cic++].GetFloat();

			pGText->Options[i].Emotes[0]._Delay  = fields[cic++].GetUInt32();
			pGText->Options[i].Emotes[0]._Emote  = fields[cic++].GetUInt32();

			pGText->Options[i].Emotes[1]._Delay  = fields[cic++].GetUInt32();
			pGText->Options[i].Emotes[1]._Emote  = fields[cic++].GetUInt32();

			pGText->Options[i].Emotes[2]._Delay  = fields[cic++].GetUInt32();
			pGText->Options[i].Emotes[2]._Emote  = fields[cic++].GetUInt32();
		}
        
		if ( !pGText->Text_ID ) continue;
		AddGossipText( pGText );

	} while( result->NextRow() );

	sLog.outString( "" );
	sLog.outString( ">> Loaded %d npc texts", count );
	delete result;
}


ItemPage *ObjectMgr::RetreiveItemPageText(uint32 Page_ID)
{
	ItemPage *pIText;
	QueryResult *result = sDatabase.PQuery("SELECT * FROM item_pages WHERE ID = '%d';", Page_ID);

	if( !result ) return NULL;
	int cic, count = 0;
	pIText = new ItemPage;

	do {
		count++;
		cic = 0;

		Field *fields = result->Fetch();


		pIText->Page_ID    = fields[cic++].GetUInt32();

		pIText->PageText   = fields[cic++].GetString();
		pIText->Next_Page  = fields[cic++].GetUInt32();

		if ( !pIText->Page_ID ) break;
	} while( result->NextRow() );

	delete result;
	return pIText;
}


void ObjectMgr::AddAreaTriggerPoint(AreaTriggerPoint *pArea)
{
	ASSERT( pArea->Trigger_ID );
	ASSERT( mAreaTriggerMap.find(pArea->Trigger_ID) == mAreaTriggerMap.end() );

	mAreaTriggerMap[pArea->Trigger_ID] = pArea;
}


AreaTriggerPoint *ObjectMgr::GetAreaTriggerQuestPoint(uint32 Trigger_ID)
{
    AreaTriggerMap::const_iterator itr;
    for (itr = mAreaTriggerMap.begin(); itr != mAreaTriggerMap.end(); itr++)
    {
        if(itr->second->Trigger_ID == Trigger_ID)
            return itr->second;
    }
    return NULL;
}


void ObjectMgr::LoadAreaTriggerPoints()
{
	int count = 0;
	QueryResult *result = sDatabase.PQuery( "SELECT * FROM triggerquestrelation;" );
	AreaTriggerPoint *pArea;

	if( !result ) return;

	
	barGoLink bar( result->GetRowCount() );

	do {
		count++;

		
		bar.step();

		pArea = new AreaTriggerPoint;

		Field *fields = result->Fetch();

		
		
		pArea->Trigger_ID      = fields[0].GetUInt32();
		pArea->Quest_ID        = fields[1].GetUInt32();
		pArea->Creature_ID     = fields[2].GetUInt32();

		AddAreaTriggerPoint( pArea );

	} while( result->NextRow() );

	sLog.outString( "" );
	sLog.outString( ">> Loaded %d quest trigger points", count );
	delete result;
}



bool ObjectMgr::GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask )
{
    
    QueryResult *result = sDatabase.PQuery("SELECT taxipath.destination FROM taxipath WHERE taxipath.source = '%d' ORDER BY destination LIMIT 1;", curloc);

    if( ! result )
    {
        return 1;
    }
    Field *fields = result->Fetch();
    uint8 destination = fields[0].GetUInt8();
    uint8 field = (uint8)((destination - 1) / 32);
    Mask[field] |= 1 << ( (destination - 1 ) % 32 );

    return 1;
}


uint32 ObjectMgr::GetNearestTaxiNode( float x, float y, float z, uint32 mapid )
{
    
    QueryResult *result = sDatabase.PQuery("SELECT taxinodes.ID, SQRT(pow(taxinodes.x-'%f',2)+pow(taxinodes.y-'%f',2)+pow(taxinodes.z-'%f',2)) as distance FROM taxinodes WHERE taxinodes.continent = '%u' ORDER BY distance LIMIT 1;", x, y, z, mapid);

    if( ! result  )
    {
        return 0;
    }
    Field *fields = result->Fetch();
    return fields[0].GetUInt8();

    
}


void ObjectMgr::GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{
    
    QueryResult *result = sDatabase.PQuery("SELECT taxipath.price, taxipath.ID FROM taxipath WHERE taxipath.source = '%u' AND taxipath.destination = '%u';", source, destination);

    if( ! result )
    {
        path = 0;
        cost = 0;
        return;
    }
    Field *fields = result->Fetch();
    cost = fields[0].GetUInt32();
    path = fields[1].GetUInt16();
}


uint16 ObjectMgr::GetTaxiMount( uint32 id )
{

    QueryResult *result = sDatabase.PQuery("SELECT taxinodes.mount FROM taxinodes WHERE taxinodes.ID = '%u';", id);
    
    if( ! result )
    {
        return 0;
    }

    Field *fields = result->Fetch();
    return fields[0].GetUInt16();

    
}


void ObjectMgr::GetTaxiPathNodes( uint32 path, Path &pathnodes )
{

    QueryResult *result = sDatabase.PQuery("SELECT taxipathnodes.X, taxipathnodes.Y, taxipathnodes.Z FROM taxipathnodes WHERE taxipathnodes.path = '%u';", path);

    if( ! result )
	return;

    uint16 count = result->GetRowCount();
    sLog.outDebug(" ROW COUNT %u ",count);
    pathnodes.Resize( count );
    unsigned int i = 0;

    do
    {
        Field *fields = result->Fetch();
        pathnodes[ i ].x = fields[0].GetFloat();
        pathnodes[ i ].y = fields[1].GetFloat();
        pathnodes[ i ].z = fields[2].GetFloat();
        i++;
    } while( result->NextRow() );
}


GraveyardTeleport *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId)
{

    QueryResult *result = sDatabase.PQuery("SELECT SQRT(POW('%f'-X,2)+POW('%f'-Y,2)+POW('%f'-Z,2)) as distance,X,Y,Z,mapId from graveyards where mapId = '%d' ORDER BY distance ASC LIMIT 1;", x, y, z, MapId);

    if( ! result )
        return NULL;
    
    Field *fields = result->Fetch();
    GraveyardTeleport *pgrave = new GraveyardTeleport;
    
    pgrave->X = fields[1].GetFloat();
    pgrave->Y = fields[2].GetFloat();
    pgrave->Z = fields[3].GetFloat();
    pgrave->MapId = fields[4].GetUInt32();

    return pgrave;
}

AreaTrigger *ObjectMgr::GetAreaTrigger(uint32 Trigger_ID)
{

    QueryResult *result = sDatabase.PQuery("SELECT triggerID FROM areatrigger WHERE triggerID = '%d';", Trigger_ID);

    if ( result )
    {
        Field *fields = result->Fetch();
        uint32 totrigger = fields[0].GetUInt32();
        if( totrigger != 0)
        {

	    QueryResult *result1 = sDatabase.PQuery("SELECT TargetMapID,TargetPosX,TargetPosY,TargetPosZ FROM areatrigger WHERE triggerID = '%d';", totrigger);
            if ( result1 )
            {
                Field *fields1 = result1->Fetch();
                AreaTrigger *at = new AreaTrigger;

                at->mapId = fields1[0].GetUInt32();

                at->X = fields1[1].GetFloat();
                at->Y = fields1[2].GetFloat();
                at->Z = fields1[3].GetFloat();

                return at;
            }
        }
    }
    return NULL;
}

void ObjectMgr::LoadTeleportCoords()
{

    QueryResult *result = sDatabase.PQuery( "SELECT * FROM areatrigger;" );

    if( !result )
        return;

	uint32 count = 0;

    TeleportCoords *pTC;


    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

		count++;

        pTC = new TeleportCoords;
        pTC->id = fields[0].GetUInt32();
		//pTC->Name = fields[6].GetString();
        pTC->mapId = fields[5].GetUInt32();
        pTC->x = fields[1].GetFloat();
        pTC->y = fields[2].GetFloat();
        pTC->z = fields[3].GetFloat();

        AddTeleportCoords(pTC);

    } while( result->NextRow() );

    delete result;

	sLog.outString( "" );
	sLog.outString( ">> Loaded %d teleport definitions", count );
}


void ObjectMgr::SetHighestGuids()
{
 
    QueryResult *result = sDatabase.Query( "SELECT MAX(guid) FROM characters;" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(guid) FROM creatures;" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(guid) FROM item_instances;" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    

    result = sDatabase.Query("SELECT MAX(guid) FROM gameobjects;" );
    if( result )
    {
        m_hiGoGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query("SELECT MAX(id) FROM auctionhouse;" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_auctionid = 0;
    }
    result = sDatabase.PQuery( "SELECT MAX(mailid) FROM mail;" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_mailid = 0;
    }
    
    result = sDatabase.PQuery( "SELECT MAX(guid) FROM corpses;" );
    if( result )
    {
        m_hiCorpseGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }
	
}


uint32 ObjectMgr::GenerateAuctionID()
{
 
    return ++m_auctionid;
}


uint32 ObjectMgr::GenerateMailID()
{
    return ++m_mailid;
}


uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
   

    switch(guidhigh)
    {
        case HIGHGUID_ITEM          : return ++m_hiItemGuid;
		case HIGHGUID_UNIT          : return ++m_hiCreatureGuid;
        case HIGHGUID_PLAYER        : return ++m_hiCharGuid;
        case HIGHGUID_GAMEOBJECT    : return ++m_hiGoGuid;
        case HIGHGUID_CORPSE        : return ++m_hiCorpseGuid;
        case HIGHGUID_DYNAMICOBJECT : return ++m_hiDoGuid;
        default                     : ASSERT(0);
    }

	return 0;    
}





GameObjectInfo *ObjectMgr::GetGameObjectInfo(uint32 id)
{
	//debug
	if(sGOStorage.iNumRecords<=id)
	{
		sLog.outString("ERROR: There is no GO with proto %u id the DB",id);
		return NULL;
	}

	return (sGOStorage.iNumRecords<=id)?NULL:(GameObjectInfo *)sGOStorage.pIndex[id];

}

void ObjectMgr::LoadGameobjectInfo()
{
	sGOStorage.Load();

	sLog.outString( ">> Loaded %d game object templates", sGOStorage.iNumRecords );
	
}



ItemPrototype* ObjectMgr::GetItemPrototype(uint32 id)
{
	return (sItemStorage.iNumRecords<=id)?NULL:(ItemPrototype*)sItemStorage.pIndex[id];
}
