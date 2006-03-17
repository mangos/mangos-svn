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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Stats.h"
#include "Log.h"
#include "LootMgr.h"
#include "Chat.h" 
#include "MapManager.h"
#include "FactionTemplateResolver.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"


Creature::Creature() : Unit(), i_AI(NULL)
{
    mQuestIds.clear();

    m_corpseDelay = 45000;
    m_respawnDelay = 25000;

    m_respawnTimer = 0;
    m_deathTimer = 0;
    m_valuesCount = UNIT_END;

    
    itemcount = 0;
    memset(item_list, 0, 8*128);

    m_moveBackward = false;
    m_moveRandom = false;
    m_moveRun = false;
    i_creatureState = STOPPED; 

    m_regenTimer=0;
    m_moveSpeed = 1.0f;
}


Creature::~Creature()
{
    mQuestIds.clear( );
    delete i_AI;
    i_AI = NULL;
}


void 
Creature::_RealtimeSetCreatureInfo()
{
    
	
    
    
    

    CreatureInfo *ci = NULL;
    bool need_save = false;
    
    if (GetNameID() >= 0 && GetNameID() < 999999)
		ci = objmgr.GetCreatureName(GetNameID());
    
    if (ci)
    {
	if (this->GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS) != ci->bounding_radius)
	    this->SetFloatValue( UNIT_FIELD_BOUNDINGRADIUS, ci->bounding_radius);
	
	
	
		if (this->GetUInt32Value(UNIT_FIELD_DISPLAYID) != ci->DisplayID)
			this->SetUInt32Value( UNIT_FIELD_DISPLAYID, ci->DisplayID );

		if (this->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID) != ci->DisplayID)
			this->SetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID, ci->DisplayID );

		if (this->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) != ci->mount)
			this->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, ci->mount );
		
		if (this->GetUInt32Value(UNIT_FIELD_LEVEL) != ci->level)
			this->SetUInt32Value( UNIT_FIELD_LEVEL, ci->level );

		if (this->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != ci->faction)
			this->SetUInt32Value( UNIT_FIELD_FACTIONTEMPLATE, ci->faction );
	
		
		if (this->GetUInt32Value(UNIT_FIELD_FLAGS) != ci->Type)
			this->SetUInt32Value( UNIT_FIELD_FLAGS, ci->Type );

		
		{
			if ( objmgr.HasTrainerspells( this->GetNameID() ) )
			{


				if (ci->flag != UNIT_NPC_FLAG_TRAINER)
					ci->flag = UNIT_NPC_FLAG_TRAINER;

				this->SetUInt32Value( UNIT_NPC_FLAGS, ci->flag);
			}
			else if (this->getItemCount() > 0)
			{


				if (ci->flag != UNIT_NPC_FLAG_VENDOR)
					ci->flag = UNIT_NPC_FLAG_VENDOR;

				this->SetUInt32Value( UNIT_NPC_FLAGS, ci->flag);
			}
			else
			{
				this->SetUInt32Value( UNIT_NPC_FLAGS, ci->flag);
			}
		}
		
		if (ci->maxhealth > 0 && (ci->flags1 & UNIT_DYNFLAG_DEAD))
			ci->flags1 &= ~UNIT_DYNFLAG_DEAD;

		if (ci->maxhealth > 0 && (ci->flags1 & UNIT_DYNFLAG_LOOTABLE))
			ci->flags1 &= ~UNIT_DYNFLAG_LOOTABLE;

		if (this->GetUInt32Value(UNIT_DYNAMIC_FLAGS) != ci->flags1)
			this->SetUInt32Value( UNIT_DYNAMIC_FLAGS, ci->flags1);

		if (ci->maxhealth <= 0)
		{
			if (ci->level >= 64)
				ci->maxhealth = urand(ci->level*50, ci->level*80);
			else if (ci->level > 48)
				ci->maxhealth = urand(ci->level*40, ci->level*70);
			else if (ci->level > 32)
				ci->maxhealth = urand(ci->level*30, ci->level*60);
			else if (ci->level > 24)
				ci->maxhealth = urand(ci->level*30, ci->level*60);
			else if (ci->level > 16)
				ci->maxhealth = urand(ci->level*40, ci->level*60);
			else
				ci->maxhealth = urand(ci->level*70, ci->level*150);
			
			need_save = true;
		}

		if (this->GetUInt32Value(UNIT_FIELD_MAXHEALTH) != ci->maxhealth)
			this->SetUInt32Value( UNIT_FIELD_MAXHEALTH, ci->maxhealth );

		if (this->GetUInt32Value(UNIT_FIELD_BASE_HEALTH) != ci->maxhealth)
			this->SetUInt32Value( UNIT_FIELD_BASE_HEALTH, ci->maxhealth );




		if (this->GetUInt32Value(UNIT_FIELD_BASE_MANA) != ci->maxmana)
			this->SetUInt32Value( UNIT_FIELD_BASE_MANA, ci->maxmana);
	
		if (ci->baseattacktime <= 1000) 
		{
			if (ci->level > 48)
				ci->baseattacktime = urand(1000, 1500);
			else if (ci->level > 32)
				ci->baseattacktime = urand(1000, 2000);
			else if (ci->level > 24)
				ci->baseattacktime = urand(1000, 3000);
			else if (ci->level > 16)
				ci->baseattacktime = urand(2000, 3000);
			else if (ci->level > 8)
				ci->baseattacktime = urand(2000, 4000);
			else
				ci->baseattacktime = urand(3000, 4000);

			need_save = true;
		}
	
		if (this->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME) != ci->baseattacktime)
			this->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME, ci->baseattacktime);
	
		if (ci->rangeattacktime <= 1000) 
		{
			if (ci->level > 48)
				ci->rangeattacktime = urand(1000, 1500);
			else if (ci->level > 32)
				ci->rangeattacktime = urand(1000, 2000);
			else if (ci->level > 24)
				ci->rangeattacktime = urand(1000, 3000);
			else if (ci->level > 16)
				ci->rangeattacktime = urand(2000, 3000);
			else if (ci->level > 8)
				ci->rangeattacktime = urand(2000, 4000);
			else
				ci->rangeattacktime = urand(3000, 4000);

			need_save = true;
		}
	
		if (this->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME) != ci->rangeattacktime)
			this->SetUInt32Value( UNIT_FIELD_RANGEDATTACKTIME, ci->rangeattacktime);
	
		
		if (((ci->mindmg > 1000 && ci->level < 48) || ci->mindmg <= 0)) 
		{
			if (ci->level > 40)
			{
				ci->mindmg = float(ci->level-urand(0, 5));
			}
			else if (ci->level > 30)
			{
				ci->mindmg = float(ci->level-urand(0, 10));
			}
			else if (ci->level > 20)
			{
				ci->mindmg = float(ci->level-urand(0, 15));
			}
			else if (ci->level > 10)
			{
				ci->mindmg = float(ci->level-urand(0, 9));
			}
			else if (ci->level > 5)
			{
				ci->mindmg = float(ci->level-urand(0, 4));
			}
			else
			{
				ci->mindmg = float(ci->level-1);
			}
	    
			if (ci->mindmg <= 0)
				ci->mindmg = float(1);
		}
	
		if (((ci->maxdmg > 1000 && ci->level < 48) || ci->maxdmg <= 0)) 
		{
			if (ci->level > 40)
			{
				ci->maxdmg = float(ci->level+urand(1, 50));
			}
			else if (ci->level > 20)
			{
				ci->maxdmg = float(ci->level+urand(1, 30));
			}
			else if (ci->level > 10)
			{
			ci->maxdmg = float(ci->level+urand(1, 15));
			}
			else if (ci->level > 5)
			{
			ci->maxdmg = float(ci->level+urand(1, 8));
			}
			else
			{
				ci->maxdmg = float(ci->level+urand(1, 4));
			}
	    
			if (ci->maxdmg <= 1)
				ci->maxdmg = float(2);
		}

		if (ci->mindmg > ci->maxdmg)
		{
			uint32 max = ci->mindmg;

			ci->mindmg = ci->maxdmg;
			ci->maxdmg = max;

			need_save = true;
		}
	
		if (this->GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE) != ci->mindmg)
			this->SetFloatValue( UNIT_FIELD_MINRANGEDDAMAGE, ci->mindmg );

		if (this->GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE) != ci->maxdmg)
			this->SetFloatValue( UNIT_FIELD_MAXRANGEDDAMAGE, ci->maxdmg );
	
		if (this->GetFloatValue(UNIT_FIELD_MINDAMAGE) != ci->mindmg)
			this->SetFloatValue( UNIT_FIELD_MINDAMAGE, ci->mindmg );

		if (this->GetFloatValue(UNIT_FIELD_MAXDAMAGE) != ci->maxdmg)
			this->SetFloatValue( UNIT_FIELD_MAXDAMAGE, ci->maxdmg );
	
		
	

		if (ci->scale <= 0 || ci->scale > 2.0)
			ci->scale = 1.0;

		if (this->GetFloatValue(OBJECT_FIELD_SCALE_X) != ci->scale)
			this->SetFloatValue( OBJECT_FIELD_SCALE_X, ci->scale );

		
	
		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY) != ci->slot1model)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ci->slot1model);

		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_INFO) != ci->slot1pos)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO, ci->slot1pos);
	
		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01) != ci->slot2model)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ci->slot2model);
	
		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_INFO) != ci->slot2pos)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO+1, ci->slot2pos);
	
		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02) != ci->slot3model)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ci->slot3model);
	
		if (this->GetUInt32Value(UNIT_VIRTUAL_ITEM_INFO) != ci->slot3pos)
			this->SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO+2, ci->slot3pos);
    }

	if (this->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) <= 0 || this->GetUInt32Value( UNIT_FIELD_BASE_HEALTH ) <= 0)
	{
		uint32 maxhealth;

		if (ci->level >= 64)
			maxhealth = urand(getLevel()*50, getLevel()*80);
		else if (ci->level > 48)
			maxhealth = urand(getLevel()*40, getLevel()*70);
		else if (ci->level > 32)
			maxhealth = urand(getLevel()*30, getLevel()*60);
		else if (ci->level > 24)
			maxhealth = urand(getLevel()*30, getLevel()*60);
		else if (ci->level > 16)
			maxhealth = urand(getLevel()*40, getLevel()*60);
		else
			maxhealth = urand(getLevel()*70, getLevel()*150);

		this->SetUInt32Value( UNIT_FIELD_HEALTH, maxhealth );
		this->SetUInt32Value( UNIT_FIELD_BASE_HEALTH, maxhealth );
	}

	
	if( ci->speed > 0 )
	    m_moveSpeed = ci->speed;
	
	if (need_save)
	{
		
	}

    
    
    
}


void Creature::AIM_Update(const uint32 &diff)
{
    
    switch( m_deathState )
    {
    case JUST_DIED:
	{
	    SetUInt32Value(UNIT_NPC_FLAGS, 0);
	    m_deathState = CORPSE;

	    
	    i_AI->UpdateAI(diff); 
	    break;
	}
    case DEAD:
	{
	    if( m_respawnTimer <= diff )
	    {
		DEBUG_LOG("Respawning...");		

		
		SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
		m_deathState = ALIVE;
		ClearState(ALL_STATE);
		i_motionMaster.Clear(); 
		MapManager::Instance().GetMap(GetMapId())->Add(this); 
	    }
	    else
		m_respawnTimer -= diff;
	    break;
	}
    case CORPSE:
	{
	    if( m_deathTimer <= diff )
	    {
		DEBUG_LOG("Removing corpse...");
		ObjectAccessor::Instance().RemoveCreatureCorpseFromPlayerView(this);
		setDeathState(DEAD);
		m_respawnTimer = m_respawnDelay;
		GetRespawnCoord(m_positionX, m_positionY, m_positionZ);
	    }
	    else
		m_deathTimer -= diff;
	    break;
	}
    case ALIVE:
	{
	    Unit::Update( diff );	    
	    i_AI->UpdateAI(diff);
	    i_motionMaster.UpdateMotion(diff);
	    break;
	}
    default:
	break;
    }
}

void Creature::AIM_Initialize()
{
    i_motionMaster.Initialize(this);
    i_AI = FactorySelector::selectAI(this);
}


void Creature::Update( uint32 p_time )
{
    AIM_Update( p_time );
}


void Creature::Create (uint32 guidlow, const char* name, uint32 mapid, float x, float y, float z, float ang, uint32 nameId)
{
    Object::_Create(guidlow, HIGHGUID_UNIT, mapid, x, y, z, ang, nameId);

    respawn_cord[0] = x;
    respawn_cord[1] = y;
    respawn_cord[2] = z;
    m_name = name;
}




uint32 Creature::getDialogStatus(Player *pPlayer, uint32 defstatus)
{
	bool wasReward  = false;
	bool wasRewardRep  = false;
	bool wasAvail   = false;
	bool wasIncompl = false;
	bool wasAnavail = false;

	bool wasAvailShow   = false;
	bool wasUnavailShow = false;

    uint32 quest_id;
    uint32 status;
    Quest *pQuest;

	for( std::list<uint32>::iterator i = mQuestIds.begin( ); i != mQuestIds.end( ); ++ i )
    {
        quest_id = *i;
        status = pPlayer->getQuestStatus(quest_id);
        pQuest = objmgr.GetQuest(quest_id);

		if ( pQuest == NULL ) continue;

		if ( !pQuest->PreReqSatisfied( pPlayer ) || 
			 !pQuest->IsCompatible( pPlayer ) ||
			  pQuest->RewardIsTaken( pPlayer ) 
			) continue;

		if ( status == QUEST_STATUS_INCOMPLETE ) wasIncompl = true;

		if ( status == QUEST_STATUS_COMPLETE )
		{
			if (pQuest->HasFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ))			
				wasRewardRep = true; else
				wasReward  = true;
		}
		if ( status == QUEST_STATUS_AVAILABLE )
		{
			if ( pQuest->CanShowAvailable( pPlayer ) ) wasAvailShow = true;

			wasAvail   = true;
		}

		if ( status == QUEST_STATUS_UNAVAILABLE )
		{
			wasAnavail = true;
			if ( pQuest->CanShowUnsatified( pPlayer ) ) wasUnavailShow = true;

		}

        if ( status == QUEST_STATUS_NONE )
        {
			if (!pQuest->LevelSatisfied( pPlayer ))
			{
                pPlayer->addNewQuest(quest_id, QUEST_STATUS_UNAVAILABLE );
				if ( pQuest->CanShowUnsatified( pPlayer ) ) wasUnavailShow = true;

				wasAnavail = true;
			}
            else
			{
                pPlayer->addNewQuest(quest_id, QUEST_STATUS_AVAILABLE );
				if ( pQuest->CanShowAvailable( pPlayer ) ) wasAvailShow = true;

				wasAvail = true;
			}
        }
    }

   

	for( std::list<uint32>::iterator i = mInvolvedQuestIds.begin( ); i != mInvolvedQuestIds.end( ); ++ i )
    {
        quest_id = *i;
        status = pPlayer->getQuestStatus(quest_id);
        pQuest = objmgr.GetQuest(quest_id);

		if ( status == QUEST_STATUS_INCOMPLETE )
		{
			if ( pQuest->HasFlag( QUEST_SPECIAL_FLAGS_SPEAKTO ) )
			    wasReward = true; else
				wasIncompl = true;
		}
    }

	if (wasReward) return DIALOG_STATUS_REWARD;
	if (wasRewardRep) return DIALOG_STATUS_REWARD_REP;

	if (wasAvail)    
	{
		if (wasAvailShow)
			return DIALOG_STATUS_AVAILABLE; else
			return DIALOG_STATUS_CHAT;
	}

	if (wasIncompl)  return DIALOG_STATUS_INCOMPLETE;

	if ( defstatus != DIALOG_STATUS_NONE )
		return defstatus;

	if (wasAnavail)  
	{
		if (wasUnavailShow)
			return DIALOG_STATUS_UNAVAILABLE;
	}

    return DIALOG_STATUS_NONE;
}



Quest *Creature::getNextAvailableQuest(Player *pPlayer, Quest *prevQuest)
{
    Quest *pQuest;

	if ( prevQuest->m_qNextQuest != 0 )
	{
		pQuest = objmgr.GetQuest( prevQuest->m_qNextQuest );

		if (pQuest)
		{
			if ( pQuest->CanBeTaken(pPlayer) )
				return pQuest;
		}
	}

	return NULL;		 
}




void Creature::prepareQuestMenu( Player *pPlayer )
{
    uint32 quest_id;
    uint32 status;
    Quest *pQuest;

	for( std::list<uint32>::iterator i = mQuestIds.begin( ); i != mQuestIds.end( ); ++ i )
    {
        quest_id = *i;
        status = pPlayer->getQuestStatus(quest_id);
        pQuest = objmgr.GetQuest(quest_id);

		
		if ( status == QUEST_STATUS_INCOMPLETE )
			pPlayer->PlayerTalkClass->GetQuestMenu()->QuestItem( quest_id, DIALOG_STATUS_INCOMPLETE, false );

		
		if ( ( status == QUEST_STATUS_AVAILABLE ) && ( pQuest->CanBeTaken(pPlayer) ) )
			pPlayer->PlayerTalkClass->GetQuestMenu()->QuestItem( quest_id, DIALOG_STATUS_AVAILABLE, true );
	}

	for( std::list<uint32>::iterator i = mInvolvedQuestIds.begin( ); i != mInvolvedQuestIds.end( ); ++ i )
    {
        quest_id = *i;
        status = pPlayer->getQuestStatus(quest_id);
        pQuest = objmgr.GetQuest(quest_id);

		
		if ( status == QUEST_STATUS_INCOMPLETE )
			pPlayer->PlayerTalkClass->GetQuestMenu()->QuestItem( quest_id, DIALOG_STATUS_INCOMPLETE, false );

		
		if ( status == QUEST_STATUS_INCOMPLETE )
			pPlayer->PlayerTalkClass->GetQuestMenu()->QuestItem( quest_id, DIALOG_STATUS_REWARD, false );
	}
}

bool Creature::hasQuest(uint32 quest_id)
{
    for( std::list<uint32>::iterator i = mQuestIds.begin( ); i != mQuestIds.end( ); ++ i )
    {
        if (*i == quest_id)
            return true;
    }

    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id)
{
    for( std::list<uint32>::iterator i = mInvolvedQuestIds.begin( ); i != mInvolvedQuestIds.end( ); ++ i )
    {
        if (*i == quest_id)
            return true;
    }

    return false;
}





void Creature::generateLoot()
{
    memset(item_list, 0, 8*128);
    itemcount = 0; 
    int LootValue = 0, MaxLootValue = 0;
    
    
    int itemsToGet = 0;
    int creature_level = getLevel();
    if(creature_level < 10)
    {
        itemsToGet = rand()%2; 
    }
    else if(creature_level < 25)
    {
        itemsToGet = rand()%3; 
    }
    else if(creature_level < 40)
    {
        itemsToGet = rand()%4; 
    }
    else if(creature_level < 60)
    {
        itemsToGet = rand()%5; 
    }
    else if(creature_level < 80)
    {
        itemsToGet = rand()%6; 
    }
    else 
    {
        itemsToGet = rand()%7; 
    }
    
    m_lootMoney = (uint32)(creature_level * (rand()%5 + 1)*sWorld.getRate(RATE_DROP)); 
    
    if( itemsToGet == 0 )
    return; 

    
    MaxLootValue = (int)(((creature_level * (rand()%40+50))/5)*sWorld.getRate(RATE_DROP)+rand()%5+5);

    

    const LootMgr::LootList &loot_list(LootManager.getCreaturesLootList(GetUInt32Value(OBJECT_FIELD_ENTRY)));
    bool not_done = (loot_list.size()  && itemsToGet);
    std::vector<short> indexes(loot_list.size());
    std::generate(indexes.begin(), indexes.end(), SequenceGen());
    sLog.outDebug("Number of items to get %d", itemsToGet);
    
    while (not_done)
    {
    
    int idx = rand()%indexes.size();
    const LootItem &item(loot_list[indexes[idx]]);
    indexes.erase(indexes.begin()+idx);
    ItemPrototype *pCurItem = objmgr.GetItemPrototype(item.itemid);
    
    if( pCurItem != NULL && item.chance >= (rand()%100) )
    {
        if( !(LootValue > MaxLootValue) )
        {
        LootValue += pCurItem->BuyPrice;
        addItem(item.itemid, 1);        
        --itemsToGet;
        }
    }
    
    not_done = (itemsToGet && indexes.size() && !(LootValue > MaxLootValue));
    }
}




void Creature::AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run)
{
    WorldPacket data;
    data.Initialize( SMSG_MONSTER_MOVE );
    data << GetGUID();
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    data << (uint32)getMSTime();
    data << uint8(0);
    data << uint32(run ? 0x00000100 : 0x00000000);
    data << time;
    data << uint32(1);
    data << x << y << z;
    WPAssert( data.size() == 49 );
    SendMessageToSet( &data, false );
}

void Creature::SaveToDB()
{
	std::stringstream ss;
    ss << "DELETE FROM creatures WHERE id=" << GetGUIDLow();
    sDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");
    ss << "INSERT INTO creatures (id, mapId, zoneId, name_id, positionX, positionY, positionZ, orientation, data) VALUES ( "
        << GetGUIDLow() << ", "
        << GetMapId() << ", "
        << GetZoneId() << ", "
        << GetUInt32Value(OBJECT_FIELD_ENTRY) << ", "
        << m_positionX << ", "
        << m_positionY << ", "
        << m_positionZ << ", "
        << m_orientation << ", '";

    for( uint16 index = 0; index < m_valuesCount; index ++ )
        ss << GetUInt32Value(index) << " ";

	ss << "\")";

    sDatabase.Execute( ss.str( ).c_str( ) );

    
}



void Creature::LoadFromDB(uint32 guid)
{

    std::stringstream ss;
    ss << "SELECT * FROM creatures WHERE id=" << guid;

    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    ASSERT(result);

    Field *fields = result->Fetch();

    

    Create(fields[8].GetUInt32(), objmgr.GetCreatureName(fields[8].GetUInt32())->Name.c_str(), fields[6].GetUInt32(),
        fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), fields[8].GetUInt32());

    m_zoneId = fields[5].GetUInt32();

    m_moveRandom = fields[9].GetBool();
    m_moveRun = fields[10].GetBool();

    LoadValues(fields[7].GetString());
    
    
    SetNameId(fields[8].GetUInt32());
    _RealtimeSetCreatureInfo();

    delete result;

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ) )
        _LoadGoods();

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ) )
        _LoadQuests();

    
    SetUInt32Value( OBJECT_FIELD_GUID+1, HIGHGUID_UNIT );
    AIM_Initialize();

}

void Creature::_LoadGoods()
{
    
    itemcount = 0;

    
    std::stringstream query;
    query << "SELECT * FROM vendors WHERE vendorGuid=" << GetGUIDLow();

    QueryResult *result = sDatabase.Query( query.str().c_str() );

	
	if(!result)
	{
		std::stringstream query7;
		query7 << "SELECT * FROM vendors WHERE vendorGuid=" << GetNameID();

		result = sDatabase.Query( query7.str().c_str() );

		if (result)
			Log::getSingleton( ).outError( "Vendor %u has items.", GetNameID() );
	}

	if(!result)
	{
		std::stringstream query2;
		query2 << "SELECT * FROM vendors WHERE vendorGuid=" << uint64(GetGUIDLow()+1);

		result = sDatabase.Query( query2.str().c_str() );
	}

	if(!result)
	{
		std::stringstream query5;
		query5 << "SELECT * FROM vendors WHERE vendorGuid=" << uint64(GetGUIDLow()-10);

		result = sDatabase.Query( query5.str().c_str() );
	}

	if(!result)
	{
		std::stringstream query3;
		query3 << "SELECT * FROM vendors WHERE vendorGuid=" << uint64(GetGUID());

		result = sDatabase.Query( query3.str().c_str() );
	}

	if(!result)
	{
		std::stringstream query4;
		query4 << "SELECT * FROM vendors WHERE vendorGuid=" << uint64(GetGUID()+1);

		result = sDatabase.Query( query4.str().c_str() );
	}

	if(!result)
	{
		std::stringstream query6;
		query6 << "SELECT * FROM vendors WHERE vendorGuid=" << uint64(GetGUID()-10);

		result = sDatabase.Query( query6.str().c_str() );
	}

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            if (getItemCount() >= MAX_CREATURE_ITEMS)
            {
                
                
                Log::getSingleton( ).outError( "Vendor %u has too many items (%u >= %i). Check the DB!", GetNameID(), getItemCount(), MAX_CREATURE_ITEMS );
                break;
            }

            setItemId(getItemCount() , fields[1].GetUInt32());
            setItemAmount(getItemCount() , fields[2].GetUInt32());
            increaseItemCount();

        }
        while( result->NextRow() );

        delete result;
    }
}



void Creature::_LoadQuests()
{
    

    mQuestIds.clear();
	mInvolvedQuestIds.clear();

    std::stringstream query;
    query << "SELECT * FROM creaturequestrelation WHERE creatureId=" << GetUInt32Value(OBJECT_FIELD_ENTRY) << " ORDER BY questId";

    QueryResult *result = sDatabase.Query( query.str().c_str() );
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            addQuest(fields[1].GetUInt32());
        }
        while( result->NextRow() );

        delete result;
    }

	

    std::stringstream query1;					    
	query1 << "SELECT * FROM creatureinvolvedrelation WHERE creatureId=" << GetUInt32Value (OBJECT_FIELD_ENTRY) << " ORDER BY questId";

    result = sDatabase.Query( query1.str().c_str() );
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
			uint32 inv = fields[1].GetUInt32();

			Quest *pQuest = objmgr.GetQuest( inv );
			if (!pQuest) continue;

            addInvolvedQuest(inv);
        }
        while( result->NextRow() );

        delete result;
    }
}


void Creature::DeleteFromDB()
{
    char sql[256];

    sprintf(sql, "DELETE FROM creatures WHERE id=%u", GetGUIDLow());
    sDatabase.Execute(sql);
    sprintf(sql, "DELETE FROM vendors WHERE vendorGuid=%u", GetGUIDLow());
    sDatabase.Execute(sql);
    sprintf(sql, "DELETE FROM trainers WHERE trainerGuid=%u", GetGUIDLow());
    sDatabase.Execute(sql);
    sprintf(sql, "DELETE FROM creaturequestrelation WHERE creatureId=%u", GetGUIDLow());
    sDatabase.Execute(sql);
}


float Creature::GetAttackDistance(Unit *pl)
{
    uint16 playlevel     = (uint16)pl->GetUInt32Value(UNIT_FIELD_LEVEL);
    uint16 creaturelevel = (uint16)this->GetUInt32Value(UNIT_FIELD_LEVEL);
    int16 leveldif      = playlevel - creaturelevel;

    float RetDistance=10.0;

    if ( leveldif > 9 )
    { 
        RetDistance = 3;            
    }
    else
    {
        if (leveldif > 0)
            RetDistance =  10 *  GetFloatValue(UNIT_FIELD_COMBATREACH) - 2*(float)leveldif;
        else
            RetDistance = 10 *  GetFloatValue(UNIT_FIELD_COMBATREACH);
        RetDistance = RetDistance>50?50:RetDistance;
        RetDistance = RetDistance<3?3:RetDistance;
    }

    return RetDistance;
}
