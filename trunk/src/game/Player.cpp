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
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Group.h"
#include "Formulas.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "Util.h"

#include <cmath>

Player::Player (WorldSession *session): Unit()
{
    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    m_session = session;

    info = NULL;
    m_divider = 0;
    m_timedquest = 0;

    m_afk = 0;
    m_acceptTicket = true;
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
    m_resurrectGUID = 0;
    m_resurrectX = m_resurrectY = m_resurrectZ = 0;
    m_resurrectHealth = m_resurrectMana = 0;

    memset(m_items, 0, sizeof(Item*)*BUYBACK_SLOT_END);
    memset(m_buybackitems, 0, sizeof(Item*)*(BUYBACK_SLOT_END - BUYBACK_SLOT_START));

    m_pDuel       = NULL;
    m_pDuelSender = NULL;
    m_isInDuel = false;

    m_GuildIdInvited = 0;

    m_groupLeader = 0;
    m_isInGroup = false;
    m_isInvited = false;

    m_dontMove = false;

    m_total_honor_points = 0;

    pTrader = NULL;

    m_cinematic = 0;

    PlayerTalkClass = new PlayerMenu( GetSession() );
    m_currentBuybackSlot = BUYBACK_SLOT_START;

    for ( int aX = 0 ; aX < 8 ; aX++ )
        m_Tutorials[ aX ] = 0x00;
    ItemsSetEff[0]=NULL;
    ItemsSetEff[1]=NULL;
    ItemsSetEff[2]=NULL;
    m_regenTimer = 0;
    m_breathTimer = 0;
    m_isunderwater = 0;
    m_drunkTimer = 0;
    m_drunk = 0;
    m_restTime = 0;
    m_lastManaUse = 0;
    m_deathTimer = 0;

    m_pvp_count = 0;
    m_pvp_counting = false;

    m_bgInBattleGround = false;
    m_bgBattleGroundID = 0;

    m_movement_flags = 0;

    m_BlockValue = 0;

    m_logintime = time(NULL);
    m_Last_tick = m_logintime;
}

Player::~Player ()
{
    uint32 eslot;
    for(int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
    {
        eslot = j - BUYBACK_SLOT_START;
        if(m_buybackitems[eslot])
        {
            m_buybackitems[eslot]->DeleteFromDB();
            m_buybackitems[eslot]->RemoveFromWorld();
            delete m_buybackitems[eslot];
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

    uint32 eslot;
    for(int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
    {
        eslot = j - BUYBACK_SLOT_START;
        m_buybackitems[eslot] = NULL;
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
        case WARRIOR: powertype = 1; unitfield = 0x1100EE00; break;
        case PALADIN: powertype = 0; unitfield = 0x0000EE00; break;
        case HUNTER: powertype = 0; unitfield = 0x0000EE00; break;
        case ROGUE: powertype = 3; unitfield = 0x00000000; break;
        case PRIEST: powertype = 0; unitfield = 0x0000EE00; break;
        case SHAMAN: powertype = 0; unitfield = 0x0000EE00; break;
        case MAGE: powertype = 0; unitfield = 0x0000EE00; break;
        case WARLOCK: powertype = 0; unitfield = 0x0000EE00; break;
        case DRUID: powertype = 0; unitfield = 0x0000EE00; break;
    }

    if ( race == TAUREN )
    {
        SetFloatValue(OBJECT_FIELD_SCALE_X, 1.35f);
    }
    else SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    SetStat(STAT_STRENGTH,info->strength );
    SetStat(STAT_AGILITY,info->agility );
    SetStat(STAT_STAMINA,info->stamina );
    SetStat(STAT_INTELLECT,info->intellect );
    SetStat(STAT_SPIRIT,info->spirit );
    SetArmor(info->basearmor );
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, info->attackpower );

    SetHealth(info->health);
    SetMaxHealth(info->health);

    SetPower(   POWER_MANA, info->mana );
    SetMaxPower(POWER_MANA, info->mana );
    SetPower(   POWER_RAGE, 0 );
    SetMaxPower(POWER_RAGE, info->rage );
    SetPower(   POWER_FOCUS, info->focus );
    SetMaxPower(POWER_FOCUS, info->focus );
    SetPower(   POWER_ENERGY, info->energy );
    SetMaxPower(POWER_ENERGY, info->energy );

    SetFloatValue(UNIT_FIELD_MINDAMAGE, info->mindmg );
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, info->maxdmg );
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, info->ranmindmg );
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, info->ranmaxdmg );

    SetAttackTime(BASE_ATTACK,   2000 );                    // melee attack time
    SetAttackTime(RANGED_ATTACK, 2000 );                    // ranged attack time

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f   );

    SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId + gender );
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId + gender );

    SetLevel( 1 );

    setFactionForRace(m_race);

    SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( race ) | ( class_ << 8 ) | ( gender << 16 ) | ( powertype << 24 ) ) );
    SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield );
    SetUInt32Value(UNIT_FIELD_BYTES_2, 0xEEEEEE00 );
    SetUInt32Value(UNIT_FIELD_FLAGS , UNIT_FLAG_NONE | UNIT_FLAG_NOT_IN_PVP );

    SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0x10);
                                                            //-1 is default value
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

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

    // Played time
    m_Last_tick = time(NULL);
    m_Played_time[0] = 0;
    m_Played_time[1] = 0;

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
    spell_itr = info->spell.begin();

    for (; spell_itr!=info->spell.end(); spell_itr++)
    {
        tspell = (*spell_itr);
        if (tspell)
        {
            sLog.outDebug("PLAYER: Adding initial spell, id = %u",tspell);
            addSpell(tspell,1);
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

    m_rating = 0;
    m_highest_rank = 0;
    m_standing = 0;

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
        SetMaxHealth( uint32(GetMaxHealth() * 1.05));       // only integer part
    }

    // school resistances
    if (Player::HasSpell(20596))
    {
        SetResistance(SPELL_SCHOOL_FROST, 10 );
    }
    if (Player::HasSpell(20583))
    {
        SetResistance(SPELL_SCHOOL_NATURE, 10 );
    }
    if (Player::HasSpell(20579))
    {
        SetResistance(SPELL_SCHOOL_SHADOW, 10 );
    }
    if (Player::HasSpell(20592))
    {
        SetResistance(SPELL_SCHOOL_ARCANE, 10 );
    }

    // initilize potential block chance (used if item with Block value equiped)
    SetFloatValue(PLAYER_BLOCK_PERCENTAGE, 5 + (float(GetDefenceSkillValue()) - getLevel()*5)*0.04);

    // apply original stats mods before item equipment that call before equip _RemoveStatsMods()
    _ApplyStatsMods();

    uint16 dest;
    uint8 msg;
    Item *pItem;
    for (; item_id_itr!=info->item_id.end(); item_id_itr++, item_bagIndex_itr++, item_slot_itr++, item_amount_itr++)
    {
        titem_id = (*item_id_itr);
        titem_bagIndex = (*item_bagIndex_itr);
        titem_slot = (*item_slot_itr);
        titem_amount = (*item_amount_itr);

        if (titem_id)
        {
            sLog.outDebug("STORAGE: Creating initial item, itemId = %u, bagIndex = %u, slot = %u, count = %u",titem_id, titem_bagIndex, titem_slot, titem_amount);

            pItem = CreateItem( titem_id, titem_amount);
            if( pItem )
            {
                dest = ((titem_bagIndex << 8) | titem_slot);
                if( IsInventoryPos( dest ) )
                {
                    msg = CanStoreItem( titem_bagIndex, titem_slot, dest, pItem, false );
                    if( msg == EQUIP_ERR_OK )
                        StoreItem( dest, pItem, true);
                    else
                    {
                        sLog.outDebug("STORAGE: Can't store item, error msg = %u",msg);
                        delete pItem;
                    }
                }
                else if( IsEquipmentPos( dest ) )
                {
                    msg = CanEquipItem( titem_slot, dest, pItem, false );
                    if( msg == EQUIP_ERR_OK )
                        EquipItem( dest, pItem, true);
                    else
                    {
                        sLog.outDebug("STORAGE: Can't equip item, error msg = %u",msg);
                        delete pItem;
                    }
                }
                else if( IsBankPos( dest ) )
                {
                    msg = CanBankItem( titem_bagIndex, titem_slot, dest, pItem, false );
                    if( msg == EQUIP_ERR_OK )
                        BankItem( dest, pItem, true);
                    else
                    {
                        sLog.outDebug("STORAGE: Can't bank item, error msg = %u",msg);
                        delete pItem;
                    }
                }
                else
                    delete pItem;
            }
        }
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
    GetSession()->SendPacket(&data);
}

void Player::ModifyMirrorTimer(uint8 Type, uint32 MaxValue, uint32 CurrentValue, uint32 Regen)
{
    //TYPE: 0 = fatigue 1 = breath 2 = fire
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
            uint32 damage = (GetMaxHealth() / 5) + rand()%getLevel();

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
            uint32 damage = (GetMaxHealth() / 3) + rand()%getLevel();

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

void Player::HandleSobering()
{
    m_drunkTimer = 0;
    if (m_drunk <= (0xFFFF / 30))
    {
        m_drunk = 0;
    }
    else
    {
        m_drunk -= (0xFFFF / 30);
    }
    SetUInt32Value(PLAYER_BYTES_3, (GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0000) | m_drunk);
}

void Player::SetDrunkValue(uint16 newDrunkValue)
{
    m_drunk = newDrunkValue;
    SetUInt32Value(PLAYER_BYTES_3,
        (GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0000) | m_drunk);
}

void Player::Update( uint32 p_time )
{
    if(!IsInWorld())
        return;

    WorldPacket data;

    Unit::Update( p_time );

    time_t now = time (NULL);

    UpdatePVPFlag(time(NULL));

    CheckDuelDistance();

    CheckExploreSystem();

    Quest *pQuest;
    if( GetTimedQuest() != 0 )
    {
        pQuest = objmgr.GetQuest( GetTimedQuest() );
        if ( pQuest )
        {
            if( mQuestStatus[GetTimedQuest()].m_timer > 0 )
            {
                if( mQuestStatus[GetTimedQuest()].m_timer <= p_time )
                {
                    FailTimedQuest( pQuest );
                }
                else
                {
                    mQuestStatus[GetTimedQuest()].m_timer -= p_time;
                }
            }
        }
    }

    if (isAttacking())
    {
        if (isAttackReady() && m_currentSpell == 0 )
        {
            Unit *pVictim = getVictim();
            //if(!pVictim) {
            //    Attack((Unit *)ObjectAccessor::Instance().FindPlayer(m_curSelection));
            //    Unit *pVictim = getVictim();
            //}

            // default combat reach 10
            // TODO add weapon,skill check

            float pldistance = 10.0f;
            if(getClass() == WARRIOR)
            {
                pldistance += 1;
            }
            if(GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND ) != 0)
            {
                pldistance += 2;
            }
            if(GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS) != 0)
            {
                pldistance += 3;
            }

            if (pVictim)
            {
                if( GetDistanceSq(pVictim) > pldistance )
                {
                    setAttackTimer(uint32(1000));
                    SendAttackSwingNotInRange();
                }
                //120 degreas of radiant range
                //(120/360)*(2*PI) = 2,094395102/2 = 1,047197551    //1,57079633-1,047197551   //1,57079633+1,047197551
                else if( !HasInArc( 2.0943951024, pVictim ))
                {
                    setAttackTimer(uint32(1000));
                    SendAttackSwingBadFacingAttack();
                }
                else
                {
                    setAttackTimer(0);
                    AttackerStateUpdate(pVictim);
                }
            }
        }
    }
    else if (isAttacked())
    {
        // Leave here so we don't forget this case
        // Attacked, but not necessarily attacking
    }
    else
    {
        // Leave here so we don't forget this case
        // Not attacking or attacked
    }

    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        m_restTime ++;

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
        if( isInDuel() )
        {
            DuelComplete();
        }
        else
        {
            KillPlayer();
        }
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

    // Played time
    if (now > m_Last_tick)
    {
        uint32 elapsed = (now - m_Last_tick);
        m_Played_time[0] += elapsed;                        // Total played time
        m_Played_time[1] += elapsed;                        // Level played time
        m_Last_tick = now;
    }

    if (m_drunk)
    {
        m_drunkTimer += p_time;

        if (m_drunkTimer > 30000)
            HandleSobering();
    }

    if(m_deathTimer > 0)
    {
        if(p_time >= m_deathTimer)
        {
            m_deathTimer = 0;
            BuildPlayerRepop();
            RepopAtGraveyard();
        }
        else
            m_deathTimer -= p_time;
    }
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

    *p_data << uint8(getLevel());                           //1
    uint32 zoneId = MapManager::Instance ().GetMap(m_mapId)->GetZoneId(m_positionX,m_positionY);

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

void Player::TeleportTo(uint32 mapid, float x, float y, float z, float orientation)
{
    AttackStop();
    RemoveAllAttackers();

    if(this->GetMapId() == mapid)
    {
        // near teleport
        WorldPacket data;
        BuildTeleportAckMsg(&data, x, y, z, orientation);
        GetSession()->SendPacket(&data);
        SetPosition( x, y, z, orientation );
        BuildHeartBeatMsg(&data);
        SendMessageToSet(&data, true);
    }
    else
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

        // Resend spell list to client after far teleport.
        SendInitialSpells();
    }
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

    uint32 maxRage = GetMaxPower(POWER_RAGE);
    uint32 Rage = GetPower(POWER_RAGE);

    if(attacker)
        Rage += (uint32)(10*damage/(getLevel()*0.5f));
    else
        Rage += (uint32)(10*damage/(getLevel()*1.5f));

    if(Rage > maxRage)  Rage = maxRage;

    SetPower(POWER_RAGE, Rage);
}

void Player::RegenerateAll()
{

    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;

    // Not in combat or they have regeneration
    // TODO: Replace the 20555 with test for if they have an aura of regeneration
    if (!isInCombat() || Player::HasSpell(20555))
    {
        RegenerateHealth();
        if (!isInCombat())
            Regenerate(POWER_RAGE);
    }

    Regenerate( POWER_ENERGY );
    Regenerate( POWER_MANA );

    m_regenTimer = regenDelay;

}

void Player::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    if(power != POWER_RAGE)
    {
        if (curValue >= maxValue)   return;
    }
    else if (curValue == 0)
        return;

    float ManaIncreaseRate = sWorld.getRate(RATE_POWER_MANA);
    float RageIncreaseRate = sWorld.getRate(RATE_POWER_RAGE);

    uint16 Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;
    if( RageIncreaseRate <= 0 ) RageIncreaseRate = 1;

    uint32 addvalue = 0;

    switch (power)
    {
        case POWER_MANA:
            // If < 5s after previous cast which used mana, no regeneration unless
            // we happen to have a modifer that adds it back
            // If > 5s, get portion between the 5s and now, up to a maximum of 2s worth
            uint32 msecSinceLastCast;
            msecSinceLastCast = ((uint32)getMSTime() - m_lastManaUse);
            if (msecSinceLastCast >= 7000)
            {
                ManaIncreaseRate *= 1;
            }
            else
            {
                long regenInterrupt = GetTotalAuraModifier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
                if (msecSinceLastCast < 5000)
                {
                    ManaIncreaseRate *= (float)regenInterrupt / 100;
                }
                else
                {
                    ManaIncreaseRate =  (((1 - (float)(msecSinceLastCast - 5000)/2000)) * regenInterrupt)
                        + (((float)(msecSinceLastCast - 5000)/2000) * ManaIncreaseRate * 100);
                    ManaIncreaseRate /= 100;
                }
            }
            ManaIncreaseRate = (ManaIncreaseRate * 100 + GetTotalAuraModifier(SPELL_AURA_MOD_POWER_REGEN_PERCENT)) / 100;

            switch (Class)
            {
                case DRUID:   addvalue = uint32((Spirit/5 + 15) * ManaIncreaseRate); break;
                case HUNTER:  addvalue = uint32((Spirit/5 + 15) * ManaIncreaseRate); break;
                case MAGE:    addvalue = uint32((Spirit/4 + 12.5) * ManaIncreaseRate); break;
                case PALADIN: addvalue = uint32((Spirit/5 + 15)  * ManaIncreaseRate); break;
                case PRIEST:  addvalue = uint32((Spirit/4 + 12.5) * ManaIncreaseRate); break;
                case SHAMAN:  addvalue = uint32((Spirit/5 + 17) * ManaIncreaseRate); break;
                case WARLOCK: addvalue = uint32((Spirit/5 + 15)  * ManaIncreaseRate); break;
            }
            break;
        case POWER_RAGE:                                    // Regenerate rage
            addvalue = uint32(1.66 * RageIncreaseRate);
            break;
        case POWER_ENERGY:                                  // Regenerate energy (rogue)
            addvalue = uint32(20);
            break;
    }

    if (power != POWER_RAGE)
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
    }
    else
    {
        if(curValue <= addvalue)
            curValue = 0;
        else
            curValue -= addvalue;
    }
    SetPower(power, curValue);
}

void Player::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH);

    uint16 Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;

    uint32 addvalue = 0;

    switch (Class)
    {
        case DRUID:   addvalue = uint32((Spirit*0.09 + 6.5) * HealthIncreaseRate); break;
        case HUNTER:  addvalue = uint32((Spirit*0.25) * HealthIncreaseRate); break;
        case MAGE:    addvalue = uint32((Spirit*0.10) * HealthIncreaseRate); break;
        case PALADIN: addvalue = uint32((Spirit*0.25) * HealthIncreaseRate); break;
        case PRIEST:  addvalue = uint32((Spirit*0.10) * HealthIncreaseRate); break;
        case ROGUE:   addvalue = uint32((Spirit*0.50 + 2.0) * HealthIncreaseRate); break;
        case SHAMAN:  addvalue = uint32((Spirit*0.11) * HealthIncreaseRate); break;
        case WARLOCK: addvalue = uint32((Spirit*0.07 + 6.0) * HealthIncreaseRate); break;
        case WARRIOR: addvalue = uint32((Spirit*0.80) * HealthIncreaseRate); break;
    }
    if (HasSpell(20555))                                    // TODO: Should be aura controlled
    {
        if (isInCombat())
            addvalue*=uint32(0.10);
        else
            addvalue*=uint32(1.10);
    }

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
    SetHealth(curValue);
}

bool Player::isAcceptTickets() const
{
    return GetSession()->GetSecurity() >=2 && m_acceptTicket;
}

void Player::SendLogXPGain(uint32 GivenXP, Unit* victim)
{
    WorldPacket data;
    data.Initialize( SMSG_LOG_XPGAIN );
    data << ( victim ? victim->GetGUID() : uint64(0) );
    data << GivenXP;                                        // given experience
    data << ( victim ? (uint8)0 : (uint8)1 );               // 00-kill_xp type, 01-non_kill_xp type

    if (isRested())
        data << GivenXP;                                    // rested given experience
    else
        data << (GivenXP/2);                                // unrested given experience

    data << float(1);                                       //still a unknown static
    GetSession()->SendPacket(&data);
}

void Player::GiveXP(uint32 xp, Unit* victim)
{
    if ( xp < 1 )
        return;

    uint32 level = getLevel();

    // XP to money conversion processed in Player::RewardQuest
    if(level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    SendLogXPGain(xp,victim);

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp;

    while( newXP >= nextLvlXP && level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
    {
        newXP -= nextLvlXP;

        GiveLevel();

        level = getLevel();
        nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    }

    SetUInt32Value(PLAYER_XP, newXP);
}

// Update player to next level
// Current player expirience not update (must be update by caller)
void Player::GiveLevel()
{
    uint32 MPGain,HPGain,STRGain,STAGain,AGIGain,INTGain,SPIGain;
    MPGain=HPGain=STRGain=STAGain=AGIGain=INTGain=SPIGain=0;

    uint32 level = getLevel();

    level += 1;

    // Remove item, aura, stats bonuses
    _RemoveAllItemMods();
    _RemoveAllAuraMods();
    _RemoveStatsMods();

    // base stats
    float newMP  = (getClass() == WARRIOR || getClass() == ROGUE) ? 0 : GetMaxPower(POWER_MANA);

    float newHP  = GetMaxHealth();
    float newSTR = GetStat(STAT_STRENGTH);
    float newSTA = GetStat(STAT_STAMINA);
    float newAGI = GetStat(STAT_AGILITY);
    float newINT = GetStat(STAT_INTELLECT);
    float newSPI = GetStat(STAT_SPIRIT);

    // Remove class and race bonuses from base stats
    if (Player::HasSpell(20550))                            //endurance skill support (+5% to total health)
        newHP = newHP / 1.05;

    if (Player::HasSpell(20598))                            //Human Spirit skill support (+5% to total spirit)
        newSPI = newSPI / 1.05;

    if (Player::HasSpell(20591))                            //Expansive mind support (+5% to total Intellect)
        newINT  = newINT / 1.05;

    // Gain stats
    MPGain = (getClass() == WARRIOR || getClass() == ROGUE) ? 0 : uint32(newSPI / 2);
    HPGain = uint32(newSTA / 2);
    BuildLvlUpStats(&STRGain,&STAGain,&AGIGain,&INTGain,&SPIGain);

    // Apply gain stats
    newMP  += MPGain;
    newHP  += HPGain;
    newSTR += STRGain;
    newSTA += STAGain;
    newAGI += AGIGain;
    newINT += INTGain;
    newSPI += SPIGain;

    // Apply class and race bonuses to stats
    if (Player::HasSpell(20550))                            //endurance skill support (+5% to total health)
        newHP  = newHP * 1.05;

    if (Player::HasSpell(20598))                            //Human Spirit skill support (+5% to total spirit)
        newSPI  = newSPI * 1.05;

    if (Player::HasSpell(20591))                            //Expansive mind support (+5% to total Intellect)
        newINT = newINT * 1.05;

    // update level, talants, max level of skills
    SetLevel( level);
    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, MaNGOS::XP::xp_to_level(level));

    if( level > 9)
        SetUInt32Value(PLAYER_CHARACTER_POINTS1,GetUInt32Value(PLAYER_CHARACTER_POINTS1)+1);

    UpdateMaxSkills ();

    // save new stats
    if(getClass() != WARRIOR && getClass() != ROGUE)
    {
        SetPower(   POWER_MANA, uint32(newMP));             // only integer part
        SetMaxPower(POWER_MANA, uint32(newMP));             // only integer part
    }

    SetHealth(   uint32(newHP));                            // only integer part
    SetMaxHealth(uint32(newHP));                            // only integer part

    SetStat(STAT_STRENGTH, uint32(newSTR));                 // only integer part
    SetStat(STAT_STAMINA,  uint32(newSTA));                 // only integer part
    SetStat(STAT_AGILITY,  uint32(newAGI));                 // only integer part
    SetStat(STAT_INTELLECT,uint32(newINT));                 // only integer part
    SetStat(STAT_SPIRIT,   uint32(newSPI));                 // only integer part

    // update dependent from level part BlockChanceWithoutMods = 5 + (GetDefenceSkillValue() - getLevel()*5)*0.04);
    ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE, - 5*0.04,true);

    // apply stats, aura, items mods
    _ApplyStatsMods();
    _ApplyAllAuraMods();
    _ApplyAllItemMods();

    // send levelup info to client
    WorldPacket data;
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

    // Level Played Time reset
    m_Played_time[1] = 0;
}

void Player::BuildLvlUpStats(uint32 *STR,uint32 *STA,uint32 *AGI,uint32 *INT,uint32 *SPI)
{
    uint8 _class=getClass();
    uint8 lvl=getLevel();

    switch(_class)
    {
        case WARRIOR:
            *STR += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
            *STA += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
            *AGI += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
            *INT += (lvl > 9 && !(lvl%2) ? 1: 0);
            *SPI += (lvl > 9 && !(lvl%2) ? 1: 0);
            break;
        case PALADIN:
            *STR += (lvl > 3  ? 1: 0);
            *STA += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
            *AGI += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
            *INT += (lvl > 6 && (lvl%2) ? 1: 0);
            *SPI += (lvl > 7 ? 1: 0);
            break;
        case HUNTER:
            *STR += (lvl > 4  ? 1: 0);
            *STA += (lvl > 4  ? 1: 0);
            *AGI += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
            *INT += (lvl > 8 && (lvl%2) ? 1: 0);
            *SPI += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
            break;
        case ROGUE:
            *STR += (lvl > 5  ? 1: 0);
            *STA += (lvl > 4  ? 1: 0);
            *AGI += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
            *INT += (lvl > 8 && !(lvl%2) ? 1: 0);
            *SPI += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
            break;
        case PRIEST:
            *STR += (lvl > 9 && !(lvl%2) ? 1: 0);
            *STA += (lvl > 5  ? 1: 0);
            *AGI += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
            *INT += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
            *SPI += (lvl > 3  ? 1: 0);
            break;
        case SHAMAN:
            *STR += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
            *STA += (lvl > 4 ? 1: 0);
            *AGI += (lvl > 7 && !(lvl%2) ? 1: 0);
            *INT += (lvl > 5 ? 1: 0);
            *SPI += (lvl > 4 ? 1: 0);
            break;
        case MAGE:
            *STR += (lvl > 9 && !(lvl%2) ? 1: 0);
            *STA += (lvl > 5  ? 1: 0);
            *AGI += (lvl > 9 && !(lvl%2) ? 1: 0);
            *INT += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
            *SPI += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
            break;
        case WARLOCK:
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

void Player::SendInitialSpells()
{
    WorldPacket data;
    uint16 spellCount = 0;

    PlayerSpellList::const_iterator itr;

    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if((*itr)->active)
            spellCount +=1;
    }

    data.Initialize( SMSG_INITIAL_SPELLS );
    data << uint8(0);
    data << uint16(spellCount);

    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(!(*itr)->active)
            continue;
        data << uint16((*itr)->spellId);
        data << uint16((*itr)->slotId);
    }
    data << uint16(0);

    WPAssert(data.size() == 5+(4*size_t(spellCount)));

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

void Player::addSpell(uint16 spell_id, uint8 active, uint16 slot_id)
{
    uint32 newrank = 0;
    uint32 oldrank = 0;
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spell_id);
    if(!spellInfo) return;
    newrank = FindSpellRank(spell_id);

    if(spellInfo->Effect[0] == 60)
        GetSession()->SendProficiency(spellInfo->EquippedItemClass,spellInfo->EquippedItemSubClass);

    PlayerSpell *newspell;

    newspell = new PlayerSpell;
    newspell->spellId = spell_id;
    newspell->active = active;

    WorldPacket data;
    if(newrank > 0 && newspell->active)
    {
        PlayerSpellList::iterator itr,next;
        for (itr = m_spells.begin(); itr != m_spells.end(); itr=next)
        {
            next = itr;
            next++;
            if(!(*itr)->spellId)
                continue;
            if(IsRankSpellDueToSpell(spellInfo,(*itr)->spellId))
            {
                oldrank = FindSpellRank((*itr)->spellId);
                if(oldrank == 0)
                    break;
                if(newrank > oldrank && (*itr)->active)
                {
                    data.Initialize(SMSG_SUPERCEDED_SPELL);
                    data << uint32((*itr)->spellId);
                    data << uint32(spell_id);
                    GetSession()->SendPacket( &data );
                    (*itr)->active = 0;
                }
                if(newrank < oldrank)
                {
                    data.Initialize(SMSG_SUPERCEDED_SPELL);
                    data << uint32(spell_id);
                    data << uint32((*itr)->spellId);
                    GetSession()->SendPacket( &data );
                    newspell->active = 0;
                }
            }
        }
    }

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
        PlayerSpellList::iterator itr;
        for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
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
                    GetSession()->SendPacket(&data);
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
    GetSession()->SendPacket(&data);

    addSpell(spell_id,1);

}

bool Player::removeSpell(uint16 spell_id)
{
    PlayerSpellList::iterator itr;
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
    updateMask->SetBit(UNIT_FIELD_OFFHANDATTACKTIME);
    updateMask->SetBit(UNIT_FIELD_RANGEDATTACKTIME);
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

bool Player::HasSpell(uint32 spell) const
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spell);

    if (!spellInfo) return true;

    // Look in the effects of spell , if is a Learn Spell Effect, see if is equal to triggerspell
    // If inst, look if have this spell.
    for (PlayerSpellList::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
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

bool Player::CanLearnProSpell(uint32 spell)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spell);

    if (!spellInfo)
        return false;
    if(spellInfo->Effect[0] != 36)
        return true;

    uint32 skill = spellInfo->EffectMiscValue[1];
    uint32 value = 0;

    if( skill != SKILL_HERBALISM && skill != SKILL_MINING && skill != SKILL_LEATHERWORKING
        && skill != SKILL_BLACKSMITHING && skill != SKILL_ALCHEMY && skill != SKILL_ENCHANTING
        && skill != SKILL_TAILORING && skill != SKILL_ENGINERING && skill != SKILL_SKINNING)
        return true;
    for (PlayerSpellList::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        SpellEntry *pSpellInfo = sSpellStore.LookupEntry((*itr)->spellId);
        if(!pSpellInfo)
            continue;

        if(pSpellInfo->Effect[1] == 118)
        {
            uint32 pskill = pSpellInfo->EffectMiscValue[1];
            if( pskill != SKILL_HERBALISM && pskill != SKILL_MINING && pskill != SKILL_LEATHERWORKING
                && pskill != SKILL_BLACKSMITHING && pskill != SKILL_ALCHEMY && pskill != SKILL_ENCHANTING
                && pskill != SKILL_TAILORING && pskill != SKILL_ENGINERING && pskill != SKILL_SKINNING)
                continue;
            if(pskill == skill)
            {
                return true;
                break;
            }
            else value += 1;
        }
    }
    if(value >= sWorld.getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL))
        return false;
    else return true;
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

    //loginDatabase.PExecute("UPDATE `realmcharacters` SET `numchars` = `numchars` - 1 WHERE `acctid` = %d AND `realmid` = %d", GetSession()->GetAccountId(), realmID);
    QueryResult *resultCount = sDatabase.PQuery("SELECT COUNT(guid) FROM `character` WHERE `account` = '%d'", GetSession()->GetAccountId());
    uint32 charCount = 0;
    if (resultCount)
    {
        Field *fields = resultCount->Fetch();
        charCount = fields[0].GetUInt32();
        delete resultCount;
        loginDatabase.PExecute("INSERT INTO `realmcharacters` (`numchars`, `acctid`, `realmid`) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE `numchars` = %d", charCount, GetSession()->GetAccountId(), realmID, charCount);
    }

    for(int i = 0; i < BANK_SLOT_ITEM_END; i++)
    {
        if(m_items[i] == NULL)
            continue;
        m_items[i]->DeleteFromDB();
        if(m_items[i]->IsBag())
            ((Bag*)m_items[i])->DeleteFromDB();
    }

    sDatabase.PExecute("DELETE FROM `character_queststatus` WHERE `guid` = '%u'",guid);
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

    SetHealth( 1 );

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

    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NONE | UNIT_FLAG_NOT_IN_PVP );
    SetUInt32Value(UNIT_FIELD_AURA + 32, 8326);             // set ghost form
    SetUInt32Value(UNIT_FIELD_AURA + 33, 0x5068 );          //!dono

    SetUInt32Value(UNIT_FIELD_AURAFLAGS + 4, 0xEE);

    SetUInt32Value(UNIT_FIELD_AURASTATE, 0x02);

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,(float)1.0);    //see radius of death player?

    SetUInt32Value(UNIT_FIELD_BYTES_1, 0x1000000);          //Set standing so always be standing

    SetUInt32Value(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

}

void Player::SendDelayResponse(const uint32 ml_seconds)
{
    WorldPacket data;
    data.Initialize( SMSG_QUERY_TIME_RESPONSE );
    data << (uint32)getMSTime();
    GetSession()->SendPacket( &data );
}

void Player::ResurrectPlayer()
{
    // remove death flag + set aura
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

    // return the PvP enable flag to normal
    SetPvP( GetPvP() );

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

    m_deathTimer = 0;
}

void Player::KillPlayer()
{
    WorldPacket data;

    SetMovement(MOVE_ROOT);

    StopMirrorTimer(0);
    StopMirrorTimer(1);
    StopMirrorTimer(2);

    setDeathState(CORPSE);
    SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_IN_PVP );

    SetFlag( UNIT_DYNAMIC_FLAGS, 0x00 );

    // 6 minutes until repop at graveyard
    m_deathTimer = 360000;

    // create the body
    CreateCorpse();

    // save body in db
    m_pCorpse->SaveToDB();

}

void Player::CreateCorpse()
{
    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    m_pCorpse = new Corpse();
    if(!m_pCorpse->Create(objmgr.GenerateLowGuid(HIGHGUID_CORPSE), this, GetMapId(), GetPositionX(),
        GetPositionY(), GetPositionZ(), GetOrientation()))
    {
        delete m_pCorpse;
        m_pCorpse = NULL;
        return;
    }

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
    GraveyardTeleport *ClosestGrave = objmgr.GetClosestGraveYard( m_positionX, m_positionY, m_positionZ, GetMapId(), GetTeam() );

    if(ClosestGrave)
    {
        // stop countdown until repop
        m_deathTimer = 0;

        TeleportTo(ClosestGrave->MapId, ClosestGrave->X, ClosestGrave->Y, ClosestGrave->Z, ClosestGrave->orientation);
        delete ClosestGrave;
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
    if(UpdateSkill(SKILL_DEFENSE))
    {
        // update dependent from defense skill part BlockChanceWithoutMods = 5 + (GetDefenceSkillValue() - getLevel()*5)*0.04);
        ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE, 0.04,true);
    }
}

//skill+1, checking for max value
bool Player::UpdateSkill(uint32 skill_id)
{
    if(!skill_id) return false;
    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if (GetUInt32Value(PLAYER_SKILL(i)) == skill_id) break;
    if(i>=PLAYER_MAX_SKILLS) return false;

    uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
    uint16 value = SKILL_VALUE(data);
    uint16 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max)) return false;

    if (uint32(value/max)*512 < urand(0,512))
    {
        SetUInt32Value(PLAYER_SKILL(i)+1,data+1);
        return true;
    }

    return false;
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
    if(skill_id == SKILL_MINING && value>75)
        return;
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

void Player::UpdateMeleeSkillWeapon()
{
    Item *tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if (!tmpitem)
        UpdateSkill(SKILL_UNARMED);
    else if(tmpitem->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        UpdateSkill(tmpitem->GetSkill());

    tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

    if (tmpitem)
        UpdateSkill(tmpitem->GetSkill());
}

void Player::UpdateRangedSkillWeapon()
{
    Item* tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

    if (tmpitem)
        UpdateSkill(tmpitem->GetSkill());
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
        uint32 max = data>>16;
        uint32 max_Skill = data%0x10000+getLevel()*5*0x10000;
        if((max_Skill>>16) > 300)
            max_Skill = data%0x10000+300*0x10000;

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

void Player::UpdateSkillsToMaxSkillsForLevel()
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

        uint32 max = data>>16;

        if(max > 1)
        {
            uint32 new_data = max * 0x10000 + max;
            SetUInt32Value(PLAYER_SKILL(i)+1,new_data);
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

bool Player::HasSkill(uint32 skill) const
{
    if(!skill)return false;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if (GetUInt32Value(PLAYER_SKILL(i)) == skill)
        {
            return true;
        }
    }
    return false;
}

uint16 Player::GetSkillValue(uint32 skill) const
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

void Player::SendInitialActions()
{
    sLog.outString( "Initializing Action Buttons for '%u'", GetGUIDLow() );
    WorldPacket data;
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
    GetSession()->SendPacket(data);
}

void Player::CheckExploreSystem()
{

    if (m_deathState & DEAD)
        return;
    if (isInFlight())
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
        if(!p)
        {
            sLog.outError("PLAYER: Player %u discovered unknown area (x: %u y: %u map: %u", GetGUID(), m_positionX,m_positionY,GetMapId());
        }
        else if(p->area_level)
        {
            uint32 XP = p->area_level*10;
            uint32 area = p->ID;
            GiveXP( XP, NULL );

            SendExplorationExperience(area,XP);

            sLog.outDetail("PLAYER: Player %u discovered a new area: %u", GetGUID(), area);
        }
    }

}

void Player::setFactionForRace(uint8 race)
{
    m_team = 0;
    uint32 faction = 0;
    switch(race)
    {
        case HUMAN:
            faction = 1;
            m_team = (uint32)ALLIANCE;
            break;
        case DWARF:
            faction = 3;
            m_team = (uint32)ALLIANCE;
            break;
        case NIGHTELF:
            faction = 4;
            m_team = (uint32)ALLIANCE;
            break;
        case GNOME:
            faction = 115;
            m_team = (uint32)ALLIANCE;
            break;

        case ORC:
            faction = 2;
            m_team = (uint32)HORDE;
            break;
        case UNDEAD_PLAYER:
            faction = 5;
            m_team = (uint32)HORDE;
            break;
        case TAUREN:
            faction = 6;
            m_team = (uint32)HORDE;
            break;
        case TROLL:
            faction = 116;
            m_team = (uint32)HORDE;
            break;
    }
    setFaction( faction );
}

void Player::UpdateReputation() const
{
    std::list<struct Factions>::const_iterator itr;

    sLog.outDebug( "WORLD: Player::UpdateReputation" );

    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {
        SendSetFactionStanding(&*itr);
    }
}

void Player::SendSetFactionStanding(const Factions* faction) const
{
    WorldPacket data;

    if(faction->Flags & 0x00000001 )                        //If faction is visible then update it
    {
        data.Initialize(SMSG_SET_FACTION_STANDING);
        data << (uint32) faction->Flags;
        data << (uint32) faction->ReputationListID;
        data << (uint32) faction->Standing;
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

void Player::SetInitialFactions()
{
    Factions newFaction;
    FactionEntry *factionEntry = NULL;

    for(unsigned int i = 1; i <= sFactionStore.GetNumRows(); i++)
    {
        factionEntry = sFactionStore.LookupEntry(i);

        if( factionEntry && (factionEntry->reputationListID >= 0))
        {
            newFaction.ID = factionEntry->ID;
            newFaction.ReputationListID = factionEntry->reputationListID;
            newFaction.Standing = 0;
            //Set visible to factions of own team
            if( GetTeam() == factionEntry->team ) newFaction.Flags = 1;
            else newFaction.Flags = 0;

            //If the faction's team is enemy of my one we are at war!
            if(GetTeam() == ALLIANCE )
            {
                if( factionEntry->team == HORDE || factionEntry->team == HORDE_FORCES )
                    newFaction.Flags = (newFaction.Flags | 2);
            }
            else
            if(GetTeam() == HORDE    )
            {
                if( factionEntry->team == ALLIANCE || factionEntry->team == ALLIANCE_FORCES )
                    newFaction.Flags = (newFaction.Flags | 2);
            }

            factions.push_back(newFaction);
        }
    }
}

uint32 Player::GetReputation(uint32 faction_id) const
{
    FactionEntry *factionEntry = sFactionStore.LookupEntry(faction_id);

    std::list<struct Factions>::const_iterator itr;
    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {
        if(int32(itr->ReputationListID) == factionEntry->reputationListID)
        {
            return itr->Standing;
        }
    }
    return 0;
}

bool Player::SetStanding(uint32 faction, int standing)
{
    FactionTemplateEntry *factionTemplateEntry = sFactionTemplateStore.LookupEntry(faction);

    if(!factionTemplateEntry)
    {
        sLog.outError("Player::SetStanding: Can't update reputation of %s for unknown faction (faction template id) #%u.",GetName(),faction);
        return false;
    }

    FactionEntry *factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->faction);

    // Faction without recorded reputation. Just ignore.
    if(!factionEntry)
        return false;

    return ModifyFactionReputation(factionEntry,standing);
}

bool Player::ModifyFactionReputation(FactionEntry* factionEntry, int32 standing)
{
    std::list<struct Factions>::iterator itr;
    for(itr = factions.begin(); itr != factions.end(); ++itr)
    {
        if(int32(itr->ReputationListID) == factionEntry->reputationListID)
        {
            itr->Standing = (((int32)itr->Standing + standing) > 0 ? itr->Standing + standing: 0);
            itr->Flags = (itr->Flags | 0x00000001);
            SendSetFactionStanding(&*itr);
            return true;
        }
    }
    return false;
}

//Calculates how many reputation points player gains in wich victim's enemy factions
void Player::CalculateReputation(Unit *pVictim)
{
    if( !pVictim ) return;

    if( pVictim->GetTypeId() != TYPEID_PLAYER )
    {
        SetStanding( pVictim->getFaction(), (-100) );
    }
}

//Calculate how many reputation points player gain with the quest
void Player::CalculateReputation(Quest *pQuest, uint64 guid)
{
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
    if( pCreature )
    {
        int dif = getLevel() - pQuest->GetQuestInfo()->MinLevel;
        if(dif < 0) dif = 0;
        else if(dif > 5) dif = 5;

        int RepPoints;
        if(HasSpell(20599))                                 //spell : diplomacy
            RepPoints = (uint32)(((5-dif)*0.20)*110);       //human gain more 10% rep.
        else
            RepPoints = (uint32)(((5-dif)*0.20)*100);
        SetStanding(pCreature->getFaction(), (RepPoints > 0 ? RepPoints : 1) );
    }
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
            date = fields[2].GetUInt32();

            if(fields[0].GetUInt32() == HONORABLE_KILL)
            {
                lifetime_honorableKills++;
                //total_honor += fields[1].GetFloat();

                if( date == today)
                {
                    today_honorableKills++;
                }
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

                //All honor points until last week
                if( date < LastWeekEnd )
                {
                    total_honor += fields[1].GetFloat();
                }

            }
            else if(fields[0].GetUInt32() == DISHONORABLE_KILL)
            {
                lifetime_dishonorableKills++;
                //total_honor -= fields[1].GetFloat();

                if( date == today)
                {
                    today_dishonorableKills++;
                }

                //All honor points until last week
                if( date < LastWeekEnd )
                {
                    total_honor -= fields[1].GetFloat();
                }
            }
        }
        while( result->NextRow() );

        delete result;
    }

    //Store Total Honor points...
    SetTotalHonor(total_honor);

    //RIGHEST RANK
    //If the new rank is highest then the old one, then m_highest_rank is updated
    if( CalculateHonorRank(total_honor) > GetHonorHighestRank() )
    {
        SetHonorHighestRank( CalculateHonorRank(total_honor) );
    }

    //RATING
    SetHonorRating( MaNGOS::Honor::CalculeRating(this) );

    //STANDING
    SetHonorLastWeekStanding( MaNGOS::Honor::CalculeStanding(this) );

    //TODO Fix next rank bar... it is not working fine! For while it be set with the total honor points...
    //NEXT RANK BAR
    SetUInt32Value(PLAYER_FIELD_HONOR_BAR, (uint32)( (total_honor < 0) ? 0: total_honor) );

    //RANK (Patent)
    if( CalculateHonorRank(total_honor) )
        SetUInt32Value(PLAYER_BYTES_3, (( CalculateHonorRank(total_honor) << 24) + 0x04000000) + m_drunk);
    else
        SetUInt32Value(PLAYER_BYTES_3, m_drunk);

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
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK, GetHonorLastWeekStanding());

    //LIFE TIME
    SetUInt32Value(PLAYER_FIELD_SESSION_KILLS, (lifetime_dishonorableKills << 16) + lifetime_honorableKills );
    SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, lifetime_dishonorableKills);
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, lifetime_honorableKills);
    //TODO: Into what field we need to set it? Fix it!
    SetUInt32Value(PLAYER_FIELD_PVP_MEDALS/*???*/, (GetHonorHighestRank() != 0 ? ((GetHonorHighestRank() << 24) + 0x040F0001) : 0) );
}

uint32 Player::GetHonorRank() const
{
    return CalculateHonorRank(m_total_honor_points);
}

//What is Player's rank... private, scout...
uint32 Player::CalculateHonorRank(float honor_points) const
{
    int rank = 0;

    if(honor_points <=    0.00) rank = 0; else
        if(honor_points <  2000.00) rank = 1;
    else
        rank = ( (int)(honor_points / 5000) + 1);

    return rank;
}

//How many times Player kill pVictim...
int Player::CalculateTotalKills(Player *pVictim) const
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

//If players are too far way of duel flag... then player loose the duel
void Player::CheckDuelDistance()
{
    if( !isInDuel() ) return;

    uint64 duelFlagGUID = GetUInt64Value(PLAYER_DUEL_ARBITER);

    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*this, duelFlagGUID);

    //If the distance of duel flag is > 50
    if( !obj || GetDistanceSq(obj) > (float)2500 )
    {
        DuelComplete();
    }

}

void Player::DuelComplete()
{
    if( !isInDuel() ) return;

    WorldPacket data;
    uint64 duelFlagGUID = GetUInt64Value(PLAYER_DUEL_ARBITER);

    AttackStop();
    m_pDuel->AttackStop();

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

    //Player kneel when finish the duel
    HandleEmoteCommand(ANIM_EMOTE_BEG);

    SetInDuel(false);
    m_pDuel->SetInDuel(false);

    #if 1
    //Restore the state of pvpOn
    RestorePvpState();
    m_pDuel->RestorePvpState();
    //Restore to correct factiontemplate
    setFactionForRace(getRace());
    m_pDuel->setFactionForRace(m_pDuel->getRace());
    #endif

    //ResurrectPlayer();
    setDeathState(ALIVE);

    //Remove Duel Flag object
    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*this, duelFlagGUID);

    if(obj)
    {
        RemoveGameObject(obj->GetSpellId(),false);
        m_pDuel->RemoveGameObject(obj->GetSpellId(),false);
        MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj, true);
    }

    SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    m_pDuel->SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    m_pDuel->SetUInt32Value(PLAYER_DUEL_TEAM, 0);

}

static unsigned long    holdrand = 0x89abcdef;

void Rand_Init(int seed)
{
    holdrand = seed;
}

int irand(int min, int max)
{
    assert((max - min) < 32768);

    max++;
    holdrand = (holdrand * 214013L) + 2531011L;

    return (((holdrand >> 17) * (max - min)) >> 15) + min;
}

void Player::SendAttackStart(Unit* pVictim)
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
    clearUnitState(UNIT_STAT_IN_FLIGHT);
    SetMoney( m_dismountCost);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    /* Remove the "player locked" flag, to allow movement */
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT | UNIT_FLAG_DISABLE_MOVE );
}

void Player::_ApplyItemMods(Item *item, uint8 slot,bool apply)
{
    if(slot >= INVENTORY_SLOT_BAG_END || !item) return;

    ItemPrototype *proto = item->GetProto();

    if(!proto) return;

    sLog.outString("applying mods for item %u ",item->GetGUIDLow());
    if(proto->ItemSet)
    {
        if (apply)
            AddItemsSetItem(this,item);
        else
            RemoveItemsSetItem(this,proto);
    }

    _RemoveStatsMods();

    int32 val;
    std::string typestr;
    std::string applystr = apply ? "Add" : "Remove";
    for (int i = 0; i < 10; i++)
    {
        val = proto->ItemStat[i].ItemStatValue ;

        switch (proto->ItemStat[i].ItemStatType)
        {
            case POWER:                                     // modify MP
                ApplyMaxPowerMod(POWER_MANA, val, apply);
                typestr = "Mana";
                break;
            case HEALTH:                                    // modify HP
                ApplyMaxHealthMod(val, apply);
                typestr = "Health";
                break;
            case AGILITY:                                   // modify agility
                ApplyStatMod(STAT_AGILITY,                val, apply);
                ApplyModUInt32Value(PLAYER_FIELD_POSSTAT1,val, apply);
                typestr = "AGILITY";
                break;
            case STRENGHT:                                  //modify strength
                ApplyStatMod(STAT_STRENGTH,               val, apply);
                ApplyModUInt32Value(PLAYER_FIELD_POSSTAT0,val, apply);
                typestr = "STRENGHT";
                break;
            case INTELLECT:                                 //modify intellect
                ApplyStatMod(STAT_INTELLECT,               val,    apply);
                ApplyModUInt32Value(PLAYER_FIELD_POSSTAT3,val,    apply);
                ApplyMaxPowerMod(POWER_MANA,              val*15, apply);
                typestr = "INTELLECT";
                break;
            case SPIRIT:                                    //modify spirit
                ApplyStatMod(STAT_SPIRIT,                 val, apply);
                ApplyModUInt32Value(PLAYER_FIELD_POSSTAT4,val, apply);
                typestr = "SPIRIT";
                break;
            case STAMINA:                                   //modify stamina
                ApplyStatMod(STAT_STAMINA                ,val,   apply);
                ApplyModUInt32Value(PLAYER_FIELD_POSSTAT2,val,   apply);
                ApplyMaxHealthMod(                        val*10,apply);
                typestr = "STAMINA";
                break;
        }
        if(val > 0)
            sLog.outDebug("%s %s: \t\t%u", applystr.c_str(), typestr.c_str(), val);

    }

    if (proto->Armor)
    {
        ApplyArmorMod( proto->Armor, apply);
        sLog.outDebug("%s Armor: \t\t%u", applystr.c_str(),  proto->Armor);
    }

    if (proto->Block)
    {
        ApplyBlockValueMod(proto->Block, apply);
        sLog.outDebug("%s Block: \t\t%u", applystr.c_str(),  proto->Block);
    }

    if (proto->HolyRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_HOLY, proto->HolyRes, apply);
        sLog.outDebug("%s HolyRes: \t\t%u", applystr.c_str(),  proto->HolyRes);
    }

    if (proto->FireRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_FIRE, proto->FireRes, apply);
        sLog.outDebug("%s FireRes: \t\t%u", applystr.c_str(),  proto->FireRes);
    }

    if (proto->NatureRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_NATURE, proto->NatureRes, apply);
        sLog.outDebug("%s NatureRes: \t\t%u", applystr.c_str(),  proto->NatureRes);
    }

    if (proto->FrostRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_FROST, proto->FrostRes, apply);
        sLog.outDebug("%s FrostRes: \t\t%u", applystr.c_str(),  proto->FrostRes);
    }

    if (proto->ShadowRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_SHADOW, proto->ShadowRes, apply);
        sLog.outDebug("%s ShadowRes: \t\t%u", applystr.c_str(),  proto->ShadowRes);
    }

    if (proto->ArcaneRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_ARCANE, proto->ArcaneRes, apply);
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
        ApplyModFloatValue(MINDAMAGEFIELD, proto->Damage[0].DamageMin, apply);
        sLog.outString("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMin, GetFloatValue(MINDAMAGEFIELD));
    }

    if (proto->Damage[0].DamageMax  > 0 && MAXDAMAGEFIELD)
    {
        ApplyModFloatValue(MAXDAMAGEFIELD, proto->Damage[0].DamageMax, apply);
        sLog.outString("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMax, GetFloatValue(MAXDAMAGEFIELD));
    }

    if (proto->Delay)
    {
        if(slot == EQUIPMENT_SLOT_RANGED)
        {
            SetAttackTime(RANGED_ATTACK, apply ? proto->Delay: 2000);
            typestr = "Range";
            sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
        else if(slot==EQUIPMENT_SLOT_MAINHAND)
        {
            SetAttackTime(BASE_ATTACK, apply ? proto->Delay: 2000);
            typestr = "Mainhand";
            sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
        else if(slot==EQUIPMENT_SLOT_OFFHAND)
        {
            SetAttackTime(OFF_ATTACK, apply ? proto->Delay: 2000);
            typestr = "Offhand";
            sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
    }

    _ApplyStatsMods();

    if(apply)
        CastItemEquipSpell(item);
    else
        for (int i = 0; i < 5; i++)
            if(proto->Spells[i].SpellId)
                RemoveAurasDueToSpell(proto->Spells[i].SpellId );

    for(int enchant_solt =  0 ; enchant_solt < 21; enchant_solt+=3)
    {
        uint32 Enchant_id = item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+enchant_solt);
        if(Enchant_id)
            AddItemEnchant(Enchant_id, apply);
    }

    sLog.outDebug("_ApplyItemMods complete.");
}

void Player::CastItemEquipSpell(Item *item)
{
    if(!item) return;

    ItemPrototype *proto = item->GetProto();

    if(!proto) return;

    SpellEntry *spellInfo;

    for (int i = 0; i < 5; i++)
    {
        if(!proto->Spells[i].SpellId ) continue;
        if(proto->Spells[i].SpellTrigger != ON_EQUIP) continue;

        spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId);
        if(!spellInfo)
        {
            DEBUG_LOG("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId);
            continue;
        }

        DEBUG_LOG("WORLD: cast Item spellId - %i", proto->Spells[i].SpellId);

        Spell spell(this, spellInfo, true, 0);

        SpellCastTargets targets;
        targets.setUnitTarget( this );
        spell.m_CastItem = item;
        spell.prepare(&targets);
    }
}

void Player::CastItemCombatSpell(Item *item,Unit* Target)
{
    if(!item)
        return;

    ItemPrototype *proto = item->GetProto();
    if(!proto)
        return;

    if (!Target || Target == this )
        return;

    SpellEntry *spellInfo;

    for (int i = 0; i < 5; i++)
    {
        if(!proto->Spells[i].SpellId ) continue;
        if(proto->Spells[i].SpellTrigger != CHANCE_ON_HIT) continue;

        spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId);
        if(!spellInfo)
        {
            DEBUG_LOG("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId);
            continue;
        }

        DEBUG_LOG("WORLD: cast Item spellId - %i", proto->Spells[i].SpellId);

        Spell spell(this, spellInfo, true, 0);

        SpellCastTargets targets;
        targets.setUnitTarget( Target );
        spell.m_CastItem = item;
        spell.prepare(&targets);
    }

    // item combat enchantments
    for(int e_slot = 0; e_slot < 21; e_slot++)
    {
        uint32 enchant_id = item->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+e_slot);
        SpellItemEnchantment *pEnchant;
        pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant) continue;
        uint32 enchant_display = pEnchant->display_type;
        uint32 enchant_value1 = pEnchant->value1;
        uint32 enchant_spell_id = pEnchant->spellid;
        SpellEntry *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);
        if(!enchantSpell_info) continue;
        if(enchant_display!=4 && enchant_display!=2 && this->IsItemSpellToCombat(enchantSpell_info))
        {
            if (urand(0,100) <= enchant_value1 || enchant_value1 == 0)
            {
                Spell spell(this, enchantSpell_info, true, 0);
                SpellCastTargets targets;
                targets.setUnitTarget(Target);
                spell.prepare(&targets);
            }
        }
    }
}

// only some item spell/auras effects can be executed when item is equiped.
// If not you can have unexpected beaviur. like item giving damage to player when equip.
bool Player::IsItemSpellToEquip(SpellEntry *spellInfo)
{
    return (GetDuration(spellInfo) == -1);                  // infinite duration -> passive aura
    /*
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
    */
}

// only some item spell/auras effects can be executed when in combat.
// If not you can have unexpected beaviur. like having stats always growing each attack.
bool Player::IsItemSpellToCombat(SpellEntry *spellInfo)
{
    return (GetDuration(spellInfo) != -1);                  // infinite duration -> passive aura

    /*
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
    */
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
    Loot    *loot;

    if (IS_GAMEOBJECT_GUID(guid))
    {
        GameObject *go =
            ObjectAccessor::Instance().GetGameObject(*this, guid);

        if (!go)
            return;

        loot = &go->loot;

        if(loot->empty())
        {
            uint32 lootid =  go->lootid;

            if(lootid)
                FillLoot(this,loot,lootid);

            if(loot_type == 3)
                go->getFishLoot(loot);
        }
    }
    else
    {
        Creature *creature =
            ObjectAccessor::Instance().GetCreature(*this, guid);

        if (!creature)
            return;

        loot   = &creature->loot;

        if(loot->empty())
        {
            uint32 lootid = creature->GetCreatureInfo()->lootid;

            if (!creature->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_VENDOR) && lootid)
                FillLoot(this,loot,lootid);

            creature->generateMoneyLoot();

            if (loot_type == 2)
                creature->getSkinLoot();
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

uint8 Player::CheckFishingAble() const
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
/***                    STORAGE SYSTEM                 ***/
/*********************************************************/

void Player::SetVirtualItemSlot( uint8 i, Item* item)
{
    assert(i < 3);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + 2*i,    item ? item->GetGUIDLow()              : 0);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + 2*i +1, item ? item->GetProto()->Sheath        : 0);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+i,item ? item->GetProto()->DisplayInfoID : 0);
}

void Player::SetSheath( uint32 sheathed )
{
    switch (sheathed)
    {
        case 0:                                             // no prepeared weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
        case 1:                                             // prepeared melee weapon
            SetVirtualItemSlot(0,GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND));
            SetVirtualItemSlot(1,GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND));
            SetVirtualItemSlot(2,NULL);
            break;
        case 2:                                             // prepeared ranged weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED));
            break;
        default:
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
    }
}

uint8 Player::FindEquipSlot( uint32 type, uint32 slot, bool swap ) const
{
    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch( type )
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
        default :
            return NULL_SLOT;
    }

    if( slot != NULL_SLOT )
    {
        if( swap || !GetItemByPos( INVENTORY_SLOT_BAG_0, slot ) )
        {
            for (int i = 0; i < 4; i++)
            {
                if ( slots[i] == slot )
                    return slot;
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            if ( slots[i] != NULL_SLOT && ( swap || !GetItemByPos( INVENTORY_SLOT_BAG_0, slots[i] ) ) )
                return slots[i];
        }
    }
    return slots[0];
}

Item* Player::CreateItem( uint32 item, uint32 count ) const
{
    ItemPrototype *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Item *pItem = NewItemOrBag( pProto );
        if ( count > pProto->Stackable )
            count = pProto->Stackable;
        if ( count < 1 )
            count = 1;
        if( pItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), item, const_cast<Player*>(this)) )
        {
            pItem->SetCount( count );
            return pItem;
        }
        else
            delete pItem;
    }
    return NULL;
}

uint32 Player::GetItemCount( uint32 item ) const
{
    Item *pItem;
    uint32 count = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
            count += pItem->GetCount();
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                if( pItem && pItem->GetEntry() == item )
                    count += pItem->GetCount();
            }
        }
    }
    return count;
}

uint32 Player::GetBankItemCount( uint32 item ) const
{
    Item *pItem;
    uint32 count = 0;
    for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
            count += pItem->GetCount();
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                if( pItem && pItem->GetEntry() == item )
                    count += pItem->GetCount();
            }
        }
    }
    return count;
}

uint16 Player::GetPosByGuid( uint64 guid ) const
{
    Item *pItem;
    uint16 pos;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        pItem = GetItemByPos( pos );
        if( pItem && pItem->GetGUID() == guid )
            return pos;
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        pBag = (Bag*)GetItemByPos( pos );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pos = ((i << 8) | j);
                    pItem = GetItemByPos( pos );
                    if( pItem && pItem->GetGUID() == guid )
                        return pos;
                }
            }
        }
    }
    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        pBag = (Bag*)GetItemByPos( pos );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pos = ((i << 8) | j);
                    pItem = GetItemByPos( pos );
                    if( pItem && pItem->GetGUID() == guid )
                        return pos;
                }
            }
        }
    }
    return 0;
}

Item* Player::GetItemByPos( uint16 pos ) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    return GetItemByPos( bag, slot );
}

Item* Player::GetItemByPos( uint8 bag, uint8 slot ) const
{
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= EQUIPMENT_SLOT_START && slot < BANK_SLOT_BAG_END ) )
        return m_items[slot];
    else
    {
        Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
        if ( pBag )
            return pBag->GetItemByPos(slot);
    }
    return NULL;
}

bool Player::HasBankBagSlot( uint8 slot ) const
{
    uint32 maxslot = ((GetUInt32Value(PLAYER_BYTES_2) & 0x70000) >> 16) + BANK_SLOT_BAG_START;
    if( slot < maxslot )
        return true;
    return false;
}

bool Player::IsInventoryPos( uint16 pos ) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if( bag == INVENTORY_SLOT_BAG_0 && slot == NULL_SLOT )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END ) )
        return true;
    if( bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END )
        return true;
    return false;
}

bool Player::IsEquipmentPos( uint16 pos ) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END ) )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END ) )
        return true;
    return false;
}

bool Player::IsBankPos( uint16 pos ) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END ) )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END ) )
        return true;
    if( bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END )
        return true;
    return false;
}

bool Player::HasItemCount( uint32 item, uint32 count ) const
{
    Item *pItem;
    uint32 tempcount = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            tempcount += pItem->GetCount();
            if( tempcount >= count )
                return true;
        }
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pItem = GetItemByPos( i, j );
                    if( pItem && pItem->GetEntry() == item )
                    {
                        tempcount += pItem->GetCount();
                        if( tempcount >= count )
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

uint8 Player::CanStoreNewItem( uint8 bag, uint8 slot, uint16 &dest, uint32 item, uint32 count, bool swap ) const
{
    dest = 0;
    Item *pItem = CreateItem( item, count );
    if( pItem )
    {
        uint8 result = CanStoreItem( bag, slot, dest, pItem, swap );
        delete pItem;
        return result;
    }
    if( !swap )
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanStoreItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanStoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype *pProto = pItem->GetProto();
        if( pProto )
        {
            Item *pItem2;
            Bag *pBag;
            ItemPrototype *pBagProto;
            uint16 pos;
            if(pItem->IsBindedNotWith(GetGUID()))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;
            if( bag == 0 )
            {
                if( !swap && pProto->MaxCount > 0 )
                {
                    uint32 curcount = 0;
                    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pItem2 = GetItemByPos( pos );
                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                        {
                            curcount += pItem2->GetCount();
                            if( curcount + pItem->GetCount() > pProto->MaxCount )
                                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                        }
                    }
                    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ((i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                                    {
                                        curcount += pItem2->GetCount();
                                        if( curcount + pItem->GetCount() > pProto->MaxCount )
                                            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                                    }
                                }
                            }
                        }
                    }
                    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ((i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                                    {
                                        curcount += pItem2->GetCount();
                                        if( curcount + pItem->GetCount() > pProto->MaxCount )
                                            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                                    }
                                }
                            }
                        }
                    }
                }
                if( pProto->Stackable > 1 )
                {
                    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pItem2 = GetItemByPos( pos );
                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                        {
                            dest = pos;
                            return EQUIP_ERR_OK;
                        }
                    }
                    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ( (i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                    {
                                        dest = pos;
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                }
                if( pProto->Class == ITEM_CLASS_PROJECTILE )
                {
                    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto && pBagProto->SubClass == pProto->SubClass )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                    pItem2 = GetItemByPos( i, j );
                                    if( !pItem2 )
                                    {
                                        dest = ( (i << 8) | j );
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                }
                for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
                {
                    pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                    if( !pItem2 )
                    {
                        dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        return EQUIP_ERR_OK;
                    }
                }
                for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                {
                    pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                    if( pBag )
                    {
                        pBagProto = pBag->GetProto();
                        if( pBagProto && pBagProto->Class != ITEM_CLASS_QUIVER )
                        {
                            for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                            {
                                pItem2 = GetItemByPos( i, j );
                                if( !pItem2 )
                                {
                                    dest = ( (i << 8) | j );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                    }
                }
                return EQUIP_ERR_INVENTORY_FULL;
            }
            else
            {
                if( slot == NULL_SLOT )
                {
                    if( pProto->InventoryType == INVTYPE_BAG )
                    {
                        Bag *pBag = (Bag*)pItem;
                        if( pBag && !pBag->IsEmpty() )
                            return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
                    }
                    if( pProto->Stackable > 1 )
                    {
                        if( bag == INVENTORY_SLOT_BAG_0 )
                        {
                            for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
                            {
                                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                                if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                {
                                    dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                        else
                        {
                            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                            if( pBag )
                            {
                                pBagProto = pBag->GetProto();
                                if( pBagProto )
                                {
                                    for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                    {
                                        pItem2 = GetItemByPos( bag, j );
                                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                        {
                                            dest = ( (bag << 8) | j );
                                            return EQUIP_ERR_OK;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if( bag == INVENTORY_SLOT_BAG_0 )
                    {
                        for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
                        {
                            pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                            if( !pItem2 )
                            {
                                dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                return EQUIP_ERR_OK;
                            }
                        }
                    }
                    else
                    {
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                if( pBagProto->Class == ITEM_CLASS_QUIVER && pBagProto->SubClass != pProto->SubClass )
                                    return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
                                if( pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass > ITEM_SUBCLASS_CONTAINER && pBagProto->SubClass != pProto->SubClass )
                                    return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pItem2 = GetItemByPos( bag, j );
                                    if( !pItem2 )
                                    {
                                        dest = ( (bag << 8) | j );
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                    return EQUIP_ERR_BAG_FULL;
                }
                else
                {
                    if( pProto->InventoryType == INVTYPE_BAG )
                    {
                        Bag *pBag = (Bag*)pItem;
                        if( pBag && !pBag->IsEmpty() )
                            return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
                    }
                    pItem2 = GetItemByPos( bag, slot );
                    if( pItem2 && !swap )
                    {
                        if( pProto->Stackable > 1 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() < pProto->Stackable )
                        {
                            dest = ( (bag << 8) | slot );
                            return EQUIP_ERR_OK;
                        }
                        else
                            return EQUIP_ERR_COULDNT_SPLIT_ITEMS;
                    }
                    else
                    {
                        if( bag == INVENTORY_SLOT_BAG_0 )
                        {
                            dest = ( (bag << 8) | slot );
                            return EQUIP_ERR_OK;
                        }
                        else
                        {
                            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                            if( pBag )
                            {
                                pBagProto = pBag->GetProto();
                                if( pBagProto )
                                {
                                    if( pBagProto->Class == ITEM_CLASS_QUIVER && pBagProto->SubClass != pProto->SubClass )
                                        return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
                                    if( pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass > ITEM_SUBCLASS_CONTAINER && pBagProto->SubClass != pProto->SubClass )
                                        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
                                    dest = ( (bag << 8) | slot );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if( !swap )
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
    return 0;
}

uint8 Player::CanEquipItem( uint8 slot, uint16 &dest, Item *pItem, bool swap, bool check_alive ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype *pProto = pItem->GetProto();
        if( pProto )
        {
            if(isInCombat()&& pProto->Class != ITEM_CLASS_WEAPON && pProto->Class != ITEM_CLASS_PROJECTILE)
                return EQUIP_ERR_CANT_DO_IN_COMBAT;

            uint32 type = pProto->InventoryType;
            uint8 eslot = FindEquipSlot( type, slot, swap );
            if( eslot == NULL_SLOT )
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
            uint8 msg = CanUseItem( pItem , check_alive );
            if( msg != EQUIP_ERR_OK )
                return msg;
            if( !swap && GetItemByPos( INVENTORY_SLOT_BAG_0, eslot ) )
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;
            if( type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND || type == INVTYPE_WEAPONOFFHAND )
            {
                uint8 twinslot = ( eslot == EQUIPMENT_SLOT_MAINHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND );
                Item *twinItem = GetItemByPos( INVENTORY_SLOT_BAG_0, twinslot );
                if( twinItem && twinItem->GetProto()->InventoryType != INVTYPE_SHIELD && !HasSpell( 274 ) )
                    return EQUIP_ERR_CANT_DUAL_WIELD;
            }
            if( type == INVTYPE_SHIELD )
            {
                uint8 twinslot = ( eslot == EQUIPMENT_SLOT_MAINHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND );
                Item *twinItem = GetItemByPos( INVENTORY_SLOT_BAG_0, twinslot );
                if( twinItem )
                {
                    uint32 twintype = twinItem->GetProto()->InventoryType;
                    if( twintype == INVTYPE_2HWEAPON )
                        return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
                }
            }
            if( type == INVTYPE_2HWEAPON )
            {
                uint8 twinslot = ( eslot == EQUIPMENT_SLOT_MAINHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND );
                Item *twinItem = GetItemByPos( INVENTORY_SLOT_BAG_0, twinslot );
                if( twinItem )
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
            }
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }
    if( !swap )
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanBankItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanBankItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype *pProto = pItem->GetProto();
        if( pProto )
        {
            Item *pItem2;
            Bag *pBag;
            ItemPrototype *pBagProto;
            uint16 pos;
            if( pItem->IsBindedNotWith(GetGUID()) )
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;
            if( bag == 0 )
            {
                if( !swap && pProto->MaxCount > 0 )
                {
                    uint32 curcount = 0;
                    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pItem2 = GetItemByPos( pos );
                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                        {
                            curcount += pItem2->GetCount();
                            if( curcount + pItem->GetCount() > pProto->MaxCount )
                                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                        }
                    }
                    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ((i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                                    {
                                        curcount += pItem2->GetCount();
                                        if( curcount + pItem->GetCount() > pProto->MaxCount )
                                            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                                    }
                                }
                            }
                        }
                    }
                    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                    {
                        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ((i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() )
                                    {
                                        curcount += pItem2->GetCount();
                                        if( curcount + pItem->GetCount() > pProto->MaxCount )
                                            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                                    }
                                }
                            }
                        }
                    }
                }
                if( pProto->Stackable > 1 )
                {
                    for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pItem2 = GetItemByPos( pos );
                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                        {
                            dest = pos;
                            return EQUIP_ERR_OK;
                        }
                    }
                    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( pos );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ( (i << 8) | j );
                                    pItem2 = GetItemByPos( pos );
                                    if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                    {
                                        dest = pos;
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                }
                if( pProto->Class == ITEM_CLASS_PROJECTILE )
                {
                    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto && pBagProto->SubClass == pProto->SubClass )
                            {
                                for(int j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                    pItem2 = GetItemByPos( i, j );
                                    if( !pItem2 )
                                    {
                                        dest = ( (i << 8) | j );
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                }
                for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
                {
                    pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                    if( !pItem2 )
                    {
                        dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        return EQUIP_ERR_OK;
                    }
                }
                for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                {
                    pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                    if( pBag )
                    {
                        pBagProto = pBag->GetProto();
                        if( pBagProto && pBagProto->Class != ITEM_CLASS_QUIVER )
                        {
                            for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                            {
                                pItem2 = GetItemByPos( i, j );
                                if( !pItem2 )
                                {
                                    dest = ( (i << 8) | j );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                    }
                }
                return EQUIP_ERR_BANK_FULL;
            }
            else
            {
                if( slot == NULL_SLOT )
                {
                    if( pProto->InventoryType == INVTYPE_BAG )
                    {
                        Bag *pBag = (Bag*)pItem;
                        if( pBag && !pBag->IsEmpty() )
                            return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
                    }
                    if( pProto->Stackable > 1 )
                    {
                        if( bag == INVENTORY_SLOT_BAG_0 )
                        {
                            for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
                            {
                                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                                if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                {
                                    dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                        else
                        {
                            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                            if( pBag )
                            {
                                pBagProto = pBag->GetProto();
                                if( pBagProto )
                                {
                                    for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                    {
                                        pItem2 = GetItemByPos( bag, j );
                                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                                        {
                                            dest = ( (bag << 8) | j );
                                            return EQUIP_ERR_OK;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if( bag == INVENTORY_SLOT_BAG_0 )
                    {
                        for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
                        {
                            pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                            if( !pItem2 )
                            {
                                dest = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                                return EQUIP_ERR_OK;
                            }
                        }
                    }
                    else
                    {
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            if( pBagProto )
                            {
                                if( pBagProto->Class == ITEM_CLASS_QUIVER && pBagProto->SubClass != pProto->SubClass )
                                    return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
                                if( pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass > ITEM_SUBCLASS_CONTAINER && pBagProto->SubClass != pProto->SubClass )
                                    return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
                                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                                {
                                    pItem2 = GetItemByPos( bag, j );
                                    if( !pItem2 )
                                    {
                                        dest = ( (bag << 8) | j );
                                        return EQUIP_ERR_OK;
                                    }
                                }
                            }
                        }
                    }
                    return EQUIP_ERR_BAG_FULL;
                }
                else
                {
                    if( pProto->InventoryType == INVTYPE_BAG )
                    {
                        Bag *pBag = (Bag*)pItem;
                        if( pBag )
                        {
                            if( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END )
                            {
                                if( !HasBankBagSlot( slot ) )
                                    return EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT;
                                if( uint8 cantuse = CanUseItem( pItem ) != EQUIP_ERR_OK )
                                    return cantuse;
                            }
                            else
                            {
                                if( !pBag->IsEmpty() )
                                    return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
                            }
                        }
                    }
                    else
                    {
                        if( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END )
                            return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;
                    }
                    pItem2 = GetItemByPos( bag, slot );
                    if( pItem2 && !swap )
                    {
                        if( pProto->Stackable > 1 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
                        {
                            dest = ( (bag << 8) | slot );
                            return EQUIP_ERR_OK;
                        }
                        else
                            return EQUIP_ERR_COULDNT_SPLIT_ITEMS;
                    }
                    else
                    {
                        if( bag == INVENTORY_SLOT_BAG_0 )
                        {
                            dest = ( (bag << 8) | slot );
                            return EQUIP_ERR_OK;
                        }
                        else
                        {
                            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                            if( pBag )
                            {
                                pBagProto = pBag->GetProto();
                                if( pBagProto )
                                {
                                    if( pBagProto->Class == ITEM_CLASS_QUIVER && pBagProto->SubClass != pProto->SubClass )
                                        return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
                                    if( pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass > ITEM_SUBCLASS_CONTAINER && pBagProto->SubClass != pProto->SubClass )
                                        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
                                    dest = ( (bag << 8) | slot );
                                    return EQUIP_ERR_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if( !swap )
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
    return 0;
}

uint8 Player::CanUseItem( Item *pItem, bool check_alive ) const
{
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanUseItem item = %u", pItem->GetEntry());
        if( !isAlive() && check_alive )
            return EQUIP_ERR_YOU_ARE_DEAD;
        //if( isStunned() )
        //    return EQUIP_ERR_YOU_ARE_STUNNED;
        ItemPrototype *pProto = pItem->GetProto();
        if( pProto )
        {
            if( pItem->IsBindedNotWith(GetGUID()) )
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;
            if( (pProto->AllowableClass & getClassMask()) == 0 || (pProto->AllowableRace & getRaceMask()) == 0 )
                return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
            if( pItem->GetSkill() != 0  )
            {
                if( GetSkillValue( pItem->GetSkill() ) == 0 )
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            }
            if( pProto->RequiredSkill != 0  )
            {
                if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
                else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                    return EQUIP_ERR_SKILL_ISNT_HIGH_ENOUGH;
            }
            if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            if( GetHonorRank() < pProto->RequiredHonorRank )
                return EQUIP_ITEM_RANK_NOT_ENOUGH;
            /*if( GetREputation() < pProto->RequiredReputation )
                return EQUIP_ITEM_REPUTATION_NOT_ENOUGH;
            */
            if( getLevel() < pProto->RequiredLevel )
                return EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;
            return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 Player::CanUseAmmo( uint32 item ) const
{
    sLog.outDebug( "STORAGE: CanUseAmmo item = %u", item);
    if( !isAlive() )
        return EQUIP_ERR_YOU_ARE_DEAD;
    //if( isStunned() )
    //    return EQUIP_ERR_YOU_ARE_STUNNED;
    ItemPrototype *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        if( (pProto->AllowableClass & getClassMask()) == 0 || (pProto->AllowableRace & getRaceMask()) == 0 )
            return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
        if( pProto->RequiredSkill != 0  )
        {
            if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                return EQUIP_ERR_SKILL_ISNT_HIGH_ENOUGH;
        }
        if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
            return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
        if( GetHonorRank() < pProto->RequiredHonorRank )
            return EQUIP_ITEM_RANK_NOT_ENOUGH;
        /*if( GetREputation() < pProto->RequiredReputation )
        return EQUIP_ITEM_REPUTATION_NOT_ENOUGH;
        */
        if( getLevel() < pProto->RequiredLevel )
            return EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;
        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

void Player::StoreNewItem( uint16 pos, uint32 item, uint32 count, bool update )
{
    Item *pItem = CreateItem( item, count );
    if( pItem )
    {
        ItemPrototype *pProto = pItem->GetProto();
        if( pProto && pProto->Class == ITEM_CLASS_QUEST )
            ItemAdded( item, count );
        StoreItem( pos, pItem, update );
    }
}

void Player::StoreItem( uint16 pos, Item *pItem, bool update )
{
    if( pItem )
    {
        if( pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP )
            pItem->SetBinding( true );

        uint8 bag = pos >> 8;
        uint8 slot = pos & 255;

        sLog.outDebug( "STORAGE: StoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());

        Item *pItem2 = GetItemByPos( bag, slot );

        if( !pItem2 )
        {
            if( bag == INVENTORY_SLOT_BAG_0 )
            {
                m_items[slot] = pItem;
                SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
                pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
                pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetGUID() );

                pItem->SetSlot( slot );

                if( IsInWorld() && update )
                {
                    pItem->AddToWorld();
                    pItem->SendUpdateToPlayer( this );
                }
            }
            else
            {
                Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
                if( pBag )
                {
                    pBag->StoreItem( slot, pItem, update );
                    if( IsInWorld() && update )
                    {
                        pItem->AddToWorld();
                        pItem->SendUpdateToPlayer( this );
                    }
                }
            }
        }
        else
        {
            pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
            if( IsInWorld() && update )
                pItem2->SendUpdateToPlayer( this );
        }
    }
}

void Player::EquipItem( uint16 pos, Item *pItem, bool update )
{
    if( pItem )
    {
        // check also  BIND_WHEN_PICKED_UP for .additem or .additemset case by GM (not binded at adding to inventory)
        if( pItem->GetProto()->Bonding == BIND_WHEN_EQUIPED || pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP )
            pItem->SetBinding( true );

        uint8 bag = pos >> 8;
        uint8 slot = pos & 255;

        sLog.outDebug( "STORAGE: EquipItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        m_items[slot] = pItem;
        SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
        pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
        pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetGUID() );
        pItem->SetSlot( slot );

        if( slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
        {
            int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
            SetUInt32Value(VisibleBase, pItem->GetEntry());
            SetUInt32Value(VisibleBase + 1, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT));
            SetUInt32Value(VisibleBase + 2, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 3));
            SetUInt32Value(VisibleBase + 3, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 6));
            SetUInt32Value(VisibleBase + 4, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 9));
            SetUInt32Value(VisibleBase + 5, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 12));
            SetUInt32Value(VisibleBase + 6, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 15));
            SetUInt32Value(VisibleBase + 7, pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT + 18));
            SetUInt32Value(VisibleBase + 8, pItem->GetUInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
        }

        if(isAlive())
            _ApplyItemMods(pItem, slot, true);

        if( IsInWorld() && update )
        {
            pItem->AddToWorld();
            pItem->SendUpdateToPlayer( this );
        }
    }
}

void Player::BankItem( uint16 pos, Item *pItem, bool update )
{
    StoreItem( pos, pItem, update);
}

void Player::RemoveItem( uint8 bag, uint8 slot, bool update )
{
    Item *pItem = GetItemByPos( bag, slot );
    if( pItem )
    {
        sLog.outDebug( "STORAGE: RemoveItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        if( bag == INVENTORY_SLOT_BAG_0 )
        {
            m_items[slot] = NULL;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if ( slot >= EQUIPMENT_SLOT_START && slot < INVENTORY_SLOT_BAG_END )
            {
                _ApplyItemMods(pItem, slot, false);
                if ( slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
                {
                    int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
                    for (int i = VisibleBase; i < VisibleBase + 12; ++i)
                        SetUInt32Value(i, 0);
                }
            }
        }
        else
        {
            Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
            if( pBag )
                pBag->RemoveItem(slot, update);
        }
        pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, 0 );
        // pItem->SetUInt64Value( ITEM_FIELD_OWNER, 0 ); not clear owner at remove (it will be set at store). This used in mail and auction code
        pItem->SetSlot( NULL_SLOT );
        if( IsInWorld() && update )
            pItem->SendUpdateToPlayer( this );
    }
}

void Player::RemoveItemCount( uint32 item, uint32 count, bool update )
{
    sLog.outDebug( "STORAGE: RemoveItemCount item = %u, count = %u", item, count);
    Item *pItem;
    uint32 remcount = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                RemoveItem( INVENTORY_SLOT_BAG_0, i, update );
            }
            else
            {
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() && update )
                    pItem->SendUpdateToPlayer( this );
                return;
            }
        }
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pItem = GetItemByPos( i, j );
                    if( pItem && pItem->GetEntry() == item )
                    {
                        if( pItem->GetCount() + remcount <= count )
                        {
                            remcount += pItem->GetCount();
                            RemoveItem( i, j, update );
                        }
                        else
                        {
                            pItem->SetCount( pItem->GetCount() - count + remcount );
                            if( IsInWorld() && update )
                                pItem->SendUpdateToPlayer( this );
                            return;
                        }
                    }
                }
            }
        }
    }
}

void Player::DestroyItem( uint8 bag, uint8 slot, bool update )
{
    Item *pItem = GetItemByPos( bag, slot );
    if( pItem )
    {
        sLog.outDebug( "STORAGE: DestroyItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());
        pItem->SetOwnerGUID(0);
        pItem->SetSlot( 0 );
        pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, 0 );
        pItem->DeleteFromDB();
        ItemPrototype *pProto = pItem->GetProto();

        if( bag == INVENTORY_SLOT_BAG_0 )
        {
            if( pProto && pProto->Class == ITEM_CLASS_QUEST )
                ItemRemoved( pItem->GetEntry(), pItem->GetCount() );

            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if ( slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END )
            {
                _ApplyItemMods(pItem, slot, false);
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 12);
                for (int i = VisibleBase; i < VisibleBase + 12; ++i)
                    SetUInt32Value(i, 0);
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
                        //uint32 enchant_value2 = pEnchant->value2;
                        uint32 enchant_spell_id = pEnchant->spellid;
                        //uint32 enchant_aura_id = pEnchant->aura_id;
                        //uint32 enchant_description = pEnchant->description;
                        //SpellEntry *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);
                        if(enchant_display ==4)
                            SetArmor(GetArmor()-enchant_value1);
                        else if(enchant_display ==2)
                        {
                            SetUInt32Value(UNIT_FIELD_MINDAMAGE,GetUInt32Value(UNIT_FIELD_MINDAMAGE)-enchant_value1);
                            SetUInt32Value(UNIT_FIELD_MAXDAMAGE,GetUInt32Value(UNIT_FIELD_MAXDAMAGE)-enchant_value1);
                        }
                        else
                        {
                            RemoveAurasDueToSpell(enchant_spell_id);
                        }
                    }
                }
            }

            m_items[slot] = NULL;
            if( IsInWorld() && update )
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer( this );
            }
        }
        else
        {
            Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
            if( pBag )
            {
                if( pProto && pProto->Class == ITEM_CLASS_QUEST )
                    ItemRemoved( pItem->GetEntry(), pItem->GetCount() );
                pBag->RemoveItem(slot, update);

                if( IsInWorld() && update )
                {
                    pItem->RemoveFromWorld();
                    pItem->DestroyForPlayer(this);
                }
            }
        }
    }
}

void Player::DestroyItemCount( uint32 item, uint32 count, bool update )
{
    sLog.outDebug( "STORAGE: DestroyItemCount item = %u, count = %u", item, count);
    Item *pItem;
    ItemPrototype *pProto;
    uint32 remcount = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);
            }
            else
            {
                pProto = pItem->GetProto();
                if( pProto && pProto->Class == ITEM_CLASS_QUEST )
                    ItemRemoved( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                return;
            }
        }
    }
    Bag *pBag;
    ItemPrototype *pBagProto;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();
            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pItem = pBag->GetItemByPos(j);
                    if( pItem && pItem->GetEntry() == item )
                    {
                        if( pItem->GetCount() + remcount <= count )
                        {
                            remcount += pItem->GetCount();
                            DestroyItem( i, j, update );
                        }
                        else
                        {
                            pProto = pItem->GetProto();
                            if( pProto && pProto->Class == ITEM_CLASS_QUEST )
                                ItemRemoved( pItem->GetEntry(), count - remcount );
                            pItem->SetCount( pItem->GetCount() - count + remcount );
                            if( IsInWorld() && update )
                                pItem->SendUpdateToPlayer( this );
                            return;
                        }
                    }
                }
            }
        }
    }
}

void Player::SplitItem( uint16 src, uint16 dst, uint32 count )
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos( srcbag, srcslot );
    if( pSrcItem )
    {
        sLog.outDebug( "STORAGE: SplitItem bag = %u, slot = %u, item = %u, count = %u", dstbag, dstslot, pSrcItem->GetEntry(), count);
        Item *pNewItem = CreateItem( pSrcItem->GetEntry(), count );
        if( pNewItem )
        {
            uint16 dest;
            uint8 msg;
            if( IsInventoryPos( dst ) )
            {
                msg = CanStoreItem( dstbag, dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    pSrcItem->SetCount( pSrcItem->GetCount() - count );
                    if( IsInWorld() )
                        pSrcItem->SendUpdateToPlayer( this );
                    StoreItem( dest, pNewItem, true);
                }
                else
                {
                    delete pNewItem;
                    SendEquipError( msg, pSrcItem, NULL );
                }
            }
            else if( IsBankPos ( dst ) )
            {
                msg = CanBankItem( dstbag, dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    pSrcItem->SetCount( pSrcItem->GetCount() - count );
                    if( IsInWorld() )
                        pSrcItem->SendUpdateToPlayer( this );
                    BankItem( dest, pNewItem, true);
                }
                else
                {
                    delete pNewItem;
                    SendEquipError( msg, pSrcItem, NULL );
                }
            }
            else if( IsEquipmentPos ( dst ) )
            {
                msg = CanEquipItem( dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    pSrcItem->SetCount( pSrcItem->GetCount() - count );
                    if( IsInWorld() )
                        pSrcItem->SendUpdateToPlayer( this );
                    EquipItem( dest, pNewItem, true);
                }
                else
                {
                    delete pNewItem;
                    SendEquipError( msg, pSrcItem, NULL );
                }
            }
            return;
        }
    }
    SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, NULL );
}

void Player::SwapItem( uint16 src, uint16 dst )
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos( srcbag, srcslot );
    Item *pDstItem = GetItemByPos( dstbag, dstslot );

    if( pSrcItem )
    {
        sLog.outDebug( "STORAGE: SwapItem bag = %u, slot = %u, item = %u", dstbag, dstslot, pSrcItem->GetEntry());

        if(!isAlive() )
        {
            SendEquipError( EQUIP_ERR_YOU_ARE_DEAD, pSrcItem, pDstItem );
            return;
        }

        if(IsEquipmentPos ( src ) && isInCombat() &&
            pSrcItem->GetProto()->Class != ITEM_CLASS_WEAPON && pSrcItem->GetProto()->Class != ITEM_CLASS_PROJECTILE)
        {
            SendEquipError( EQUIP_ERR_CANT_DO_IN_COMBAT, pSrcItem, pDstItem );
            return;
        }

        if( srcslot == dstbag )
        {
            SendEquipError( EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem );
            return;
        }

        uint16 dest;
        uint8 msg;
        if( !pDstItem )
        {
            if( IsInventoryPos( dst ) )
            {
                msg = CanStoreItem( dstbag, dstslot, dest, pSrcItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    RemoveItem(srcbag, srcslot, true);
                    StoreItem( dest, pSrcItem, true);
                    return;
                }
                else
                    SendEquipError( msg, pSrcItem, NULL );
            }
            else if( IsBankPos ( dst ) )
            {
                msg = CanBankItem( dstbag, dstslot, dest, pSrcItem, false);
                if( msg == EQUIP_ERR_OK )
                {
                    RemoveItem(srcbag, srcslot, true);
                    BankItem( dest, pSrcItem, true);
                    return;
                }
                else
                    SendEquipError( msg, pSrcItem, NULL );
            }
            else if( IsEquipmentPos ( dst ) )
            {
                msg = CanEquipItem( dstslot, dest, pSrcItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    RemoveItem(srcbag, srcslot, true);
                    EquipItem( dest, pSrcItem, true);
                    return;
                }
                else
                    SendEquipError( msg, pSrcItem, NULL );
            }
        }
        else
        {
            if( IsInventoryPos( dst ) )
            {
                if( CanStoreItem( dstbag, dstslot, dest, pSrcItem, false ) == EQUIP_ERR_OK )
                {
                    if( pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetProto()->Stackable )
                    {
                        RemoveItem(srcbag, srcslot, true);
                        StoreItem( dest, pSrcItem, true);
                    }
                    else
                    {
                        pSrcItem->SetCount( pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetProto()->Stackable );
                        pDstItem->SetCount( pSrcItem->GetProto()->Stackable );
                        if( IsInWorld() )
                        {
                            pSrcItem->SendUpdateToPlayer( this );
                            pDstItem->SendUpdateToPlayer( this );
                        }
                    }
                    return;
                }
            }
            else if( IsBankPos ( dst ) )
            {
                if( CanBankItem( dstbag, dstslot, dest, pSrcItem, false ) == EQUIP_ERR_OK )
                {
                    if( pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetProto()->Stackable )
                    {
                        RemoveItem(srcbag, srcslot, true);
                        BankItem( dest, pSrcItem, true);
                    }
                    else
                    {
                        pSrcItem->SetCount( pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetProto()->Stackable );
                        pDstItem->SetCount( pSrcItem->GetProto()->Stackable );
                        if( IsInWorld() )
                        {
                            pSrcItem->SendUpdateToPlayer( this );
                            pDstItem->SendUpdateToPlayer( this );
                        }
                    }
                    return;
                }
            }
            else if( IsEquipmentPos ( dst ) )
            {
                if( CanEquipItem( dstslot, dest, pSrcItem, false ) == EQUIP_ERR_OK )
                {
                    if( pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetProto()->Stackable )
                    {
                        RemoveItem(srcbag, srcslot, true);
                        EquipItem( dest, pSrcItem, true);
                    }
                    else
                    {
                        pSrcItem->SetCount( pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetProto()->Stackable );
                        pDstItem->SetCount( pSrcItem->GetProto()->Stackable );
                        if( IsInWorld() )
                        {
                            pSrcItem->SendUpdateToPlayer( this );
                            pDstItem->SendUpdateToPlayer( this );
                        }
                    }
                    return;
                }
            }
            if( IsInventoryPos( dst ) )
                msg = CanStoreItem( dstbag, dstslot, dest, pSrcItem, true );
            else if( IsBankPos( dst ) )
                msg = CanBankItem( dstbag, dstslot, dest, pSrcItem, true );
            else if( IsEquipmentPos( dst ) )
                msg = CanEquipItem( dstslot, dest, pSrcItem, true );
            if( msg == EQUIP_ERR_OK )
            {
                uint16 dest2;
                if( IsInventoryPos( src ) )
                {
                    msg = CanStoreItem( srcbag, srcslot, dest2, pDstItem, true );
                    if( msg != EQUIP_ERR_OK )
                    {
                        SendEquipError( EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG, pSrcItem, pDstItem );
                        return;
                    }
                }
                else if( IsBankPos( src ) )
                {
                    msg = CanBankItem( srcbag, srcslot, dest2, pDstItem, true );
                    if( msg != EQUIP_ERR_OK )
                    {
                        SendEquipError( EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG, pSrcItem, pDstItem );
                        return;
                    }
                }
                else if( IsEquipmentPos( src ) )
                {
                    msg = CanEquipItem( srcslot, dest2, pDstItem, true);
                    if( msg != EQUIP_ERR_OK )
                    {
                        SendEquipError( EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, pSrcItem, pDstItem );
                        return;
                    }
                }
                RemoveItem(dstbag, dstslot, false);
                RemoveItem(srcbag, srcslot, false);
                if( IsInventoryPos( dst ) )
                    StoreItem(dest, pSrcItem, true);
                else if( IsBankPos( dst ) )
                    BankItem(dest, pSrcItem, true);
                else if( IsEquipmentPos( dst ) )
                    EquipItem(dest, pSrcItem, true);
                if( IsInventoryPos( src ) )
                    StoreItem(dest2, pDstItem, true);
                else if( IsBankPos( src ) )
                    BankItem(dest2, pDstItem, true);
                else if( IsEquipmentPos( src ) )
                    EquipItem(dest2, pDstItem, true);
                return;
            }
            else
                SendEquipError( msg, pSrcItem, pDstItem );
            return;
        }
    }
}

void Player::AddItemToBuyBackSlot( uint32 slot, Item *pItem )
{
    if( pItem )
    {
        if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
        {
            RemoveItemFromBuyBackSlot( slot );
            sLog.outDebug( "STORAGE: AddItemToBuyBackSlot item = %u, slot = %u", pItem->GetEntry(), slot);
            uint32 eslot = slot - BUYBACK_SLOT_START;

            m_buybackitems[eslot] = pItem;
            time_t base = time(NULL);
            time_t etime = base + (30 * 3600);

            SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, pItem->GetGUID() );
            ItemPrototype *pProto = pItem->GetProto();
            if( pProto )
                SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, pProto->SellPrice * pItem->GetCount() );
            else
                SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0 );
            SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, (uint32)etime );
        }
    }
}

Item* Player::GetItemFromBuyBackSlot( uint32 slot )
{
    sLog.outDebug( "STORAGE: GetItemFromBuyBackSlot slot = %u", slot);
    if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
        return m_buybackitems[slot - BUYBACK_SLOT_START];
    return NULL;
}

void Player::RemoveItemFromBuyBackSlot( uint32 slot )
{
    sLog.outDebug( "STORAGE: RemoveItemFromBuyBackSlot slot = %u", slot);
    if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
    {
        uint32 eslot = slot - BUYBACK_SLOT_START;
        Item *pItem = m_buybackitems[eslot];
        if( pItem )
            pItem->RemoveFromWorld();

        m_buybackitems[eslot] = NULL;
        SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, 0 );
        SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0 );
        SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, 0 );
    }
}

void Player::SendEquipError( uint8 msg, Item* pItem, Item *pItem2 )
{
    sLog.outDetail( "WORLD: Sent SMSG_INVENTORY_CHANGE_FAILURE" );
    WorldPacket data;
    data.Initialize( SMSG_INVENTORY_CHANGE_FAILURE );
    data << msg;
    if( msg == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N )
        data << (pItem ? pItem->GetProto()->RequiredLevel : uint32(0));
    data << (pItem ? pItem->GetGUID() : uint64(0));
    data << (pItem2 ? pItem2->GetGUID() : uint64(0));
    data << uint8(0);
    GetSession()->SendPacket(&data);
}

void Player::SendBuyError( uint8 msg, Creature* pCreature, uint32 item, uint32 param )
{
    sLog.outDetail( "WORLD: Sent SMSG_BUY_FAILED" );
    WorldPacket data;
    data.Initialize( SMSG_BUY_FAILED );
    data << (pCreature ? pCreature->GetGUID() : uint64(0));
    data << item;
    if( param > 0 )
        data << param;
    data << msg;
    GetSession()->SendPacket(&data);
}

void Player::SendSellError( uint8 msg, Creature* pCreature, uint64 guid, uint32 param )
{
    sLog.outDetail( "WORLD: Sent SMSG_SELL_ITEM" );
    WorldPacket data;
    data.Initialize( SMSG_SELL_ITEM );
    data << (pCreature ? pCreature->GetGUID() : uint64(0));
    data << guid;
    if( param > 0 )
        data << param;
    data << msg;
    GetSession()->SendPacket(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void Player::PrepareQuestMenu( uint64 guid )
{
    Object *pObject;
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
    if( pCreature )
        pObject = (Object*)pCreature;
    else
    {
        GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*this, guid);
        if( pGameObject )
            pObject = (Object*)pGameObject;
        else
            return;
    }

    uint32 status;
    Quest *pQuest;
    QuestMenu *qm = PlayerTalkClass->GetQuestMenu();
    qm->ClearMenu();

    for( std::list<Quest*>::iterator i = pObject->mInvolvedQuests.begin( ); i != pObject->mInvolvedQuests.end( ); i++ )
    {
        pQuest = *i;
        if ( !pQuest )
            continue;

        status = GetQuestStatus( pQuest );
        if ( status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus( pQuest ) )
            qm->AddMenuItem( pQuest->GetQuestInfo()->QuestId, DIALOG_STATUS_REWARD, false );
        else if ( status == QUEST_STATUS_INCOMPLETE )
            qm->AddMenuItem( pQuest->GetQuestInfo()->QuestId, DIALOG_STATUS_INCOMPLETE, false );
    }

    for( std::list<Quest*>::iterator i = pObject->mQuests.begin( ); i != pObject->mQuests.end( ); i++ )
    {
        pQuest = *i;
        if ( !pQuest )
            continue;

        status = GetQuestStatus( pQuest );
        if ( status == QUEST_STATUS_NONE && CanTakeQuest( pQuest, false ) )
            qm->AddMenuItem( pQuest->GetQuestInfo()->QuestId, DIALOG_STATUS_AVAILABLE, true );
    }
}

void Player::SendPreparedQuest( uint64 guid )
{
    QuestMenu* pQuestMenu = PlayerTalkClass->GetQuestMenu();
    if( !pQuestMenu || pQuestMenu->MenuItemCount() < 1 )
        return;

    uint32 status = pQuestMenu->GetItem(0).m_qIcon;
    if ( pQuestMenu->MenuItemCount() == 1 )
    {
        Quest *pQuest = objmgr.GetQuest( pQuestMenu->GetItem(0).m_qId );
        if ( pQuest )
        {
            if( status == DIALOG_STATUS_REWARD && !GetQuestRewardStatus( pQuest ) )
                PlayerTalkClass->SendRequestedItems( pQuest, guid, true, true );
            else if( status == DIALOG_STATUS_INCOMPLETE )
                PlayerTalkClass->SendRequestedItems( pQuest, guid, false, true );
            else
                PlayerTalkClass->SendQuestDetails( pQuest, guid, true );
        }
    }
    else
    {
        QEmote qe;
        qe._Delay = 0;
        qe._Emote = 0;
        std::string title = "";
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
        if( pCreature )
        {
            uint32 textid = pCreature->GetNpcTextId();
            GossipText * gossiptext = objmgr.GetGossipText(textid);
            if( !gossiptext )
            {
                qe._Delay = TEXTEMOTE_MASSAGE;              //zyg: player emote
                qe._Emote = TEXTEMOTE_HELLO;                //zyg: NPC emote
                title = "Do Quest ?";
            }
            else
            {
                qe = gossiptext->Options[0].Emotes[0];
                title = gossiptext->Options[0].Text_0;
                if( &title == NULL )
                    title = "";
            }
        }
        PlayerTalkClass->SendQuestMenu( qe, title, guid );
    }
}

Quest *Player::GetActiveQuest( uint32 quest_id ) const
{
    StatusMap::const_iterator itr = mQuestStatus.find(quest_id);

    return (itr != mQuestStatus.end()) ?  itr->second.m_quest : NULL;
}

Quest* Player::GetNextQuest( uint64 guid, Quest *pQuest )
{
    if( pQuest )
    {
        Object *pObject;
        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
        if( pCreature )
            pObject = (Object*)pCreature;
        else
        {
            GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*this, guid);
            if( pGameObject )
                pObject = (Object*)pGameObject;
            else
                return NULL;;
        }

        uint32 quest = pQuest->GetQuestInfo()->NextQuestId;
        for( std::list<Quest*>::iterator i = pObject->mQuests.begin( ); i != pObject->mQuests.end( ); i++ )
        {
            Quest *pQuest2 = *i;
            if( pQuest2->GetQuestInfo()->QuestId == quest )
            {
                if ( CanTakeQuest( pQuest2, false ) )
                    return pQuest2;
                else
                    return NULL;
            }
        }
    }
    return NULL;
}

bool Player::CanSeeStartQuest( Quest *pQuest )
{
    if( pQuest )
    {
        if( SatisfyQuestRace( pQuest, false ) && SatisfyQuestClass( pQuest, false ) && SatisfyQuestSkill( pQuest, false ) && SatisfyQuestReputation( pQuest, false ) && SatisfyQuestPreviousQuest( pQuest, false ) )
            return ( (int32)getLevel() >= (int32)pQuest->GetQuestInfo()->MinLevel - (int32)7 );
    }
    return false;
}

bool Player::CanTakeQuest( Quest *pQuest, bool msg )
{
    if( pQuest )
        return ( SatisfyQuestStatus( pQuest, msg ) && SatisfyQuestRace( pQuest, msg ) && SatisfyQuestLevel( pQuest, msg ) && SatisfyQuestClass( pQuest, msg ) && SatisfyQuestSkill( pQuest, msg ) && SatisfyQuestReputation( pQuest, msg ) && SatisfyQuestPreviousQuest( pQuest, msg ) && SatisfyQuestTimed( pQuest, msg ) );
    return false;
}

bool Player::CanAddQuest( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        if( !SatisfyQuestLog( msg ) )
            return false;

        uint32 srcitem = pQuest->GetQuestInfo()->SrcItemId;
        if( srcitem > 0 )
        {
            uint32 count = pQuest->GetQuestInfo()->SrcItemCount;
            uint16 dest;
            if( count <= 0 )
                count = 1;
            uint8 msg = CanStoreNewItem( 0, NULL_SLOT, dest, srcitem, count, false );
            if( msg != EQUIP_ERR_OK )
            {
                SendEquipError( msg, NULL, NULL );
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Player::CanCompleteQuest( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;

        if( mQuestStatus[quest].m_status == QUEST_STATUS_COMPLETE )
            return true;

        if ( mQuestStatus[quest].m_status == QUEST_STATUS_INCOMPLETE )
        {
            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( pQuest->GetQuestInfo()->ReqItemCount[i]!= 0 && mQuestStatus[quest].m_itemcount[i] < pQuest->GetQuestInfo()->ReqItemCount[i] )
                        return false;
                }
            }

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    // GO activate objectives
                    if( pQuest->GetQuestInfo()->ReqKillMobOrGOId <= 0 )
                        continue;

                    if( pQuest->GetQuestInfo()->ReqKillMobOrGOCount[i] != 0 && mQuestStatus[quest].m_mobcount[i] < pQuest->GetQuestInfo()->ReqKillMobOrGOCount[i] )
                        return false;
                }
            }

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_EXPLORATION ) && !mQuestStatus[quest].m_explored )
                return false;

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) && mQuestStatus[quest].m_timer == 0 )
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
    if( pQuest )
    {
        uint16 dest;
        uint8 msg;
        if ( pQuest->m_rewchoiceitemscount > 0 )
        {
            if( pQuest->GetQuestInfo()->RewChoiceItemId[reward] )
            {
                msg = CanStoreNewItem( 0, NULL_SLOT, dest, pQuest->GetQuestInfo()->RewChoiceItemId[reward], pQuest->GetQuestInfo()->RewChoiceItemCount[reward], false );
                if( msg != EQUIP_ERR_OK )
                {
                    SendEquipError( msg, NULL, NULL );
                    return false;
                }
            }
        }

        if ( pQuest->m_rewitemscount > 0 )
        {
            for (int i = 0; i < QUEST_REWARDS_COUNT; i++)
            {
                if( pQuest->GetQuestInfo()->RewItemId[i] )
                {
                    msg = CanStoreNewItem( 0, NULL_SLOT, dest, pQuest->GetQuestInfo()->RewItemId[i], pQuest->GetQuestInfo()->RewItemCount[i], false );
                    if( msg != EQUIP_ERR_OK )
                    {
                        SendEquipError( msg, NULL, NULL );
                        return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

void Player::AddQuest( Quest *pQuest )
{
    if( pQuest )
    {
        uint16 log_slot = GetQuestSlot( NULL );
        if( log_slot )
        {
            uint32 quest = pQuest->GetQuestInfo()->QuestId;

            mQuestStatus[quest].m_quest = pQuest;
            mQuestStatus[quest].m_status = QUEST_STATUS_INCOMPLETE;
            mQuestStatus[quest].m_rewarded = false;
            mQuestStatus[quest].m_explored = false;

            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                    mQuestStatus[quest].m_itemcount[i] = 0;
            }
            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                    mQuestStatus[quest].m_mobcount[i] = 0;
            }

            GiveQuestSourceItem( pQuest );
            AdjustQuestReqItemCount( pQuest );

            SetUInt32Value(log_slot + 0, quest);
            SetUInt32Value(log_slot + 1, 0);

            if( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
            {
                uint32 limittime = pQuest->GetQuestInfo()->LimitTime;
                SetTimedQuest( pQuest );
                mQuestStatus[quest].m_timer = limittime * 60000;
                uint64 ktime = 0; // unkwnown and dependent from server start time
                uint32 qtime = static_cast<uint32>(time(NULL) - ktime) + limittime;
                SetUInt32Value( log_slot + 2, qtime );
            }
            else
            {
                mQuestStatus[quest].m_timer = 0;
                SetUInt32Value( log_slot + 2, 0 );
            }
        }
    }
}

void Player::CompleteQuest( Quest *pQuest )
{
    if( pQuest )
    {
        SetQuestStatus( pQuest, QUEST_STATUS_COMPLETE);

        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
        {
            uint32 state = GetUInt32Value( log_slot + 1 );
            state |= 1 << 24;
            SetUInt32Value( log_slot + 1, state );
        }

        SendQuestComplete( pQuest );
    }
}

void Player::IncompleteQuest( Quest *pQuest )
{
    if( pQuest )
    {
        SetQuestStatus( pQuest, QUEST_STATUS_INCOMPLETE );

        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
        {
            uint32 state = GetUInt32Value( log_slot + 1 );
            state &= ~(1 << 24);
            SetUInt32Value( log_slot + 1, state );
        }
    }
}

void Player::RewardQuest( Quest *pQuest, uint32 reward )
{
    if( pQuest )
    {
        uint16 dest;
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++ )
        {
            if ( pQuest->GetQuestInfo()->ReqItemId[i] )
                RemoveItemCount( pQuest->GetQuestInfo()->ReqItemId[i], pQuest->GetQuestInfo()->ReqItemCount[i], true);
        }

        if( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
            SetTimedQuest( 0 );
        if ( pQuest->m_rewchoiceitemscount > 0 )
        {
            if( pQuest->GetQuestInfo()->RewChoiceItemId[reward] )
            {
                if( CanStoreNewItem( 0, NULL_SLOT, dest, pQuest->GetQuestInfo()->RewChoiceItemId[reward], pQuest->GetQuestInfo()->RewChoiceItemCount[reward], false ) == EQUIP_ERR_OK )
                    StoreNewItem( dest, pQuest->GetQuestInfo()->RewChoiceItemId[reward], pQuest->GetQuestInfo()->RewChoiceItemCount[reward], true);
            }
        }

        if ( pQuest->m_rewitemscount > 0 )
        {
            for (int i=0; i < QUEST_REWARDS_COUNT; i++)
            {
                if( pQuest->GetQuestInfo()->RewItemId[i] )
                {
                    if( CanStoreNewItem( 0, NULL_SLOT, dest, pQuest->GetQuestInfo()->RewItemId[i], pQuest->GetQuestInfo()->RewItemCount[i], false ) == EQUIP_ERR_OK )
                        StoreNewItem( dest, pQuest->GetQuestInfo()->RewItemId[i], pQuest->GetQuestInfo()->RewItemCount[i], true);
                }
            }
        }

        if ( pQuest->GetQuestInfo()->RewSpell > 0 )
        {
            WorldPacket sdata;

            sdata.Initialize (SMSG_LEARNED_SPELL);
            sdata << pQuest->GetQuestInfo()->RewSpell;
            GetSession()->SendPacket( &sdata );
            addSpell( (uint16)pQuest->GetQuestInfo()->RewSpell,1);
        }

        uint32 quest = pQuest->GetQuestInfo()->QuestId;

        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
        {
            SetUInt32Value(log_slot + 0, 0);
            SetUInt32Value(log_slot + 1, 0);
            SetUInt32Value(log_slot + 2, 0);
        }

        if ( getLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
            GiveXP( pQuest->XPValue( this ), NULL );
        else
            ModifyMoney( MaNGOS::XP::xp_to_money(pQuest->XPValue( this )) );

        ModifyMoney( pQuest->GetQuestInfo()->RewMoney );

        if ( !pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ) )
            mQuestStatus[quest].m_rewarded = true;
        else
            SetQuestStatus(pQuest, QUEST_STATUS_NONE);

        SendQuestReward( pQuest );
    }
}

void Player::FailQuest( Quest *pQuest )
{
    if( pQuest )
    {
        IncompleteQuest( pQuest );

        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
            SetUInt32Value( log_slot + 2, 1 );
        SendQuestFailed( pQuest );
    }
}

void Player::FailTimedQuest( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;

        mQuestStatus[quest].m_timer = 0;

        IncompleteQuest( pQuest );

        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
            SetUInt32Value( log_slot + 2, 1 );
        SendQuestTimerFailed( pQuest );
    }
}

bool Player::SatisfyQuestClass( Quest *pQuest, bool msg )
{
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
        return true;

    }
    return false;
}

bool Player::SatisfyQuestLevel( Quest *pQuest, bool msg )
{
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
        {
            WorldPacket data;
            data.Initialize( SMSG_QUESTLOG_FULL );
            GetSession()->SendPacket( &data );
            sLog.outDebug( "WORLD: Sent QUEST_LOG_FULL_MESSAGE" );
        }
        return false;
    }
}

bool Player::SatisfyQuestPreviousQuest( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        uint32 questId = pQuest->GetQuestInfo()->QuestId;

        QuestRelations::iterator iter = sPrevQuests.lower_bound(questId);
        QuestRelations::iterator end  = sPrevQuests.upper_bound(questId);

        // First quest in series
        if(iter == end)
            return true;

        // Have one form prev, quests in rewarded state
        for(; iter != end; ++iter )
        {
            uint32 prevId = iter->second;

            if( mQuestStatus.find( prevId ) != mQuestStatus.end() && mQuestStatus[prevId].m_rewarded )
                return true;
        }

        // Have only prev. quests in non-rewarded state
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
    }
    return false;
}

bool Player::SatisfyQuestRace( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        uint32 reqraces = pQuest->GetQuestInfo()->RequiredRaces;
        if ( reqraces == QUEST_RACE_NONE )
            return true;
        if( (reqraces & getRaceMask()) == 0 )
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
    if( pQuest )
    {
        uint32 faction_id = pQuest->GetQuestInfo()->RequiredRepFaction;
        if(!faction_id)
            return true;

        return GetReputation(faction_id) >= pQuest->GetQuestInfo()->RequiredRepValue;
    }
    return false;
}

bool Player::SatisfyQuestSkill( Quest *pQuest, bool msg )
{
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
        if ( GetTimedQuest() != 0 && pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED) )
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
    if( pQuest )
    {
        uint32 srcitem = pQuest->GetQuestInfo()->SrcItemId;
        if( srcitem > 0 )
        {
            uint16 dest;
            uint32 count = pQuest->GetQuestInfo()->SrcItemCount;
            if( count <= 0 )
                count = 1;
            uint8 msg = CanStoreNewItem( 0, NULL_SLOT, dest, srcitem, count, false );
            if( msg == EQUIP_ERR_OK )
            {
                StoreNewItem(dest, srcitem, count, true);
                return true;
            }
            else
                SendEquipError( msg, NULL, NULL );
            return false;
        }
    }
    return true;
}

void Player::TakeQuestSourceItem( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 srcitem = pQuest->GetQuestInfo()->SrcItemId;
        if( srcitem > 0 )
        {
            uint32 count = pQuest->GetQuestInfo()->SrcItemCount;
            if( count <= 0 )
                count = 1;
            DestroyItemCount(srcitem, count, true);
        }
    }
}

bool Player::GetQuestRewardStatus( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if  ( mQuestStatus.find( quest ) == mQuestStatus.end() )
            return false;
        return mQuestStatus[quest].m_rewarded;
    }
    return false;
}

uint32 Player::GetQuestStatus( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if  ( mQuestStatus.find( quest ) != mQuestStatus.end() )
            return mQuestStatus[quest].m_status;
    }
    return QUEST_STATUS_NONE;
}

void Player::SetQuestStatus( Quest *pQuest, uint32 status )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        if ( status == QUEST_STATUS_NONE )
        {
            mQuestStatus.erase( quest );

            if( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
                SetTimedQuest( 0 );
        }
        else
            mQuestStatus[quest].m_status = status;
    }
}

void Player::AdjustQuestReqItemCount( Quest *pQuest )
{
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
                if( reqitemcount != 0 )
                {
                    curitemcount = GetItemCount(pQuest->GetQuestInfo()->ReqItemId[i]) + GetBankItemCount(pQuest->GetQuestInfo()->ReqItemId[i]);
                    mQuestStatus[quest].m_itemcount[i] = min(curitemcount, reqitemcount);
                }
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

void Player::AreaExplored( Quest *pQuest )
{
    if( pQuest )
    {
        uint16 log_slot = GetQuestSlot( pQuest );
        if( log_slot )
        {
            uint32 quest = pQuest->GetQuestInfo()->QuestId;
            mQuestStatus[quest].m_explored = true;
        }
        if( CanCompleteQuest( pQuest ) )
            CompleteQuest( pQuest );
    }
}

void Player::ItemAdded( uint32 entry, uint32 count )
{
    uint32 quest;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 additemcount;
    for( int i = 0; i < 20; i++ )
    {
        quest = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        if ( quest != 0 && mQuestStatus[quest].m_status == QUEST_STATUS_INCOMPLETE )
        {
            Quest *pQuest = mQuestStatus[quest].m_quest;
            if( pQuest && pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    reqitem = pQuest->GetQuestInfo()->ReqItemId[j];
                    if ( reqitem == entry )
                    {
                        reqitemcount = pQuest->GetQuestInfo()->ReqItemCount[j];
                        curitemcount = mQuestStatus[quest].m_itemcount[j];
                        if ( curitemcount < reqitemcount )
                        {
                            additemcount = ( curitemcount + count <= reqitemcount ? count : reqitemcount - curitemcount);
                            mQuestStatus[quest].m_itemcount[j] += additemcount;
                            SendQuestUpdateAddItem( pQuest, j, additemcount );
                        }
                        if ( CanCompleteQuest( pQuest ) )
                            CompleteQuest( pQuest );
                        return;
                    }
                }
            }
        }
    }
}

void Player::ItemRemoved( uint32 entry, uint32 count )
{
    uint32 quest;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 remitemcount;
    for( int i = 0; i < 20; i++ )
    {
        quest = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        if ( quest != 0 )
        {
            Quest *pQuest = mQuestStatus[quest].m_quest;
            if( pQuest && pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    reqitem = pQuest->GetQuestInfo()->ReqItemId[j];
                    if ( reqitem == entry )
                    {
                        reqitemcount = pQuest->GetQuestInfo()->ReqItemCount[j];
                        if( mQuestStatus[quest].m_status != QUEST_STATUS_COMPLETE )
                            curitemcount = mQuestStatus[quest].m_itemcount[j];
                        else
                            curitemcount = GetItemCount(entry) + GetBankItemCount(entry);
                        if ( curitemcount - count < reqitemcount )
                        {
                            remitemcount = ( curitemcount <= reqitemcount ? count : count + reqitemcount - curitemcount);
                            mQuestStatus[quest].m_itemcount[j] = curitemcount - remitemcount;
                            IncompleteQuest( pQuest );
                        }
                        return;
                    }
                }
            }
        }
    }
}

void Player::KilledMonster( uint32 entry, uint64 guid )
{
    uint32 quest;
    uint32 reqkill;
    uint32 reqkillcount;
    uint32 curkillcount;
    uint32 addkillcount = 1;
    for( int i = 0; i < 20; i++ )
    {
        quest = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        if ( quest != 0 && mQuestStatus[quest].m_status == QUEST_STATUS_INCOMPLETE )
        {
            Quest *pQuest = mQuestStatus[quest].m_quest;
            if( pQuest && pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    reqkill = mQuestStatus[quest].m_quest->GetQuestInfo()->ReqKillMobOrGOId[j];

                    // GO activate qobjective or none
                    if(reqkill <=0)
                        continue;

                    if ( reqkill == entry )
                    {
                        reqkillcount = pQuest->GetQuestInfo()->ReqKillMobOrGOCount[j];
                        curkillcount = mQuestStatus[quest].m_mobcount[j];
                        if ( curkillcount < reqkillcount )
                        {
                            mQuestStatus[quest].m_mobcount[j] = curkillcount + addkillcount;
                            SendQuestUpdateAddKill( pQuest, guid, j, curkillcount, addkillcount);
                        }
                        if ( CanCompleteQuest( pQuest ) )
                            CompleteQuest( pQuest );
                        return;
                    }
                }
            }
        }
    }
}

bool Player::HaveQuestForItem( uint32 itemid )
{
    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        quest_status qs=i->second;

        if (qs.m_status == QUEST_STATUS_INCOMPLETE)
        {
            if (!qs.m_quest) continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                QuestInfo* qinfo = qs.m_quest->GetQuestInfo();
                if(itemid == qinfo->ReqItemId[j] && qs.m_itemcount[j] < qinfo->ReqItemCount[j] )
                    return true;
            }
        }
    }
    return false;
}

void Player::SendQuestComplete( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        WorldPacket data;
        data.Initialize( SMSG_QUESTUPDATE_COMPLETE );
        data << quest;
        GetSession()->SendPacket( &data );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_COMPLETE quest = %u", quest );
    }
}

void Player::SendQuestReward( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE quest = %u", quest );
        WorldPacket data;
        data.Initialize( SMSG_QUESTGIVER_QUEST_COMPLETE );
        data << quest;
        data << uint32(0x03);
        if ( getLevel() < 60 )
        {
            data << pQuest->XPValue( this );
            data << pQuest->GetQuestInfo()->RewMoney;
        }
        else
        {
            data << uint32(0);
            data << pQuest->GetQuestInfo()->RewMoney + pQuest->XPValue( this );
        }
        data << uint32( pQuest->m_rewitemscount );

        for (int i = 0; i < QUEST_REWARDS_COUNT; i++)
        {
            if ( pQuest->GetQuestInfo()->RewItemId[i] > 0 )
                data << pQuest->GetQuestInfo()->RewItemId[i] << pQuest->GetQuestInfo()->RewItemCount[i];
        }

        GetSession()->SendPacket( &data );
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

void Player::SendQuestTimerFailed( Quest *pQuest )
{
    if( pQuest )
    {
        uint32 quest = pQuest->GetQuestInfo()->QuestId;
        WorldPacket data;
        data.Initialize( SMSG_QUESTUPDATE_FAILEDTIMER );
        data << quest;
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_FAILEDTIMER");
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

void Player::SendQuestUpdateAddItem( Quest *pQuest, uint32 item_idx, uint32 count )
{
    if( pQuest )
    {
        WorldPacket data;
        data.Initialize( SMSG_QUESTUPDATE_ADD_ITEM );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_ADD_ITEM" );
        data << pQuest->GetQuestInfo()->ReqItemId[item_idx];
        data << count;
        GetSession()->SendPacket( &data );
    }
}

void Player::SendQuestUpdateAddKill( Quest *pQuest, uint64 guid, uint32 creature_idx, uint32 old_count, uint32 add_count )
{
    assert(old_count + add_count < 64 && "mob count store in 6 bits 2^6 = 64 (0..63)");
    if( pQuest )
    {
        WorldPacket data;
        data.Initialize( SMSG_QUESTUPDATE_ADD_KILL );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_ADD_KILL" );
        data << pQuest->GetQuestInfo()->QuestId;
        data << pQuest->GetQuestInfo()->ReqKillMobOrGOId[ creature_idx ];
        data << old_count + add_count;
        data << pQuest->GetQuestInfo()->ReqKillMobOrGOCount[ creature_idx ];
        data << guid;
        GetSession()->SendPacket(&data);

        uint16 log_slot = GetQuestSlot( pQuest );
        uint32 kills = GetUInt32Value( log_slot + 1 );
        kills = kills + (add_count << ( 6 * creature_idx ));
        SetUInt32Value( log_slot + 1, kills );
    }
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/

bool Player::LoadFromDB( uint32 guid )
{

    QueryResult *result = sDatabase.PQuery("SELECT `guid`,`realm`,`account`,`data`,`name`,`race`,`class`,`position_x`,`position_y`,`position_z`,`map`,`orientation`,`taximask`,`online`,`highest_rank`,`standing`, `rating`,`cinematic` FROM `character` WHERE `guid` = '%lu';",(unsigned long)guid);

    if(!result)
        return false;

    Field *fields = result->Fetch();

    Object::_Create( guid, HIGHGUID_PLAYER );

    LoadValues( fields[3].GetString() );
    m_drunk = GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF;

    m_name = fields[4].GetCppString();

    sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());
    outDebugValues();

    m_race = fields[5].GetUInt8();
    //Need to call it to initialize m_team (m_team can be calculated from m_race)
    //Other way is to saves m_team into characters table.
    setFactionForRace(m_race);
    SetCharm(0);

    m_class = fields[6].GetUInt8();

    info = objmgr.GetPlayerCreateInfo(m_race, m_class);

    m_positionX = fields[7].GetFloat();
    m_positionY = fields[8].GetFloat();
    m_positionZ = fields[9].GetFloat();
    m_mapId = fields[10].GetUInt32();
    m_orientation = fields[11].GetFloat();
    m_highest_rank = fields[14].GetUInt32();
    m_standing = fields[15].GetUInt32();
    m_rating = fields[16].GetFloat();
    m_cinematic = fields[17].GetUInt32();

    if( HasFlag(PLAYER_FLAGS, 8) )
        SetUInt32Value(PLAYER_FLAGS, 0);

    if( HasFlag(PLAYER_FLAGS, 0x11) )
        m_deathState = DEAD;

    LoadTaxiMask( fields[12].GetString() );

    delete result;

    _LoadMail();

    _LoadSpells();

    _LoadActions();

    _LoadQuestStatus();

    _LoadTutorials();

    _LoadBids();

    _LoadAuras();

    _LoadInventory();

    _LoadReputation();

    _LoadCorpse();

    _LoadPet();

    // Skip _ApplyAllAuraMods(); -- applied in _LoadAuras by AddAura calls at aura load
    // Skip _ApplyAllItemMods(); -- applied in _LoadInventory() by EquipItem calls at item load

    sLog.outDebug("The value of player %s after load item and aura is: ", m_name.c_str());
    outDebugValues();

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
            if(!spellproto)
            {
                sLog.outError("Unknown aura (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            Aura* aura = new Aura(spellproto, effindex, this, this);
            if (remaintime == -1)
            {
                sLog.outDebug("SpellAura (id=%u) has duration:%d ", spellid, remaintime);
                //continue;
                //temporary disable the Aura with Druation=-1 to avoid spell lost and action lost.
                //need more fix about Aura Reload.
            }
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
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `game_corpse` WHERE `player` = '%u' AND `bones_flag` = '0';",GetGUIDLow());

    if(!result) return;

    Field *fields = result->Fetch();

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
    for(int i = EQUIPMENT_SLOT_START; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            delete m_items[i];
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + i*2), 0);
            m_items[i] = 0;
        }
    }

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '%u';",GetGUIDLow(),INVENTORY_SLOT_BAG_0);

    uint16 dest;
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint8  slot      = fields[2].GetUInt8();
            uint32 item_guid = fields[3].GetUInt32();
            uint32 item_id   = fields[4].GetUInt32();

            ItemPrototype* proto = objmgr.GetItemPrototype(item_id);

            if(!proto)
            {
                sLog.outError( "Player::_LoadInventory: Player %s have unknown item (id: #%u) in inventory, skipped.", GetName(),item_id );
                continue;
            }

            Item *item = NewItemOrBag(proto);
            item->SetSlot(slot);

            if(!item->LoadFromDB(item_guid, GetGUID(), 1))
                continue;

            dest = ((INVENTORY_SLOT_BAG_0 << 8) | slot);
            if( IsInventoryPos( dest ) )
            {
                if( CanStoreItem( INVENTORY_SLOT_BAG_0, slot, dest, item, false ) == EQUIP_ERR_OK )
                    StoreItem(dest, item, true);
                else
                    delete item;
            }
            else if( IsEquipmentPos( dest ) )
            {
                if( CanEquipItem( slot, dest, item, false, false ) == EQUIP_ERR_OK )
                    EquipItem(dest, item, true);
                else
                    delete item;
            }
            else if( IsBankPos( dest ) )
            {
                if( CanBankItem( INVENTORY_SLOT_BAG_0, slot, dest, item, false ) == EQUIP_ERR_OK )
                    BankItem(dest, item, true);
                else
                    delete item;
            }
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
            be->subject = fields[3].GetCppString();
            be->body = fields[4].GetCppString();
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

void Player::_LoadPet()
{
    uint64 pet_guid = GetPetGUID();
    if(pet_guid)
    {
        Creature* in_pet = ObjectAccessor::Instance().GetCreature(*this, pet_guid);
        if(in_pet)
            return;
        Pet *pet = new Pet();
        pet->LoadPetFromDB(this);
    }
}

void Player::_LoadQuestStatus()
{
    mQuestStatus.clear();

    Quest *pQuest;
    uint32 quest;

    QueryResult *result = sDatabase.PQuery("SELECT `quest`,`status`,`rewarded`,`explored`,`timer`,`mobcount1`,`mobcount2`,`mobcount3`,`mobcount4`,`itemcount1`,`itemcount2`,`itemcount3`,`itemcount4` FROM `character_queststatus` WHERE `guid` = '%u';", GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            pQuest = objmgr.GetQuest(fields[0].GetUInt32());
            if( pQuest )
            {
                quest = pQuest->GetQuestInfo()->QuestId;

                mQuestStatus[quest].m_quest = pQuest;
                mQuestStatus[quest].m_status = fields[1].GetUInt32();
                mQuestStatus[quest].m_rewarded = ( fields[2].GetUInt32() > 0 );
                mQuestStatus[quest].m_explored = ( fields[3].GetUInt32() > 0 );

                if( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) && !mQuestStatus[quest].m_rewarded )
                    SetTimedQuest( pQuest );

                mQuestStatus[quest].m_timer = 0;
                mQuestStatus[quest].m_mobcount[0] = fields[5].GetUInt32();
                mQuestStatus[quest].m_mobcount[1] = fields[6].GetUInt32();
                mQuestStatus[quest].m_mobcount[2] = fields[7].GetUInt32();
                mQuestStatus[quest].m_mobcount[3] = fields[8].GetUInt32();
                mQuestStatus[quest].m_itemcount[0] = fields[9].GetUInt32();
                mQuestStatus[quest].m_itemcount[1] = fields[10].GetUInt32();
                mQuestStatus[quest].m_itemcount[2] = fields[11].GetUInt32();
                mQuestStatus[quest].m_itemcount[3] = fields[12].GetUInt32();

                sLog.outDebug("Quest status is {%u} for quest {%u}", mQuestStatus[quest].m_status, quest);
            }
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
        //LoadReputationFromDBC();
        //Set initial reputations
        SetInitialFactions();
    }
}

void Player::_LoadSpells()
{

    m_spells.clear();

    QueryResult *result = sDatabase.PQuery("SELECT `spell`,`slot`,`active` FROM `character_spell` WHERE `guid` = '%u';",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            addSpell(fields[0].GetUInt16(), fields[2].GetUInt8(), fields[1].GetUInt16());
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
    if (isInFlight())
    {
        return;
        SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
        RemoveFlag( UNIT_FIELD_FLAGS ,UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_MOUNT );
    }

    // Set player sit state to standing on save
    RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);

    //remove restflag when save
    //this is becouse of the rename char stuff
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

    sLog.outDebug("The value of player %s before unload item and aura is: ", m_name.c_str());
    outDebugValues();

    if(isAlive())
    {
        _RemoveAllItemMods();
        _RemoveAllAuraMods();
    }

    bool inworld = IsInWorld();
    if (inworld)
        RemoveFromWorld();

    std::stringstream ss;

    sDatabase.PExecute("DELETE FROM `character` WHERE `guid` = '%u'",GetGUIDLow());

    ss.rdbuf()->str("");
    ss << "INSERT INTO `character` (`guid`,`account`,`name`,`race`,`class`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`data`,`taximask`,`online`,`highest_rank`,`standing`,`rating`,`cinematic`) VALUES ("
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
    ss << m_standing;

    ss << ", ";
    ss << m_rating;

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
    SavePet();

    if(m_pCorpse) m_pCorpse->SaveToDB(false);

    Creature *OldSummon = GetPet();
    if(OldSummon && OldSummon->isPet())
    {
        ((Pet*)OldSummon)->SavePetToDB();
    }

    sLog.outDebug("Save Basic value of player %s is: ", m_name.c_str());
    outDebugValues();

    if(isAlive())
    {
        _ApplyAllAuraMods();
        _ApplyAllItemMods();
    }

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

    AuraMap const& auras = GetAuras();
    for(AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        sDatabase.PExecute("INSERT INTO `character_aura` (`guid`,`spell`,`effect_index`,`remaintime`) VALUES ('%u', '%u', '%u', '%d');", GetGUIDLow(), (uint32)(*itr).second->GetId(), (uint32)(*itr).second->GetEffIndex(), int((*itr).second->GetAuraDuration()));
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
    sDatabase.PExecute("DELETE FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '%u';",GetGUIDLow(), INVENTORY_SLOT_BAG_0);

    for(int i = EQUIPMENT_SLOT_START; i < BANK_SLOT_BAG_END; i++)
    {
        if ( m_items[i] != 0 )
        {
            sDatabase.PExecute("INSERT INTO `character_inventory` (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), INVENTORY_SLOT_BAG_0, i, m_items[i]->GetGUIDLow(), m_items[i]->GetEntry());
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
    sDatabase.PExecute("DELETE FROM `character_queststatus` WHERE `guid` = '%u'",GetGUIDLow());

    for( StatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++ i )
    {
        sDatabase.PExecute("INSERT INTO `character_queststatus` (`guid`,`quest`,`status`,`rewarded`,`explored`,`timer`,`mobcount1`,`mobcount2`,`mobcount3`,`mobcount4`,`itemcount1`,`itemcount2`,`itemcount3`,`itemcount4`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), i->first, i->second.m_status, i->second.m_rewarded, i->second.m_explored, i->second.m_timer, i->second.m_mobcount[0], i->second.m_mobcount[1], i->second.m_mobcount[2], i->second.m_mobcount[3], i->second.m_itemcount[0], i->second.m_itemcount[1], i->second.m_itemcount[2], i->second.m_itemcount[3]);
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

    for (PlayerSpellList::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        sDatabase.PQuery("INSERT INTO `character_spell` (`guid`,`spell`,`slot`,`active`) VALUES ('%u', '%u', '%u','%u');", GetGUIDLow(), (*itr)->spellId, (*itr)->slotId,(*itr)->active);
    }
}

void Player::_SaveTutorials()
{
    sDatabase.PExecute("DELETE FROM `character_tutorial` WHERE `guid` = '%u'",GetGUIDLow());
    sDatabase.PExecute("INSERT INTO `character_tutorial` (`guid`,`tut0`,`tut1`,`tut2`,`tut3`,`tut4`,`tut5`,`tut6`,`tut7`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u');", GetGUIDLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
}

void Player::SavePet()
{
    Creature* pet = GetPet();
    if(pet)
    {
        std::string name;
        uint32 actState;
        name = GetName();
        name.append("\\\'s Pet");
        actState = STATE_RA_FOLLOW;

        sDatabase.PExecute("DELETE FROM `character_pet` WHERE `owner` = '%u' AND `current` = 1", GetGUIDLow() );
        sDatabase.PExecute("INSERT INTO `character_pet` (`entry`,`owner`,`level`,`exp`,`nextlvlexp`,`spell1`,`spell2`,`spell3`,`spell4`,`action`,`fealty`,`name`,`current`) VALUES (%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,\"%s\",1)",
            pet->GetEntry(), GetGUIDLow(), pet->getLevel(), pet->GetUInt32Value(UNIT_FIELD_PETEXPERIENCE), pet->GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP),
            pet->m_spells[0], pet->m_spells[1], pet->m_spells[2], pet->m_spells[3], actState, pet->GetPower(POWER_HAPPINESS), name.c_str());
    }
}

void Player::outDebugValues() const
{
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetMaxHealth(), GetMaxPower(POWER_MANA));
    sLog.outDebug("AGILITY is: \t\t%u\t\tSTRENGHT is: \t\t%u",GetStat(STAT_AGILITY), GetStat(STAT_STRENGTH));
    sLog.outDebug("INTELLECT is: \t\t%u\t\tSPIRIT is: \t\t%u",GetStat(STAT_INTELLECT), GetStat(STAT_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%u\t\tSPIRIT is: \t\t%u",GetStat(STAT_STAMINA), GetStat(STAT_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%f",GetArmor(), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetResistance(SPELL_SCHOOL_HOLY), GetResistance(SPELL_SCHOOL_FIRE));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetResistance(SPELL_SCHOOL_NATURE), GetResistance(SPELL_SCHOOL_FROST));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetResistance(SPELL_SCHOOL_SHADOW), GetResistance(SPELL_SCHOOL_ARCANE));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetAttackTime(BASE_ATTACK), GetAttackTime(RANGED_ATTACK));
}

/*********************************************************/
/***              LOW LEVEL FUNCTIONS:Notifiers        ***/
/*********************************************************/

inline void Player::SendAttackSwingNotInRange()
{
    WorldPacket data;
    data.Initialize(SMSG_ATTACKSWING_NOTINRANGE);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingNotStanding()
{
    WorldPacket data;
    data.Initialize(SMSG_ATTACKSWING_NOTSTANDING);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingDeadTarget()
{
    WorldPacket data;
    data.Initialize(SMSG_ATTACKSWING_DEADTARGET);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingCantAttack()
{
    WorldPacket data;
    data.Initialize(SMSG_ATTACKSWING_CANT_ATTACK);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingCancelAttack()
{
    WorldPacket data;
    data.Initialize(SMSG_CANCEL_COMBAT);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingBadFacingAttack()
{
    WorldPacket data;
    data.Initialize(SMSG_ATTACKSWING_BADFACING);
    GetSession()->SendPacket( &data );
}

void Player::PlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data;
    data.Initialize(SMSG_PLAY_SOUND);
    data << Sound;
    if (OnlySelf)
        GetSession()->SendPacket( &data );
    else
        SendMessageToSet( &data, true );
}

void Player::SendExplorationExperience(uint32 Area, uint32 Experience)
{
    WorldPacket data;
    data.Initialize( SMSG_EXPLORATION_EXPERIENCE );
    data << Area;
    data << Experience;
    GetSession()->SendPacket(&data);
}

/*********************************************************/
/***              Update timers                        ***/
/*********************************************************/

void Player::UpdatePVPFlag(time_t currTime)
{
    //Player is counting to set/unset pvp flag
    if( !m_pvp_counting ) return;

    //Is player is in a PvP action stop counting
    if( (isInCombat() || isInDuel()) && getVictim()->GetTypeId() == TYPEID_PLAYER )
    {
        m_pvp_counting = false;
        return;
    }

    if( GetPvP() )
    {
        //Wait 5 min until remove pvp mode
        if( currTime < m_pvp_count + 300 ) return;

        SetPvP(false);

        sChatHandler.SendSysMessage(GetSession(), "PvP toggled off.");
    }
    else
    {
        SetPvP(true);
    }
}

void Player::UnsummonPet(bool remove)
{
    Creature* pet = GetPet();
    if(!pet) return;

    SavePet();
    SetPet(0);

    WorldPacket data;
    data.Initialize(SMSG_DESTROY_OBJECT);
    data << pet->GetGUID();
    SendMessageToSet (&data, true);

    MapManager::Instance().GetMap(pet->GetMapId())->Remove(pet,remove);

    data.Initialize(SMSG_PET_SPELLS);
    data << uint64(0);
    GetSession()->SendPacket(&data);
}

void Player::Uncharm()
{
    Creature* charm = GetCharm();
    if(!charm) return;

    SetCharm(0);

    CreatureInfo *cinfo = charm->GetCreatureInfo();
    charm->SetUInt64Value(UNIT_FIELD_CHARMEDBY,0);
    charm->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);
    
    charm->AIM_Initialize();
    WorldPacket data;
    data.Initialize(SMSG_PET_SPELLS);
    data << uint64(0);
    GetSession()->SendPacket(&data);
}

void Player::PetSpellInitialize()
{
    Creature* pet = GetPet();
    if(!pet)
        pet = GetCharm();
    if(pet)
    {

        WorldPacket data;
        uint16 Command = 7;
        uint16 State = 6;

        sLog.outDebug("Pet Spells Groups");

        data.clear();
        data.Initialize(SMSG_PET_SPELLS);

        data << (uint64)pet->GetGUID() << uint32(0x00000000) << uint32(0x00001000);

        data << uint16 (2) << uint16(Command << 8) << uint16 (1) << uint16(Command << 8) << uint16 (0) << uint16(Command << 8);

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
                                                            //C100 = maybe group
            data << uint16 (pet->m_spells[i]) << uint16 (0xC100);

        data << uint16 (2) << uint16(State << 8) << uint16 (1) << uint16(State << 8) << uint16 (0) << uint16(State << 8);

        GetSession()->SendPacket(&data);
    }
}

int32 Player::GetTotalFlatMods(uint32 spellId, uint8 op)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo) return 0;
    int32 total = 0;
    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier *mod = *itr;
        if (!mod) continue;
        if ((mod->mask & spellInfo->SpellFamilyFlags) == 0) continue;
        if (mod->type == SPELLMOD_FLAT)
            total += mod->value;
    }
    return total;
}

int32 Player::GetTotalPctMods(uint32 spellId, uint8 op)
{
    SpellEntry *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo) return 0;
    int32 total = 0;
    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier *mod = *itr;
        if (!mod) continue;
        if ((mod->mask & spellInfo->SpellFamilyFlags) == 0) continue;
        if (mod->type == SPELLMOD_PCT)
            total += mod->value;
    }
    return total;
}

void Player::ApplyBlockValueMod(int32 val,bool apply)
{
    ApplyModUInt32Var(m_BlockValue,val,apply);
}
