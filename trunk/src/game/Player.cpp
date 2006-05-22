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
#include "Pet.h"
#include "SpellAuras.h"

#include <cmath>

uint8 b_HP=0, b_Spirit=0, b_Stamina=0, b_IQ=0, b_Agility=0, b_Strength=0, b_Weaps;
uint8 exHP=0, exSpirit=0, exStamina=0, exIQ=0, exAgility=0, exStrangth=0;

Player::Player (WorldSession *session): Unit()
{
    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    m_session = session;

    info = NULL;
    m_divider = 0;

    m_afk = 0;
    m_curTarget = 0;
    m_curSelection = 0;
    m_lootGuid = 0;
    m_petInfoId = 0;
    m_petLevel = 0;
    m_petFamilyId = 0;

    m_regenTimer = 0;
    m_dismountCost = 0;

    m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

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

    inCombat = false;
    pTrader = NULL;

    m_cinematic = 0;

    PlayerTalkClass = new PlayerMenu( GetSession() );
    m_timedquest = 0;
    m_currentBuybackSlot = 0;

    for ( int aX = 0 ; aX < 8 ; aX++ )
        m_Tutorials[ aX ] = 0x00;
    ItemsSetEff[0]=NULL;
    ItemsSetEff[1]=NULL;
    ItemsSetEff[2]=NULL;
    m_regenTimer = 0;
    m_breathTimer = 0;
    m_isunderwater = 0;
    m_restTime = 0;
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

    delete info;
    delete PlayerTalkClass;
}

bool Player::Create( uint32 guidlow, WorldPacket& data )
{
    int i;
    uint8 race,class_,gender,skin,face,hairStyle,hairColor,facialHair,outfitId;

    Object::_Create(guidlow, HIGHGUID_PLAYER);

    data >> m_name;
    data >> race >> class_ >> gender >> skin >> face;
    data >> hairStyle >> hairColor >> facialHair >> outfitId;

    info = objmgr.GetPlayerCreateInfo((uint32)race, (uint32)class_);
    if(!info) return false;

    for (i = 0; i < BANK_SLOT_BAG_END; i++)
        m_items[i] = NULL;

    for(int j = 0; j < BUYBACK_SLOT_END; j++)
    {
        m_buybackitems[j] = NULL;
        //        SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+j*2,0);
        //        SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+j,0);
        //        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+j,0);
    }

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
        case WARRIOR: powertype = 1; unitfield = 0x11000000; break;
        case PALADIN: powertype = 0; unitfield = 0x0000EE00; break;
        case HUNTER: powertype = 0; unitfield = 0x0000EE00; break;
        case ROGUE: powertype = 3; unitfield = 0x0000EE00; break;
        case PRIEST: powertype = 0; unitfield = 0x0000EE00; break;
        case SHAMAN: powertype = 0; unitfield = 0x0000EE00; break;
        case MAGE: powertype = 0; unitfield = 0x0000EE00; break;
        case WARLOCK: powertype = 0; unitfield = 0x0000EE00; break;
        case DRUID: powertype = 0; unitfield = 0x0000EE00; break;
    }

    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    SetUInt32Value(UNIT_FIELD_STR, info->strength );
    SetUInt32Value(UNIT_FIELD_AGILITY, info->ability );
    SetUInt32Value(UNIT_FIELD_STAMINA, info->stamina );
    SetUInt32Value(UNIT_FIELD_IQ, info->intellect );
    SetUInt32Value(UNIT_FIELD_SPIRIT, info->spirit );
    SetUInt32Value(UNIT_FIELD_ARMOR, info->basearmor );
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, info->attackpower );

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
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 2000 );       // melee attack time
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, 2000  );    // ranged attack time

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
                                                            //-1 is default value
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

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
        SetUInt32Value(PLAYER_FIELD_HONOR_HIGHEST_RANK, 0);    SetUInt32Value(PLAYER_FIELD_TODAY_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONORABLE_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONORABLE_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_HONOR, 0);    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, 0);    SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONOR, 0);    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_HONOR, 0);    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_STANDING, 0);    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONOR, 0);    SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, 0);
    */

    _ApplyStatsMods();

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

    for (; item_id_itr!=info->item_id.end(); item_id_itr++, item_bagIndex_itr++, item_slot_itr++, item_amount_itr++)
    {
        titem_id = (*item_id_itr);
        titem_bagIndex = (*item_bagIndex_itr);
        titem_slot = (*item_slot_itr);
        titem_amount = (*item_amount_itr);

        if (titem_id)
        {
            sLog.outDebug("ITEM: Creating initial item, itemId = %u, bagIndex = %u, slot = %u, count = %u",titem_id, titem_bagIndex, titem_slot, titem_amount);
            AddItem(titem_bagIndex, titem_slot, CreateNewItem(titem_id, titem_amount), true);
            //AddNewItem(titem_bagIndex, titem_slot, titem_id, titem_amount, false, false);
        }
    }

    spell_itr = info->spell.begin();

    for (; spell_itr!=info->spell.end(); spell_itr++)
    {
        tspell = (*spell_itr);
        if (tspell)
        {
            sLog.outDebug("PLAYER: Adding initial spell, id = %u",tspell);
            addSpell(tspell, 0);
        }
    }

    for(i=0 ; i<3; i++)
        skill_itr[i] = info->skill[i].begin();

    for (; skill_itr[0]!=info->skill[0].end() && skill_itr[1]!=info->skill[1].end() && skill_itr[2]!=info->skill[2].end(); )
    {
        for (i=0; i<3; i++)
            tskill[i] = (*skill_itr[i]);

        if (tskill[0])
        {
            sLog.outDebug("PLAYER: Adding initial skill line, skillId = %u, value = %u, max = %u", tskill[0], tskill[1], tskill[2]);
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

    m_petInfoId = 0;
    m_petLevel = 0;
    m_petFamilyId = 0;

    m_highest_rank = 0;
    m_last_week_rank = 0;

    // Skill spec +5
    if (Player::HasSpell(20597))
    {
        SetSkill(43,1 ,10);
        SetSkill(55,1 ,10);
    }
    if (Player::HasSpell(20864))
    {
        SetSkill(54,1 ,10);
        SetSkill(160,1 ,10);
    }
    if (Player::HasSpell(20574))
    {
        SetSkill(44,1 ,10);
        SetSkill(172,1 ,10);
    }
    if (Player::HasSpell(20558))
    {
        SetSkill(176,1 ,10);
    }
    if (Player::HasSpell(26290))
    {
        SetSkill(45,1 ,10);
        SetSkill(226,1 ,10);
    }

    //+5% HP if has skill Endurance
    if (Player::HasSpell(20550))
    {
        b_HP = uint32(GetUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.05);
        SetUInt32Value(UNIT_FIELD_MAXHEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH) + b_HP);
    }

    // school resistances
    if (Player::HasSpell(20596))
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_04, 10);
    }
    if (Player::HasSpell(20583))
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_03, 10);
    }
    if (Player::HasSpell(20579))
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_05, 10);
    }
    if (Player::HasSpell(20592))
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_06, 10);
    }
    return true;
}

void Player::StartMirrorTimer(uint8 Type, uint32 MaxValue)
{
    //TYPE: 0 = fartigua 1 = breath 2 = fire?
    WorldPacket data;
    uint32 BreathRegen = (uint32)-1;
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
    //m_session->SendPacket(&data);
    //Let other players see that you get damage
    SendMessageToSet(&data, true);
    DealDamage((Unit*)this, Amount, 0, true);
}

void Player::HandleDrowing(uint32 UnderWaterTime)
{
    WorldPacket data;

    if (Player::HasSpell(5227))
    {
        UnderWaterTime*=4;
    }

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
            //TODO: Check this formula
            uint64 guid = GetGUID();
            uint32 damage = (GetUInt32Value(UNIT_FIELD_MAXHEALTH) / 5) + rand()%GetUInt32Value(UNIT_FIELD_LEVEL);

            EnvironmentalDamage(guid, DAMAGE_DROWNING,damage);
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
            //uint32 damage = 10;
            uint32 damage = (GetUInt32Value(UNIT_FIELD_MAXHEALTH) / 3) + rand()%GetUInt32Value(UNIT_FIELD_LEVEL);

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

    Quest *pQuest;

    if ( m_timedquest > 0 )
    {
        pQuest = objmgr.GetQuest( m_timedquest );

        if (pQuest)
            if ( !pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED) ) pQuest = NULL;

        if (pQuest)
        {
            if ( mQuestStatus[m_timedquest].m_timer <= p_time)
            {

                PlayerTalkClass->SendQuestIncompleteToLog( pQuest );
                PlayerTalkClass->SendQuestUpdateFailedTimer( pQuest );

                mQuestStatus[m_timedquest].status = QUEST_STATUS_INCOMPLETE;
                m_timedquest = 0;
                mQuestStatus[m_timedquest].m_timer = 0;
                mQuestStatus[m_timedquest].m_timerrel = 0;
            }
            else
            {
                mQuestStatus[m_timedquest].m_timer -= p_time;
            }
        }
    }

    if (m_state & UNIT_STAT_ATTACKING)
    {
        inCombat = true;

        if (isAttackReady())
        {
            Unit *pVictim = NULL;
            pVictim = ObjectAccessor::Instance().GetCreature(*this, m_curSelection);
            if(!pVictim)
                pVictim = (Unit *)ObjectAccessor::Instance().FindPlayer(m_curSelection);

            // default combat reach 10
            // TODO add weapon,skill check

            float pldistance = 10.0f;
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
                clearUnitState(UNIT_STAT_IN_COMBAT);
                smsg_AttackStop(m_curSelection);
            }
            else if( GetDistanceSq(pVictim) > pldistance )
            {
                setAttackTimer(uint32(1000));
                data.Initialize(SMSG_ATTACKSWING_NOTINRANGE);
                GetSession()->SendPacket(&data);
            }
            //120 degreas of radiant range
            //(120/360)*(2*PI) = 2,094395102/2 = 1,047197551    //1,57079633-1,047197551   //1,57079633+1,047197551
            else if( !IsInArc( 2.0943951024, pVictim ))
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
    }
    else
    {
        inCombat = false;
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
            m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
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

    *p_data << uint8(GetUInt32Value(UNIT_FIELD_LEVEL));     //1
    uint32 zoneId=sAreaStore.LookupEntry(MapManager::Instance ().GetMap(m_mapId)->GetAreaFlag(m_positionX,m_positionY))->zone;

    *p_data << zoneId;
    *p_data << GetMapId();

    *p_data << m_positionX;
    *p_data << m_positionY;
    *p_data << m_positionZ;

    *p_data << GetUInt32Value(PLAYER_GUILDID);              //probebly wrong

    //*p_data << GetUInt32Value(PLAYER_GUILDRANK);    //this was
    *p_data << uint8(0x0);
    *p_data << uint8(GetUInt32Value(PLAYER_FLAGS) << 1);
    *p_data << uint8(0x0);                                  //Bit 4 is something dono
    *p_data << uint8(0x0);                                  //is this player_GUILDRANK????

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
    MapManager::Instance().GetMap(GetMapId())->Remove(this, false);
    WorldPacket data;
    data.Initialize(SMSG_TRANSFER_PENDING);
    data << uint32(mapid);
    GetSession()->SendPacket(&data);

    data.Initialize(SMSG_NEW_WORLD);
    data << (uint32)mapid << (float)x << (float)y << (float)z << (float)orientation;
    GetSession()->SendPacket( &data );

    SetMapId(mapid);
    Relocate(x,y,z,orientation);
    SetPosition(x,y,z,orientation);
    SetDontMove(true);
    //SaveToDB();

    MapManager::Instance().GetMap(mapid)->Add(this);
}

void Player::AddToWorld()
{
    Object::AddToWorld();

    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->AddToWorld();
    }
    AddWeather();
}

void Player::RemoveFromWorld()
{

    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->RemoveFromWorld();
    }

    Object::RemoveFromWorld();
}

void Player::CalcRage( uint32 damage,bool attacker )
{

    uint32 maxRage = GetUInt32Value(UNIT_FIELD_MAXPOWER2);
    uint32 Rage = GetUInt32Value(UNIT_FIELD_POWER2);

    if(attacker)
        Rage += (uint32)(damage/(getLevel()*0.5f));
    else
        Rage += (uint32)(damage/(getLevel()*1.5f));

    if(Rage > maxRage)  Rage = maxRage;

    SetUInt32Value(UNIT_FIELD_POWER2, Rage);
}

void Player::RegenerateAll()
{

    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;

    if (!(m_state & UNIT_STAT_ATTACKING))
    {
        Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH);
        Regenerate( UNIT_FIELD_POWER2, UNIT_FIELD_MAXPOWER2);
    }

    else
    {
        if (Player::HasSpell(20555))
        {
            Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH);
        }
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
        if (curValue >= maxValue)   return;
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
            if (Player::HasSpell(20555))
            {
                if (Player::inCombat)
                {
                    addvalue*=uint32(0.10);
                }
                else
                {
                    addvalue*=uint32(1.10);
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
            case PLAYER_STATE_SIT:          addvalue = (uint32)(addvalue*2.0f); break;
            case PLAYER_STATE_SLEEP:        addvalue = (uint32)(addvalue*3.0f); break;
            case PLAYER_STATE_KNEEL:        addvalue = (uint32)(addvalue*1.5f); break;
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

void Player::GiveXP(uint32 xp, const uint64 &guid)
{
    if ( xp < 1 )
        return;

    WorldPacket data;
    if (guid != 0)
    {
        data.Initialize( SMSG_LOG_XPGAIN );
        data << guid;
        data << uint32(xp);                                 // given experience
        data << uint8(0);                                   // 00-kill_xp type, 01-non_kill_xp type
        uint32 xpunrested = xp/2;
        data << uint32(xpunrested);                         // unrested given experience
                                                            // unknown (static.. it was same at 4 different killed creatures!)
        data << uint8(0) << uint8(0) << uint8(0x80) << uint8(0x3f);
        GetSession()->SendPacket(&data);
    }

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp;

    while (newXP >= nextLvlXP)
    {
        uint16 level = (uint16)GetUInt32Value(UNIT_FIELD_LEVEL);

        uint32 MPGain,HPGain,STRGain,STAGain,AGIGain,INTGain,SPIGain;
        MPGain=HPGain=STRGain=STAGain=AGIGain=INTGain=SPIGain=0;

        level += 1;
        newXP -= nextLvlXP;

        //_RemoveStatsMods();

        BuildLvlUpStats(&HPGain,&MPGain,&STRGain,&STAGain,&AGIGain,&INTGain,&SPIGain);

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
        nextLvlXP = MaNGOS::XP::xp_to_level(level);

        UpdateMaxSkills ();

        //fill new stats
        if(getClass() != WARRIOR && getClass() != ROGUE)
        {
            SetUInt32Value(UNIT_FIELD_POWER1, newMP);
            SetUInt32Value(UNIT_FIELD_MAXPOWER1, newMP);
        }

        if (Player::HasSpell(20550))                        //endurance skill support (+5% to total health)
        {
            (uint32)exHP = (uint32)newHP / 1.05;                    //must remove previous bonus, so stat wouldn't grow toomuch
            b_HP = uint8(exHP * 0.05);
            newHP += b_HP;
        }
        if (Player::HasSpell(20598))                        //Human Spirit skill support (+5% to total spirit)
        {
            (uint32)exSpirit = (uint32)newSPI / 1.05;               //must remove previous bonus, so stat wouldn't grow toomuch
            b_Spirit = uint8(exSpirit * 0.05);
            newSPI += b_Spirit;
        }
        if (Player::HasSpell(20591))                        //Expansive mind support (+5% to total Intellect)
        {
            (uint32)exIQ = (uint32)newINT / 1.05;                   //must remove previous bonus, so stat wouldn't grow toomuch
            b_IQ = uint8(exIQ * 0.05);
            newINT += b_IQ;
        }

        SetUInt32Value(UNIT_FIELD_HEALTH, newHP);
        SetUInt32Value(UNIT_FIELD_MAXHEALTH, newHP);

        SetUInt32Value(UNIT_FIELD_STR, newSTR);
        SetUInt32Value(UNIT_FIELD_STAMINA, newSTA);
        SetUInt32Value(UNIT_FIELD_AGILITY, newAGI);
        SetUInt32Value(UNIT_FIELD_IQ, newINT);
        SetUInt32Value(UNIT_FIELD_SPIRIT, newSPI);

        //_ApplyStatsMods();

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

        SetUInt32Value(PLAYER_XP, newXP);
        uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
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
            *STR += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
            *STA += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
            *AGI += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
            *INT += (lvl > 9 && !(lvl%2) ? 1: 0);
            *SPI += (lvl > 9 && !(lvl%2) ? 1: 0);
            break;
        case PALADIN:
            //*HP +=
            //*MP +=
            *STR += (lvl > 3  ? 1: 0);
            *STA += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
            *AGI += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
            *INT += (lvl > 6 && (lvl%2) ? 1: 0);
            *SPI += (lvl > 7 ? 1: 0);
            break;
        case HUNTER:
            //*HP +=
            //*MP +=
            *STR += (lvl > 4  ? 1: 0);
            *STA += (lvl > 4  ? 1: 0);
            *AGI += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
            *INT += (lvl > 8 && (lvl%2) ? 1: 0);
            *SPI += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
            break;
        case ROGUE:
            //*HP +=
            //*MP +=
            *STR += (lvl > 5  ? 1: 0);
            *STA += (lvl > 4  ? 1: 0);
            *AGI += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
            *INT += (lvl > 8 && !(lvl%2) ? 1: 0);
            *SPI += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
            break;
        case PRIEST:
            //*HP +=
            //*MP +=
            *STR += (lvl > 9 && !(lvl%2) ? 1: 0);
            *STA += (lvl > 5  ? 1: 0);
            *AGI += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
            *INT += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
            *SPI += (lvl > 3  ? 1: 0);
            break;
        case SHAMAN:
            //*HP +=
            //*MP +=
            *STR += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
            *STA += (lvl > 4 ? 1: 0);
            *AGI += (lvl > 7 && !(lvl%2) ? 1: 0);
            *INT += (lvl > 5 ? 1: 0);
            *SPI += (lvl > 4 ? 1: 0);
            break;
        case MAGE:
            //*HP +=
            //*MP +=
            *STR += (lvl > 9 && !(lvl%2) ? 1: 0);
            *STA += (lvl > 5  ? 1: 0);
            *AGI += (lvl > 9 && !(lvl%2) ? 1: 0);
            *INT += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
            *SPI += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
            break;
        case WARLOCK:
            //*HP +=
            //*MP +=
            *STR += (lvl > 9 && !(lvl%2) ? 1: 0);
            *STA += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
            *AGI += (lvl > 9 && !(lvl%2) ? 1: 0);
            *INT += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
            *SPI += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
            break;
        case DRUID:
            *STR += (lvl > 38 ? 2: (lvl > 6 && (lvl%2) ? 1: 0));
            *STA += (lvl > 32 ? 2: (lvl > 4 ? 1: 0));
            *AGI += (lvl > 38 ? 2: (lvl > 8 && (lvl%2) ? 1: 0));
            *INT += (lvl > 38 ? 3: (lvl > 4 ? 1: 0));
            *SPI += (lvl > 38 ? 3: (lvl > 5 ? 1: 0));
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

void Player::addSpell(uint16 spell_id, uint16 slot_id)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spell_id);
    if(!spellInfo) return;

    Playerspell *newspell;

    newspell = new Playerspell;
    newspell->spellId = spell_id;

    WorldPacket data;

    uint8 op;
    uint16 tmpslot=slot_id,val=0;
    int16 tmpval=0;
    uint16 mark=0;
    uint32 shiftdata=0x01;
    uint32 EffectVal;
    uint32 Opcode=SMSG_SET_FLAT_SPELL_MODIFIER;

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
                if(tmpval > 0)
                {
                    val =  tmpval+1;
                    mark = 0x0;
                }
                else
                {
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

            for(uint8 i=0;i<32;i++)
            {
                if ( EffectVal&shiftdata )
                {
                    data.Initialize(Opcode);
                    data << uint8(i);
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

void Player::learnSpell(uint16 spell_id)
{

    WorldPacket data;
    data.Initialize(SMSG_LEARNED_SPELL);
    data <<uint32(spell_id);
    m_session->SendPacket(&data);

    addSpell(spell_id);

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

    }
    //Players visible items are not inventory stuff
    //431) = 884 (0x374) = main weapon
    for(uint16 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        updateMask->SetBit((uint16)(PLAYER_VISIBLE_ITEM_1_0 + (i*12)));
        //updateMask->SetBit((uint16)(PLAYER_VISIBLE_ITEM_1_0 + 1 + (i*12)));
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

    for(int i = 0; i < EQUIPMENT_SLOT_END; i++)
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
            if(spellInfo->Effect[i]==36)                    // Learn Spell effect
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

void Player::DeleteFromDB()
{
    uint32 guid = GetGUIDLow();

    sDatabase.PExecute("DELETE FROM `character` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_spell` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_tutorial` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_inventory` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_social` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `mail` WHERE `receiver` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `game_corpse` WHERE `player` = '%u'",guid);

    for(int i = 0; i < BANK_SLOT_ITEM_END; i++)
    {
        if(m_items[i] == NULL)
            continue;
        m_items[i]->DeleteFromDB();
        if(m_items[i]->IsBag())
            ((Bag*)m_items[i])->DeleteFromDB();
    }

    sDatabase.PExecute("DELETE FROM `character_queststatus` WHERE `playerid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_action` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_reputation` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_homebind` WHERE `guid` = '%u'",guid);
    sDatabase.PExecute("DELETE FROM `character_kill` WHERE `guid` = '%u'",guid);
    // Temporary disabled, we need to lookup both auctionhouse and auctionhouse_items
    // together. auctionhouse_items are saved by item_guid not by player guid.
    // sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `itemowner` = '%u'",guid);
    // sDatabase.PExecute("DELETE FROM `auctionhouse_item` WHERE `guid` = '%u'",guid);
    // sDatabase.PExecute("DELETE FROM `auctionhouse_bid` WHERE `bidder` = '%u'",guid);

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
        case MOVE_RUN:
        {
            SetSpeed( value / SPEED_RUN );
            if(forced) { data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_RUN_SPEED); }
            data << uint8(0xFF);
            data << GetGUID();
            data << (uint32)0;
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_WALKBACK:
        {
            SetSpeed( value / SPEED_WALKBACK );
            if(forced) { data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED); }
            data << uint8(0xFF);
            data << GetGUID();
            data << (uint32)0;
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_SWIM:
        {
            SetSpeed( value / SPEED_SWIM );
            if(forced) { data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE); }
            else { data.Initialize(MSG_MOVE_SET_SWIM_SPEED); }
            data << uint8(0xFF);
            data << GetGUID();
            data << (uint32)0;
            data << float(value);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_SWIMBACK:
        {
            SetSpeed( value / SPEED_SWIMBACK );
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED);
            data << uint8(0xFF);
            data << GetGUID();
            data << (uint32)0;
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
        SetPlayerSpeed(MOVE_RUN, (float)12.75, true);
        SetPlayerSpeed(MOVE_SWIM, (float)8.85, true);
    }
    else
    {
        SetPlayerSpeed(MOVE_RUN, (float)10.625, true);
        SetPlayerSpeed(MOVE_SWIM, (float)7.375, true);
    }

    //! corpse reclaim delay 30 * 1000ms
    data.Initialize(SMSG_CORPSE_RECLAIM_DELAY );
    data << (uint32)30000;
    GetSession()->SendPacket( &data );

    //TODO: Check/research this
    data.Initialize(SMSG_SPELL_START );
    data << uint8(0xFF) << GetGUID()                        //9
        << uint8(0xFF) << GetGUID()                         //9
    //<< uint16(8326); //2
        << uint32(20305)                                    //2
        << uint16(0x02)

        << uint32(0x00)<< uint16(0x00);                     //6
    GetSession()->SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO);
    data << uint8(0xFF) << GetGUID() << uint8(0xFF) << GetGUID() << uint16(8326);
                                                            /// uint8(0x0D) = probably race + 2
    data << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << GetGUID();
    data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
    GetSession()->SendPacket( &data );

    data.Initialize(SMSG_UPDATE_AURA_DURATION);
    data << uint32(0x20) << uint8(0);
    GetSession()->SendPacket( &data );

    StopMirrorTimer(0);                                     //disable timers(bars)
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    SetUInt32Value(UNIT_FIELD_FLAGS, 0x08);
    SetUInt32Value(UNIT_FIELD_AURA + 32, 8326);             // set ghost form

    SetUInt32Value(UNIT_FIELD_AURA + 33, 0x5068 );          //!dono

    SetUInt32Value(UNIT_FIELD_AURAFLAGS + 4, 0xEE);

    SetUInt32Value(UNIT_FIELD_AURASTATE, 0x02);

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,(float)1.0);    //see radius of death player?

    SetUInt32Value(UNIT_FIELD_BYTES_1, 0x1000000);          //Set standing so always be standing

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

    SetMovement(MOVE_LAND_WALK);
    SetMovement(MOVE_UNROOT);

    SetPlayerSpeed(MOVE_RUN, (float)7.5, true);
    SetPlayerSpeed(MOVE_SWIM, (float)4.9, true);

    SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);

    SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

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
    if(!m_pCorpse->Create(objmgr.GenerateLowGuid(HIGHGUID_CORPSE), this, GetMapId(), GetPositionX(),
        GetPositionY(), GetPositionZ(), GetOrientation()))
        return;

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

    std::string corpsename = m_name;
    corpsename.append(" corpse.");

    this->PlayerTalkClass->SendPointOfInterest( GetPositionX(), GetPositionY(), 7, 6, 30, corpsename.c_str());
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

    m_pCorpse->SaveToDB(true);
    m_pCorpse = NULL;

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
                //        if ( pNewDurability == 0 || pNewDurability * 100 / pDurability < 25)
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

        // we should be able to make 2 kinds of teleport after death
        // near if in the same zoneid and far if in different zoneid

        WorldPacket data;

        // teleport near
        //SetDontMove(true);
        //MapManager::Instance().GetMap(GetMapId())->Remove(this, false);

        BuildTeleportAckMsg(&data, closestX, closestY, closestZ, 0.0);
        GetSession()->SendPacket(&data);

        SetPosition( closestX, closestY, closestZ, 0.0);
        BuildHeartBeatMsg(&data);
        SendMessageToSet(&data, true);
        //RemoveFromWorld();
        //MapManager::Instance().GetMap(GetMapId())->Add(this);
        //SetDontMove(false);
        //SaveToDB();

        // teleport far
        //smsg_NewWorld(GetMapId(), closestX, closestY, closestZ, 0.0);

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

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_social` WHERE `flags` = 'FRIEND' AND `guid` = '%u';", GetGUIDLow());

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

void Player::UpdateSkillPro(uint32 spellid)
{
    SkillLineAbility *pSkill = sSkillLineAbilityStore.LookupEntry(spellid);
    if(!pSkill)
        return;
    uint32 minValue = pSkill->min_value;
    uint32 maxValue = pSkill->max_value;
    uint32 skill_id = pSkill->miscid;

    if(!skill_id)return;
    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if (GetUInt32Value(PLAYER_SKILL(i)) == skill_id) break;
    if(i>=PLAYER_MAX_SKILLS) return;

    uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
    uint16 value = SKILL_VALUE(data);
    uint16 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max)) return;
    if(value >= maxValue+25 )
        return;
    else if(value >= maxValue)
    {
        if(urand(0,100) <30)
            SetUInt32Value(PLAYER_SKILL(i)+1,data+1);
        return;
    }
    else if(value >= minValue)
    {
        if(urand(0,100) <70)
            SetUInt32Value(PLAYER_SKILL(i)+1,data+1);
        return;
    }
    else if(value >= 1)
    {
        SetUInt32Value(PLAYER_SKILL(i)+1,data+1);
        return;
    }
    else return;

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
        uint32 pskill = GetUInt32Value(PLAYER_SKILL(i));
        if(pskill == SKILL_HERBALISM || pskill == SKILL_MINING || pskill ==SKILL_FISHING
            || pskill == SKILL_FIRST_AID || pskill == SKILL_COOKING || pskill == SKILL_LEATHERWORKING
            || pskill == SKILL_BLACKSMITHING || pskill == SKILL_ALCHEMY || pskill == SKILL_ENCHANTING
            || pskill == SKILL_TAILORING || pskill == SKILL_ENGINERING || pskill == SKILL_SKINNING)
            continue;
        uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
        uint32 max=data>>16;
        uint32 max_Skill = data%0x10000+GetUInt32Value(UNIT_FIELD_LEVEL)*5*0x10000;
        if(max!=1 && max != 300)
        {
            SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill);
            if (Player::HasSpell(20597))
            {
                if (GetUInt32Value(PLAYER_SKILL(i)) == 43 || GetUInt32Value(PLAYER_SKILL(i)) == 55)
                {
                    SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill+5*0x10000);
                }
            }
            if (Player::HasSpell(20864))
            {
                if (GetUInt32Value(PLAYER_SKILL(i))==54 || GetUInt32Value(PLAYER_SKILL(i))==160)
                {
                    SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill+5*0x10000);
                }
            }
            if (Player::HasSpell(20574))
            {
                if (GetUInt32Value(PLAYER_SKILL(i))==44 || GetUInt32Value(PLAYER_SKILL(i))==172)
                {
                    SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill+5*0x10000);
                }
            }
            if (Player::HasSpell(20558))
            {
                if (GetUInt32Value(PLAYER_SKILL(i))==176)
                {
                    SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill+5*0x10000);
                }
            }
            if (Player::HasSpell(26290))
            {
                if (GetUInt32Value(PLAYER_SKILL(i))==45 || GetUInt32Value(PLAYER_SKILL(i))==226)
                {
                    SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill+5*0x10000);
                }
            }
        }
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

    if(i<PLAYER_MAX_SKILLS)                                 //has skill
    {
        if(currVal)
            SetUInt32Value(PLAYER_SKILL(i)+1,currVal+maxVal*0x10000);
        else                                                //remove
        {
            SetUInt64Value(PLAYER_SKILL(i),0);
            SetUInt32Value(PLAYER_SKILL(i)+2,0);
        }
    }else if(currVal)                                       //add
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
    sLog.outString( "Initializing Action Buttons for '%u'", GetGUIDLow() );
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
    sLog.outString( "Action Buttons for '%u' Initialized", GetGUIDLow() );
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
    sLog.outString( "Player '%u' Added Action '%u' to Button '%u'", GetGUIDLow(), action, button );
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
    sLog.outString( "Action Button '%u' Removed from Player '%u'", button, GetGUIDLow() );
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

void Player::DealWithSpellDamage(DynamicObject &obj)
{
    obj.DealWithSpellDamage(*this);
}

bool Player::SetPosition(const float &x, const float &y, const float &z, const float &orientation)
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

void Player::SendMessageToSet(WorldPacket *data, bool self)
{
    MapManager::Instance().GetMap(m_mapId)->MessageBoardcast(this, data, self);
}

void Player::SendDirectMessage(WorldPacket *data)
{
    m_session->SendPacket(data);
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
                m_faction = 115;
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
                m_faction = 116;
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
                itr->Standing = (((int)itr->Standing + standing) > 0 ? itr->Standing + standing: 0);
                itr->Flags = (itr->Flags | 1);
                UpdateReputation();
                return true;
            }
        }
    }

    return false;
}

//Update honor fields
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

    float total_honor = 0;
    float yestardayHonor = 0;
    float thisWeekHonor = 0;
    float lastWeekHonor = 0;

    time( &rawtime );
    now = localtime( &rawtime );

    today = ((uint32)(now->tm_year << 16)|(uint32)(now->tm_yday));

    Yestarday     = today - 1;
    ThisWeekBegin = today - now->tm_wday;
    ThisWeekEnd   = ThisWeekBegin + 7;
    LastWeekBegin = ThisWeekBegin - 7;
    LastWeekEnd   = LastWeekBegin + 7;

    sLog.outDetail("PLAYER: UpdateHonor");

    QueryResult *result = sDatabase.PQuery("SELECT `type`,`honor`,`date` FROM `character_kill` WHERE `guid` = '%u';", GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            if(fields[0].GetUInt32() == HONORABLE_KILL)
            {
                lifetime_honorableKills++;
                total_honor += fields[1].GetFloat();
            }
            else if(fields[0].GetUInt32() == DISHONORABLE_KILL)
            {
                lifetime_dishonorableKills++;
                total_honor -= fields[1].GetFloat();
            }

            date = fields[2].GetUInt32();

            if( date == today)
            {
                if(fields[0].GetUInt32() == HONORABLE_KILL)
                    today_honorableKills++;
                else
                if(fields[0].GetUInt32() == DISHONORABLE_KILL)
                    today_dishonorableKills++;
            }
            //if is a honorable kill
            if(fields[0].GetUInt32() == HONORABLE_KILL)
            {
                if( date == Yestarday)
                {
                    yestardayKills++;
                    yestardayHonor += fields[1].GetFloat();
                }
                if( (date >= ThisWeekBegin) && (date < ThisWeekEnd) )
                {
                    thisWeekKills++;
                    thisWeekHonor += fields[1].GetFloat();
                }
                if( (date >= LastWeekBegin) && (date < LastWeekEnd) )
                {
                    lastWeekKills++;
                    lastWeekHonor += fields[1].GetFloat();
                }
            }

        }
        while( result->NextRow() );

        delete result;
    }

    //TODAY
    SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, (today_dishonorableKills << 16) + today_honorableKills );
    //YESTERDAY
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_KILLS, yestardayKills);
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, (uint32)yestardayHonor);
    //THIS WEEK
    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_KILLS, thisWeekKills);
    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_CONTRIBUTION, (uint32)thisWeekHonor);
    //LAST WEEK
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_KILLS, lastWeekKills);
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_CONTRIBUTION, (uint32)lastWeekHonor);
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK, GetHonorLastWeekRank());

    //TODO Fix next rank bar... it is not working fine!
    //NEXT RANK BAR //Total honor points
    SetUInt32Value(PLAYER_FIELD_BYTES2, (uint32)( (total_honor < 0) ? 0: total_honor) );

    if( CalculateHonorRank(total_honor) )
        SetUInt32Value(PLAYER_BYTES_3, (( (uint32)CalculateHonorRank(total_honor) << 24) + 0x04000000) );
    else
        SetUInt32Value(PLAYER_BYTES_3, 0);

    //LIFE TIME
    SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, (lifetime_dishonorableKills << 16) + lifetime_honorableKills );
    SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, lifetime_dishonorableKills);
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, lifetime_honorableKills);

    //If the new rank is highest then the old one, then m_highest_rank is updated
    if( CalculateHonorRank(total_honor) > GetHonorHighestRank() )
        m_highest_rank = CalculateHonorRank(total_honor);

    if ( GetHonorHighestRank() )
        SetUInt32Value(PLAYER_FIELD_PVP_MEDALS, ((uint32) GetHonorHighestRank() << 24) + 0x040F0001 );
    else
        SetUInt32Value(PLAYER_FIELD_PVP_MEDALS, 0);

    //Store Total Honor points...
    m_total_honor_points = total_honor;
}

int Player::GetHonorRank()
{
    return CalculateHonorRank(m_total_honor_points);
}

//What is Player's rank... private, scout...
int Player::CalculateHonorRank(float honor_points)
{
    int rank = 0;

    if(honor_points <=    0.00) rank = 0; else
        if(honor_points <  2000.00) rank = 1;
    else
        rank = ( (int)(honor_points / 5000) + 1);

    return rank;
}

//How many times Player kill pVictim...
int Player::CalculateTotalKills(Player *pVictim)
{
    int total_kills = 0;

    QueryResult *result = sDatabase.PQuery("SELECT `honor` FROM `character_kill` WHERE `guid` = '%u' AND `creature_template` = '%u';", GetGUIDLow(), pVictim->GetEntry());

    if(result)
    {
        total_kills = result->GetRowCount();
        delete result;
    }
    return total_kills;
}

//How much honor Player gains/loses killing uVictim
void Player::CalculateHonor(Unit *uVictim)
{
    float parcial_honor_points = 0;
    int kill_type = 0;
    bool savekill = false;

    sLog.outDetail("PLAYER: CalculateHonor");

    if( !uVictim ) return;

    if( uVictim->GetTypeId() == TYPEID_UNIT )
    {
        Creature *cVictim = (Creature *)uVictim;
        if( cVictim->isCivilian() )
        {
            parcial_honor_points = MaNGOS::Honor::DishonorableKillPoints( getLevel() );
            kill_type = DISHONORABLE_KILL;
            savekill = true;
        }
    }
    else
    if( uVictim->GetTypeId() == TYPEID_PLAYER )
    {
        Player *pVictim = (Player *)uVictim;

        if( GetTeam() == pVictim->GetTeam() ) return;

        if( getLevel() < (pVictim->getLevel()+5) )
        {
            parcial_honor_points = MaNGOS::Honor::HonorableKillPoints( this, pVictim );
            kill_type = HONORABLE_KILL;
            savekill = true;
        }
    }

    if (savekill)
    {
        time_t rawtime;
        struct tm * now;
        uint32 today = 0;
        time( &rawtime );
        now = localtime( &rawtime );
        today = ((uint32)(now->tm_year << 16)|(uint32)(now->tm_yday));

        sDatabase.PExecute("INSERT INTO `character_kill` (`guid`,`creature_template`,`honor`,`date`,`type`) VALUES (%u, %u, %f, %u, %u);", (uint32)GetGUIDLow(), (uint32)uVictim->GetEntry(), (float)parcial_honor_points, (uint32)today, (uint8)kill_type);

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
    if(getClass() == CLASS_ROGUE)
    {
        uint32 spellid;
        for(spellid = 1784;spellid <1787;spellid++)
        {
            RemoveAura(spellid);
        }
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
    clearUnitState(UNIT_STAT_IN_FLIGHT);
    SetUInt32Value( PLAYER_FIELD_COINAGE , m_dismountCost);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

    /* Remove the "player locked" flag, to allow movement */
    if (GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
        RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );
}

/*********************************************************/
/***                   STORAGE SYSTEM                  ***/
/*********************************************************/
void Player::SetSheath(uint32 sheathed)
{
    if (sheathed)
    {
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

        if (GetItemBySlot(EQUIPMENT_SLOT_MAINHAND))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, item->GetGUIDLow());
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_01, itemSheathType);
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, itemProto->DisplayInfoID);
        }
        if (GetItemBySlot(EQUIPMENT_SLOT_OFFHAND))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_02, item->GetGUIDLow());
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_03, itemSheathType);
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, itemProto->DisplayInfoID);
        }
        if (GetItemBySlot(EQUIPMENT_SLOT_RANGED))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_04, item->GetGUIDLow());
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_05, itemSheathType);
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, itemProto->DisplayInfoID);
        }
    }
    else
    {
        if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_01, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, uint32(0));
        }
        if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_02, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_03, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, uint32(0));
        }
        if (GetUInt32Value (UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2))
        {
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_04, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO_05, uint32(0));
            this->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, uint32(0));
        }
    }
}

bool Player::CanUseItem(ItemPrototype * proto)
{
    uint32 reqSpell = 0;
    uint32 reqSkill = proto->RequiredSkill;
    uint32 reqSkillRank = proto->RequiredSkillRank;
    uint8 error_code = EQUIP_ERR_OK;

    if (getLevel() < proto->RequiredLevel) error_code = EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;

    if (error_code)
    {
        WorldPacket data;
        Item* pItem = NewItemOrBag(proto);
        if(!pItem->Create (objmgr.GenerateLowGuid (HIGHGUID_ITEM), proto->ItemId, this))
            return false;

        data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);

        data << error_code;
        if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N)
        {
            data << proto->RequiredLevel;
        }
        data << (pItem ? pItem->GetGUID(): uint64(0));
        data << uint64(0);
        data << uint8(0);

        GetSession()->SendPacket (&data);
        delete pItem;
        return false;
    }
    else
    {
        return true;
    }
}

uint32 Player::GetSkillByProto(ItemPrototype *proto)
{
    const static uint32 item_weapon_skills[]=
    {
        SKILL_AXES, SKILL_2H_AXES,SKILL_BOWS, SKILL_GUNS,SKILL_MACES, SKILL_2H_MACES,
        SKILL_POLEARMS, SKILL_SWORDS,SKILL_2H_SWORDS,0, SKILL_STAVES,0,0,0,0, SKILL_DAGGERS,
        SKILL_THROWN, SKILL_SPEARS, SKILL_CROSSBOWS, SKILL_WANDS, SKILL_FISHING
    };

    const static uint32 item_armor_skills[]=
    {
        0,SKILL_CLOTH,SKILL_LEATHER,SKILL_MAIL,SKILL_PLATE_MAIL,0,SKILL_SHIELD
    };

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

uint32 Player::GetSpellByProto(ItemPrototype *proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_WEAPON:
            switch (proto->SubClass)
            {
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
            switch(proto->SubClass)
            {
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

void Player::GetSlotByItem(uint32 type, uint8 slots[4])
{
    for (int i = 0; i < 4; i++) slots[i] = NULL_SLOT;
    switch(type)
    {
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

uint8 Player::FindEquipSlot(uint32 type)
{
    uint8 slots[4];
    GetSlotByItem(type, slots);
    if (slots[0] == NULL_SLOT) return INVENTORY_SLOT_ITEM_END;
    for (int i = 0; i < 4; i++)
    {
        if (slots[i] != NULL_SLOT)
            if (!GetItemBySlot(slots[i])) return slots[i];
    }
    return slots[0];
}

uint8 Player::FindFreeItemSlot(uint32 type)
{
    uint8 slots[4];
    GetSlotByItem(type, slots);
    if (slots[0] == NULL_SLOT) return INVENTORY_SLOT_ITEM_END;
    for (int i = 0; i < 4; i++)
    {
        if (slots[i] != NULL_SLOT)
            if (!GetItemBySlot(slots[i])) return slots[i];
    }
    return INVENTORY_SLOT_ITEM_END;
}

int Player::CountFreeBagSlot()
{
    int count = 0;
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
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
    {                                                       // Player slots, inventory and bank slots
        if (slot < EQUIPMENT_SLOT_END)
        {                                                   // Equiping item

            if (!(proto->AllowableRace & getRaceMask()))
                return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
            if (!(proto->AllowableClass & getClassMask()))
                return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
            if (proto->RequiredLevel > getLevel())
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
                    return  EQUIP_ERR_SKILL_ISNT_HIGH_ENOUGH;

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
    {                                                       //additional bags & additional bank bags
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
bool Player::SplitItem(uint8 srcBag, uint8 srcSlot, uint8 dstBag, uint8 dstSlot, uint8 count)
{
    UpdateData upd;
    WorldPacket data;
    uint8 error_code = 0;

    Item *dstItem = GetItemBySlot(dstBag, dstSlot);
    Item *srcItem = GetItemBySlot(srcBag, srcSlot);

    if(!srcItem) error_code = EQUIP_ERR_ITEM_NOT_FOUND;

    if(!error_code)
    {
        if (count == srcItem->GetCount()) { return SwapItem(dstBag, dstSlot, srcBag, srcSlot); }
        if (count > srcItem->GetCount()) error_code = EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT;
    }

    if (dstItem && srcItem && !error_code)
    {
        // Same items
        if (dstItem->GetEntry() == srcItem->GetEntry())
        {
            uint32 stack = dstItem->GetMaxStackCount();
            uint32 dstCount = dstItem->GetCount();
            uint32 srcCount = srcItem->GetCount();

            // If item is stackable and stack is not full, add to stack
            if (dstCount+count <= stack)
            {
                dstItem->SetCount(dstCount + count);
                srcItem->SetCount(srcCount - count);
                upd.Clear();
                dstItem->BuildCreateUpdateBlockForPlayer(&upd, this);
                srcItem->BuildCreateUpdateBlockForPlayer(&upd, this);
                upd.BuildPacket(&data);
                GetSession()->SendPacket(&data);
                //_SaveInventory();
                return true;
            }
        }
        error_code = EQUIP_ERR_COULDNT_SPLIT_ITEMS;
    }

    if (!error_code)
    {
        dstItem = new Item;                                 // Don't think there are stackable bags
        if(!dstItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM),srcItem->GetEntry(),this))
            return false;
        dstItem->SetCount(count);
        error_code = CanEquipItemInSlot(dstBag, dstSlot, dstItem, srcItem);
    }

    if (!error_code)
    {
        AddItem(dstBag, dstSlot, dstItem, true);
        srcItem->SetCount(srcItem->GetCount() - count);
        srcItem->SendUpdateToPlayer(this);
        //_SaveInventory();
        return true;
    }
    else
    {
        data.Initialize (SMSG_INVENTORY_CHANGE_FAILURE);
        data << uint8(error_code);
        if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N)
        {
            uint32 reqlevel = 0;
            if (srcItem)
                if (srcItem->GetProto()->RequiredLevel > getLevel()) reqlevel = srcItem->GetProto()->RequiredLevel;
            if ((dstItem) && (!reqlevel))
                if (dstItem->GetProto()->RequiredLevel > getLevel()) reqlevel = dstItem->GetProto()->RequiredLevel;
            data << reqlevel;
        }
        data << uint64((srcItem ? srcItem->GetGUID(): 0));
        data << uint64((dstItem ? dstItem->GetGUID(): 0));
        data << uint8(0);
        m_session->SendPacket(&data);
        return false;
    }
}

// Swap items from one slot to another
bool Player::SwapItem(uint8 dstBag, uint8 dstSlot, uint8 srcBag, uint8 srcSlot)
{
    UpdateData upd;
    WorldPacket data;
    uint8 error_code = 0;

    Item *dstItem = GetItemBySlot(dstBag, dstSlot);
    Item *srcItem = GetItemBySlot(srcBag, srcSlot);

    if (dstItem && srcItem)
    {
        // Same items
        if (dstItem->GetEntry() == srcItem->GetEntry())
        {
            uint32 stack = dstItem->GetMaxStackCount();
            uint32 dstCount = dstItem->GetCount();
            uint32 srcCount = srcItem->GetCount();

            // If item is stackable and stack is not full, add to stack
            if (dstCount < stack)
            {
                dstItem->SetCount((dstCount+srcCount > stack)?stack:(dstCount+srcCount));
                upd.Clear();
                dstItem->BuildCreateUpdateBlockForPlayer(&upd, this);
                if (dstCount+srcCount > stack)
                {
                    srcItem->SetCount(srcCount - (stack - dstCount));
                    srcItem->BuildCreateUpdateBlockForPlayer(&upd, this);
                }
                else
                {
                    RemoveItemFromSlot(srcBag, srcSlot);
                    //srcItem->DeleteFromDB();
                    delete srcItem;
                }
                upd.BuildPacket(&data);
                GetSession()->SendPacket(&data);
                //_SaveInventory();
                return true;
            }
        }
    }

    if (srcItem)
    {
        error_code = CanEquipItemInSlot(dstBag, dstSlot, srcItem, dstItem);
    }
    else
    {
        error_code = EQUIP_ERR_ITEM_NOT_FOUND;
    }

    if ((!error_code) && (dstItem)) error_code = CanEquipItemInSlot(srcBag, srcSlot, dstItem, srcItem);

    if (!error_code)
    {
        if (dstItem) RemoveItemFromSlot(dstBag, dstSlot);
        if (srcItem) RemoveItemFromSlot(srcBag, srcSlot);
        if (dstItem) AddItem(srcBag, srcSlot, dstItem, true);
        if (srcItem) AddItem(dstBag, dstSlot, srcItem, true);
        //_SaveInventory();
        return true;
    }
    else
    {
        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
        data << uint8(error_code);
        if (error_code == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N)
        {
            uint32 reqlevel = 0;
            if (srcItem)
                if (srcItem->GetProto()->RequiredLevel > getLevel()) reqlevel = srcItem->GetProto()->RequiredLevel;
            if ((dstItem) && (!reqlevel))
                if (dstItem->GetProto()->RequiredLevel > getLevel()) reqlevel = dstItem->GetProto()->RequiredLevel;
            data << reqlevel;
        }
        data << uint64((srcItem ? srcItem->GetGUID(): 0));
        data << uint64((dstItem ? dstItem->GetGUID(): 0));
        data << uint8(0);
        m_session->SendPacket(&data);
        return false;
    }
}

// This function creates the item and puts it in the bag
// Avoid direct calls to this function, use AddNewItem instead
Item* Player::CreateNewItem (uint32 itemId, uint8 count)
{
    ItemPrototype *proto = objmgr.GetItemPrototype(itemId);
    if(!proto)
    {
        sLog.outError("CreateNewItem: Unknown itemId, itemId = %i", itemId);
        return NULL;
    }
    Item *pItem = NewItemOrBag(proto);
    if (count > proto->Stackable) { count = proto->Stackable; }
    if (count < 1) { count = 1; }
    if(!pItem->Create (objmgr.GenerateLowGuid (HIGHGUID_ITEM), itemId, this))
        return NULL;
    pItem->SetCount (count);
    return pItem;
}

// Returns the amount of items that player has (include bank or not)
uint16 Player::GetItemCount(uint32 itemId, bool includebank)
{
    uint16 countitems = 0;
    Item* pItem = 0;
    Bag* pBag = 0;

    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if (pItem)
        {
            if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
        }
    }
    for (uint8 bagIndex = CLIENT_SLOT_01; bagIndex <= CLIENT_SLOT_04; bagIndex++)
    {
        pBag = GetBagBySlot(bagIndex);
        if (pBag)
        {
            for (uint8 slot=0; slot < pBag->GetProto()->ContainerSlots; slot++)
            {
                pItem = pBag->GetItemFromBag(slot);
                if (pItem)
                {
                    if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
                }
            }
        }
    }
    if(!includebank)
        return countitems;

    for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if (pItem)
        {
            if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
        }
    }
    for (uint8 bagIndex = BANK_SLOT_BAG_START; bagIndex <= BANK_SLOT_BAG_END; bagIndex++)
    {
        pBag = GetBagBySlot(bagIndex);
        if (pBag)
        {
            for (uint8 slot=0; slot<pBag->GetProto()->ContainerSlots; slot++)
            {
                pItem = pBag->GetItemFromBag(slot);
                if (pItem)
                {
                    if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
                }
            }
        }
    }
    return countitems;
}

uint16 Player::GetItemCountAll(uint32 itemId, bool includeEquipment,bool includebank)
{
    uint16 countitems = 0;
    Item* pItem = 0;
    Bag* pBag = 0;

    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if (pItem)
        {
            if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
        }
    }
    for (uint8 bagIndex = CLIENT_SLOT_01; bagIndex <= CLIENT_SLOT_04; bagIndex++)
    {
        pBag = GetBagBySlot(bagIndex);
        if (pBag)
        {
            for (uint8 slot=0; slot < pBag->GetProto()->ContainerSlots; slot++)
            {
                pItem = pBag->GetItemFromBag(slot);
                if (pItem)
                {
                    if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
                }
            }
        }
    }
    if(!includeEquipment)
        return countitems;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        pItem = GetItemBySlot(i);
        if (pItem)
        {
            if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
        }
    }

    if(!includebank)
        return countitems;

    for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if (pItem)
        {
            if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
        }
    }
    for (uint8 bagIndex = BANK_SLOT_BAG_START; bagIndex <= BANK_SLOT_BAG_END; bagIndex++)
    {
        pBag = GetBagBySlot(bagIndex);
        if (pBag)
        {
            for (uint8 slot=0; slot<pBag->GetProto()->ContainerSlots; slot++)
            {
                pItem = pBag->GetItemFromBag(slot);
                if (pItem)
                {
                    if (pItem->GetEntry() == itemId) { countitems += pItem->GetCount(); }
                }
            }
        }
    }
    return countitems;
}

//where =1: inventory; =2: bank; =3 all
uint32 Player::CanAddItemCount(uint32 itemid, uint32 where)
{
    return CanAddItemCount(CreateNewItem(itemid, 1), where);
}

//where =1: inventory; =2: bank; =3 all
uint32 Player::CanAddItemCount(Item* item, uint32 where)
{
    if(!item)
        return 0;
    uint8 i;
    uint32 stack = item->GetMaxStackCount();
    uint32 count = 0;
    Item *pItem;
    Bag *pBag;
    if(GetItemCount(item->GetEntry(),true) >=1 && item->GetProto()->MaxCount == 1)
        return 0;

    if(where & 1)
    {
        for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem)
                count += stack;
            else if (pItem->GetEntry() == item->GetEntry())
                count += stack - pItem->GetCount();
        }
        for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem || !pItem->IsBag())
                continue;
            pBag = (Bag*)pItem;
            if(pBag && pBag->IsBag())
            {
                for(uint8 j = 0; j < pBag->GetProto()->ContainerSlots; j++)
                {
                    pItem = GetItemBySlot(j);
                    if(!pItem)
                        count += stack;
                    else if (pItem->GetEntry() == item->GetEntry())
                        count += stack - pItem->GetCount();
                }
            }
        }
    }
    if(where & 2)
    {
        for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem)
                count += stack;
            else if (pItem->GetEntry() == item->GetEntry())
                count += stack - pItem->GetCount();
        }
        for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem || !pItem->IsBag())
                continue;
            pBag = (Bag*)pItem;
            if(pBag && pBag->IsBag())
            {
                for(uint8 j = 0; j < pBag->GetProto()->ContainerSlots; j++)
                {
                    pItem = GetItemBySlot(j);
                    if(!pItem)
                        count += stack;
                    else if (pItem->GetEntry() == item->GetEntry())
                        count += stack - pItem->GetCount();
                }
            }
        }
    }
    return count;
}

// Adds a new item to player inventory
// - if addmaxpossible = false, items will be added just if user has enough space to put all the amount (count)
// search for other slots and count will be limited to max stack
// - Return value is the amount of items created
uint8 Player::AddNewItem(uint32 itemId, uint32 count, bool addmaxpossible)
{
    if (!itemId)
    {
        sLog.outError("AddNewItem: No itemId provided");
        return 0;
    }
    Item *pItem = CreateNewItem(itemId, count);
    if(pItem)
        return AddItemToInventory(pItem, addmaxpossible);
    return 0;
}

// Use AddItemToInventory or AddItemToBank or AddNewItem. if slot < EQUIPMENT_SLOT_END use this function.
// Adds an existing item (pointed by *item) to player inventory
// - bagIndex and slot can not be NULL (YOU MUST USE NULL_SLOT FOR SLOTS)
// - If allowstack is true, the function will try to stack items, otherwise it will just add if
// the slot is free
// - Notice that if a slot is specified and this slot is free, the function will not search for stacks
// - Return values: 0 - item not added
//                  1 - item added to a free slot (and perhaps to a stack)
//                  2 - item added to a stack (item should be deleted)
uint8 Player::AddItem(uint8 bagIndex,uint8 slot, Item *item, bool allowstack)
{
    uint32 additemcount = 0;
    if (!item)
    {
        sLog.outError("AddItem: No item provided");
        return 0;
    }
    if (!item->GetProto())
    {
        sLog.outError("AddItem: Unknown item, itemId = %i",item->GetEntry());
        return 0;
    }

    UpdateData upd;
    WorldPacket packet;
    Item *pItem = 0;
    Bag *pBag = 0;
    uint32 stack = item->GetMaxStackCount();
    uint32 count = item->GetCount();

    if(stack > 1 && allowstack )
    {
        switch(bagIndex)
        {
            case 0:
            case CLIENT_SLOT_BACK:
                if (slot >= BANK_SLOT_BAG_END)
                {
                    sLog.outError("AddItem: Invalid slot, slot = %i", slot);
                    return 0;
                }
                else if ((((slot >= INVENTORY_SLOT_BAG_START) && (slot < INVENTORY_SLOT_BAG_END)) || ((slot >= BANK_SLOT_BAG_START) && (slot < BANK_SLOT_BAG_END))) && (item->GetProto()->InventoryType != INVTYPE_BAG))
                {
                    sLog.outError("AddItem: Non-bag item in bag slot, itemId = %i, slot = %i", item->GetEntry(), slot);
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
                if (pBag)
                {
                    if ((slot >= pBag->GetProto()->ContainerSlots))
                    {
                        sLog.outError("AddItem: Invalid slot, bagIndex = %i, slot = %i", bagIndex, slot);
                        return 0;
                    }
                    pItem = pBag->GetItemFromBag(slot);
                }
                else
                {
                    sLog.outError("AddItem: No bag in that bagIndex, bagIndex = %i", bagIndex);
                    return 0;
                }
                break;
            default:
                sLog.outError("AddItem: Unknown bagIndex, bagIndex = %i", bagIndex);
                return 0;
        }

        if (pItem)
        {
            if (pItem->GetEntry() != item->GetEntry())
            {
                sLog.outError("AddItem: Player slot already has another item" );
                return 0;
            }
            else
            {
                additemcount = ((pItem->GetCount() + count) <= stack) ? count: (stack - pItem->GetCount());
                if ( additemcount > 0 )
                {
                    pItem->SetCount(pItem->GetCount() + additemcount);
                    if( pItem->GetProto()->Class == ITEM_CLASS_QUEST )
                        ItemAdded(pItem->GetEntry(), additemcount);
                    pItem->SendUpdateToPlayer(this);
                    sLog.outDetail("AddItem: Item %i added to bag %i - slot %i (stacked)",  pItem->GetEntry(), bagIndex, slot);
                    return 2;
                }
                else
                {
                    sLog.outError("AddItem: Player slot is full" );
                    return 0;
                }
            }
        }
    }
    item->SetOwner(this);
    //item->SetCount(count);
    if (((bagIndex >= INVENTORY_SLOT_BAG_START) && (bagIndex < INVENTORY_SLOT_BAG_END)) || ((bagIndex >= BANK_SLOT_BAG_START) && (bagIndex < BANK_SLOT_BAG_END)))
    {
        pBag = GetBagBySlot(bagIndex);
        if(!pBag)
        {
            sLog.outError("AddItem: Non-bag item in bag slot, itemId = %i, slot = %i", item->GetEntry(), slot);
            return 0;
        }
        pBag->AddItemToBag(slot, item);
        if (IsInWorld())
        {
            item->AddToWorld();
            upd.Clear();
            pBag->BuildCreateUpdateBlockForPlayer(&upd, this);
            upd.BuildPacket(&packet);
            GetSession()->SendPacket(&packet);
        }
        sLog.outDetail("AddItem: Item %i added to bag, bagIndex = %i, slot = %i", item->GetEntry(), bagIndex, slot);
    }
    else
    {
        item->SetSlot( slot );
        SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), item->GetGUID());
        item->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
        if (slot < EQUIPMENT_SLOT_END)
        {
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
            for(int enchant_solt =  0 ; enchant_solt < 21; enchant_solt+=3)
            {
                uint32 Enchant_id = item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+enchant_solt);
                if(Enchant_id)
                    AddItemEnchant(Enchant_id);
            }
        }
        if (IsInWorld())
        {
            item->AddToWorld();
            item->SendUpdateToPlayer(this);
            sLog.outDetail("AddItem: Item %i added to slot, slot = %i", item->GetEntry(), slot);
        }
        m_items[slot] = item;
    }
    if( item->GetProto()->Class == ITEM_CLASS_QUEST )
        ItemAdded(item->GetEntry(), count);
    return 1;
}

//Adds an existing item to inventory
// - Return values: 0 - item not added
//                  1 - item added to a free slot (and perhaps to a stack)
//                  2 - item added to a stack (item should be deleted)
//                  3 - item added some.
uint8 Player::AddItemToInventory(Item *item, bool addmaxpossible)
{
    if (!item)
    {
        sLog.outError("AddItemToInventory: No item provided");
        return 0;
    }
    ItemPrototype *proto = item->GetProto();
    if (!proto)
    {
        sLog.outError("AddItemToInventory: Unknown item, itemId = %i",item->GetEntry());
        return 0;
    }
    uint32 count = item->GetCount();
    if(CanAddItemCount(item, 1) < count && proto->MaxCount ==1)
    {
        sLog.outError("AddItemToInventory: Can't add, item is unique.");
        return 0;
    }
    if(CanAddItemCount(item, 1) < count && !addmaxpossible)
    {
        sLog.outError("AddItemToInventory: Can't add, Bag is full.");
        return 0;
    }

    Item *pItem = 0;
    Bag *pBag = 0;
    uint32 stack = item->GetMaxStackCount();
    uint8 i;
    if( stack > 1 )
    {
        for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(pItem && pItem->GetProto()->ItemId == item->GetProto()->ItemId)
            {
                if(pItem->GetCount() + item->GetCount() <= stack)
                    return AddItem(0, i, item, true);
                else
                {
                    item->SetCount(stack - pItem->GetCount());
                    count = count - item->GetCount();
                    AddItem(0, i, item, true);
                    item->SetCount(count);
                }
            }
        }
        for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem || !pItem->IsBag())
                continue;
            pBag = (Bag*)pItem;
            for (uint8 j=0; j < pBag->GetProto()->ContainerSlots; j++)
            {
                pItem = pBag->GetItemFromBag(j);
                if(pItem && pItem->GetProto()->ItemId == item->GetProto()->ItemId)
                {
                    if(pItem->GetCount() + item->GetCount() <= stack)
                        return AddItem(i, j, item, true);
                    else
                    {
                        item->SetCount(stack - pItem->GetCount());
                        count = count - item->GetCount();
                        AddItem(i, j, item, true);
                        item->SetCount(count);
                    }
                }
            }
        }
    }
    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if(!pItem)
        {
            if(item->GetCount() <= stack)
                return AddItem(0, i, item, true);
            else
            {
                item->SetCount(stack);
                count = count - item->GetCount();
                AddItem(0, i, item, true);
                item->SetCount(count);
            }
        }
    }
    for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pItem = GetItemBySlot(i);
        if(!pItem || !pItem->IsBag())
            continue;
        pBag = (Bag*)pItem;
        for (uint8 j=0; j < pBag->GetProto()->ContainerSlots; j++)
        {
            pItem = pBag->GetItemFromBag(j);
            if(!pItem)
            {
                if(item->GetCount() <= stack)
                    return AddItem(i, j, item, true);
                else
                {
                    item->SetCount(stack);
                    count = count - item->GetCount();
                    AddItem(i, j, item, true);
                    item->SetCount(count);
                }
            }
        }
    }
    return 3;
}

//Adds an existing item to bank
//Same options as AddItemToInventory
uint8 Player::AddItemToBank(Item *item, bool addmaxpossible)
{
    if (!item)
    {
        sLog.outError("AddItemToBank: No item provided");
        return 0;
    }
    ItemPrototype *proto = item->GetProto();
    if (!proto)
    {
        sLog.outError("AddItemToBank: Unknown item, itemId = %i",item->GetEntry());
        return 0;
    }
    uint32 count = item->GetCount();
    if(CanAddItemCount(item, 1) < count && proto->MaxCount ==1)
    {
        sLog.outError("AddItemToInventory: Can't add, item is unique.");
        return 0;
    }
    if(CanAddItemCount(item, 2) < count && !addmaxpossible)
    {
        sLog.outError("AddItemToBank: Can't add, Bank is full.");
        return 0;
    }

    Item *pItem = 0;
    Bag *pBag = 0;
    uint32 stack = item->GetMaxStackCount();
    uint8 i;

    if( stack > 1 )
    {
        for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(pItem && pItem->GetProto()->ItemId == item->GetProto()->ItemId)
            {
                if(pItem->GetCount() + item->GetCount() <= stack)
                    return AddItem(0, i, item, true);
                else
                {
                    item->SetCount(stack - pItem->GetCount());
                    count = count - item->GetCount();
                    AddItem(0, i, item, true);
                    item->SetCount(count);
                }
            }
        }
        for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem || !pItem->IsBag())
                continue;
            pBag = (Bag*)pItem;
            for (uint8 j=0; j < pBag->GetProto()->ContainerSlots; j++)
            {
                pItem = pBag->GetItemFromBag(j);
                if(pItem && pItem->GetProto()->ItemId == item->GetProto()->ItemId)
                {
                    if(pItem->GetCount() + item->GetCount() <= stack)
                        return AddItem(i, j, item, true);
                    else
                    {
                        item->SetCount(stack - pItem->GetCount());
                        count = count - item->GetCount();
                        AddItem(i, j, item, true);
                        item->SetCount(count);
                    }
                }
            }
        }
    }
    for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        pItem = GetItemBySlot(i);
        if(!pItem)
        {
            if(item->GetCount() <= stack)
                return AddItem(0, i, item, true);
            else
            {
                item->SetCount(stack);
                count = count - item->GetCount();
                AddItem(0, i, item, true);
                item->SetCount(count);
            }
        }
    }
    for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        pItem = GetItemBySlot(i);
        if(!pItem || !pItem->IsBag())
            continue;
        pBag = (Bag*)pItem;
        for (uint8 j=0; j < pBag->GetProto()->ContainerSlots; j++)
        {
            pItem = pBag->GetItemFromBag(j);
            if(!pItem)
            {
                if(item->GetCount() <= stack)
                    return AddItem(i, j, item, true);
                else
                {
                    item->SetCount(stack);
                    count = count - item->GetCount();
                    AddItem(i, j, item, true);
                    item->SetCount(count);
                }
            }
        }
    }
    return 3;
}

void Player::RemoveItemFromInventory(uint32 itemId,uint32 itemcount)
{
    if(itemId==0)
        return;
    UpdateData upd;
    WorldPacket packet;
    uint32 removed=0,oldcnt=0;
    Bag* pBag;
    Item* pItem;
    bool client_remove=true;
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = m_items[i];
        if (!pItem)
            continue;
        if(pItem->GetProto()->ItemId==itemId)
        {
            if((oldcnt=pItem->GetCount())>itemcount)
            {
                m_items[i]->SetCount(oldcnt-itemcount);
                removed+=itemcount;
                sLog.outDetail("RemoveItemFromSlot: Item removed,slot = %i, count = %u", i, removed);
                return;
            }
            else
            {
                removed+=oldcnt;
                m_items[i] = NULL;
                SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (i*2)), 0);
                _ApplyItemMods(pItem, i, false);
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (i * 12);
                for (int k = VisibleBase; k < VisibleBase + 12; ++k)
                    SetUInt32Value(k, 0);

                if (client_remove)
                {
                    pItem->SetOwner(0);
                    if (IsInWorld())
                    {
                        pItem->RemoveFromWorld();
                        pItem->DestroyForPlayer(this);
                        ItemRemoved(pItem->GetEntry(), itemcount);
                    }
                }
            }
            if(removed>=itemcount)
            {
                sLog.outDetail("RemoveItemFromSlot: Item removed,slot = %i, count = %u", i, removed);
                return;
            }
        }
    }
    for(uint8 i=INVENTORY_SLOT_BAG_START;i<INVENTORY_SLOT_BAG_END;i++)
    {
        pBag = GetBagBySlot(i);
        if (pBag)
        {
            uint32 ContainerSlots=pBag->GetProto()->ContainerSlots;
            for( uint8 j=0; j<ContainerSlots; j++ )
            {
                pItem = pBag->GetItemFromBag(j);
                if ( pItem && pItem->GetProto()->ItemId == itemId )
                {
                    removed += pBag->RemoveItem( j, itemcount );
                    /*if (client_remove)
                    {
                        pItem->SetOwner(0);
                        if (IsInWorld())
                        {
                            pItem->RemoveFromWorld();
                            pItem->DestroyForPlayer(this);
                            ItemRemoved(pItem->GetEntry());
                        }
                    }*/
                    pBag->SendUpdateToPlayer(this);

                    if(removed>=itemcount)
                    {
                        sLog.outDetail("RemoveItemFromSlot: Item removed,slot = %i, count = %u", i, removed);
                        return;
                    }
                }
            }
        }
    }
}

// Removes an Item from a bag and/or slot
// bagIndex can be NULL
// Return value is a pointer to deleted item, NULL if no item deleted
Item* Player::RemoveItemFromSlot(uint8 bagIndex, uint8 slot, bool client_remove)
{
    UpdateData upd;
    WorldPacket packet;
    Item *pretItem = 0;
    Bag *pBag = 0;
    Item *pItem = 0;

    switch (bagIndex)
    {
        case 0:
        case CLIENT_SLOT_BACK:
            if (slot >= BANK_SLOT_BAG_END)
            {
                sLog.outError("RemoveItemFromSlot: Invalid slot, slot = %i", slot);
                return 0;
            }
            pItem = m_items[slot];
            if (!pItem)
            {
                sLog.outError("RemoveItemFromSlot: No item found in that slot, slot = %i", slot);
                return 0;
            }
            pretItem = pItem;
            m_items[slot] = NULL;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);
            if (slot < EQUIPMENT_SLOT_END)
            {
                _ApplyItemMods(pItem, slot, false);
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
                for (int i = VisibleBase; i < VisibleBase + 12; ++i)
                {
                    SetUInt32Value(i, 0);
                }
                for(int enchant_solt =  0 ; enchant_solt < 21; enchant_solt+=3)
                {
                    uint32 Enchant_id = pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+enchant_solt);
                    if( Enchant_id)
                    {
                        SpellItemEnchantment *pEnchant;
                        pEnchant = sSpellItemEnchantmentStore.LookupEntry(Enchant_id);
                        if(!pEnchant)
                            continue;
                        uint32 enchant_display = pEnchant->display_type;
                        uint32 enchant_value1 = pEnchant->value1;
                        uint32 enchant_value2 = pEnchant->value2;
                        uint32 enchant_spell_id = pEnchant->spellid;
                        uint32 enchant_aura_id = pEnchant->aura_id;
                        uint32 enchant_description = pEnchant->description;
                        //SpellEntry *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);
                        if(enchant_display ==4)
                            SetUInt32Value(UNIT_FIELD_ARMOR,GetUInt32Value(UNIT_FIELD_ARMOR)-enchant_value1);
                        else if(enchant_display ==2)
                        {
                            SetUInt32Value(UNIT_FIELD_MINDAMAGE,GetUInt32Value(UNIT_FIELD_MINDAMAGE)-enchant_value1);
                            SetUInt32Value(UNIT_FIELD_MAXDAMAGE,GetUInt32Value(UNIT_FIELD_MAXDAMAGE)-enchant_value1);
                        }
                        else
                        {
                            RemoveAura(enchant_spell_id);
                        }
                    }
                }
            }
            if (client_remove)
            {
                pItem->SetOwner(0);
                if (IsInWorld())
                {
                    pItem->RemoveFromWorld();
                    pItem->DestroyForPlayer(this);
                }
            }
            sLog.outDetail("RemoveItemFromSlot: Item removed, slot = %i", slot);
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
            if (pBag)
            {
                pItem = pBag->GetItemFromBag(slot);
                if (pItem)
                {
                    pretItem = pBag->RemoveItem(slot);
                    if (client_remove)
                    {
                        pItem->SetOwner(0);
                        if (IsInWorld())
                        {
                            pItem->RemoveFromWorld();
                            pItem->DestroyForPlayer(this);
                        }
                    }

                    pBag->SendUpdateToPlayer(this);
                    sLog.outDetail("RemoveItemFromSlot: Item removed, bagIndex = %i, slot = %i", bagIndex, slot);
                    break;
                }
                else
                {
                    sLog.outError("RemoveItemFromSlot: No item found, bagIndex = %i, slot = %i", bagIndex, slot);
                    return 0;
                }
            }
            else
            {
                sLog.outError("RemoveItemFromSlot: No bag in that bagIndex, bagIndex = %i", bagIndex);
                return 0;
            }
            break;
        default:
            sLog.outError("RemoveItemFromSlot: Unknow bagIndex, bagIndex = %i", bagIndex);
            return 0;
    }
    return pretItem;
}

Item* Player::GetItemBySlot(uint8 bagIndex,uint8 slot) const
{
    Bag *pBag = 0;

    switch (bagIndex)
    {
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
            if (pBag)
            {
                return pBag->GetItemFromBag(slot);
            }
            break;
        default:
            sLog.outDetail("GetItemBySlot: unknow bagIndex, bagIndex = %i\n", bagIndex);
            break;
    }
    return 0;
}

uint32 Player::GetSlotByItemID(uint32 ID)
{
    for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
    {
        if(m_items[i] != 0)
            if(m_items[i]->GetProto()->ItemId == ID)
                return i;
    }
    return 0;
}

Item* Player::GetItemByItemType(uint32 type)
{
    for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
    {
        if(m_items[i] != 0)
            if(m_items[i]->GetProto()->InventoryType == type)
                return m_items[i];
    }
    return NULL;
}

bool Player::GetSlotByItemID(uint32 ID,uint8 &bagIndex,uint8 &slot,bool CheckInventorySlot,bool additems)
{
    Bag *pBag;
    Item *pItem;
    if(CheckInventorySlot)
    {
        for(uint8 j=0;j<INVENTORY_SLOT_ITEM_END;j++)
        {
            pItem = GetItemBySlot(j);
            if(!pItem)
                continue;
            if(additems)
            {
                if(pItem->GetProto()->ItemId == ID
                    && pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) < pItem->GetProto()->MaxCount)
                    slot = j;
            }
            else if(pItem->GetProto()->ItemId == ID)
                slot = j;
        }
        if(slot)
        {
            if(slot >=23 && slot <=39)
            {
                bagIndex = CLIENT_SLOT_BACK;
                return true;
            }
            if(slot >0 && slot <19)
            {
                bagIndex = 0;
                return true;
            }
        }
    }
    else if(additems)
    {
        for(uint32 i=INVENTORY_SLOT_ITEM_START;i<INVENTORY_SLOT_ITEM_END;i++)
        {
            pItem = GetItemBySlot(i);
            if(!pItem)
                continue;
            if(pItem->GetProto()->ItemId == ID
                && pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) < pItem->GetProto()->MaxCount)
            {
                bagIndex = CLIENT_SLOT_BACK;
                slot = i;
                return true;
            }
        }
    }
    else
    {
        slot=GetSlotByItemID(ID);
        if(slot)
        {
            bagIndex = 0;
            return true;
        }
    }
    for(uint8 i=CLIENT_SLOT_01;i<=CLIENT_SLOT_04;i++)
    {
        pBag = GetBagBySlot(i);
        if (pBag)
            for(uint8 pSlot=0; pSlot < pBag->GetProto()->ContainerSlots; pSlot++)
        {
            pItem = pBag->GetItemFromBag(pSlot);
            if(pItem)
            {
                if(additems)
                {
                    if(pItem->GetProto()->ItemId == ID
                        && pItem->GetUInt32Value(ITEM_FIELD_STACK_COUNT) < pItem->GetProto()->MaxCount)
                    {
                        slot = pSlot;
                        bagIndex = i;
                        pBag = NULL;
                        pItem = NULL;
                        return true;
                    }
                }
                else if(pItem->GetProto()->ItemId == ID)
                {
                    slot = pSlot;
                    bagIndex = i;
                    pBag = NULL;
                    pItem = NULL;
                    return true;
                }

            }
        }
    }
    pBag = NULL;
    pItem = NULL;
    return false;
}

uint32 Player::GetSlotByItemGUID(uint64 guid)
{
    for(uint32 i=0;i<INVENTORY_SLOT_ITEM_END;i++)
    {
        if(m_items[i] != 0)
            if(m_items[i]->GetGUID() == guid)
                return i;
    }
    return 0;
}

bool Player::GetSlotByItemGUID(uint64 guid,uint8 &bagIndex,uint8 &slot)
{
    slot=GetSlotByItemGUID(guid);
    if (slot)
    {
        bagIndex = CLIENT_SLOT_BACK;
        return true;
    }

    for (uint8 i=CLIENT_SLOT_01;i<=CLIENT_SLOT_04;i++)
    {
        if (Bag *pBag = GetBagBySlot(i))
        {
            uint8 s=pBag->GetSlotByItemGUID(guid);
            if (s != NULL_SLOT)
            {
                slot = s;
                bagIndex = i;
                return true;
            }
        }
    }
    return false;
}

void Player::AddItemToBuyBackSlot(uint32 slot,Item *item)
{
    if (item && item->GetProto())
        sLog.outError("AddItemtoBuyBackSlot Item: \"%s\" [%u] Slot: [%u]", item->GetProto()->Name1, item->GetProto()->ItemId, slot);
    else
    {
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

Item* Player::GetItemFromBuyBackSlot(uint32 slot)
{
    if (slot >= BUYBACK_SLOT_END)
        return NULL;

    return m_buybackitems[slot];
}

Item* Player::RemoveItemFromBuyBackSlot(uint32 slot)
{
    if (slot >= BUYBACK_SLOT_END)
        return NULL;

    Item *pItem = m_buybackitems[slot];
    if(m_buybackitems[slot])
    {
        //m_buybackitems[slot]->DeleteFromDB();
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

    if(slot >= INVENTORY_SLOT_BAG_END || !item) return;

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
    std::string typestr;
    std::string applystr = "Add";
    if(!apply)
        applystr = "Remove";
    for (int i = 0; i < 10; i++)
    {
        val = proto->ItemStat[i].ItemStatValue ;

        switch (proto->ItemStat[i].ItemStatType)
        {
            case POWER:                                     // modify MP
                SetUInt32Value(UNIT_FIELD_MAXPOWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1)+(apply? val:-val));
                typestr = "Mana";
                break;
            case HEALTH:                                    // modify HP
                SetUInt32Value(UNIT_FIELD_MAXHEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH)+(apply? val:-val));
                typestr = "Health";
                break;
            case AGILITY:                                   // modify agility
                SetUInt32Value(UNIT_FIELD_AGILITY,GetUInt32Value(UNIT_FIELD_AGILITY)+(apply? val:-val));
                SetUInt32Value(PLAYER_FIELD_POSSTAT1,GetUInt32Value(PLAYER_FIELD_POSSTAT1)+(apply? val:-val));
                typestr = "AGILITY";
                break;
            case STRENGHT:                                  //modify strength
                SetUInt32Value(UNIT_FIELD_STR,GetUInt32Value(UNIT_FIELD_STR)+(apply? val:-val));
                SetUInt32Value(PLAYER_FIELD_POSSTAT0,GetUInt32Value(PLAYER_FIELD_POSSTAT0)+(apply? val:-val));
                typestr = "STRENGHT";
                break;
            case INTELLECT:                                 //modify intellect
                SetUInt32Value(UNIT_FIELD_IQ,GetUInt32Value(UNIT_FIELD_IQ)+(apply? val:-val));
                SetUInt32Value(PLAYER_FIELD_POSSTAT3,GetUInt32Value(PLAYER_FIELD_POSSTAT3)+(apply? val:-val));
                SetUInt32Value(UNIT_FIELD_MAXPOWER1, GetUInt32Value(UNIT_FIELD_MAXPOWER1)+(apply? val:-val)*15);
                typestr = "INTELLECT";
                break;
            case SPIRIT:                                    //modify spirit
                SetUInt32Value(UNIT_FIELD_SPIRIT,GetUInt32Value(UNIT_FIELD_SPIRIT)+(apply? val:-val));
                SetUInt32Value(PLAYER_FIELD_POSSTAT4,GetUInt32Value(PLAYER_FIELD_POSSTAT4)+(apply? val:-val));
                typestr = "SPIRIT";
                break;
            case STAMINA:                                   //modify stamina
                SetUInt32Value(UNIT_FIELD_STAMINA,GetUInt32Value(UNIT_FIELD_STAMINA)+(apply? val:-val));
                SetUInt32Value(PLAYER_FIELD_POSSTAT2,GetUInt32Value(PLAYER_FIELD_POSSTAT2)+(apply? val:-val));
                SetUInt32Value(UNIT_FIELD_MAXHEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH)+(apply? val:-val)*10);
                typestr = "STAMINA";
                break;
        }
        if(val > 0)
            sLog.outDebug("%s %s: \t\t%u", applystr.c_str(), typestr.c_str(), val);

    }

    if (proto->Armor)
    {
        SetUInt32Value(UNIT_FIELD_ARMOR, GetUInt32Value(UNIT_FIELD_ARMOR) + (apply ? proto->Armor: -(int32)proto->Armor));
        sLog.outDebug("%s Armor: \t\t%u", applystr.c_str(),  proto->Armor);
    }

    if (proto->Block)
    {
        SetFloatValue(PLAYER_BLOCK_PERCENTAGE, GetFloatValue(PLAYER_BLOCK_PERCENTAGE) + (apply ? proto->Block: -(float)proto->Block));
        sLog.outDebug("%s Block: \t\t%u", applystr.c_str(),  proto->Block);
    }

    if (proto->HolyRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_01, GetUInt32Value(UNIT_FIELD_RESISTANCES_01) + (apply ? proto->HolyRes: -(int32)proto->HolyRes));
        sLog.outDebug("%s HolyRes: \t\t%u", applystr.c_str(),  proto->HolyRes);
    }

    if (proto->FireRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_02, GetUInt32Value(UNIT_FIELD_RESISTANCES_02) + (apply ? proto->FireRes: -(int32)proto->FireRes));
        sLog.outDebug("%s FireRes: \t\t%u", applystr.c_str(),  proto->FireRes);
    }

    if (proto->NatureRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_03, GetUInt32Value(UNIT_FIELD_RESISTANCES_03) + (apply ? proto->NatureRes: -(int32)proto->NatureRes));
        sLog.outDebug("%s NatureRes: \t\t%u", applystr.c_str(),  proto->NatureRes);
    }

    if (proto->FrostRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_04, GetUInt32Value(UNIT_FIELD_RESISTANCES_04) + (apply ? proto->FrostRes: -(int32)proto->FrostRes));
        sLog.outDebug("%s FrostRes: \t\t%u", applystr.c_str(),  proto->FrostRes);
    }

    if (proto->ShadowRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_05, GetUInt32Value(UNIT_FIELD_RESISTANCES_05) + (apply ? proto->ShadowRes: -(int32)proto->ShadowRes));
        sLog.outDebug("%s ShadowRes: \t\t%u", applystr.c_str(),  proto->ShadowRes);
    }

    if (proto->ArcaneRes)
    {
        SetUInt32Value(UNIT_FIELD_RESISTANCES_06, GetUInt32Value(UNIT_FIELD_RESISTANCES_06) + (apply ? proto->ArcaneRes: -(int32)proto->ArcaneRes));
        sLog.outDebug("%s ArcaneRes: \t\t%u", applystr.c_str(),  proto->ArcaneRes);
    }

    uint8 MINDAMAGEFIELD = 0;
    uint8 MAXDAMAGEFIELD = 0;

    if( slot == EQUIPMENT_SLOT_RANGED && ( proto->InventoryType == INVTYPE_RANGED ||
        proto->InventoryType == INVTYPE_THROWN || proto->InventoryType == INVTYPE_RANGEDRIGHT))
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINRANGEDDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXRANGEDDAMAGE;
        typestr = "Ranged";
    }
    else if(slot==EQUIPMENT_SLOT_MAINHAND)
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXDAMAGE;
        typestr = "Mainhand";
    }
    else if(slot==EQUIPMENT_SLOT_OFFHAND)
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINOFFHANDDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXOFFHANDDAMAGE;
        typestr = "Offhand";
    }

    if (proto->Damage[0].DamageMin > 0 && MINDAMAGEFIELD)
    {
        SetFloatValue(MINDAMAGEFIELD, GetFloatValue(MINDAMAGEFIELD) + (apply ? proto->Damage[0].DamageMin: -proto->Damage[0].DamageMin));
        sLog.outString("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMin, GetFloatValue(MINDAMAGEFIELD));
    }

    if (proto->Damage[0].DamageMax  > 0 && MAXDAMAGEFIELD)
    {
        SetFloatValue(MAXDAMAGEFIELD, GetFloatValue(MAXDAMAGEFIELD) + (apply ? proto->Damage[0].DamageMax: -proto->Damage[0].DamageMax));
        sLog.outString("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMax, GetFloatValue(MAXDAMAGEFIELD));
    }

    if (proto->Delay)
    {
        if(slot == EQUIPMENT_SLOT_RANGED)
        {
            SetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1, apply ? proto->Delay: 2000);
            typestr = "Range";
            sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
        else if(slot==EQUIPMENT_SLOT_MAINHAND || slot==EQUIPMENT_SLOT_OFFHAND)
        {
            SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, apply ? proto->Delay: 2000);
            typestr = "Mainhand";
            sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
    }

    if(apply)
        CastItemSpell(item,(Unit*)this);
    else
        for (int i = 0; i < 5; i++)
            if(proto->Spells[i].SpellId)
                RemoveAura(proto->Spells[i].SpellId );
    sLog.outDebug("_ApplyItemMods complete.");
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
        if(!spellInfo)
        {
            DEBUG_LOG("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId);
            continue;
        }

        if(Target->GetGUID() == GetGUID() && !IsItemSpellToEquip(spellInfo)) continue;
        else if(Target->GetGUID() != GetGUID() && IsItemSpellToEquip(spellInfo)) continue;

        DEBUG_LOG("WORLD: cast Item spellId - %i", proto->Spells[i].SpellId);

        spell = new Spell(this, spellInfo, false, 0);
        WPAssert(spell);

        SpellCastTargets targets;
        targets.setUnitTarget( Target );
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

void Player::_RemoveAllItemMods()
{
    sLog.outDebug("_RemoveAllItemMods start.");
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            _ApplyItemMods(m_items[i],i, false);
    }
    sLog.outDebug("_RemoveAllItemMods complete.");
}

void Player::_ApplyAllItemMods()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            _ApplyItemMods(m_items[i],i, true);
    }
}

/*Loot type MUST be
1-corpse, go
2-skinning
3-Fishing
*/

void Player::SendLoot(uint64 guid, uint8 loot_type)
{
    Player  *player = this;
    Loot    *loot;

    if (IS_GAMEOBJECT_GUID(guid))
    {
        GameObject *go =
            ObjectAccessor::Instance().GetGameObject(*player, guid);

        if (!go)
            return;

        go->generateLoot();
        if(loot_type == 3)
        {
            loot = &go->loot;
            uint32 zone = GetZoneId();
            uint32 lootid = 30000 + zone;
            //in some DB,30000 is't right.check your DB.if 30001 -32XXX is fish loot.
            go->getFishLoot(loot,lootid);
        }
        loot = &go->loot;
    }
    else
    {
        Creature *creature =
            ObjectAccessor::Instance().GetCreature(*player, guid);

        if (!creature)
            return;

        loot = &creature->loot;
        if (loot_type == 2)
        {
            creature->getSkinLoot();
            loot = &creature->loot;
        }
    }

    m_lootGuid = guid;

    WorldPacket data;
    data.Initialize (SMSG_LOOT_RESPONSE);

    data << guid;
    data << loot_type;                                      //loot_type;
    data << *loot;

    SendMessageToSet(&data, true);
}

void Player::SendUpdateWordState(uint16 Field, uint16 Value)
{
    WorldPacket data;
    data.Initialize(SMSG_UPDATE_WORLD_STATE);               //0x2D4
    data << uint32(Field);
    data << uint32(Value);
    GetSession()->SendPacket(&data);
}

void Player::SendInitWorldStates(uint32 MapID)
{
    // TODO Figure out the unknown data.

    if ((MapID == 0) || (MapID == 1))
    {
        sLog.outDebug("Sending SMSG_INIT_WORLD_STATES to Map:%u",MapID);

        uint16 NumberOfFields = 108;
        WorldPacket data;
        data.Initialize (SMSG_INIT_WORLD_STATES);           //0x2C5
        data <<
            (uint32)MapID <<
            (uint16)NumberOfFields <<
        //field (uint16)  value (uint16)
            (uint16)0x07AE<< (uint16)0x01<<
            (uint16)0x0532<< (uint16)0x01<<
            (uint16)0x0531<< (uint16)0x00<<
            (uint16)0x052E<< (uint16)0x00<<
            (uint16)0x06F9<< (uint16)0x00<<
            (uint16)0x06F3<< (uint16)0x00<<
            (uint16)0x06F1<< (uint16)0x00<<
            (uint16)0x06EE<< (uint16)0x00<<
            (uint16)0x06ED<< (uint16)0x00<<
            (uint16)0x0571<< (uint16)0x00<<
            (uint16)0x0570<< (uint16)0x00<<
            (uint16)0x0567<< (uint16)0x01<<
            (uint16)0x0566<< (uint16)0x01<<
            (uint16)0x0550<< (uint16)0x01<<
            (uint16)0x0544<< (uint16)0x00<<
            (uint16)0x0536<< (uint16)0x00<<
            (uint16)0x0535<< (uint16)0x01<<
            (uint16)0x03C6<< (uint16)0x00<<
            (uint16)0x03C4<< (uint16)0x00<<
            (uint16)0x03C2<< (uint16)0x00<<
            (uint16)0x07A8<< (uint16)0x00<<
            (uint16)0x07A3<< (uint16)0x270F<<
            (uint16)0x0574<< (uint16)0x00<<
            (uint16)0x0573<< (uint16)0x00<<
            (uint16)0x0572<< (uint16)0x00<<
            (uint16)0x056F<< (uint16)0x00<<
            (uint16)0x056E<< (uint16)0x00<<
            (uint16)0x056D<< (uint16)0x00<<
            (uint16)0x056C<< (uint16)0x00<<
            (uint16)0x056B<< (uint16)0x00<<
            (uint16)0x056A<< (uint16)0x01<<
            (uint16)0x0569<< (uint16)0x01<<
            (uint16)0x0568<< (uint16)0x01<<
            (uint16)0x0565<< (uint16)0x00<<
            (uint16)0x0564<< (uint16)0x00<<
            (uint16)0x0563<< (uint16)0x00<<
            (uint16)0x0562<< (uint16)0x00<<
            (uint16)0x0561<< (uint16)0x00<<
            (uint16)0x0560<< (uint16)0x00<<
            (uint16)0x055F<< (uint16)0x00<<
            (uint16)0x055E<< (uint16)0x00<<
            (uint16)0x055D<< (uint16)0x00<<
            (uint16)0x055C<< (uint16)0x00<<
            (uint16)0x055B<< (uint16)0x00<<
            (uint16)0x055A<< (uint16)0x00<<
            (uint16)0x0559<< (uint16)0x00<<
            (uint16)0x0558<< (uint16)0x00<<
            (uint16)0x0557<< (uint16)0x00<<
            (uint16)0x0556<< (uint16)0x00<<
            (uint16)0x0555<< (uint16)0x00<<
            (uint16)0x0554<< (uint16)0x01<<
            (uint16)0x0553<< (uint16)0x01<<
            (uint16)0x0552<< (uint16)0x01<<
            (uint16)0x0551<< (uint16)0x01<<
            (uint16)0x054F<< (uint16)0x00<<
            (uint16)0x054E<< (uint16)0x00<<
            (uint16)0x054D<< (uint16)0x01<<
            (uint16)0x054C<< (uint16)0x00<<
            (uint16)0x054B<< (uint16)0x00<<
            (uint16)0x0545<< (uint16)0x00<<
            (uint16)0x0543<< (uint16)0x01<<
            (uint16)0x0542<< (uint16)0x00<<
            (uint16)0x0540<< (uint16)0x00<<
            (uint16)0x053F<< (uint16)0x00<<
            (uint16)0x053E<< (uint16)0x00<<
            (uint16)0x053D<< (uint16)0x00<<
            (uint16)0x053C<< (uint16)0x00<<
            (uint16)0x053B<< (uint16)0x00<<
            (uint16)0x053A<< (uint16)0x01<<
            (uint16)0x0539<< (uint16)0x00<<
            (uint16)0x0538<< (uint16)0x00<<
            (uint16)0x0537<< (uint16)0x00<<
            (uint16)0x0534<< (uint16)0x00<<
            (uint16)0x0533<< (uint16)0x00<<
            (uint16)0x0530<< (uint16)0x00<<
            (uint16)0x052F<< (uint16)0x00<<
            (uint16)0x052D<< (uint16)0x01<<
            (uint16)0x0516<< (uint16)0x01<<
            (uint16)0x0515<< (uint16)0x00<<
            (uint16)0x03B6<< (uint16)0x00<<
            (uint16)0x0745<< (uint16)0x02<<
            (uint16)0x0736<< (uint16)0x01<<
            (uint16)0x0735<< (uint16)0x01<<
            (uint16)0x0734<< (uint16)0x01<<
            (uint16)0x0733<< (uint16)0x01<<
            (uint16)0x0732<< (uint16)0x01<<
            (uint16)0x0702<< (uint16)0x00<<
            (uint16)0x0701<< (uint16)0x00<<
            (uint16)0x0700<< (uint16)0x00<<
            (uint16)0x06FE<< (uint16)0x00<<
            (uint16)0x06FD<< (uint16)0x00<<
            (uint16)0x06FC<< (uint16)0x00<<
            (uint16)0x06FB<< (uint16)0x00<<
            (uint16)0x06F8<< (uint16)0x00<<
            (uint16)0x06F7<< (uint16)0x00<<
            (uint16)0x06F6<< (uint16)0x00<<
            (uint16)0x06F4<< (uint16)0x7D0<<
            (uint16)0x06F2<< (uint16)0x00<<
            (uint16)0x06F0<< (uint16)0x00<<
            (uint16)0x06EF<< (uint16)0x00<<
            (uint16)0x06EC<< (uint16)0x00<<
            (uint16)0x06EA<< (uint16)0x00<<
            (uint16)0x06E9<< (uint16)0x00<<
            (uint16)0x06E8<< (uint16)0x00<<
            (uint16)0x06E7<< (uint16)0x00<<
            (uint16)0x0518<< (uint16)0x00<<
            (uint16)0x0517<< (uint16)0x00<<
            (uint16)0x0703<< (uint16)0x00;
        GetSession()->SendPacket(&data);
    }

    //BattleGround currently only map 489
    else if (MapID == 489)                                  // && and guid is in a current Battlefield)
    {
        sLog.outDebug("Sending SMSG_INIT_WORLD_STATES to Map:%u",MapID);

        uint16 NumberOfFields = 114;
        WorldPacket data;
        data.Initialize (SMSG_INIT_WORLD_STATES);
        data <<

            (uint32)MapID<<
            (uint16)NumberOfFields <<
        //field (uint16)  value (uint16)
            (uint16)0x07AE<< (uint16)0x01<<
            (uint16)0x0532<< (uint16)0x01<<
            (uint16)0x0531<< (uint16)0x00<<
            (uint16)0x052E<< (uint16)0x00<<
            (uint16)0x06F9<< (uint16)0x00<<
            (uint16)0x06F3<< (uint16)0x00<<
            (uint16)0x06F1<< (uint16)0x00<<
            (uint16)0x06EE<< (uint16)0x00<<
            (uint16)0x06ED<< (uint16)0x00<<
            (uint16)0x0571<< (uint16)0x00<<
            (uint16)0x0570<< (uint16)0x00<<
            (uint16)0x0567<< (uint16)0x01<<
            (uint16)0x0566<< (uint16)0x01<<
            (uint16)0x0550<< (uint16)0x01<<
            (uint16)0x0544<< (uint16)0x00<<
            (uint16)0x0536<< (uint16)0x00<<
            (uint16)0x0535<< (uint16)0x01<<
            (uint16)0x03C6<< (uint16)0x00<<
            (uint16)0x03C4<< (uint16)0x00<<
            (uint16)0x03C2<< (uint16)0x00<<
            (uint16)0x07A8<< (uint16)0x00<<
            (uint16)0x07A3<< (uint16)0x270F <<
            (uint16)0x060B<< (uint16)0x02<<
            (uint16)0x0574<< (uint16)0x00<<
            (uint16)0x0573<< (uint16)0x00<<
            (uint16)0x0572<< (uint16)0x00<<
            (uint16)0x056F<< (uint16)0x00<<
            (uint16)0x056E<< (uint16)0x00<<
            (uint16)0x056D<< (uint16)0x00<<
            (uint16)0x056C<< (uint16)0x00<<
            (uint16)0x056B<< (uint16)0x00<<
            (uint16)0x056A<< (uint16)0x01<<
            (uint16)0x0569<< (uint16)0x01<<
            (uint16)0x0568<< (uint16)0x01<<
            (uint16)0x0565<< (uint16)0x00<<
            (uint16)0x0564<< (uint16)0x00<<
            (uint16)0x0563<< (uint16)0x00<<
            (uint16)0x0562<< (uint16)0x00<<
            (uint16)0x0561<< (uint16)0x00<<
            (uint16)0x0560<< (uint16)0x00<<
            (uint16)0x055F<< (uint16)0x00<<
            (uint16)0x055E<< (uint16)0x00<<
            (uint16)0x055D<< (uint16)0x00<<
            (uint16)0x055C<< (uint16)0x00<<
            (uint16)0x055B<< (uint16)0x00<<
            (uint16)0x055A<< (uint16)0x00<<
            (uint16)0x0559<< (uint16)0x00<<
            (uint16)0x0558<< (uint16)0x00<<
            (uint16)0x0557<< (uint16)0x00<<
            (uint16)0x0556<< (uint16)0x00<<
            (uint16)0x0555<< (uint16)0x00<<
            (uint16)0x0554<< (uint16)0x01<<
            (uint16)0x0553<< (uint16)0x01<<
            (uint16)0x0552<< (uint16)0x01<<
            (uint16)0x0551<< (uint16)0x01<<
            (uint16)0x054F<< (uint16)0x00<<
            (uint16)0x054E<< (uint16)0x00<<
            (uint16)0x054D<< (uint16)0x01<<
            (uint16)0x054C<< (uint16)0x00<<
            (uint16)0x054B<< (uint16)0x00<<
            (uint16)0x0545<< (uint16)0x00<<
            (uint16)0x0543<< (uint16)0x01<<
            (uint16)0x0542<< (uint16)0x00<<
            (uint16)0x0540<< (uint16)0x00<<
            (uint16)0x053F<< (uint16)0x00<<
            (uint16)0x053E<< (uint16)0x00<<
            (uint16)0x053D<< (uint16)0x00<<
            (uint16)0x053C<< (uint16)0x00<<
            (uint16)0x053B<< (uint16)0x00<<
            (uint16)0x053A<< (uint16)0x01<<
            (uint16)0x0539<< (uint16)0x00<<
            (uint16)0x0538<< (uint16)0x00<<
            (uint16)0x0537<< (uint16)0x00<<
            (uint16)0x0534<< (uint16)0x00<<
            (uint16)0x0533<< (uint16)0x00<<
            (uint16)0x0530<< (uint16)0x00<<
            (uint16)0x052F<< (uint16)0x00<<
            (uint16)0x052D<< (uint16)0x01<<
            (uint16)0x0516<< (uint16)0x01<<
            (uint16)0x0515<< (uint16)0x00<<
            (uint16)0x03B6<< (uint16)0x00<<
            (uint16)0x0745<< (uint16)0x02<<
            (uint16)0x0736<< (uint16)0x01<<
            (uint16)0x0735<< (uint16)0x01<<
            (uint16)0x0734<< (uint16)0x01<<
            (uint16)0x0733<< (uint16)0x01<<
            (uint16)0x0732<< (uint16)0x01<<
            (uint16)0x0702<< (uint16)0x00<<
            (uint16)0x0701<< (uint16)0x00<<
            (uint16)0x0700<< (uint16)0x00<<
            (uint16)0x06FE<< (uint16)0x00<<
            (uint16)0x06FD<< (uint16)0x00<<
            (uint16)0x06FC<< (uint16)0x00<<
            (uint16)0x06FB<< (uint16)0x00<<
            (uint16)0x06F8<< (uint16)0x00<<
            (uint16)0x06F7<< (uint16)0x00<<
            (uint16)0x06F6<< (uint16)0x00<<
            (uint16)0x06F4<< (uint16)0x07D0 <<
            (uint16)0x06F2<< (uint16)0x00<<
            (uint16)0x06F0<< (uint16)0x00<<
            (uint16)0x06EF<< (uint16)0x00<<
            (uint16)0x06EC<< (uint16)0x00<<
            (uint16)0x06EA<< (uint16)0x00<<
            (uint16)0x06E9<< (uint16)0x00<<
            (uint16)0x06E8<< (uint16)0x00<<
            (uint16)0x06E7<< (uint16)0x00<<
            (uint16)0x0641<< (uint16)0x03<<
            (uint16)0x062E<< (uint16)0x00<<
            (uint16)0x062D<< (uint16)0x00<<
            (uint16)0x060A<< (uint16)0x00<<
            (uint16)0x0609<< (uint16)0x00<<
            (uint16)0x0518<< (uint16)0x00<<
            (uint16)0x0517<< (uint16)0x00<<
            (uint16)0x0703<< (uint16)0x00;
        GetSession()->SendPacket(&data);
    }
}

void Player::AddWeather()
{
    uint32 zoneid = GetZoneId();
    if(!sWorld.FindWeather(zoneid))
    {
        Weather *wth = new Weather(this);
        sWorld.AddWeather(wth);
    }
}

uint32 Player::ApplyRestBonus(uint32 xp)
{
    uint32 bonus = m_restTime / 1000;
    if(bonus < 1 )
        bonus = 1;
    if(bonus > 3 )
        bonus = 3;
    if(m_restTime < bonus * 1000)
        m_restTime = 0;
    else
        m_restTime -= bonus * 1000;
    return bonus * xp;
}

uint8 Player::CheckFishingAble()
{
    uint32 zone = GetZoneId();
    uint32 fish_value = GetSkillValue(SKILL_FISHING);
    uint32 ZoneMaxSkill;
    switch(zone)
    {
        case 1:
            ZoneMaxSkill=50;
            break;
        case 2:
            ZoneMaxSkill=100;
            break;
        case 8:
            ZoneMaxSkill=225;
            break;
        case 9:
            ZoneMaxSkill=50;
            break;
        case 10:
            ZoneMaxSkill=50;
            break;
        case 11:
            ZoneMaxSkill=150;
            break;
        case 12:
            ZoneMaxSkill=50;
            break;
        case 14:
            ZoneMaxSkill=50;
            break;
        case 15:
            ZoneMaxSkill=225;
            break;
        case 16:
            ZoneMaxSkill=275;
            break;
        case 17:
            ZoneMaxSkill=275;
            break;
        case 18:
            ZoneMaxSkill=50;
            break;
        case 28:
            ZoneMaxSkill=290;
            break;
        case 33:
            ZoneMaxSkill=225;
            break;
        case 35:
            ZoneMaxSkill=225;
            break;
        case 37:
            ZoneMaxSkill=225;
            break;
        case 38:
            ZoneMaxSkill=100;
            break;
        case 40:
            ZoneMaxSkill=100;
            break;
        case 43:
            ZoneMaxSkill=225;
            break;
        case 44:
            ZoneMaxSkill=125;
            break;
        case 45:
            ZoneMaxSkill=200;
            break;
        case 47:
            ZoneMaxSkill=250;
            break;
        case 55:
            ZoneMaxSkill=200;
            break;
        case 57:
            ZoneMaxSkill=50;
            break;
        case 60:
            ZoneMaxSkill=50;
            break;
        case 61:
            ZoneMaxSkill=50;
            break;
        case 62:
            ZoneMaxSkill=50;
            break;
        case 63:
            ZoneMaxSkill=50;
            break;
        case 64:
            ZoneMaxSkill=50;
            break;
        case 68:
            ZoneMaxSkill=150;
            break;
        case 69:
            ZoneMaxSkill=125;
            break;
        case 71:
            ZoneMaxSkill=225;
            break;
        case 74:
            ZoneMaxSkill=225;
            break;
        case 75:
            ZoneMaxSkill=225;
            break;
        case 76:
            ZoneMaxSkill=225;
            break;
        case 85:
            ZoneMaxSkill=50;
            break;
        case 86:
            ZoneMaxSkill=50;
            break;
        case 87:
            ZoneMaxSkill=50;
            break;
        case 88:
            ZoneMaxSkill=50;
            break;
        case 89:
            ZoneMaxSkill=50;
            break;
        case 92:
            ZoneMaxSkill=50;
            break;
        case 100:
            ZoneMaxSkill=225;
            break;
        case 102:
            ZoneMaxSkill=225;
            break;
        case 104:
            ZoneMaxSkill=225;
            break;
        case 115:
            ZoneMaxSkill=100;
            break;
        case 116:
            ZoneMaxSkill=225;
            break;
        case 117:
            ZoneMaxSkill=225;
            break;
        case 122:
            ZoneMaxSkill=225;
            break;
        case 129:
            ZoneMaxSkill=225;
            break;
        case 130:
            ZoneMaxSkill=100;
            break;
        case 139:
            ZoneMaxSkill=300;
            break;
        case 141:
            ZoneMaxSkill=50;
            break;
        case 146:
            ZoneMaxSkill=50;
            break;
        case 150:
            ZoneMaxSkill=150;
            break;
        case 162:
            ZoneMaxSkill=50;
            break;
        case 163:
            ZoneMaxSkill=50;
            break;
        case 168:
            ZoneMaxSkill=50;
            break;
        case 169:
            ZoneMaxSkill=50;
            break;
        case 172:
            ZoneMaxSkill=100;
            break;
        case 187:
            ZoneMaxSkill=50;
            break;
        case 188:
            ZoneMaxSkill=50;
            break;
        case 193:
            ZoneMaxSkill=290;
            break;
        case 202:
            ZoneMaxSkill=290;
            break;
        case 211:
            ZoneMaxSkill=50;
            break;
        case 221:
            ZoneMaxSkill=50;
            break;
        case 223:
            ZoneMaxSkill=50;
            break;
        case 226:
            ZoneMaxSkill=100;
            break;
        case 227:
            ZoneMaxSkill=100;
            break;
        case 237:
            ZoneMaxSkill=100;
            break;
        case 249:
            ZoneMaxSkill=280;
            break;
        case 256:
            ZoneMaxSkill=50;
            break;
        case 258:
            ZoneMaxSkill=50;
            break;
        case 259:
            ZoneMaxSkill=50;
            break;
        case 265:
            ZoneMaxSkill=50;
            break;
        case 266:
            ZoneMaxSkill=50;
            break;
        case 267:
            ZoneMaxSkill=150;
            break;
        case 271:
            ZoneMaxSkill=150;
            break;
        case 272:
            ZoneMaxSkill=150;
            break;
        case 279:
            ZoneMaxSkill=200;
            break;
        case 284:
            ZoneMaxSkill=200;
            break;
        case 295:
            ZoneMaxSkill=150;
            break;
        case 297:
            ZoneMaxSkill=225;
            break;
        case 298:
            ZoneMaxSkill=150;
            break;
        case 299:
            ZoneMaxSkill=150;
            break;
        case 300:
            ZoneMaxSkill=225;
            break;
        case 301:
            ZoneMaxSkill=225;
            break;
        case 302:
            ZoneMaxSkill=225;
            break;
        case 305:
            ZoneMaxSkill=100;
            break;
        case 306:
            ZoneMaxSkill=100;
            break;
        case 307:
            ZoneMaxSkill=250;
            break;
        case 309:
            ZoneMaxSkill=100;
            break;
        case 310:
            ZoneMaxSkill=225;
            break;
        case 311:
            ZoneMaxSkill=225;
            break;
        case 312:
            ZoneMaxSkill=225;
            break;
        case 314:
            ZoneMaxSkill=200;
            break;
        case 317:
            ZoneMaxSkill=200;
            break;
        case 323:
            ZoneMaxSkill=100;
            break;
        case 324:
            ZoneMaxSkill=200;
            break;
        case 327:
            ZoneMaxSkill=200;
            break;
        case 328:
            ZoneMaxSkill=200;
            break;
        case 331:
            ZoneMaxSkill=150;
            break;
        case 350:
            ZoneMaxSkill=250;
            break;
        case 351:
            ZoneMaxSkill=250;
            break;
        case 353:
            ZoneMaxSkill=250;
            break;
        case 356:
            ZoneMaxSkill=250;
            break;
        case 361:
            ZoneMaxSkill=250;
            break;
        case 363:
            ZoneMaxSkill=50;
            break;
        case 367:
            ZoneMaxSkill=50;
            break;
        case 368:
            ZoneMaxSkill=50;
            break;
        case 373:
            ZoneMaxSkill=50;
            break;
        case 374:
            ZoneMaxSkill=50;
            break;
        case 375:
            ZoneMaxSkill=300;
            break;
        case 382:
            ZoneMaxSkill=125;
            break;
        case 384:
            ZoneMaxSkill=125;
            break;
        case 385:
            ZoneMaxSkill=125;
            break;
        case 386:
            ZoneMaxSkill=125;
            break;
        case 387:
            ZoneMaxSkill=125;
            break;
        case 388:
            ZoneMaxSkill=125;
            break;
        case 391:
            ZoneMaxSkill=125;
            break;
        case 392:
            ZoneMaxSkill=125;
            break;
        case 393:
            ZoneMaxSkill=50;
            break;
        case 401:
            ZoneMaxSkill=125;
            break;
        case 405:
            ZoneMaxSkill=200;
            break;
        case 406:
            ZoneMaxSkill=135;
            break;
        case 414:
            ZoneMaxSkill=150;
            break;
        case 415:
            ZoneMaxSkill=150;
            break;
        case 416:
            ZoneMaxSkill=150;
            break;
        case 418:
            ZoneMaxSkill=150;
            break;
        case 420:
            ZoneMaxSkill=150;
            break;
        case 421:
            ZoneMaxSkill=150;
            break;
        case 422:
            ZoneMaxSkill=150;
            break;
        case 424:
            ZoneMaxSkill=150;
            break;
        case 429:
            ZoneMaxSkill=150;
            break;
        case 433:
            ZoneMaxSkill=150;
            break;
        case 434:
            ZoneMaxSkill=150;
            break;
        case 437:
            ZoneMaxSkill=150;
            break;
        case 441:
            ZoneMaxSkill=150;
            break;
        case 442:
            ZoneMaxSkill=100;
            break;
        case 443:
            ZoneMaxSkill=100;
            break;
        case 445:
            ZoneMaxSkill=100;
            break;
        case 448:
            ZoneMaxSkill=100;
            break;
        case 449:
            ZoneMaxSkill=100;
            break;
        case 452:
            ZoneMaxSkill=100;
            break;
        case 453:
            ZoneMaxSkill=100;
            break;
        case 454:
            ZoneMaxSkill=100;
            break;
        case 456:
            ZoneMaxSkill=100;
            break;
        case 460:
            ZoneMaxSkill=135;
            break;
        case 463:
            ZoneMaxSkill=275;
            break;
        case 464:
            ZoneMaxSkill=135;
            break;
        case 478:
            ZoneMaxSkill=50;
            break;
        case 490:
            ZoneMaxSkill=275;
            break;
        case 493:
            ZoneMaxSkill=300;
            break;
        case 496:
            ZoneMaxSkill=225;
            break;
        case 497:
            ZoneMaxSkill=225;
            break;
        case 501:
            ZoneMaxSkill=225;
            break;
        case 502:
            ZoneMaxSkill=225;
            break;
        case 504:
            ZoneMaxSkill=225;
            break;
        case 508:
            ZoneMaxSkill=225;
            break;
        case 509:
            ZoneMaxSkill=225;
            break;
        case 510:
            ZoneMaxSkill=225;
            break;
        case 511:
            ZoneMaxSkill=225;
            break;
        case 513:
            ZoneMaxSkill=225;
            break;
        case 516:
            ZoneMaxSkill=225;
            break;
        case 517:
            ZoneMaxSkill=225;
            break;
        case 518:
            ZoneMaxSkill=200;
            break;
        case 537:
            ZoneMaxSkill=250;
            break;
        case 538:
            ZoneMaxSkill=250;
            break;
        case 542:
            ZoneMaxSkill=250;
            break;
        case 543:
            ZoneMaxSkill=250;
            break;
        case 556:
            ZoneMaxSkill=50;
            break;
        case 576:
            ZoneMaxSkill=150;
            break;
        case 598:
            ZoneMaxSkill=200;
            break;
        case 602:
            ZoneMaxSkill=200;
            break;
        case 604:
            ZoneMaxSkill=200;
            break;
        case 618:
            ZoneMaxSkill=300;
            break;
        case 636:
            ZoneMaxSkill=135;
            break;
        case 656:
            ZoneMaxSkill=300;
            break;
        case 657:
            ZoneMaxSkill=225;
            break;
        case 702:
            ZoneMaxSkill=50;
            break;
        case 719:
            ZoneMaxSkill=135;
            break;
        case 720:
            ZoneMaxSkill=135;
            break;
        case 797:
            ZoneMaxSkill=225;
            break;
        case 799:
            ZoneMaxSkill=150;
            break;
        case 810:
            ZoneMaxSkill=50;
            break;
        case 814:
            ZoneMaxSkill=50;
            break;
        case 815:
            ZoneMaxSkill=125;
            break;
        case 818:
            ZoneMaxSkill=50;
            break;
        case 878:
            ZoneMaxSkill=275;
            break;
        case 879:
            ZoneMaxSkill=150;
            break;
        case 896:
            ZoneMaxSkill=150;
            break;
        case 917:
            ZoneMaxSkill=100;
            break;
        case 919:
            ZoneMaxSkill=100;
            break;
        case 922:
            ZoneMaxSkill=100;
            break;
        case 923:
            ZoneMaxSkill=50;
            break;
        case 927:
            ZoneMaxSkill=50;
            break;
        case 968:
            ZoneMaxSkill=250;
            break;
        case 977:
            ZoneMaxSkill=250;
            break;
        case 978:
            ZoneMaxSkill=250;
            break;
        case 979:
            ZoneMaxSkill=250;
            break;
        case 983:
            ZoneMaxSkill=250;
            break;
        case 988:
            ZoneMaxSkill=250;
            break;
        case 997:
            ZoneMaxSkill=125;
            break;
        case 998:
            ZoneMaxSkill=125;
            break;
        case 1001:
            ZoneMaxSkill=125;
            break;
        case 1002:
            ZoneMaxSkill=125;
            break;
        case 1008:
            ZoneMaxSkill=250;
            break;
        case 1017:
            ZoneMaxSkill=150;
            break;
        case 1018:
            ZoneMaxSkill=150;
            break;
        case 1020:
            ZoneMaxSkill=150;
            break;
        case 1021:
            ZoneMaxSkill=150;
            break;
        case 1022:
            ZoneMaxSkill=150;
            break;
        case 1023:
            ZoneMaxSkill=150;
            break;
        case 1024:
            ZoneMaxSkill=150;
            break;
        case 1025:
            ZoneMaxSkill=150;
            break;
        case 1039:
            ZoneMaxSkill=150;
            break;
        case 1056:
            ZoneMaxSkill=290;
            break;
        case 1097:
            ZoneMaxSkill=150;
            break;
        case 1099:
            ZoneMaxSkill=300;
            break;
        case 1101:
            ZoneMaxSkill=250;
            break;
        case 1102:
            ZoneMaxSkill=250;
            break;
        case 1106:
            ZoneMaxSkill=250;
            break;
        case 1112:
            ZoneMaxSkill=250;
            break;
        case 1116:
            ZoneMaxSkill=250;
            break;
        case 1117:
            ZoneMaxSkill=250;
            break;
        case 1119:
            ZoneMaxSkill=250;
            break;
        case 1120:
            ZoneMaxSkill=250;
            break;
        case 1121:
            ZoneMaxSkill=250;
            break;
        case 1126:
            ZoneMaxSkill=225;
            break;
        case 1136:
            ZoneMaxSkill=250;
            break;
        case 1156:
            ZoneMaxSkill=225;
            break;
        case 1176:
            ZoneMaxSkill=250;
            break;
        case 1222:
            ZoneMaxSkill=275;
            break;
        case 1227:
            ZoneMaxSkill=275;
            break;
        case 1228:
            ZoneMaxSkill=275;
            break;
        case 1229:
            ZoneMaxSkill=275;
            break;
        case 1230:
            ZoneMaxSkill=275;
            break;
        case 1231:
            ZoneMaxSkill=275;
            break;
        case 1234:
            ZoneMaxSkill=275;
            break;
        case 1256:
            ZoneMaxSkill=275;
            break;
        case 1296:
            ZoneMaxSkill=50;
            break;
        case 1297:
            ZoneMaxSkill=50;
            break;
        case 1336:
            ZoneMaxSkill=250;
            break;
        case 1337:
            ZoneMaxSkill=250;
            break;
        case 1338:
            ZoneMaxSkill=100;
            break;
        case 1339:
            ZoneMaxSkill=200;
            break;
        case 1477:
            ZoneMaxSkill=275;
            break;
        case 1519:
            ZoneMaxSkill=50;
            break;
        case 1557:
            ZoneMaxSkill=175;
            break;
        case 1577:
            ZoneMaxSkill=225;
            break;
        case 1578:
            ZoneMaxSkill=225;
            break;
        case 1581:
            ZoneMaxSkill=100;
            break;
        case 1617:
            ZoneMaxSkill=50;
            break;
        case 1638:
            ZoneMaxSkill=50;
            break;
        case 1662:
            ZoneMaxSkill=50;
            break;
        case 1681:
            ZoneMaxSkill=200;
            break;
        case 1682:
            ZoneMaxSkill=200;
            break;
        case 1684:
            ZoneMaxSkill=200;
            break;
        case 1701:
            ZoneMaxSkill=125;
            break;
        case 1738:
            ZoneMaxSkill=225;
            break;
        case 1739:
            ZoneMaxSkill=225;
            break;
        case 1740:
            ZoneMaxSkill=225;
            break;
        case 1760:
            ZoneMaxSkill=225;
            break;
        case 1762:
            ZoneMaxSkill=250;
            break;
        case 1764:
            ZoneMaxSkill=225;
            break;
        case 1765:
            ZoneMaxSkill=225;
            break;
        case 1767:
            ZoneMaxSkill=275;
            break;
        case 1770:
            ZoneMaxSkill=275;
            break;
        case 1777:
            ZoneMaxSkill=225;
            break;
        case 1778:
            ZoneMaxSkill=225;
            break;
        case 1780:
            ZoneMaxSkill=225;
            break;
        case 1797:
            ZoneMaxSkill=225;
            break;
        case 1798:
            ZoneMaxSkill=225;
            break;
        case 1883:
            ZoneMaxSkill=250;
            break;
        case 1884:
            ZoneMaxSkill=250;
            break;
        case 1939:
            ZoneMaxSkill=250;
            break;
        case 1940:
            ZoneMaxSkill=250;
            break;
        case 1942:
            ZoneMaxSkill=250;
            break;
        case 1977:
            ZoneMaxSkill=225;
            break;
        case 1997:
            ZoneMaxSkill=275;
            break;
        case 1998:
            ZoneMaxSkill=275;
            break;
        case 2017:
            ZoneMaxSkill=300;
            break;
        case 2077:
            ZoneMaxSkill=100;
            break;
        case 2078:
            ZoneMaxSkill=100;
            break;
        case 2079:
            ZoneMaxSkill=225;
            break;
        case 2097:
            ZoneMaxSkill=175;
            break;
        case 2100:
            ZoneMaxSkill=245;
            break;
        case 2158:
            ZoneMaxSkill=250;
            break;
        case 2246:
            ZoneMaxSkill=300;
            break;
        case 2256:
            ZoneMaxSkill=300;
            break;
        case 2270:
            ZoneMaxSkill=300;
            break;
        case 2272:
            ZoneMaxSkill=300;
            break;
        case 2277:
            ZoneMaxSkill=300;
            break;
        case 2279:
            ZoneMaxSkill=300;
            break;
        case 2298:
            ZoneMaxSkill=300;
            break;
        case 2302:
            ZoneMaxSkill=225;
            break;
        case 2317:
            ZoneMaxSkill=250;
            break;
        case 2318:
            ZoneMaxSkill=225;
            break;
        case 2321:
            ZoneMaxSkill=275;
            break;
        case 2322:
            ZoneMaxSkill=50;
            break;
        case 2323:
            ZoneMaxSkill=250;
            break;
        case 2324:
            ZoneMaxSkill=200;
            break;
        case 2325:
            ZoneMaxSkill=150;
            break;
        case 2326:
            ZoneMaxSkill=100;
            break;
        case 2364:
            ZoneMaxSkill=100;
            break;
        case 2365:
            ZoneMaxSkill=150;
            break;
        case 2398:
            ZoneMaxSkill=100;
            break;
        case 2399:
            ZoneMaxSkill=50;
            break;
        case 2400:
            ZoneMaxSkill=250;
            break;
        case 2401:
            ZoneMaxSkill=200;
            break;
        case 2402:
            ZoneMaxSkill=100;
            break;
        case 2403:
            ZoneMaxSkill=225;
            break;
        case 2405:
            ZoneMaxSkill=200;
            break;
        case 2408:
            ZoneMaxSkill=200;
            break;
        case 2457:
            ZoneMaxSkill=150;
            break;
        case 2477:
            ZoneMaxSkill=300;
            break;
        case 2481:
            ZoneMaxSkill=275;
            break;
        case 2521:
            ZoneMaxSkill=250;
            break;
        case 2522:
            ZoneMaxSkill=250;
            break;
        case 2558:
            ZoneMaxSkill=300;
            break;
        case 2562:
            ZoneMaxSkill=300;
            break;
        case 2597:
            ZoneMaxSkill=300;
            break;
        case 2618:
            ZoneMaxSkill=275;
            break;
        case 2619:
            ZoneMaxSkill=300;
            break;
        case 2620:
            ZoneMaxSkill=290;
            break;
        case 2624:
            ZoneMaxSkill=300;
            break;
        case 2631:
            ZoneMaxSkill=300;
            break;
        case 2797:
            ZoneMaxSkill=150;
            break;
        case 2837:
            ZoneMaxSkill=300;
            break;
        case 2897:
            ZoneMaxSkill=150;
            break;
        default:
            ZoneMaxSkill=50;
            break;
    }
    if((ZoneMaxSkill-50) > fish_value )
        return 0;
    else if(ZoneMaxSkill-50 <= fish_value && fish_value < ZoneMaxSkill-25)
        return 1;
    else if(ZoneMaxSkill-25 <= fish_value && fish_value < ZoneMaxSkill)
        return 2;
    else if(ZoneMaxSkill <= fish_value && fish_value < ZoneMaxSkill + 25)
        return 3;
    else return 4;
}

void Player::SetBindPoint(uint64 guid)
{
    WorldPacket data;
    data.Initialize( SMSG_BINDER_CONFIRM );
    data << guid;
    GetSession()->SendPacket( &data );
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/
bool Player::CanSeeStartQuest( Quest *pQuest )
{
    sLog.outString( "QUEST: CanSeeQuest");
    if( pQuest )
    {
        if( SatisfyQuestRace( pQuest, false ) && SatisfyQuestClass( pQuest, false ) && SatisfyQuestSkill( pQuest, false ) && SatisfyQuestReputation( pQuest, false ) && SatisfyQuestPreviousQuest( pQuest, false ) )
            return ( (int32)getLevel() >= (int32)pQuest->GetQuestInfo()->MinLevel - (int32)7 );
    }
    return false;
}

bool Player::CanTakeQuest( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: CanTakeQuest");
    if( pQuest )
        return ( SatisfyQuestStatus( pQuest, msg ) && SatisfyQuestRace( pQuest, msg ) && SatisfyQuestLevel( pQuest, msg ) && SatisfyQuestClass( pQuest, msg ) && SatisfyQuestSkill( pQuest, msg ) && SatisfyQuestReputation( pQuest, msg ) && SatisfyQuestPreviousQuest( pQuest, msg ) && SatisfyQuestTimed( pQuest, msg ) );
    return false;
}

bool Player::CanAddQuest( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: CanAddQuest");
    if( pQuest )
    {
        if( !SatisfyQuestLog( msg ) )
            return false;

        if( !GiveQuestSourceItem( pQuest ) )
        {
            if( msg )
            {
                WorldPacket data;
                data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
                data << uint8(EQUIP_ERR_BAG_FULL);
                data << uint64(0);
                data << uint64(0);
                data << uint8(0);
                GetSession()->SendPacket(&data);
            }
            return false;
        }

        return true;
    }
    return false;
}

bool Player::CanCompleteQuest( Quest *pQuest )
{
    sLog.outString( "QUEST: CanCompleteQuest");
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;

        if( mQuestStatus[quest].status == QUEST_STATUS_COMPLETE )
            return true;

        if ( mQuestStatus[quest].status == QUEST_STATUS_INCOMPLETE )
        {
            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( mQuestStatus[quest].m_questItemCount[i] < pQuest->GetQuestInfo()->ReqItemCount[i] )
                        return false;
                }
            }

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( mQuestStatus[quest].m_questMobCount[i] < pQuest->GetQuestInfo()->ReqKillMobCount[i] )
                        return false;
                }
            }

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_EXPLORATION ) && !mQuestStatus[quest].m_explored )
                return false;

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) && (mQuestStatus[quest].m_timer <= 0) )
                return false;

            if ( pQuest->GetQuestInfo()->RewMoney < 0 )
            {
                if ( GetMoney() + pQuest->GetQuestInfo()->RewMoney < 0 )
                    return false;
            }
            return true;
        }
    }
    return false;
}

bool Player::CanRewardQuest( Quest *pQuest, uint32 reward, bool msg )
{
    sLog.outString( "QUEST: CanRewardQuest");
    if( pQuest )
    {
        WorldPacket data;
        uint32 count;
        if ( pQuest->m_qRewChoiceItemsCount > 0 )
        {
            count = CanAddItemCount(pQuest->GetQuestInfo()->RewChoiceItemId[reward], pQuest->GetQuestInfo()->RewChoiceItemCount[reward]);
            if  ( count < pQuest->GetQuestInfo()->RewChoiceItemCount[reward] )
            {
                if( msg )
                {
                    data.clear();
                    data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
                    data << uint8(EQUIP_ERR_BAG_FULL);
                    data << uint64(0);
                    data << uint64(0);
                    data << uint8(0);
                    GetSession()->SendPacket(&data);
                }
                return false;
            }
        }

        if ( pQuest->m_qRewItemsCount > 0 )
        {
            for (int i=0; i < QUEST_REWARDS_COUNT; i++)
            {
                count = CanAddItemCount(pQuest->GetQuestInfo()->RewItemId[i], pQuest->GetQuestInfo()->RewItemCount[i]);
                if  (  count < pQuest->GetQuestInfo()->RewItemCount[i] )
                {
                    if( msg )
                    {
                        data.clear();
                        data.Initialize(SMSG_INVENTORY_CHANGE_FAILURE);
                        data << uint8(EQUIP_ERR_BAG_FULL);
                        data << uint64(0);
                        data << uint64(0);
                        data << uint8(0);
                        GetSession()->SendPacket(&data);
                    }
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

void Player::AddQuest( Quest *pQuest )
{
    sLog.outString( "QUEST: AddQuest");
    if( pQuest )
    {
        uint16 log_slot = GetQuestSlot( NULL );
        if( log_slot )
        {
            uint32 quest = pQuest->GetQuestInfo()->QuestId;

            SetUInt32Value(log_slot + 0, quest);
            SetUInt32Value(log_slot + 1, 0);
            SetUInt32Value(log_slot + 2, 0);

            mQuestStatus[quest].m_quest = pQuest;
            mQuestStatus[quest].status = QUEST_STATUS_INCOMPLETE;
            mQuestStatus[quest].rewarded = false;

            GiveQuestSourceItem( pQuest );
            AdjustQuestReqItemCount( pQuest );
        }
    }
}

void Player::CompleteQuest( Quest *pQuest )
{
    sLog.outString( "QUEST: CompleteQuest");
    if( pQuest )
    {
        SetQuestStatus( pQuest, QUEST_STATUS_COMPLETE);
        PlayerTalkClass->SendQuestCompleteToLog( pQuest );
    }
}

void Player::RewardQuest( Quest *pQuest, uint32 reward )
{
    sLog.outString( "QUEST: RewardQuest");
    if( pQuest )
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++ )
        {
            if ( pQuest->GetQuestInfo()->ReqItemId[i] )
                RemoveItemFromInventory( pQuest->GetQuestInfo()->ReqItemId[i], pQuest->GetQuestInfo()->ReqItemCount[i]);
        }

        if ( pQuest->m_qRewChoiceItemsCount > 0 )
        {
            if( pQuest->GetQuestInfo()->RewChoiceItemId[reward] )
                AddNewItem(pQuest->GetQuestInfo()->RewChoiceItemId[reward], pQuest->GetQuestInfo()->RewChoiceItemCount[reward], false);
        }

        if ( pQuest->m_qRewItemsCount > 0 )
        {
            for (int i=0; i < QUEST_REWARDS_COUNT; i++)
            {
                if( pQuest->GetQuestInfo()->RewItemId[i] )
                    AddNewItem(pQuest->GetQuestInfo()->RewItemId[i], pQuest->GetQuestInfo()->RewItemCount[i], false);
            }
        }

        if ( pQuest->GetQuestInfo()->RewSpell > 0 )
        {
            WorldPacket sdata;

            sdata.Initialize (SMSG_LEARNED_SPELL);
            sdata << pQuest->GetQuestInfo()->RewSpell;
            m_session->SendPacket( &sdata );
            addSpell( (uint16)pQuest->GetQuestInfo()->RewSpell );
        }

        uint32 quest = pQuest->GetQuestInfo()->QuestId;

        PlayerTalkClass->SendQuestComplete( pQuest );
        uint16 log_slot = GetQuestSlot( pQuest );
        SetUInt32Value(log_slot + 0, 0);
        SetUInt32Value(log_slot + 1, 0);
        SetUInt32Value(log_slot + 2, 0);

        if ( getLevel() < 60 )
        {
            GiveXP( pQuest->XPValue( this ), this->GetGUID() );
            ModifyMoney( pQuest->GetQuestInfo()->RewMoney );
        }
        else
            ModifyMoney( pQuest->GetQuestInfo()->RewMoney + pQuest->XPValue( this ) );

        if ( !pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ) )
            mQuestStatus[quest].rewarded = true;
        else
            SetQuestStatus(pQuest, QUEST_STATUS_NONE);
    }
}

bool Player::SatisfyQuestClass( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestClass");
    if( pQuest )
    {
        uint32 reqclasses = pQuest->GetQuestInfo()->RequiredClass;
        if ( reqclasses == QUEST_CLASS_NONE )
            return true;
        if( (reqclasses & getClassMask()) == 0 )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
    }
    return false;
}

bool Player::SatisfyQuestLevel( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestLevel");
    if( pQuest )
    {
        if( getLevel() < pQuest->GetQuestInfo()->MinLevel )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestLog( bool msg )
{
    uint16 log_slot = GetQuestSlot( NULL );
    if( log_slot )
        return true;
    else
    {
        if( msg )
            PlayerTalkClass->SendQuestLogFull();
        return false;
    }
}

bool Player::SatisfyQuestPreviousQuest( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestPreviousQuest");
    if( pQuest )
    {
        uint32 previousquest = pQuest->GetQuestInfo()->PrevQuestId;
        if( previousquest == 0 )
            return true;
        if( mQuestStatus.find( previousquest ) == mQuestStatus.end() || !mQuestStatus[previousquest].rewarded )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestRace( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestRace");
    if( pQuest )
    {
        uint32 reqraces = pQuest->GetQuestInfo()->RequiredRaces;
        if ( reqraces == QUEST_RACE_NONE )
            return true;
        if( (reqraces & getRace()) == 0 )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_RACE );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestReputation( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestReputation");
    if( pQuest )
        return true;
    return false;
}

bool Player::SatisfyQuestSkill( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestSkill");
    if( pQuest )
    {
        uint32 reqskill = pQuest->GetQuestInfo()->RequiredTradeskill;
        if( reqskill == QUEST_TRSKILL_NONE )
            return true;
        if( GetSkillValue( reqskill ) == 0 )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestStatus( Quest *pQuest, bool msg )
{
    sLog.outString( "QUEST: SatisfyQuestStatus");
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if  ( mQuestStatus.find( quest ) != mQuestStatus.end() )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_HAVE_QUEST );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestTimed( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        if ( GetTimedQuest() && pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED) )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_HAVE_TIMED_QUEST );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::GiveQuestSourceItem( Quest *pQuest )
{
    sLog.outString( "QUEST: GiveQuestSourceItem");
    if( pQuest )
    {
        uint32 srcitem = pQuest->GetQuestInfo()->SrcItemId;
        if( srcitem > 0 )
        {
            uint32 count = pQuest->GetQuestInfo()->SrcItemCount;
            if( count <= 0 )
                count = 1;
            if( count == AddNewItem(srcitem, count, false))
                return true;
            return false;
        }
        return true;
    }
    return false;
}

void Player::TakeQuestSourceItem( Quest *pQuest )
{
    sLog.outString( "QUEST: TakeQuestSourceItem");
    if( pQuest )
    {
        uint32 srcitem = pQuest->GetQuestInfo()->SrcItemId;
        if( srcitem > 0 )
        {
            uint32 count = pQuest->GetQuestInfo()->SrcItemCount;
            if( count <= 0 )
                count = 1;
            RemoveItemFromInventory(srcitem, count);;
        }
    }
}

bool Player::GetQuestRewardStatus( Quest *pQuest )
{
    sLog.outString( "QUEST: GetQuestRewardStatus");
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if  ( mQuestStatus.find( quest ) == mQuestStatus.end() )
            return false;
        return mQuestStatus[quest].rewarded;
    }
    return false;
}

uint32 Player::GetQuestStatus( Quest *pQuest )
{
    sLog.outString( "QUEST: GetQuestStatus");
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if  ( mQuestStatus.find( quest ) != mQuestStatus.end() )
            return mQuestStatus[quest].status;
    }
    return QUEST_STATUS_NONE;
}

void Player::SetQuestStatus( Quest *pQuest, uint32 status )
{
    sLog.outString( "QUEST: SetQuestStatus");
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if ( status == QUEST_STATUS_NONE )
            mQuestStatus.erase( quest );
        else
            mQuestStatus[quest].status = status;
    }
}

void Player::AdjustQuestReqItemCount( Quest *pQuest )
{
    sLog.outString( "QUEST: AdjustQuestReqItemCount");
    if( pQuest )
    {
        if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
        {
            uint32 quest = pQuest->GetQuestInfo()->QuestId;
            uint32 reqitemcount;
            uint32 curitemcount;
            for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            {
                reqitemcount = pQuest->GetQuestInfo()->ReqItemCount[i];
                curitemcount = GetItemCount(pQuest->GetQuestInfo()->ReqItemId[i], false);
                mQuestStatus[quest].m_questItemCount[i] = min(curitemcount, reqitemcount);
            }
        }
    }
}

uint16 Player::GetQuestSlot( Quest *pQuest )
{
    uint32 quest = 0;
    if( pQuest )
        quest = pQuest->GetQuestInfo()->QuestId;

    for ( uint16 i = 0; i < 20; i++ )
    {
        if ( GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i) == quest )
            return PLAYER_QUEST_LOG_1_1 + 3*i;
    }
    return 0;
}

void Player::ItemAdded( uint32 entry, uint32 count )
{
    quest_status qs;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 additemcount;
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        qs = i->second;
        if ( qs.status == QUEST_STATUS_INCOMPLETE )
        {
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                reqitem = qs.m_quest->GetQuestInfo()->ReqItemId[j];
                if ( reqitem == entry )
                {
                    reqitemcount = qs.m_quest->GetQuestInfo()->ReqItemCount[j];
                    curitemcount = mQuestStatus[i->first].m_questItemCount[j];
                    if ( curitemcount < reqitemcount )
                    {
                        additemcount = (curitemcount + count <= reqitemcount ? count: reqitemcount - curitemcount);
                        mQuestStatus[i->first].m_questItemCount[j] += additemcount;
                        PlayerTalkClass->SendQuestUpdateAddItem(qs.m_quest, j, additemcount);
                    }
                    if ( CanCompleteQuest( qs.m_quest ) )
                    {
                        CompleteQuest( qs.m_quest );
                        PlayerTalkClass->SendQuestUpdateComplete( qs.m_quest );
                    }
                    return;
                }
            }
        }
    }
}

void Player::ItemRemoved( uint32 entry, uint32 count )
{
    quest_status qs;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 remitemcount;
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        qs = i->second;
        if ( qs.status == QUEST_STATUS_COMPLETE )
        {
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                reqitem = qs.m_quest->GetQuestInfo()->ReqItemId[j];
                if ( reqitem == entry )
                {
                    reqitemcount = qs.m_quest->GetQuestInfo()->ReqItemCount[j];
                    curitemcount = GetItemCount(entry, true);
                    if ( curitemcount - count < reqitemcount )
                    {
                        remitemcount = reqitemcount - curitemcount + count;
                        mQuestStatus[i->first].m_questItemCount[j] = curitemcount - remitemcount;
                        PlayerTalkClass->SendQuestIncompleteToLog(qs.m_quest);
                        SetQuestStatus(qs.m_quest, QUEST_STATUS_INCOMPLETE);
                    }
                    return;
                }
            }
        }
    }
}

void Player::KilledMonster( uint32 entry, uint64 guid )
{
    quest_status qs;
    uint32 reqkill;
    uint32 reqkillcount;
    uint32 curkillcount;
    uint32 addkillcount = 1;
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        qs = i->second;
        if ( qs.status == QUEST_STATUS_INCOMPLETE )
        {
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                reqkill = qs.m_quest->GetQuestInfo()->ReqKillMobId[j];
                if ( reqkill == entry )
                {
                    reqkillcount = qs.m_quest->GetQuestInfo()->ReqKillMobCount[j];
                    curkillcount = qs.m_questMobCount[j];
                    if ( curkillcount < reqkillcount )
                    {
                        mQuestStatus[i->first].m_questMobCount[j] = curkillcount + addkillcount;
                        PlayerTalkClass->SendQuestUpdateAddKill(qs.m_quest, guid, curkillcount + addkillcount, j);
                    }
                    if ( CanCompleteQuest( qs.m_quest ) )
                    {
                        CompleteQuest( qs.m_quest );
                        PlayerTalkClass->SendQuestUpdateComplete( qs.m_quest );
                    }
                    return;
                }
            }
        }
    }
}

void Player::AddQuestsLoot( Creature* creature )
{
    quest_status qs;
    uint32 itemid=0;
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        qs=i->second;
        if (qs.status == QUEST_STATUS_INCOMPLETE)
        {
            if (!qs.m_quest) continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                itemid=qs.m_quest->GetQuestInfo()->ReqItemId[j];
                if ( itemid>0 )
                {
                    ChangeLoot(&(creature->loot),creature->GetCreatureInfo()->lootid,itemid,70.0f);
                }
            }
        }
    }
}

void Player::SendQuestFailed( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        WorldPacket data;
        data.Initialize( SMSG_QUESTGIVER_QUEST_FAILED );
        data << quest;
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_FAILED");
    }
}

void Player::SendCanTakeQuestResponse( uint32 msg )
{
    WorldPacket data;
    data.Initialize( SMSG_QUESTGIVER_QUEST_INVALID );
    data << msg;
    GetSession()->SendPacket( &data );
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void Player::SendPushToPartyResponse( Player *pPlayer, uint32 msg )
{
    if( pPlayer )
    {
        WorldPacket data;
        data.Initialize( MSG_QUEST_PUSH_RESULT );
        data << pPlayer->GetGUID();
        data << msg;
        data << uint8(0);
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent MSG_QUEST_PUSH_RESULT");
    }
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/
bool Player::LoadFromDB( uint32 guid )
{

    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`realm`,`account`,`data`,`name`,`race`,`class`,`position_x`,`position_y`,`position_z`,`map`,`orientation`,`taximask`,`online`,`honor`,`last_week_honor`,`cinematic` FROM `character` WHERE `guid` = '%lu';",(unsigned long)guid);

    if(!result)
        return false;

    Field *fields = result->Fetch();

    Object::_Create( guid, HIGHGUID_PLAYER );

    LoadValues( fields[3].GetString() );

    m_name = fields[4].GetString();
    sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetUInt32Value(UNIT_FIELD_MAXHEALTH), GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    sLog.outDebug("AGILITY is: \t\t%u\t\tSTRENGHT is: \t\t%u",GetUInt32Value(UNIT_FIELD_AGILITY), GetUInt32Value(UNIT_FIELD_STR));
    sLog.outDebug("INTELLECT is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_IQ), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_STAMINA), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%u",GetUInt32Value(UNIT_FIELD_ARMOR), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_01), GetUInt32Value(UNIT_FIELD_RESISTANCES_02));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_03), GetUInt32Value(UNIT_FIELD_RESISTANCES_04));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_05), GetUInt32Value(UNIT_FIELD_RESISTANCES_06));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetUInt32Value(UNIT_FIELD_BASEATTACKTIME), GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1));

    m_race = fields[5].GetUInt8();
    //Need to call it to initialize m_team (m_team can be calculated from m_race)
    //Other way is to saves m_team into characters table.
    setFaction(m_race, 0);

    m_class = fields[6].GetUInt8();

    info = objmgr.GetPlayerCreateInfo(m_race, m_class);

    m_positionX = fields[7].GetFloat();
    m_positionY = fields[8].GetFloat();
    m_positionZ = fields[9].GetFloat();
    m_mapId = fields[10].GetUInt32();
    m_orientation = fields[11].GetFloat();
    m_highest_rank = fields[14].GetUInt32();
    m_last_week_rank = fields[15].GetUInt32();
    m_cinematic = fields[16].GetUInt32();

    if( HasFlag(PLAYER_FLAGS, 8) )
        SetUInt32Value(PLAYER_FLAGS, 0);

    if( HasFlag(PLAYER_FLAGS, 0x11) )
        m_deathState = DEAD;

    LoadTaxiMask( fields[12].GetString() );

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

    sLog.outDebug("The value of player %s after load item and aura is: ", m_name.c_str());
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetUInt32Value(UNIT_FIELD_MAXHEALTH), GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    sLog.outDebug("AGILITY is: \t\t%u\t\tSTRENGHT is: \t\t%u",GetUInt32Value(UNIT_FIELD_AGILITY), GetUInt32Value(UNIT_FIELD_STR));
    sLog.outDebug("INTELLECT is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_IQ), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_STAMINA), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%u",GetUInt32Value(UNIT_FIELD_ARMOR), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_01), GetUInt32Value(UNIT_FIELD_RESISTANCES_02));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_03), GetUInt32Value(UNIT_FIELD_RESISTANCES_04));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_05), GetUInt32Value(UNIT_FIELD_RESISTANCES_06));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetUInt32Value(UNIT_FIELD_BASEATTACKTIME), GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1));
    //_ApplyAllAuraMods();
    //_ApplyAllItemMods();
    return true;
}

void Player::_LoadActions()
{

    m_actions.clear();

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_action` WHERE `guid` = '%u' ORDER BY `button`;",GetGUIDLow());

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

void Player::_LoadAuras()
{
    m_Auras.clear();

    for(uint8 i = 0; i < 48; i++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURA + i), 0);
    for(uint8 j = 0; j < 6; j++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + j), 0);

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_aura` WHERE `guid` = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 spellid = fields[1].GetUInt32();
            uint32 effindex = fields[2].GetUInt32();
            int32 remaintime = (int32)fields[3].GetUInt32();

            SpellEntry* spellproto = sSpellStore.LookupEntry(spellid);
            assert(spellproto);

            Aura* aura = new Aura(spellproto, effindex, this, this);
            aura->SetAuraDuration(remaintime);
            AddAura(aura);
        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_LoadBids()
{

    m_bids.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `id`,`amount` FROM `auctionhouse_bid` WHERE `bidder` = '%u';",GetGUIDLow());

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

void Player::_LoadCorpse()
{
    // TODO do we need to load all corpses ?
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `game_corpse` WHERE `player` = '%u' AND `bones_flag` = '0';",GetGUIDLow());

    if(!result) return;

    Field *fields = result->Fetch();

    DeleteCorpse();
    m_pCorpse = new Corpse();

    float positionX = fields[2].GetFloat();
    float positionY = fields[3].GetFloat();
    float positionZ = fields[4].GetFloat();
    float ort       = fields[5].GetFloat();
    //uint32 zoneid   = fields[6].GetUInt32();
    uint32 mapid    = fields[7].GetUInt32();

    m_pCorpse->Relocate(positionX,positionY,positionZ,ort);
    m_pCorpse->SetMapId(mapid);
    //m_pCorpse->SetZoneId(zoneid);
    m_pCorpse->LoadValues( fields[8].GetString() );

    MapManager::Instance().GetMap(m_pCorpse->GetMapId())->Add(m_pCorpse);

    delete result;
}

void Player::_LoadInventory()
{
    for(uint16 i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            delete m_items[i];
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + i*2), 0);
            m_items[i] = 0;
        }
    }

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '0';",GetGUIDLow());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint8  slot      = fields[2].GetUInt8();
            uint32 item_guid = fields[3].GetUInt32();
            uint32 item_id   = fields[4].GetUInt32();

            ItemPrototype* proto = objmgr.GetItemPrototype(item_id);

            Item *item = NewItemOrBag(proto);
            item->SetOwner(this);
            item->SetSlot(slot);
            if(!item->LoadFromDB(item_guid, 1))
                continue;
            AddItem(0, slot, item, true);
        } while (result->NextRow());

        delete result;
    }
}

void Player::_LoadMail()
{

    m_mail.clear();

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `mail` WHERE `receiver` = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            Mail *be = new Mail;
            be->messageID = fields[0].GetUInt32();
            be->sender = fields[1].GetUInt32();
            be->receiver = fields[2].GetUInt32();
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

void Player::_LoadQuestStatus()
{
    mQuestStatus.clear();

    Quest *pQuest;
    uint32 quest;

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_queststatus` WHERE `playerid` = '%u';", GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            pQuest = objmgr.GetQuest(fields[1].GetUInt32());
            quest = pQuest->GetQuestInfo()->QuestId;

            mQuestStatus[quest].m_quest = pQuest;
            mQuestStatus[quest].status = fields[2].GetUInt32();
            mQuestStatus[quest].rewarded            = ( fields[3].GetUInt32() > 0 );
            mQuestStatus[quest].m_questMobCount[0]  = fields[4].GetUInt32();
            mQuestStatus[quest].m_questMobCount[1]  = fields[5].GetUInt32();
            mQuestStatus[quest].m_questMobCount[2]  = fields[6].GetUInt32();
            mQuestStatus[quest].m_questMobCount[3]  = fields[7].GetUInt32();
            mQuestStatus[quest].m_questItemCount[0] = fields[8].GetUInt32();
            mQuestStatus[quest].m_questItemCount[1] = fields[9].GetUInt32();
            mQuestStatus[quest].m_questItemCount[2] = fields[10].GetUInt32();
            mQuestStatus[quest].m_questItemCount[3] = fields[11].GetUInt32();
            mQuestStatus[quest].m_timerrel          = fields[12].GetUInt32();
            mQuestStatus[quest].m_explored          = ( fields[13].GetUInt32() > 0 );

            time_t q_abs = time(NULL);

            if (pQuest && (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED)) )
            {
                sLog.outDebug("Time now {%u}, then {%u} in quest {%u}!", q_abs, mQuestStatus[quest].m_timerrel, quest);
                if  ( mQuestStatus[quest].m_timerrel > q_abs )
                {
                    mQuestStatus[quest].m_timer = (mQuestStatus[quest].m_timerrel - q_abs) * 1000;
                    sLog.outDebug("Setup timer at {%u}msec. for quest {%u}!", mQuestStatus[quest].m_timer, quest);
                    m_timedquest = quest;
                    continue;
                }
                else
                {
                    sLog.outDebug("Timer expired for quest {%u}!", quest);
                    mQuestStatus[quest].m_timer    = 0;

                    if ( mQuestStatus[quest].status == QUEST_STATUS_COMPLETE )
                        mQuestStatus[quest].status = QUEST_STATUS_INCOMPLETE;

                    mQuestStatus[quest].m_timerrel = 0;
                }
            }

            sLog.outDebug("Quest status is {%u} for quest {%u}", mQuestStatus[quest].status, quest);

        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_LoadReputation()
{
    Factions newFaction;

    factions.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `faction`,`reputation`,`standing`,`flags` FROM `character_reputation` WHERE `guid` = '%u';",GetGUIDLow());

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

void Player::_LoadSpells()
{

    m_spells.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `spell`,`slot` FROM `character_spell` WHERE `guid` = '%u';",GetGUIDLow());

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
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_tutorial` WHERE `guid` = '%u';",GetGUIDLow());

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

/*********************************************************/
/***                   SAVE SYSTEM                     ***/
/*********************************************************/
void Player::SaveToDB()
{
    if (hasUnitState(UNIT_STAT_IN_FLIGHT))
    {
        SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
        RemoveFlag( UNIT_FIELD_FLAGS ,0x000004 );
        RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );
    }

    // Set player sit state to standing on save
    RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
    RemoveFlag(UNIT_FIELD_FLAGS, 0x40000);

    //remove restflag when save
    //this is becouse of the rename char stuff
    RemoveFlag(PLAYER_FLAGS, 0x20);

    sLog.outDebug("The value of player %s after load item and aura is: ", m_name.c_str());
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetUInt32Value(UNIT_FIELD_MAXHEALTH), GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    sLog.outDebug("AGILITY is: \t\t%u\t\tSTRENGHT is: \t\t%u",GetUInt32Value(UNIT_FIELD_AGILITY), GetUInt32Value(UNIT_FIELD_STR));
    sLog.outDebug("INTELLECT is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_IQ), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_STAMINA), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%u",GetUInt32Value(UNIT_FIELD_ARMOR), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_01), GetUInt32Value(UNIT_FIELD_RESISTANCES_02));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_03), GetUInt32Value(UNIT_FIELD_RESISTANCES_04));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_05), GetUInt32Value(UNIT_FIELD_RESISTANCES_06));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetUInt32Value(UNIT_FIELD_BASEATTACKTIME), GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1));
    //_RemoveStatsMods();
    _RemoveAllItemMods();
    _RemoveAllAuraMods();

    bool inworld = IsInWorld();
    if (inworld)
        RemoveFromWorld();

    std::stringstream ss;

    sDatabase.PExecute("DELETE FROM `character` WHERE `guid` = '%u'",GetGUIDLow());

    ss.rdbuf()->str("");
    ss << "INSERT INTO `character` (`guid`,`account`,`name`,`race`,`class`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`data`,`taximask`,`online`,`honor`,`last_week_honor`,`cinematic`) VALUES ("
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
    {
        ss << GetUInt32Value(i) << " ";
    }

    ss << "', '";

    for( i = 0; i < 8; i++ )
        ss << m_taximask[i] << " ";

    ss << "', ";
    inworld ? ss << 1: ss << 0;

    ss << ", ";
    ss << m_highest_rank;

    ss << ", ";
    ss << m_last_week_rank;

    ss << ", ";
    ss << m_cinematic;

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

    if(m_pCorpse) m_pCorpse->SaveToDB(false);

    uint64 petguid;
    if((petguid=GetUInt64Value(UNIT_FIELD_SUMMON)) != 0)
    {
        Creature *OldSummon = ObjectAccessor::Instance().GetCreature(*this, petguid);
        if(OldSummon && OldSummon->isPet())
        {
            ((Pet*)OldSummon)->SavePetToDB();
        }
    }

    sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetUInt32Value(UNIT_FIELD_MAXHEALTH), GetUInt32Value(UNIT_FIELD_MAXPOWER1));
    sLog.outDebug("AGILITY is: \t\t%u\t\tSTRENGHT is: \t\t%u",GetUInt32Value(UNIT_FIELD_AGILITY), GetUInt32Value(UNIT_FIELD_STR));
    sLog.outDebug("INTELLECT is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_IQ), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%u\t\tSPIRIT is: \t\t%u",GetUInt32Value(UNIT_FIELD_STAMINA), GetUInt32Value(UNIT_FIELD_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%u",GetUInt32Value(UNIT_FIELD_ARMOR), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_01), GetUInt32Value(UNIT_FIELD_RESISTANCES_02));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_03), GetUInt32Value(UNIT_FIELD_RESISTANCES_04));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetUInt32Value(UNIT_FIELD_RESISTANCES_05), GetUInt32Value(UNIT_FIELD_RESISTANCES_06));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetUInt32Value(UNIT_FIELD_BASEATTACKTIME), GetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1));
    _ApplyAllAuraMods();
    _ApplyAllItemMods();
    //_ApplyStatsMods();
    //_ApplyStatsMods(); //debug wkjhsadfjkhasdl;fh

    if (inworld)
        AddToWorld();
}

void Player::_SaveActions()
{
    std::stringstream query;
    sDatabase.PExecute("DELETE FROM `character_action` WHERE `guid` = '%u'",GetGUIDLow());

    std::list<struct actions>::iterator itr;
    for (itr = m_actions.begin(); itr != m_actions.end(); ++itr)
    {
        sDatabase.PExecute("INSERT INTO `character_action` (`guid`,`button`,`action`,`type`,`misc`) VALUES ('%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), (uint32)itr->button, (uint32)itr->action, (uint32)itr->type, (uint32)itr->misc);
    }
}

void Player::_SaveAuctions()
{
    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `itemowner` = '%u'",GetGUIDLow());

    ObjectMgr::AuctionEntryMap::iterator itr;
    for (itr = objmgr.GetAuctionsBegin();itr != objmgr.GetAuctionsEnd();itr++)
    {
        AuctionEntry *Aentry = itr->second;
        if ((Aentry) && (Aentry->owner == GetGUIDLow()))
        {
            Item *it = objmgr.GetAItem(Aentry->item);

            sDatabase.PExecute("DELETE FROM `auctionhouse_item` WHERE `guid` = '%u'",it->GetGUIDLow());
            sDatabase.PExecute("INSERT INTO `auctionhouse` (`auctioneerguid`,`itemguid`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`id`) VALUES ('%u', '%u', '%u', '%u', '%d', '%u', '%u', '%u');", Aentry->auctioneer, Aentry->item, Aentry->owner, Aentry->buyout, Aentry->time, Aentry->bidder, Aentry->bid, Aentry->Id);

            std::stringstream ss;
            ss << "INSERT INTO `auctionhouse_item` (`guid`,`data`) VALUES ("
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

void Player::_SaveAuras()
{
    sDatabase.PExecute("DELETE FROM `character_aura` WHERE `guid` = '%u'",GetGUIDLow());

    AuraList auras = GetAuras();
    AuraList::iterator itr;
    for (itr = auras.begin(); itr != auras.end(); ++itr)
    {
        sDatabase.PExecute("INSERT INTO `character_aura` (`guid`,`spell`,`effect_index`,`remaintime`) VALUES ('%u', '%u', '%u', '%d');", GetGUIDLow(), (uint32)(*itr)->GetId(), (uint32)(*itr)->GetEffIndex(), int((*itr)->GetAuraDuration()));
    }
}

void Player::_SaveBids()
{
    sDatabase.PExecute("DELETE FROM `auctionhouse_bid` WHERE `bidder` = '%u'",GetGUIDLow());

    std::list<bidentry*>::iterator itr;
    for (itr = m_bids.begin(); itr != m_bids.end(); itr++)
    {
        AuctionEntry *a = objmgr.GetAuction((*itr)->AuctionID);
        if (a)
        {
            sDatabase.PExecute("INSERT INTO `auctionhouse_bid` (`bidder`,`id`,`amount`) VALUES ('%u', '%u', '%u');", GetGUIDLow(), (*itr)->AuctionID, (*itr)->amt);
        }
    }

}

void Player::_SaveInventory()
{
    sDatabase.PExecute("DELETE FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '0';",GetGUIDLow());

    for(uint8 i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if (m_items[i] != 0)
        {
            sDatabase.PExecute("INSERT INTO `character_inventory` (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', 0, '%u', '%u', '%u');", GetGUIDLow(), i, m_items[i]->GetGUIDLow(), m_items[i]->GetEntry());
            m_items[i]->SaveToDB();
        }
    }
}

void Player::_SaveMail()
{

    sDatabase.PExecute("DELETE FROM `mail` WHERE `receiver` = '%u'",GetGUIDLow());

    std::list<Mail*>::iterator itr;
    for (itr = m_mail.begin(); itr != m_mail.end(); itr++)
    {
        Mail *m = (*itr);

        QueryResult *result = sDatabase.PQuery("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->receiver, m->subject.c_str(), m->body.c_str(), m->item,  m->time, m->money, m->COD, m->checked);
        delete result;
    }
}

void Player::_SaveQuestStatus()
{
    sDatabase.PExecute("DELETE FROM `character_queststatus` WHERE `playerid` = '%u'",GetGUIDLow());

    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        sDatabase.PExecute("INSERT INTO `character_queststatus` (`playerid`,`questid`,`status`,`rewarded`,`questMobCount1`,`questMobCount2`,`questMobCount3`,`questMobCount4`,`questItemCount1`,`questItemCount2`,`questItemCount3`,`questItemCount4`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u','%u');", GetGUIDLow(), i->first, i->second.status, i->second.rewarded, i->second.m_questMobCount[0], i->second.m_questMobCount[1], i->second.m_questMobCount[2], i->second.m_questMobCount[3], i->second.m_questItemCount[0], i->second.m_questItemCount[1], i->second.m_questItemCount[2], i->second.m_questItemCount[3]);
    }
}

void Player::_SaveReputation()
{
    std::list<Factions>::iterator itr;

    std::stringstream ss;

    sDatabase.PExecute("DELETE FROM `character_reputation` WHERE `guid` = '%u'",GetGUIDLow());

    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {

        sDatabase.PExecute("INSERT INTO `character_reputation` (`guid`,`faction`,`reputation`,`standing`,`flags`) VALUES ('%u', '%u', '%u', '%u', '%u');", (uint32)GetGUIDLow(), itr->ID, itr->ReputationListID, itr->Standing, itr->Flags);

    }
}

void Player::_SaveSpells()
{
    sDatabase.PExecute("DELETE FROM `character_spell` WHERE `guid` = '%u'",GetGUIDLow());

    std::list<Playerspell*>::iterator itr;
    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        sDatabase.PQuery("INSERT INTO `character_spell` (`guid`,`spell`,`slot`) VALUES ('%u', '%u', '%u');", GetGUIDLow(), (*itr)->spellId, (*itr)->slotId);
    }
}

void Player::_SaveTutorials()
{
    sDatabase.PExecute("DELETE FROM `character_tutorial` WHERE `guid` = '%u'",GetGUIDLow());
    sDatabase.PExecute("INSERT INTO `character_tutorial` (`guid`,`tut0`,`tut1`,`tut2`,`tut3`,`tut4`,`tut5`,`tut6`,`tut7`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
}
