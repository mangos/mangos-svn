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
#include "Log.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateMask.h"
#include "Player.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Spell.h"
#include "Stats.h"
#include "UpdateData.h"
#include "Channel.h"
#include "Chat.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Group.h"
#include "Formulas.h"

#include <cmath>

Player::Player (WorldSession *session): Unit()
{
    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    m_session = session;
    
    info = NULL;
    
    m_afk = 0;
    m_curTarget = 0;
    m_curSelection = 0;
    m_lootGuid = 0;
    m_petInfoId = 0;
    m_petLevel = 0;
    m_petFamilyId = 0;

    m_regenTimer = 0;
    m_dismountCost = 0;

    m_nextSave = 900000;                          
   
    m_pCorpse = NULL;
    m_currentSpell = NULL;
    m_resurrectGUID = 0;
    m_resurrectX = m_resurrectY = m_resurrectZ = 0;
    m_resurrectHealth = m_resurrectMana = 0;

    
    memset(m_items, 0, sizeof(Item*)*BANK_SLOT_BAG_END);
	memset(m_buybackitems, 0, sizeof(Item*)*BUYBACK_SLOT_END);

		m_pDuel       = NULL;
		m_pDuelSender = NULL;
		m_isInDuel = false;

    m_GuildIdInvited = 0;
    
    m_groupLeader = 0;
    m_isInGroup = false;
    m_isInvited = false;
   
    m_dontMove = false;

	m_total_honor_points = 0;
    
    logoutDelay = LOGOUTDELAY;
    inCombat = false;
    pTrader = NULL;

    PlayerTalkClass = new PlayerMenu( GetSession() );
    m_timedQuest = 0;
	m_currentBuybackSlot = 0;

    for ( int aX = 0 ; aX < 8 ; aX++ )
        m_Tutorials[ aX ] = 0x00;
    ItemsSetEff[0]=NULL;
	ItemsSetEff[1]=NULL;
	ItemsSetEff[2]=NULL;
 	m_regenTimer = 0;
	m_breathTimer = 0;
	m_isunderwater = 0;
}


Player::~Player ()
{
	for(int j = 0; j < BUYBACK_SLOT_END; j++)
	{
	    if(m_buybackitems[j])
	    {
		   m_buybackitems[j]->DeleteFromDB();
           m_buybackitems[j]->RemoveFromWorld();
		   delete m_buybackitems[j];
	    }
	}
    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            delete m_items[i];
    }
    CleanupChannels();

    delete PlayerTalkClass;
}

void Player::Create( uint32 guidlow, WorldPacket& data )
{
    int i;
    uint8 race,class_,gender,skin,face,hairStyle,hairColor,facialHair,outfitId;

    Object::_Create(guidlow, HIGHGUID_PLAYER);

    
    for (i = 0; i < BANK_SLOT_BAG_END; i++)
        m_items[i] = NULL;

	for(int j = 0; j < BUYBACK_SLOT_END; j++)
	{
        m_buybackitems[j] = NULL;
//	    SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+j*2,0);
//	    SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+j,0);
//	    SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+j,0);
    }

    data >> m_name;
    data >> race >> class_ >> gender >> skin >> face;
    data >> hairStyle >> hairColor >> facialHair >> outfitId;

    info = objmgr.GetPlayerCreateInfo((uint32)race, (uint32)class_);
    ASSERT(info);

    m_race = race;
    m_class = class_;

    m_mapId = info->mapId;
    m_positionX = info->positionX;
    m_positionY = info->positionY;
    m_positionZ = info->positionZ;
    memset(m_taximask, 0, sizeof(m_taximask));

    uint8 powertype = 0;
    uint32 unitfield = 0;
    switch(class_)
    {
        case WARRIOR : powertype = 1; unitfield = 0x11000000; break;      
        case PALADIN : powertype = 0; unitfield = 0x0000EE00; break;      
        case HUNTER  : powertype = 0; unitfield = 0x0000EE00; break;
        case ROGUE   : powertype = 3; unitfield = 0x0000EE00; break;      
        case PRIEST  : powertype = 0; unitfield = 0x0000EE00; break;
        case SHAMAN  : powertype = 0; unitfield = 0x0000EE00; break;
        case MAGE    : powertype = 0; unitfield = 0x0000EE00; break;
        case WARLOCK : powertype = 0; unitfield = 0x0000EE00; break;
        case DRUID   : powertype = 0; unitfield = 0x0000EE00; break;
    }   
                                         
    
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    
    SetUInt32Value(UNIT_FIELD_STR, info->strength );
    SetUInt32Value(UNIT_FIELD_AGILITY, info->ability );
    SetUInt32Value(UNIT_FIELD_STAMINA, info->stamina );
    SetUInt32Value(UNIT_FIELD_IQ, info->intellect );
    SetUInt32Value(UNIT_FIELD_SPIRIT, info->spirit );
	SetUInt32Value(UNIT_FIELD_ARMOR, info->basearmor );
	SetUInt32Value(UNIT_FIELD_ATTACKPOWER, info->attackpower );
    
    SetUInt32Value(UNIT_FIELD_HEALTH, info->health);
	SetUInt32Value(UNIT_FIELD_MAXHEALTH, info->health);

    SetUInt32Value(UNIT_FIELD_POWER1, info->mana );
	SetUInt32Value(UNIT_FIELD_MAXPOWER1, info->mana );
    SetUInt32Value(UNIT_FIELD_POWER2, 0 );
	SetUInt32Value(UNIT_FIELD_MAXPOWER2, info->rage );
    SetUInt32Value(UNIT_FIELD_POWER3, info->focus );
	SetUInt32Value(UNIT_FIELD_MAXPOWER3, info->focus );
    SetUInt32Value(UNIT_FIELD_POWER4, info->energy );
    SetUInt32Value(UNIT_FIELD_MAXPOWER4, info->energy );

	SetFloatValue(UNIT_FIELD_MINDAMAGE, info->mindmg );
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, info->maxdmg );
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, info->ranmindmg );
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, info->ranmaxdmg );
	SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 2000 ); // melee attack time
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, 2000  ); // ranged attack time

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f   );
    
	SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId + gender );
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId + gender );

    SetUInt32Value(UNIT_FIELD_LEVEL, 1 );
    
    setFaction(m_race, 0); 
    LoadReputationFromDBC();
    
    SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( race ) | ( class_ << 8 ) | ( gender << 16 ) | ( powertype << 24 ) ) );
    SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield );
    SetUInt32Value(UNIT_FIELD_BYTES_2, 0xEEEEEE00 );
	SetUInt32Value(UNIT_FIELD_FLAGS , 0x08 );
	SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0x10);
    
    SetPvP(false);
    
    SetUInt32Value(PLAYER_BYTES, ((skin) | (face << 8) | (hairStyle << 16) | (hairColor << 24)));
    SetUInt32Value(PLAYER_BYTES_2, (facialHair | (0xEE << 8) | (0x00 << 16) | (0x02 << 24)));
    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, 400);
    SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );


    SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, 1.00);
/*  
	SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, 0);
    SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, 0);

       
	SetUInt32Value(PLAYER_GUILDID, 0);
	SetUInt32Value(PLAYER_GUILDRANK, 0);
	SetUInt32Value(PLAYER_GUILD_TIMESTAMP, 0);

	SetUInt32Value(PLAYER_FIELD_HONOR_RANK, 0);
	SetUInt32Value(PLAYER_FIELD_HONOR_HIGHEST_RANK, 0);	SetUInt32Value(PLAYER_FIELD_TODAY_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONORABLE_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONORABLE_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONOR, 0);	SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, 0);	SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONOR, 0);	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONOR, 0);	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_STANDING, 0);	SetUInt32Value(PLAYER_FIELD_LIFETIME_HONOR, 0);	SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, 0);
*/    

	_ApplyStatsMods();

	Item *item;
	uint32 titem_id;
	uint8 titem_slot;
	uint8 titem_bagIndex;
	uint32 titem_amount;
	uint16 tspell, tskill[3], taction[4];
	std::list<uint32>::iterator item_id_itr;
	std::list<uint8>::iterator item_bagIndex_itr;
	std::list<uint8>::iterator item_slot_itr;
	std::list<uint32>::iterator item_amount_itr;
	std::list<uint16>::iterator spell_itr, skill_itr[3], action_itr[4];

	item_id_itr = info->item_id.begin();
	item_bagIndex_itr = info->item_bagIndex.begin();
	item_slot_itr = info->item_slot.begin();
	item_amount_itr = info->item_amount.begin();

	for (; item_id_itr!=info->item_id.end(); item_id_itr++, item_bagIndex_itr++, item_slot_itr++, item_amount_itr++) {
		titem_id = (*item_id_itr);
		titem_bagIndex = (*item_bagIndex_itr);
		titem_slot = (*item_slot_itr);
		titem_amount = (*item_amount_itr);

		if (titem_id) {
			item = new Item();
			sLog.outDebug("ITEM: Creating initial item, itemId = %d, bagIndex = %d, slot = %d, count = %d",titem_id, titem_bagIndex, titem_slot, titem_amount);
			AddNewItem(titem_bagIndex, titem_slot, titem_id, titem_amount, false, false);
		}
	}

	spell_itr = info->spell.begin();

	for (; spell_itr!=info->spell.end(); spell_itr++) {
		tspell = (*spell_itr);
		if (tspell) {
			sLog.outDebug("PLAYER: Adding initial spell, id = %d",tspell);
			addSpell(tspell, 0);
		}
	}

	for(i=0 ; i<3; i++) 
		skill_itr[i] = info->skill[i].begin();
    
	for (; skill_itr[0]!=info->skill[0].end() && skill_itr[1]!=info->skill[1].end() && skill_itr[2]!=info->skill[2].end(); ) {
		for (i=0; i<3; i++)
			tskill[i] = (*skill_itr[i]);

		if (tskill[0]) {
			sLog.outDebug("PLAYER: Adding initial skill line, skillId = %d, value = %d, max = %d", tskill[0], tskill[1], tskill[2]);
			SetSkill(tskill[0], tskill[1], tskill[2]);
		}

		for(i=0; i<3; i++)
			skill_itr[i]++;
	}

	for(i=0; i<4; i++)
		action_itr[i] = info->action[i].begin();
    
	for (; action_itr[0]!=info->action[0].end() && action_itr[1]!=info->action[1].end();) 
	{
		for( i=0; i<4 ;i++)
			taction[i] = (*action_itr[i]);
        
		addAction((uint8)taction[0], taction[1], (uint8)taction[2], (uint8)taction[3]);
		
		for( i=0; i<4 ;i++)
			action_itr[i]++;
	}

	delete info;

	m_petInfoId = 0;
	m_petLevel = 0;
	m_petFamilyId = 0;

	m_highest_rank = 0;
	m_last_week_rank = 0;

}

void Player::StartMirrorTimer(uint8 Type, uint32 MaxValue)
{
	//TYPE: 0 = fartigua 1 = breath 2 = fire?
	WorldPacket data;
	uint32 BreathRegen = (uint8)-1;
	data.Initialize(SMSG_START_MIRROR_TIMER);
	data << (uint32)Type;	
	data << MaxValue;
	data << MaxValue;
	data << BreathRegen;
	data << (uint32)0;
	data << (uint8)0;
	m_session->SendPacket(&data);
}

void Player::ModifyMirrorTimer(uint8 Type, uint32 MaxValue, uint32 CurrentValue, uint32 Regen)
{
	//TYPE: 0 = fartigua 1 = breath 2 = fire
	WorldPacket data;
	data.Initialize(SMSG_START_MIRROR_TIMER);
	data << (uint32)Type;
	data << CurrentValue;
	data << MaxValue;
	data << Regen; 
	data << (uint32)0;
	data << (uint8)0;
	GetSession()->SendPacket( &data );
}

void Player::StopMirrorTimer(uint8 Type)
{
	WorldPacket data;
	data.Initialize(SMSG_STOP_MIRROR_TIMER);
	data << (uint32)Type;
	GetSession()->SendPacket( &data );
}

void Player::EnvironmentalDamage(uint64 Guid, uint8 Type, uint32 Amount)
{
	WorldPacket data;
	data.Initialize(SMSG_ENVIRONMENTALDAMAGELOG);
	data << Guid;
	data << (uint8)Type;
	data << Amount;
	data << (uint32)0;
	data << (uint32)0;
	m_session->SendPacket(&data);
	DealDamage((Unit*)this, Amount, 0);
}

void Player::HandleDrowing(uint32 UnderWaterTime)
{
	WorldPacket data;
	
	//if have water breath , then remove bar
	if(waterbreath)
	{
		StopMirrorTimer(1);

		m_breathTimer = 0;
		m_isunderwater = 0;	
		return;
	}

	if ((m_isunderwater & 0x01) && !(m_isunderwater & 0x80) && (!(m_deathState & DEAD)))
	{
		//single trigger timer
		if (!((m_isunderwater & 0x02)))
		{
			m_isunderwater|= 0x02;
			m_breathTimer = UnderWaterTime + 1000;
		}
		//single trigger "Breathbar"
		if ((m_breathTimer <= UnderWaterTime) && (!(m_isunderwater & 0x04)))
		{
			m_isunderwater|= 0x04;
			StartMirrorTimer(1, UnderWaterTime);
		}
		//continius trigger drowning "Damage"
		if ((m_breathTimer == 0) && (m_isunderwater & 0x01))
		{
			uint64 guid;
			guid = GetGUID();
	
			EnvironmentalDamage(guid, DAMAGE_DROWNING,10);
			m_breathTimer = 2000;
		}
	}
	
	//single trigger retract bar
	else if ((!(m_isunderwater & 0x01)) && (!(m_isunderwater & 0x08)) && (m_isunderwater & 0x02) && (m_breathTimer > 0) && (!(m_deathState & DEAD)))
	{
		m_isunderwater = 0x08;
		
		uint32 BreathRegen = 10;
		ModifyMirrorTimer(1, UnderWaterTime, m_breathTimer,BreathRegen);
		m_breathTimer = ((UnderWaterTime + 1000) - m_breathTimer) / BreathRegen;
		m_isunderwater = 0x10;
	}
	//remove bar
	else if ((m_breathTimer < 50) && (!(m_isunderwater & 0x01)) && (m_isunderwater == 0x10))
	{
		StopMirrorTimer(1);

		m_breathTimer = 0;
		m_isunderwater = 0;
	}
}

void Player::HandleLava()
{
	if ((m_isunderwater & 0x80) && (!(m_deathState & DEAD)))
	{
	//Single trigger Set BreathTimer
		if (!((m_isunderwater & 0x04)))
		{
			m_isunderwater|= 0x04;
			m_breathTimer = 1000;
		}
	//Reset BreathTimer and still in the lava
		if (!m_breathTimer)
		{
			uint64 guid;
			uint32 damage = 10;

			guid = GetGUID();
			EnvironmentalDamage(guid, DAMAGE_LAVA, damage);
			m_breathTimer = 1000;
		}

	}
	//Death timer disabled and WaterFlags reset
	else if (m_deathState == DEAD)
	{
		m_breathTimer = 0;
		m_isunderwater = 0;
	}
}

void Player::Update( uint32 p_time )
{
    if(!IsInWorld())
        return;
    
    WorldPacket data;

    Unit::Update( p_time );

    CheckExploreSystem();
    
    quest_status QS;
    Quest *pQuest;

    if ( m_timedQuest > 0 )
    {
        QS     = getQuestStatusStruct( m_timedQuest );
        pQuest = objmgr.GetQuest( m_timedQuest );

        if (pQuest)
            if ( !pQuest->HasFlag(QUEST_SPECIAL_FLAGS_TIMED) ) pQuest = NULL;

        if (pQuest)
        {
            if ( QS.m_timer <= p_time)
            {

                PlayerTalkClass->SendQuestIncompleteToLog( pQuest );
                PlayerTalkClass->SendQuestUpdateFailedTimer( pQuest );

                QS.status = QUEST_STATUS_INCOMPLETE;
                m_timedQuest = 0;
                QS.m_timer = 0;
                QS.m_timerrel = 0;
                loadExistingQuest(QS);
            } else  
            {
                QS.m_timer -= p_time;
                loadExistingQuest(QS);
            }
        }
    }

    if (m_state & UF_ATTACKING)
    {
        inCombat = true;
        logoutDelay = LOGOUTDELAY;
		
        
        if (isAttackReady())
        {
            Unit *pVictim = NULL;
            pVictim = ObjectAccessor::Instance().GetCreature(*this, m_curSelection);
			if(!pVictim)
				pVictim = (Unit *)ObjectAccessor::Instance().FindPlayer(m_curSelection);

        // default combat reach 10
        // TODO add weapon,skill check
			
            float pldistance = 0;
			float plrotation = GetFacing(pVictim);
			if(getClass() == 1)
                {
                pldistance = pldistance + 1;
                }
            if(GetItemBySlot(EQUIPMENT_SLOT_MAINHAND) != 0)
                {
                pldistance = pldistance + 2;
                }
            if(GetItemBySlot(EQUIPMENT_SLOT_HANDS) != 0)
                {
                pldistance = pldistance + 3;
                }

            if (!pVictim)
            {
                sLog.outDetail("Player::Update:  No valid current selection to attack, stopping attack\n");
                this->setRegenTimer(5000);        
                clearStateFlag(UF_ATTACKING);
                smsg_AttackStop(m_curSelection);
            }
			else if(GetDistanceSq(pVictim) > pldistance+12)
            {
				setAttackTimer(uint32(1000));
				data.Initialize(SMSG_ATTACKSWING_NOTINRANGE);
				GetSession()->SendPacket(&data);
            }
			//120 degreas of radiant range
			//(120/360)*(2*PI) = 2,094395102/2 = 1,047197551	//1,57079633-1,047197551   //1,57079633+1,047197551
			else if( (plrotation < (float)0.523598779) || (plrotation > (float)2.617993881))
            {
				setAttackTimer(uint32(1000));
				data.Initialize(SMSG_ATTACKSWING_BADFACING);
				GetSession()->SendPacket(&data);
            }
            else
            {
				setAttackTimer(0);			
				uint32 dmg;
				dmg = CalculateDamage (this);
				AttackerStateUpdate(pVictim, dmg);
			}
        }
    } else { 
        //if( logoutDelay>0 ) logoutDelay--;
        //else {
          logoutDelay = LOGOUTDELAY;
          inCombat = false;
        //}
    }

    if(m_regenTimer > 0)
    {
        if(p_time >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= p_time;
    }
    
    
    if (isAlive())
    {
        RegenerateAll();
    }

    
    if (m_deathState == JUST_DIED)
    {
        if( m_isInDuel )
            DuelComplete();
        else
            KillPlayer();
    }
    
    if(m_nextSave > 0)
    {
        if(p_time >= m_nextSave)
        {
            m_nextSave = 600000;                  
            SaveToDB();
            sLog.outBasic("Player '%u' '%s' Saved", this->GetGUID(), this->GetName());
        }
        else
        {
            m_nextSave -= p_time;
        }
    }

	//Breathtimer
	if(m_breathTimer > 0)
    {
        if(p_time >= m_breathTimer)
			m_breathTimer = 0;
		else
		    m_breathTimer -= p_time;
		
    }

	//Handle Water/drowning
	HandleDrowing(60000);

	//Handle lava
	HandleLava();
}

void Player::BuildEnumData( WorldPacket * p_data )
{
    *p_data << GetGUID();
    *p_data << m_name;

	uint32 bytes = GetUInt32Value(UNIT_FIELD_BYTES_0);
    *p_data << uint8(bytes);
	*p_data << uint8(bytes >> 8);
    *p_data << uint8(bytes >> 16);

    bytes = GetUInt32Value(PLAYER_BYTES);
    *p_data << uint8(bytes);
    *p_data << uint8(bytes >> 8);
    *p_data << uint8(bytes >> 16);
    *p_data << uint8(bytes >> 24);

    bytes = GetUInt32Value(PLAYER_BYTES_2);
    *p_data << uint8(bytes);
	
	
	*p_data << uint8(GetUInt32Value(UNIT_FIELD_LEVEL)); //1
	uint32 zoneId=sAreaStore.LookupEntry(MapManager::Instance ().GetMap(m_mapId)->GetAreaFlag(m_positionX,m_positionY))->zone;
	
	*p_data << zoneId;
    *p_data << GetMapId();

    *p_data << m_positionX;
    *p_data << m_positionY;
    *p_data << m_positionZ;


	*p_data << GetUInt32Value(PLAYER_GUILDID);		//probebly wrong
	
	//*p_data << GetUInt32Value(PLAYER_GUILDRANK);	//this was
	*p_data << uint8(0x0);
	*p_data << uint8(GetUInt32Value(PLAYER_FLAGS) << 1);
	*p_data << uint8(0x0);  //Bit 4 is something dono
	*p_data << uint8(0x0);	//is this player_GUILDRANK????
	
	*p_data << (uint8)0;
    *p_data << (uint32)m_petInfoId;
    *p_data << (uint32)m_petLevel;
    *p_data << (uint32)m_petFamilyId;

	for (int i = 0; i < 20; i++)
    {
        if (m_items[i] != NULL)
        {
            *p_data << (uint32)m_items[i]->GetProto()->DisplayInfoID;
            *p_data << (uint8)m_items[i]->GetProto()->InventoryType;
        }
        else
        {
            *p_data << (uint32)0;
            *p_data << (uint8)0;
        }
		
    }
	    
}

void Player::smsg_NewWorld(uint32 mapid, float x, float y, float z, float orientation)
{
    WorldPacket data;
    data.Initialize(SMSG_TRANSFER_PENDING);
    data << uint32(mapid);

    GetSession()->SendPacket(&data);
    MapManager::Instance().GetMap(GetMapId())->Remove(this, false);

    data.Initialize(SMSG_NEW_WORLD);
    data << (uint32)mapid << (float)x << (float)y << (float)z << (float)orientation;
    GetSession()->SendPacket( &data );
    SetMapId(mapid);
    Relocate(x, y, z, orientation);
    SetPosition(x,y,z,orientation);
    MapManager::Instance().GetMap(GetMapId())->Add(this); 
    SetDontMove(true);
    SaveToDB();
}

void Player::AddToWorld() {
    Object::AddToWorld();

    
    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->AddToWorld();
    }
}


void Player::RemoveFromWorld() {
    
    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->RemoveFromWorld();
    }

    Object::RemoveFromWorld();
}

void Player::finishExplorationQuest( Quest *pQuest )
{
	if(!pQuest) return;

    if ( getQuestStatus( pQuest->m_qId ) == QUEST_STATUS_INCOMPLETE )
    {
        quest_status qs = getQuestStatusStruct( pQuest->m_qId );

        if ( !qs.m_explored )
        {
            PlayerTalkClass->SendQuestUpdateComplete( pQuest );
            qs.m_explored = true;
            loadExistingQuest( qs );
        }

        if ( checkQuestStatus( pQuest ) )
        {
            setQuestStatus( pQuest->m_qId, QUEST_STATUS_COMPLETE, false );
            PlayerTalkClass->SendQuestCompleteToLog( pQuest );
        }
    }
}

bool Player::isQuestComplete(Quest *pQuest, Creature *pCreature)
{
    if (!pQuest) return false;

    if ( getQuestStatus( pQuest->m_qId ) == QUEST_STATUS_INCOMPLETE )
    {

        if ( pQuest->HasFlag( QUEST_SPECIAL_FLAGS_SPEAKTO | QUEST_SPECIAL_FLAGS_DELIVER ) )
        {
            if ( pCreature->hasInvolvedQuest( pQuest->m_qId ) && 
                HasItemInBackpack( pQuest->m_qQuestItem ) )
                return true; else
                return false;
        }

        if ( pQuest->HasFlag( QUEST_SPECIAL_FLAGS_SPEAKTO ) )
        {
            if ( pCreature->hasInvolvedQuest( pQuest->m_qId ) )
                return true; else
                return false;
        }

        return false;
    }

    if ( getQuestStatus( pQuest->m_qId ) == QUEST_STATUS_COMPLETE )
    {
        return true;
    }

    return false;
}

bool Player::isQuestTakable(Quest *pQuest)
{
    if (!pQuest) return false;

    uint32 status = getQuestStatus(pQuest->m_qId);


    if ( pQuest->CanBeTaken( this ) )
    {
        if ( status == QUEST_STATUS_NONE )
        {
            status = addNewQuest( pQuest );
        }

        if (status == QUEST_STATUS_AVAILABLE)
        {
            return true;
        }
    }

    return false;
}

quest_status Player::getQuestStatusStruct(uint32 quest_id)
{
    return mQuestStatus[quest_id];
}

uint32 Player::getQuestStatus(uint32 quest_id)
{
    if  ( mQuestStatus.find( quest_id ) == mQuestStatus.end( ) ) return QUEST_STATUS_NONE;
    return mQuestStatus[quest_id].status;
}

bool Player::getQuestRewardStatus(uint32 quest_id)
{
    if  ( mQuestStatus.find( quest_id ) == mQuestStatus.end( ) ) return false;
    return mQuestStatus[quest_id].rewarded;
}


uint32 Player::addNewQuest(Quest *quest, uint32 status)
{
    quest_status qs;
    qs.m_quest = quest;
    qs.status   = status;
    qs.rewarded = false;

    mQuestStatus[quest->m_qId] = qs;
    return status;
};



void Player::loadExistingQuest(quest_status qs)
{
    mQuestStatus[qs.m_quest->m_qId] = qs;
}


void Player::setQuestStatus(uint32 quest_id, uint32 new_status, bool new_rewarded)
{
    if ( new_status == QUEST_STATUS_AVAILABLE || new_status == QUEST_STATUS_INCOMPLETE )
    {
        m_timedQuest = 0;

        mQuestStatus[quest_id].m_questMobCount[0] = 0;
        mQuestStatus[quest_id].m_questMobCount[1] = 0;
        mQuestStatus[quest_id].m_questMobCount[2] = 0;
        mQuestStatus[quest_id].m_questMobCount[3] = 0;

        if ( new_status == QUEST_STATUS_INCOMPLETE )
        {
            Quest *pQuest = mQuestStatus[quest_id].m_quest;
            if (pQuest)
                if (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_TIMED))
                {
                    
                    time_t kk = time(NULL);
                    kk += pQuest->m_qObjTime * 60;

                    mQuestStatus[quest_id].m_timerrel = (int32)kk;
                    mQuestStatus[quest_id].m_timer = pQuest->m_qObjTime * 60 * 1000; 

                    m_timedQuest = quest_id;
                    PlayerTalkClass->SendQuestUpdateSetTimer( pQuest, pQuest->m_qObjTime );
                }
        }
    }
    

    mQuestStatus[quest_id].status     = new_status;
    mQuestStatus[quest_id].rewarded   = new_rewarded;
    mQuestStatus[quest_id].m_explored = false;
}

void Player::sendPreparedGossip( uint32 textid, QEmote em, std::string QTitle, uint64 guid )
{
    GossipMenu* _GossipMenu = PlayerTalkClass->GetGossipMenu();
    QuestMenu* _QuestMenu   = PlayerTalkClass->GetQuestMenu();

    if ( _GossipMenu->ItemsInMenu() == 0 )
    {
        if ( _QuestMenu->QuestsInMenu() == 1 )
        {
            Quest *pQuest = objmgr.GetQuest( _QuestMenu->GetItem(0).m_qId );
            if (pQuest)
            {
                if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_REWARD )
                    PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0 );
                

                if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_AVAILABLE )
                    PlayerTalkClass->SendQuestDetails( pQuest, guid, true );

                if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_INCOMPLETE )
                {
                    Creature *pNPC = ObjectAccessor::Instance().GetCreature( *this, GUID_LOPART(guid));

                    if ( isQuestComplete( pQuest, pNPC ) )
                        PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0); else
                        PlayerTalkClass->SendRequestedItems(pQuest, guid, false);
                }
            }

            return;
        }

        if  (_QuestMenu->QuestsInMenu() > 1 )
            PlayerTalkClass->SendQuestMenu( em, QTitle, guid ); else
            PlayerTalkClass->SendGossipMenu(textid, guid);

        return;
    }

    PlayerTalkClass->SendGossipMenu(textid, guid);
}


uint16 Player::getOpenQuestSlot()
{
    uint16 start = PLAYER_QUEST_LOG_1_1;
    uint16 end = PLAYER_QUEST_LOG_1_1 + 60;
    for (uint16 i = start; i <= end; i+=3)
        if (GetUInt32Value(i) == 0)
            return i;

    return 0;
}


uint16 Player::getQuestSlot(uint32 quest_id)
{
    uint16 start = PLAYER_QUEST_LOG_1_1;
    uint16 end = PLAYER_QUEST_LOG_1_1 + 60;
    for (uint16 i = start; i <= end; i+=3)
        if (GetUInt32Value(i) == quest_id)
            return i;

    return 0;
}


uint16 Player::getQuestSlotById(uint32 slot_id)
{
    uint16 start = PLAYER_QUEST_LOG_1_1;
    uint16 end   = PLAYER_QUEST_LOG_1_1 + 60;
    uint16 idx   = 0;

    for (uint16 i = start; i <= end; i+=3)
    { 
        idx++; 
        if ( idx == slot_id )
            return i;
    }

    return 0;
}

void Player::AddedItemToBackpack(uint32 entry, uint32 count)
{
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        if (i->second.status == QUEST_STATUS_INCOMPLETE)
        {
            Quest *pQuest = objmgr.GetQuest(i->first);
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (pQuest->m_qObjItemId [j] == entry)
                {
                    if ( !HasItemInBackpack( pQuest->m_qObjItemId[j], pQuest->m_qObjItemCount[j] ) )
                    {
                        PlayerTalkClass->
                            SendQuestUpdateAddItem(  
                                                    pQuest, 
                                                    j, 
                                                    count                                                   
                                                   );

                    }

                    if ( HasItemInBackpack( pQuest->m_qObjItemId[j], pQuest->m_qObjItemCount[j] ) )
                    {
                        PlayerTalkClass->SendQuestUpdateComplete( pQuest );
                    }

                    if ( checkQuestStatus(i->second.m_quest) )
                    {
                        PlayerTalkClass->SendQuestCompleteToLog( pQuest );
                        i->second.status = QUEST_STATUS_COMPLETE;
                    }


                    SaveToDB();
                    return;
                } 
            } 
        } 
    } 
}

void Player::RemovedItemFromBackpack(uint32 entry)
{
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        if (i->second.status == QUEST_STATUS_COMPLETE)
        {
            Quest *pQuest = objmgr.GetQuest(i->first);
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (pQuest->m_qObjItemId[j] == entry)
                {
                    if ( !HasItemInBackpack( pQuest->m_qObjItemId[j], pQuest->m_qObjItemCount[j] ) )
                    {
                        i->second.status = QUEST_STATUS_INCOMPLETE;
                    }

                    SaveToDB();
                    return;
                } 
            } 
        } 
    } 
}

void Player::SetBindPoint(uint64 guid)
{

        // end gossip
        WorldPacket data;
        data.Initialize( SMSG_GOSSIP_COMPLETE );
        GetSession()->SendPacket( &data );

	data.Initialize( MSG_BINDPOINT_CONFIRM );
	data << guid;
	GetSession()->SendPacket( &data );
}

void Player::KilledMonster(uint32 entry, uint64 guid)
{

    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        if (i->second.status == QUEST_STATUS_INCOMPLETE)
        {
            Quest *pQuest = objmgr.GetQuest(i->first);

            if (!pQuest) continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (pQuest->m_qObjMobId[j] == entry)
                {
                    if (i->second.m_questMobCount[j] < pQuest->m_qObjMobCount[j])
                    {
                        i->second.m_questMobCount[j]++;

                        
                        PlayerTalkClass->
                              SendQuestUpdateAddKill(
                                                      pQuest, 
                                                      guid, 
                                                      i->second.m_questMobCount[j],
                                                      j
                                                     );
                    }

                    if (i->second.m_questMobCount[j] == pQuest->m_qObjMobCount[j])
                    {
                        PlayerTalkClass->SendQuestUpdateComplete( pQuest );
                    }

                    if (checkQuestStatus(i->second.m_quest) && (i->second.status == QUEST_STATUS_INCOMPLETE))
                    {
                        PlayerTalkClass->SendQuestCompleteToLog( pQuest );
                        i->second.status = QUEST_STATUS_COMPLETE;
                    }

                    SaveToDB();
                    return;
                } 
            } 
        } 
    } 
}

void Player::CalcRage( uint32 damage,bool attacker )
{
    
    uint32 maxRage = GetUInt32Value(UNIT_FIELD_MAXPOWER2);
    uint32 Rage = GetUInt32Value(UNIT_FIELD_POWER2);
    
    if(attacker)
    	Rage += (uint32)(damage/(getLevel()*0.5f));
    else
    	Rage += (uint32)(damage/(getLevel()*1.5f));
    	
    if(Rage > maxRage)	Rage = maxRage;
    	                   
    SetUInt32Value(UNIT_FIELD_POWER2, Rage);
}

void Player::RegenerateAll()
{
    
    
    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;
    
    if (!(m_state & UF_ATTACKING))                
    {
        Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH);
        Regenerate( UNIT_FIELD_POWER2, UNIT_FIELD_MAXPOWER2);
    }
    
    Regenerate( UNIT_FIELD_POWER4, UNIT_FIELD_MAXPOWER4);
    Regenerate( UNIT_FIELD_POWER1, UNIT_FIELD_MAXPOWER1);

    m_regenTimer = regenDelay;

}



void Player::Regenerate(uint16 field_cur, uint16 field_max)
{
	uint32 curValue = GetUInt32Value(field_cur);    
	uint32 maxValue = GetUInt32Value(field_max);
    
	if(field_cur != UNIT_FIELD_POWER2)
	{
		if (curValue >= maxValue)	return;
	}
	else if (curValue == 0)	
		return;

       
	float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH); 
	float ManaIncreaseRate = sWorld.getRate(RATE_POWER1);   
	float RageIncreaseRate = sWorld.getRate(RATE_POWER2);   
	float EnergyIncreaseRate = sWorld.getRate(RATE_POWER3);
    
	uint16 Spirit = GetUInt32Value(UNIT_FIELD_SPIRIT);
	uint16 Class = getClass();

	if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;
	if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;
	if( RageIncreaseRate <= 0 ) RageIncreaseRate = 1;
	if( EnergyIncreaseRate <= 0 ) EnergyIncreaseRate = 1;

	uint32 addvalue = 0;

	switch (field_cur)
	{
		case UNIT_FIELD_HEALTH:
		{
			switch (Class)
			{
				case WARRIOR: addvalue = uint32((Spirit*0.80) * HealthIncreaseRate); break;
				case PALADIN: addvalue = uint32((Spirit*0.25) * HealthIncreaseRate); break;
				case HUNTER:  addvalue = uint32((Spirit*0.25) * HealthIncreaseRate); break;
				case ROGUE:   addvalue = uint32((Spirit*0.50+2) * HealthIncreaseRate); break;
				case PRIEST:  addvalue = uint32((Spirit*0.10) * HealthIncreaseRate); break;
				case SHAMAN:  addvalue = uint32((Spirit*0.11) * HealthIncreaseRate); break;
				case MAGE:    addvalue = uint32((Spirit*0.10) * HealthIncreaseRate); break;
				case WARLOCK: addvalue = uint32((Spirit*0.11) * HealthIncreaseRate); break;
				case DRUID:   addvalue = uint32((Spirit*0.11) * HealthIncreaseRate); break;
			}
		}break;
		case UNIT_FIELD_POWER1: 
		{
			switch (Class)
			{
				case PALADIN: addvalue = uint32((Spirit/4 + 8)  * ManaIncreaseRate); break;
				case HUNTER:  addvalue = uint32((Spirit/4 + 11) * ManaIncreaseRate); break;
				case PRIEST:  addvalue = uint32((Spirit/4 + 13) * ManaIncreaseRate); break;
				case SHAMAN:  addvalue = uint32((Spirit/5 + 17) * ManaIncreaseRate); break;
				case MAGE:    addvalue = uint32((Spirit/4 + 11) * ManaIncreaseRate); break;
				case WARLOCK: addvalue = uint32((Spirit/4 + 8)  * ManaIncreaseRate); break;
				case DRUID:   addvalue = uint32((Spirit/5 + 15) * ManaIncreaseRate); break;
			}
		}break;
		case UNIT_FIELD_POWER2: 
    {       
			addvalue = uint32(1.66 * RageIncreaseRate);
		}break;
		case UNIT_FIELD_POWER4: 
		{
			addvalue = uint32(20);
		}break;
	}
     
	if (field_cur != UNIT_FIELD_POWER2)
	{
		switch (getStandState())
		{
			case PLAYER_STATE_SIT_CHAIR:
			case PLAYER_STATE_SIT_LOW_CHAIR:
			case PLAYER_STATE_SIT_MEDIUM_CHAIR:
			case PLAYER_STATE_SIT_HIGH_CHAIR:
			case PLAYER_STATE_SIT:			addvalue = (uint32)(addvalue*2.0f); break;
			case PLAYER_STATE_SLEEP:		addvalue = (uint32)(addvalue*3.0f); break;
			case PLAYER_STATE_KNEEL:		addvalue = (uint32)(addvalue*1.5f); break;
		}
		curValue += addvalue;
		if (curValue > maxValue) curValue = maxValue;
		SetUInt32Value(field_cur, curValue);
	}
	else
	{
		curValue -= addvalue;
		if (curValue < 0) curValue = 0;
		SetUInt32Value(field_cur, curValue);
	}
}

bool Player::checkQuestStatus(Quest *pQuest)
{
    quest_status qs = mQuestStatus[pQuest->m_qId];

    bool Result = true;


    if (pQuest->HasFlag( QUEST_SPECIAL_FLAGS_SPEAKTO))
        Result &= false;

    if (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_EXPLORATION))
        Result &= qs.m_explored;

    if (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_TIMED))
        Result &= (qs.m_timer > 0);

    if (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_DELIVER))
        if (HasItemInBackpack( pQuest->m_qObjItemId[0], pQuest->m_qObjItemCount[0]) &&
            HasItemInBackpack( pQuest->m_qObjItemId[1], pQuest->m_qObjItemCount[1]) &&
            HasItemInBackpack( pQuest->m_qObjItemId[2], pQuest->m_qObjItemCount[2]) &&
            HasItemInBackpack( pQuest->m_qObjItemId[3], pQuest->m_qObjItemCount[3]))
                Result &= true; else
                Result &= false;

    if (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_KILL))
        if (qs.m_questMobCount[0] >= pQuest->m_qObjMobCount[0] &&
            qs.m_questMobCount[1] >= pQuest->m_qObjMobCount[1] &&
            qs.m_questMobCount[2] >= pQuest->m_qObjMobCount[2] &&
            qs.m_questMobCount[3] >= pQuest->m_qObjMobCount[3]) 
            Result &= true; else 
            Result &= false;

    return Result;
}

void Player::GiveXP(uint32 xp, const uint64 &guid)
{
	if ( xp < 1 )
		return;

	WorldPacket data;
	if (guid != 0)
	{
		data.Initialize( SMSG_LOG_XPGAIN );
		data << guid;
		data << uint32(xp); // given experience
		data << uint8(0); // 00-kill_xp type, 01-non_kill_xp type
		uint32 xpunrested = xp/2;
		data << uint32(xpunrested); // unrested given experience
		data << uint8(0) << uint8(0) << uint8(0x80) << uint8(0x3f); // unknown (static.. it was same at 4 different killed creatures!)
		GetSession()->SendPacket(&data);
	}

	uint32 curXP = GetUInt32Value(PLAYER_XP);
	uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
	uint32 newXP = curXP + xp;
	
	
	if (newXP >= nextLvlXP)
	{   
		uint16 level = (uint16)GetUInt32Value(UNIT_FIELD_LEVEL);
		
		uint32 MPGain,HPGain,STRGain,STAGain,AGIGain,INTGain,SPIGain;
		MPGain=HPGain=STRGain=STAGain=AGIGain=INTGain=SPIGain=0;
		
		level += 1;                                          
		newXP -= nextLvlXP;
		
		_RemoveStatsMods();

		BuildLvlUpStats(&MPGain,&HPGain,&STRGain,&STAGain,&AGIGain,&INTGain,&SPIGain);
		
		uint32 newMP;
		
		if(getClass() == WARRIOR || getClass() == ROGUE)
			MPGain = 0;
		else
			newMP = GetUInt32Value(UNIT_FIELD_MAXPOWER1) + MPGain;
		
		uint32 newHP = GetUInt32Value(UNIT_FIELD_MAXHEALTH) + HPGain;
		uint32 newSTR = GetUInt32Value(UNIT_FIELD_STR) + STRGain;
		uint32 newSTA = GetUInt32Value(UNIT_FIELD_STAMINA) + STAGain;
		uint32 newAGI = GetUInt32Value(UNIT_FIELD_AGILITY) + AGIGain;
		uint32 newINT = GetUInt32Value(UNIT_FIELD_IQ) + INTGain;
		uint32 newSPI = GetUInt32Value(UNIT_FIELD_SPIRIT) + SPIGain;
		                  
		if( level > 9)
		{            
			uint32 curTalentPoints = GetUInt32Value(PLAYER_CHARACTER_POINTS1);
			SetUInt32Value(PLAYER_CHARACTER_POINTS1,curTalentPoints+1);
		}
		
		SetUInt32Value(UNIT_FIELD_LEVEL, level);
		SetUInt32Value(PLAYER_NEXT_LEVEL_XP, MaNGOS::XP::xp_to_level(level));
		UpdateMaxSkills ();    	
		
		//fill new stats
		if(getClass() != WARRIOR && getClass() != ROGUE)
		{
			SetUInt32Value(UNIT_FIELD_POWER1, newMP);
			SetUInt32Value(UNIT_FIELD_MAXPOWER1, newMP);
		}
		
		SetUInt32Value(UNIT_FIELD_HEALTH, newHP);
		SetUInt32Value(UNIT_FIELD_MAXHEALTH, newHP);
		
		SetUInt32Value(UNIT_FIELD_STR, newSTR);
		SetUInt32Value(UNIT_FIELD_STAMINA, newSTA);
		SetUInt32Value(UNIT_FIELD_AGILITY, newAGI);
		SetUInt32Value(UNIT_FIELD_IQ, newINT);
		SetUInt32Value(UNIT_FIELD_SPIRIT, newSPI);
		
		_ApplyStatsMods();       

		data.Initialize(SMSG_LEVELUP_INFO);
		data << uint32(level);
		data << uint32(HPGain);          
		data << uint32(MPGain);
		data << uint32(0);                        
		data << uint32(0);                        
		data << uint32(0);                        
		data << uint32(0);
		            
		data << uint32(STRGain);
		data << uint32(STAGain);
		data << uint32(AGIGain);
		data << uint32(INTGain);
		data << uint32(SPIGain);                                              
                       

		WPAssert(data.size() == 48);
		GetSession()->SendPacket(&data);
	}
    
	SetUInt32Value(PLAYER_XP, newXP);
}

void Player::BuildLvlUpStats(uint32 *HP,uint32 *MP,uint32 *STR,uint32 *STA,uint32 *AGI,uint32 *INT,uint32 *SPI)
{
	uint8 _class=getClass();
	uint8 lvl=getLevel();
	
	*MP = (uint32)(GetUInt32Value(UNIT_FIELD_SPIRIT) / 2);
	*HP = (uint32)(GetUInt32Value(UNIT_FIELD_STAMINA) / 2);
	
	switch(_class)
	{
		case WARRIOR:
			 //*HP +=
			 //*MP +=
			*STR += (lvl > 23 ? 2 : (lvl > 1  ? 1 : 0)); 
			*STA += (lvl > 23 ? 2 : (lvl > 1  ? 1 : 0)); 
			*AGI += (lvl > 36 ? 1 : (lvl > 6 && (lvl%2) ? 1 : 0));   
			*INT += (lvl > 9 && !(lvl%2) ? 1 : 0);
			*SPI += (lvl > 9 && !(lvl%2) ? 1 : 0);
			break;
		case PALADIN:
			 //*HP +=
			 //*MP +=
			*STR += (lvl > 3  ? 1 : 0);
			*STA += (lvl > 33 ? 2 : (lvl > 1 ? 1 : 0)); 
			*AGI += (lvl > 38 ? 1 : (lvl > 7 && !(lvl%2) ? 1 : 0)); 
			*INT += (lvl > 6 && (lvl%2) ? 1 : 0);
			*SPI += (lvl > 7 ? 1 : 0);
			break;
		case HUNTER:
			//*HP +=
			//*MP +=
		  *STR += (lvl > 4  ? 1 : 0);
		  *STA += (lvl > 4  ? 1 : 0);
		  *AGI += (lvl > 33 ? 2 : (lvl > 1 ? 1 : 0));  
		  *INT += (lvl > 8 && (lvl%2) ? 1 : 0); 
		  *SPI += (lvl > 38 ? 1 : (lvl > 9 && !(lvl%2) ? 1 : 0));
			break;
		case ROGUE:
			//*HP +=
			//*MP +=			
			*STR += (lvl > 5  ? 1 : 0);
			*STA += (lvl > 4  ? 1 : 0);
			*AGI += (lvl > 16 ? 2 : (lvl > 1 ? 1 : 0)); 
			*INT += (lvl > 8 && !(lvl%2) ? 1 : 0); 
			*SPI += (lvl > 38 ? 1 : (lvl > 9 && !(lvl%2) ? 1 : 0));
			break;
		case PRIEST:
			//*HP +=
			//*MP +=
			*STR += (lvl > 9 && !(lvl%2) ? 1 : 0);
			*STA += (lvl > 5  ? 1 : 0);
			*AGI += (lvl > 38 ? 1 : (lvl > 8 && (lvl%2) ? 1 : 0));
			*INT += (lvl > 22 ? 2 : (lvl > 1 ? 1 : 0));  
			*SPI += (lvl > 3  ? 1 : 0);
			break;
		case SHAMAN:
			//*HP +=
			//*MP +=
			*STR += (lvl > 34 ? 1 : (lvl > 6 && (lvl%2) ? 1 : 0)); 
			*STA += (lvl > 4 ? 1 : 0);
			*AGI += (lvl > 7 && !(lvl%2) ? 1 : 0);  
			*INT += (lvl > 5 ? 1 : 0);
			*SPI += (lvl > 4 ? 1 : 0);
			break;
		case MAGE:
			//*HP +=
			//*MP +=
			STR += (lvl > 9 && !(lvl%2) ? 1 : 0);
			STA += (lvl > 5  ? 1 : 0);
			AGI += (lvl > 9 && !(lvl%2) ? 1 : 0);
			INT += (lvl > 24 ? 2 : (lvl > 1 ? 1 : 0));  
			SPI += (lvl > 33 ? 2 : (lvl > 2 ? 1 : 0));
			break;
		case WARLOCK:
			//*HP +=
			//*MP +=
			STR += (lvl > 9 && !(lvl%2) ? 1 : 0);
			STA += (lvl > 38 ? 2 : (lvl > 3 ? 1 : 0));   
			AGI += (lvl > 9 && !(lvl%2) ? 1 : 0);  
			INT += (lvl > 33 ? 2 : (lvl > 2 ? 1 : 0));  
			SPI += (lvl > 38 ? 2 : (lvl > 3 ? 1 : 0));   
			break;
		case DRUID:
			STR += (lvl > 38 ? 2 : (lvl > 6 && (lvl%2) ? 1 : 0));
			STA += (lvl > 32 ? 2 : (lvl > 4 ? 1 : 0));   
			AGI += (lvl > 38 ? 2 : (lvl > 8 && (lvl%2) ? 1 : 0));
			INT += (lvl > 38 ? 3 : (lvl > 4 ? 1 : 0));   
			SPI += (lvl > 38 ? 3 : (lvl > 5 ? 1 : 0));  
	}
	
}	
	

void Player::smsg_InitialSpells()
{
    WorldPacket data;
    uint16 spellCount = m_spells.size();

    data.Initialize( SMSG_INITIAL_SPELLS );
    data << uint8(0);
    data << uint16(spellCount);                   

    std::list<Playerspell*>::iterator itr;
    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        data << uint16((*itr)->spellId);             
        data << uint16((*itr)->slotId);              
    }
    data << uint16(0);

    WPAssert(data.size() == 5+(4*spellCount));

    GetSession()->SendPacket(&data);

    sLog.outDetail( "CHARACTER: Sent Initial Spells" );
}


bidentry* Player::GetBid(uint32 id)
{
    std::list<bidentry*>::iterator itr;
    for (itr = m_bids.begin(); itr != m_bids.end();)
    {
        if ((*itr)->AuctionID == id)
        {
            return (*itr);
        }
        else
        {
            ++itr;
        }
    }
    return NULL;

}


void Player::AddBid(bidentry *be)
{
    std::list<bidentry*>::iterator itr;
    for (itr = m_bids.begin(); itr != m_bids.end();)
    {
        if ((*itr)->AuctionID == be->AuctionID)
        {
             
            
            m_bids.erase(itr++);
            
        }
        else
        {
            ++itr;
        }
    }
    m_bids.push_back(be);

}


void Player::RemoveMail(uint32 id)
{
    std::list<Mail*>::iterator itr;
    for (itr = m_mail.begin(); itr != m_mail.end();)
    {
        if ((*itr)->messageID == id)
        {
            m_mail.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
}


void Player::AddMail(Mail *m)
{
    std::list<Mail*>::iterator itr;
    for (itr = m_mail.begin(); itr != m_mail.end();)
    {
        if ((*itr)->messageID == m->messageID)
        {
            m_mail.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
    m_mail.push_back(m);
}


void Player::_SaveAuctions()
{
    sDatabase.PExecute("DELETE FROM auctionhouse WHERE itemowner = '%d'",GetGUIDLow());

    ObjectMgr::AuctionEntryMap::iterator itr;
    for (itr = objmgr.GetAuctionsBegin();itr != objmgr.GetAuctionsEnd();itr++)
    {
        AuctionEntry *Aentry = itr->second;
        if ((Aentry) && (Aentry->owner == GetGUIDLow()))
        {
            Item *it = objmgr.GetAItem(Aentry->item);

	    sDatabase.PExecute("DELETE FROM auctioned_items WHERE guid = '%u'",it->GetGUIDLow());
            
            sDatabase.PExecute("INSERT INTO auctionhouse (auctioneerguid, itemguid,itemowner,buyoutprice,time,buyguid,lastbid,Id) VALUES ('%d', '%d', '%d', '%d', '%u', '%d', '%d', '%d');", Aentry->auctioneer, Aentry->item, Aentry->owner, Aentry->buyout, Aentry->time, Aentry->bidder, Aentry->bid, Aentry->Id);

            std::stringstream ss;
            ss << "INSERT INTO auctioned_items (guid, data) VALUES ("
                << it->GetGUIDLow() << ", '";     
            for(uint16 i = 0; i < it->GetValuesCount(); i++ )
            {
                ss << it->GetUInt32Value(i) << " ";
            }
            ss << "' )";
            sDatabase.Execute( ss.str().c_str() );
        }
    }
}


void Player::_SaveMail()
{

    sDatabase.PExecute("DELETE FROM mail WHERE reciever = '%u'",GetGUIDLow());

    std::list<Mail*>::iterator itr;
    for (itr = m_mail.begin(); itr != m_mail.end(); itr++)
    {
        Mail *m = (*itr);

        QueryResult *result = sDatabase.PQuery("INSERT INTO mail (mailId,sender,reciever,subject,body,item,time,money,COD,checked) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->reciever, m->subject.c_str(), m->body.c_str(), m->item,  m->time, m->money, m->COD, m->checked);
        delete result;
    }
}


void Player::_SaveBids()
{
    sDatabase.PExecute("DELETE FROM bids WHERE bidder = '%d'",GetGUIDLow());

    std::list<bidentry*>::iterator itr;
    for (itr = m_bids.begin(); itr != m_bids.end(); itr++)
    {
        AuctionEntry *a = objmgr.GetAuction((*itr)->AuctionID);
        if (a)
        {
            sDatabase.PExecute("INSERT INTO bids (bidder, Id, amt) VALUES ('%d', '%d', '%d');", GetGUIDLow(), (*itr)->AuctionID, (*itr)->amt);
        }
    }

}


void Player::_LoadMail()
{

    m_mail.clear();

    QueryResult *result = sDatabase.PQuery("SELECT * FROM mail WHERE reciever = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            Mail *be = new Mail;
            be->messageID = fields[0].GetUInt32();
            be->sender = fields[1].GetUInt32();
            be->reciever = fields[2].GetUInt32();
            be->subject = fields[3].GetString();
            be->body = fields[4].GetString();
            be->item = fields[5].GetUInt32();
            be->time = fields[6].GetUInt32();
            be->money = fields[7].GetUInt32();
            be->COD = fields[8].GetUInt32();
            be->checked = fields[9].GetUInt32();
            m_mail.push_back(be);
        }
        while( result->NextRow() );

        delete result;
    }

}


void Player::_LoadBids()
{
    
    m_bids.clear();

    QueryResult *result = sDatabase.PQuery("SELECT Id,amt FROM bids WHERE bidder = '%d';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            bidentry *be = new bidentry;
            be->AuctionID = fields[0].GetUInt32();
            be->amt = fields[1].GetUInt32();
            m_bids.push_back(be);
        }
        while( result->NextRow() );

        delete result;
    }

}


void Player::addSpell(uint16 spell_id, uint16 slot_id)
{
	SpellEntry *spellInfo = sSpellStore.LookupEntry(spell_id);
	if(!spellInfo) return;
    
	Playerspell *newspell;
	
	newspell = new Playerspell;
	newspell->spellId = spell_id;
    
	uint8 op;
	uint16 tmpslot=slot_id,val=0;
	int16 tmpval=0;
	uint16 mark=0; 
	uint32 shiftdata=0x01;
	uint8  FlatId=0;
	uint32 EffectVal;
	uint32 Opcode=SMSG_SET_FLAT_SPELL_MODIFIER;
    
	WorldPacket data;

	if (tmpslot == 0xffff)
	{
		uint16 maxid = 0;
		std::list<Playerspell*>::iterator itr;
		for (itr = m_spells.begin(); itr != m_spells.end(); itr++)
		{
			if ((*itr)->slotId > maxid) maxid = (*itr)->slotId;
		}
		tmpslot = maxid + 1;
	}
	
	for(int i=0;i<3;i++)
	{
		if(spellInfo->EffectItemType[i])
		{
			EffectVal=spellInfo->EffectItemType[i];
			op=spellInfo->EffectMiscValue[i];
			tmpval = spellInfo->EffectBasePoints[i];

			if(tmpval != 0)
			{
				if(tmpval > 0){
					val =  tmpval+1;
					mark = 0x0;
				}else{
					val  = 0xFFFF + (tmpval+2);
					mark = 0xFFFF;
				}
			}

			switch(spellInfo->EffectApplyAuraName[i])
			{
				case 107:
					Opcode=SMSG_SET_FLAT_SPELL_MODIFIER; 
					break;
				case 108:
					Opcode=SMSG_SET_PCT_SPELL_MODIFIER;  
					break;
			}

			for(int i=0;i<32;i++)
			{
				if ( EffectVal&shiftdata )
				{
					FlatId=i;
                  
					data.Initialize(Opcode);
					data << uint8(FlatId);
					data << uint8(op);
					data << uint16(val);
					data << uint16(mark);
					m_session->SendPacket(&data);    
				}
				shiftdata=shiftdata<<1;
			}
		}
	}
	
	newspell->slotId = tmpslot;
	m_spells.push_back(newspell);
}



bool Player::removeSpell(uint16 spell_id)
{
    std::list<Playerspell*>::iterator itr;
    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if ((*itr)->spellId == spell_id)
        {
            m_spells.erase(itr);
            return true;
        }
    }
    return false;
}


Mail* Player::GetMail(uint32 id)
{
    std::list<Mail*>::iterator itr;
    for (itr = m_mail.begin(); itr != m_mail.end(); itr++)
    {
        if ((*itr)->messageID == id)
        {
            return (*itr);
        }
    }
    return NULL;
}







void Player::_SetCreateBits(UpdateMask *updateMask, Player *target) const
{
    if(target == this)
    {
        Object::_SetCreateBits(updateMask, target);
    }
    else
    {
        UpdateMask mask;
        mask.SetCount(m_valuesCount);
        _SetVisibleBits(&mask, target);

        for(uint16 index = 0; index < m_valuesCount; index++)
        {
            if(GetUInt32Value(index) != 0 && mask.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}


void Player::_SetUpdateBits(UpdateMask *updateMask, Player *target) const
{
    if(target == this)
    {
        Object::_SetUpdateBits(updateMask, target);
    }
    else
    {
        UpdateMask mask;
        mask.SetCount(m_valuesCount);
        _SetVisibleBits(&mask, target);

        Object::_SetUpdateBits(updateMask, target);
        *updateMask &= mask;
    }
}


void Player::_SetVisibleBits(UpdateMask *updateMask, Player *target) const
{
    updateMask->SetBit(OBJECT_FIELD_GUID);
    updateMask->SetBit(OBJECT_FIELD_TYPE);
    updateMask->SetBit(OBJECT_FIELD_SCALE_X);

    updateMask->SetBit(UNIT_FIELD_SUMMON);
    updateMask->SetBit(UNIT_FIELD_SUMMON+1);

    updateMask->SetBit(UNIT_FIELD_TARGET);
    updateMask->SetBit(UNIT_FIELD_TARGET+1);

    updateMask->SetBit(UNIT_FIELD_HEALTH);
    updateMask->SetBit(UNIT_FIELD_POWER1);
    updateMask->SetBit(UNIT_FIELD_POWER2);
    updateMask->SetBit(UNIT_FIELD_POWER3);
    updateMask->SetBit(UNIT_FIELD_POWER4);
    updateMask->SetBit(UNIT_FIELD_POWER5);

    updateMask->SetBit(UNIT_FIELD_MAXHEALTH);
    updateMask->SetBit(UNIT_FIELD_MAXPOWER1);
    updateMask->SetBit(UNIT_FIELD_MAXPOWER2);
    updateMask->SetBit(UNIT_FIELD_MAXPOWER3);
    updateMask->SetBit(UNIT_FIELD_MAXPOWER4);
    updateMask->SetBit(UNIT_FIELD_MAXPOWER5);

    updateMask->SetBit(UNIT_FIELD_LEVEL);
    updateMask->SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    updateMask->SetBit(UNIT_FIELD_BYTES_0);
    updateMask->SetBit(UNIT_FIELD_FLAGS);
    for(uint16 i = UNIT_FIELD_AURA; i < UNIT_FIELD_AURASTATE; i ++)
        updateMask->SetBit(i);
    updateMask->SetBit(UNIT_FIELD_BASEATTACKTIME);
    updateMask->SetBit(UNIT_FIELD_BASEATTACKTIME+1);
    updateMask->SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    updateMask->SetBit(UNIT_FIELD_COMBATREACH);
    updateMask->SetBit(UNIT_FIELD_DISPLAYID);
    updateMask->SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    updateMask->SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateMask->SetBit(UNIT_FIELD_BYTES_1);
    updateMask->SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateMask->SetBit(UNIT_FIELD_PETNUMBER);
    updateMask->SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    updateMask->SetBit(UNIT_DYNAMIC_FLAGS);

    updateMask->SetBit(PLAYER_BYTES);
    updateMask->SetBit(PLAYER_BYTES_2);
    updateMask->SetBit(PLAYER_BYTES_3);
	updateMask->SetBit(PLAYER_GUILDID);
	updateMask->SetBit(PLAYER_GUILDRANK);
    updateMask->SetBit(PLAYER_GUILD_TIMESTAMP);

    
    for(uint16 i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        
        updateMask->SetBit((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + i*2));
     
        updateMask->SetBit((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (i*2) + 1));
        
        
        
        updateMask->SetBit((uint16)(PLAYER_VISIBLE_ITEM_1_0 + (i*12)));
    }

    updateMask->SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO_01);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO_02);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO_03);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO_04);
    updateMask->SetBit(UNIT_VIRTUAL_ITEM_INFO_05);
}


void Player::BuildCreateUpdateBlockForPlayer( UpdateData *data, Player *target ) const
{
    
    for(int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i] == NULL)
            continue;

        m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
    }

    if(target == this)
    {
        
        for(int i = EQUIPMENT_SLOT_END; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
        }
    }

    Unit::BuildCreateUpdateBlockForPlayer( data, target );
}


void Player::DestroyForPlayer( Player *target ) const
{
    Unit::DestroyForPlayer( target );

    
    for(int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i] == NULL)
            continue;

        m_items[i]->DestroyForPlayer( target );
    }

    if(target == this)
    {
        
        for(int i = EQUIPMENT_SLOT_END; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer( target );
        }
    }
}

void Player::SaveToDB() {
	
	if (testStateFlag(PLAYER_IN_FLIGHT)) {
		SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
		RemoveFlag( UNIT_FIELD_FLAGS ,0x000004 );
		RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );
	}
	RemoveFlag( UNIT_FIELD_FLAGS, 0x40000 );

    
	_RemoveStatsMods();  
	_RemoveAllItemMods();
	_RemoveAllAuraMods();

	bool inworld = IsInWorld();
	if (inworld)
    RemoveFromWorld();

	std::stringstream ss;

	sDatabase.PExecute("DELETE FROM characters WHERE guid = '%u'",GetGUIDLow());


	ss.rdbuf()->str("");
	ss << "INSERT INTO characters (guid, acct, name, race, class, mapId, positionX, positionY, positionZ, orientation, data, taximask, online, highest_rank, last_week_rank) VALUES ("
        << GetGUIDLow() << ", "                   
        << GetSession()->GetAccountId() << ", '"
        << m_name << "', "
        << m_race << ", "
        << m_class << ", "
	
        << m_mapId << ", "
   
        << m_positionX << ", "
        << m_positionY << ", "
        << m_positionZ << ", "
        << m_orientation << ", '";

	uint16 i;
	for( i = 0; i < m_valuesCount; i++ )
        ss << GetUInt32Value(i) << " ";

	ss << "', '";

	for( i = 0; i < 8; i++ )
        ss << m_taximask[i] << " ";

	ss << "', ";
	inworld ? ss << 1 : ss << 0;
	
	ss << ", ";
	ss << m_highest_rank;
	
	ss << ", ";
	ss << m_last_week_rank;

	ss << " )";

	sDatabase.Execute( ss.str().c_str() );

	_SaveMail();
	_SaveBids();
	_SaveAuctions();
	_SaveInventory();
	_SaveQuestStatus();
	_SaveTutorials();
	_SaveSpells();
	_SaveActions();
	_SaveAuras();
	_SaveReputation();
	
	if(m_pCorpse) m_pCorpse->SaveToDB();
	
		
	_ApplyAllAuraMods();
	_ApplyAllItemMods();
	_ApplyStatsMods();

	if (inworld)
    AddToWorld();
}


void Player::_SaveQuestStatus()
{
    sDatabase.PExecute("DELETE FROM queststatus WHERE playerId = '%u'",GetGUIDLow());

    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
	    sDatabase.PExecute("INSERT INTO queststatus (playerId,questId,status,questMobCount1,questMobCount2,questMobCount3,questMobCount4,questItemCount1,questItemCount2,questItemCount3,questItemCount4) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), i->first, i->second.status, i->second.m_questMobCount[0], i->second.m_questMobCount[1], i->second.m_questMobCount[2], i->second.m_questMobCount[3], i->second.m_questItemCount[0], i->second.m_questItemCount[1], i->second.m_questItemCount[2], i->second.m_questItemCount[3]);
    }
}


void Player::_SaveInventory() {
	sDatabase.PExecute("DELETE FROM inventory WHERE player_guid = '%d'",GetGUIDLow());
    
	for(unsigned int i = 0; i < BANK_SLOT_BAG_END; i++) {
		if (m_items[i] != 0) {
			m_items[i]->SaveToDB();
			sDatabase.PExecute("INSERT INTO inventory VALUES ('%u', '%d', '%u', '%u');", GetGUIDLow(), i, m_items[i]->GetGUIDLow(), m_items[i]->GetEntry()); 
		}
	}
}


void Player::_SaveSpells()
{
    sDatabase.PExecute("DELETE FROM char_spells WHERE charId = '%u'",GetGUIDLow());

    std::list<Playerspell*>::iterator itr;
    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        sDatabase.PQuery("INSERT INTO char_spells (charId,spellId,slotId) VALUES ('%u', '%u', '%u');", GetGUIDLow(), (*itr)->spellId, (*itr)->slotId);
    }
}


void Player::_SaveTutorials()
{
    sDatabase.PExecute("DELETE FROM tutorials WHERE playerId = '%u'",GetGUIDLow());
    sDatabase.PExecute("INSERT INTO tutorials (playerId,tut0,tut1,tut2,tut3,tut4,tut5,tut6,tut7) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
}

void Player::_SaveActions()
{
    std::stringstream query;
    sDatabase.PExecute("DELETE FROM char_actions WHERE charId = '%d'",GetGUIDLow());

    std::list<struct actions>::iterator itr;
    for (itr = m_actions.begin(); itr != m_actions.end(); ++itr)
    {
        sDatabase.PExecute("INSERT INTO char_actions (charId,button,action,type,misc) VALUES ('%d', '%u', '%u', '%u', '%d');", GetGUIDLow(), int(itr->button), int(itr->action), int(itr->type), int(itr->misc));
    }
}


void Player::_SaveAuras()
{
}



void Player::LoadFromDB( uint32 guid )
{

    QueryResult *result = sDatabase.PQuery("SELECT * FROM characters WHERE guid = '%lu';",(unsigned long)guid);

    ASSERT(result);

    Field *fields = result->Fetch();

    Object::_Create( guid, HIGHGUID_PLAYER );


    LoadValues( fields[2].GetString() );

    
    m_name = fields[3].GetString();
    m_race = fields[4].GetUInt8();
	//Need to call it to initialize m_team (m_team can be calculated from m_race)
	//Other way is to saves m_team into characters table.
	setFaction(m_race, 0);
    
	m_class = fields[5].GetUInt8();

	info = objmgr.GetPlayerCreateInfo(m_race, m_class);

    m_positionX = fields[6].GetFloat();
    m_positionY = fields[7].GetFloat();
    m_positionZ = fields[8].GetFloat();
    m_mapId = fields[9].GetUInt32();
    m_orientation = fields[10].GetFloat();
	m_highest_rank = fields[13].GetUInt32();
	m_last_week_rank = fields[14].GetUInt32();

    if( HasFlag(PLAYER_FLAGS, 8) )
    SetUInt32Value(PLAYER_FLAGS, 0);

    if( HasFlag(PLAYER_FLAGS, 0x11) )
        m_deathState = DEAD;
	

    LoadTaxiMask( fields[11].GetString() );

    delete result;



    _LoadMail();

    _LoadInventory();

    _LoadSpells();

    _LoadActions();

    _LoadQuestStatus();

    _LoadTutorials();

    _LoadBids();

    _LoadAuras();

    _LoadReputation();
    
    _LoadCorpse();

    _ApplyAllAuraMods();
    _ApplyAllItemMods();

}


void Player::_LoadInventory() {
	for(uint16 i = 0; i < BANK_SLOT_BAG_END; i++) {
		if(m_items[i]) {
			delete m_items[i];
			SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + i*2), 0);
			m_items[i] = 0;
		}
	}
    
	QueryResult *result = sDatabase.PQuery("SELECT * FROM inventory WHERE player_guid = '%d';",GetGUIDLow());

	if (result) {
		uint8 slot;
		uint32 item_guid, item_id;
		Item* item;
		ItemPrototype* proto;

		do {
			Field *fields = result->Fetch();
			slot = fields[1].GetUInt8();
			item_guid = fields[2].GetUInt32();
			item_id = fields[3].GetUInt32();

			proto = objmgr.GetItemPrototype(item_id);

			if (proto->InventoryType == INVTYPE_BAG) {
				item = new Bag;
			} else {
				item = new Item;
			}

			item->SetOwner(this);
			item->LoadFromDB(item_guid, 1);
			AddItem(0, slot, item, false, false, true);
		} while (result->NextRow());

		delete result;
	}
}


bool Player::HasSpell(uint32 spell) 
{	
	std::list<Playerspell*>::iterator itr;
		
	SpellEntry *spellInfo = sSpellStore.LookupEntry(spell);
	
	if (!spellInfo) return true;
	
	// Look in the effects of spell , if is a Learn Spell Effect, see if is equal to triggerspell
	// If inst, look if have this spell.
	for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
	{
		for(uint8 i=0;i<3;i++)
		{
			if(spellInfo->Effect[i]==36) // Learn Spell effect
			{
			 if ( (*itr)->spellId == spellInfo->EffectTriggerSpell[i] )
				return true;
			}
			else if((*itr)->spellId == spellInfo->Id)
				return true;
		}
	}
			
	return false;
}


void Player::_LoadSpells()
{
    
    m_spells.clear();

    QueryResult *result = sDatabase.PQuery("SELECT spellId, slotId FROM char_spells WHERE charId = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            addSpell(fields[0].GetUInt16(), fields[1].GetUInt16());
        }
        while( result->NextRow() );

        delete result;
    }
}


void Player::_LoadTutorials()
{
    QueryResult *result = sDatabase.PQuery("SELECT * FROM tutorials WHERE playerId = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            for (int iI=0; iI<8; iI++) 
                m_Tutorials[iI] = fields[iI + 1].GetUInt32();

        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_LoadActions()
{
    
    m_actions.clear();

    QueryResult *result = sDatabase.PQuery("SELECT * FROM char_actions WHERE charId = '%d' ORDER BY button;",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            addAction(fields[1].GetUInt8(), fields[2].GetUInt16(), fields[3].GetUInt8(), fields[4].GetUInt8());
        }
        while( result->NextRow() );

        delete result;
    }
}



void Player::_LoadQuestStatus()
{

    mQuestStatus.clear();

	Quest *pQuest;

    QueryResult *result = sDatabase.PQuery("SELECT * FROM queststatus WHERE playerId = '%u';", GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            quest_status qs;
            qs.status              = fields[2].GetUInt32();
            qs.rewarded            = ( fields[3].GetUInt32() > 0 );
            qs.m_questMobCount[0]  = fields[4].GetUInt32();
            qs.m_questMobCount[1]  = fields[5].GetUInt32();
            qs.m_questMobCount[2]  = fields[6].GetUInt32();
            qs.m_questMobCount[3]  = fields[7].GetUInt32();
            qs.m_questItemCount[0] = fields[8].GetUInt32();
            qs.m_questItemCount[1] = fields[9].GetUInt32();
            qs.m_questItemCount[2] = fields[10].GetUInt32();
            qs.m_questItemCount[3] = fields[11].GetUInt32();
            qs.m_timerrel          = fields[12].GetUInt32();
            qs.m_explored          = ( fields[13].GetUInt32() > 0 );

            

            time_t q_abs = time(NULL);
            pQuest = objmgr.GetQuest(fields[1].GetUInt32());
			qs.m_quest = pQuest;

            if (pQuest && (pQuest->HasFlag(QUEST_SPECIAL_FLAGS_TIMED)) )
            {
                sLog.outDebug("Time now {%u}, then {%u} in quest {%u}!", q_abs, qs.m_timerrel, qs.m_quest->m_qId);
                if  ( qs.m_timerrel > q_abs ) 
                {
                    qs.m_timer = (qs.m_timerrel - q_abs) * 1000; 
                    sLog.outDebug("Setup timer at {%u}msec. for quest {%u}!", qs.m_timer, qs.m_quest->m_qId);
                    loadExistingQuest(qs);
                    m_timedQuest = qs.m_quest->m_qId;

                    continue;
                } else 
                {
                    sLog.outDebug("Timer expired for quest {%u}!", qs.m_quest->m_qId);
                    qs.m_timer    = 0;

                    if ( qs.status == QUEST_STATUS_COMPLETE )
                        qs.status     = QUEST_STATUS_INCOMPLETE;

                    qs.m_timerrel = 0;
                }
            }

            sLog.outDebug("Quest status is {%u} for quest {%u}", qs.status, qs.m_quest->m_qId);
            loadExistingQuest(qs);

        }
        while( result->NextRow() );

        delete result;
    }
}


void Player::_LoadAuras()
{
}


void Player::DeleteFromDB()
{
		uint32 guid = GetGUIDLow();
		
    sDatabase.PExecute("DELETE FROM characters WHERE guid = '%u'",guid);    
    sDatabase.PExecute("DELETE FROM char_spells WHERE charid = '%u'",guid);
    sDatabase.PExecute("DELETE FROM tutorials WHERE playerId = '%u'",guid);
    sDatabase.PExecute("DELETE FROM inventory WHERE player_guid = '%d'",guid);
    sDatabase.PExecute("DELETE FROM social WHERE guid = '%u'",guid);
    sDatabase.PExecute("DELETE FROM mail WHERE reciver = '%u'",guid);
    sDatabase.PExecute("DELETE FROM corpses WHERE player_guid = '%u'",guid);

    for(int i = 0; i < BANK_SLOT_ITEM_END; i++)
    {
        if(m_items[i] == NULL)
            continue;

        m_items[i]->DeleteFromDB();
    }

    sDatabase.PExecute("DELETE FROM queststatus WHERE playerId = '%u'",guid);    
    sDatabase.PExecute("DELETE FROM char_actions WHERE charId = '%d'",guid);    
    sDatabase.PExecute("DELETE FROM reputation WHERE playerID = '%d'",guid);
    sDatabase.PExecute("DELETE FROM honor WHERE guid = '%d'",guid);
    sDatabase.PExecute("DELETE FROM homebind WHERE guid = '%d'",guid);
    sDatabase.PExecute("DELETE FROM honor WHERE guid = '%d'",guid);
    sDatabase.PExecute("DELETE FROM kills WHERE killerID = '%d'",guid);
}

void Player::DeleteCorpse()
{
	if(m_pCorpse)
	{
		MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Remove(m_pCorpse,true);
		m_pCorpse = NULL;
	} 
}

void Player::SetMovement(uint8 pType)
{
    WorldPacket data;

    switch(pType)
    {
        case MOVE_ROOT:
        {
            data.Initialize(SMSG_FORCE_MOVE_ROOT);
            data << uint8(0xFF) << GetGUID();
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_UNROOT:
        {
            data.Initialize(SMSG_FORCE_MOVE_UNROOT);
            data << uint8(0xFF) << GetGUID();
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_WATER_WALK:
        {
            data.Initialize(SMSG_MOVE_WATER_WALK);
            data << uint8(0xFF) << GetGUID();
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_LAND_WALK:
        {
            data.Initialize(SMSG_MOVE_LAND_WALK);
            data << uint8(0xFF) << GetGUID();
            GetSession()->SendPacket( &data );
        }break;
        default:break;
    }
}


void Player::SetPlayerSpeed(uint8 SpeedType, float value, bool forced)
{
    WorldPacket data;

    switch(SpeedType)
    {
        case RUN:
        {
            if(forced) { data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_RUN_SPEED); }
	        data << uint8(0xFF);
            data << GetGUID();
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case RUNBACK:
        {
            if(forced) { data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED); }
	        data << uint8(0xFF);
            data << GetGUID();
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case SWIM:
        {
            if(forced) { data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_SWIM_SPEED); }
	        data << uint8(0xFF);
            data << GetGUID();
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case SWIMBACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED);
	        data << uint8(0xFF);
            data << GetGUID();
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        default:break;
    }
}


void Player::BuildPlayerRepop()
{
    WorldPacket data;
    
    SetUInt32Value( UNIT_FIELD_HEALTH, 1 );

    SetMovement(MOVE_WATER_WALK);
    SetMovement(MOVE_UNROOT);

	// setting new speed
	if (getRace() == RACE_NIGHT_ELF)
        {
	    SetPlayerSpeed(RUN, (float)12.75, true);
	    SetPlayerSpeed(SWIM, (float)8.85, true);
	}
	else {
	    SetPlayerSpeed(RUN, (float)10.625, true);
	    SetPlayerSpeed(SWIM, (float)7.375, true);
	}

	
	//! corpse reclaim delay 30 * 1000ms
    data.Initialize(SMSG_CORPSE_RECLAIM_DELAY );
    data << (uint32)30000;
    GetSession()->SendPacket( &data );


    data.Initialize(SMSG_SPELL_START );
    data << uint8(0xFF) << GetGUID() << uint8(0xFF) << GetGUID() << uint16(8326);
	data << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
    GetSession()->SendPacket( &data );


    data.Initialize(SMSG_SPELL_GO);
    data << uint8(0xFF) << GetGUID() << uint8(0xFF) << GetGUID() << uint16(8326);
    data << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << GetGUID(); /// uint8(0x0D) = probably race + 2
	data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
    GetSession()->SendPacket( &data );


	data.Initialize(SMSG_UPDATE_AURA_DURATION);
    data << uint32(0x20) << uint8(0);
    GetSession()->SendPacket( &data );




    StopMirrorTimer(0);									//disable timers(bars)
	StopMirrorTimer(1);
	StopMirrorTimer(2);

	SetUInt32Value(UNIT_FIELD_FLAGS, 0x08);
//	if (getRace() != RACE_NIGHT_ELF)	// only when not elf
	SetUInt32Value(UNIT_FIELD_AURA + 32, 8326);		// set ghost form 
	
	
	SetUInt32Value(UNIT_FIELD_AURA + 33, 0x5068 );		//!dono

	SetUInt32Value(UNIT_FIELD_AURAFLAGS + 4, 0xEE);

	SetUInt32Value(UNIT_FIELD_AURASTATE, 0x02);

	SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,(float)1.0);//see radius of death player?
	
	SetUInt32Value(UNIT_FIELD_BYTES_1, 0x1000000);		//Set standing so always be standing
	
	SetUInt32Value(PLAYER_FLAGS, 0x10);
    
    CreateCorpse();
}

void Player::SendDelayResponse(const uint32 ml_seconds) 
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );
    //data << (uint32)ml_seconds;
    data << (uint32)getMSTime();
    GetSession()->SendPacket( &data ); 
}


void Player::ResurrectPlayer()
{
    // remove death flag + set aura
    RemoveFlag(PLAYER_FLAGS, 0x10);

    // remove duel flags
    if( GetPvP() )
        RemoveFlag( UNIT_FIELD_FLAGS, 0x1000 );  

    setDeathState(ALIVE);

    GetSession()->GetPlayer()->SetMovement(MOVE_LAND_WALK);
    GetSession()->GetPlayer()->SetMovement(MOVE_UNROOT);

    GetSession()->GetPlayer( )->SetPlayerSpeed(RUN, (float)7.5, true);
    GetSession()->GetPlayer( )->SetPlayerSpeed(SWIM, (float)4.9, true);

    GetSession()->GetPlayer( )->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    GetSession()->GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    GetSession()->GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    GetSession()->GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    GetSession()->GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    GetSession()->GetPlayer( )->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    if(getRace() == NIGHTELF)                     
    {
        DeMorph();
    }

}


void Player::KillPlayer()
{
    WorldPacket data;

    SetMovement(MOVE_ROOT);

    StopMirrorTimer(0);
	StopMirrorTimer(1);
	StopMirrorTimer(2);

    setDeathState(CORPSE);
    SetFlag( UNIT_FIELD_FLAGS, 0x08 );            
		
    SetFlag( UNIT_DYNAMIC_FLAGS, 0x00 );
    
    if(getRace() == NIGHTELF)                     
    {
        this->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10045);
    }
}


void Player::CreateCorpse()
{
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    if(m_pCorpse)
    {
    	m_pCorpse->DeleteFromDB();
	DeleteCorpse();
    }
    
    m_pCorpse = new Corpse();
    m_pCorpse->Create(objmgr.GenerateLowGuid(HIGHGUID_CORPSE), this, GetMapId(), GetPositionX(),
       GetPositionY(), GetPositionZ(), GetOrientation());

    _uf = GetUInt32Value(UNIT_FIELD_BYTES_0);
    _pb = GetUInt32Value(PLAYER_BYTES);
    _pb2 = GetUInt32Value(PLAYER_BYTES_2);

    uint8 race       = (uint8)(_uf);
    uint8 skin       = (uint8)(_pb);
    uint8 face       = (uint8)(_pb >> 8);
    uint8 hairstyle  = (uint8)(_pb >> 16);
    uint8 haircolor  = (uint8)(_pb >> 24);
    uint8 facialhair = (uint8)(_pb2);

    _cfb1 = ((0x00) | (race << 8) | (0x00 << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    m_pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_1, _cfb1 );
    m_pCorpse->SetUInt32Value( CORPSE_FIELD_BYTES_2, _cfb2 );
    m_pCorpse->SetUInt32Value( CORPSE_FIELD_FLAGS, 4 );
    m_pCorpse->SetUInt32Value( CORPSE_FIELD_DISPLAY_ID, GetUInt32Value(UNIT_FIELD_DISPLAYID) );

    uint32 iDisplayID;
    uint16 iIventoryType;
    uint32 _cfi;
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
      if(m_items[i])
      {
        iDisplayID = m_items[i]->GetProto()->DisplayInfoID;
        iIventoryType = (uint16)m_items[i]->GetProto()->InventoryType;

        _cfi =  (uint16(iDisplayID)) | (iIventoryType)<< 24;
        m_pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
      }
    }
        
    MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Add(m_pCorpse);
}

void Player::SpawnCorpseBones()
{
    if(!m_pCorpse) return;

    m_pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, 5);
    m_pCorpse->SetUInt64Value(CORPSE_FIELD_OWNER, 0);
        
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
       if(m_pCorpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
          m_pCorpse->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
    }

    DEBUG_LOG("Deleting Corpse and swpaning bones.\n");

    WorldPacket data;
    data.Initialize(SMSG_DESTROY_OBJECT);
    data << (uint64)m_pCorpse->GetGUID();
    GetSession()->SendPacket(&data);

    MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Add(m_pCorpse);

    m_pCorpse->SaveToDB();
    m_pCorpse = NULL;

// TODO
// we need to add field timer in corpse table
// and after passing certain time we should call delete    

}


void Player::DeathDurabilityLoss(double percent)
{
    uint32 pDurability, pNewDurability;

    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(m_items[i])
        {
            pDurability =  m_items[i]->GetUInt32Value(ITEM_FIELD_DURABILITY);
            if(pDurability)
            {
                pNewDurability = (uint32)(pDurability*percent);
		if ( pNewDurability < 1 )
		{
		pNewDurability = 1;
		}
                pNewDurability = (pDurability - pNewDurability);

                if(pNewDurability < 0)
		{
		pNewDurability = 0;
		}

                m_items[i]->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);

		// we have durability 25% or 0 we should modify item stats
//		if ( pNewDurability == 0 || pNewDurability * 100 / pDurability < 25)
		if ( pNewDurability == 0 )
		{
		_ApplyItemMods(m_items[i],i, false);
		}



            }
        }
    }
}


void Player::RepopAtGraveyard()
{
    
    float closestX = 0, closestY = 0, closestZ = 0;
    WorldPacket data;    

    GraveyardTeleport *ClosestGrave = objmgr.GetClosestGraveYard( m_positionX, m_positionY, m_positionZ, GetMapId() );

    if(ClosestGrave)
    {
        closestX = ClosestGrave->X;
        closestY = ClosestGrave->Y;
        closestZ = ClosestGrave->Z;
        delete ClosestGrave;
    }
    

    if(closestX != 0 && closestY != 0 && closestZ != 0)
    {

	//GetSession()->GetPlayer()->smsg_NewWorld(GetMapId(), closestX, closestY, closestZ,0.0);

        WorldPacket data;

        BuildTeleportAckMsg(&data, closestX, closestY, closestZ, 0);
        GetSession()->SendPacket(&data);
        
        SetPosition(closestX, closestY, closestZ, 0);
        BuildHeartBeatMsg(&data);
        SendMessageToSet(&data, true);

    }

    
    
}


void Player::JoinedChannel(Channel *c)
{
    m_channels.push_back(c);
}


void Player::LeftChannel(Channel *c)
{
    m_channels.remove(c);
}


void Player::CleanupChannels()
{
    list<Channel *>::iterator i;
    for(i = m_channels.begin(); i != m_channels.end(); i++)
        (*i)->Leave(this,false);
}

void Player::BroadcastToFriends(std::string msg)
{
        Field *fields;
        Player *pfriend;
                        
	QueryResult *result = sDatabase.PQuery("SELECT * FROM social WHERE flags = 'FRIEND' AND guid = '%d';", GetGUID());

        if(!result) return;

        do
        {
            WorldPacket data;
            fields = result->Fetch();
            
            sChatHandler.FillSystemMessageData(&data, 0, msg.c_str());
            pfriend = ObjectAccessor::Instance().FindPlayer(fields[2].GetUInt64());

            if (pfriend && pfriend->IsInWorld())
                pfriend->GetSession()->SendPacket(&data);
                
        }while( result->NextRow() );
	delete result;
}
void Player::UpdateDefense() 
{
	UpdateSkill(SKILL_DEFENSE);
}

//skill+1, checking for max value
void Player::UpdateSkill(uint32 skill_id)
{
	if(!skill_id)return;
	uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
		if (GetUInt32Value(PLAYER_SKILL(i)) == skill_id) break;
	if(i>=PLAYER_MAX_SKILLS) return;
	
	uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
	uint16 value = SKILL_VALUE(data);
	uint16 max = SKILL_MAX(data);
	
	if ((!max) || (!value) || (value >= max)) return;
	
	if (!((value/max)*512 >= urand(0,512))) 
	{
		SetUInt32Value(PLAYER_SKILL(i)+1,data+1);

	}


}



void Player::UpdateSkillWeapon()
{
	
	Item *tmpitem = GetItemBySlot(EQUIPMENT_SLOT_MAINHAND);
	if (!tmpitem) tmpitem = GetItemBySlot(EQUIPMENT_SLOT_RANGED);

	if (!tmpitem)
		UpdateSkill(SKILL_UNARMED);
	else 
		UpdateSkill(GetSkillByProto(tmpitem->GetProto()));


}


void Player::ModifySkillBonus(uint32 skillid,int32 val)
{
	for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
		if (GetUInt32Value(PLAYER_SKILL(i)) == skillid)
		{
			
			SetUInt32Value(PLAYER_SKILL(i)+2,((int32)(GetUInt32Value(PLAYER_SKILL(i)+2)))+val);
			return;
		}
}
void Player::UpdateMaxSkills()
{
	for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
		if (GetUInt32Value(PLAYER_SKILL(i)))
		{
			uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
			uint32 max=data>>16;
			if(max!=1 && max != 300)
			SetUInt32Value(PLAYER_SKILL(i)+1,data%0x10000+GetUInt32Value(UNIT_FIELD_LEVEL)*5*0x10000);
		}
}



// This functions sets a skill line value (and adds if doesn't exist yet)
// To "remove" a skill line, set it's values to zero
void Player::SetSkill(uint32 id, uint16 currVal, uint16 maxVal)
{
	if(!id) return;
	uint16 i=0;
	for (; i < PLAYER_MAX_SKILLS; i++)
		if (GetUInt32Value(PLAYER_SKILL(i)) == id) break;
	
	if(i<PLAYER_MAX_SKILLS)//has skill
	{
		if(currVal)
			SetUInt32Value(PLAYER_SKILL(i)+1,currVal+maxVal*0x10000);
		else //remove
		{
			SetUInt64Value(PLAYER_SKILL(i),0);
			SetUInt32Value(PLAYER_SKILL(i)+2,0);
		}
	}else if(currVal)//add
	{
		
		for (i=0; i < PLAYER_MAX_SKILLS; i++)
		if (!GetUInt32Value(PLAYER_SKILL(i)))
		{
			SetUInt32Value(PLAYER_SKILL(i),id);
			SetUInt32Value(PLAYER_SKILL(i)+1,maxVal*0x10000+currVal);
			return;
		}
		
	}

}


uint16 Player::GetSkillValue(uint32 skill)
{
	if(!skill)return 0;
	for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
	{
		if (GetUInt32Value(PLAYER_SKILL(i)) == skill) 
		{
			return SKILL_VALUE(GetUInt32Value(PLAYER_SKILL(i)+1))+GetUInt32Value(PLAYER_SKILL(i)+2);
		}
	}
	return 0;
}

void Player::smsg_InitialActions()
{
    sLog.outString( "Initializing Action Buttons for '%u'", GetGUID() );
    WorldPacket data;
    uint16 actionCount = m_actions.size();
    uint16 button=0;

    std::list<struct actions>::iterator itr;
    data.Initialize(SMSG_ACTION_BUTTONS);
    for (itr = m_actions.begin(); itr != m_actions.end();)
    {
        if (itr->button == button)
        {
            data << uint16(itr->action);
            data << uint8(itr->type);
            data << uint8(itr->misc);
            ++itr;
        }
        else
        {
            data << uint32(0);
        }
        button++;
    }

    if (button < 120 )
    {
        for (int temp_counter=(120-button); temp_counter>0; temp_counter--)
        {
            data << uint32(0);
        }
    }
    GetSession()->SendPacket( &data );
    sLog.outString( "Action Buttons for '%u' Initialized", GetGUID() );
}


void Player::addAction(const uint8 button, const uint16 action, const uint8 type, const uint8 misc)
{
    bool ButtonExists = false;
    std::list<struct actions>::iterator itr;
    for (itr = m_actions.begin(); itr != m_actions.end(); ++itr)
    {
        if (itr->button == button)
        {
            itr->button=button;
            itr->action=action;
            itr->type=type;
            itr->misc=misc;
            ButtonExists = true;
            break;
        }
    }
    if (!ButtonExists)
    {
        struct actions newaction;
        newaction.button=button;
        newaction.action=action;
        newaction.type=type;
        newaction.misc=misc;
        m_actions.push_back(newaction);
    }
    sLog.outString( "Player '%u' Added Action '%u' to Button '%u'", GetGUID(), action, button );
}


void Player::removeAction(uint8 button)
{
    std::list<struct actions>::iterator itr;
    for (itr = m_actions.begin(); itr != m_actions.end(); ++itr)
    {
        if (itr->button == button)
        {
            m_actions.erase(itr);
            break;
        }
    }
    sLog.outString( "Action Button '%u' Removed from Player '%u'", button, GetGUID() );
}


void Player::SetDontMove(bool dontMove)
{
    m_dontMove = dontMove;
}



bool Player::IsGroupMember(Player *plyr)
{
    if(!plyr->IsInGroup())
        return false;
    Group *grp = objmgr.GetGroupByLeader(plyr->GetGroupLeader());
    if(grp->GroupCheck(plyr->GetGUID()))
    {
        return true;
    }
    return false;
}


void
Player::DealWithSpellDamage(DynamicObject &obj)
{
    obj.DealWithSpellDamage(*this);
}

bool
Player::SetPosition(const float &x, const float &y, const float &z, const float &orientation)
{
    Map *m = MapManager::Instance().GetMap(m_mapId);

    const float old_x = m_positionX;
    const float old_y = m_positionY;
	const float old_r = m_orientation;
	

    if( old_x != x || old_y != y || old_r != orientation)
    m->PlayerRelocation(this, x, y, z, orientation);
    
    CheckExploreSystem();

    return true;
}


void 
Player::SendMessageToSet(WorldPacket *data, bool self)
{
    MapManager::Instance().GetMap(m_mapId)->MessageBoardcast(this, data, self);
}


void Player::CheckExploreSystem()
{

	if (m_deathState & DEAD)
	return;

    WorldPacket data;
	uint16 areaFlag=MapManager::Instance().GetMap(GetMapId())->GetAreaFlag(m_positionX,m_positionY);
	if(areaFlag==0xffff)return;
	int offset = areaFlag / 32;
	uint32 val = (uint32)(1 << (areaFlag % 32));
	uint32 currFields = GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
            
	if( !(currFields & val) )
	{
		SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));
         
		AreaTableEntry *p =sAreaStore.LookupEntry(areaFlag);
		if(p->area_level)
		{
			uint32 XP = p->area_level*10;
			uint32 area = p->ID;
			GiveXP( XP, GetGUID() );
			data.Initialize( SMSG_EXPLORATION_EXPERIENCE );
			data << area; 
			data << XP;          
			m_session->SendPacket(&data);
			
			sLog.outDetail("PLAYER: Player %u discovered a new area: %u", GetGUID(), area);
		}
	}

}


void Player::setFaction(uint8 race, uint32 faction)
{
    
    if(race > 0)
    {
        m_faction = 0;
		m_team = 0;
        switch(race)
        {
            case HUMAN:
                m_faction = 1;
                m_team = (uint32)ALLIANCE;
                break;
            case DWARF:
                m_faction = 3;
                m_team = (uint32)ALLIANCE;
                break;
            case NIGHTELF:
                m_faction = 4;
                m_team = (uint32)ALLIANCE;
                break;
            case GNOME:
                m_faction = 8;
                m_team = (uint32)ALLIANCE;
                break;
            
            case ORC:
                m_faction = 2;
                m_team = (uint32)HORDE;
                break;
            case UNDEAD_PLAYER:
                m_faction = 5;
                m_team = (uint32)HORDE;
                break;
            case TAUREN:
                m_faction = 6;
                m_team = (uint32)HORDE;
                break;
            case TROLL:
                m_faction = 9;
                m_team = (uint32)HORDE;
                break;
            }
        } else m_faction = faction;
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, m_faction );
}

void Player::UpdateReputation()
{
    
    WorldPacket data;
    std::list<struct Factions>::iterator itr;

	sLog.outDebug( "WORLD: Player::UpdateReputation" );

    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {
		data.Initialize(SMSG_SET_FACTION_STANDING);
		data << (uint32) itr->Flags; 
        data << (uint32) itr->ReputationListID;
        data << (uint32) itr->Standing;
        GetSession()->SendPacket(&data);
    }
}

bool Player::FactionIsInTheList(uint32 faction)
{
    std::list<struct Factions>::iterator itr;

    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {
        if(itr->ReputationListID == faction) return true;
    }
    return false;
}

void Player::LoadReputationFromDBC(void)
{
    Factions newFaction;
    FactionEntry *fac = NULL;
    FactionTemplateEntry *fact = NULL;
    factions.clear();

    sLog.outDetail("PLAYER: LoadReputationFromDBC");
	uint32 force,oppos,forceop;
				if(m_team == ALLIANCE) { oppos = HORDE;force =891;forceop=892;}
		else { oppos = ALLIANCE;force = 892;forceop=891; }

		//this code seems to be totaly wrong (c) Phantomas
		//for x entries in FactionStore .... we get entry from FactionTemplateStore ???
		
	for(unsigned int i = 0; i < sFactionStore.GetNumRows(); i++)
    {

		fact = sFactionTemplateStore.LookupEntry(i); 
		if(!fact)continue;
		fac  = sFactionStore.LookupEntry( fact->faction );  
		if(!fac)continue;

        if( (fac->reputationListID >= 0) && (!FactionIsInTheList(fac->reputationListID)) )
        {
            newFaction.ID = fac->ID;
            newFaction.ReputationListID = fac->reputationListID;
            newFaction.Standing = 0;
	    newFaction.Flags = 0;
			
			if(fac->faction != 0&&fac->faction!=169)
			{
				if( (fac->faction == m_team || fac->faction == force) )
				{
					newFaction.Flags = fac->something6;
				}
				else if(fac->faction == oppos || fac->faction ==forceop )
				{
					newFaction.Flags = fac->something7;
				}
			}
			else 
			{
				if(fac->something6&&!fac->something7)
				{
					newFaction.Flags = fac->something6;
				}
				else if(fac->something7&&!fac->something6)
				{
					newFaction.Flags = fac->something7;
				}
				else if(fac->something6&&fac->something7)
				{
					if(fac->ID=469)
					{
					if(m_team == ALLIANCE) newFaction.Flags = fac->something6;
					else if(m_team==HORDE) newFaction.Flags = fac->something7;
					}
					else if(fac->ID ==67)
					{
					if(m_team == ALLIANCE) newFaction.Flags = fac->something7;
					else if(m_team==HORDE) newFaction.Flags = fac->something6;				
					}
					else
					{	
					if(m_team == ALLIANCE) newFaction.Flags = fac->something6;
					else newFaction.Flags = fac->something7;
					}
					
				}
			}
			factions.push_back(newFaction);
		}
		}	
}
void Player::_LoadReputation()
{
    Factions newFaction;
    
    factions.clear();

    QueryResult *result = sDatabase.PQuery("SELECT factionID, reputationID, standing, flags FROM reputation WHERE playerID = '%d';",GetGUID());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            newFaction.ID               = fields[0].GetUInt32();
            newFaction.ReputationListID = fields[1].GetUInt32();
            newFaction.Standing         = fields[2].GetUInt32();
            newFaction.Flags            = fields[3].GetUInt32();

            factions.push_back(newFaction);
        }
        while( result->NextRow() );

        delete result;
    }
    else
    {
        LoadReputationFromDBC();
    }
}

void Player::_LoadCorpse()
{
	// TODO do we need to load all corpses ?
	QueryResult *result = sDatabase.PQuery("SELECT * FROM corpses WHERE player_guid = '%u';",GetGUIDLow());

	if(!result) return;
	
	Field *fields = result->Fetch();

	//DeleteCorpse();
	m_pCorpse = new Corpse();
	
	float positionX = fields[2].GetFloat();
	float positionY = fields[3].GetFloat();
	float positionZ = fields[4].GetFloat();
	float ort       = fields[5].GetFloat();
	uint32 mapid     = fields[6].GetUInt32();
	//uint32 zoneid    = fields[7].GetUInt32();
	
	m_pCorpse->Relocate(positionX,positionY,positionZ,ort);
	m_pCorpse->SetMapId(mapid);
	//m_pCorpse->SetZoneId(zoneid);
	m_pCorpse->LoadValues( fields[8].GetString() );

	MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Add(m_pCorpse);

	delete result;
}

void Player::_SaveReputation()
{
    std::list<Factions>::iterator itr;
    
    std::stringstream ss;

    sDatabase.PExecute("DELETE FROM reputation WHERE playerID = '%d'",GetGUID());
    
    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {

        sDatabase.PExecute("INSERT INTO reputation (playerID, factionID, reputationID, standing, flags) VALUES ('%d', '%d', '%d', '%d', '%d');", (uint32)GetGUID(), itr->ID, itr->ReputationListID, itr->Standing, itr->Flags);
        
    }
}

bool Player::SetStanding(uint32 FTemplate, int standing)
{
    FactionEntry *fac = NULL;
    FactionTemplateEntry *fact = NULL;
    std::list<struct Factions>::iterator itr;
    fact = sFactionTemplateStore.LookupEntry(FTemplate);

    if( fact != NULL )
    {
        assert( fact->ID == FTemplate );
        fac  = sFactionStore.LookupEntry( fact->faction );
        for(itr = factions.begin(); itr != factions.end(); ++itr)
        {
            if(itr->ReputationListID == fac->reputationListID) 
            {
                itr->Standing = (((int)itr->Standing + standing) > 0 ? itr->Standing + standing : 0);
                itr->Flags = (itr->Flags | 1); 
                UpdateReputation();
                return true;
            }
        }   
    }

    return false;
}




void Player::UpdateHonor(void)
{
	WorldPacket data;
	
	time_t rawtime;
	struct tm * now;
	uint32 today = 0;
	uint32 date = 0;
	
	uint32 Yestarday = 0;
	uint32 ThisWeekBegin = 0;
	uint32 ThisWeekEnd = 0;
	uint32 LastWeekBegin = 0;
	uint32 LastWeekEnd = 0;
	
	uint32 lifetime_honorableKills = 0;
	uint32 lifetime_dishonorableKills = 0;
	uint32 today_honorableKills = 0;
	uint32 today_dishonorableKills = 0;

	uint32 yestardayKills = 0;
	uint32 thisWeekKills = 0;
	uint32 lastWeekKills = 0;

	int total_honor = 0;
	int yestardayHonor = 0;
	int thisWeekHonor = 0;
	int lastWeekHonor = 0;

	
	time( &rawtime );
	now = localtime( &rawtime );
	
	today = ((uint32)(now->tm_year << 16)|(uint32)(now->tm_yday));
	
	Yestarday     = today - 1;
	ThisWeekBegin = today - now->tm_wday;
	ThisWeekEnd   = ThisWeekBegin + 7;
	LastWeekBegin = ThisWeekBegin - 7;
	LastWeekEnd   = LastWeekBegin + 7;

	sLog.outDetail("PLAYER: UpdateHonor");
    
	QueryResult *result = sDatabase.PQuery("SELECT type, honor_pts, date FROM kills WHERE killerID = '%d';", GetGUID());
	
	if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            if(fields[0].GetUInt32() == HONORABLE_KILL) lifetime_honorableKills++;
			else
			if(fields[0].GetUInt32() == DISHONORABLE_KILL) lifetime_dishonorableKills++;
			
			total_honor += fields[1].GetUInt32();

			date = fields[2].GetUInt32();

			if(	date == today)
			{
				if(fields[0].GetUInt32() == HONORABLE_KILL)
					today_honorableKills++;
				else
				if(fields[0].GetUInt32() == DISHONORABLE_KILL)
					today_dishonorableKills++;
			}
			if(	date == Yestarday)
			{
				if(fields[0].GetUInt32() == HONORABLE_KILL)
				{
					yestardayKills++;
					yestardayHonor += fields[1].GetUInt32();
				}
			}
			if( (date >= ThisWeekBegin) && (date < ThisWeekEnd) )
			{
				if(fields[0].GetUInt32() == HONORABLE_KILL)
				{
					thisWeekKills++;
					thisWeekHonor += fields[1].GetUInt32();
				}
			}
			if( (date >= LastWeekBegin) && (date < LastWeekEnd) )
			{
				if(fields[0].GetUInt32() == HONORABLE_KILL)
				{
					lastWeekKills++;
					lastWeekHonor += fields[1].GetUInt32();
				}
			}

        }
        while( result->NextRow() );
		
        delete result;
    }
	
	//LIFE TIME
	SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, (lifetime_dishonorableKills << 16) + lifetime_honorableKills );
	SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, lifetime_dishonorableKills);		
	SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, lifetime_honorableKills);
	if ( GetHonorHighestRank() )
		SetUInt32Value(PLAYER_FIELD_HONOR_HIGHEST_RANK, ((uint32) GetHonorHighestRank() << 24) + 0x040F0001 );
	else
		SetUInt32Value(PLAYER_FIELD_HONOR_HIGHEST_RANK, 0);
	//TODAY
	SetUInt32Value(PLAYER_FIELD_TODAY_KILLS, (today_dishonorableKills << 16) + today_honorableKills );
	//YESTERDAY
	SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, yestardayKills);
	SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONOR, yestardayHonor);
	//THIS WEEK
	SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONORABLE_KILLS, thisWeekKills);
	SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONOR, thisWeekHonor);
	//LAST WEEK
	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONORABLE_KILLS, lastWeekKills);
	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONOR, lastWeekHonor);
	SetUInt32Value(PLAYER_FIELD_LAST_WEEK_STANDING, GetHonorLastWeekRank());

	//RANK BAR //Total honor points
	SetUInt32Value(PLAYER_FIELD_LIFETIME_HONOR, total_honor );
	if( CalculateHonorRank(total_honor) )
		SetUInt32Value(PLAYER_FIELD_HONOR_RANK, (( (uint32)CalculateHonorRank(total_honor) << 24) + 0x04000000) );
	else
		SetUInt32Value(PLAYER_FIELD_HONOR_RANK, 0);

	//Store Total Honor points...
	m_total_honor_points = total_honor;
}


int Player::CalculateHonorRank(int honor_points)
{
	int rank = 0;

	if(honor_points <=    0) rank =  0; else
	if(honor_points <   999) rank =  1;	else
	if(honor_points <  4999) rank =  2;	else
	if(honor_points <  9999) rank =  3;	else
	if(honor_points < 14999) rank =  4;	else
	if(honor_points < 19999) rank =  5;	else
	if(honor_points < 24999) rank =  6;	else
	if(honor_points < 29999) rank =  7;	else
	if(honor_points < 34999) rank =  8;	else
	if(honor_points < 39999) rank =  9;	else
	if(honor_points < 44999) rank = 10;	else
	if(honor_points < 49999) rank = 11;	else
	if(honor_points < 54999) rank = 12;	else
	if(honor_points < 59999) rank = 13;	else
	if(honor_points < 65000) rank = 14; else
		rank = 15;

	return rank;
}


int Player::CalculateTotalKills(Player *pVictim)
{
	int total_kills = 0;

	QueryResult *result = sDatabase.PQuery("SELECT honor_pts FROM kills WHERE killerID = '%d' AND victimID = '%d';", GetGUID(), pVictim->GetGUID());
	
	if(result)
    {
		total_kills = result->GetRowCount();		
        delete result;
    }
	return total_kills;
}


void Player::CalculateHonor(Player *pVictim)
{
	int parcial_honor_points = 0;
	
	int kill_type = 0;

	sLog.outDetail("PLAYER: CalculateHonor");
	
	

	
	if ( ( pVictim )&&( GetTeam() != pVictim->GetTeam() ) )
	{	
		uint16 honorableKills = (uint16) GetUInt32Value(PLAYER_FIELD_SESSION_KILLS);
		uint16 unhonorableKills = (uint16)((GetUInt32Value(PLAYER_FIELD_SESSION_KILLS) - honorableKills) / 65536);
	
		
		int total_kills = CalculateTotalKills(pVictim);

		int k_rank = CalculateHonorRank(GetTotalHonor());
		int v_rank = pVictim->CalculateHonorRank( pVictim->GetTotalHonor() );
		int k_level = GetLevel();
		int v_level = pVictim->GetLevel();
		int diff_honor = (pVictim->GetTotalHonor() /(GetTotalHonor()+1))+1;
		int diff_level = (uint8)(v_level*(1.0/(k_level)));

		if ((GetLevel() - pVictim->GetLevel() ) >= 5 )
		{
			kill_type = DISHONORABLE_KILL;
			parcial_honor_points = 0;
		}
		else
		{
			kill_type = HONORABLE_KILL;
			
			int f = (4 - total_kills) >= 0 ? (4 - total_kills) : 0;
			parcial_honor_points = (int)((float)(f * 0.25)*(float)((k_level+(v_rank*5+1))*(1+0.05*diff_honor)*diff_level));
			
			parcial_honor_points = (parcial_honor_points <= 400) ? parcial_honor_points : 400;
		}

		
		time_t rawtime;
		struct tm * now;
		uint32 today = 0;
		time( &rawtime );
		now = localtime( &rawtime );
		
		today = ((uint32)(now->tm_year << 16)|(uint32)(now->tm_yday));
		
		sDatabase.PExecute("INSERT INTO kills (killerID, victimID, honor_pts, date, type) VALUES (%d, %d, %d, %d, %u);", (uint32)GetGUID(), (uint32)pVictim->GetGUID(), (uint32)parcial_honor_points, (uint32)today, (uint8)kill_type);

		printf("INSERT KILL: (%d, %d, %d, %d, %u)", (uint32)GetGUID(), (uint32)pVictim->GetGUID(), (uint32)parcial_honor_points, (uint32)today, (uint8)kill_type);

		UpdateHonor();
	}
}

void Player::DuelComplete()
{  
    if(!m_pDuel) return;

		WorldPacket data;
		uint64 duelFlagGUID = GetUInt64Value(PLAYER_DUEL_ARBITER);

    data.Initialize(SMSG_DUEL_WINNER);
    data << (uint8)0;
    data << m_pDuel->GetName();
    data << (uint8)0;
    data << GetName();
    GetSession()->SendPacket(&data);
    m_pDuel->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DUEL_COMPLETE);
    data << (uint8)1;                             
    GetSession()->SendPacket(&data);
    m_pDuel->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << duelFlagGUID;
    GetSession()->SendPacket(&data);
    m_pDuel->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DESTROY_OBJECT);
    data << duelFlagGUID;
    GetSession()->SendPacket(&data);
    m_pDuel->GetSession()->SendPacket(&data);

    m_isInDuel = false;
    m_pDuel->m_isInDuel = false;

    m_deathState = ALIVE;  

    GameObject* obj = NULL;
    obj = ObjectAccessor::Instance().GetGameObject(*this, duelFlagGUID);

    if(obj)
			MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
				
		SetUInt64Value(PLAYER_DUEL_ARBITER,0);
		SetUInt32Value(PLAYER_DUEL_TEAM,0);
   	m_pDuel->SetUInt64Value(PLAYER_DUEL_ARBITER,0);
    m_pDuel->SetUInt32Value(PLAYER_DUEL_TEAM,0);
}



static unsigned long    holdrand = 0x89abcdef;

void Rand_Init(int seed)
{
    holdrand = seed;
}

int irand(int min, int max)
{
    int        result;

    assert((max - min) < 32768);

    max++;
    holdrand = (holdrand * 214013L) + 2531011L;
    result = holdrand >> 17;
    result = ((result * (max - min)) >> 15) + min;
    return(result);
}

uint32 urand(uint32 min, uint32 max)
{
    uint32 result = irand(int(min), int(max));
    return result;
}







void Player::smsg_AttackStart(Unit* pVictim)
{
    WorldPacket data;

    
    if(!isAttackReady())
    {
        setAttackTimer(uint32(0));
    }

    
    data.Initialize( SMSG_ATTACKSTART );
    data << GetGUID();
    data << pVictim->GetGUID();
    SendMessageToSet(&data, true);
    DEBUG_LOG( "WORLD: Sent SMSG_ATTACKSTART" );
}

//---------------------------------------------------------//
//       Flight callback
void Player::FlightComplete()
{
    clearStateFlag(PLAYER_IN_FLIGHT);
    SetUInt32Value( PLAYER_FIELD_COINAGE , m_dismountCost);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

    /* Remove the "player locked" flag, to allow movement */
    if (GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
	RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );
}



/**********************************************************
***                    STORAGE SYSTEM                   ***
***********************************************************/


void Player::SetSheath(uint32 sheathed) {
	if (sheathed) {
		Item *item=NULL;
       
		if (GetItemBySlot(EQUIPMENT_SLOT_MAINHAND))
			item = GetItemBySlot(EQUIPMENT_SLOT_MAINHAND);
		if (GetItemBySlot(EQUIPMENT_SLOT_OFFHAND))
			item = GetItemBySlot(EQUIPMENT_SLOT_OFFHAND);
		if (GetItemBySlot(EQUIPMENT_SLOT_RANGED))
			item = GetItemBySlot(EQUIPMENT_SLOT_RANGED);
		if (!item) 
			return;

     ItemPrototype *itemProto = item->GetProto();
	  uint32 itemSheathType = itemProto->Sheath;

	  if (GetItemBySlot(EQUIPMENT_SLOT_MAINHAND)) {
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, item->GetGUIDLow());
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_01, itemSheathType);
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, itemProto->DisplayInfoID);
	  }
	  if (GetItemBySlot(EQUIPMENT_SLOT_OFFHAND)) {
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_02, item->GetGUIDLow());
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_03, itemSheathType);
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, itemProto->DisplayInfoID);
	  }
	  if (GetItemBySlot(EQUIPMENT_SLOT_RANGED)) {
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_04, item->GetGUIDLow());
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_05, itemSheathType);
		  this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, itemProto->DisplayInfoID);
	  }
	} else {
		if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0)) {
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_01, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, uint32(0));
		}
		if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1)) {
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_02, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_03, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, uint32(0));
		}
		if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2)) {
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_04, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_05, uint32(0));
			this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, uint32(0));
		}
	}
}

bool Player::CanUseItem(ItemPrototype * proto) {
	uint32 reqSpell = 0;
	uint32 reqSkill = proto->RequiredSkill;
	uint32 reqSkillRank = proto->RequiredSkillRank;
	uint8 error_code = EQUIP_ERR_OK;
	
	if (getLevel() < proto->RequiredLevel) error_code = EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;
	
	if (error_code) {
		WorldPacket	data;
		Item* pItem = new Item();
		pItem->Create (objmgr.GenerateLowGuid (HIGHGUID_ITEM), proto->ItemId, this);
		
		data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);

		data << error_code;
		if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N) {
			data << proto->RequiredLevel;
		}
		data << (pItem ? pItem->GetGUID() : uint64(0));
		data << uint64(0);
		data << uint8(0);

		GetSession()->SendPacket (&data);
		delete pItem;
		pItem = NULL;
		return false;
	} else {
		return true;
	}
}

uint32 Player::GetSkillByProto(ItemPrototype *proto)
{
	const static uint32 item_weapon_skills[]={
		SKILL_AXES,	SKILL_2H_AXES,SKILL_BOWS, SKILL_GUNS,SKILL_MACES, SKILL_2H_MACES,
		SKILL_POLEARMS,	SKILL_SWORDS,SKILL_2H_SWORDS,0, SKILL_STAVES,0,0,0,0, SKILL_DAGGERS,
		SKILL_THROWN, SKILL_SPEARS, SKILL_CROSSBOWS, SKILL_WANDS, SKILL_FISHING};

	const static uint32 item_armor_skills[]={
		0,SKILL_CLOTH,SKILL_LEATHER,SKILL_MAIL,SKILL_PLATE_MAIL,0,SKILL_SHIELD};
	

	switch (proto->Class)
	{
		case ITEM_CLASS_WEAPON:
		if(proto->SubClass>=sizeof(item_weapon_skills)/4)return 0;
		else return item_weapon_skills[proto->SubClass];

		case ITEM_CLASS_ARMOR:
		if(proto->SubClass>=sizeof(item_armor_skills)/4)return 0;
		else return item_armor_skills[proto->SubClass];
	}



/*
	switch (proto->Class) {
		case ITEM_CLASS_WEAPON:
		switch (proto->SubClass) {
				case ITEM_SUBCLASS_WEAPON_AXE: return SKILL_AXES;
				case ITEM_SUBCLASS_WEAPON_AXE2: return SKILL_2H_AXES;
				case ITEM_SUBCLASS_WEAPON_BOW: return SKILL_BOWS;
				case ITEM_SUBCLASS_WEAPON_GUN: return SKILL_GUNS;
				case ITEM_SUBCLASS_WEAPON_MACE: return SKILL_MACES;
				case ITEM_SUBCLASS_WEAPON_MACE2: return SKILL_2H_MACES;
				case ITEM_SUBCLASS_WEAPON_POLEARM: return SKILL_POLEARMS;
				case ITEM_SUBCLASS_WEAPON_SWORD: return SKILL_SWORDS;
				case ITEM_SUBCLASS_WEAPON_SWORD2: return SKILL_2H_SWORDS;
				case ITEM_SUBCLASS_WEAPON_STAFF: return SKILL_STAVES;
				case ITEM_SUBCLASS_WEAPON_DAGGER: return SKILL_DAGGERS;
				case ITEM_SUBCLASS_WEAPON_THROWN: return SKILL_THROWN;
				case ITEM_SUBCLASS_WEAPON_SPEAR: return SKILL_SPEARS;
				case ITEM_SUBCLASS_WEAPON_CROSSBOW: return SKILL_CROSSBOWS;
				case ITEM_SUBCLASS_WEAPON_WAND: return SKILL_WANDS;
				case ITEM_SUBCLASS_WEAPON_FISHING_POLE: return SKILL_FISHING;
				default: return 0;
			}
		case ITEM_CLASS_ARMOR:
			switch(proto->SubClass) {
				case ITEM_SUBCLASS_ARMOR_CLOTH: return SKILL_CLOTH;
				case ITEM_SUBCLASS_ARMOR_LEATHER: return SKILL_LEATHER;
				case ITEM_SUBCLASS_ARMOR_MAIL: return SKILL_MAIL;
				case ITEM_SUBCLASS_ARMOR_PLATE: return SKILL_PLATE_MAIL;
				case ITEM_SUBCLASS_ARMOR_SHIELD: return SKILL_SHIELD;
				default: return 0;
			}
	}*/
	return 0;
}

uint32 Player::GetSpellByProto(ItemPrototype *proto) {
	switch (proto->Class) {
		case ITEM_CLASS_WEAPON:
			switch (proto->SubClass) {
				case ITEM_SUBCLASS_WEAPON_AXE: return 196;
				case ITEM_SUBCLASS_WEAPON_AXE2: return 197;
				case ITEM_SUBCLASS_WEAPON_BOW: return 264;
				case ITEM_SUBCLASS_WEAPON_GUN: return 266;
				case ITEM_SUBCLASS_WEAPON_MACE: return 198;
				case ITEM_SUBCLASS_WEAPON_MACE2: return 199;
				case ITEM_SUBCLASS_WEAPON_POLEARM: return 200;
				case ITEM_SUBCLASS_WEAPON_SWORD: return 201;
				case ITEM_SUBCLASS_WEAPON_SWORD2: return 202;
				case ITEM_SUBCLASS_WEAPON_STAFF: return 227;
				case ITEM_SUBCLASS_WEAPON_DAGGER: return 1180;
				case ITEM_SUBCLASS_WEAPON_THROWN: return 2567;
				case ITEM_SUBCLASS_WEAPON_SPEAR: return 3386;
				case ITEM_SUBCLASS_WEAPON_CROSSBOW: return 5011;
				case ITEM_SUBCLASS_WEAPON_WAND: return 5009;
				default: return 0;
			}
		case ITEM_CLASS_ARMOR:
			switch(proto->SubClass) {
				case ITEM_SUBCLASS_ARMOR_CLOTH: return 9078;
				case ITEM_SUBCLASS_ARMOR_LEATHER: return 9077;
				case ITEM_SUBCLASS_ARMOR_MAIL: return 8737;
				case ITEM_SUBCLASS_ARMOR_PLATE: return 750;
				case ITEM_SUBCLASS_ARMOR_SHIELD: return 9116;
				default: return 0;
			}
	}
	return 0;
}

void Player::GetSlotByItem(uint32 type, uint8 slots[4]) {
	for (int i = 0; i < 4; i++) slots[i] = NULL_SLOT;
	switch(type) {
		case INVTYPE_HEAD:
			slots[0] = EQUIPMENT_SLOT_HEAD;
			break;
		case INVTYPE_NECK:
			slots[0] = EQUIPMENT_SLOT_NECK;
			break;
		case INVTYPE_SHOULDERS:
			slots[0] = EQUIPMENT_SLOT_SHOULDERS;
			break;
		case INVTYPE_BODY:
			slots[0] = EQUIPMENT_SLOT_BODY;
			break;
		case INVTYPE_CHEST:
			slots[0] = EQUIPMENT_SLOT_CHEST;
			break;
		case INVTYPE_ROBE:
			slots[0] = EQUIPMENT_SLOT_CHEST;
			break;
		case INVTYPE_WAIST:
			slots[0] = EQUIPMENT_SLOT_WAIST;
			break;
		case INVTYPE_LEGS:
			slots[0] = EQUIPMENT_SLOT_LEGS;
			break;
		case INVTYPE_FEET:
			slots[0] = EQUIPMENT_SLOT_FEET;
			break;
		case INVTYPE_WRISTS:
			slots[0] = EQUIPMENT_SLOT_WRISTS;
			break;
		case INVTYPE_HANDS:
			slots[0] = EQUIPMENT_SLOT_HANDS;
			break;
		case INVTYPE_FINGER:
			slots[0] = EQUIPMENT_SLOT_FINGER1;
			slots[1] = EQUIPMENT_SLOT_FINGER2;
			break;
		case INVTYPE_TRINKET:
			slots[0] = EQUIPMENT_SLOT_TRINKET1;
			slots[1] = EQUIPMENT_SLOT_TRINKET2;
			break;
		case INVTYPE_CLOAK:
			slots[0] =  EQUIPMENT_SLOT_BACK;
			break;
		case INVTYPE_WEAPON:
			slots[0] = EQUIPMENT_SLOT_MAINHAND;
			slots[1] = EQUIPMENT_SLOT_OFFHAND;
			break;
		case INVTYPE_SHIELD:
			slots[0] = EQUIPMENT_SLOT_OFFHAND;
			break;
		case INVTYPE_RANGED:
			slots[0] = EQUIPMENT_SLOT_RANGED;
			break;
		case INVTYPE_2HWEAPON:
			slots[0] = EQUIPMENT_SLOT_MAINHAND;
			break;
		case INVTYPE_TABARD:
			slots[0] = EQUIPMENT_SLOT_TABARD;
			break;
		case INVTYPE_WEAPONMAINHAND:
			slots[0] = EQUIPMENT_SLOT_MAINHAND;
			break;
		case INVTYPE_WEAPONOFFHAND:
			slots[0] = EQUIPMENT_SLOT_OFFHAND;
			break;
		case INVTYPE_HOLDABLE:
			slots[0] = EQUIPMENT_SLOT_MAINHAND;
			break;
		case INVTYPE_THROWN:
			slots[0] = EQUIPMENT_SLOT_RANGED;
			break;
		case INVTYPE_RANGEDRIGHT:
			slots[0] = EQUIPMENT_SLOT_RANGED;
			break;
		case INVTYPE_BAG:
			slots[0] = INVENTORY_SLOT_BAG_1;
			slots[1] = INVENTORY_SLOT_BAG_2;
			slots[2] = INVENTORY_SLOT_BAG_3;
			slots[3] = INVENTORY_SLOT_BAG_4;
			break;
	}
}

uint8 Player::FindEquipSlot(uint32 type) {
	uint8 slots[4];
	GetSlotByItem(type, slots);
	if (slots[0] == NULL_SLOT) return INVENTORY_SLOT_ITEM_END;
	for (int i = 0; i < 4; i++) {
		if (slots[i] != NULL_SLOT) 
			if (!GetItemBySlot(slots[i])) return slots[i];
	}
	return slots[0];
}

uint8 Player::FindFreeItemSlot(uint32 type) {
	uint8 slots[4];
	GetSlotByItem(type, slots);
	if (slots[0] == NULL_SLOT) return INVENTORY_SLOT_ITEM_END;
	for (int i = 0; i < 4; i++) {
		if (slots[i] != NULL_SLOT) 
			if (!GetItemBySlot(slots[i])) return slots[i];
	}
	return INVENTORY_SLOT_ITEM_END;
}

int Player::CountFreeBagSlot() {
	int count = 0;
	for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) {
		if (!GetItemBySlot(i)) count++;         
	}
	return count;
}

// Checks if player can equip something
uint8 Player::CanEquipItemInSlot(uint8 bagIndex, uint8 slot, Item* item, Item* swapitem) 
{
	
	ItemPrototype* proto=item->GetProto();
	assert(proto);

	uint32 type = proto->InventoryType;

	if (isDead())  return EQUIP_ERR_YOU_ARE_DEAD; 

	if (item->GetOwner() != this) 
		return EQUIP_ERR_DONT_OWN_THAT_ITEM; 

	if ((!bagIndex) || (bagIndex == CLIENT_SLOT_BACK))
	{ // Player slots, inventory and bank slots
		if (slot < EQUIPMENT_SLOT_END) 
		{ // Equiping item

			if (!(proto->AllowableRace & getRace())) 
				return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM; 
			if (!(proto->AllowableClass & getClass()))
				return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM; 
			if (proto->RequiredLevel > GetLevel())  
				return EQUIP_ERR_YOU_MUST_REACH_LEVEL_N; 


			uint8 slots[4];
			GetSlotByItem(type, slots);
			if(slots[0]!=slot && slots[1]!=slot && slots[2]!=slot && slots[3]!=slot)
				return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;


			if ((slot == EQUIPMENT_SLOT_MAINHAND) && (type == INVTYPE_2HWEAPON) && (GetItemBySlot(EQUIPMENT_SLOT_OFFHAND))) 
				return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED; 

			if ((slot == EQUIPMENT_SLOT_OFFHAND) && (GetItemBySlot(EQUIPMENT_SLOT_MAINHAND)))
				if (GetItemBySlot(EQUIPMENT_SLOT_MAINHAND)->GetProto()->InventoryType == INVTYPE_2HWEAPON)
				return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED; 
			
			uint32 skill=GetSkillByProto(proto);
			if(skill)
            if (!GetSkillValue(skill)) return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;


			if (!HasSpell(GetSpellByProto(proto))) return EQUIP_ERR_NO_REQUIRED_PROFICIENCY; 

			
			if(proto->RequiredSkill)
			if(GetSkillValue(proto->RequiredSkill) < proto->RequiredSkillRank )
			return 	EQUIP_ERR_SKILL_ISNT_HIGH_ENOUGH;
		
			return EQUIP_ERR_OK;
		}
		else if ((slot >= INVENTORY_SLOT_ITEM_START) && (slot < INVENTORY_SLOT_ITEM_END)) 
		{
			if (item->IsBag())
			{
				if (((Bag*)item)->IsEmpty()) { return EQUIP_ERR_OK; }
				return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
			}
			return EQUIP_ERR_OK;
		} 
		else if ((slot >= INVENTORY_SLOT_BAG_START) && (slot < INVENTORY_SLOT_BAG_END)) 
		{
			if (item->IsBag())
			{ 
				if (proto->Class == ITEM_CLASS_QUIVER)
				{
					Bag* bag;
					int count = 0;
					for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
					{
						bag = GetBagBySlot(i);
						if (bag)
							if ((bag->GetProto()->Class == ITEM_CLASS_QUIVER) && (bag != item) && (bag != swapitem)
								&& (bag->GetProto()->SubClass == proto->SubClass)) count++;
					}
					if (count)
					{
						if (proto->SubClass == ITEM_SUBCLASS_AMMO_POUCH) return EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH;
						return EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER2;
					}
				}
				return EQUIP_ERR_OK; 
			}
			return EQUIP_ERR_NOT_A_BAG;
		} 
		else if ((slot >= BANK_SLOT_ITEM_START) && (slot < BANK_SLOT_ITEM_END))
		{
			if (item->IsBag()) 
			{
				if (((Bag*)item)->IsEmpty())  return EQUIP_ERR_OK; 
				return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
			}
			return EQUIP_ERR_OK; 
		}
		else if ((slot >= BANK_SLOT_BAG_START) && (slot < BANK_SLOT_BAG_END))
		{
			uint32 bankBagSlot = ((GetUInt32Value(PLAYER_BYTES_2) & 0x70000) >> 16) + BANK_SLOT_BAG_START;
			if (slot >= bankBagSlot)  return EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT; 
			if (item->IsBag()) return EQUIP_ERR_OK; 
			return EQUIP_ERR_NOT_A_BAG;
		}
		return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
	} 
	else
	{ //additional bags & additional bank bags
		Bag* bag = GetBagBySlot(bagIndex);
		if (!bag) return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
		if (bag->GetProto()->Class == ITEM_CLASS_QUIVER) 
		{
			if (proto->Class == ITEM_CLASS_PROJECTILE) 
			{
				if (bag->GetProto()->SubClass != proto->SubClass) 
				return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
				return EQUIP_ERR_OK;
			}
			return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
		} 
		else if (item->IsBag())
		{
			if (bag == (Bag*)item)  return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG; 
			if (((Bag*)item)->IsEmpty())  return EQUIP_ERR_OK; 
			return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
		}
		return EQUIP_ERR_OK;
	}
}

// Split stacks
bool Player::SplitItem(uint8 srcBag, uint8 srcSlot, uint8 dstBag, uint8 dstSlot, uint8 count) {
	UpdateData upd;
	WorldPacket data;
	uint8 error_code = 0;

	Item *dstItem = GetItemBySlot(dstBag, dstSlot);
	Item *srcItem = GetItemBySlot(srcBag, srcSlot);

	if (count == srcItem->GetCount()) { return SwapItem(dstBag, dstSlot, srcBag, srcSlot); }
	if (count > srcItem->GetCount()) error_code = EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT;

	if (dstItem && srcItem && !error_code) {
		// Same items
		if (dstItem->GetEntry() == srcItem->GetEntry()) {
			int stack = dstItem->GetProto()->Stackable?dstItem->GetProto()->Stackable:1;
			int dstCount = dstItem->GetCount();
			int srcCount = srcItem->GetCount();

			// If item is stackable and stack is not full, add to stack
			if (dstCount+count <= stack) {
				dstItem->SetCount(dstCount + count);
				srcItem->SetCount(srcCount - count);
				upd.Clear();
				dstItem->BuildCreateUpdateBlockForPlayer(&upd, this);
				srcItem->BuildCreateUpdateBlockForPlayer(&upd, this);
				upd.BuildPacket(&data);
				GetSession()->SendPacket(&data);
				_SaveInventory();
				return true;
			}
		}
		error_code = EQUIP_ERR_COULDNT_SPLIT_ITEMS;
	}

	if (!error_code) {
		dstItem = new Item; // Don't think there are stackable bags
		dstItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM),srcItem->GetEntry(),this);
		dstItem->SetCount(count);
		error_code = CanEquipItemInSlot(dstBag, dstSlot, dstItem, srcItem);
	}

	if (!error_code) {
		AddItem(dstBag, dstSlot, dstItem, false, false, true);
		srcItem->SetCount(srcItem->GetCount() - count);
		upd.Clear();
		srcItem->BuildCreateUpdateBlockForPlayer(&upd, this);
		upd.BuildPacket(&data);
		GetSession()->SendPacket(&data);
		_SaveInventory();
		return true;
	} else {
		data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);
		data << uint8(error_code);
		if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N) {
			uint32 reqlevel = 0;
			if (srcItem)
				if (srcItem->GetProto()->RequiredLevel > GetLevel()) reqlevel = srcItem->GetProto()->RequiredLevel;
			if ((dstItem) && (!reqlevel))
				if (dstItem->GetProto()->RequiredLevel > GetLevel()) reqlevel = dstItem->GetProto()->RequiredLevel;
			data << reqlevel;
		}
		data << uint64((srcItem ? srcItem->GetGUID() : 0));
		data << uint64((dstItem ? dstItem->GetGUID() : 0));
		data << uint8(0);
		m_session->SendPacket(&data);
		return false;
	}
}

// Swap items from one slot to another
bool Player::SwapItem(uint8 dstBag, uint8 dstSlot, uint8 srcBag, uint8 srcSlot) {
	UpdateData upd;
	WorldPacket data;
	uint8 error_code = 0;

	Item *dstItem = GetItemBySlot(dstBag, dstSlot);
	Item *srcItem = GetItemBySlot(srcBag, srcSlot);

	if (dstItem && srcItem) {
		// Same items
		if (dstItem->GetEntry() == srcItem->GetEntry()) {
			int stack = dstItem->GetProto()->Stackable?dstItem->GetProto()->Stackable:1;
			int dstCount = dstItem->GetCount();
			int srcCount = srcItem->GetCount();

			// If item is stackable and stack is not full, add to stack
			if (dstCount < stack) {
				dstItem->SetCount((dstCount+srcCount > stack)?stack:(dstCount+srcCount));
				upd.Clear();
				dstItem->BuildCreateUpdateBlockForPlayer(&upd, this);
				if (dstCount+srcCount > stack) {
					srcItem->SetCount(srcCount - (stack - dstCount));
					srcItem->BuildCreateUpdateBlockForPlayer(&upd, this);
				} else {
					RemoveItemFromSlot(srcBag, srcSlot);
					srcItem->DeleteFromDB();
					delete srcItem;
				}
				upd.BuildPacket(&data);
				GetSession()->SendPacket(&data);
				_SaveInventory();
				return true;
			}
		}
	}

	if (srcItem) error_code = CanEquipItemInSlot(dstBag, dstSlot, srcItem, dstItem);
	if ((!error_code) && (dstItem)) error_code = CanEquipItemInSlot(srcBag, srcSlot, dstItem, srcItem);

	if (!error_code) {
		if (dstItem) RemoveItemFromSlot(dstBag, dstSlot);
		if (srcItem) RemoveItemFromSlot(srcBag, srcSlot);
		if (dstItem) AddItem(srcBag, srcSlot, dstItem, false, false, true);
		if (srcItem) AddItem(dstBag, dstSlot, srcItem, false, false, true);
		_SaveInventory();
		return true;
	} else {
		data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
		data << uint8(error_code);
		if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N) {
			uint32 reqlevel = 0;
			if (srcItem)
				if (srcItem->GetProto()->RequiredLevel > GetLevel()) reqlevel = srcItem->GetProto()->RequiredLevel;
			if ((dstItem) && (!reqlevel))
				if (dstItem->GetProto()->RequiredLevel > GetLevel()) reqlevel = dstItem->GetProto()->RequiredLevel;
			data << reqlevel;
		}
		data << uint64((srcItem ? srcItem->GetGUID() : 0));
		data << uint64((dstItem ? dstItem->GetGUID() : 0));
		data << uint8(0);
		m_session->SendPacket(&data);
		return false;
	}
}

// This function creates the item and puts it in the bag
// Avoid direct calls to this function, use AddNewItem instead
bool Player::CreateObjectItem (uint8 bagIndex, uint8 slot, uint32 itemId, uint8 count) { 
	UpdateData upd;
	WorldPacket packet;
	Bag* bag;

	ItemPrototype *proto = objmgr.GetItemPrototype(itemId);

	if(proto){
		Item* pItem;	    

		if (proto->InventoryType == INVTYPE_BAG) {
			pItem = new Bag();
		} else {
			pItem = new Item();
		}	

		if (count > proto->Stackable) { count = proto->Stackable; }
		if (count < 1) { count = 1; }

		pItem->Create (objmgr.GenerateLowGuid (HIGHGUID_ITEM), itemId, this);
		pItem->SetCount (count);

		switch(bagIndex) {
			case 0:
			case CLIENT_SLOT_BACK:
				AddItem(0, slot, (Item*)pItem, false, false, false);
				sLog.outDetail("CreateObjectItem : item %i created, bagIndex = backpack, slot = %i, amount = %i", itemId, slot, count);
				return true;
			case CLIENT_SLOT_01:
			case CLIENT_SLOT_02:
			case CLIENT_SLOT_03:
			case CLIENT_SLOT_04:
			case BANK_SLOT_BAG_1:
			case BANK_SLOT_BAG_2:
			case BANK_SLOT_BAG_3:
			case BANK_SLOT_BAG_4:
			case BANK_SLOT_BAG_5:
			case BANK_SLOT_BAG_6:
				bag = GetBagBySlot(bagIndex);
				if (bag) {
					AddItem(bagIndex, slot, (Item*)pItem, false, false, false);
					sLog.outDetail("CreateObjectItem : item %i created, bagIndex = %i, slot = %i, amount = %i", itemId, bagIndex, slot, count);
					return true;
				} else {
					sLog.outError("CreateObjectItem : bagIndex not a Bag, bagIndex = %i", bagIndex);
					return false;
				}
			default:
				sLog.outError("CreateObjectItem : Unknown bagIndex, bagIndex = %i", bagIndex);
				return false;
		}
	}
	sLog.outError("CreateObjectItem : Unknown itemId, itemId = %i", itemId);
	return false;
}

// Returns the amount of items that player has (bank too)
int Player::GetItemCount(uint32 itemId) {
	int countitems = 0;
	Item* pItem = 0;
	Bag* pBag = 0;

	for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) {
		pItem = GetItemBySlot(i);
		if (pItem) {
			if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
		}
	}
	for (uint8 bagIndex = CLIENT_SLOT_01; bagIndex <= CLIENT_SLOT_04; bagIndex++) {
		pBag = GetBagBySlot(bagIndex);
		if (pBag) {
			for (uint8 slot=0; slot < pBag->GetProto()->ContainerSlots; slot++) {
				pItem = pBag->GetItemFromBag(slot);
				if (pItem) {
					if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
				}
			}
		}
	}
	for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++) {
		pItem = GetItemBySlot(i);
		if (pItem) {
			if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
		}
	}
	for (uint8 bagIndex = BANK_SLOT_BAG_START; bagIndex <= BANK_SLOT_BAG_END; bagIndex++) {
		pBag = GetBagBySlot(bagIndex);
		if (pBag) {
			for (uint8 slot=0; slot<pBag->GetProto()->ContainerSlots; slot++) {
				pItem = pBag->GetItemFromBag(slot);
				if (pItem) {
					if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
				}
			}
		}
	}
	return countitems;
}

// Adds a new item to player inventory
// - if addmaxpossible = false, items will be added just if user has enough space to put all the amount (count)
// - bagIndex and slot can be NULL (YOU MUST USE NULL_SLOT FOR SLOTS), in that case function searchs for first free bag/slot
// - If dontadd is true, the function will do everything but add the item (for space check purpose)
// - Notice that if a slot is specified and this slot is free or can be stacked (same item), the function will not
// search for other slots and count will be limited to max stack
// - Return value is the amount of items created
uint32 Player::AddNewItem(uint8 bagIndex, uint8 slot, uint32 itemId, uint32 count, bool addmaxpossible, bool dontadd) {
	if (!itemId) { 
		sLog.outError("AddNewItem : No itemId provided");
		return 0; 
	}

	UpdateData upd;
	WorldPacket packet;
	ItemPrototype *proto = objmgr.GetItemPrototype(itemId);

	if (proto) {

		Item *pItem = 0;
		Bag *pBag = 0;
		int stack = proto->Stackable;
		if (stack < 1) { stack = 1; }
		if (count < 1) { count = 1; }
		int total = count;
		int freespace = 0;

		// First lets check if count is >= to maxcount
		// MaxCount is related to unique items, users can't have more than MaxCount items, not even in bank
		if	(proto->MaxCount > 0) {
			if	(GetItemCount(itemId) >= proto->MaxCount)	{
				sLog.outError("AddNewItem : Too many items, itemId = %i", itemId);
				return false;
			}
		}

		// The slot must be free or, if the same item is there
		// it should be added to stack (if it's stackable)
		if (slot != NULL_SLOT) {
			switch(bagIndex) {
				case 0:
				case CLIENT_SLOT_BACK:
					if (slot >= BANK_SLOT_BAG_END) {
						sLog.outError("AddNewItem : Invalid slot, slot = %i", slot);
						return 0;
					}
					if ((((slot >= INVENTORY_SLOT_BAG_START) && (slot < INVENTORY_SLOT_BAG_END)) || ((slot >= BANK_SLOT_BAG_START) && (slot < BANK_SLOT_BAG_END))) && (proto->InventoryType != INVTYPE_BAG)) {
						sLog.outError("AddNewItem : Non-bag item in bag slot, itemId = %i, slot = %i", itemId, slot);
						return 0;
					}
					pItem = GetItemBySlot(slot);
					break;
				case CLIENT_SLOT_01:
				case CLIENT_SLOT_02:
				case CLIENT_SLOT_03:
				case CLIENT_SLOT_04:
				case BANK_SLOT_BAG_1:
				case BANK_SLOT_BAG_2:
				case BANK_SLOT_BAG_3:
				case BANK_SLOT_BAG_4:
				case BANK_SLOT_BAG_5:
				case BANK_SLOT_BAG_6:
					pBag = GetBagBySlot(bagIndex);
					if (pBag) { 
						if (slot >= pBag->GetProto()->ContainerSlots) {
							sLog.outError("AddNewItem : Invalid slot, bagIndex = %i, slot = %i", bagIndex, slot);
							return 0;
						}
						pItem = pBag->GetItemFromBag(slot); 
					} else {
						sLog.outError("AddNewItem : No bag in that bagIndex, bagIndex = %i", bagIndex);
						return 0;
					}
					break;
				default:
					sLog.outError("AddNewItem : Unknown bagIndex, bagIndex = %i", bagIndex);
					return 0;
			}
			if (pItem) {
				if (pItem->GetEntry() != itemId) {
					sLog.outError("AddNewItem : Player slot already has another item, bagIndex = %i, slot = %i", bagIndex, slot);
					return 0;
				} else {
					if (((stack - pItem->GetCount()) >= count) || (addmaxpossible)) {
						if (!dontadd) {
							pItem->SetCount(((pItem->GetCount() + count) > stack)?stack:(pItem->GetCount() + count));
							pItem->SaveToDB();
							if (IsInWorld()) {
								upd.Clear();
								pItem->BuildCreateUpdateBlockForPlayer( &upd, this );
								upd.BuildPacket( &packet );
								m_session->SendPacket(&packet);
							}
						}
						sLog.outDetail("AddNewItem : Item added (stack), itemId = %i, amount = %i, bagIndex = %i, slot = %i, dontadd = %i", itemId, ((pItem->GetCount() + count) > stack)?(stack - pItem->GetCount()):count, bagIndex, slot, dontadd);
						return ((pItem->GetCount() + count) > stack)?(stack - pItem->GetCount()):count;
					} else {
						sLog.outError("AddNewItem : Player slot is full, bagIndex = %i, slot = %i", bagIndex, slot);
						return 0;
					}
				}
			} else {
				CreateObjectItem(bagIndex, slot, itemId, (count > stack)?stack:count);
				sLog.outDetail("AddNewItem : Item added, itemId = %i, amount = %i, bagIndex = %i, slot = %i, dontadd = %i", itemId, (count > stack)?stack:count, bagIndex, slot, dontadd);
				return count;
			}
		}

		if ((bagIndex) && (((bagIndex < CLIENT_SLOT_01) || (bagIndex > CLIENT_SLOT_04)) && (bagIndex != CLIENT_SLOT_BACK))) {
			sLog.outError("AddNewItem : Unknown bagIndex, bagIndex = %i", bagIndex);
			return 0;
		}

		if ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK) && (!GetBagBySlot(bagIndex))) {
			sLog.outError("AddNewItem : No bag in bagIndex, bagIndex = %i", bagIndex);
			return 0;
		}

		// If bag is specified, don't search in other bags
		uint8 bagIndexStart = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:CLIENT_SLOT_01;
		uint8 bagIndexEnd = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:CLIENT_SLOT_04;

		// If slot is not specified, check for free slots (and stacks, if allowstack = true)
		// cycle 0 - searching for stack space
		// cycle 1 - searching for free slots
		// cycle 2 - adding to stacks
		// cycle 3 - adding to free slots
		for (int cycle=0; cycle <= 3; cycle++) {

			if ((cycle > 1) && ((!freespace) || ((freespace) && (freespace < total) && (!addmaxpossible)))) {
				sLog.outError("AddNewItem : Not enough free space, itemId = %i, amount = %i, total space = %i, addmaxpossible = %i", itemId, total, freespace, addmaxpossible);
				return 0;
			}

			// Player backpack
			if ((bagIndex == CLIENT_SLOT_BACK) || (!bagIndex)) {
				for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) {
					pItem = GetItemBySlot(i);
					if (!pItem) {
						if (cycle == 1) {
							freespace += stack;
						} else if (cycle == 3) {
							int plus = (count >= stack)?stack:count;
							count -= plus;
							if (!dontadd) { CreateObjectItem(CLIENT_SLOT_BACK, i, itemId, plus); }
							if (!count) {
								sLog.outDetail("AddNewItem : Item added, itemId = %i, amount = %i, dontadd = %i", itemId, total, dontadd);
								return total;
							}
						}
					} else {
						if (pItem->GetEntry() == itemId) {
							if ((pItem->GetCount() < stack) && (count)) {
								int plus = count;
								if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
								if (cycle == 0) {
									freespace += plus;
								} else if (cycle == 2) {
									count -= plus;
									if (!dontadd) {
										pItem->SetCount(pItem->GetCount() + plus);
										pItem->SaveToDB();
										if (IsInWorld()) {
											upd.Clear();
											pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
											upd.BuildPacket(&packet);
											m_session->SendPacket(&packet);
										}
									}
									if (!count) {
										sLog.outDetail("AddNewItem : Item added (stacked), itemId = %i, amount = %i, dontadd = %i", itemId, total, bagIndex, slot, dontadd);
										return total;
									}
								}
							}
						}
					}
				} 
			}

			// Additional bags
			if (bagIndex != CLIENT_SLOT_BACK) {
				for(uint8 bagIndex2 = bagIndexStart; bagIndex2 <= bagIndexEnd; bagIndex2++) {
					pBag = GetBagBySlot(bagIndex2);
					if (pBag) {
						for (uint8 slot2=0; slot2 < pBag->GetProto()->ContainerSlots; slot2++) {
							pItem = pBag->GetItemFromBag(slot2);
							if (!pItem) {
								if (cycle == 1) {
									freespace += stack;
								} else if (cycle == 3) {
									int plus = (count >= stack)?stack:count;
									count -= plus;
									if (!dontadd) { CreateObjectItem(bagIndex2, slot2, itemId, plus); }
									if (!count) {
										sLog.outDetail("AddNewItem : Item added, itemId = %i, amount = %i, dontadd = %i", itemId, total, dontadd);
										return total;
									}
								}
							} else {
								if (pItem->GetEntry() == itemId) { 
									if ((pItem->GetCount() < stack) && (count)) {
										int plus = count;
										if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
										if (cycle == 0) {
											freespace += plus;
										} else if (cycle == 2) {
											count -= plus;	
											if (!dontadd) {
												pItem->SetCount(pItem->GetCount() + plus);
												pItem->SaveToDB();
												if (IsInWorld()) {
													upd.Clear();
													pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
													upd.BuildPacket(&packet);
													m_session->SendPacket(&packet);
												}
											}
											if (!count) {
												sLog.outDetail("AddNewItem : Item added (stacked), itemId = %i, amount = %i, dontadd = %i", itemId, total, bagIndex, slot, dontadd);
												return total;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		sLog.outDetail("AddNewItem : Item partially added, itemId = %i, amount = %i, rest = %i, dontadd = %i", itemId, total, count, dontadd);
		return (total - count);
	}

	sLog.outError("AddNewItem : Unknown itemId, itemId = %i", itemId);
	return 0;
}

// Adds an existing item (pointed by *item) to player inventory
// - bagIndex and slot can be NULL (YOU MUST USE NULL_SLOT FOR SLOTS), in that case function searchs for first free bag/slot
// - If allowstack is true, the function will try to stack items, otherwise it will just add if
// the slot is free
// - If dontadd is true, nothing will happen to the item (for space check purpose)
// - If dontsave is true, function will not call for inventory save (then you'll have to do it manually)
// - Notice that if a slot is specified and this slot is free, the function will not search for stacks
// - Return values: 0 - item not added
//                  1 - item added to a free slot (and perhaps to a stack)
//                  2 - item added to a stack (item should be deleted)
uint8 Player::AddItem(uint8 bagIndex,uint8 slot, Item *item, bool allowstack, bool dontadd, bool dontsave) {
	if (!item) {
		sLog.outError("AddItem: No item provided");
		return 0;
	}
	if (!item->GetProto()) {
		sLog.outError("AddItem: Unknown item, itemId = %i",item->GetEntry());
		return 0;
	}

	UpdateData upd;
	WorldPacket packet;
	Item *pItem = 0;
	Bag *pBag = 0;
	int stack = (item->GetProto()->Stackable)?(item->GetProto()->Stackable):1;
	int count = item->GetCount();

	switch(bagIndex) {
		case 0:
		case CLIENT_SLOT_BACK:
			if (slot >= BANK_SLOT_BAG_END) {
				sLog.outError("AddItem : Invalid slot, slot = %i", slot);
				return 0;
			}
			if ((((slot >= INVENTORY_SLOT_BAG_START) && (slot < INVENTORY_SLOT_BAG_END)) || ((slot >= BANK_SLOT_BAG_START) && (slot < BANK_SLOT_BAG_END))) && (item->GetProto()->InventoryType != INVTYPE_BAG)) {
				sLog.outError("AddItem : Non-bag item in bag slot, itemId = %i, slot = %i", item->GetEntry(), slot);
				return 0;
			}
			pItem = GetItemBySlot(slot);
			break;
		case CLIENT_SLOT_01:
		case CLIENT_SLOT_02:
		case CLIENT_SLOT_03:
		case CLIENT_SLOT_04:
		case BANK_SLOT_BAG_1:
		case BANK_SLOT_BAG_2:
		case BANK_SLOT_BAG_3:
		case BANK_SLOT_BAG_4:
		case BANK_SLOT_BAG_5:
		case BANK_SLOT_BAG_6:
			pBag = GetBagBySlot(bagIndex);
			if (pBag) {
				if ((slot >= pBag->GetProto()->ContainerSlots)) {
					sLog.outError("AddItem: Invalid slot, bagIndex = %i, slot = %i", bagIndex, slot);
					return 0;
				}
				pItem = pBag->GetItemFromBag(slot);
			} else {
				sLog.outError("AddItem: No bag in that bagIndex, bagIndex = %i", bagIndex);
				return 0;
			}
			break;
		default:
			sLog.outError("AddItem: Unknown bagIndex, bagIndex = %i", bagIndex);
			return 0;
	}

	if (pItem) {
		if (pItem->GetEntry() != item->GetEntry()) {
			sLog.outError("AddItem : Player slot already has another item" );
			return 0;
		} else {
			if (((stack - pItem->GetCount()) >= count) && (allowstack)) {
				if (!dontadd) {
					pItem->SetCount(((pItem->GetCount() + count) > stack)?stack:(pItem->GetCount() + count));
					_SaveInventory();
					if (IsInWorld()) {
						upd.Clear();
						pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
						upd.BuildPacket(&packet);
						m_session->SendPacket(&packet);
					}
				}
				sLog.outDetail("AddItem : Item %i added to bag %i - slot %i (stacked), dontadd = %i", pItem->GetEntry(), bagIndex, slot, dontadd);
				return 2;
			} else {
				sLog.outError("AddItem : Player slot is full" );
				return 0;
			}
		}
	} else {
		if (!dontadd) {
			item->SetOwner(this);
			item->SetCount(count);
		}

		if (((bagIndex >= CLIENT_SLOT_01) && (bagIndex <= CLIENT_SLOT_04)) || ((bagIndex >= BANK_SLOT_BAG_1) && (bagIndex <= BANK_SLOT_BAG_6))) {
			if (!dontadd) {
				pBag = GetBagBySlot(bagIndex);
				pBag->AddItemToBag(slot, item);
				if (!dontsave) { _SaveInventory(); }
				if (IsInWorld()) {
					item->AddToWorld();
					upd.Clear();
					pBag->BuildCreateUpdateBlockForPlayer(&upd, this);
					item->BuildCreateUpdateBlockForPlayer(&upd, this);
					upd.BuildPacket(&packet);
					GetSession()->SendPacket(&packet);
				}
			}
			sLog.outDetail("AddItem: Item %i added to bag, bagIndex = %i, slot = %i, dontadd = %i", item->GetEntry(), bagIndex, slot, dontadd);
		} else {
			if (!dontadd) {
				m_items[slot] = item;
				SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), m_items[slot]?m_items[slot]->GetGUID():0);
				item->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
				if (slot < EQUIPMENT_SLOT_END) {
					int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
					SetUInt32Value(VisibleBase, item->GetUInt32Value(OBJECT_FIELD_ENTRY));
					SetUInt32Value(VisibleBase + 1, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT));
					SetUInt32Value(VisibleBase + 2, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 3));
					SetUInt32Value(VisibleBase + 3, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 6));
					SetUInt32Value(VisibleBase + 4, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 9));
					SetUInt32Value(VisibleBase + 5, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 12));
					SetUInt32Value(VisibleBase + 6, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 15));
					SetUInt32Value(VisibleBase + 7, item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 18));
					SetUInt32Value(VisibleBase + 8, item->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
					_ApplyItemMods(item, slot, true);
				}
				if (!dontsave) { _SaveInventory(); }
				if (IsInWorld()) {
					item->AddToWorld();
					upd.Clear();
					item->BuildCreateUpdateBlockForPlayer(&upd, this);
					upd.BuildPacket(&packet);
					GetSession()->SendPacket(&packet);
				}
			}
			sLog.outDetail("AddItem: Item %i added to slot, slot = %i, dontadd = %i", item->GetEntry(), slot, dontadd);
		}
		return 1;
	}
}

//Adds an existing item to inventory
//---
//bagIndex == NULL && slot == NULL_SLOT : function searchs for a free slot (first in backpack, then in additional bags)
//bagIndex == NULL && slot defined : slot should be between EQUIPMENT_SLOT_START and INVENTORY_SLOT_ITEM_END
//bagIndex defined && slot == NULL_SLOT : function searchs for a free slot in the choosen bag
//bagIndex && slot defined: function adds the item to the specified slot
//allowstack == true : if slot is not free but a item with same id is on it and it can be attacked, function will stack
//dontadd == true : function will do everything but add the item, use this to check if adding is possible
//dontsave == true : function will not save the DB after the operations (you'll have to save it by yourself)
//---
//Return values:
// 0 - item not added
// 1 - item added to a free slot (and perhaps to a stack)
// 2 - item totally added to a stack (item should be deleted)
// 3 - item can't be added (function will send SMSG_INVENTORY_CHANGE_FAILURE)
uint8 Player::AddItemToInventory(uint8 bagIndex, uint8 slot, Item *item, bool allowstack, bool dontadd, bool dontsave) {
	if (!item) {
		sLog.outError("AddItemToInventory: No item provided");
		return 0;
	}
	if (!item->GetProto()) {
		sLog.outError("AddItemToInventory: Unknown item, itemId = %i",item->GetEntry());
		return 0;
	}

	UpdateData upd;
	WorldPacket packet;
	Item *pItem = 0;
	Bag *pBag = 0;
	int stack = (item->GetProto()->Stackable)?(item->GetProto()->Stackable):1;
	int count = item->GetCount();
	uint8 addtobag = 0;
	uint8 addtoslot = NULL_SLOT;
	int freespace = 0;
	int freeslots = 0;

	if (slot != NULL_SLOT) {
		switch(bagIndex) {
			case 0:
			case CLIENT_SLOT_BACK:
				if (slot >= INVENTORY_SLOT_ITEM_END) {
					sLog.outError("AddItemToInventory: Invalid slot, slot = %i", slot);
					return 0;
				}
				break;
			case CLIENT_SLOT_01:
			case CLIENT_SLOT_02:
			case CLIENT_SLOT_03:
			case CLIENT_SLOT_04:
				pBag = GetBagBySlot(bagIndex);
				if (pBag) {
					if ((slot >= pBag->GetProto()->ContainerSlots)) {
						sLog.outError("AddItemToInventory: Invalid slot, bagIndex = %i, slot = %i", bagIndex, slot);
						return 0;
					}
				} else {
					sLog.outError("AddItemToInventory: No bag in that bagIndex, bagIndex = %i", bagIndex);
					return 0;
				}
				break;
			default:
				sLog.outError("AddItemToInventory: Unknown bagIndex, bagIndex = %i", bagIndex);
				return 0;
		}
		return AddItem(bagIndex, slot, item, allowstack, dontadd, dontsave);
	} else {

		if ((bagIndex) && (((bagIndex < CLIENT_SLOT_01) || (bagIndex > CLIENT_SLOT_04)) && (bagIndex != CLIENT_SLOT_BACK))) {
			sLog.outError("AddItemToInventory: Unknown bagIndex, bagIndex = %i", bagIndex);
			return 0;
		}

		if ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK) && (!GetBagBySlot(bagIndex))) {
			sLog.outError("AddItemToInventory: No bag in bagIndex, bagIndex = %i",bagIndex);
			return 0;
		}

		// If bag is specified, don't search in other bags
		uint8 bagIndexStart = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:CLIENT_SLOT_01;
		uint8 bagIndexEnd = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:CLIENT_SLOT_04;

		// If slot is not specified, check for free slots (and stacks, if allowstack = true)
		// cycle 0 - searching for stack space
		// cycle 1 - searching for free slots
		// cycle 2 - adding to stacks
		// cycle 3 - adding to free slots
		for (int cycle=0; cycle <= 3; cycle++) {

			if ((cycle > 1) && (((allowstack) && (freespace < count) && (!freeslots)) || ((!allowstack) && (!freeslots)))) {
				sLog.outError("AddItemToInventory: Not enough free space, freespace = %i, amount = %i, allowstack = %i", freespace, count, allowstack);
				return 0;
			}

			// Player backpack
			if ((bagIndex == CLIENT_SLOT_BACK) || (!bagIndex)) {
				for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) {
					pItem = GetItemBySlot(i);
					if (!pItem) {
						if (cycle == 1) {
							freeslots++;
						} else if (cycle == 3) {
							if (!addtobag) { addtobag = CLIENT_SLOT_BACK; }
							if (addtoslot == NULL_SLOT) { addtoslot = i; }
						}
					} else {
						if ((pItem->GetEntry() == item->GetEntry()) && ((pItem->GetCount() < stack) && (count))) {
							if (cycle == 0) {
								freespace += (stack - pItem->GetCount());
							} else if ((cycle == 2) && (allowstack)) {
								int plus = count;
								if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
								count -= plus;
								if (!dontadd) {
									pItem->SetCount(pItem->GetCount() + plus);
									if (IsInWorld()) {
										upd.Clear();
										pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
										upd.BuildPacket(&packet);
										m_session->SendPacket(&packet);
									}
								}
								if (!count) {
									if ((!dontadd) && (!dontsave)) { _SaveInventory(); }
									sLog.outDetail("AddItemToInventory : Item %i added (stacked), dontadd = %i", pItem->GetEntry(), dontadd);
									return 2;
								}
							}
						}
					}
				} 
			}

			// Additional bags
			if (bagIndex != CLIENT_SLOT_BACK) {
				for (uint8 bagIndex2 = bagIndexStart; bagIndex2 <= bagIndexEnd; bagIndex2++) {
					pBag = GetBagBySlot(bagIndex2);
					if (pBag) {
						for (uint8 slot2=0; slot2 < pBag->GetProto()->ContainerSlots; slot2++) {
							pItem = pBag->GetItemFromBag(slot2);
							if (!pItem) {
								if (cycle == 1) {
									freeslots++;
								} else if (cycle == 3) {
									if (!addtobag) { addtobag = bagIndex2; }
									if (addtoslot == NULL_SLOT) { addtoslot = slot2; }
								}
							} else {
								if ((pItem->GetEntry() == item->GetEntry()) && ((pItem->GetCount() < stack) && (count))) {
									if (cycle == 0) {
										freespace += (stack - pItem->GetCount());
									} else if ((cycle == 2) && (allowstack)) {
										int plus = count;
										if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
										count -= plus;
										if (!dontadd) {
											pItem->SetCount(pItem->GetCount() + plus);
											if (IsInWorld()) {
												upd.Clear();
												pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
												upd.BuildPacket(&packet);
												m_session->SendPacket(&packet);
											}
										}
										if (!count) {
											if ((!dontadd) && (!dontsave)) { _SaveInventory(); }
											sLog.outDetail("AddItemToInventory : Item %i added (stacked), dontadd = %i", pItem->GetEntry(), dontadd);
											return 2;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return AddItem(addtobag, addtoslot, item, allowstack, dontadd, dontsave);
}

//Adds an existing item to bank
//Same options as AddItemToInventory
uint8 Player::AddItemToBank(uint8 bagIndex,uint8 slot, Item *item, bool allowstack, bool dontadd, bool dontsave) {
	if (!item) {
		sLog.outError("AddItemToBank: No item provided");
		return 0;
	}
	if (!item->GetProto()) {
		sLog.outError("AddItemToBank: Unknown item, itemId = %i",item->GetEntry());
		return 0;
	}

	UpdateData upd;
	WorldPacket packet;
	Item *pItem = 0;
	Bag *pBag = 0;
	int stack = (item->GetProto()->Stackable)?(item->GetProto()->Stackable):1;
	int count = item->GetCount();
	uint8 addtobag = 0;
	uint8 addtoslot = NULL_SLOT;
	int freespace = 0;
	int freeslots = 0;

	if (slot != NULL_SLOT) {
		switch(bagIndex) {
			case 0:
			case CLIENT_SLOT_BACK:
				if ((slot < BANK_SLOT_ITEM_START) || (slot >= BANK_SLOT_ITEM_END)) {
					sLog.outError("AddItemToBank: Invalid slot, slot = %i", slot);
					return 0;
				}
				break;
			case BANK_SLOT_BAG_1:
			case BANK_SLOT_BAG_2:
			case BANK_SLOT_BAG_3:
			case BANK_SLOT_BAG_4:
			case BANK_SLOT_BAG_5:
			case BANK_SLOT_BAG_6:
				pBag = GetBagBySlot(bagIndex);
				if (pBag) {
					if ((slot >= pBag->GetProto()->ContainerSlots)) {
						sLog.outError("AddItemToBank: Invalid slot, bagIndex = %i, slot = %i", bagIndex, slot);
						return 0;
					}
				} else {
					sLog.outError("AddItemToBank: No bag in that bagIndex, bagIndex = %i", bagIndex);
					return 0;
				}
				break;
			default:
				sLog.outError("AddItemToBank: Unknown bagIndex, bagIndex = %i", bagIndex);
				return 0;
		}
		return AddItem(bagIndex, slot, item, allowstack, dontadd, dontsave);
	} else {

		if ((bagIndex) && (((bagIndex < BANK_SLOT_BAG_1) || (bagIndex > BANK_SLOT_BAG_6)) && (bagIndex != CLIENT_SLOT_BACK))) {
			sLog.outError("AddItemToBank: Unknown bagIndex, bagIndex = %i", bagIndex);
			return 0;
		}

		if ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK) && (!GetBagBySlot(bagIndex))) {
			sLog.outError("AddItemToBank: No bag in bagIndex, bagIndex = %i",bagIndex);
			return 0;
		}

		// If bag is specified, don't search in other bags
		uint8 bagIndexStart = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:BANK_SLOT_BAG_1;
		uint8 bagIndexEnd = ((bagIndex) && (bagIndex != CLIENT_SLOT_BACK))?bagIndex:BANK_SLOT_BAG_6;

		// If slot is not specified, check for free slots (and stacks, if allowstack = true)
		// cycle 0 - searching for stack space
		// cycle 1 - searching for free slots
		// cycle 2 - adding to stacks
		// cycle 3 - adding to free slots
		for (int cycle=0; cycle <= 3; cycle++) {

			if ((cycle > 1) && (((allowstack) && (freespace < count) && (!freeslots)) || ((!allowstack) && (!freeslots)))) {
				sLog.outError("AddItemToBank: Not enough free space, freespace = %i, amount = %i, allowstack = %i", freespace, count, allowstack);
				return 0;
			}

			// Bank slots
			if ((bagIndex == CLIENT_SLOT_BACK) || (!bagIndex)) {
				for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++) {
					pItem = GetItemBySlot(i);
					if (!pItem) {
						if (cycle == 1) {
							freeslots++;
						} else if (cycle == 3) {
							if (!addtobag) { addtobag = CLIENT_SLOT_BACK; }
							if (addtoslot == NULL_SLOT) { addtoslot = i; }
						}
					} else {
						if ((pItem->GetEntry() == item->GetEntry()) && ((pItem->GetCount() < stack) && (count))) {
							if (cycle == 0) {
								freespace += (stack - pItem->GetCount());
							} else if ((cycle == 2) && (allowstack)) {
								int plus = count;
								if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
								count -= plus;
								if (!dontadd) {
									pItem->SetCount(pItem->GetCount() + plus);
									if (IsInWorld()) {
										upd.Clear();
										pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
										upd.BuildPacket(&packet);
										m_session->SendPacket(&packet);
									}
								}
								if (!count) {
									if ((!dontadd) && (!dontsave)) { _SaveInventory(); }
									sLog.outDetail("AddItemToBank : Item %i added (stacked), dontadd = %i", pItem->GetEntry(), dontadd);
									return 2;
								}
							}
						}
					}
				} 
			}

			// Additional bank bags
			if (bagIndex != CLIENT_SLOT_BACK) {
				for (uint8 bagIndex2 = bagIndexStart; bagIndex2 <= bagIndexEnd; bagIndex2++) {
					pBag = GetBagBySlot(bagIndex2);
					if (pBag) {
						for (uint8 slot2=0; slot2 < pBag->GetProto()->ContainerSlots; slot2++) {
							pItem = pBag->GetItemFromBag(slot2);
							if (!pItem) {
								if (cycle == 1) {
									freeslots++;
								} else if (cycle == 3) {
									if (!addtobag) { addtobag = bagIndex2; }
									if (addtoslot == NULL_SLOT) { addtoslot = slot2; }
								}
							} else {
								if ((pItem->GetEntry() == item->GetEntry()) && ((pItem->GetCount() < stack) && (count))) {
									if (cycle == 0) {
										freespace += (stack - pItem->GetCount());
									} else if ((cycle == 2) && (allowstack)) {
										int plus = count;
										if ((pItem->GetCount() + count) > stack) { plus = stack - pItem->GetCount(); }
										count -= plus;
										if (!dontadd) {
											pItem->SetCount(pItem->GetCount() + plus);
											if (IsInWorld()) {
												upd.Clear();
												pItem->BuildCreateUpdateBlockForPlayer(&upd, this);
												upd.BuildPacket(&packet);
												m_session->SendPacket(&packet);
											}
										}
										if (!count) {
											if ((!dontadd) && (!dontsave)) { _SaveInventory(); }
											sLog.outDetail("AddItemToBank : Item %i added (stacked), dontadd = %i", pItem->GetEntry(), dontadd);
											return 2;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return AddItem(addtobag, addtoslot, item, allowstack, dontadd, dontsave);
}

uint8 Player::AddItemToBag(uint8 bagIndex, Item *item, bool allowstack, bool dontadd, bool dontsave) {
	if ((bagIndex >= INVENTORY_SLOT_BAG_START) && (bagIndex < INVENTORY_SLOT_BAG_END)) {
		return AddItemToInventory(bagIndex, NULL_SLOT, item, allowstack, dontadd, dontsave);
	} else if ((bagIndex >= BANK_SLOT_BAG_START) && (bagIndex < BANK_SLOT_BAG_END)) {
		return AddItemToBank(bagIndex, NULL_SLOT, item, allowstack, dontadd, dontsave);
	}
	return 0;
}

// Removes an Item from a bag and/or slot
// bagIndex can be NULL
// Return value is a pointer to deleted item, NULL if no item deleted
Item* Player::RemoveItemFromSlot(uint8 bagIndex, uint8 slot, bool client_remove) {
	UpdateData upd;
	WorldPacket packet;
	Item *pretItem = 0;
	Bag *pBag = 0;
	Item *pItem = 0;

	switch (bagIndex) {
		case 0:
		case CLIENT_SLOT_BACK:
			if (slot >= BANK_SLOT_BAG_END) {
				sLog.outError("RemoveItemFromSlot : Invalid slot, slot = %i", slot);
				return 0;
			}
			pItem = m_items[slot];
			if (!pItem) {
				sLog.outError("RemoveItemFromSlot : No item found in that slot, slot = %i", slot);
				return 0;
			}
			pretItem = pItem;
			m_items[slot] = NULL;
			SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);
			if (slot < EQUIPMENT_SLOT_END) {
				_ApplyItemMods(pItem, slot, false);
				int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
				for (int i = VisibleBase; i < VisibleBase + 12; ++i) {
					SetUInt32Value(i, 0);
				}
			}
			if (client_remove) {
				pItem->SetOwner(0);
				if (IsInWorld()) {
					pItem->RemoveFromWorld();
					pItem->DestroyForPlayer(this);
				}
			}
			sLog.outDetail("RemoveItemFromSlot : Item removed, slot = %i", slot);
			break;
		case CLIENT_SLOT_01:
		case CLIENT_SLOT_02:
		case CLIENT_SLOT_03:
		case CLIENT_SLOT_04:
		case BANK_SLOT_BAG_1:
		case BANK_SLOT_BAG_2:
		case BANK_SLOT_BAG_3:
		case BANK_SLOT_BAG_4:
		case BANK_SLOT_BAG_5:
		case BANK_SLOT_BAG_6:
			pBag = GetBagBySlot(bagIndex);
			if (pBag) {
				pItem = pBag->GetItemFromBag(slot);
				if (pItem) {
					pretItem = pBag->RemoveItemFromBag(slot);
					if (client_remove) {
						pItem->SetOwner(0);
						if (IsInWorld()) {
							pItem->RemoveFromWorld();
							pItem->DestroyForPlayer(this);
						}
					}
					if (IsInWorld()) {
						upd.Clear();
						pBag->Item::BuildCreateUpdateBlockForPlayer(&upd, this);
						upd.BuildPacket(&packet);
						m_session->SendPacket(&packet);
					}
					sLog.outDetail("RemoveItemFromSlot : Item removed, bagIndex = %i, slot = %i", bagIndex, slot);
					break;
				} else {
					sLog.outError("RemoveItemFromSlot : No item found, bagIndex = %i, slot = %i", bagIndex, slot);
					return 0;
				}
			} else {
				sLog.outError("RemoveItemFromSlot : No bag in that bagIndex, bagIndex = %i", bagIndex);
				return 0;
			}
			break;
		default:
			sLog.outError("RemoveItemFromSlot : Unknow bagIndex, bagIndex = %i", bagIndex);
			return 0;
	}
	return pretItem;
}

Item* Player::GetItemBySlot(uint8 bagIndex,uint8 slot) const {
	Bag *pBag = 0;

	switch (bagIndex) {
		case 0:
		case CLIENT_SLOT_BACK:
			return GetItemBySlot(slot);
			break;
		case CLIENT_SLOT_01:
		case CLIENT_SLOT_02:
		case CLIENT_SLOT_03:
		case CLIENT_SLOT_04:
		case BANK_SLOT_BAG_1:
		case BANK_SLOT_BAG_2:
		case BANK_SLOT_BAG_3:
		case BANK_SLOT_BAG_4:
		case BANK_SLOT_BAG_5:
		case BANK_SLOT_BAG_6:
			pBag = GetBagBySlot(bagIndex);
			if (pBag) {
				return pBag->GetItemFromBag(slot);
			}
			break;
		default:
			sLog.outDetail("GetItemBySlot : unknow bagIndex, bagIndex = %i\n", bagIndex);
			break;
	}
	return 0;
}

uint32 Player::GetSlotByItemID(uint32 ID) {
	for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++) {
		if(m_items[i] != 0)
			if(m_items[i]->GetProto()->ItemId == ID)
				return i;
	}
	return 0;
}

uint32 Player::GetSlotByItemGUID(uint64 guid) {
	for(uint32 i=0;i<INVENTORY_SLOT_ITEM_END;i++) {
		if(m_items[i] != 0)
			if(m_items[i]->GetGUID() == guid)
				return i;
	}
	return 0;
}

bool Player::GetSlotByItemGUID(uint64 guid,uint8 &bagIndex,uint8 &slot) {
	slot=GetSlotByItemGUID(guid);
	if (slot) {
		bagIndex = CLIENT_SLOT_BACK;
		return true;
	}
	Bag *pBag;
	int8 s;

	for (uint8 i=CLIENT_SLOT_01;i<=CLIENT_SLOT_04;i++) {
		pBag = GetBagBySlot(i);
		if (pBag) {
			s=pBag->GetSlotByItemGUID(guid);
			if (s != -1) {
				slot = s;
				bagIndex = i;
				return true;
			}
		}       
	}
	return false;
}

void Player::AddItemToBuyBackSlot(uint32 slot,Item *item) {
	if (item && item->GetProto())
		sLog.outError("AddItemtoBuyBackSlot Item: \"%s\" [%u] Slot: [%u]", item->GetProto()->Name1, item->GetProto()->ItemId, slot);
	else {
		sLog.outError("AddItemtoBuyBackSlot Unknown Item! Slot: [%u]", slot);
		return;
	}
    
	if (slot >= BUYBACK_SLOT_END)
		return;

	RemoveItemFromBuyBackSlot(slot);

	m_buybackitems[slot] = item;
	time_t base = time(NULL);
  	time_t etime = base + (30 * 3600);

	SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+slot*2,item->GetGUID());
	SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+slot,(item->GetProto()->SellPrice) * item->GetCount());
	SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+slot,(uint32)etime);
}

Item* Player::GetItemFromBuyBackSlot(uint32 slot) {
	if (slot >= BUYBACK_SLOT_END)
		return NULL;

	return m_buybackitems[slot];
}

Item* Player::RemoveItemFromBuyBackSlot(uint32 slot) {
	if (slot >= BUYBACK_SLOT_END)
		return NULL;

	Item *pItem = m_buybackitems[slot];
	if(m_buybackitems[slot]) {
		m_buybackitems[slot]->DeleteFromDB();
		m_buybackitems[slot]->RemoveFromWorld();
	}

	m_buybackitems[slot] = NULL;
	SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+slot*2,0);
	SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+slot,0);
	SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+slot,0);

	return pItem;
}


void Player::_ApplyItemMods(Item *item, uint8 slot,bool apply) 
{
	
	if(!item) return;
	
	ItemPrototype *proto = item->GetProto();
	
	if(!proto) return;
	
	_RemoveStatsMods();
	
	if (apply)
	{
		sLog.outString("applying mods for item %u ",item->GetGUIDLow());
		if(proto->ItemSet)
			AddItemsSetItem(this,proto->ItemSet);
	}
	else
	{
		sLog.outString("removing mods for item %u ",item->GetGUIDLow());
		if(proto->ItemSet)
			RemoveItemsSetItem(this,proto->ItemSet);
	}

	int32 val;

	for (int i = 0; i < 10; i++)
	{
		val = proto->ItemStat[i].ItemStatValue ;

		switch (proto->ItemStat[i].ItemStatType) 
		{
			case POWER: // modify MP
				SetUInt32Value(UNIT_FIELD_MAXPOWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1)+(apply? val:-val));
				break;
			case HEALTH: // modify HP
				SetUInt32Value(UNIT_FIELD_MAXHEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH)+(apply? val:-val));
				break;
			case AGILITY: // modify agility				
				SetUInt32Value(UNIT_FIELD_AGILITY,GetUInt32Value(UNIT_FIELD_AGILITY)+(apply? val:-val));
				SetUInt32Value(PLAYER_FIELD_POSSTAT1,GetUInt32Value(PLAYER_FIELD_POSSTAT1)+(apply? val:-val));
				break;
			case STRENGHT: //modify strength
				SetUInt32Value(UNIT_FIELD_STR,GetUInt32Value(UNIT_FIELD_STR)+(apply? val:-val));
				SetUInt32Value(PLAYER_FIELD_POSSTAT0,GetUInt32Value(PLAYER_FIELD_POSSTAT0)+(apply? val:-val));
				break;
			case INTELLECT: //modify intellect 
				SetUInt32Value(UNIT_FIELD_IQ,GetUInt32Value(UNIT_FIELD_IQ)+(apply? val:-val));
				SetUInt32Value(PLAYER_FIELD_POSSTAT3,GetUInt32Value(PLAYER_FIELD_POSSTAT3)+(apply? val:-val));
				SetUInt32Value(UNIT_FIELD_MAXPOWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1)+(apply? val:-val)*15);
				break;
			case SPIRIT: //modify spirit
				SetUInt32Value(UNIT_FIELD_SPIRIT,GetUInt32Value(UNIT_FIELD_SPIRIT)+(apply? val:-val));
				SetUInt32Value(PLAYER_FIELD_POSSTAT4,GetUInt32Value(PLAYER_FIELD_POSSTAT4)+(apply? val:-val));
				break;
			case STAMINA: //modify stamina
				SetUInt32Value(UNIT_FIELD_STAMINA,GetUInt32Value(UNIT_FIELD_STAMINA)+(apply? val:-val));
				SetUInt32Value(PLAYER_FIELD_POSSTAT2,GetUInt32Value(PLAYER_FIELD_POSSTAT2)+(apply? val:-val));
				SetUInt32Value(UNIT_FIELD_MAXHEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH)+(apply? val:-val)*10);
				break;
		}
	}
 
	if (proto->Armor)
		SetUInt32Value(UNIT_FIELD_ARMOR, GetUInt32Value(UNIT_FIELD_ARMOR) + (apply ? proto->Armor : -(int32)proto->Armor));

	if (proto->Block)
		SetUInt32Value(PLAYER_BLOCK_PERCENTAGE, GetUInt32Value(PLAYER_BLOCK_PERCENTAGE) + (apply ? proto->Block : -(int32)proto->Block));

	if (proto->HolyRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_01, GetUInt32Value(UNIT_FIELD_RESISTANCES_01) + (apply ? proto->HolyRes : -(int32)proto->HolyRes));

	if (proto->FireRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_02, GetUInt32Value(UNIT_FIELD_RESISTANCES_02) + (apply ? proto->FireRes : -(int32)proto->FireRes));

	if (proto->NatureRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_03, GetUInt32Value(UNIT_FIELD_RESISTANCES_03) + (apply ? proto->NatureRes : -(int32)proto->NatureRes));

	if (proto->FrostRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_04, GetUInt32Value(UNIT_FIELD_RESISTANCES_04) + (apply ? proto->FrostRes : -(int32)proto->FrostRes));

	if (proto->ShadowRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_05, GetUInt32Value(UNIT_FIELD_RESISTANCES_05) + (apply ? proto->ShadowRes : -(int32)proto->ShadowRes));

	if (proto->ArcaneRes)
		SetUInt32Value(UNIT_FIELD_RESISTANCES_06, GetUInt32Value(UNIT_FIELD_RESISTANCES_06) + (apply ? proto->ArcaneRes : -(int32)proto->ArcaneRes));

	uint8 MINDAMAGEFIELD;
	uint8 MAXDAMAGEFIELD;
		
	if( proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT ||  proto->InventoryType == INVTYPE_THROWN )	
	{	
		MINDAMAGEFIELD = UNIT_FIELD_MINRANGEDDAMAGE;
		MAXDAMAGEFIELD = UNIT_FIELD_MAXRANGEDDAMAGE;
	}
	else
	{
		MINDAMAGEFIELD=(slot==EQUIPMENT_SLOT_OFFHAND)?UNIT_FIELD_MINOFFHANDDAMAGE:UNIT_FIELD_MINDAMAGE;
		MAXDAMAGEFIELD=(slot==EQUIPMENT_SLOT_OFFHAND)?UNIT_FIELD_MAXOFFHANDDAMAGE:UNIT_FIELD_MAXDAMAGE;
	}

	if (proto->Damage[0].DamageMin > 0) 
	{
		SetFloatValue(MINDAMAGEFIELD, GetFloatValue(MINDAMAGEFIELD) + (apply ? proto->Damage[0].DamageMin : -proto->Damage[0].DamageMin));
		if (apply) 
			sLog.outString("adding %f mindam ",proto->Damage[0].DamageMin);
		 else 
			sLog.outString("removing %f mindam ",proto->Damage[0].DamageMin);
		
	}

	if (proto->Damage[0].DamageMax  > 0)
	{
		SetFloatValue(MAXDAMAGEFIELD, GetFloatValue(MAXDAMAGEFIELD) + (apply ? proto->Damage[0].DamageMax : -proto->Damage[0].DamageMax));
		if (apply) 
			sLog.outString("adding %f maxdam ",proto->Damage[0].DamageMax);
		else 
			sLog.outString("removing %f maxdam ",proto->Damage[0].DamageMax);
		
	}

	if (proto->Delay) {
		if(slot!=EQUIPMENT_SLOT_OFFHAND)
			SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, apply ? proto->Delay : 2000);
		else SetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1, apply ? proto->Delay : 2000);
	}
	
	
	
	if(apply) 
	 CastItemSpell(item,(Unit*)this);
	else for (int i = 0; i < 5; i++)  
		RemoveAura(proto->Spells[i].SpellId );
		
	_ApplyStatsMods();
		 
}

void Player::CastItemSpell(Item *item,Unit* Target)
{
	if(!item) return;
	
	ItemPrototype *proto = item->GetProto();
	
	if(!proto) return;
	
	Spell *spell;
	SpellEntry *spellInfo; 
	
	for (int i = 0; i < 5; i++)  
	{                
		if(!proto->Spells[i].SpellId ) continue;
			
		spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId); 
		if(!spellInfo) { 
			DEBUG_LOG("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId); 
			continue; 
		}
 
		if(Target->GetGUID() == GetGUID() && !IsItemSpellToEquip(spellInfo)) continue;
		else if(Target->GetGUID() != GetGUID() && IsItemSpellToEquip(spellInfo)) continue;
 	                        
		DEBUG_LOG("WORLD: cast Item spellId - %i", proto->Spells[i].SpellId); 
   
		spell = new Spell(this, spellInfo, false, 0); 
		WPAssert(spell); 
	         
		SpellCastTargets targets; 
		targets.m_unitTarget = Target;
		spell->m_CastItem = item; 
		spell->prepare(&targets);
	} 
}

// only some item spell/auras effects can be executed when item is equiped.
// If not you can have unexpected beaviur. like item giving damage to player when equip.
bool Player::IsItemSpellToEquip(SpellEntry *spellInfo)
{
	for(int j = 0; j< 3; j++)
	{
		if(spellInfo->Effect[j] == 6)
		{
			switch(spellInfo->EffectApplyAuraName[j])
			{
				case 3:
				case 23:
				case 8:
				case 84:
				case 85:
				case 42:
				case 43:	
					return false;
			}
		}
	}
	
	return true;
}

// only some item spell/auras effects can be executed when in combat.
// If not you can have unexpected beaviur. like having stats always growing each attack.
bool Player::IsItemSpellToCombat(SpellEntry *spellInfo)
{
	for(int j = 0; j< 3; j++)
	{
		if(spellInfo->Effect[j] == 6)
		{
			switch(spellInfo->EffectApplyAuraName[j])
			{
				case 3:
				case 23:
				case 8:
				case 84:
				case 85:
				case 42:
				case 43:	
					return true;
			}
		}
	}
	
	return false;	
}

void Player::_RemoveAllItemMods() {
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++) {
		 if(m_items[i])
			 _ApplyItemMods(m_items[i],i, false);
	 }
}

void Player::_ApplyAllItemMods() {
	for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++) {
		if(m_items[i])
			_ApplyItemMods(m_items[i],i, true);
	}
}

/*Loot type MUST be
1-corpse, go
2-skinning
3-Fishing
*/

void Player::SendLoot(uint64 guid,uint8 loot_type)
{
	
	Loot * loot;

	if(IS_GAMEOBJECT_GUID(guid))
	{
		GameObject* go = ObjectAccessor::Instance().GetGameObject(*((Unit*)this), guid);
		if(!go)return;
		go->generateLoot();

		loot=&go->loot;

		

	}
	else
	{		
		Creature*creature = ObjectAccessor::Instance().GetCreature(*((Unit*)this), guid);
		if(!creature) return;
		
		loot=&creature->loot;
		if(loot_type==2)
			FillLoot(loot,creature->GetCreatureInfo()->SkinLootId);
		
	}
	
	
	
	m_lootGuid = guid;

	WorldPacket data;
	data.Initialize (SMSG_LOOT_RESPONSE);
			
	data << guid;
	data << loot_type;//loot_type;
	data << loot->gold;                  
	data << (uint8)loot->items.size();  

	for(uint8 i = 0; i < loot->items.size(); i++)
	{
		data << uint8(i); 
		data << uint32(loot->items[i].item.itemid);    
		data << uint32(1);//nr of items of this type
		data << uint32(loot->items[i].item.displayid); 
		data << uint64(0) << uint8(0);
	}

	SendMessageToSet(&data,true);
}
