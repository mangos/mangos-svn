/* ObjectMgr.cpp
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
#include "Log.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "World.h"
#include "WorldSession.h"
#include "Group.h"
#include "ProgressBar.cpp"

createFileSingleton( ObjectMgr );

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid = 1;
    m_hiCreatureGuid = 1;
    m_hiItemGuid = 1;
    m_hiGoGuid = 1;
    m_hiDoGuid = 1;
    m_hiNameGuid = 1;
}


ObjectMgr::~ObjectMgr()
{

    for( CreatureNameMap::iterator i = mCreatureNames.begin( ); i != mCreatureNames.end( ); ++ i )
    {
        delete i->second;
    }
    mCreatureNames.clear();

#ifndef ENABLE_GRID_SYSTEM
    for( CreatureMap::iterator i = mCreatures.begin( ); i != mCreatures.end( ); ++ i )
    {
        delete i->second;
    }
    mCreatures.clear( );

    for( GameObjectMap::iterator i = mGameObjects.begin( ); i != mGameObjects.end( ); ++ i )
    {
        delete i->second;
    }
    mGameObjects.clear( );

    for( DynamicObjectMap::iterator i = mDynamicObjects.begin( ); i != mDynamicObjects.end( ); ++ i )
    {
        delete i->second;
    }
    mDynamicObjects.clear( );

    for( CorpseMap::iterator i = mCorpses.begin( ); i != mCorpses.end( ); ++ i )
    {
        delete i->second;
    }
    mCorpses.clear( );
#endif

    for( QuestMap::iterator i = mQuests.begin( ); i != mQuests.end( ); ++ i )
    {
        delete i->second;
    }
    mQuests.clear( );

    for( ItemPrototypeMap::iterator i = mItemPrototypes.begin( ); i != mItemPrototypes.end( ); ++ i )
    {
        delete i->second;
    }
    mItemPrototypes.clear( );

    for( TrainerspellMap::iterator i = mTrainerspells.begin( ); i != mTrainerspells.end( ); ++ i )
    {
        delete i->second;
    }
    mTrainerspells.clear( );

   /* for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );*/

    for( GameObjectInfoMap::iterator iter = mGameObjectInfo.begin(); iter != mGameObjectInfo.end(); ++iter)
    {
		delete iter->second;
    }
    mGameObjectInfo.clear();

  /*  GossipNpcMap::iterator iter, end;
    for(int a=0; a < MAX_CONTINENTS; a++)
    {
        for( iter = GetGossipListBegin(a), end = GetGossipListEnd(a); iter != end; iter++ )
        {
            delete [] iter->second->pOptions;
            delete iter->second;
        }
        mGossipNpc[a].clear( );
    }*/

   /* for( GraveyardMap::iterator i = mGraveyards.begin( ); i != mGraveyards.end( ); ++ i )
    {
        delete i->second;
    }
    mGraveyards.clear( );*/
}


//
// Groups
//

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


//
// Creature names
//
CreatureInfo *ObjectMgr::GetCreatureName(uint32 id)
{
    CreatureNameMap::const_iterator itr = mCreatureNames.find( id );
    if( itr != mCreatureNames.end( ) )
        return itr->second;

    // returning unknown creature if no data found

	//Log::getSingleton( ).outDetail("Creature: %u is an Unknown Being!\n", id);
    CreatureInfo *ci=new CreatureInfo;
    ci->Name = "Unknown Being";
    ci->Id=id;
    ci->SubName = "";
    ci->Type = 0;
    ci->unknown1 = 0;
    ci->unknown2 = 0;
    ci->unknown3 = 0;
    ci->unknown4 = 0;
    ci->DisplayID = 0;
    AddCreatureName(ci);
    return ci;
}


uint32 ObjectMgr::AddCreatureName(const char* name)
{
    for( CreatureNameMap::iterator i = mCreatureNames.begin( );
        i != mCreatureNames.end( ); ++ i )
    {
        if (strcmp(i->second->Name.c_str(),name) == 0)
            return i->second->Id;
    }

    uint32 id = m_hiNameGuid++;
    AddCreatureName(id, name);

    std::stringstream ss;
    ss << "INSERT INTO creaturetemplate (entryid,name) VALUES (" << id << ", '" << name << "')";
    sDatabase.Execute( ss.str( ).c_str( ) );

    return id;
}


uint32 ObjectMgr::AddCreatureName(const char* name, uint32 displayid)
{
    for( CreatureNameMap::iterator i = mCreatureNames.begin( );
        i != mCreatureNames.end( ); ++ i )
    {
        if (strcmp(i->second->Name.c_str(),name) == 0)
            return i->second->Id;
    }

    uint32 id = m_hiNameGuid++;
    AddCreatureName(id, name, displayid);

    std::stringstream ss;
    ss << "INSERT INTO creaturetemplate (entryid,name,modelid) VALUES (" << id << ", '" << name << "', '" << displayid << "')";
    sDatabase.Execute( ss.str( ).c_str( ) );

    return id;
}


uint32 ObjectMgr::AddCreatureSubName(const char* name, const char* subname, uint32 displayid)
{
    for( CreatureNameMap::iterator i = mCreatureNames.begin( );
        i != mCreatureNames.end( ); ++ i )
    {
        if (strcmp(i->second->Name.c_str(),name) == 0)
            if (i->second->DisplayID == displayid)
                if (strcmp(i->second->SubName.c_str(),subname) == 0)
                    return i->second->Id;
    }

    uint32 id = m_hiNameGuid++;

    CreatureInfo *cInfo = new CreatureInfo;
    cInfo->DisplayID = displayid;
    cInfo->Id = id;
    cInfo->Name = name;
    cInfo->SubName = subname;
    cInfo->Type = 0;
    cInfo->unknown1 = 0;
    cInfo->unknown2 = 0;
    cInfo->unknown3 = 0;
    cInfo->unknown4 = 0;
    AddCreatureName(cInfo);

    std::stringstream ss;
    ss << "INSERT INTO creaturetemplate (entryid,name,subname,modelid) VALUES (" << id << ", '" << name;
    ss << "', '" << subname << "', '" << displayid << "')";
    sDatabase.Execute( ss.str( ).c_str( ) );

    return id;
}


void ObjectMgr::AddCreatureName(CreatureInfo *cinfo)
{
    ASSERT( mCreatureNames.find(cinfo->Id) == mCreatureNames.end() );
    // verifying if info for that creature already exists(need some cleaning here some time)
    CreatureNameMap::iterator itr = mCreatureNames.find( cinfo->Id );
    // if found we delete the old creature info
    if( itr != mCreatureNames.end( ) )
        mCreatureNames.erase(itr);
    mCreatureNames[cinfo->Id] = cinfo;
}


void ObjectMgr::AddCreatureName(uint32 id, const char* name)
{
    CreatureInfo *cinfo;
    cinfo = new CreatureInfo;
    cinfo->Id = id;
    cinfo->Name = name;
    cinfo->SubName = "";
    cinfo->Type = 0;
    cinfo->unknown1 = 0;
    cinfo->unknown2 = 0;
    cinfo->unknown3 = 0;
    cinfo->unknown4 = 0;
    cinfo->DisplayID = 0;

    ASSERT( name );
    ASSERT( mCreatureNames.find(id) == mCreatureNames.end() );
    mCreatureNames[id] = cinfo;
}


void ObjectMgr::AddCreatureName(uint32 id, const char* name, uint32 displayid)
{
    CreatureInfo *cinfo;
    cinfo = new CreatureInfo;
    cinfo->Id = id;
    cinfo->Name = name;
    cinfo->SubName = "";
    cinfo->Type = 0;
    cinfo->unknown1 = 0;
    cinfo->unknown2 = 0;
    cinfo->unknown3 = 0;
    cinfo->unknown4 = 0;
    cinfo->DisplayID = displayid;

    ASSERT( name );
    ASSERT( mCreatureNames.find(id) == mCreatureNames.end() );
    mCreatureNames[id] = cinfo;
}


void ObjectMgr::LoadCreatureNames()
{
    CreatureInfo *cn;
    QueryResult *result = sDatabase.Query( "SELECT entryid,name,IFNULL(subname,''),type,modelid FROM creaturetemplate" );
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            cn = new CreatureInfo;
            cn->Id = fields[0].GetUInt32();
            cn->Name = fields[1].GetString();
            cn->SubName = fields[2].GetString();
            cn->Type = fields[3].GetUInt32();
            cn->DisplayID = fields[4].GetUInt32();

	    // Adding unknowns here later
			cn->unknown1 = 0; 
            cn->unknown2 = 0; 
            cn->unknown3 = 0; 
            cn->unknown4 = 0; 

            AddCreatureName( cn );
        } while( result->NextRow() );

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(entryid) FROM creaturetemplate" );
    if(result)
    {
        m_hiNameGuid = (*result)[0].GetUInt32();

        delete result;
    }
}

PlayerCreateInfo* ObjectMgr::GetPlayerCreateInfo(uint32 race, uint32 class_)
{
	uint32 createId;

	QueryResult *player_result, *items_result, *spells_result, *skills_result,*actions_result ;
    Field *player_fields, *items_fields, *spells_fields, *skills_fields, *actions_fields;
	PlayerCreateInfo *pPlayerCreateInfo;

	std::stringstream player_query,item_query,spell_query,skill_query,action_query;

	player_query << "SELECT * FROM playercreateinfo WHERE race = " << race << " AND class = " << class_;
	player_result = sDatabase.Query( player_query.str().c_str() );
    
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
    pPlayerCreateInfo->health = player_fields[14].GetUInt32();
    pPlayerCreateInfo->mana = player_fields[15].GetUInt32();
    pPlayerCreateInfo->rage = player_fields[16].GetUInt32();
    pPlayerCreateInfo->focus = player_fields[17].GetUInt32();
    pPlayerCreateInfo->energy = player_fields[18].GetUInt32();
    pPlayerCreateInfo->attackpower = player_fields[19].GetUInt32();
    pPlayerCreateInfo->mindmg = player_fields[20].GetFloat();
    pPlayerCreateInfo->maxdmg = player_fields[21].GetFloat();

	delete player_result;


	item_query << "SELECT * FROM playercreateinfo_items WHERE createId = 0 OR createId = " << createId ;
	items_result = sDatabase.Query( item_query.str().c_str() );
	do 
    {
		if(!items_result) break;
		items_fields = items_result->Fetch();
		pPlayerCreateInfo->item.push_back(items_fields[1].GetUInt32());
		pPlayerCreateInfo->item_slot.push_back(items_fields[2].GetUInt8());

    } while( items_result->NextRow() );

	delete items_result;
	
	spell_query << "SELECT * FROM playercreateinfo_spells WHERE createId = 0 OR createId = " << createId ;
	spells_result = sDatabase.Query( spell_query.str().c_str() );
	do 
    {
		if(!spells_result) break;
		spells_fields = spells_result->Fetch();
		pPlayerCreateInfo->spell.push_back(spells_fields[1].GetUInt16());

    } while( spells_result->NextRow() );

	delete spells_result;
	
	skill_query << "SELECT * FROM playercreateinfo_skills WHERE createId = 0 OR createId = " << createId ;
	skills_result = sDatabase.Query( skill_query.str().c_str() );
    do 
    {
		if(!skills_result) break;
		skills_fields = skills_result->Fetch();
		pPlayerCreateInfo->skill[0].push_back(skills_fields[1].GetUInt16());
		pPlayerCreateInfo->skill[1].push_back(skills_fields[2].GetUInt16());
		pPlayerCreateInfo->skill[2].push_back(skills_fields[3].GetUInt16());

    } while( skills_result->NextRow() );

	delete skills_result;

	action_query << "SELECT * FROM playercreateinfo_actions WHERE createId = 0 OR createId = " << createId ;
	actions_result = sDatabase.Query( action_query.str().c_str() );
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
    std::stringstream query;
    query << "SELECT guid FROM characters WHERE name = '" << name << "'";

    QueryResult *result = sDatabase.Query( query.str().c_str() );
    if(result)
    {
        guid = (*result)[0].GetUInt32();

        delete result;
    }

    return guid;
}


bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{
    std::stringstream query;
    query << "SELECT name FROM characters WHERE guid=" << GUID_LOPART(guid);

    QueryResult *result = sDatabase.Query( query.str().c_str() );
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
    QueryResult *result = sDatabase.Query( "SELECT * FROM auctionhouse" );

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

int num_item_prototypes = 0;
uint32 item_proto_ids[64550];

void ObjectMgr::LoadItemPrototypes()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM items" );

    if( !result )
        return;

/// get number of rows for items
    barGoLink bar( result->GetRowCount() );

    ItemPrototype *pItemPrototype;
    int i;

	memset(&item_proto_ids,-1,sizeof(item_proto_ids));

    if( result->GetFieldCount() < 113 )
    {
        Log::getSingleton().outError("DB: Items table has incorrect number of fields");
        delete result;
        return;
    }

    do
    {
        Field *fields = result->Fetch();
/// print bar step
        bar.step();

        if( !fields[0].GetUInt32() )
        {
            Log::getSingleton().outBasic("DB: Incorrect item id found");
            continue;
        }

        pItemPrototype = new ItemPrototype;

        pItemPrototype->ItemId = fields[0].GetUInt32();

		// For random selection loots...
		item_proto_ids[num_item_prototypes] = fields[0].GetUInt32();
		//Log::getSingleton( ).outDebug( "Loot item ID: %u added.", item_proto_ids[num_item_prototypes]);
		num_item_prototypes++;

        pItemPrototype->Class = fields[2].GetUInt32();
        pItemPrototype->SubClass = fields[3].GetUInt32();
        pItemPrototype->Name1 = fields[4].GetString();
        pItemPrototype->Name2 = fields[5].GetString();
        pItemPrototype->Name3 = fields[6].GetString();
        pItemPrototype->Name4 = fields[7].GetString();
        pItemPrototype->DisplayInfoID = fields[8].GetUInt32();
        pItemPrototype->Quality = fields[9].GetUInt32();
        pItemPrototype->Flags = fields[10].GetUInt32();
        pItemPrototype->BuyPrice = fields[11].GetUInt32();
        pItemPrototype->SellPrice = fields[12].GetUInt32();
        pItemPrototype->InventoryType = fields[13].GetUInt32();
        pItemPrototype->AllowableClass = fields[14].GetUInt32();
        pItemPrototype->AllowableRace = fields[15].GetUInt32();
        pItemPrototype->ItemLevel = fields[16].GetUInt32();
        pItemPrototype->RequiredLevel = fields[17].GetUInt32();
        pItemPrototype->RequiredSkill = fields[18].GetUInt32();
        pItemPrototype->RequiredSkillRank = fields[19].GetUInt32();
        pItemPrototype->Field20 = fields[20].GetUInt32();
        pItemPrototype->Field21 = fields[21].GetUInt32();
        pItemPrototype->Field22 = fields[22].GetUInt32();
        pItemPrototype->Field23 = fields[23].GetUInt32();
        pItemPrototype->MaxCount = fields[24].GetUInt32();
        pItemPrototype->ContainerSlots = fields[25].GetUInt32();
        for(i = 0; i < 20; i+=2)
        {
            pItemPrototype->ItemStatType[i/2] = fields[26 + i].GetUInt32();
            pItemPrototype->ItemStatValue[i/2] = fields[27 + i].GetUInt32();
        }
        for(i = 0; i < 15; i+=3)
        {
            // Stupid items.sql
            int *a=(int *)malloc(sizeof(int)); *a=fields[46 + i].GetUInt32();
            int *b=(int *)malloc(sizeof(int)); *b=fields[47 + i].GetUInt32();

            pItemPrototype->DamageMin[i/3] = *(float *)a;
            pItemPrototype->DamageMax[i/3] = *(float *)b;
            // pItemPrototype->DamageMin[i/3] = fields[46 + +i].GetFloat();
            // pItemPrototype->DamageMax[i/3] = fields[47 +i].GetFloat();
            pItemPrototype->DamageType[i/3] = fields[48 + i].GetUInt32();

            free(a);free(b);
        }
        pItemPrototype->Armor = fields[61].GetUInt32();
        pItemPrototype->HolyRes = fields[62].GetUInt32();
        pItemPrototype->FireRes = fields[63].GetUInt32();
        pItemPrototype->NatureRes = fields[64].GetUInt32();
        pItemPrototype->FrostRes = fields[65].GetUInt32();
        pItemPrototype->ShadowRes = fields[66].GetUInt32();
        pItemPrototype->ArcaneRes = fields[67].GetUInt32();
        pItemPrototype->Delay = fields[68].GetUInt32();
        pItemPrototype->Field69 = fields[69].GetUInt32();
        for(i = 0; i < 30; i+=6)
        {
            pItemPrototype->SpellId[i/6] = fields[70+i].GetUInt32();
            pItemPrototype->SpellTrigger[i/6] = fields[71+i].GetUInt32();
            pItemPrototype->SpellCharges[i/6] = fields[72+i].GetUInt32();
            pItemPrototype->SpellCooldown[i/6] = fields[73+i].GetUInt32();
            pItemPrototype->SpellCategory[i/6] = fields[74+i].GetUInt32();
            pItemPrototype->SpellCategoryCooldown[i/6] = fields[75+i].GetUInt32();
        }
        pItemPrototype->Bonding = fields[100].GetUInt32();
        pItemPrototype->Description = fields[101].GetString();
        pItemPrototype->Field102 = fields[102].GetUInt32();
        pItemPrototype->Field103 = fields[103].GetUInt32();
        pItemPrototype->Field104 = fields[104].GetUInt32();
        pItemPrototype->Field105 = fields[105].GetUInt32();
        pItemPrototype->Field106 = fields[106].GetUInt32();
        pItemPrototype->Field107 = fields[107].GetUInt32();
        pItemPrototype->Field108 = fields[108].GetUInt32();
        pItemPrototype->Field109 = fields[109].GetUInt32();
        pItemPrototype->Field110 = fields[110].GetUInt32();
        pItemPrototype->Field111 = fields[111].GetUInt32();
        pItemPrototype->MaxDurability = fields[112].GetUInt32();

        AddItemPrototype(pItemPrototype);
    } while( result->NextRow() );

    delete result;
}


void ObjectMgr::LoadTrainerSpells()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM trainer" );

    if( !result )
        return;

    Trainerspell *TrainSpell;

/// get number of rows for trainers
    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();
/// print bar step
        bar.step();

        TrainSpell = new Trainerspell;

        TrainSpell->Id = fields[0].GetUInt32();
        TrainSpell->skilline1 = fields[1].GetUInt32();
        TrainSpell->skilline2 = fields[2].GetUInt32();
        TrainSpell->skilline3 = fields[3].GetUInt32();
		TrainSpell->skilline4 = fields[4].GetUInt32();
        TrainSpell->skilline5 = fields[5].GetUInt32();
        TrainSpell->skilline6 = fields[6].GetUInt32();
		TrainSpell->skilline7 = fields[7].GetUInt32();
        TrainSpell->skilline8 = fields[8].GetUInt32();
        TrainSpell->skilline9 = fields[9].GetUInt32();
		TrainSpell->skilline10 = fields[10].GetUInt32();
        TrainSpell->skilline11 = fields[11].GetUInt32();
        TrainSpell->skilline12 = fields[12].GetUInt32();
		TrainSpell->skilline13 = fields[13].GetUInt32();
        TrainSpell->skilline14 = fields[14].GetUInt32();
        TrainSpell->skilline15 = fields[15].GetUInt32();
		TrainSpell->skilline16 = fields[16].GetUInt32();
        TrainSpell->skilline17 = fields[17].GetUInt32();
        TrainSpell->skilline18 = fields[18].GetUInt32();
		TrainSpell->skilline19 = fields[19].GetUInt32();
        TrainSpell->skilline20 = fields[20].GetUInt32();
        TrainSpell->maxlvl = fields[21].GetUInt32();
        TrainSpell->charclass = fields[22].GetUInt32();
        AddTrainerspell(TrainSpell);
    } while (result->NextRow());
    delete result;

}


void ObjectMgr::LoadAuctionItems()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM auctioned_items" );

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
    QueryResult *result = sDatabase.Query( "SELECT * FROM mailed_items" );

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


void ObjectMgr::LoadQuests()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM quests" );

    if( !result )
        return;

    Quest *pQuest;
/// get number of rows for quests
    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();
/// print bar step
        bar.step();

        pQuest = new Quest;

        pQuest->m_questId = fields[0].GetUInt32();
        pQuest->m_zone = fields[1].GetUInt32();
        pQuest->m_title = fields[2].GetString();
        pQuest->m_details = fields[3].GetString();
        pQuest->m_objectives = fields[4].GetString();
        pQuest->m_completedText = fields[5].GetString();
        pQuest->m_incompleteText = fields[6].GetString();
        GUID_LOPART(pQuest->m_targetGuid) = fields[7].GetUInt32();
        // HACK!
        GUID_HIPART(pQuest->m_targetGuid) = 0xF0001000;
        pQuest->m_questItemId[0] = fields[8].GetUInt32();
        pQuest->m_questItemId[1] = fields[9].GetUInt32();
        pQuest->m_questItemId[2] = fields[10].GetUInt32();
        pQuest->m_questItemId[3] = fields[11].GetUInt32();
        pQuest->m_questItemCount[0] = fields[12].GetUInt32();
        pQuest->m_questItemCount[1] = fields[13].GetUInt32();
        pQuest->m_questItemCount[2] = fields[14].GetUInt32();
        pQuest->m_questItemCount[3] = fields[15].GetUInt32();
        pQuest->m_questMobId[0] = fields[16].GetUInt32();
        pQuest->m_questMobId[1] = fields[17].GetUInt32();
        pQuest->m_questMobId[2] = fields[18].GetUInt32();
        pQuest->m_questMobId[3] = fields[19].GetUInt32();
        pQuest->m_questMobCount[0] = fields[20].GetUInt32();
        pQuest->m_questMobCount[1] = fields[21].GetUInt32();
        pQuest->m_questMobCount[2] = fields[22].GetUInt32();
        pQuest->m_questMobCount[3] = fields[23].GetUInt32();
        pQuest->m_choiceRewards = fields[24].GetUInt16();
        pQuest->m_choiceItemId[0] = fields[25].GetUInt32();
        pQuest->m_choiceItemId[1] = fields[26].GetUInt32();
        pQuest->m_choiceItemId[2] = fields[27].GetUInt32();
        pQuest->m_choiceItemId[3] = fields[28].GetUInt32();
        pQuest->m_choiceItemId[4] = fields[29].GetUInt32();
        pQuest->m_choiceItemCount[0] = fields[30].GetUInt32();
        pQuest->m_choiceItemCount[1] = fields[31].GetUInt32();
        pQuest->m_choiceItemCount[2] = fields[32].GetUInt32();
        pQuest->m_choiceItemCount[3] = fields[33].GetUInt32();
        pQuest->m_choiceItemCount[4] = fields[34].GetUInt32();
        pQuest->m_itemRewards = fields[35].GetUInt16();
        pQuest->m_rewardItemId[0] = fields[36].GetUInt32();
        pQuest->m_rewardItemId[1] = fields[37].GetUInt32();
        pQuest->m_rewardItemId[2] = fields[38].GetUInt32();
        pQuest->m_rewardItemId[3] = fields[39].GetUInt32();
        pQuest->m_rewardItemId[4] = fields[40].GetUInt32();
        pQuest->m_rewardItemCount[0] = fields[41].GetUInt32();
        pQuest->m_rewardItemCount[1] = fields[42].GetUInt32();
        pQuest->m_rewardItemCount[2] = fields[43].GetUInt32();
        pQuest->m_rewardItemCount[3] = fields[44].GetUInt32();
        pQuest->m_rewardItemCount[4] = fields[45].GetUInt32();
        pQuest->m_rewardGold = fields[46].GetUInt32();
        pQuest->m_questXp = fields[47].GetUInt32();
        pQuest->m_originalGuid = fields[48].GetUInt32();
        GUID_LOPART(pQuest->m_originalGuid) = fields[48].GetUInt32();
        // HACK!
        GUID_HIPART(pQuest->m_originalGuid) = 0xF0001000;
        pQuest->m_requiredLevel = fields[49].GetUInt32();
        pQuest->m_previousQuest = fields[50].GetUInt32();

        AddQuest(pQuest);

    }
    while( result->NextRow() );

    delete result;
}

#ifndef ENABLE_GRID_SYSTEM
void ObjectMgr::LoadCreatures()
{
    QueryResult *result = sDatabase.Query( "SELECT id FROM creatures" );

    if( !result )
    {
        // log no creatures error
        return;
    }

    Creature* unit;
    Field *fields;
/// get number of rows for creatures
    barGoLink bar( result->GetRowCount() );

    do
    {
        fields = result->Fetch();

/// print bar step
        bar.step();

        unit = new Creature;
        ASSERT(unit);

        unit->LoadFromDB(fields[0].GetUInt32());
        AddObject(unit);
    }
    while( result->NextRow() );

    delete result;
}


void ObjectMgr::LoadGameObjects()
{

    // load game object info...
    QueryResult *result = sDatabase.Query( "SELECT id,type,displayId,faction,flags,sound0,sound1,sound2,sound3,sound4,sound5,sound6,sound7,sound8,sound9,size,name FROM gameobjecttemplate" );

    if( !result )
    {
        // log no object error
        return;
    }

    Field *fields;
/// get number of rows for items
    barGoLink bar( result->GetRowCount() );

    do
    {
        fields = result->Fetch();
/// print bar step
        bar.step();
	uint32 id = fields[0].GetUInt32();
	GameObjectInfo *info = new GameObjectInfo(id, fields[1].GetUInt32(),fields[2].GetUInt32(),fields[3].GetUInt32(), 
						  fields[4].GetUInt32(),
						  fields[5].GetUInt32(),fields[6].GetUInt32(),fields[7].GetUInt32(),fields[8].GetUInt32(),
						  fields[9].GetUInt32(),fields[10].GetUInt32(),fields[11].GetUInt32(),
						  fields[12].GetUInt32(),
						  fields[13].GetUInt32(), fields[14].GetUInt32(),
                                                  fields[15].GetFloat(),
						  fields[16].GetString());
	mGameObjectInfo[id] = info;
    }
    while( result->NextRow() );

    delete result;

    // Load game objects
    result = sDatabase.Query("SELECT id FROM gameobjects");

    if( result )
    {
	do
	{
	    fields = result->Fetch();
	    GameObject *go = new GameObject();
	    ASSERT(go);
	    
	    go->LoadFromDB(fields[0].GetUInt32());
	    AddObject(go);
	}while( result->NextRow() );

	delete result;
    }
}
// end of ENABLE_GRID_SYSTEM
#endif

/*
void ObjectMgr::LoadTaxiNodes()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM taxinodes" );

    if( !result )
        return;

    TaxiNodes *pTaxiNodes;

    do
    {
        Field *fields = result->Fetch();

        pTaxiNodes = new TaxiNodes;
        pTaxiNodes->id = fields[0].GetUInt8();
        pTaxiNodes->continent = fields[1].GetUInt8();
        pTaxiNodes->x = fields[2].GetFloat();
        pTaxiNodes->y = fields[3].GetFloat();
        pTaxiNodes->z = fields[4].GetFloat();
        // pTaxiNodes->name = fields[5].GetString();
        // pTaxiNodes->flags = fields[6].GetUInt32();
        pTaxiNodes->mount = fields[7].GetUInt16();

        AddTaxiNodes(pTaxiNodes);

    } while( result->NextRow() );

    delete result;
}*/

/*
void ObjectMgr::LoadTaxiPath()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM taxipath" );

    if( !result )
        return;

    TaxiPath *pTaxiPath;

    do
    {
        Field *fields = result->Fetch();

        pTaxiPath = new TaxiPath;
        pTaxiPath->id = fields[0].GetUInt16();
        pTaxiPath->source = fields[1].GetUInt8();
        pTaxiPath->destination = fields[2].GetUInt8();
        pTaxiPath->price = fields[3].GetUInt32();

        AddTaxiPath(pTaxiPath);

    } while( result->NextRow() );

    delete result;
}*/

/*
void ObjectMgr::LoadTaxiPathNodes()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM taxipathnodes ORDER BY 'index'" );

    if( !result )
        return;

    TaxiPathNodes *pTaxiPathNodes;

    do
    {
        Field *fields = result->Fetch();

        pTaxiPathNodes = new TaxiPathNodes;
        pTaxiPathNodes->id = fields[0].GetUInt16();
        pTaxiPathNodes->path = fields[1].GetUInt16();
        pTaxiPathNodes->index = fields[2].GetUInt8();
        pTaxiPathNodes->continent = fields[3].GetUInt8();
        pTaxiPathNodes->x = fields[4].GetFloat();
        pTaxiPathNodes->y = fields[5].GetFloat();
        pTaxiPathNodes->z = fields[6].GetFloat();
        // pTaxiPathNodes->unkown1 = fields[7].GetUInt32();
        // pTaxiPathNodes->unkown2 = fields[8].GetUInt32();

        AddTaxiPathNodes(pTaxiPathNodes);

    } while( result->NextRow() );

    delete result;
}*/


bool ObjectMgr::GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask )
{
    /*TaxiPathMap::const_iterator itr;
    TaxiPathNodesVec::iterator pathnodes_itr;
    uint8 field;

    for (itr = mTaxiPath.begin(); itr != mTaxiPath.end(); itr++)
    {
        if( itr->second->source == curloc )
        {
            for( pathnodes_itr = vTaxiPathNodes.begin();
                pathnodes_itr != vTaxiPathNodes.end(); ++pathnodes_itr )
            {
                if ((*pathnodes_itr)->path == itr->second->id)
                {
                    field = (uint8)((itr->second->destination - 1) / 32);
                    Mask[field] |= 1 << ( (itr->second->destination - 1 ) % 32 );
                    break;
                }

            }
        }
    }*/
	std::stringstream query;
	query << "SELECT taxipath.destination FROM taxipath WHERE taxipath.source = " << curloc << " ORDER BY destination LIMIT 1";
	Log::getSingleton( ).outDebug(" STRING %s ",query.str().c_str());

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

	if( result.get() == NULL )
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
	/*uint32 nearest = 0;
	float distance = -1;
	float nx, ny, nz, nd;*/

	std::stringstream query;

	query << "SELECT taxinodes.ID, SQRT(pow(taxinodes.x-(" << x << "),2)+pow(taxinodes.y-(" << y << "),2)+pow(taxinodes.z-(" << z << "),2)) as distance ";
	query << "FROM taxinodes WHERE taxinodes.continent = " << mapid << " ORDER BY distance LIMIT 1";

	//Log::getSingleton( ).outDebug(" STRING %s ",query.str().c_str());

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

    if( result.get() == NULL )
	{
        return 0;
	}
	Field *fields = result->Fetch();
	return fields[0].GetUInt8();

	/*TaxiNodesMap::const_iterator nodes_itr;
    TaxiPathMap::const_iterator path_itr;
    TaxiPathNodesVec::iterator pathnodes_itr;

    for (nodes_itr = mTaxiNodes.begin(); nodes_itr != mTaxiNodes.end(); nodes_itr++)
    {
        if( nodes_itr->second->continent == mapid )
        {
            nx = nodes_itr->second->x - x;
            ny = nodes_itr->second->y - y;
            nz = nodes_itr->second->z - z;
            nd = nx * nx + ny * ny + nz * nz;
            if( nd < distance || distance < 0 )
            {
                for (path_itr = mTaxiPath.begin();
                    path_itr != mTaxiPath.end();
                    ++path_itr)
                {
                    if( path_itr->second->source == nodes_itr->second->id )
                        break;
                }

                if( path_itr == mTaxiPath.end() )
                    continue;

                for (pathnodes_itr = vTaxiPathNodes.begin();
                    pathnodes_itr != vTaxiPathNodes.end();
                    ++pathnodes_itr)
                {
                    if( (*pathnodes_itr)->path == path_itr->second->id &&
                        (*pathnodes_itr)->continent == mapid &&
                        (*pathnodes_itr)->index == 0)
                    {
                        distance = nd;
                        nearest = nodes_itr->second->id;
                        break;
                    }
                }
            }
        }
    }

    return nearest;*/
}


void ObjectMgr::GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{
    /*TaxiPathMap::const_iterator itr;

    for (itr = mTaxiPath.begin(); itr != mTaxiPath.end(); itr++)
    {
        if( (itr->second->source == source) && (itr->second->destination == destination) )
            path = itr->second->id;
        cost = itr->second->price;
    }*/

	std::stringstream query;

	query << "SELECT taxipath.price, taxipath.ID FROM taxipath WHERE taxipath.source = " << source << " AND taxipath.destination = " << destination;
	
	Log::getSingleton( ).outDebug(" STRING %s ",query.str().c_str());

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

    if( result.get() == NULL )
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
	std::stringstream query;

	query << "SELECT taxinodes.mount FROM taxinodes WHERE taxinodes.ID = " << id;
	
	Log::getSingleton( ).outDebug(" STRING %s ",query.str().c_str());

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

	if( result.get() == NULL )
	{
		return 0;
	}

	Field *fields = result->Fetch();
	return fields[0].GetUInt16();

    /*TaxiNodesMap::const_iterator itr;

    for (itr = mTaxiNodes.begin(); itr != mTaxiNodes.end(); itr++)
    {
        if( (itr->second->id == id) )
            return itr->second->mount;
    }

    return 0;*/
}


void ObjectMgr::GetTaxiPathNodes( uint32 path, Path *pathnodes )
{
	std::stringstream query;

	query << "SELECT taxipathnodes.X, taxipathnodes.Y, taxipathnodes.Z FROM taxipathnodes WHERE taxipathnodes.path = " << path;
	
	Log::getSingleton( ).outDebug(" STRING %s ",query.str().c_str());

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

	if( result.get() == NULL )
	{
		 pathnodes->setLength( 0 );
	}
	
	uint16 count = result->GetRowCount();

	Log::getSingleton( ).outDebug(" ROW COUNT %u ",count);

	pathnodes->setLength( count );

	uint16 i = 0;

	do
	{
		Field *fields = result->Fetch();
		pathnodes->getNodes( )[ i ].x = fields[0].GetFloat();
        pathnodes->getNodes( )[ i ].y = fields[1].GetFloat();
        pathnodes->getNodes( )[ i ].z = fields[2].GetFloat();
        i++;
	} while( result->NextRow() );

    /*uint16 i = 0;

    for( TaxiPathNodesVec::iterator itr = vTaxiPathNodes.begin();
        itr != vTaxiPathNodes.end(); ++itr )
    {
        if ((*itr)->path == path)
        {
            i++;
        }
    }

    pathnodes->setLength( i );

    i = 0;
    for( TaxiPathNodesVec::iterator itr = vTaxiPathNodes.begin();
        itr != vTaxiPathNodes.end(); ++itr )
    {
        if ((*itr)->path == path)
        {
            pathnodes->getNodes( )[ i ].x = (*itr)->x;
            pathnodes->getNodes( )[ i ].y = (*itr)->y;
            pathnodes->getNodes( )[ i ].z = (*itr)->z;
            i++;
        }
    }*/
}

#ifndef ENABLE_GRID_SYSTEM
Corpse *ObjectMgr::GetCorpseByOwner(Player *pOwner)
{
    CorpseMap::const_iterator itr;
    for (itr = mCorpses.begin(); itr != mCorpses.end(); itr++)
    {
        if(!itr->second->GetUInt64Value(CORPSE_FIELD_OWNER))
            continue;
        if(itr->second->GetUInt64Value(CORPSE_FIELD_OWNER) == pOwner->GetGUID())
            return itr->second;
    }
    return NULL;
}


void ObjectMgr::LoadCorpses()
{
    Corpse *pCorpse;

    QueryResult *result = sDatabase.Query( "SELECT * FROM Corpses" );

    if( !result )
        return;

/// get number of rows for items
        barGoLink bar( result->GetRowCount() );

    do
    {
        pCorpse = new Corpse;
        Field *fields = result->Fetch();
/// print bar step
        bar.step();
        pCorpse->Create(fields[0].GetUInt32());

        pCorpse->SetPosition(fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
        pCorpse->SetZoneId(fields[5].GetUInt32());
        pCorpse->SetMapId(fields[6].GetUInt32());
        pCorpse->LoadValues( fields[7].GetString());;
        Log::getSingleton( ).outError("AddObject at ObjectMgr.cpp line 940");
        AddObject(pCorpse);
    } while( result->NextRow() );

    delete result;
}
#endif

/*
void ObjectMgr::AddGossipText(GossipText *pGText)
{
    ASSERT( pGText->ID );
    ASSERT( mGossipText.find(pGText->ID) == mGossipText.end() );
    mGossipText[pGText->ID] = pGText;
}*/


GossipText *ObjectMgr::GetGossipText(uint32 ID)
{
    /*GossipTextMap::const_iterator itr;
    for (itr = mGossipText.begin(); itr != mGossipText.end(); itr++)
    {
        if(itr->second->ID == ID)
            return itr->second;
    }
    return NULL;*/

	std::stringstream query;
	query << "SELECT * FROM npc_text WHERE ID = " << ID;
	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

    if( result.get() == NULL )
        return NULL;

	Field *fields = result->Fetch();
    GossipText *pGText = new GossipText;
    pGText->ID = fields[0].GetUInt32();
    pGText->Text = fields[1].GetString();
	
	return pGText;
}

/*
void ObjectMgr::LoadGossipText()
{
    GossipText *pGText;

    std::auto_ptr<QueryResult> result(sDatabase.Query( "SELECT * FROM npc_text" ));
    if( result.get() == NULL )
        return;
    
    do
    {
        Field *fields = result->Fetch();
        pGText = new GossipText;
        pGText->ID = fields[0].GetUInt32();
        pGText->Text = fields[1].GetString();
        AddGossipText(pGText);
	
    }while( result->NextRow() );
}*/

/*
void ObjectMgr::AddGossip(GossipNpc *pGossip)
{
    Creature *cr =  _getCreature(pGossip->Guid);
    if( cr != NULL )
    {
		const uint32 mapid = cr->GetMapId();;
		ASSERT( pGossip->Guid );
		ASSERT( mGossipNpc[mapid].find(pGossip->Guid) == mGossipNpc[mapid].end() );
		mGossipNpc[mapid][pGossip->Guid] = pGossip;
    }
}*/

/*GossipNpc *ObjectMgr::DefaultGossip()
{
    //GossipNpcMap::iterator iter = mGossipNpc[mapid].find(guid);
    //return ( iter == mGossipNpc[mapid].end() ? NULL : iter->second);

	GossipNpc *pGossip = new GossipNpc;
	pGossip->Guid = 999999;
	pGossip->TextID = 999999;
	pGossip->OptionCount = 0;

	std::stringstream query1;
	query1 << "SELECT * FROM npc_options WHERE NPC_GUID =" << 999999;
	std::auto_ptr<QueryResult> result2(sDatabase.Query( query1.str().c_str() ));

	if( result2.get() != NULL )
	{
		unsigned int count = 0;
		pGossip->OptionCount = result2->GetRowCount();
		pGossip->pOptions = new GossipOptions[pGossip->OptionCount];
		do
		{
			Field *fields1 = result2->Fetch();
			//pGossip->pOptions[count].ID = fields1[0].GetUInt32();
			pGossip->pOptions[count].Guid = fields1[1].GetUInt32();
			pGossip->pOptions[count].Icon = fields1[2].GetUInt16();
			pGossip->pOptions[count].OptionText = fields1[3].GetString();
			pGossip->pOptions[count].NextTextID = fields1[4].GetUInt32();
			pGossip->pOptions[count].Special = fields1[5].GetUInt32();
			++count;
		}
		while ( result2->NextRow() );
	}
	return pGossip;
}*/

GossipNpc *ObjectMgr::DefaultGossip(uint32 guid)
{
	GossipNpc *pGossip = new GossipNpc;
	pGossip->Guid = guid;
	pGossip->TextID = 999999;
	pGossip->OptionCount = 0;

	/*pGossip->OptionCount = 2;
	pGossip->pOptions = new GossipOptions[pGossip->OptionCount];
	pGossip->pOptions[0].Guid = 999999;
	pGossip->pOptions[0].Icon = 1;
	pGossip->pOptions[0].OptionText = "Goodbye.";
	pGossip->pOptions[0].NextTextID = 999999;
	pGossip->pOptions[0].Special = 0;

	pGossip->pOptions[1].Guid = 999999;
	pGossip->pOptions[1].Icon = 1;
	pGossip->pOptions[1].OptionText = "Goodbye.";
	pGossip->pOptions[1].NextTextID = 999999;
	pGossip->pOptions[1].Special = 0;*/

	return pGossip;
}

GossipNpc *ObjectMgr::DefaultVendorGossip()
{
	GossipNpc *pGossip = new GossipNpc;
	pGossip->Guid = 999999;
	pGossip->TextID = 999999;
	pGossip->OptionCount = 0;

/*	pGossip->OptionCount = 1;
	pGossip->pOptions = new GossipOptions[pGossip->OptionCount];
	pGossip->pOptions[0].Guid = 999999;
	pGossip->pOptions[0].Icon = 1;
	pGossip->pOptions[0].OptionText = "Goodbye.";
	pGossip->pOptions[0].NextTextID = 999999;
	pGossip->pOptions[0].Special = 3;

	pGossip->pOptions[1].Guid = 999999;
	pGossip->pOptions[1].Icon = 1;
	pGossip->pOptions[1].OptionText = "Goodbye.";
	pGossip->pOptions[1].NextTextID = 999999;
	pGossip->pOptions[1].Special = 3;*/

	return pGossip;
}

GossipNpc *ObjectMgr::GetGossipByGuid(uint32 guid/*, uint32 mapid*/)
{
    //GossipNpcMap::iterator iter = mGossipNpc[mapid].find(guid);
    //return ( iter == mGossipNpc[mapid].end() ? NULL : iter->second);

	std::stringstream query;
	query << "SELECT * FROM npc_gossip WHERE NPC_GUID = " << guid;
	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

    if( result.get() == NULL )
        return NULL;

	Field *fields = result->Fetch();
	GossipNpc *pGossip = new GossipNpc;
	pGossip->Guid = guid;
	pGossip->TextID = fields[1].GetUInt32();
	pGossip->OptionCount = 0;

	std::stringstream query1;
	query1 << "SELECT * FROM npc_options WHERE NPC_GUID =" << guid;
	std::auto_ptr<QueryResult> result2(sDatabase.Query( query1.str().c_str() ));

	if( result2.get() != NULL )
	{
		unsigned int count = 0;
		pGossip->OptionCount = result2->GetRowCount();
		pGossip->pOptions = new GossipOptions[pGossip->OptionCount];
		do
		{
			Field *fields1 = result2->Fetch();
			//pGossip->pOptions[count].ID = fields1[0].GetUInt32();
			pGossip->pOptions[count].Guid = fields1[1].GetUInt32();
			pGossip->pOptions[count].Icon = fields1[2].GetUInt16();
			pGossip->pOptions[count].OptionText = fields1[3].GetString();
			pGossip->pOptions[count].NextTextID = fields1[4].GetUInt32();
			pGossip->pOptions[count].Special = fields1[5].GetUInt32();
			++count;
		}
		while ( result2->NextRow() );
	}
	return pGossip;
}

/*
void ObjectMgr::LoadGossips()
{
    std::auto_ptr<QueryResult> result(sDatabase.Query( "SELECT * FROM npc_gossip" ));
    if( result.get() == NULL )
        return;

    do
    {
        Field *fields = result->Fetch();
        GossipNpc *pGossip = new GossipNpc;
	   // pGossip->ID = fields[0].GetUInt32();
        pGossip->Guid = fields[0].GetUInt32();
        pGossip->TextID = fields[1].GetUInt32();
//        pGossip->OptionCount = fields[4].GetUInt32();
		pGossip->OptionCount = 0;

	//	if( pGossip->OptionCount > 0 )
	//		pGossip->pOptions = new GossipOptions[pGossip->OptionCount];

        std::stringstream query;
        query << "SELECT * FROM npc_options WHERE NPC_GUID =" << pGossip->Guid;
        std::auto_ptr<QueryResult> result2(sDatabase.Query( query.str().c_str() ));
        if( result2.get() != NULL )
        {
			unsigned int count = 0;
			pGossip->OptionCount = result2->GetRowCount();
			pGossip->pOptions = new GossipOptions[pGossip->OptionCount];

		//	bool still_good = true;
			do
			{
			//while( count < pGossip->OptionCount && still_good)
			//{
				Field *fields1 = result2->Fetch();
				pGossip->pOptions[count].ID = fields1[0].GetUInt32();
				pGossip->pOptions[count].Guid = fields1[1].GetUInt32();
				pGossip->pOptions[count].Icon = fields1[2].GetUInt32();
				pGossip->pOptions[count].OptionText = fields1[3].GetString();
				pGossip->pOptions[count].NextTextID = fields1[4].GetUInt32();
				pGossip->pOptions[count].Special = fields1[5].GetUInt32();
				++count;
				//still_good = result2->NextRow();
			//}
			}
			while ( result2->NextRow() );
			// incase the count does not match the number of options there
			//pGossip->OptionCount = count;
        }
        AddGossip(pGossip);
    } while( result->NextRow() );
}*/

/*
void ObjectMgr::AddGraveyard(GraveyardTeleport *pgrave)
{
    ASSERT( pgrave );
    ASSERT( mGraveyards.find(pgrave->ID) == mGraveyards.end() );
    mGraveyards[pgrave->ID] = pgrave;
}*/

/*
void ObjectMgr::LoadGraveyards()
{
    QueryResult *result = sDatabase.Query( "SELECT * FROM graveyards" );
    if( !result )
        return;

    do
    {
        Field *fields = result->Fetch();
        GraveyardTeleport *pgrave = new GraveyardTeleport;
        pgrave->ID = fields[0].GetUInt32();
        pgrave->X = fields[1].GetFloat();
        pgrave->Y = fields[2].GetFloat();
        pgrave->Z = fields[3].GetFloat();
        pgrave->O = fields[4].GetFloat();
        pgrave->ZoneId = fields[5].GetUInt32();
        pgrave->MapId = fields[6].GetUInt32();
        pgrave->FactionID = fields[7].GetUInt32();

        AddGraveyard(pgrave);

    }while( result->NextRow() );
}*/

GraveyardTeleport *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId)
{
	std::stringstream query;
	query << "SELECT SQRT(POW(" << x << "-X,2)+POW(" << y << "-Y,2)+POW(" << z << "-Z,2)) as distance,X,Y,Z,mapId from graveyards where mapId = " << MapId << " ORDER BY distance ASC LIMIT 1";

	std::auto_ptr<QueryResult> result(sDatabase.Query( query.str().c_str() ));

    if( result.get() == NULL )
        return NULL;
	
	Field *fields = result->Fetch();
	GraveyardTeleport *pgrave = new GraveyardTeleport;
	//pgrave->ID = fields[1].GetUInt32();
	pgrave->X = fields[1].GetFloat();
	pgrave->Y = fields[2].GetFloat();
	pgrave->Z = fields[3].GetFloat();
    pgrave->MapId = fields[4].GetUInt32();

	return pgrave;
}

AreaTrigger *ObjectMgr::GetAreaTrigger(uint32 trigger)
{
	std::stringstream query;

    query << "SELECT totrigger FROM areatriggers WHERE id = " << trigger;

	std::auto_ptr<QueryResult> result(sDatabase.Query(query.str().c_str()));

    if (result.get() != NULL )
    {
		Field *fields = result->Fetch();
		uint32 totrigger = fields[0].GetUInt32();
		if( totrigger != 0)
		{
			std::stringstream query1;

			query1 << "SELECT mapid,coord_x,coord_y,coord_z FROM areatriggers WHERE id = " << totrigger;

			std::auto_ptr<QueryResult> result1(sDatabase.Query(query1.str().c_str()));

			if ( result1.get() != NULL )
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
///    QueryResult *result = sDatabase.Query( "SELECT * FROM teleport" );
    QueryResult *result = sDatabase.Query( "SELECT * FROM teleport_cords" );

    if( !result )
        return;

    TeleportCoords *pTC;

/// get number of rows for teleport_coords
    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();
/// print bar step
        bar.step();

        pTC = new TeleportCoords;
        pTC->id = fields[0].GetUInt32();
        pTC->mapId = fields[1].GetUInt32();
        pTC->x = fields[2].GetFloat();
        pTC->y = fields[3].GetFloat();
        pTC->z = fields[4].GetFloat();

        AddTeleportCoords(pTC);

    } while( result->NextRow() );

    delete result;
}


void ObjectMgr::SetHighestGuids()
{
    QueryResult *result;
    uint32 corpseshi=0;
    uint32 gameobjectshi=0;

    result = sDatabase.Query( "SELECT MAX(guid) FROM characters" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(id) FROM creatures" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(guid) FROM item_instances" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    // result = sDatabase.Query( "SELECT MAX(name_id) FROM creature_names" );
    //result = sDatabase.Query( "SELECT MAX(entryid) FROM creaturetemplate" );
	result = sDatabase.Query( "SELECT MAX(modelid) FROM creaturetemplate" );
    if( result )
    {
        m_hiNameGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(id) FROM gameobjects" );
    if( result )
    {
        gameobjectshi = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(Id) FROM auctionhouse" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_auctionid = 0;
    }
    result = sDatabase.Query( "SELECT MAX(mailId) FROM mail" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_mailid = 0;
    }
    // FIXME: Corpses are Gameobjects. IF we add other gameobjects we need a unified table
    result = sDatabase.Query( "SELECT MAX(guid) FROM corpses" );
    if( result )
    {
        corpseshi = (*result)[0].GetUInt32()+1;

        delete result;
    }
    if(corpseshi > gameobjectshi )
    {
        m_hiGoGuid = corpseshi;
    }
    else
    {
        m_hiGoGuid = gameobjectshi;
    }
}


uint32 ObjectMgr::GenerateAuctionID()
{
    objmgr.m_auctionid++;
    return objmgr.m_auctionid;
}


uint32 ObjectMgr::GenerateMailID()
{
    objmgr.m_mailid++;
    return objmgr.m_mailid;
}


uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
    uint32 guidlow = 0;

    switch(guidhigh)
    {
        case HIGHGUID_ITEM          : guidlow = objmgr.m_hiItemGuid++;     break;
        // case HIGHGUID_CONTAINER     : guidlow = objmgr.m_hiItemGuid++;     break;
        case HIGHGUID_UNIT          : guidlow = objmgr.m_hiCreatureGuid++; break;
        case HIGHGUID_PLAYER        : guidlow = objmgr.m_hiCharGuid++;     break;
        case HIGHGUID_GAMEOBJECT    : guidlow = objmgr.m_hiGoGuid++;       break;
        // case HIGHGUID_CORPSE        : guidlow = objmgr.m_hiGoGuid++;       break;
        case HIGHGUID_DYNAMICOBJECT : guidlow = objmgr.m_hiDoGuid++;       break;
        default                     : ASSERT(guidlow);
    }

    return guidlow;
}


// static initailization
GameObjectInfo ObjectMgr::si_UnknownGameObjectInfo;
