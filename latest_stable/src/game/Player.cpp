/*
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
#include "UpdateData.h"
#include "Channel.h"
#include "Chat.h"
#include "MapManager.h"
#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Group.h"
#include "Guild.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "Util.h"
#include "Transports.h"
#include "Weather.h"
#include "BattleGround.h"

#include <cmath>

#define DEFAULT_SWITCH_WEAPON        1500

typedef struct FriendStr
{
    uint64 PlayerGUID;
    unsigned char Status;

    uint32 Area;
    uint32 Level;
    uint32 Class;
} FriendStr ;

const int32 Player::ReputationRank_Length[MAX_REPUTATION_RANK] = {36000, 3000, 3000, 3000, 6000, 12000, 21000, 1000};

UpdateMask Player::updateVisualBits;

Player::Player (WorldSession *session): Unit( 0 )
{
    m_transport = 0;
    m_transX = 0.0f;
    m_transY = 0.0f;
    m_transZ = 0.0f;
    m_transO = 0.0f;

    m_objectType |= TYPE_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    m_session = session;

    m_divider = 0;

    m_GMFlags = 0;
    if(GetSession()->GetSecurity() >=2)
        SetAcceptTicket(true);

    // players always and GM if set in config accept whispers by default
    if(GetSession()->GetSecurity() == 0 || sWorld.getConfig(CONFIG_GM_WISPERING_TO))
        SetAcceptWhispers(true);

    m_curTarget = 0;
    m_curSelection = 0;
    m_lootGuid = 0;

    m_usedTalentCount = 0;

    m_regenTimer = 0;
    m_weaponChangeTimer = 0;
    m_dismountCost = 0;

    m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

    m_resurrectGUID = 0;
    m_resurrectX = m_resurrectY = m_resurrectZ = 0;
    m_resurrectHealth = m_resurrectMana = 0;

    memset(m_items, 0, sizeof(Item*)*PLAYER_SLOTS_COUNT);

    groupInfo.group  = 0;
    groupInfo.invite = 0;

    duel = 0;

    m_GuildIdInvited = 0;

    m_dungeonDifficulty = DUNGEONDIFFICULTY_NORMAL;

    m_needRename = false;

    m_dontMove = false;

    pTrader = 0;
    ClearTrade();

    m_cinematic = 0;

    PlayerTalkClass = new PlayerMenu( GetSession() );
    m_currentBuybackSlot = BUYBACK_SLOT_START;

    for ( int aX = 0 ; aX < 8 ; aX++ )
        m_Tutorials[ aX ] = 0x00;
    ItemsSetEff[0]=0;
    ItemsSetEff[1]=0;
    ItemsSetEff[2]=0;
    m_regenTimer = 0;
    m_weaponChangeTimer = 0;
    m_breathTimer = 0;
    m_isunderwater = 0;
    m_isInWater = false;
    m_drunkTimer = 0;
    m_drunk = 0;
    m_restTime = 0;
    m_lastManaUse = 0;
    m_deathTimer = 0;
    m_resurrectingSicknessExpire = 0;

    m_DetectInvTimer = 1000;
    m_DiscoveredPj = 0;
    m_enableDetect = true;

    m_bgBattleGroundID = 0;
    m_bgBattleGroundQueueID = 0;

    m_movement_flags = 0;

    m_logintime = time(NULL);
    m_Last_tick = m_logintime;
    m_WeaponProficiency = 0;
    m_ArmorProficiency = 0;
    m_canParry = false;
    m_canDualWield = false;

    ////////////////////Rest System/////////////////////
    time_inn_enter=0;
    inn_pos_x=0;
    inn_pos_y=0;
    inn_pos_z=0;
    m_rest_bonus=0;
    rest_type=0;
    ////////////////////Rest System/////////////////////

    m_mailsLoaded = false;
    m_mailsUpdated = false;
    unReadMails = 0;
    m_nextMailDelivereTime = 0;

    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;
    m_itemUpdateQueueBlocked = false;

    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_forced_speed_changes[i] = 0;

    m_stableSlots = 0;

    /////////////////// Instance System /////////////////////

    m_Loaded = false;
    m_HomebindTimer = 0;
    m_InstanceValid = true;
}

Player::~Player ()
{
    if(m_uint32Values)                                      // only for fully created Object
    {
        CombatStop(true);
        DuelComplete(0);
        RemoveAllAuras();
    }

    TradeCancel(false);

    // Note: buy back item already deleted from DB when player was saved
    for(int i = 0; i < PLAYER_SLOTS_COUNT; ++i)
    {
        if(m_items[i])
            delete m_items[i];
    }
    CleanupChannels();

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        delete itr->second;

    //all mailed items should be deleted, also all mail should be deallocated
    for (PlayerMails::iterator itr =  m_mail.begin(); itr != m_mail.end();++itr)
        delete *itr;

    for (ItemMap::iterator iter = mMitems.begin(); iter != mMitems.end(); ++iter)
        delete iter->second;                                //if item is duplicated... then server may crash ... but that item should be deallocated

    delete PlayerTalkClass;

    if (m_transport)
    {
        m_transport->RemovePassenger(this);
    }
}

bool Player::Create( uint32 guidlow, WorldPacket& data )
{
    int i;
    uint8 race,class_,gender,skin,face,hairStyle,hairColor,facialHair,outfitId;

    Object::_Create(guidlow, HIGHGUID_PLAYER);

    data >> m_name;

    if(m_name.size() == 0)
        return false;

    normalizePlayerName(m_name);

    data >> race >> class_ >> gender >> skin >> face;
    data >> hairStyle >> hairColor >> facialHair >> outfitId;

    PlayerInfo const* info = objmgr.GetPlayerInfo(race, class_);
    if(!info)
    {
        sLog.outError("Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    for (i = 0; i < PLAYER_SLOTS_COUNT; i++)
        m_items[i] = NULL;

    //for(int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
    //{
    //    SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+j*2,0);
    //    SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+j,0);
    //    SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+j,0);
    //}

    m_race = race;
    m_class = class_;

    SetMapId(info->mapId);
    Relocate(info->positionX,info->positionY,info->positionZ);

    // Taxi nodes setup
    memset(m_taximask, 0, sizeof(m_taximask));
    /*
        // Automatically add the race's taxi hub to the character's taximask at creation time ( 1 << (taxi_node_id-1) )
        ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
        if(!rEntry)
        {
            sLog.outError("Race %u not found in DBÑ (Wrong DBC files?)",race);
            return false;
        }

        m_taximask[0] = rEntry->startingTaxiMask;
    */
    switch(race)
    {
        case 1:         m_taximask[0]= 1 << ( 2-1); break;  // Human
        case 2:         m_taximask[0]= 1 << (23-1); break;  // Orc
        case 3:         m_taximask[0]= 1 << ( 6-1); break;  // Dwarf
                                                            // Night Elf
        case 4:         m_taximask[0]= (1 << (26-1)) | (1 << (27-1)); break;
        case 5:         m_taximask[0]= 1 << (11-1); break;  // Undead
        case 6:         m_taximask[0]= 1 << (22-1); break;  // Tauren
        case 7:         m_taximask[0]= 1 << ( 6-1); break;  // Gnome
        case 8:         m_taximask[0]= 1 << (23-1); break;  // Troll
        //case 10:        m_taximask[0]= 1 << (1-1); break; // Blood Elf
                                                            // Draenei
        case 11:        m_taximask[0+94/32]= 1 << (94%32-1); break;
    }

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(class_);
    if(!cEntry)
    {
        sLog.outError("Class %u not found in DBC (Wrong DBC files?)",class_);
        return false;
    }

    uint8 powertype = cEntry->powerType;

    uint32 unitfield;
    if(powertype == POWER_RAGE)
        unitfield = 0x00110000;
    else if(powertype == POWER_ENERGY)
        unitfield = 0x00000000;
    else if(powertype == POWER_MANA)
        unitfield = 0x00000000;
    else
    {
        sLog.outError("Invalid default powertype %u for player (class %u)",powertype,class_);
        return false;
    }

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f   );

    switch(gender)
    {
        case GENDER_FEMALE:
            SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId_f );
            SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId_f );
            break;
        case GENDER_MALE:
            SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId_m );
            SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId_m );
            break;
        default:
            break;
    }

    setFactionForRace(m_race);

    SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( race ) | ( class_ << 8 ) | ( gender << 16 ) | ( powertype << 24 ) ) );
    SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield );
    SetUInt32Value(UNIT_FIELD_BYTES_2, ( 0x28 << 8 ) );     // players - 0x2800, 0x2801, units - 0x1001
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1 );
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);               // fix cast time showed in spell tooltip on client

                                                            //-1 is default value
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    SetUInt32Value(PLAYER_BYTES, (skin | (face << 8) | (hairStyle << 16) | (hairColor << 24)));
    SetUInt32Value(PLAYER_BYTES_2, (facialHair | (0x00 << 8) | (0x00 << 16) | (0x02 << 24)));
    SetUInt32Value(PLAYER_BYTES_3, gender);
    //SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );

    SetUInt32Value( PLAYER_GUILDID, 0 );
    SetUInt32Value( PLAYER_GUILDRANK, 0 );
    SetUInt32Value( PLAYER_GUILD_TIMESTAMP, 0 );

    SetUInt32Value( PLAYER_FIELD_KNOWN_TITLES, 0 );         // 0=disabled
    SetUInt32Value( PLAYER_CHOSEN_TITLE, 0 );
    SetUInt32Value( PLAYER_FIELD_KILLS, 0 );
    SetUInt32Value( PLAYER_FIELD_KILLS_LIFETIME, 0 );
    SetUInt32Value( PLAYER_FIELD_HONOR_TODAY, 0 );
    SetUInt32Value( PLAYER_FIELD_HONOR_YESTERDAY, 0 );
    SetUInt32Value( PLAYER_FIELD_MAX_LEVEL, 70 );

    // Played time
    m_Last_tick = time(NULL);
    m_Played_time[0] = 0;
    m_Played_time[1] = 0;

    // base stats and related field values
    InitStatsForLevel(1,false,false);
    InitTalentForLevel();

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()
    _ApplyStatsMods();

    uint32 titem_id;
    uint32 titem_amount;
    uint16 tspell, tskill, taction[4];
    std::list<uint16>::const_iterator skill_itr, action_itr[4];
    std::list<CreateSpellPair>::const_iterator spell_itr;

    spell_itr = info->spell.begin();

    for (; spell_itr!=info->spell.end(); spell_itr++)
    {
        tspell = spell_itr->first;
        if (tspell)
        {
            sLog.outDebug("PLAYER: Adding initial spell, id = %u",tspell);
            addSpell(tspell,spell_itr->second);
        }
    }

    skill_itr = info->skill.begin();

    for (; skill_itr!=info->skill.end(); )
    {
        tskill = (*skill_itr);

        if (tskill)
        {
            sLog.outDebug("PLAYER: Adding initial skill line, skillId = %u, value = 5, max = 5", tskill);
            SetSkill(tskill, 5, 5);                         // (5,5) is default values for skill pages
        }

        skill_itr++;
    }

    for(i=0; i<4; i++)
        action_itr[i] = info->action[i].begin();

    for (; action_itr[0]!=info->action[0].end() && action_itr[1]!=info->action[1].end();)
    {
        for( i=0; i<4 ;i++)
            taction[i] = (*action_itr[i]);

        addActionButton((uint8)taction[0], taction[1], (uint8)taction[2], (uint8)taction[3]);

        for( i=0; i<4 ;i++)
            action_itr[i]++;
    }

    UpdateBlockPercentage();

    uint16 dest;
    uint8 msg;
    Item *pItem;
    for (PlayerCreateInfoItems::const_iterator item_id_itr = info->item.begin(); item_id_itr!=info->item.end(); ++item_id_itr++)
    {
        titem_id     = item_id_itr->item_id;
        titem_amount = item_id_itr->item_amount;

        if (titem_id)
        {
            sLog.outDebug("STORAGE: Creating initial item, itemId = %u, count = %u",titem_id, titem_amount);

            pItem = CreateItem( titem_id, titem_amount);
            if( pItem )
            {
                msg = CanEquipItem( NULL_SLOT, dest, pItem, false );
                if( msg == EQUIP_ERR_OK )
                    EquipItem( dest, pItem, true);
                else
                {
                    // store in main bag to simplify second pass
                    msg = CanStoreItem( INVENTORY_SLOT_BAG_0, NULL_SLOT, dest, pItem, false );
                    if( msg == EQUIP_ERR_OK )
                        StoreItem( dest, pItem, true);
                    else
                    {
                        sLog.outError("STORAGE: Can't equip or store initial item %u for race %u class %u , error msg = %u",titem_id,race,class_,msg);
                        delete pItem;
                    }
                }
            }
            else
                sLog.outError("STORAGE: Can't create initial item %u (not existed item id) for race %u class %u , error msg = %u",titem_id,race,class_,msg);
        }
    }

    // bags and main-hand weapon must equipped at this moment
    // now second pass for not equipped (offhand weapon/shield if it attempt equipped before main-hand weapon)
    // or ammo not equipped in special bag
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        int16 pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
        pItem = GetItemByPos( pos );

        if(pItem)
        {
            // equip offhand weapon/shield if it attempt equipped before main-hand weapon
            msg = CanEquipItem( NULL_SLOT, dest, pItem, false );
            if( msg == EQUIP_ERR_OK )
            {
                RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                EquipItem( dest, pItem, true);
            }else
            // move other items to more appropriate slots (ammo not equipped in special bag)
            {
                msg = CanStoreItem( NULL_BAG, NULL_SLOT, dest, pItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                    pItem = StoreItem( dest, pItem, true);
                }

                // if  this is ammo then use it
                uint8 msg = CanUseAmmo( pItem->GetProto()->ItemId );
                if( msg == EQUIP_ERR_OK )
                    SetAmmo( pItem->GetProto()->ItemId );
            }
        }
    }
    // all item positions resolved

    return true;
}

void Player::StartMirrorTimer(MirrorTimerType Type, uint32 MaxValue)
{
    uint32 BreathRegen = (uint32)-1;

    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << MaxValue;
    data << MaxValue;
    data << BreathRegen;
    data << (uint8)0;
    data << (uint32)0;                                      // spell id
    GetSession()->SendPacket(&data);
}

void Player::ModifyMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, uint32 Regen)
{
    if(Type==BREATH_TIMER)
        m_breathTimer = ((MaxValue + 1000) - CurrentValue) / Regen;

    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;                                      // spell id
    GetSession()->SendPacket( &data );
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    if(Type==BREATH_TIMER)
        m_breathTimer = 0;

    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    GetSession()->SendPacket( &data );
}

void Player::EnvironmentalDamage(uint64 Guid, uint8 Type, uint32 Amount)
{
    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, (21));
    data << Guid;
    data << (uint8)Type;
    data << Amount;
    data << (uint32)0;
    data << (uint32)0;
    //m_session->SendPacket(&data);
    //Let other players see that you get damage
    SendMessageToSet(&data, true);
    DealDamage((Unit*)this, Amount, SELF_DAMAGE, 0, NULL, 0, true);
}

void Player::HandleDrowning(uint32 UnderWaterTime)
{
    if(!m_isunderwater)
        return;

    AuraList& mModWaterBreathing = GetAurasByType(SPELL_AURA_MOD_WATER_BREATHING);
    for(AuraList::iterator i = mModWaterBreathing.begin(); i != mModWaterBreathing.end(); ++i)
        UnderWaterTime = uint32(UnderWaterTime * (100.0f + (*i)->GetModifier()->m_amount) / 100.0f);

    //if have water breath , then remove bar
    if(waterbreath || !isAlive())
    {
        StopMirrorTimer(BREATH_TIMER);
        m_isunderwater = 0;
        return;
    }

    if ((m_isunderwater & 0x01) && !(m_isunderwater & 0x80) && isAlive())
    {
        //single trigger timer
        if (!(m_isunderwater & 0x02))
        {
            m_isunderwater|= 0x02;
            m_breathTimer = UnderWaterTime + 1000;
        }
        //single trigger "Breathbar"
        if ( m_breathTimer <= UnderWaterTime && !(m_isunderwater & 0x04))
        {
            m_isunderwater|= 0x04;
            StartMirrorTimer(BREATH_TIMER, UnderWaterTime);
        }
        //continius trigger drowning "Damage"
        if ((m_breathTimer == 0) && (m_isunderwater & 0x01))
        {
            //TODO: Check this formula
            uint64 guid = GetGUID();
            uint32 damage = GetMaxHealth() / 5 + urand(0, getLevel()-1);

            EnvironmentalDamage(guid, DAMAGE_DROWNING,damage);
            m_breathTimer = 2000;
        }
    }
    //single trigger retract bar
    else if (!(m_isunderwater & 0x01) && !(m_isunderwater & 0x08) && (m_isunderwater & 0x02) && (m_breathTimer > 0) && isAlive())
    {
        m_isunderwater = 0x08;

        uint32 BreathRegen = 10;
        ModifyMirrorTimer(BREATH_TIMER, UnderWaterTime, m_breathTimer,BreathRegen);
        m_isunderwater = 0x10;
    }
    //remove bar
    else if ((m_breathTimer < 50) && !(m_isunderwater & 0x01) && (m_isunderwater == 0x10))
    {
        StopMirrorTimer(BREATH_TIMER);
        m_isunderwater = 0;
    }
}

void Player::HandleLava()
{
    if ((m_isunderwater & 0x80) && isAlive())
    {
        //Single trigger Set BreathTimer
        if (!(m_isunderwater & 0x04))
        {
            m_isunderwater|= 0x04;
            m_breathTimer = 1000;
        }
        //Reset BreathTimer and still in the lava
        if (!m_breathTimer)
        {
            uint64 guid;
            //uint32 damage = 10;
            uint32 damage = GetMaxHealth() / 3 + urand(0, getLevel()-1);

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
    SetUInt32Value(PLAYER_BYTES_3, (GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0001) | m_drunk);
}

void Player::SetDrunkValue(uint16 newDrunkValue)
{
    m_drunk = newDrunkValue;
    SetUInt32Value(PLAYER_BYTES_3,
        (GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0001) | m_drunk);
}

void Player::Update( uint32 p_time )
{
    if(!IsInWorld())
        return;

    // undelivered mail
    if(m_nextMailDelivereTime && m_nextMailDelivereTime <= time(NULL))
    {
        SendNewMail();
        ++unReadMails;

        // It will be recalculate at mailbox open (for unReadMails important non-0 until mailbox open, it also will be recalculated)
        m_nextMailDelivereTime = 0;
    }

    Unit::Update( p_time );

    // update player only attacks
    if(uint32 ranged_att = getAttackTimer(RANGED_ATTACK))
    {
        setAttackTimer(RANGED_ATTACK, (p_time >= ranged_att ? 0 : ranged_att - p_time) );
    }

    if(uint32 off_att = getAttackTimer(OFF_ATTACK))
    {
        setAttackTimer(OFF_ATTACK, (p_time >= off_att ? 0 : off_att - p_time) );
    }

    time_t now = time (NULL);

    UpdatePvPFlag(time(NULL));

    UpdateDuelFlag(time(NULL));

    CheckDuelDistance(time(NULL));

    CheckExploreSystem();

    if (m_timedquests.size() > 0)
    {
        std::set<uint32>::iterator iter = m_timedquests.begin();
        while (iter != m_timedquests.end())
        {
            //if( mQuestStatus[*iter].m_timer > 0 )
            //{
            if( mQuestStatus[*iter].m_timer <= p_time )
            {
                uint32 quest_id  = *iter;
                ++iter;                                     // current iter will be removed in FailTimedQuest
                FailTimedQuest( quest_id );
            }
            else
            {
                mQuestStatus[*iter].m_timer -= p_time;
                if (mQuestStatus[*iter].uState != QUEST_NEW) mQuestStatus[*iter].uState = QUEST_CHANGED;
                ++iter;
            }
            //}
        }
    }

    if (isAttacking())
    {
        Unit *pVictim = getVictim();
        if( m_currentSpell == 0 && pVictim)
        {

            // default combat reach 10
            // TODO add weapon,skill check

            float pldistance = ATTACK_DIST;

            if (isAttackReady(BASE_ATTACK))
            {
                if(!IsWithinDistInMap(pVictim, pldistance))
                {
                    setAttackTimer(BASE_ATTACK,1000);
                    SendAttackSwingNotInRange();
                }
                //120 degrees of radiant range
                else if( !HasInArc( 2*M_PI/3, pVictim ))
                {
                    setAttackTimer(BASE_ATTACK,1000);
                    SendAttackSwingBadFacingAttack();
                }
                else
                {
                    // prevent base and off attack in same time, delay attack at 0.2 sec
                    if(haveOffhandWeapon())
                    {
                        uint32 off_att = getAttackTimer(OFF_ATTACK);
                        if(off_att < ATTACK_DISPLAY_DELAY)
                            setAttackTimer(OFF_ATTACK,ATTACK_DISPLAY_DELAY);
                    }
                    AttackerStateUpdate(pVictim, BASE_ATTACK);
                    resetAttackTimer(BASE_ATTACK);
                }
            }

            if ( haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
            {
                if(!IsWithinDistInMap(pVictim, pldistance))
                {
                    setAttackTimer(OFF_ATTACK,1000);
                }
                else if( !HasInArc( 2*M_PI/3, pVictim ))
                {
                    setAttackTimer(OFF_ATTACK,1000);
                }
                else
                {
                    // prevent base and off attack in same time, delay attack at 0.2 sec
                    uint32 base_att = getAttackTimer(BASE_ATTACK);
                    if(base_att < ATTACK_DISPLAY_DELAY)
                        setAttackTimer(BASE_ATTACK,ATTACK_DISPLAY_DELAY);
                    // do attack
                    AttackerStateUpdate(pVictim, OFF_ATTACK);
                    resetAttackTimer(OFF_ATTACK);
                }
            }

            Unit *owner = pVictim->GetOwner();
            Unit *u = owner ? owner : pVictim;
            if(u->IsPvP() && (!duel || duel->opponent != u))
                UpdatePvP(true);
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
    {
        if(roll_chance_i(3) && GetTimeInnEter() > 0)        //freeze update
        {
            int time_inn = time(NULL)-GetTimeInnEter();
            if (time_inn >= 10)                             //freeze update
            {
                float bubble = sWorld.getRate(RATE_REST_INGAME);
                                                            //speed collect rest bonus (section/in hour)
                SetRestBonus( GetRestBonus()+ time_inn*((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)/144000)*bubble );
                UpdateInnerTime(time(NULL));
            }
        }
        if(GetRestType()==1)                                //rest in tavern
        {
            if(sqrt((GetPositionX()-GetInnPosX())*(GetPositionX()-GetInnPosX())+(GetPositionY()-GetInnPosY())*(GetPositionY()-GetInnPosY())+(GetPositionZ()-GetInnPosZ())*(GetPositionZ()-GetInnPosZ()))>40)
            {
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
            }
        }
    }

    if(m_regenTimer > 0)
    {
        if(p_time >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= p_time;
    }

    if (m_weaponChangeTimer > 0)
    {
        if(p_time >= m_weaponChangeTimer)
            m_weaponChangeTimer = 0;
        else
            m_weaponChangeTimer -= p_time;
    }

    if (isAlive())
    {
        RegenerateAll();
    }

    if (m_deathState == JUST_DIED)
    {
        KillPlayer();
    }

    if(m_nextSave > 0)
    {
        if(p_time >= m_nextSave)
        {
            // m_nextSave reseted in SaveToDB call
            SaveToDB(false);
            sLog.outBasic("Player '%u' '%s' Saved", GetGUIDLow(), GetName());
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
    HandleDrowning(60000);

    //Handle lava
    HandleLava();

    //Handle detect invisible players
    if (m_DetectInvTimer > 0)
    {
        if (p_time >= m_DetectInvTimer)
        {
            m_DetectInvTimer = 3000;
            HandleInvisiblePjs();
        }
        else
            m_DetectInvTimer -= p_time;
    }

    // Played time
    if (now > m_Last_tick)
    {
        uint32 elapsed = uint32(now - m_Last_tick);
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
    UpdateEnchantTime(p_time);
    UpdateHomebindTime(p_time);
}

void Player::setDeathState(DeathState s)
{
    uint32 soulstoneSpellId = 0;

    bool cur = isAlive();

    if(s == JUST_DIED && cur)
    {
        // remove resurrection sickness before other mods to prevent incorrect stats calculation
        RemoveAurasDueToSpell(SPELL_PASSIVE_RESURRECTION_SICKNESS);
        // remove form before other mods to prevent incorrect stats calculation
        RemoveAurasDueToSpell(m_ShapeShiftForm);

        _RemoveAllItemMods();
        RemovePet(NULL,PET_SAVE_AS_CURRENT);

        // save value before aura remove in Unit::setDeathState
        soulstoneSpellId = GetUInt32Value(PLAYER_SELF_RES_SPELL);
    }
    Unit::setDeathState(s);

    // restore soulstone spell id for player after aura remove
    if(s == JUST_DIED && cur)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, soulstoneSpellId);

    if(isAlive() && !cur)
    {
        _ApplyAllItemMods();

        // restore default warrior stance
        if(getClass()== CLASS_WARRIOR)
            CastSpell(this,SPELL_PASSIVE_BATTLE_STANCE,true);
    }
}

void Player::BuildEnumData( WorldPacket * p_data )
{
    *p_data << GetGUID();
    *p_data << m_name;

    *p_data << getRace();
    *p_data << getClass();
    *p_data << getGender();

    uint32 bytes = GetUInt32Value(PLAYER_BYTES);
    *p_data << uint8(bytes);
    *p_data << uint8(bytes >> 8);
    *p_data << uint8(bytes >> 16);
    *p_data << uint8(bytes >> 24);

    bytes = GetUInt32Value(PLAYER_BYTES_2);
    *p_data << uint8(bytes);

    *p_data << uint8(getLevel());                           // player level
    uint32 zoneId = MapManager::Instance().GetMap(GetMapId(), this)->GetZoneId(GetPositionX(),GetPositionY());

    *p_data << zoneId;
    *p_data << GetMapId();

    *p_data << GetPositionX();
    *p_data << GetPositionY();
    *p_data << GetPositionZ();

    *p_data << GetUInt32Value(PLAYER_GUILDID);              // guild id

    *p_data << uint8(0x0);                                  // different values on off, looks like flags

    uint8 flags = 0;
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
        flags |= 0x04;
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
        flags |= 0x08;
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        flags |= 0x20;
    if(isNeedRename())
        flags |= 0x40;
    *p_data << uint8(flags);    // flags description below
    // 0x01 - unknown
    // 0x02 - unknown
    // 0x04 - hide helm
    // 0x08 - hide cloak
    // 0x10 - unknown
    // 0x20 - dead(ghost)
    // 0x40 - need rename

    *p_data << uint8(0xa0);                                 // Bit 4 is something dono
    *p_data << uint8(0x0);                                  // is this player_GUILDRANK????

    *p_data << (uint8)1;                                    // 0x1 there

    // Pets info
    {
        uint32 petDisplayId = 0;
        uint32 petLevel   = 0;
        uint32 petFamily  = 0;

        // show pet at selection character in character list  only for non-ghost character
        if(isAlive())
        {
            QueryResult *result = sDatabase.Query("SELECT `entry`,`modelid`,`level` FROM `character_pet` WHERE `owner` = '%u' AND `slot` = '0'", GetGUIDLow() );
            if(result)
            {
                Field* fields = result->Fetch();

                uint32 entry = fields[0].GetUInt32();
                CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);
                if(cInfo)
                {
                    petDisplayId = fields[1].GetUInt32();
                    petLevel     = fields[2].GetUInt32();
                    petFamily    = cInfo->family;
                }
                delete result;
            }
        }

        *p_data << (uint32)petDisplayId;
        *p_data << (uint32)petLevel;
        *p_data << (uint32)petFamily;
    }

    ItemPrototype const *items[EQUIPMENT_SLOT_END];
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        items[i] = NULL;

    QueryResult *result = sDatabase.Query("SELECT `slot`,`item_template` FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = 0",GetGUIDLow());
    if (result)
    {
        do
        {
            Field *fields  = result->Fetch();
            uint8  slot    = fields[0].GetUInt8() & 255;
            uint32 item_id = fields[1].GetUInt32();
            if( slot >= EQUIPMENT_SLOT_END )
                continue;

            items[slot] = objmgr.GetItemPrototype(item_id);
            if(!items[slot])
            {
                sLog.outError( "Player::BuildEnumData: Player %s have unknown item (id: #%u) in inventory, skipped.", GetName(),item_id );
                continue;
            }
        } while (result->NextRow());
        delete result;
    }

    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if (items[i] != NULL)
        {
            *p_data << (uint32)items[i]->DisplayInfoID;
            *p_data << (uint8)items[i]->InventoryType;
        }
        else
        {
            *p_data << (uint32)0;
            *p_data << (uint8)0;
        }
    }
    *p_data << (uint32)0;                                   // first bag display id
    *p_data << (uint8)0;                                    // first bag inventory type
}

bool Player::ToggleAFK()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);

    return HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);
}

bool Player::ToggleDND()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);

    return HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);
}

uint8 Player::chatTag()
{
    if(isGameMaster())
        return 3;
    else if(isDND())
        return 2;
    if(isAFK())
        return 1;
    else
        return 0;
}

void Player::SendFriendlist()
{
    uint8 i=0;
    Field *fields;
    Player* pObj;
    FriendStr friendstr[255];

    QueryResult *result = sDatabase.Query("SELECT `friend` FROM `character_social` WHERE `flags` = 'FRIEND' AND `guid` = '%u'",GetGUIDLow());
    if(result)
    {
        fields = result->Fetch();

        uint32 team = GetTeam();
        uint32 security = GetSession()->GetSecurity();
        bool allowTwoSideWhoList = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
        bool gmInWhoList         = sWorld.getConfig(CONFIG_GM_IN_WHO_LIST);

        do
        {
            friendstr[i].PlayerGUID = fields[0].GetUInt64();
            pObj = ObjectAccessor::Instance().FindPlayer(friendstr[i].PlayerGUID);

            // PLAYER see his team only and PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
            // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
            if( pObj && pObj->GetName() &&
                ( security > 0 ||
                ( pObj->GetTeam() == team || allowTwoSideWhoList ) &&
                (pObj->GetSession()->GetSecurity() == 0 || gmInWhoList && pObj->isVisibleFor(this,false) )))
            {
                if(pObj->isAFK())
                    friendstr[i].Status = 2;
                else if(pObj->isDND())
                    friendstr[i].Status = 4;
                else
                    friendstr[i].Status = 1;
                friendstr[i].Area = pObj->GetZoneId();
                friendstr[i].Level = pObj->getLevel();
                friendstr[i].Class = pObj->getClass();
            }
            else
            {
                friendstr[i].Status = 0;
                friendstr[i].Area = 0;
                friendstr[i].Level = 0;
                friendstr[i].Class = 0;
            }
            i++;

            // prevent overflow
            if(i==255)
                break;
        } while( result->NextRow() );

        delete result;
    }

    WorldPacket data(SMSG_FRIEND_LIST, (1+i*15));           // just can guess size
    data << i;

    for (int j=0; j < i; j++)
    {
        sLog.outDetail( "WORLD: Adding Friend Guid: %u, Status:%u, Area:%u, Level:%u Class:%u",GUID_LOPART(friendstr[j].PlayerGUID), friendstr[j].Status, friendstr[j].Area,friendstr[j].Level,friendstr[j].Class  );

        data << friendstr[j].PlayerGUID << friendstr[j].Status ;
        if (friendstr[j].Status != 0)
            data << friendstr[j].Area << friendstr[j].Level << friendstr[j].Class;
    }

    this->GetSession()->SendPacket( &data );
    sLog.outDebug( "WORLD: Sent (SMSG_FRIEND_LIST)" );
}

void Player::AddToIgnoreList(uint64 guid, std::string name)
{
    // prevent list (client-side) overflow
    if(m_ignorelist.size() >= (255-1))
        return;

    sDatabase.Execute("INSERT INTO `character_social` (`guid`,`name`,`friend`,`flags`) VALUES ('%u', '%s', '%u', 'IGNORE')",
        GetGUIDLow(), name.c_str(), GUID_LOPART(guid));
    m_ignorelist.insert(GUID_LOPART(guid));
}

void Player::RemoveFromIgnoreList(uint64 guid)
{
    sDatabase.Execute("DELETE FROM `character_social` WHERE `flags` = 'IGNORE' AND `guid` = '%u' AND `friend` = '%u'",GetGUIDLow(), GUID_LOPART(guid));
    m_ignorelist.erase(GUID_LOPART(guid));
}

void Player::LoadIgnoreList()
{
    QueryResult *result = sDatabase.Query("SELECT `friend` FROM `character_social` WHERE `flags` = 'IGNORE' AND `guid` = '%u'", GetGUIDLow());

    if(!result) return;

    do
    {
        Field *fields  = result->Fetch();
        m_ignorelist.insert(fields[0].GetUInt32());

        // prevent list (client-side) overflow
        if(m_ignorelist.size() >= 255)
            break;
    }
    while( result->NextRow() );

    delete result;
}

void Player::SendIgnorelist()
{
    if(m_ignorelist.empty())
        return;

    WorldPacket dataI(SMSG_IGNORE_LIST, (1+m_ignorelist.size()*8));
    dataI << uint8(m_ignorelist.size());

    for(IgnoreList::iterator iter = m_ignorelist.begin(); iter != m_ignorelist.end(); ++iter)
    {
        dataI << uint64(MAKE_GUID(*iter,HIGHGUID_PLAYER));
    }

    GetSession()->SendPacket( &dataI );
    sLog.outDebug( "WORLD: Sent (SMSG_IGNORE_LIST)" );
}

void Player::TeleportTo(uint32 mapid, float x, float y, float z, float orientation, bool outofrange, bool ignore_transport, bool is_gm_command)
{
    // preparing unsummon pet if lost (we must get pet before teleportation or will not find it later)
    Pet* pet = GetPet();

    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);

    // this map not exist in client
    if(!mEntry)
        return;

    // don't let enter battlegrounds without assigned battleground id (for example through areatrigger)...
    if(!InBattleGround() && mEntry->map_type == MAP_BATTLEGROUND && !GetSession()->GetSecurity())
        return;

    QueryResult *result = loginDatabase.Query("SELECT `tbc` FROM `account` WHERE `id`='%u'", GetSession()->GetAccountId());

    if(!result)
        return;                                             // unknown client or error

    Field *fields = result->Fetch();

    uint32 not_tbc_map = 0x10;                              // if any problems, then check 0x80000 also...
    // with 0x80000 we can teleport to Kharazan with normal client
    // with 0x10 we can't teleport to Kharazan with normal client
    uint8 tbc = fields[0].GetUInt8();
    delete result;

    if(tbc == 0 && (mEntry->map_flag & not_tbc_map) == 0)   // normal client and TBC map
    {
        sLog.outDebug("Player %s using Normal client and tried teleport to non existing map %u", GetName(), mapid);

        if(GetTransport())
            RepopAtGraveyard();                             // teleport to near graveyard if on transport, looks blizz like :)

        // send error message
        WorldPacket data(SMSG_TRANSFER_ABORTED, 4+2);
        data << mapid;
        data << uint16(0x0106);                             // unk, probably error message id, now it's "You must have The Burning Crusade expansion installed to access this area.", look for other messages in GlobalStrings.lua (TRANSFER_ABORT_*).
        GetSession()->SendPacket(&data);

        return;                                             // normal client can't teleport to this map...
    }
    else if(tbc == 1)                                       // can teleport to any existing map
    {
        sLog.outDebug("Player %s have TBC client and will teleported to map %u", GetName(), mapid);
    }
    else
    {
        sLog.outDebug("Player %s have normal client and will teleported to standard map %u", GetName(), mapid);
    }
    /*
    only TBC (no 0x80000 and 0x10 flags...)
    3604590=0x37006E=0x200000 + 0x100000 + 0x40000 + 0x20000 + 0x10000 + 0x40 + 0x20 + 0x8 + 0x4 + 0x2

    Kharazan (normal/TBC??), but not have 0x10 flag (accessible by normal client?)
    4128878=0x3F006E=0x200000 + 0x100000 + 0x80000 + 0x40000 + 0x20000 + 0x10000 + 0x40 + 0x20 + 0x8 + 0x4 + 0x2

    normal+TBC maps
    4128894=0x3F007E=0x200000 + 0x100000 + 0x80000 + 0x40000 + 0x20000 + 0x10000 + 0x40 + 0x20 + 0x10 + 0x8 + 0x4 + 0x2

    normal+TBC maps
    8323198=0x7F007E=0x400000 + 0x200000 + 0x100000 + 0x80000 + 0x40000 + 0x20000 + 0x10000 + 0x40 + 0x20 + 0x10 + 0x8 + 0x4 + 0x2
    */

    // prepare zone change detect
    uint32 old_zone = GetZoneId();

    // if we were on a transport, leave
    if (ignore_transport && m_transport)
    {
        m_transport->RemovePassenger(this);
        m_transport = NULL;
        m_transX = 0.0f;
        m_transY = 0.0f;
        m_transZ = 0.0f;
        m_transO = 0.0f;
    }

    SetSemaphoreTeleport(true);

    if ((this->GetMapId() == mapid) && (!m_transport))
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
        Map* oldmap = MapManager::Instance().GetMap(GetMapId(), this);

        // now we must check if we are going to be homebind after teleport, if it is so,
        // we must re-instantiate again (entering instance to be homebind is not very good idea)
        // this only affects entering instances, not re-logging in (teleport is not used on login)
        Map* map = MapManager::Instance().GetMap(mapid, this);
        // verify, if it does want to homebind us (but only if we are not in GM state,
        // GMs are allowed to teleport everywhere they want to be)
        if (!m_InstanceValid && (!isGameMaster() || !is_gm_command))
        {
            // yes, we are going to be homebind, avoid this action :)
            SetInstanceId(0);                               // reset instance id
            m_BoundInstances.erase(mapid);                  // unbind our instance binding
                                                            // obtain new map
            map = MapManager::Instance().GetMap(mapid, this);
        }

        if (map && MapManager::Instance().CanPlayerEnter(mapid, this) &&  map->AddInstanced(this))
        {
            if (oldmap) oldmap->Remove(this, false);

            WorldPacket data(SMSG_TRANSFER_PENDING, (4+4+4));
            data << uint32(mapid);
            if (m_transport)
            {
                data << m_transport->GetEntry() << GetMapId();
            }
            GetSession()->SendPacket(&data);

            data.Initialize(SMSG_NEW_WORLD, (20));
            if (m_transport)
            {
                data << (uint32)mapid << m_transX << m_transY << m_transZ << m_transO;
            }
            else
            {
                data << (uint32)mapid << (float)x << (float)y << (float)z << (float)orientation;
            }
            GetSession()->SendPacket( &data );

            data.Initialize(SMSG_UNKNOWN_811, 4);
            data << uint32(0);
            GetSession()->SendPacket( &data );

            SetMapId(mapid);

            // orientation+0.1 used to pressure SetPosition send notifiers at teleportaion like in normal move case
            if(m_transport)
            {
                Relocate(x + m_transX, y + m_transY, z + m_transZ, orientation + m_transO+0.1);
                SetPosition(x + m_transX, y + m_transY, z + m_transZ, orientation + m_transO);
            }
            else
            {
                Relocate(x, y, z, orientation+0.1);
                SetPosition(x, y, z, orientation);
            }

            // resurrect character at enter into instance where his corpse exist
            CorpsePtr corpse = GetCorpse();
            if (corpse && corpse->GetType() == CORPSE_RESURRECTABLE && corpse->GetMapId() == mapid)
            {
                if( mEntry && (mEntry->map_type == MAP_INSTANCE || mEntry->map_type == MAP_RAID) )
                {
                    ResurrectPlayer();
                    SetHealth( GetMaxHealth()/2 );
                    SpawnCorpseBones();
                    SaveToDB(false);
                }
            }
            SetDontMove(true);
            //SaveToDB();

            // Client reset some data at NEW_WORLD teleport, resending its to client.
            SendInitWorldStates();
            SendInitialSpells();
            SendInitialActionButtons();

            // reset instance validity
            m_InstanceValid = true;

            // re-add us to the map here
            MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
        }
    }

    if (outofrange)
    {
        CombatStop();

        // remove selection
        if(GetSelection())
        {
            Unit* unit = ObjectAccessor::Instance().GetUnit(*this, GetSelection());
            if(unit)
                SendOutOfRange(unit);
        }

        // unsommon pet if lost
        if(pet && !IsWithinDistInMap(pet, OWNER_MAX_DISTANCE))
            RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
    }

    SetSemaphoreTeleport(false);

    UpdateZone(GetZoneId());

    if(old_zone != GetZoneId() && pvpInfo.inHostileArea)    // only at zone change
        CastSpell(this, 2479, true);
}

void Player::AddToWorld()
{
    Object::AddToWorld();

    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->AddToWorld();
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        if(m_items[i])
            m_items[i]->AddToWorld();
    }

    CorpsePtr corpse = GetCorpse();
    if(corpse)
        corpse->UpdateForPlayer(this,true);
}

void Player::RemoveFromWorld()
{
    for(int i = 0; i < BANK_SLOT_BAG_END; i++)
    {
        if(m_items[i])
            m_items[i]->RemoveFromWorld();
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        if(m_items[i])
            m_items[i]->RemoveFromWorld();
    }

    Object::RemoveFromWorld();
}

void Player::CalcRage( uint32 damage,bool attacker )
{
    float addRage;

    float rageconversion = ((0.0091107836 * getLevel()*getLevel())+3.225598133*getLevel())+4.2652911;

    if(attacker)
        addRage = damage/rageconversion*7.5;
    else
        addRage = damage/rageconversion*2.5;

    ModifyPower(POWER_RAGE, uint32(addRage*10));
}

void Player::RegenerateAll()
{
    if (m_regenTimer != 0)
        return;
    uint32 regenDelay = 2000;

    // Not in combat or they have regeneration
    if (!isInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
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
        if (curValue >= maxValue)
            return;
    }
    else if (curValue == 0)
        return;

    float addvalue = 0.0;

    switch (power)
    {
        case POWER_MANA:
        {
            float Spirit = GetStat(STAT_SPIRIT);
            uint8 Class = getClass();

            float ManaIncreaseRate = sWorld.getRate(RATE_POWER_MANA);
            if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;
            // If < 5s after previous cast which used mana, no regeneration unless
            // we happen to have a modifer that adds it back
            // If > 5s, get portion between the 5s and now, up to a maximum of 2s worth
            uint32 msecSinceLastCast;
            msecSinceLastCast = ((uint32)getMSTime() - m_lastManaUse);
            if (msecSinceLastCast < 7000)
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
                case CLASS_DRUID:   addvalue = (Spirit/5 + 15)   * ManaIncreaseRate; break;
                case CLASS_HUNTER:  addvalue = (Spirit/5 + 15)   * ManaIncreaseRate; break;
                case CLASS_MAGE:    addvalue = (Spirit/4 + 12.5) * ManaIncreaseRate; break;
                case CLASS_PALADIN: addvalue = (Spirit/5 + 15)   * ManaIncreaseRate; break;
                case CLASS_PRIEST:  addvalue = (Spirit/4 + 12.5) * ManaIncreaseRate; break;
                case CLASS_SHAMAN:  addvalue = (Spirit/5 + 17)   * ManaIncreaseRate; break;
                case CLASS_WARLOCK: addvalue = (Spirit/5 + 15)   * ManaIncreaseRate; break;
            }
        }   break;
        case POWER_RAGE:                                    // Regenerate rage
        {
            float RageIncreaseRate = sWorld.getRate(RATE_POWER_RAGE);
            if( RageIncreaseRate <= 0 ) RageIncreaseRate = 1;
            addvalue = 30 * RageIncreaseRate;               // 3 rage by tick
        }   break;
        case POWER_ENERGY:                                  // Regenerate energy (rogue)
            addvalue = 20;
            break;
        case POWER_FOCUS:
        case POWER_HAPPINESS:
            break;
    }

    if (power != POWER_RAGE)
    {
        curValue += uint32(addvalue);
        if (curValue > maxValue)
            curValue = maxValue;
    }
    else
    {
        if(curValue <= uint32(addvalue))
            curValue = 0;
        else
            curValue -= uint32(addvalue);
    }
    SetPower(power, curValue);
}

void Player::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH);

    float Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;

    float addvalue = 0.0;

    switch (Class)
    {
        case CLASS_DRUID:   addvalue = (Spirit*0.11 + 1)   * HealthIncreaseRate; break;
        case CLASS_HUNTER:  addvalue = (Spirit*0.43 - 5.5) * HealthIncreaseRate; break;
        case CLASS_MAGE:    addvalue = (Spirit*0.11 + 1)   * HealthIncreaseRate; break;
        case CLASS_PALADIN: addvalue = (Spirit*0.25)       * HealthIncreaseRate; break;
        case CLASS_PRIEST:  addvalue = (Spirit*0.15 + 1.4) * HealthIncreaseRate; break;
        case CLASS_ROGUE:   addvalue = (Spirit*0.84 - 13)  * HealthIncreaseRate; break;
        case CLASS_SHAMAN:  addvalue = (Spirit*0.28 - 3.6) * HealthIncreaseRate; break;
        case CLASS_WARLOCK: addvalue = (Spirit*0.12 + 1.5) * HealthIncreaseRate; break;
        case CLASS_WARRIOR: addvalue = (Spirit*1.26 - 22.6)* HealthIncreaseRate; break;
    }

    if (!isInCombat())
    {
        AuraList& mModHealthRegenPct = GetAurasByType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
        for(AuraList::iterator i = mModHealthRegenPct.begin(); i != mModHealthRegenPct.end(); ++i)
            addvalue *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
    }
    else if(HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
        addvalue *= m_AuraModifiers[SPELL_AURA_MOD_REGEN_DURING_COMBAT] / 100.0f;

    switch (getStandState())
    {
        case PLAYER_STATE_SIT_CHAIR:
        case PLAYER_STATE_SIT_LOW_CHAIR:
        case PLAYER_STATE_SIT_MEDIUM_CHAIR:
        case PLAYER_STATE_SIT_HIGH_CHAIR:
        case PLAYER_STATE_SIT:
        case PLAYER_STATE_SLEEP:
        case PLAYER_STATE_KNEEL:
            addvalue *= 1.5; break;
    }

    if(addvalue < 0)
        addvalue = 0;

    ModifyHealth(int32(addvalue));
}

bool Player::isAcceptTickets() const
{
    return GetSession()->GetSecurity() >=2 && (m_GMFlags & GM_ACCEPT_TICKETS);
}

bool Player::CanInteractWithNPCs(bool alive) const
{
    if(alive && !isAlive())
        return false;
    if(isInFlight())
        return false;
    if(isInCombat())
        return false;

    return true;
}

bool Player::IsUnderWater() const
{
    return IsInWater() &&
        GetPositionZ() < (MapManager::Instance().GetMap(GetMapId(), this)->GetWaterLevel(GetPositionX(),GetPositionY())-2);
}

void Player::SetInWater(bool apply)
{
    //define player in water by opcodes
    //move player's guid into HateOfflineList of those mobs
    //which can't swim and move guid back into ThreatList when
    //on surface.
    //TODO: exist also swimming mobs, and function must be symmetric to enter/leave water
    m_isInWater = apply;

    // form update
    if(apply)
    {
        // remove travel forms
        if(m_form == FORM_TRAVEL || m_form == FORM_GHOSTWOLF)
            RemoveAurasDueToSpell(m_ShapeShiftForm);

        // remove mounts, check flight mounts also (flying=swimming in air :D)
        if(IsMounted() && !IsFlying())
            RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    }
    else
    {
        // remove aqua form
        if(m_form == FORM_AQUA)
            RemoveAurasDueToSpell(m_ShapeShiftForm);
    }

    // update threat tables
    uint64 guid = GetGUID();
    InHateListOf& InHateList = GetInHateListOf();
    for(InHateListOf::iterator iter = InHateList.begin(); iter != InHateList.end(); iter++)
    {
        if(!isInAccessablePlaceFor(*iter) )
            (*iter)->MoveGuidToOfflineList(guid);
        else
            (*iter)->MoveGuidToThreatList(guid);
    }
}

void Player::SetGameMaster(bool on)
{
    if(on)
    {
        m_GMFlags |= GM_ON;
        setFaction(35);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);
        MoveToHateOfflineList();
        CombatStop(true);
    }
    else
    {
        m_GMFlags &= ~GM_ON;
        setFactionForRace(getRace());
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);
        MoveToThreatList();
    }
}

void Player::SetGMVisible(bool on)
{
    if(on)
    {
        m_GMFlags &= ~GM_INVISIBLE;                         //remove flag

        SetVisibility(VISIBILITY_ON);
    }
    else
    {
        m_GMFlags |= GM_INVISIBLE;                          //add flag

        SetAcceptWhispers(false);
        SetGameMaster(true);

        SetVisibility(VISIBILITY_OFF);
    }
}

bool Player::IsGroupVisibleFor(Player* p) const
{
    switch(sWorld.getConfig(CONFIG_GROUP_VISIBILITY))
    {
        default: return IsInSameGroupWith(p);
        case 1:  return IsInSameRaidWith(p);
        case 2:  return GetTeam()==p->GetTeam();
    }
}

bool Player::IsInSameGroupWith(Player const* p) const
{
    return  groupInfo.group != NULL &&
        groupInfo.group == p->groupInfo.group &&
        groupInfo.group->SameSubGroup(GetGUID(), p->GetGUID());
}

///- If the player is invited, remove him. If the group if then only 1 person, disband the group.
/// \todo Should'nt we also check if there is no other invitees before disbanding the group?
void Player::UninviteFromGroup()
{
    if(groupInfo.invite)                                    // uninvite invitee
    {
        Group* group = groupInfo.invite;
        group->RemoveInvite(this);

        if(group->GetMembersCount() <= 1)                   // group has just 1 member => disband
        {
            group->Disband(true);
            objmgr.RemoveGroup(group);
            delete group;
            group = NULL;
        }
    }
}

void Player::RemoveFromGroup(Group* group, uint64 guid)
{
    if(group)
    {
        if (group->GetMembersCount() <= 2)
        {   
            // If there is only 2 members left and 1 leave, just remove the whole group!
            group->Disband();
            objmgr.RemoveGroup(group);
            delete group;
        }
        else if (group->RemoveMember(guid, 0) <= 1)
        {
            // group->RemoveMember(guid, 0) causes a crash with 2 members! fix above!
            group->Disband();
            objmgr.RemoveGroup(group);
            delete group;
        }
    }
}

void Player::SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 RestXP)
{
    WorldPacket data(SMSG_LOG_XPGAIN, (21));
    data << ( victim ? victim->GetGUID() : uint64(0) );
    data << uint32(GivenXP+RestXP);                         // given experience
    data << ( victim ? (uint8)0 : (uint8)1 );               // 00-kill_xp type, 01-non_kill_xp type
    data << uint32(GivenXP);                                // experience without rested bonus
    data << float(1);                                       // 1 - none 0 - 100% group bonus output
    GetSession()->SendPacket(&data);
}

void Player::GiveXP(uint32 xp, Unit* victim)
{
    if ( xp < 1 )
        return;

    if(!isAlive())
        return;

    uint32 level = getLevel();

    // XP to money conversion processed in Player::RewardQuest
    if(level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    // XP resting bonus for kill
    uint32 rested_bonus_xp = victim ? GetXPRestBonus(xp) : 0;

    SendLogXPGain(xp,victim,rested_bonus_xp);

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp + rested_bonus_xp;

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
// Current player experience not update (must be update by caller)
void Player::GiveLevel()
{
    uint32 level = getLevel();

    if ( level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
        return;

    level += 1;

    InitStatsForLevel(level);
    InitTalentForLevel();

    // give level to summoned pet
    Pet* pet = GetPet();
    if(pet && pet->getPetType()==SUMMON_PET)
        pet->GivePetLevel(level);
}

void Player::InitTalentForLevel()
{
    uint32 level = getLevel();
    // talents base at level diff ( talents = level - 9 but some can be used already)
    if(level < 10)
    {
        // Remove all talent points
        if(m_usedTalentCount > 0)                           // Free any used talents
        {
            resetTalents(true);
            SetUInt32Value(PLAYER_CHARACTER_POINTS1,0);
        }
    }
    else
    {
        // if used more that have then reset
        if(m_usedTalentCount > level-9)
            resetTalents(true);
        // else update amount of free points
        else
            SetUInt32Value(PLAYER_CHARACTER_POINTS1,level-9-m_usedTalentCount);
    }
}

void Player::InitStatsForLevel(uint32 level, bool sendgain, bool remove_mods)
{
    // Remove item, aura, stats bonuses
    if(remove_mods)
    {
        _RemoveAllItemMods();
        _RemoveAllAuraMods();
        _RemoveStatsMods();
    }

    PlayerLevelInfo info;

    objmgr.GetPlayerLevelInfo(getRace(),getClass(),level,&info);

    if(sendgain)
    {
        // send levelup info to client
        WorldPacket data(SMSG_LEVELUP_INFO, (7*4+(MAX_STATS-STAT_STRENGTH)+4));
        data << uint32(level);
        data << uint32(int32(info.health) - GetMaxHealth());
        data << uint32(int32(info.mana)   - GetMaxPower(POWER_MANA));
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);

        for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
            data << uint32(int32(info.stats[i]) - GetStat(Stats(i)));

        GetSession()->SendPacket(&data);
    }

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, MaNGOS::XP::xp_to_level(level));

    // update level, max level of skills
    if(getLevel()!=level)
        m_Played_time[1] = 0;                               // Level Played Time reset
    SetLevel( level);
    UpdateMaxSkills ();

    // set default cast time multiplier
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    // reset size before reapply auras
    if (getRace() == RACE_TAUREN)
        SetFloatValue(OBJECT_FIELD_SCALE_X,1.35f);
    else
        SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

    // save base values (bonuses already included in stored stats
    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetStat(Stats(i), info.stats[i]);

    // restore if need some important flags
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN1 );

    SetArmor(m_createStats[STAT_AGILITY]*2);

    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetPosStat(Stats(i), 0);

    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetNegStat(Stats(i), 0);

    // Base crit values
    switch(getClass())
    {
        case CLASS_PALADIN: SetFloatValue(PLAYER_CRIT_PERCENTAGE, 0.7 ); break;
        case CLASS_PRIEST:  SetFloatValue(PLAYER_CRIT_PERCENTAGE, 3.0 ); break;
        case CLASS_SHAMAN:  SetFloatValue(PLAYER_CRIT_PERCENTAGE, 1.7 ); break;
        case CLASS_MAGE:    SetFloatValue(PLAYER_CRIT_PERCENTAGE, 3.2 ); break;
        case CLASS_WARLOCK: SetFloatValue(PLAYER_CRIT_PERCENTAGE, 2.0 ); break;
        case CLASS_DRUID:   SetFloatValue(PLAYER_CRIT_PERCENTAGE, 0.92); break;
        case CLASS_ROGUE:
        case CLASS_HUNTER:
        case CLASS_WARRIOR:
        default:            SetFloatValue(PLAYER_CRIT_PERCENTAGE, 0.0 ); break;
    }

    // Base spell crit values
    float base_spell_crit[MAX_CLASSES] = {0,0,3.70,0,0,2.97,0,3.54,3.70,3.18,0,3.33};
    for (uint8 i = 0; i < 7; ++i)
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1+i, base_spell_crit[getClass()]);

    // Base parry percents
    SetFloatValue(PLAYER_PARRY_PERCENTAGE, 5);

    // Base dodge values
    switch(getClass())
    {
        case CLASS_PALADIN: SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.75); break;
        case CLASS_HUNTER:  SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.64); break;
        case CLASS_PRIEST:  SetFloatValue(PLAYER_DODGE_PERCENTAGE, 3.0 ); break;
        case CLASS_SHAMAN:  SetFloatValue(PLAYER_DODGE_PERCENTAGE, 1.75); break;
        case CLASS_MAGE:    SetFloatValue(PLAYER_DODGE_PERCENTAGE, 3.25); break;
        case CLASS_WARLOCK: SetFloatValue(PLAYER_DODGE_PERCENTAGE, 2.0 ); break;
        case CLASS_DRUID:   SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.75); break;
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
        default:            SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.0 ); break;
    }

    // set armor (resistance 0) to original value (create_agility*2)
    SetArmor(m_createStats[STAT_AGILITY]*2);
    SetResistanceBuffMods(SpellSchools(0), true, 0);
    SetResistanceBuffMods(SpellSchools(0), false, 0);
    // set other resistance to original value (0)
    for (int i = 1; i < MAX_SPELL_SCHOOL; i++)
    {
        SetResistance(SpellSchools(i), 0);
        SetResistanceBuffMods(SpellSchools(i), true, 0);
        SetResistanceBuffMods(SpellSchools(i), false, 0);
    }

    InitDataForForm();

    // save new stats
    SetMaxPower(POWER_MANA,  info.mana);
    SetMaxPower(POWER_RAGE,  1000 );
    SetMaxPower(POWER_ENERGY,100 );
    SetMaxPower(POWER_FOCUS, 0 );
    SetMaxPower(POWER_HAPPINESS, 0 );

    SetMaxHealth(info.health);

    // cleanup mounted state (it will set correctly at aura loading if player saved at mount.
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT );

    // apply stats, aura, items mods
    if(remove_mods)
    {
        _ApplyStatsMods();
        _ApplyAllAuraMods();
        _ApplyAllItemMods();
    }

    // update dependent from level part BlockChanceWithoutMods = 5 + (GetDefenceSkillValue() - getLevel()*5)*0.04);
    // must called with applied AuraMods (removed in call code)
    UpdateBlockPercentage();

    // set current level health and mana/energy to maximum after appling all mods.
    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    if(GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetPower(POWER_RAGE, GetMaxPower(POWER_RAGE));
    SetPower(POWER_FOCUS, 0);
    SetPower(POWER_HAPPINESS, 0);
}

void Player::SendInitialSpells()
{
    uint16 spellCount = 0;

    PlayerSpellMap::const_iterator itr;

    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(itr->second->state == PLAYERSPELL_REMOVED)
            continue;

        if(itr->second->active)
            spellCount +=1;
    }

    WorldPacket data(SMSG_INITIAL_SPELLS, (1+2+2+4*m_spells.size()));
    data << uint8(0);
    data << uint16(spellCount);

    for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(itr->second->state == PLAYERSPELL_REMOVED)
            continue;

        if(!itr->second->active)
            continue;

        data << uint16(itr->first);
        //data << uint16(itr->second->slotId);
        data << uint16(0); // it's not slot id
    }

    uint16 spellCooldowns = m_spellCooldowns.size();
    data << spellCooldowns;
    for(SpellCooldowns::const_iterator itr=m_spellCooldowns.begin(); itr!=m_spellCooldowns.end(); itr++)
    {
        SpellEntry const *sEntry = sSpellStore.LookupEntry(itr->first);
        if(!sEntry)
            continue;

        data << uint16(itr->first);

        time_t cooldown = 0;
        time_t curTime = time(NULL);
        if(itr->second.end > curTime)
            cooldown = (itr->second.end-curTime)*1000;

        data << uint16(itr->second.itemid);                 // cast item id
        data << uint16(sEntry->Category);                   // spell category
        if(sEntry->Category)                                // may be wrong, but anyway better than nothing...
        {
            data << uint32(0);
            data << uint32(cooldown);
        }
        else
        {
            data << uint32(cooldown);
            data << uint32(0);
        }
    }

    GetSession()->SendPacket(&data);

    sLog.outDetail( "CHARACTER: Sent Initial Spells" );
}

void Player::RemoveMail(uint32 id)
{
    for(PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end();++itr)
    {
        if ((*itr)->messageID == id)
        {
            //do not delete item. because Player::removeMail() is called when returning mail to sender.
            m_mail.erase(itr);
            return;
        }
    }
}

//call this function when mail receiver is online
void Player::CreateMail(uint32 mailId, uint8 messageType, uint32 sender, std::string subject, uint32 itemTextId, uint32 itemGuid, uint32 item_template, time_t expire_time, time_t deliver_time, uint32 money, uint32 COD, uint32 checked, Item* pItem)
{
    if(deliver_time <= time(NULL))                          // ready now
    {
        unReadMails++;
        SendNewMail();
    }
    else                                                    // not ready and no have ready mails
    {
        if(!m_nextMailDelivereTime || m_nextMailDelivereTime > deliver_time)
            m_nextMailDelivereTime =  deliver_time;
    }

    if ( !m_mailsLoaded )
    {
        if ( pItem )
            delete pItem;
        return;
    }
    Mail * m = new Mail;
    m->messageID = mailId;
    m->messageType = messageType;
    m->sender = sender;
    m->receiver = this->GetGUIDLow();
    m->subject = subject;
    m->itemTextId = itemTextId;
    m->item_guid = itemGuid;
    m->item_template = item_template;
    m->expire_time = expire_time;
    m->deliver_time = deliver_time;
    m->money = money;
    m->COD = COD;
    m->checked = checked;
    m->state = MAIL_STATE_UNCHANGED;

    m_mail.push_front(m);                                   //to insert new mail to beginning of maillist
    if ( pItem )
        AddMItem(pItem);
}

void Player::SendMailResult(uint32 mailId, uint32 mailAction, uint32 mailError, uint32 equipError)
{
    WorldPacket data(SMSG_SEND_MAIL_RESULT, (12));
    data << (uint32) mailId;
    data << (uint32) mailAction;
    data << (uint32) mailError;
    if (equipError)
        data << (uint32) equipError;
    GetSession()->SendPacket(&data);
}

void Player::SendNewMail()
{
    // deliver undelivered mail
    WorldPacket data(SMSG_RECEIVED_MAIL, 4);
    data << (uint32) 0;
    GetSession()->SendPacket(&data);
}

void Player::UpdateNextMailTimeAndUnreads()
{
    // calculate next delivery time (min. from non-delivered mails
    // and recalculate unReadMail
    time_t cTime = time(NULL);
    m_nextMailDelivereTime = 0;
    unReadMails = 0;
    for(PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if((*itr)->deliver_time > cTime)
        {
            if(!m_nextMailDelivereTime || m_nextMailDelivereTime > (*itr)->deliver_time)
                m_nextMailDelivereTime = (*itr)->deliver_time;
        }
        if(((*itr)->state | READ) == 0)
            ++unReadMails;
    }
}

bool Player::addSpell(uint16 spell_id, uint8 active, PlayerSpellState state, uint16 slot_id)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
    {
        sLog.outError("Player::addSpell: Non-existed in SpellStore spell #%u request.",spell_id);
        return false;
    }

    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        if (itr->second->state == PLAYERSPELL_REMOVED)
        {
            delete itr->second;
            m_spells.erase(itr);
            state = PLAYERSPELL_CHANGED;
        }
        else if (state == PLAYERSPELL_UNCHANGED && itr->second->state != PLAYERSPELL_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second->state = PLAYERSPELL_UNCHANGED;
            return false;
        }
        else
            return false;
    }

    PlayerSpell *newspell;

    newspell = new PlayerSpell;
    newspell->active = active;
    newspell->state = state;

    bool superceded_old = false;

    // replace spells in action bars and spellbook to bigger rank if only one spell rank must be accessible
    if(newspell->active && !objmgr.canStackSpellRank(spellInfo))
    {
        PlayerSpellMap::iterator itr;
        for (itr = m_spells.begin(); itr != m_spells.end(); itr++)
        {
            if(itr->second->state == PLAYERSPELL_REMOVED) continue;
            SpellEntry const *i_spellInfo = sSpellStore.LookupEntry(itr->first);
            if(!i_spellInfo) continue;

            if(objmgr.IsRankSpellDueToSpell(spellInfo,itr->first))
            {
                if(itr->second->active)
                {
                    WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));

                    if(objmgr.GetSpellRank(spell_id) >= objmgr.GetSpellRank(itr->first))
                    {
                        data << uint16(itr->first);
                        data << uint16(spell_id);

                        // mark old spell as disable (SMSG_SUPERCEDED_SPELL replace it in client by new)
                        itr->second->active = 0;
                        superceded_old = true;              // new spell replace old in action bars and spell book.
                    }
                    else
                    {
                        data << uint16(spell_id);
                        data << uint16(itr->first);

                        // mark new spell as disable (not learned yet for client and will not learned)
                        newspell->active = 0;
                    }

                    GetSession()->SendPacket( &data );
                }
            }
        }
    }

    uint16 tmpslot=slot_id;

    if (tmpslot == 0xffff)
    {
        uint16 maxid = 0;
        PlayerSpellMap::iterator itr;
        for (itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        {
            if(itr->second->state == PLAYERSPELL_REMOVED) continue;
            if (itr->second->slotId > maxid) maxid = itr->second->slotId;
        }
        tmpslot = maxid + 1;
    }

    newspell->slotId = tmpslot;
    m_spells[spell_id] = newspell;

    if (IsPassiveSpell(spell_id) || GetTalentSpellCost(spell_id) != 0 && objmgr.IsSpellLearnSpell(spell_id))
    {
        // if spell doesn't require a stance or the player is in the required stance
        if( (!spellInfo->Stances && spell_id != 5420 && spell_id != 5419 && spell_id != 7376 &&
            spell_id != 7381 && spell_id != 21156 && spell_id != 21009 && spell_id != 21178) ||
            m_form != 0 && (spellInfo->Stances & (1<<(m_form-1))) ||
            (spell_id ==  5420 && m_form == FORM_TREE) ||
            (spell_id ==  5419 && m_form == FORM_TRAVEL) ||
            (spell_id ==  7376 && m_form == FORM_DEFENSIVESTANCE) ||
            (spell_id ==  7381 && m_form == FORM_BERSERKERSTANCE) ||
            (spell_id == 21156 && m_form == FORM_BATTLESTANCE)||
            (spell_id == 21178 && m_form == FORM_BEAR))
            CastSpell(this, spell_id, true);
    }

    // update used talent points count
    m_usedTalentCount += GetTalentSpellCost(spell_id);

    // add dependent skills
    uint16 maxskill     = GetMaxSkillValueForLevel();

    ObjectMgr::SpellLearnSkillNode const* spellLearnSkill = objmgr.GetSpellLearnSkill(spell_id);

    if(spellLearnSkill)
    {
        uint32 skill_value = GetPureSkillValue(spellLearnSkill->skill);
        uint32 skill_max_value = GetMaxSkillValue(spellLearnSkill->skill);

        if(skill_value < spellLearnSkill->value)
            skill_value = spellLearnSkill->value;

        uint32 new_skill_max_value = spellLearnSkill->maxvalue == 0 ? maxskill : spellLearnSkill->maxvalue;

        if(skill_max_value < new_skill_max_value)
            skill_max_value =  new_skill_max_value;

        SetSkill(spellLearnSkill->skill,skill_value,skill_max_value);
    }

    // learn dependent spells
    ObjectMgr::SpellLearnSpellMap::const_iterator spell_begin = objmgr.GetBeginSpellLearnSpell(spell_id);
    ObjectMgr::SpellLearnSpellMap::const_iterator spell_end   = objmgr.GetEndSpellLearnSpell(spell_id);

    for(ObjectMgr::SpellLearnSpellMap::const_iterator itr = spell_begin; itr != spell_end; ++itr)
    {
        if(!itr->second.autoLearned && (!itr->second.ifNoSpell || !HasSpell(itr->second.ifNoSpell)))
            learnSpell(itr->second.spell);
    }

    // return true (for send learn packet) only if spell active (in case ranked spells) and not replace old spell
    return newspell->active && !superceded_old;
}

bool Player::learnSpell(uint16 spell_id)
{
    // prevent duplicated entires in spell book
    if (!addSpell(spell_id,1))
        return false;

    WorldPacket data(SMSG_LEARNED_SPELL, 4);
    data <<uint32(spell_id);
    GetSession()->SendPacket(&data);
    return true;
}

void Player::removeSpell(uint16 spell_id)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    if(itr->second->state == PLAYERSPELL_REMOVED)
        return;

    // removing
    WorldPacket data(SMSG_REMOVED_SPELL, 4);
    data << spell_id;
    GetSession()->SendPacket(&data);

    if(itr->second->state == PLAYERSPELL_NEW)
    {
        delete itr->second;
        m_spells.erase(itr);
    }
    else
        itr->second->state = PLAYERSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);

    // free talent points
    uint32 talentCosts = GetTalentSpellCost(spell_id);
    if(talentCosts > 0)
    {
        if(talentCosts < m_usedTalentCount)
            m_usedTalentCount -= talentCosts;
        else
            m_usedTalentCount = 0;
    }

    // remove dependent skill
    ObjectMgr::SpellLearnSkillNode const* spellLearnSkill = objmgr.GetSpellLearnSkill(spell_id);

    if(spellLearnSkill)
    {
        uint32 prev_spell = objmgr.GetPrevSpellInChain(spell_id);
        if(!prev_spell)                                     // first rank, remove skill
            SetSkill(spellLearnSkill->skill,0,0);
        else
        {
            // search prev. skill setting by spell ranks chain
            ObjectMgr::SpellLearnSkillNode const* prevSkill = objmgr.GetSpellLearnSkill(prev_spell);
            while(!prevSkill && prev_spell)
            {
                prev_spell = objmgr.GetPrevSpellInChain(prev_spell);
                prevSkill = objmgr.GetSpellLearnSkill(objmgr.GetFirstSpellInChain(prev_spell));
            }

            if(!prevSkill)                                  // not found prev skill setting, remove skill
                SetSkill(spellLearnSkill->skill,0,0);
            else                                            // set to prev. skill setting values
            {
                uint32 skill_value = GetPureSkillValue(prevSkill->skill);
                uint32 skill_max_value = GetMaxSkillValue(prevSkill->skill);

                if(skill_value >  prevSkill->value)
                    skill_value = prevSkill->value;

                uint32 new_skill_max_value = prevSkill->maxvalue == 0 ? GetMaxSkillValueForLevel() : prevSkill->maxvalue;

                if(skill_max_value > new_skill_max_value)
                    skill_max_value =  new_skill_max_value;

                SetSkill(prevSkill->skill,skill_value,skill_max_value);
            }
        }

    }

    // remove dependent spells
    ObjectMgr::SpellLearnSpellMap::const_iterator spell_begin = objmgr.GetBeginSpellLearnSpell(spell_id);
    ObjectMgr::SpellLearnSpellMap::const_iterator spell_end   = objmgr.GetEndSpellLearnSpell(spell_id);

    for(ObjectMgr::SpellLearnSpellMap::const_iterator itr = spell_begin; itr != spell_end; ++itr)
        removeSpell(itr->second.spell);
}

void Player::RemoveAllSpellCooldown()
{
    if(m_spellCooldowns.size() > 0)
    {
        for(SpellCooldowns::const_iterator itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end(); ++itr)
        {
            sLog.outError("SpellID:%u",uint32(itr->first));
            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8+4));
            data << uint32(itr->first);
            data << GetGUID();
            GetSession()->SendPacket(&data);
        }
        m_spellCooldowns.clear();
    }
}

void Player::_LoadSpellCooldowns()
{
    m_spellCooldowns.clear();

    QueryResult *result = sDatabase.Query("SELECT `spell`,`item`,`time` FROM `character_spell_cooldown` WHERE `guid` = '%u'",GetGUIDLow());

    if(result)
    {
        time_t curTime = time(NULL);

        do
        {
            Field *fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            uint32 item_id  = fields[1].GetUInt32();
            time_t db_time  = (time_t)fields[2].GetUInt64();

            if(!sSpellStore.LookupEntry(spell_id))
            {
                sLog.outError("Player %u have unknown spell %u in `character_spell_cooldown`, skipping.",GetGUIDLow(),spell_id);
                continue;
            }

            // skip outdated cooldown
            if(db_time <= curTime)
                continue;

            AddSpellCooldown(spell_id, item_id, db_time);

            sLog.outDebug("Player (GUID: %u) spell %u, item %u cooldown loaded (%u secs).", GetGUIDLow(), spell_id, item_id, uint32(db_time-curTime));
        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_SaveSpellCooldowns()
{
    sDatabase.Execute("DELETE FROM `character_spell_cooldown` WHERE `guid` = '%u'", GetGUIDLow());

    time_t curTime = time(NULL);

    // remove outdated and save active
    for(SpellCooldowns::iterator itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end();)
    {
        if(itr->second.end <= curTime)
            m_spellCooldowns.erase(itr++);
        else
        {
            sDatabase.Execute("INSERT INTO `character_spell_cooldown` (`guid`,`spell`,`item`,`time`) VALUES ('%u', '%u', '%u', '" I64FMTD "')", GetGUIDLow(), itr->first, itr->second.itemid, uint64(itr->second.end));
            ++itr;
        }
    }
}

uint32 Player::resetTalentsCost() const
{
    // The first time reset costs 1 gold
    if(m_resetTalentsCost < 1*GOLD)
        return 1*GOLD;
    // then 5 gold
    else if(m_resetTalentsCost < 5*GOLD)
        return 5*GOLD;
    // After that it increases in increments of 5 gold
    else if(m_resetTalentsCost < 10*GOLD)
        return 10*GOLD;
    else
    {
        uint32 months = (sWorld.GetGameTime() - m_resetTalentsTime)/MONTH;
        if(months > 0)
        {
            // This cost will be reduced by a rate of 5 gold per month
            int32 new_cost = int32(m_resetTalentsCost) - 5*GOLD*months;
            // to a minimum of 10 gold.
            return (new_cost < 10*GOLD ? 10*GOLD : new_cost);
        }
        else
        {
            // After that it increases in increments of 5 gold
            int32 new_cost = m_resetTalentsCost + 5*GOLD;
            // until it hits a cap of 50 gold.
            if(new_cost > 50*GOLD)
                new_cost = 50*GOLD;
            return new_cost;
        }
    }
}

bool Player::resetTalents(bool no_cost)
{
    uint32 level = getLevel();
    if (m_usedTalentCount == 0)
    {
        SetUInt32Value(PLAYER_CHARACTER_POINTS1,(level < 10 ? 0 : level-9));
        return false;
    }

    uint32 cost = 0;

    if(!no_cost)
    {
        cost = resetTalentsCost();

        if (GetMoney() < cost)
        {
            SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            return false;
        }
    }

    for (int i = 0; i < sTalentStore.GetNumRows(); i++)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; j++)
        {
            for(PlayerSpellMap::iterator itr = GetSpellMap().begin(); itr != GetSpellMap().end();)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED)
                {
                    ++itr;
                    continue;
                }

                // remove learned spells (all ranks)
                uint32 itrFirstId = objmgr.GetFirstSpellInChain(itr->first);
                if (itrFirstId == talentInfo->RankID[j])
                {
                    RemoveAurasDueToSpell(itr->first);
                    removeSpell(itr->first);
                    itr = GetSpellMap().begin();
                    continue;
                }
                else
                    ++itr;
            }
        }
    }

    SetUInt32Value(PLAYER_CHARACTER_POINTS1,(level < 10 ? 0 : level-9));

    if(!no_cost)
    {
        ModifyMoney(-(int32)cost);

        m_resetTalentsCost = cost;
        m_resetTalentsTime = time(NULL);
    }
    return true;
}

bool Player::_removeSpell(uint16 spell_id)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        delete itr->second;
        m_spells.erase(itr);
        return true;
    }
    return false;
}

Mail* Player::GetMail(uint32 id)
{
    for(PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); itr++)
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
        for(uint16 index = 0; index < m_valuesCount; index++)
        {
            if(GetUInt32Value(index) != 0 && updateVisualBits.GetBit(index))
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
        Object::_SetUpdateBits(updateMask, target);
        *updateMask &= updateVisualBits;
    }
}

void Player::InitVisibleBits()
{
    updateVisualBits.SetCount(PLAYER_END);

    updateVisualBits.SetBit(OBJECT_FIELD_GUID);
    updateVisualBits.SetBit(OBJECT_FIELD_TYPE);
    updateVisualBits.SetBit(OBJECT_FIELD_SCALE_X);

    updateVisualBits.SetBit(UNIT_FIELD_SUMMON);
    updateVisualBits.SetBit(UNIT_FIELD_SUMMON+1);

    updateVisualBits.SetBit(UNIT_FIELD_TARGET);
    updateVisualBits.SetBit(UNIT_FIELD_TARGET+1);

    updateVisualBits.SetBit(UNIT_FIELD_HEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_POWER1);
    updateVisualBits.SetBit(UNIT_FIELD_POWER2);
    updateVisualBits.SetBit(UNIT_FIELD_POWER3);
    updateVisualBits.SetBit(UNIT_FIELD_POWER4);
    updateVisualBits.SetBit(UNIT_FIELD_POWER5);

    updateVisualBits.SetBit(UNIT_FIELD_MAXHEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER1);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER2);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER3);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER4);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER5);

    updateVisualBits.SetBit(UNIT_FIELD_LEVEL);
    updateVisualBits.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_0);
    updateVisualBits.SetBit(UNIT_FIELD_FLAGS);
    for(uint16 i = UNIT_FIELD_AURA; i < UNIT_FIELD_AURASTATE; i ++)
        updateVisualBits.SetBit(i);
    updateVisualBits.SetBit(UNIT_FIELD_BASEATTACKTIME);
    updateVisualBits.SetBit(UNIT_FIELD_OFFHANDATTACKTIME);
    updateVisualBits.SetBit(UNIT_FIELD_RANGEDATTACKTIME);
    updateVisualBits.SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    updateVisualBits.SetBit(UNIT_FIELD_COMBATREACH);
    updateVisualBits.SetBit(UNIT_FIELD_DISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_1);
    updateVisualBits.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_PETNUMBER);
    updateVisualBits.SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    updateVisualBits.SetBit(UNIT_DYNAMIC_FLAGS);
    updateVisualBits.SetBit(UNIT_MOD_CAST_SPEED);           // ?
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_2);            // ?

    updateVisualBits.SetBit(PLAYER_FLAGS);
    updateVisualBits.SetBit(PLAYER_BYTES);
    updateVisualBits.SetBit(PLAYER_BYTES_2);
    updateVisualBits.SetBit(PLAYER_BYTES_3);
    updateVisualBits.SetBit(PLAYER_GUILDID);
    updateVisualBits.SetBit(PLAYER_GUILDRANK);
    updateVisualBits.SetBit(PLAYER_GUILD_TIMESTAMP);
    updateVisualBits.SetBit(PLAYER_DUEL_TEAM);
    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER);
    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER+1);

    // PLAYER_QUEST_LOG_x also visible bit on official...
    for(uint16 i = PLAYER_QUEST_LOG_1_1; i < PLAYER_QUEST_LOG_LAST_2; i+=3)
        updateVisualBits.SetBit(i);

    for(uint16 i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        updateVisualBits.SetBit((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + i*2));
        updateVisualBits.SetBit((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (i*2) + 1));
    }
    //Players visible items are not inventory stuff
    //431) = 884 (0x374) = main weapon
    for(uint16 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        uint16 visual_base = PLAYER_VISIBLE_ITEM_1_0 + (i*16);

        // item entry
        updateVisualBits.SetBit(visual_base + 0);

        // item enchantment IDs
        for(uint8 j = 0; j < 11; ++j)
            updateVisualBits.SetBit(visual_base +1 + j);

        // random properties
        updateVisualBits.SetBit((uint16)(PLAYER_VISIBLE_ITEM_1_PROPERTIES + (i*16)));
    }

    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO + 1);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO + 2);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO + 3);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO + 4);
    updateVisualBits.SetBit(UNIT_VIRTUAL_ITEM_INFO + 5);
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

        for(int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
        }
        for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
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

        for(int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer( target );
        }
        for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
        {
            if(m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer( target );
        }
    }
}

bool Player::HasSpell(uint32 spell) const
{
    PlayerSpellMap::const_iterator itr = m_spells.find((uint16)spell);
    return (itr != m_spells.end() && itr->second->state != PLAYERSPELL_REMOVED);
}

TrainerSpellState Player::GetTrainerSpellState(TrainerSpell const* trainer_spell)
{
    if (!trainer_spell)
        return TRAINER_SPELL_RED;

    uint32 learned_spell_id = trainer_spell->spell->EffectTriggerSpell[0];

    // get learned spell info
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(learned_spell_id);
    if (!spellInfo)
        return TRAINER_SPELL_RED;

    // known spell
    if(HasSpell(learned_spell_id))
        return TRAINER_SPELL_GRAY;

    // check level requirement
    if(getLevel() < ( trainer_spell->reqlevel ? trainer_spell->reqlevel : spellInfo->spellLevel))
        return TRAINER_SPELL_RED;

    // check prev.rank requirement
    uint32 prev_id =  objmgr.GetPrevSpellInChain(learned_spell_id);
    if(prev_id && !HasSpell(prev_id))
        return TRAINER_SPELL_RED;

    // check skill requirement
    if(trainer_spell->reqskill && GetPureSkillValue(trainer_spell->reqskill) < trainer_spell->reqskillvalue)
        return TRAINER_SPELL_RED;

    // secondary prof. or not prof. spell
    uint32 skill = spellInfo->EffectMiscValue[1];

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL || !IsPrimaryProfessionSkill(skill))
        return TRAINER_SPELL_GREEN;

    // check primary prof. limit
    uint32 value = 0;

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second->state == PLAYERSPELL_REMOVED) continue;
        SpellEntry const *pSpellInfo = sSpellStore.LookupEntry(itr->first);
        if(!pSpellInfo) continue;

        if(pSpellInfo->Effect[1] == SPELL_EFFECT_SKILL)
        {
            uint32 pskill = pSpellInfo->EffectMiscValue[1];
            if( !IsPrimaryProfessionSkill(pskill))
                continue;

            // not check prof count for not first prof. spells (when skill already known)
            if(pskill == skill)
                return TRAINER_SPELL_GREEN;

            // count only first rank prof. spells
            if(objmgr.GetSpellRank(pSpellInfo->Id)==1)
                value += 1;
        }
    }

    if(value >= sWorld.getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL))
        return TRAINER_SPELL_RED;

    return TRAINER_SPELL_GREEN;
}

void Player::DeleteFromDB()
{
    uint32 guid = GetGUIDLow();

    // convert corpse to bones if exist (to prevent exiting Corpse in World without DB entry)
    // bones will be deleted by corpse/bones deleting thread shortly
    SpawnCorpseBones();

    // remove from guild
    if(GetGuildId() != 0)
    {
        Guild* guild = objmgr.GetGuildById(GetGuildId());
        if(guild)
            guild->DelMember(guid);
    }

    // remove from group
    UninviteFromGroup();
    RemoveFromGroup();

    // remove signs from petitions (also remove petitions if owner);
    RemovePetitionsAndSigns(GetGUID());

    // unsummon and delete pet not required: player deleted from CLI or character list with not loaded pet.

    // NOW we can finally clear other DB data releted to character
    //sDatabase.BeginTransaction();

    for(int i = 0; i < BANK_SLOT_ITEM_END; i++)
    {
        if(m_items[i] == NULL)
            continue;
        m_items[i]->DeleteFromDB();                         // Bag items delete also by virtual call Bag::DeleteFromDB
    }

    //sDatabase.Execute("DELETE FROM `character` WHERE `guid` = '%u'",guid);
    sDatabase.WaitExecute("DELETE FROM `character` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_aura` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_spell` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_tutorial` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `item_instance` WHERE `owner_guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_gifts` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_inventory` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_queststatus` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_action` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_reputation` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_homebind` WHERE `guid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_kill` WHERE `guid` = '%u'",guid);

    sDatabase.Execute("DELETE FROM `character_social` WHERE `guid` = '%u' OR `friend`='%u'",guid,guid);
    m_ignorelist.clear();

    sDatabase.Execute("DELETE FROM `mail` WHERE `receiver` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `character_pet` WHERE `owner` = '%u'",guid);
    //sDatabase.CommitTransaction();

    //loginDatabase.Execute("UPDATE `realmcharacters` SET `numchars` = `numchars` - 1 WHERE `acctid` = %d AND `realmid` = %d", GetSession()->GetAccountId(), realmID);
    QueryResult *resultCount = sDatabase.Query("SELECT COUNT(guid) FROM `character` WHERE `account` = '%d'", GetSession()->GetAccountId());
    uint32 charCount = 0;
    if (resultCount)
    {
        Field *fields = resultCount->Fetch();
        charCount = fields[0].GetUInt32();
        delete resultCount;
        loginDatabase.Execute("INSERT INTO `realmcharacters` (`numchars`, `acctid`, `realmid`) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE `numchars` = '%d'", charCount, GetSession()->GetAccountId(), realmID, charCount);
    }
}

void Player::SetMovement(uint8 pType)
{
    WorldPacket data;

    switch(pType)
    {
        case MOVE_ROOT:
        {
            data.Initialize(SMSG_FORCE_MOVE_ROOT, GetPackGUID().size()+4);
            data.append(GetPackGUID());
            data << uint32(0);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_UNROOT:
        {
            data.Initialize(SMSG_FORCE_MOVE_UNROOT, GetPackGUID().size()+4);
            data.append(GetPackGUID());
            data << uint32(0);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_WATER_WALK:
        {
            data.Initialize(SMSG_MOVE_WATER_WALK, GetPackGUID().size()+4);
            data.append(GetPackGUID());
            data << uint32(0);
            GetSession()->SendPacket( &data );
        }break;
        case MOVE_LAND_WALK:
        {
            data.Initialize(SMSG_MOVE_LAND_WALK, GetPackGUID().size()+4);
            data.append(GetPackGUID());
            data << uint32(0);
            GetSession()->SendPacket( &data );
        }break;
        default:break;
    }
}

void Player::BuildPlayerRepop()
{
    if(getRace() == RACE_NIGHTELF)
        CastSpell(this, 20584, true);                       // auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
    CastSpell(this, 8326, true);                            // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

    // there must be SMSG.FORCE_RUN_SPEED_CHANGE, SMSG.FORCE_SWIM_SPEED_CHANGE, SMSG.MOVE_WATER_WALK
    // there must be SMSG.STOP_MIRROR_TIMER
    // there we must send 888 opcode

    // place corpse instead player body
    if(!GetCorpse())
        CreateCorpse();

    CorpsePtr corpse = GetCorpse();
    if (!corpse)
    {
        sLog.outError("Error creating corpse for Player %s [%u]", GetName(), GetGUIDLow());
        return;
    }

    // now show corpse for all
    if(corpse)
    {
        corpse->SetInstanceId(this->GetInstanceId());
        corpse->AddToWorld();
        MapManager::Instance().GetMap(corpse->GetMapId(), this)->Add(corpse);
    }

    // convert player body to ghost
    SetHealth( 1 );

    SetMovement(MOVE_WATER_WALK);
    SetMovement(MOVE_UNROOT);

    // setting new speed
    /*if (getRace() == RACE_NIGHTELF)
    {
        SetSpeed(MOVE_RUN,  1.5f*1.2f, true);
        SetSpeed(MOVE_SWIM, 1.5f*1.2f, true);
    }
    else
    {
        SetSpeed(MOVE_RUN,  1.5f, true);
        SetSpeed(MOVE_SWIM, 1.5f, true);
    }*/

    //! corpse reclaim delay 30 * 1000ms
    WorldPacket data(SMSG_CORPSE_RECLAIM_DELAY, 4);
    data << (uint32)(CORPSE_RECLAIM_DELAY*1000);
    GetSession()->SendPacket( &data );

    // to prevent cheating
    if(corpse)
        corpse->ResetGhostTime();

    data.Initialize(SMSG_UPDATE_AURA_DURATION, 5);          // last check 2.0.10
    data << uint8(0x28) << uint32(0);
    GetSession()->SendPacket( &data );

    StopMirrorTimer(FATIGUE_TIMER);                         //disable timers(bars)
    StopMirrorTimer(BREATH_TIMER);
    StopMirrorTimer(FIRE_TIMER);

    //SetUInt32Value(UNIT_FIELD_AURA + 32, 8326);           // set ghost form
    //SetUInt32Value(UNIT_FIELD_AURA + 33, 20584);          //!dono

    //SetUInt32Value(UNIT_FIELD_AURAFLAGS + 4, 0xEE);

    //SetUInt32Value(UNIT_FIELD_AURASTATE, 0x02);

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,(float)1.0);    //see radius of death player?

    SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_ALWAYS_STAND);

    //if (getRace() == RACE_NIGHTELF)
    //    SetUInt32Value(UNIT_FIELD_DISPLAYID, 1825);

    // set initial flags + set ghost + restore pvp
    //SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NONE | UNIT_FLAG_UNKNOWN1 | (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP)?UNIT_FLAG_PVP:0) );

    // reserve some flags + ad ghost flag
    //uint32 old_safe_flags = GetUInt32Value(PLAYER_FLAGS) & ( PLAYER_FLAGS_IN_PVP | PLAYER_FLAGS_HIDE_CLOAK | PLAYER_FLAGS_HIDE_HELM );
    //SetUInt32Value(PLAYER_FLAGS, old_safe_flags | PLAYER_FLAGS_GHOST );
}

void Player::SendDelayResponse(const uint32 ml_seconds)
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4);
    data << (uint32)getMSTime();
    GetSession()->SendPacket( &data );
}

void Player::ResurrectPlayer()
{
    WorldPacket data(SMSG_SH_POSITION, 4*4);                // remove spirit healer position
    data << uint32(-1);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    GetSession()->SendPacket(&data);

    // speed change, land walk

    // remove death flag + set aura
    //RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    RemoveFlag(UNIT_FIELD_BYTES_1, PLAYER_STATE_FLAG_ALL);
    if(getRace() == RACE_NIGHTELF)
        RemoveAurasDueToSpell(20584);                       // speed bonuses
    RemoveAurasDueToSpell(8326);                            // SPELL_AURA_GHOST

    setDeathState(ALIVE);

    SetMovement(MOVE_LAND_WALK);
    SetMovement(MOVE_UNROOT);

    if(InBattleGround())                                    // special case for battleground resurrection
    {
        SetHealth(GetMaxHealth());
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
        SetPower(POWER_RAGE, 0);
        SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    }

    //SetSpeed(MOVE_RUN,  1.0f, true);
    //SetSpeed(MOVE_SWIM, 1.0f, true);

    //SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);

    //SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    //SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    //SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    //SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    //SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    //if(getRace() == RACE_NIGHTELF)
    //{
    //    DeMorph();
    //}

    m_deathTimer = 0;

    // set resurrection sickness if not expired
    if(!m_resurrectingSicknessExpire)
        return;

    // check expire
    time_t curTime = time(NULL);
    if(m_resurrectingSicknessExpire <= curTime)
    {
        m_resurrectingSicknessExpire = 0;
        return;
    }

    // set resurrection sickness!
    uint32 delta = m_resurrectingSicknessExpire-time(NULL);

    SpellEntry const *spellInfo = sSpellStore.LookupEntry( SPELL_PASSIVE_RESURRECTION_SICKNESS );
    if(spellInfo)
    {
        Spell spell(this, spellInfo, true, NULL);

        SpellCastTargets targets;
        targets.setUnitTarget( this );

        spell.prepare(&targets);

        for(int i =0; i < 3; ++i)
        {
            Aura* Aur = GetAura(SPELL_PASSIVE_RESURRECTION_SICKNESS,i);
            if(Aur)
            {
                Aur->SetAuraDuration(delta*1000);
                Aur->UpdateAuraDuration();
            }
        }
    }
}

void Player::KillPlayer()
{
    if(InBattleGround())
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(GetBattleGroundId());
        if(bg)
        {
            if(GetTeam() == HORDE && bg->IsAllianceFlagPickedup())
            {
                if(bg->GetAllianceFlagPickerGUID() == GetGUID())
                {
                    bg->SetAllianceFlagPicker(0);
                    CastSpell(this, 23336, true);           // Alliance Flag Drop
                }
            }
            if(GetTeam() == ALLIANCE && bg->IsHordeFlagPickedup())
            {
                if(bg->GetHordeFlagPickerGUID() == GetGUID())
                {
                    bg->SetHordeFlagPicker(0);
                    CastSpell(this, 23334, true);           // Horde Flag Drop
                }
            }
            bg->UpdatePlayerScore(this, 4, 1);              // add +1 deaths
        }
    }

    SetMovement(MOVE_ROOT);

    StopMirrorTimer(FATIGUE_TIMER);                         //disable timers(bars)
    StopMirrorTimer(BREATH_TIMER);
    StopMirrorTimer(FIRE_TIMER);

    setDeathState(CORPSE);
    //SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_IN_PVP );

    SetFlag( UNIT_DYNAMIC_FLAGS, 0x00 );

    // 6 minutes until repop at graveyard
    m_deathTimer = 360000;

    // dead player body showed at this moment, corpse will be show at Player ghost repop
    CreateCorpse();
}

void Player::CreateCorpse()
{
    // prevent existence 2 corpse for player
    SpawnCorpseBones();

    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    CorpsePtr corpse(new Corpse(this, CORPSE_RESURRECTABLE));

    if(!corpse->Create(objmgr.GenerateLowGuid(HIGHGUID_CORPSE), this, GetMapId(), GetPositionX(),
        GetPositionY(), GetPositionZ(), GetOrientation()))
    {
        return ;
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

    corpse->SetUInt32Value( CORPSE_FIELD_BYTES_1, _cfb1 );
    corpse->SetUInt32Value( CORPSE_FIELD_BYTES_2, _cfb2 );
    corpse->SetUInt32Value( CORPSE_FIELD_FLAGS, 4 );
    corpse->SetUInt32Value( CORPSE_FIELD_DISPLAY_ID, GetUInt32Value(UNIT_FIELD_DISPLAYID) );

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
            corpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
        }
    }

    corpse->SaveToDB();

    // register for player, but not show
    ObjectAccessor::Instance().AddCorpse(corpse);
}

void Player::SpawnCorpseBones()
{
    if(ObjectAccessor::Instance().ConvertCorpseForPlayer(GetGUID()))
        SaveToDB(false);                                         // prevent loading as ghost without corpse
}

CorpsePtr& Player::GetCorpse() const
{
    return ObjectAccessor::Instance().GetCorpseForPlayerGUID(GetGUID());
}

void Player::DurabilityLossAll(double percent)
{
    for (uint16 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        DurabilityLoss(i,percent);
}

void Player::DurabilityLoss(uint8 equip_pos, double percent)
{
    if(!m_items[equip_pos])
        return;

    uint32 pDurability =  m_items[equip_pos]->GetUInt32Value(ITEM_FIELD_DURABILITY);

    if(!pDurability)
        return;

    uint32 pDurabilityLoss = (uint32)(pDurability*percent);

    if(pDurabilityLoss < 1 )
        pDurabilityLoss = 1;

    uint32 pNewDurability = pDurability - pDurabilityLoss;

    // we have durability 25% or 0 we should modify item stats
    // modify item stats _before_ Durability set to 0 to pass _ApplyItemMods internal check
    //        if ( pNewDurability == 0 || pNewDurability * 100 / pDurability < 25)
    if ( pNewDurability == 0 )
        _ApplyItemMods(m_items[equip_pos],equip_pos, false);

    m_items[equip_pos]->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);
    m_items[equip_pos]->SetState(ITEM_CHANGED, this);
}

void Player::DurabilityPointsLoss(uint8 equip_pos, uint32 points)
{
    if(!m_items[equip_pos])
        return;

    uint32 pDurability =  m_items[equip_pos]->GetUInt32Value(ITEM_FIELD_DURABILITY);

    if(!pDurability)
        return;

    uint32 pNewDurability = pDurability >= points ? pDurability - points : 0;

    // we have durability 25% or 0 we should modify item stats
    // modify item stats _before_ Durability set to 0 to pass _ApplyItemMods internal check
    //        if ( pNewDurability == 0 || pNewDurability * 100 / pDurability < 25)
    if ( pNewDurability == 0 )
        _ApplyItemMods(m_items[equip_pos],equip_pos, false);

    m_items[equip_pos]->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);
    m_items[equip_pos]->SetState(ITEM_CHANGED, this);
}

void Player::DurabilityRepairAll(bool cost, bool discount)
{
    for (uint16 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        DurabilityRepair(( (INVENTORY_SLOT_BAG_0 << 8) | i ),cost,discount);
}

void Player::DurabilityRepair(uint16 pos, bool cost, bool discount)
{
    Item* item = GetItemByPos(pos);

    if(!item)
        return;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    if(!maxDurability)
        return;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    // some simple repair formula depending on durability lost
    if(cost)
    {
        uint32 costs = maxDurability - curDurability;

        if(discount)
            costs = 9 * costs / 10;

        if (GetMoney() < costs)
        {
            DEBUG_LOG("You do not have enough money");
            return;
        }

        ModifyMoney( -int32(costs) );
    }

    item->SetUInt32Value(ITEM_FIELD_DURABILITY, maxDurability);
    item->SetState(ITEM_CHANGED, this);

    // reapply mods for total broken and repaired item if equipped
    if(IsEquipmentPos(pos) && !curDurability)
        _ApplyItemMods(item,pos & 255, true);
}

void Player::RepopAtGraveyard()
{
    WorldSafeLocsEntry const *ClosestGrave = objmgr.GetClosestGraveYard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam() );

    if(ClosestGrave)
    {
        // stop countdown until repop
        m_deathTimer = 0;

        TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, GetOrientation());

        if(isDead())                                        // not send if alive, because it used in TeleportTo()
        {
            WorldPacket data(SMSG_SH_POSITION, 4*4);        // show spirit healer position on minimap
            data << ClosestGrave->map_id;
            data << ClosestGrave->x;
            data << ClosestGrave->y;
            data << ClosestGrave->z;
            GetSession()->SendPacket(&data);
        }
        if(CorpsePtr corpse = GetCorpse())
            corpse->UpdateForPlayer(this,true);
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
        (*i)->Leave(GetGUID(),false,0);
    sLog.outDebug("Player: channels cleaned up!");
}

void Player::BroadcastPacketToFriendListers(WorldPacket *packet)
{
    Field *fields;
    Player *pfriend;

    QueryResult *result = sDatabase.Query("SELECT `guid` FROM `character_social` WHERE `flags` = 'FRIEND' AND `friend` = '%u'", GetGUIDLow());

    if(!result) return;

    uint32 team = GetTeam();
    uint32 security = GetSession()->GetSecurity();
    bool gmInWhoList         = sWorld.getConfig(CONFIG_GM_IN_WHO_LIST);
    bool allowTwoSideWhoList = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);

    do
    {
        fields = result->Fetch();

        pfriend = ObjectAccessor::Instance().FindPlayer(fields[0].GetUInt64());

        // PLAYER see his team only and PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
        // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
        if( pfriend && pfriend->IsInWorld() &&
            ( pfriend->GetSession()->GetSecurity() > 0 ||
            ( pfriend->GetTeam() == team || allowTwoSideWhoList ) &&
            (security == 0 || gmInWhoList && isVisibleFor(pfriend,false) )))
        {
            pfriend->GetSession()->SendPacket(packet);
        }

    }while( result->NextRow() );
    delete result;
}

void Player::UpdateDefense()
{
    if(UpdateSkill(SKILL_DEFENSE))
    {
        // update dependent from defense skill part BlockChanceWithoutMods = 5 + (GetDefenceSkillValue() - getLevel()*5)*0.04);
        UpdateBlockPercentage();
    }
}

void Player::ApplyDefenseBonusesMod(float value, bool apply)
{
    ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE, value * 0.04, apply);
    ApplyModFloatValue(PLAYER_PARRY_PERCENTAGE, value * 0.04, apply);
    ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE, value * 0.04, apply);
}

void Player::ApplyRatingMod(uint16 index, int32 value, bool apply)
{
    ApplyModUInt32Value(index, value, apply);

    float RatingCoeffecient = 0;
    float RatingChange = 0.0;

    //Global formulas for all skills based on player level
    uint32 level = getLevel();
    if (level < 10)
        RatingCoeffecient = 2.0 / 52.0;
    else if (level < 60)
        RatingCoeffecient = (level - 8.0) / 52.0;
    else if (level < 70)
        RatingCoeffecient = 82.0 / (262.0 - 3.0 * level);
    else RatingCoeffecient = (level + 12.0) / 52.0;

    switch (index)
    {
        case PLAYER_FIELD_MELEE_WEAPON_SKILL_RATING:
        case PLAYER_FIELD_OFFHAND_WEAPON_SKILL_RATING:
        case PLAYER_FIELD_RANGED_WEAPON_SKILL_RATING:
        {
            //Weapon skill: 2.5
            RatingChange = value/(2.5 * RatingCoeffecient);
            /*uint16  slot;
            switch (index)
            {
                case PLAYER_FIELD_MELEE_WEAPON_SKILL_RATING: slot = EQUIPMENT_SLOT_MAINHAND; break;
                case PLAYER_FIELD_OFFHAND_WEAPON_SKILL_RATING: slot = EQUIPMENT_SLOT_OFFHAND; break;
                case PLAYER_FIELD_RANGED_WEAPON_SKILL_RATING: slot = EQUIPMENT_SLOT_RANGED; break;
            }
            Item *item = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            uint32 skill = item && !item->IsBroken() && ((Player*)this)->IsUseEquipedWeapon()
                ? item->GetSkill() : SKILL_UNARMED;
            ModifySkillBonus(skill, (apply ? (int32)RatingChange: -(int32)RatingChange));*/
        }
        break;
        case PLAYER_FIELD_DEFENCE_RATING:
            //Defense: 1.5
            RatingChange = value/(1.5 * RatingCoeffecient);
            ModifySkillBonus(SKILL_DEFENSE,(apply ? (int32)RatingChange: -(int32)RatingChange));
            ApplyDefenseBonusesMod(RatingChange, apply);
            break;
        case PLAYER_FIELD_DODGE_RATING:
            //Dodge: 12
            RatingChange = value/(12.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_DODGE_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_PARRY_RATING:
            //Dodge: 12
            RatingChange = value/(20.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_PARRY_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_BLOCK_RATING:
            //Block: 5
            RatingChange = value/(5.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_BLOCK_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_MELEE_HIT_RATING:
            //Hit (melee): 10
            RatingChange = value/(10.0 * RatingCoeffecient);
            m_modHitChance += apply?int32(RatingChange):-int32(RatingChange);
            break;
        case PLAYER_FIELD_RANGED_HIT_RATING:
            //Hit (melee): 10
            RatingChange = value/(10.0 * RatingCoeffecient);
            m_modHitChance += apply?int32(RatingChange):-int32(RatingChange);
            break;
        case PLAYER_FIELD_SPELL_HIT_RATING:
            //Hit (spells): 8
            RatingChange = value/(8.0 * RatingCoeffecient);
            m_modSpellHitChance += apply?int32(RatingChange):-int32(RatingChange);
            break;
        case PLAYER_FIELD_MELEE_CRIT_RATING:
            //Crit (melee and spells): 14
            RatingChange = value/(14.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_RANGED_CRIT_RATING:
            //Crit (melee and spells): 14
            RatingChange = value/(14.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_SPELL_CRIT_RATING:
            //Crit (melee and spells): 14
            RatingChange = value/(14.0 * RatingCoeffecient);
            ApplyModFloatValue(PLAYER_HOLY_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            ApplyModFloatValue(PLAYER_FIRE_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            ApplyModFloatValue(PLAYER_NATURE_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            ApplyModFloatValue(PLAYER_FROST_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            ApplyModFloatValue(PLAYER_SHADOW_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            ApplyModFloatValue(PLAYER_ARCANE_SPELL_CRIT_PERCENTAGE,RatingChange,apply);
            break;
        case PLAYER_FIELD_MELEE_HASTE_RATING:
            //Haste: 6.67
            RatingChange = value/(6.66667f * RatingCoeffecient);
            if(RatingChange >= 0)
            {
                ApplyAttackTimePercentMod(BASE_ATTACK,RatingChange,apply);
                ApplyAttackTimePercentMod(OFF_ATTACK,RatingChange,apply);
            }
            else
            {
                ApplyAttackTimePercentMod(BASE_ATTACK,-RatingChange,!apply);
                ApplyAttackTimePercentMod(OFF_ATTACK,-RatingChange,!apply);
            }
            break;
        case PLAYER_FIELD_RANGED_HASTE_RATING:
            //Haste: 6.67
            RatingChange = value/(6.66667f * RatingCoeffecient);
            if(RatingChange >= 0)
                ApplyAttackTimePercentMod(RANGED_ATTACK, RatingChange, apply);
            else
                ApplyAttackTimePercentMod(RANGED_ATTACK, -RatingChange, !apply);
            break;
        case PLAYER_FIELD_SPELL_HASTE_RATING:
            //Haste: 6.67
            RatingChange = value/(6.66667f * RatingCoeffecient);
            ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED,RatingChange,!apply);
            m_modCastSpeedPct += apply?int32(RatingChange):-int32(RatingChange);
            break;
        case PLAYER_FIELD_HIT_RATING:
            //Hit (melee): 10
            RatingChange = value/(10.0 * RatingCoeffecient);
            ApplyModUInt32Value(PLAYER_FIELD_MELEE_HIT_RATING, value, apply);
            ApplyModUInt32Value(PLAYER_FIELD_RANGED_HIT_RATING, value, apply);
            m_modHitChance += apply?int32(RatingChange):-int32(RatingChange);
            break;
        case PLAYER_FIELD_CRIT_RATING:
            //Crit (melee and spells): 14
            RatingChange = value/(14.0 * RatingCoeffecient);
            ApplyModUInt32Value(PLAYER_FIELD_MELEE_CRIT_RATING, value, apply);
            ApplyModUInt32Value(PLAYER_FIELD_RANGED_CRIT_RATING, value, apply);
            ApplyModFloatValue(PLAYER_CRIT_PERCENTAGE,RatingChange,apply);
            break;
        /*
        case PLAYER_FIELD_HIT_AVOIDANCE_RATING:
            break;
        case PLAYER_FIELD_CRIT_AVOIDANCE_RATING:
            break;
        */
        case PLAYER_FIELD_RESILIENCE_RATING:
            //Resilience: 25
            RatingChange = value/(25.0 * RatingCoeffecient);
            ApplyModUInt32Value(PLAYER_FIELD_UNK4_RATING, value, apply);
            ApplyModUInt32Value(PLAYER_FIELD_UNK5_RATING, value, apply);
            m_modResilience += apply?RatingChange:-RatingChange;
            break;
    }
}

void Player::UpdateBlockPercentage()
{
    AuraList& mModBlockPercent = GetAurasByType(SPELL_AURA_MOD_BLOCK_PERCENT);

    if (HasAuraType(SPELL_AURA_MOD_BLOCK_PERCENT))
    {
        for(AuraList::iterator i = mModBlockPercent.begin(); i != mModBlockPercent.end(); ++i)
            (*i)->ApplyModifier(false);
    }

    float chance = 5 - (getLevel()*5 - GetPureDefenceSkillValue()) * 0.04;
    chance = chance < 0 ? 0 : chance;

    SetFloatValue(PLAYER_BLOCK_PERCENTAGE, chance);

    if (HasAuraType(SPELL_AURA_MOD_BLOCK_PERCENT))
    {
        for(AuraList::iterator i = mModBlockPercent.begin(); i != mModBlockPercent.end(); ++i)
            (*i)->ApplyModifier(true);
    }
}

//skill+1, checking for max value
bool Player::UpdateSkill(uint32 skill_id)
{
    if(!skill_id) return false;
    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skill_id) break;
    if(i>=PLAYER_MAX_SKILLS) return false;

    uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
    uint32 value = SKILL_VALUE(data);
    uint32 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max)) return false;

    if (value*512 < max*urand(0,512))
    {
        SetUInt32Value(PLAYER_SKILL(i)+1,data+1);
        return true;
    }

    return false;
}

#define HalfChanceSkillSteps 75

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if ( SkillValue >= GrayLevel )
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_GREY)*10;
    if ( SkillValue >= GreenLevel )
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_GREEN)*10;
    if ( SkillValue >= YellowLevel )
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_YELLOW)*10;
    return sWorld.getConfig(CONFIG_SKILL_CHANCE_ORANGE)*10;
}

bool Player::UpdateCraftSkill(uint32 spellid)
{
    sLog.outDebug("UpdateCraftSkill spellid %d", spellid);

    SkillLineAbilityEntry const *pAbility = sSkillLineAbilityStore.LookupEntry(spellid);
    if ( !pAbility ) return false;

    uint32 SkillId = pAbility->skillId;
    if ( !SkillId ) return false;

    uint32 SkillValue = GetPureSkillValue(SkillId);

    return UpdateSkillPro(pAbility->skillId, SkillGainChance(SkillValue,
        pAbility->max_value,
        (pAbility->max_value + pAbility->min_value)/2,
        pAbility->min_value));
}

bool Player::UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator )
{
    sLog.outDebug("UpdateGatherSkill(SkillId %d SkillLevel %d RedLevel %d)", SkillId, SkillValue, RedLevel);

    // For skinning and Mining chance decrease with level. 1-74 - no decrease, 75-149 - 2 times, 225-299 - 8 times
    switch (SkillId)
    {
        case SKILL_HERBALISM:
        case SKILL_LOCKPICKING:
            return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator);
        case SKILL_SKINNING:
        case SKILL_MINING:
            return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator) >> (SkillValue/HalfChanceSkillSteps) );
    }
    return false;
}

bool Player::UpdateFishingSkill()
{
    sLog.outDebug("UpdateFishingSkill");

    uint32 SkillValue = GetPureSkillValue(SKILL_FISHING);

    int32 chance = SkillValue < 75 ? 100 : 2500/(SkillValue-50);

    return UpdateSkillPro(SKILL_FISHING,chance*10);
}

bool Player::UpdateSkillPro(uint16 SkillId, int32 Chance)
{
    sLog.outDebug("UpdateSkillPro(SkillId %d, Chance %3.1f%%)", SkillId, Chance/10.0);
    if ( !SkillId )
        return false;

    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if ( SKILL_VALUE(GetUInt32Value(PLAYER_SKILL(i))) == SkillId ) break;
    if ( i >= PLAYER_MAX_SKILLS )
        return false;

    uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
    uint16 SkillValue = SKILL_VALUE(data);
    uint16 MaxValue   = SKILL_MAX(data);

    if ( !MaxValue || !SkillValue || SkillValue >= MaxValue )
        return false;

    int32 Roll = irand(1,1000);

    if ( Roll <= Chance )
    {
        SetUInt32Value(PLAYER_SKILL(i)+1,MAKE_SKILL_VALUE(SkillValue+1,MaxValue));
        sLog.outDebug("Player::UpdateSkillPro Chance=%3.1f%% taken", Chance/10.0);
        return true;
    }

    sLog.outDebug("Player::UpdateSkillPro Chance=%3.1f%% missed", Chance/10.0);
    return false;
}

void Player::UpdateWeaponSkill (WeaponAttackType attType)
{
    // no skill gain in pvp
    Unit *pVictim = getVictim();
    if(pVictim && pVictim->GetTypeId() == TYPEID_PLAYER)
        return;

    switch(attType)
    {
        case BASE_ATTACK:
        {
            Item *tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

            if (!tmpitem || tmpitem->IsBroken() || !IsUseEquipedWeapon())
                UpdateSkill(SKILL_UNARMED);
            else if(tmpitem->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
                UpdateSkill(tmpitem->GetSkill());

        };break;
        case OFF_ATTACK:
        {
            Item *tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

            if (tmpitem && tmpitem->GetProto()->Class == ITEM_CLASS_WEAPON && !tmpitem->IsBroken() && IsUseEquipedWeapon())
                UpdateSkill(tmpitem->GetSkill());
        };break;
        case RANGED_ATTACK:
        {
            Item* tmpitem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

            if (tmpitem && tmpitem->GetProto()->Class == ITEM_CLASS_WEAPON && !tmpitem->IsBroken() && IsUseEquipedWeapon())
                UpdateSkill(tmpitem->GetSkill());
        };break;
    }
}

void Player::UpdateCombatSkills(Unit *pVictim, WeaponAttackType attType, MeleeHitOutcome outcome, bool defence)
{
    switch(outcome)
    {
        case MELEE_HIT_CRIT:
            return;
        case MELEE_HIT_DODGE:
            return;
        case MELEE_HIT_PARRY:
            return;
        case MELEE_HIT_BLOCK:
            return;

        default:
            break;
    }

    uint32 plevel = getLevel();                             // if defence than pVictim == attacker
    uint32 greylevel = MaNGOS::XP::GetGrayLevel(plevel);
    uint32 moblevel = pVictim->getLevel();
    if(moblevel < greylevel)
        return;

    if (moblevel > plevel + 5)
        moblevel = plevel + 5;

    uint32 lvldif = moblevel - greylevel;
    if(lvldif < 3)
        lvldif = 3;

    uint32 skilldif = 5 * plevel - (defence ? GetPureDefenceSkillValue() : GetPureWeaponSkillValue(attType));
    if(skilldif <= 0)
        return;

    float chance = float(3 * lvldif * skilldif) / plevel;
    if(!defence)
    {
        if(getClass() == CLASS_WARRIOR || getClass() == CLASS_ROGUE)
            chance *= 0.1 * GetStat(STAT_INTELLECT);
    }

    chance = chance < 1.0 ? 1.0 : chance;                   //minimum chance to increase skill is 1%

    if(roll_chance_f(chance))
    {
        if(defence)
            UpdateDefense();
        else
            UpdateWeaponSkill(attType);
    }
    else
        return;
}

void Player::ModifySkillBonus(uint32 skillid,int32 val)
{
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skillid)
    {
        SetUInt32Value(PLAYER_SKILL(i)+2,uint16(int16(GetUInt32Value(PLAYER_SKILL(i)+2))+val));
        return;
    }
}

void Player::UpdateMaxSkills()
{
    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();

    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
        if (GetUInt32Value(PLAYER_SKILL(i)))
    {
        uint32 pskill = GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF;
        if( IsProfessionSkill(pskill) || pskill == SKILL_RIDING )
            continue;
        uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);
        uint32 max = data>>16;

        // update only level dependent max skill values
        if(max!=1 && max != maxconfskill)
        {
            uint32 max_Skill = data%0x10000+GetMaxSkillValueForLevel()*0x10000;
            SetUInt32Value(PLAYER_SKILL(i)+1,max_Skill);
        }
    }
}

void Player::UpdateSkillsToMaxSkillsForLevel()
{
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
        if (GetUInt32Value(PLAYER_SKILL(i)))
    {
        uint32 pskill = GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF;
        if( IsProfessionSkill(pskill) || pskill == SKILL_RIDING )
            continue;
        uint32 data = GetUInt32Value(PLAYER_SKILL(i)+1);

        uint32 max = data>>16;

        if(max > 1)
        {
            uint32 new_data = max * 0x10000 + max;
            SetUInt32Value(PLAYER_SKILL(i)+1,new_data);
        }
        if(pskill == SKILL_DEFENSE)
        {
            UpdateBlockPercentage();
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
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == id) break;

    if(i<PLAYER_MAX_SKILLS)                                 //has skill
    {
        if(currVal)
            SetUInt32Value(PLAYER_SKILL(i)+1,currVal+maxVal*0x10000);
        else                                                //remove
        {
            SetUInt64Value(PLAYER_SKILL(i),0);
            SetUInt32Value(PLAYER_SKILL(i)+2,0);
            // remove spells that depend on this skill when removing the skill
            for (PlayerSpellMap::const_iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
            {
                next++;
                if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                SkillLineAbilityEntry const *ability = sSkillLineAbilityStore.LookupEntry(itr->first);
                if (ability && ability->skillId == id)
                    removeSpell(itr->first);
            }
        }
    }else if(currVal)                                       //add
    {

        for (i=0; i < PLAYER_MAX_SKILLS; i++)
            if (!GetUInt32Value(PLAYER_SKILL(i)))
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(id);
            if(!pSkill)
            {
                sLog.outError("Skill not found in SkillLineStore: skill #%u", id);
                return;
            }
            // enable unlearn button for professions only
            if (pSkill->categoryId == 11)
                SetUInt32Value(PLAYER_SKILL(i), id | (1 << 16));
            else
                SetUInt32Value(PLAYER_SKILL(i),id);
            SetUInt32Value(PLAYER_SKILL(i)+1,maxVal*0x10000+currVal);
            // apply skill bonuses
            SetUInt32Value(PLAYER_SKILL(i)+2,0);
            AuraList& mModSkill = GetAurasByType(SPELL_AURA_MOD_SKILL);
            for(AuraList::iterator i = mModSkill.begin(); i != mModSkill.end(); ++i)
                if ((*i)->GetModifier()->m_miscvalue == id)
                    (*i)->ApplyModifier(true);
            AuraList& mModSkillTalent = GetAurasByType(SPELL_AURA_MOD_SKILL_TALENT);
            for(AuraList::iterator i = mModSkillTalent.begin(); i != mModSkillTalent.end(); ++i)
                if ((*i)->GetModifier()->m_miscvalue == id)
                    (*i)->ApplyModifier(true);
            return;
        }
    }
}

bool Player::HasSkill(uint32 skill) const
{
    if(!skill)return false;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skill)
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
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skill)
        {
            int16 result = SKILL_VALUE(GetUInt32Value(PLAYER_SKILL(i)+1))+int16(GetUInt32Value(PLAYER_SKILL(i)+2));
            return result < 0 ? 0 : result;
        }
    }
    return 0;
}

uint16 Player::GetMaxSkillValue(uint32 skill) const
{
    if(!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skill)
        {
            return SKILL_MAX(GetUInt32Value(PLAYER_SKILL(i)+1));
        }
    }
    return 0;
}

uint16 Player::GetPureSkillValue(uint32 skill) const
{
    if(!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL(i)) & 0x0000FFFF) == skill)
        {
            return SKILL_VALUE(GetUInt32Value(PLAYER_SKILL(i)+1));
        }
    }
    return 0;
}

void Player::SendInitialActionButtons()
{
    sLog.outDetail( "Initializing Action Buttons for '%u'", GetGUIDLow() );

    WorldPacket data(SMSG_ACTION_BUTTONS, (120*4));
    for(int button = 0; button < 120; ++button)
    {
        ActionButtonList::const_iterator itr = m_actionButtons.find(button);
        if(itr != m_actionButtons.end() && itr->second.uState != ACTIONBUTTON_DELETED)
        {
            data << uint16(itr->second.action);
            data << uint8(itr->second.misc);
            data << uint8(itr->second.type);
        }
        else
        {
            data << uint32(0);
        }
    }

    GetSession()->SendPacket( &data );
    sLog.outDetail( "Action Buttons for '%u' Initialized", GetGUIDLow() );
}

void Player::addActionButton(const uint8 button, const uint16 action, const uint8 type, const uint8 misc)
{
    if(button >= 120)
    {
        sLog.outError( "Action %u not added into button %u for player %s: button must be < 120", action, button, GetName() );
        return;
    }

    // check cheating with adding non-known spells to action bar
    if(type==ACTION_BUTTON_SPELL)
    {
        if(!sSpellStore.LookupEntry(action))
        {
            sLog.outError( "Action %u not added into button %u for player %s: spell not exist", action, button, GetName() );
            return;
        }

        if(!HasSpell(action))
        {
            sLog.outError( "Action %u not added into button %u for player %s: player don't known this spell", action, button, GetName() );
            return;
        }
    }

    if (m_actionButtons.find(button)==m_actionButtons.end())
    {                                                       // just add new button
        m_actionButtons[button] = ActionButton(action,type,misc);
    } else
    {                                                       // change state of current button
        ActionButtonUpdateState uState = m_actionButtons[button].uState;
        m_actionButtons[button] = ActionButton(action,type,misc);
        if (uState != ACTIONBUTTON_NEW) m_actionButtons[button].uState = ACTIONBUTTON_CHANGED;
    };

    sLog.outDetail( "Player '%u' Added Action '%u' to Button '%u'", GetGUIDLow(), action, button );
}

void Player::removeActionButton(uint8 button)
{
    m_actionButtons[button].uState = ACTIONBUTTON_DELETED;
    sLog.outDetail( "Action Button '%u' Removed from Player '%u'", button, GetGUIDLow() );
}

void Player::SetDontMove(bool dontMove)
{
    m_dontMove = dontMove;
}

bool Player::SetPosition(float x, float y, float z, float orientation)
{
    Map *m = MapManager::Instance().GetMap(GetMapId(), this);

    const float old_x = GetPositionX();
    const float old_y = GetPositionY();
    const float old_r = GetOrientation();

    if( old_x != x || old_y != y || old_r != orientation)
    {
        m->PlayerRelocation(this, x, y, z, orientation);

        // remove at movement non-move stealth aura
        if(HasFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_FLAG_STEALTH))
            RemoveAurasDueToSpell(20580);

        // remove death simulation at move
        if(hasUnitState(UNIT_STAT_DIED))
            RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);
    }

    // reread after Map::Relocation
    m = MapManager::Instance().GetMap(GetMapId(), this);
    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();

    float water_z = m->GetWaterLevel(x,y);
    uint8 flag1 = m->GetTerrainType(x,y);

    //!Underwater check
    if ((z < (water_z - 2)) && (flag1 & 0x01))
        m_isunderwater|= 0x01;
    else if (z > (water_z - 2))
        m_isunderwater&= 0x7A;

    //!in lava check
    if ((z < (water_z - 0)) && (flag1 & 0x02))
        m_isunderwater|= 0x80;

    CheckExploreSystem();

    return true;
}

void Player::SetRecallPosition(uint32 map, float x, float y, float z, float o)
{
    m_recallMap = map;
    m_recallX = x;
    m_recallY = y;
    m_recallZ = z;
    m_recallO = o;
}

void Player::SendMessageToSet(WorldPacket *data, bool self)
{
    MapManager::Instance().GetMap(GetMapId(), this)->MessageBoardcast(this, data, self);
}

void Player::SendMessageToOwnTeamSet(WorldPacket *data, bool self)
{
    MapManager::Instance().GetMap(GetMapId(), this)->MessageBoardcast(this, data, self,true);
}

void Player::SendDirectMessage(WorldPacket *data)
{
    GetSession()->SendPacket(data);
}

void Player::CheckExploreSystem()
{
    if (!isAlive())
        return;

    if (isInFlight())
        return;

    uint16 areaFlag=MapManager::Instance().GetMap(GetMapId(), this)->GetAreaFlag(GetPositionX(),GetPositionY());
    if(areaFlag==0xffff)return;
    int offset = areaFlag / 32;

    if(offset >= 64)
    {
        sLog.outError("ERROR: Wrong area flag %u in map data for (X: %f Y: %f) point to field PLAYER_EXPLORED_ZONES_1 + %u ( %u must be < 64 ).",areaFlag,GetPositionX(),GetPositionY(),offset,offset);
        return;
    }

    uint32 val = (uint32)(1 << (areaFlag % 32));
    uint32 currFields = GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    if( !(currFields & val) )
    {
        SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

        AreaTableEntry const *p = GetAreaEntryByAreaFlag(areaFlag);
        if(!p)
        {
            sLog.outError("PLAYER: Player %u discovered unknown area (x: %f y: %f map: %u", GetGUIDLow(), GetPositionX(),GetPositionY(),GetMapId());
        }
        else if(p->area_level > 0)
        {
            uint32 area = p->ID;
            if (getLevel() >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                SendExplorationExperience(area,0);
            }
            else
            {
                uint32 XP = uint32(p->area_level*10*sWorld.getRate(RATE_XP_EXPLORE));
                GiveXP( XP, NULL );
                SendExplorationExperience(area,XP);
            }
            sLog.outDetail("PLAYER: Player %u discovered a new area: %u", GetGUIDLow(), area);
        }
    }
}

uint32 Player::TeamForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if(!rEntry)
    {
        sLog.outError("Race %u not found in DBC: wrong DBC files?",uint32(race));
        return ALLIANCE;
    }

    switch(rEntry->TeamID)
    {
        case 7: return ALLIANCE;
        case 1: return HORDE;
    }

    sLog.outError("Race %u have wrong team id in DBC: wrong DBC files?",uint32(race),rEntry->TeamID);
    return ALLIANCE;
}

uint32 Player::getFactionForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if(!rEntry)
    {
        sLog.outError("Race %u not found in DBC: wrong DBC files?",uint32(race));
        return 0;
    }

    return rEntry->FactionID;
}

void Player::setFactionForRace(uint8 race)
{
    m_team = TeamForRace(race);
    setFaction( getFactionForRace(race) );
}

void Player::UpdateReputation() const
{
    sLog.outDebug( "WORLD: Player::UpdateReputation" );

    for(FactionsList::const_iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        SendSetFactionStanding(&*itr);
    }
}

void Player::SendSetFactionStanding(const Faction* faction) const
{
    if(faction->Flags & FACTION_FLAG_VISIBLE)               //If faction is visible then update it
    {
        WorldPacket data(SMSG_SET_FACTION_STANDING, (12));  // last check 2.0.10
        data << (uint32) 1;
        data << (uint32) faction->ReputationListID;
        data << (uint32) faction->Standing;
        GetSession()->SendPacket(&data);
    }
    /*
    Packet SMSG.SET_FACTION_STANDING (292), len: 54
    0000: 24 01 06 00 00 00 13 00 00 00 8f 21 00 00 0b 00 : $..........!....
    0010: 00 00 91 0c 00 00 31 00 00 00 91 0c 00 00 14 00 : ......1.........
    0020: 00 00 22 12 00 00 15 00 00 00 67 45 00 00 12 00 : ..".......gE....
    0030: 00 00 a4 0c 00 00 -- -- -- -- -- -- -- -- -- -- : ......
    */
    //changed in 2.0.x:
    /*uint32 count;
    for (uint32 i = 0; i < count; i++)
    {
        uint32 ReputationListID;
        uint32 Standing;
    }*/
}

void Player::SendInitialReputations()
{
    WorldPacket data(SMSG_INITIALIZE_FACTIONS, (4+128*5));
    data << uint32 (0x00000080);
    for(uint32 a=0; a<128; a++)
    {
        FactionsList::iterator itr = FindReputationListIdInFactionList(a);

        if(itr != m_factions.end())
        {
            data << uint8  (itr->Flags);
            data << uint32 (itr->Standing);
        }
        else
        {
            data << uint8  (0x00);
            data << uint32 (0x00000000);
        }
    }
    GetSession()->SendPacket(&data);
}

void Player::SetFactionAtWar(uint32 repListID, bool atWar)
{
    for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if(itr->ReputationListID == repListID)
        {
            if(((itr->Flags & FACTION_FLAG_AT_WAR) != 0) == atWar)               // already set
                break;

            if( atWar )
                itr->Flags |= FACTION_FLAG_AT_WAR;
            else
                itr->Flags &= ~FACTION_FLAG_AT_WAR;
            if(itr->uState != FACTION_NEW) itr->uState = FACTION_CHANGED;
            break;
        }
    }
}

void Player::SetFactionInactive(uint32 repListID, bool inactive)
{
    for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if(itr->ReputationListID == repListID)
        {
            if(((itr->Flags & FACTION_FLAG_INACTIVE) != 0) == inactive)         // already set
                break;

            if(inactive)
                itr->Flags |= FACTION_FLAG_INACTIVE;
            else
                itr->Flags &= ~FACTION_FLAG_INACTIVE;

            if(itr->uState != FACTION_NEW)
                itr->uState = FACTION_CHANGED;

            break;
        }
    }
}

FactionsList::iterator Player::FindReputationListIdInFactionList(uint32 repListId)
{
    for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if(itr->ReputationListID == repListId) return itr;
    }
    return m_factions.end();
}

void Player::SendSetFactionVisible(const Faction* faction) const
{
    /*
        // this code must be use at Gossip/NPC handler
        // we must check if faction already visible/in list?
        uint32 faction = unit->getFactionTemplateEntry()->faction;
        for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
        {
            if(itr->ID == faction)
            {
                //if(!_player->FactionIsInTheList(itr->ReputationListID)
                SendSetFactionVisible(&*itr);
                break;
            }
        }
    */
    // make faction visible in reputation list, on blizz it use when talking with NPC of new faction
    // we must check if faction already visible?
    WorldPacket data(SMSG_SET_FACTION_VISIBLE, 4);
    data << faction->ReputationListID;
    GetSession()->SendPacket(&data);
}

void Player::SetInitialFactions()
{
    Faction newFaction;
    FactionEntry const *factionEntry = NULL;

    for(unsigned int i = 1; i < sFactionStore.GetNumRows(); i++)
    {
        factionEntry = sFactionStore.LookupEntry(i);

        if( factionEntry && (factionEntry->reputationListID >= 0))
        {
            newFaction.ID = factionEntry->ID;
            newFaction.ReputationListID = factionEntry->reputationListID;
            newFaction.Standing = 0;
            newFaction.Flags = 0x0;
            newFaction.uState = FACTION_NEW;

            // show(1) and disable AtWar button(16) of own team factions
            if( GetTeam() == factionEntry->team )
                newFaction.Flags = FACTION_FLAG_OWN_TEAM | FACTION_FLAG_VISIBLE;

            //If the faction is Hostile or Hated  of my one we are at war!
            if(GetBaseReputationRank(factionEntry) <= REP_HOSTILE)
                newFaction.Flags |= FACTION_FLAG_AT_WAR;

            m_factions.push_back(newFaction);
        }
    }
}

int32 Player::GetBaseReputation(const FactionEntry *factionEntry) const
{
    if (!factionEntry)
        return 0;

    uint32 Race = getRace();
    for (int i=0; i < 4; i++)
    {
        if ( factionEntry->BaseRepMask[i] & (1 << (Race-1)))
            return factionEntry->BaseRepValue[i];
    }
    sLog.outError("Player::GetBaseReputation: can't get base reputation of %s for faction id %d", GetName(), factionEntry->ID);
    return 0;
}

int32 Player::GetReputation(uint32 FactionTemplateId) const
{
    FactionTemplateEntry const *factionTemplateEntry = sFactionTemplateStore.LookupEntry(FactionTemplateId);

    if(!factionTemplateEntry)
    {
        sLog.outError("Player::GetReputation: Can't get reputation of %s for unknown faction (faction template id) #%u.",GetName(), FactionTemplateId);
        return 0;
    }
    FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->faction);

    return GetReputation(factionEntry);
}

int32 Player::GetReputation(const FactionEntry *factionEntry) const
{
    // Faction without recorded reputation. Just ignore.
    if(!factionEntry)
        return 0;

    for(FactionsList::const_iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if(int32(itr->ReputationListID) == factionEntry->reputationListID)
            return GetBaseReputation(factionEntry) + itr->Standing;
    }
    return 0;
}

ReputationRank Player::GetReputationRank(uint32 faction) const
{
    FactionEntry const*factionEntry = sFactionStore.LookupEntry(faction);
    if(!factionEntry)
        return MIN_REPUTATION_RANK;

    return GetReputationRank(factionEntry);
}

ReputationRank Player::ReputationToRank(int32 standing) const
{
    int32 Limit = Reputation_Cap + 1;
    for (int i = MAX_REPUTATION_RANK-1; i >= MIN_REPUTATION_RANK; --i)
    {
        Limit -= ReputationRank_Length[i];
        if (standing >= Limit )
            return ReputationRank(i);
    }
    return MIN_REPUTATION_RANK;
}

ReputationRank Player::GetReputationRank(const FactionEntry *factionEntry) const
{
    int32 Reputation = GetReputation(factionEntry);
    return ReputationToRank(Reputation);
}

ReputationRank Player::GetBaseReputationRank(const FactionEntry *factionEntry) const
{
    int32 Reputation = GetBaseReputation(factionEntry);
    return ReputationToRank(Reputation);
}

bool Player::ModifyFactionReputation(uint32 FactionTemplateId, int32 DeltaReputation)
{
    FactionTemplateEntry const*factionTemplateEntry = sFactionTemplateStore.LookupEntry(FactionTemplateId);

    if(!factionTemplateEntry)
    {
        sLog.outError("Player::ModifyFactionReputation: Can't update reputation of %s for unknown faction (faction template id) #%u.", GetName(), FactionTemplateId);
        return false;
    }

    FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->faction);

    // Faction without recorded reputation. Just ignore.
    if(!factionEntry)
        return false;

    return ModifyFactionReputation(factionEntry, DeltaReputation);
}

bool Player::ModifyFactionReputation(FactionEntry const* factionEntry, int32 standing)
{
    for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if(int32(itr->ReputationListID) == factionEntry->reputationListID)
        {
            int32 BaseRep = GetBaseReputation(factionEntry);
            int32 new_rep = BaseRep + itr->Standing + standing;

            if (new_rep > Reputation_Cap)
                new_rep = Reputation_Cap;
            else
            if (new_rep < Reputation_Bottom)
                new_rep = Reputation_Bottom;

            itr->Standing = new_rep - BaseRep;
            itr->Flags |= FACTION_FLAG_VISIBLE;
            if(itr->uState != FACTION_NEW) itr->uState = FACTION_CHANGED;

            SendSetFactionStanding(&*itr);
            return true;
        }
    }
    return false;
}

//Calculate total reputation percent player gain with quest/creature level
int32 Player::CalculateReputationGain(uint32 creatureOrQuestLevel, int32 rep) const
{
    int32 Factor;
    int32 dif = int32(getLevel()) - creatureOrQuestLevel;

    // This part is before_2.01_like
    if (dif <= 5)
        Factor = 5;                                         // 100%
    else if (dif >= 10)
        Factor = 1;                                         // 20%
    else
        Factor = (10-dif);                                  // 20%...100% with step 20%

    int32 percent = Factor*20;

    if(rep > 0)
        percent += m_AuraModifiers[SPELL_AURA_MOD_REPUTATION_GAIN];
    else
        percent -= m_AuraModifiers[SPELL_AURA_MOD_REPUTATION_GAIN];

    if(percent <=0)
        return 0;

    // Uncomment the next line to be 2.01_like or maybe not (see Wiki)
    // percent = 100 + m_AuraModifiers[SPELL_AURA_MOD_REPUTATION_GAIN];
    return rep*percent/100;
}

//Calculates how many reputation points player gains in wich victim's enemy factions
void Player::CalculateReputation(Unit *pVictim)
{
    if(!pVictim || pVictim->GetTypeId() == TYPEID_PLAYER)
        return;

    ReputationOnKillEntry const* Rep = objmgr.GetReputationOnKilEntry(pVictim->GetEntry());

    if(!Rep)
        return;

    if(Rep->repfaction1 && (!Rep->team_dependent || GetTeam()==ALLIANCE))
    {
        int32 donerep1 = CalculateReputationGain(pVictim->getLevel(),Rep->repvalue1);
        FactionEntry const *factionEntry1 = sFactionStore.LookupEntry(Rep->repfaction1);
        uint32 current_reputation_rank1 = GetReputationRank(factionEntry1);
        if(factionEntry1 && current_reputation_rank1 <= Rep->reputration_max_cap1)
            ModifyFactionReputation(factionEntry1, donerep1);

        // Wiki: Team factions value divided by 2
        if(Rep->is_teamaward1 != 0)
        {
            FactionEntry const *team1_factionEntry = sFactionStore.LookupEntry(factionEntry1->team);
            if(team1_factionEntry)
                ModifyFactionReputation(team1_factionEntry, donerep1 / 2);
        }
    }

    if(Rep->repfaction2 && (!Rep->team_dependent || GetTeam()==HORDE))
    {
        int32 donerep2 = CalculateReputationGain(pVictim->getLevel(),Rep->repvalue2);
        FactionEntry const *factionEntry2 = sFactionStore.LookupEntry(Rep->repfaction2);
        uint32 current_reputation_rank2 = GetReputationRank(factionEntry2);
        if(factionEntry2 && current_reputation_rank2 <= Rep->reputration_max_cap2)
            ModifyFactionReputation(factionEntry2, donerep2);

        // Wiki: Team factions value divided by 2
        if(Rep->is_teamaward2 != 0)
        {
            FactionEntry const *team2_factionEntry = sFactionStore.LookupEntry(factionEntry2->team);
            if(team2_factionEntry)
                ModifyFactionReputation(team2_factionEntry, donerep2 / 2);
        }
    }
}

//Calculate how many reputation points player gain with the quest
void Player::CalculateReputation(Quest *pQuest, uint64 guid)
{
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
    if( !pCreature )
        return;

    // quest reputation reward/losts
    for(int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        if(pQuest->RewRepFaction[i] && pQuest->RewRepValue[i] )
        {
            int32 rep = CalculateReputationGain(pQuest->GetQuestLevel(),pQuest->RewRepValue[i]);
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->RewRepFaction[i]);
            if(factionEntry)
                ModifyFactionReputation(factionEntry, rep);
        }
    }

    // TODO: implement reputation spillover
}

void Player::UpdateArenaFields(void)
{
    /* arena calcs go here */
}

void Player::UpdateHonorFields()
{
    uint32 today = uint32(time(NULL) / DAY) * DAY;
    uint32 yesterday = today - DAY;

    QueryResult *result = sDatabase.Query("SELECT sum(`honor`) FROM `character_kill` WHERE `guid`='%u' AND `date`<'%u'", GUID_LOPART(GetGUID()), today);
    if(result && result->Fetch()[0].GetFloat() != 0)
    {
        float honor=0.0, honor_yesterday=0.0;
        uint32 kills_yesterday=0;

        honor = result->Fetch()[0].GetFloat();
        delete result;
        result = sDatabase.Query("SELECT sum(`honor`),count(`honor`) FROM `character_kill` WHERE `guid`='%u' AND `date`<'%u' AND `date`>='%u'", GUID_LOPART(GetGUID()), today, yesterday);
        if(result)
        {
            honor_yesterday = result->Fetch()[0].GetFloat();
            kills_yesterday = result->Fetch()[1].GetUInt32();
            delete result;
        }

        SetHonorPoints(GetHonorPoints()+uint32(honor));
        SetUInt32Value(PLAYER_FIELD_HONOR_TODAY, 0);
        SetUInt32Value(PLAYER_FIELD_HONOR_YESTERDAY, (uint32)(honor_yesterday*10));
        SetUInt32Value(PLAYER_FIELD_KILLS, (kills_yesterday<<16));

        sDatabase.Query("DELETE FROM `character_kill` WHERE `date`<'%u' AND `guid`='%u'", today,GUID_LOPART(GetGUID()));
    }
}

//How much honor Player gains from uVictim
void Player::CalculateHonor(Unit *uVictim)
{
    if(!uVictim || uVictim->GetTypeId() == TYPEID_UNIT)
        return;
    if(uVictim->GetAura(2479, 0))
        return;

    UpdateHonorFields();                                    // to prevent CalcluateHonor() on a new day before old honor was UpdateHonorFields()

    float honor = ((float)urand(1,80))/10;                  // honor between: 0.1 - 8.0
    float approx_honor = honor * (((float)urand(8,12))/10); // approx honor: 80% - 120% of real honor
    sDatabase.Execute("INSERT INTO `character_kill` (`guid`,`creature_template`,`honor`,`date`) VALUES (%u, %u, %f, %u)", GUID_LOPART(GetGUID()), uVictim->GetEntry(), honor, time(0));

    ApplyModUInt32Value(PLAYER_FIELD_KILLS, 1, true);       // add 1 today_kill
                                                            // add 1 lifetime_kill
    ApplyModUInt32Value(PLAYER_FIELD_KILLS_LIFETIME, 1, true);
    ApplyModUInt32Value(PLAYER_FIELD_HONOR_TODAY, (uint32)(approx_honor*10), true);
}

uint32 Player::GetGuildIdFromDB(uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT `guildid` FROM `guild_member` WHERE `guid`='"<<guid<<"'";
    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    if( result )
    {
        uint32 v = result->Fetch()[0].GetUInt32();
        delete result;
        return v;
    }
    else
        return 0;
}

uint32 Player::GetRankFromDB(uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT `rank` FROM `guild_member` WHERE `guid`='"<<guid<<"'";
    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    if( result )
    {
        uint32 v = result->Fetch()[0].GetUInt32();
        delete result;
        return v;
    }
    else
        return 0;
}

uint32 Player::GetZoneIdFromDB(uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT `map`,`position_x`,`position_y` FROM `character` WHERE `guid`='"<<GUID_LOPART(guid)<<"'";
    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    if( !result )
        return 0;

    Field* fields = result->Fetch();

    uint32 map  = fields[0].GetUInt32();
    float posx = fields[1].GetFloat();
    float posy = fields[2].GetFloat();

    delete result;

    return MapManager::Instance().GetZoneId(map,posx,posy);
}

void Player::UpdateZone(uint32 newZone)
{
    /// \todo Fix me: We might receive zoneupdate for a new entered zone, but the player coordinates are still in the old zone
    if (newZone != GetZoneId())
        sLog.outDebug("Zone update problem: received zone = %u, current zone = %u",newZone,GetZoneId());

    AreaTableEntry const* zone = GetAreaEntryByAreaID(newZone);
    if(!zone)
        return;

    if (sWorld.getConfig(CONFIG_WEATHER))
    {
        Weather *wth = sWorld.FindWeather(zone->ID);
        if(wth)
        {
            wth->SendWeatherUpdateToPlayer(this);
        }
        else
        {
            if(!sWorld.AddWeather(zone->ID))
            {
                // send fine weather packet to remove old zone's weather
                Weather::SendFineWeatherUpdateToPlayer(this);
            }
        }
    }

    pvpInfo.inHostileArea =
        GetTeam() == ALLIANCE && zone->team == AREATEAM_HORDE ||
        GetTeam() == HORDE    && zone->team == AREATEAM_ALLY  ||
        sWorld.IsPvPRealm()   && zone->team == AREATEAM_NONE;

    if(pvpInfo.inHostileArea)                               // in hostile area
    {
        if(!IsPvP() || pvpInfo.endTimer != 0)
            UpdatePvP(true, true);
    }
    else                                                    // in friendly area
    {
        if(IsPvP() && !HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP) && pvpInfo.endTimer == 0)
            pvpInfo.endTimer = time(0);                     // start toggle-off
    }

    // zonetype is flags
    // flags & 0x00000001   (1)     - snow (only Dun Morogh, Naxxramas, Razorfen Downs and Winterspring)
    // flags & 0x00000002   (2)     - unknown, (only Naxxramas and Razorfen Downs)
    // flags & 0x00000004   (4)     - On Map Dungeon
    // flags & 0x00000008   (8)     - slave capital city flag?
    // flags & 0x00000010   (16)    - unknown
    // flags & 0x00000020   (32)    - slave capital city flag?
    // flags & 0x00000040   (64)    - many zones have this flag
    // flags & 0x00000080   (128)   - arena
    // flags & 0x00000100   (256)   - main capital city flag
    // flags & 0x00000200   (512)   - only for one zone named "City" (where it located?)
    // flags & 0x00000400   (1024)  - outland zones? (only Eye of the Storm not have this flag, but have 0x00004000 flag)
    // flags & 0x00000800   (2048)  - sanctuary area (PvP disabled)
    // flags & 0x00001000   (4096)  - only Netherwing Ledge, Socrethar's Seat, Tempest Keep, The Arcatraz, The Botanica, The Mechanar
    // flags & 0x00002000   (8192)  - not used now (no area/zones with this flag set) ...
    // flags & 0x00004000   (16384) - outland zones? (only Circle of Blood Arena not have this flag, but have 0x00000400 flag)
    // flags & 0x00008000   (32768) - pvp objective area?

    if((zone->flags & 0x800) != 0)                          // in sanctuary
    {
        UpdatePvP(false, true);                             // i'm right? need disable PvP in this area...
    }
    else if((zone->flags & 0x100) != 0)                     // in capital city
    {
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        SetRestType(2);
        InnEnter(time(0),0,0,0);
    }
    else                                                    // anywhere else
    {
        if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) && GetRestType()==2)
        {
            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        }
    }
}

//If players are too far way of duel flag... then player loose the duel
void Player::CheckDuelDistance(time_t currTime)
{
    if(!duel) return;

    uint64 duelFlagGUID = GetUInt64Value(PLAYER_DUEL_ARBITER);
    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*this, duelFlagGUID);
    if(!obj)
        return;

    if(duel->outOfBound == 0)
    {
        if(!IsWithinDistInMap(obj, 50))
        {
            duel->outOfBound = currTime;

            WorldPacket data(SMSG_DUEL_OUTOFBOUNDS, 0);
            GetSession()->SendPacket(&data);
        }
    }
    else
    {
        if(IsWithinDistInMap(obj, 40))
        {
            duel->outOfBound = 0;

            WorldPacket data(SMSG_DUEL_INBOUNDS, 0);
            GetSession()->SendPacket(&data);
        }
        else if(currTime >= (duel->outOfBound+10))
        {
            DuelComplete(2);
        }
    }
}

//type: 0=cleanup ; 1=i won ; 2=i fled
void Player::DuelComplete(uint8 type)
{
    // duel not requested
    if(!duel)
        return;

    WorldPacket data(SMSG_DUEL_COMPLETE, (1));
    data << (uint8)((type!=0) ? 1 : 0);
    GetSession()->SendPacket(&data);
    duel->opponent->GetSession()->SendPacket(&data);

    if(type != 0)
    {
        data.Initialize(SMSG_DUEL_WINNER, (1+20));          // we guess size
        data << (uint8)((type==1) ? 0 : 1);                 // 0 = just won; 1 = fled
        data << duel->opponent->GetName();
        data << GetName();
        SendMessageToSet(&data,true);
    }

    // cool-down duel spell
    /*data.Initialize(SMSG_SPELL_COOLDOWN, 17);

    data<<GetGUID();
    data<<uint8(0x0);

    data<<(uint32)7266;
    data<<uint32(0x0);
    GetSession()->SendPacket(&data);
    data.Initialize(SMSG_SPELL_COOLDOWN, 17);
    data<<duel->opponent->GetGUID();
    data<<uint8(0x0);
    data<<(uint32)7266;
    data<<uint32(0x0);
    duel->opponent->GetSession()->SendPacket(&data);*/

    //Remove Duel Flag object
    GameObject* obj = ObjectAccessor::Instance().GetGameObject(*this, GetUInt64Value(PLAYER_DUEL_ARBITER));
    if(obj)
        duel->initiator->RemoveGameObject(obj,true);

    /* remove auras */
    vector<uint32> auras2remove;
    AuraMap& vAuras = duel->opponent->GetAuras();
    for (AuraMap::iterator i = vAuras.begin(); i != vAuras.end(); i++)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGUID() == GetGUID() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }
    for(int i=0; i<auras2remove.size(); i++)
        duel->opponent->RemoveAurasDueToSpell(auras2remove[i]);

    auras2remove.clear();
    AuraMap& Auras = GetAuras();
    for (AuraMap::iterator i = Auras.begin(); i != Auras.end(); i++)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGUID() == duel->opponent->GetGUID() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }
    for(int i=0; i<auras2remove.size(); i++)
        RemoveAurasDueToSpell(auras2remove[i]);

    //cleanups
    SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    duel->opponent->SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    delete duel->opponent->duel;
    duel->opponent->duel = 0;
    delete duel;
    duel = 0;
}

//---------------------------------------------------------//
//       Flight callback
void Player::FlightComplete()
{
    clearUnitState(UNIT_STAT_IN_FLIGHT);
    SetMoney( m_dismountCost);
    Unmount();
    MoveToThreatList();
    if(pvpInfo.inHostileArea)
        CastSpell(this, 2479, true);
}

void Player::_ApplyItemMods(Item *item, uint8 slot,bool apply)
{
    if(slot >= INVENTORY_SLOT_BAG_END || !item) return;

    // not apply/remove mods for broken item
    if(item->IsBroken()) return;

    ItemPrototype const *proto = item->GetProto();

    if(!proto) return;

    sLog.outDetail("applying mods for item %u ",item->GetGUIDLow());

    if(proto->ItemSet)
    {
        if (apply)
            AddItemsSetItem(this,item);
        else
            RemoveItemsSetItem(this,proto);
    }

    _RemoveStatsMods();

    // remove ammo bonuses at un-apply
    if( !apply && slot==EQUIPMENT_SLOT_RANGED )
        _ApplyAmmoBonuses(apply);

    AuraList& mModBaseResistancePct = GetAurasByType(SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
    AuraList& mModHaste             = GetAurasByType(SPELL_AURA_MOD_HASTE);
    AuraList& mModRangedHaste       = GetAurasByType(SPELL_AURA_MOD_RANGED_HASTE);
    AuraList& mModRangedAmmoHaste   = GetAurasByType(SPELL_AURA_MOD_RANGED_AMMO_HASTE);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(false);

    _ApplyItemBonuses(proto,slot,apply);

    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(true);

    // add ammo bonuses at apply
    if( apply && slot==EQUIPMENT_SLOT_RANGED )
        _ApplyAmmoBonuses(apply);

    _ApplyStatsMods();

    if(apply)
        CastItemEquipSpell(item);
    else
        for (int i = 0; i < 5; i++)
            if(proto->Spells[i].SpellId)
                RemoveAurasDueToSpell(proto->Spells[i].SpellId );

    ApplyEnchantment(item, apply);

    if(proto->Socket[0].Color)    //only (un)equipping of items with sockets can influence metagems, so no need to waste time with normal items
        CorrectMetaGemEnchants(slot, apply);

    sLog.outDebug("_ApplyItemMods complete.");
}

void Player::_ApplyItemBonuses(ItemPrototype const *proto,uint8 slot,bool apply)
{
    if(slot >= INVENTORY_SLOT_BAG_END || !proto) return;

    int32 val;
    std::string typestr;
    std::string applystr = apply ? "Add" : "Remove";
    for (int i = 0; i < 10; i++)
    {
        val = proto->ItemStat[i].ItemStatValue;

        if(val==0)
            continue;

        switch (proto->ItemStat[i].ItemStatType)
        {
            case ITEM_STAT_POWER:                           // modify MP
                ApplyMaxPowerMod(POWER_MANA, val, apply);
                //typestr = "Mana";
                break;
            case ITEM_STAT_HEALTH:                          // modify HP
                ApplyMaxHealthMod(val, apply);
                //typestr = "Health";
                break;
            case ITEM_STAT_AGILITY:                         // modify agility
                ApplyStatMod(STAT_AGILITY,                val, apply);
                if(val > 0)
                    ApplyPosStatMod(STAT_AGILITY,         val, apply);
                else
                    ApplyNegStatMod(STAT_AGILITY,        -val, apply);
                //typestr = "AGILITY";
                break;
            case ITEM_STAT_STRENGTH:                        //modify strength
                ApplyStatMod(STAT_STRENGTH,               val, apply);
                if(val > 0)
                    ApplyPosStatMod(STAT_STRENGTH,        val, apply);
                else
                    ApplyNegStatMod(STAT_STRENGTH,       -val, apply);
                //typestr = "STRENGHT";
                break;
            case ITEM_STAT_INTELLECT:                       //modify intellect
                ApplyStatMod(STAT_INTELLECT,              val, apply);
                if(val > 0)
                    ApplyPosStatMod(STAT_INTELLECT,       val, apply);
                else
                    ApplyNegStatMod(STAT_INTELLECT,      -val, apply);
                //ApplyMaxPowerMod(POWER_MANA,              val*15, apply);
                //typestr = "INTELLECT";
                break;
            case ITEM_STAT_SPIRIT:                          //modify spirit
                ApplyStatMod(STAT_SPIRIT,                 val, apply);
                if(val > 0)
                    ApplyPosStatMod(STAT_SPIRIT,          val, apply);
                else
                    ApplyNegStatMod(STAT_SPIRIT,         -val, apply);
                //typestr = "SPIRIT";
                break;
            case ITEM_STAT_STAMINA:                         //modify stamina
                ApplyStatMod(STAT_STAMINA,                val, apply);
                if(val > 0)
                    ApplyPosStatMod(STAT_STAMINA,         val, apply);
                else
                    ApplyNegStatMod(STAT_STAMINA,        -val, apply);
                //ApplyMaxHealthMod(                        val*10,apply);
                //typestr = "STAMINA";
                break;

            case ITEM_STAT_DEFENCE_RATING:
                ApplyRatingMod(PLAYER_FIELD_DEFENCE_RATING, val, apply);
                break;
            case ITEM_STAT_DODGE_RATING:
                ApplyRatingMod(PLAYER_FIELD_DODGE_RATING, val, apply);
                break;
            case ITEM_STAT_PARRY_RATING:
                ApplyRatingMod(PLAYER_FIELD_PARRY_RATING, val, apply);
                break;
            case ITEM_STAT_SHIELD_BLOCK_RATING:
                ApplyRatingMod(PLAYER_FIELD_BLOCK_RATING, val, apply);
                break;
            case ITEM_STAT_MELEE_HIT_RATING:
                ApplyRatingMod(PLAYER_FIELD_MELEE_HIT_RATING, val, apply);
                break;
            case ITEM_STAT_RANGED_HIT_RATING:
                ApplyRatingMod(PLAYER_FIELD_RANGED_HIT_RATING, val, apply);
                break;
            case ITEM_STAT_SPELL_HIT_RATING:
                ApplyRatingMod(PLAYER_FIELD_SPELL_HIT_RATING, val, apply);
                break;
            case ITEM_STAT_MELEE_CS_RATING:
                ApplyRatingMod(PLAYER_FIELD_MELEE_CRIT_RATING, val, apply);
                break;
            case ITEM_STAT_RANGED_CS_RATING:
                ApplyRatingMod(PLAYER_FIELD_RANGED_CRIT_RATING, val, apply);
                break;
            case ITEM_STAT_SPELL_CS_RATING:
                ApplyRatingMod(PLAYER_FIELD_SPELL_CRIT_RATING, val, apply);
                break;
            case ITEM_STAT_MELEE_HA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_MELEE_HA_RATING, val, apply);
                break;
            case ITEM_STAT_RANGED_HA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_RANGED_HA_RATING, val, apply);
                break;
            case ITEM_STAT_SPELL_HA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_SPELL_HA_RATING, val, apply);
                break;
            case ITEM_STAT_MELEE_CA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_MELEE_CA_RATING, val, apply);
                break;
            case ITEM_STAT_RANGED_CA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_RANGED_CA_RATING, val, apply);
                break;
            case ITEM_STAT_SPELL_CA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_SPELL_CA_RATING, val, apply);
                break;
            case ITEM_STAT_MELEE_HASTE_RATING:
                ApplyRatingMod(PLAYER_FIELD_MELEE_HASTE_RATING, val, apply);
                break;
            case ITEM_STAT_RANGED_HASTE_RATING:
                ApplyRatingMod(PLAYER_FIELD_RANGED_HASTE_RATING, val, apply);
                break;
            case ITEM_STAT_SPELL_HASTE_RATING:
                ApplyRatingMod(PLAYER_FIELD_SPELL_HASTE_RATING, val, apply);
                break;
            case ITEM_STAT_HIT_RATING:
                ApplyRatingMod(PLAYER_FIELD_HIT_RATING, val, apply);
                break;
            case ITEM_STAT_CS_RATING:
                ApplyRatingMod(PLAYER_FIELD_CRIT_RATING, val, apply);
                break;
            case ITEM_STAT_HA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_HA_RATING, val, apply);
                break;
            case ITEM_STAT_CA_RATING:
                //ApplyRatingMod(PLAYER_FIELD_CA_RATING, val, apply);
                break;
            case ITEM_STAT_RESILIENCE_RATING:
                ApplyRatingMod(PLAYER_FIELD_RESILIENCE_RATING, val, apply);
                break;
            case ITEM_STAT_HASTE_RATING:
                ApplyRatingMod(PLAYER_FIELD_MELEE_HASTE_RATING, val, apply);
                ApplyRatingMod(PLAYER_FIELD_RANGED_HASTE_RATING, val, apply);
                ApplyRatingMod(PLAYER_FIELD_SPELL_HASTE_RATING, val, apply);
                break;
        }
        //sLog.outDebug("%s %s: \t\t%u", applystr.c_str(), typestr.c_str(), val);
    }

    if (proto->Armor)
    {
        ApplyArmorMod( proto->Armor, apply);
        //sLog.outDebug("%s Armor: \t\t%u", applystr.c_str(),  proto->Armor);
    }

    if (proto->Block)
    {
        m_AuraModifiers[SPELL_AURA_MOD_SHIELD_BLOCKVALUE]+= (apply ? long(proto->Block) : -long(proto->Block) );
        //sLog.outDebug("%s Block: \t\t%u", applystr.c_str(),  proto->Block);
    }

    if (proto->HolyRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_HOLY, proto->HolyRes, apply);
        //sLog.outDebug("%s HolyRes: \t\t%u", applystr.c_str(),  proto->HolyRes);
    }

    if (proto->FireRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_FIRE, proto->FireRes, apply);
        //sLog.outDebug("%s FireRes: \t\t%u", applystr.c_str(),  proto->FireRes);
    }

    if (proto->NatureRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_NATURE, proto->NatureRes, apply);
        //sLog.outDebug("%s NatureRes: \t\t%u", applystr.c_str(),  proto->NatureRes);
    }

    if (proto->FrostRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_FROST, proto->FrostRes, apply);
        //sLog.outDebug("%s FrostRes: \t\t%u", applystr.c_str(),  proto->FrostRes);
    }

    if (proto->ShadowRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_SHADOW, proto->ShadowRes, apply);
        //sLog.outDebug("%s ShadowRes: \t\t%u", applystr.c_str(),  proto->ShadowRes);
    }

    if (proto->ArcaneRes)
    {
        ApplyResistanceMod(SPELL_SCHOOL_ARCANE, proto->ArcaneRes, apply);
        //sLog.outDebug("%s ArcaneRes: \t\t%u", applystr.c_str(),  proto->ArcaneRes);
    }

    if(!IsUseEquipedWeapon())
        return;

    uint8 MINDAMAGEFIELD = 0;
    uint8 MAXDAMAGEFIELD = 0;

    if( slot == EQUIPMENT_SLOT_RANGED && (
        proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_THROWN ||
        proto->InventoryType == INVTYPE_RANGEDRIGHT ))
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINRANGEDDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXRANGEDDAMAGE;
        //typestr = "Ranged";
    }
    else if(slot==EQUIPMENT_SLOT_MAINHAND)
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXDAMAGE;
        //typestr = "Mainhand";
    }
    else if(slot==EQUIPMENT_SLOT_OFFHAND)
    {
        MINDAMAGEFIELD = UNIT_FIELD_MINOFFHANDDAMAGE;
        MAXDAMAGEFIELD = UNIT_FIELD_MAXOFFHANDDAMAGE;
        //typestr = "Offhand";
    }

    if (proto->Damage[0].DamageMin > 0 && MINDAMAGEFIELD)
    {
        ApplyModFloatValue(MINDAMAGEFIELD, proto->Damage[0].DamageMin, apply);
        //sLog.outDetail("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMin, GetFloatValue(MINDAMAGEFIELD));
    }

    if (proto->Damage[0].DamageMax  > 0 && MAXDAMAGEFIELD)
    {
        ApplyModFloatValue(MAXDAMAGEFIELD, proto->Damage[0].DamageMax, apply);
        //sLog.outDetail("%s %s mindam: %f, now is: %f", applystr.c_str(), typestr.c_str(), proto->Damage[0].DamageMax, GetFloatValue(MAXDAMAGEFIELD));
    }

    if (proto->Delay)
    {
        if(slot == EQUIPMENT_SLOT_RANGED)
        {
            SetAttackTime(RANGED_ATTACK, apply ? proto->Delay: 2000);
            //typestr = "Range";
            //sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
        else if(slot==EQUIPMENT_SLOT_MAINHAND)
        {
            SetAttackTime(BASE_ATTACK, apply ? proto->Delay: 2000);
            //typestr = "Mainhand";
            //sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
        else if(slot==EQUIPMENT_SLOT_OFFHAND)
        {
            SetAttackTime(OFF_ATTACK, apply ? proto->Delay: 2000);
            //typestr = "Offhand";
            //sLog.outDebug("%s %s Delay: \t\t%u", applystr.c_str(), typestr.c_str(), proto->Delay);
        }
    }
}

void Player::CastItemEquipSpell(Item *item)
{
    if(!item) return;

    ItemPrototype const *proto = item->GetProto();

    if(!proto) return;

    for (int i = 0; i < 5; i++)
    {
        if(!proto->Spells[i].SpellId ) continue;
        if(proto->Spells[i].SpellTrigger != ON_EQUIP) continue;

        SpellEntry const *spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId);
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId);
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
    if(!item || item->IsBroken())
        return;

    ItemPrototype const *proto = item->GetProto();
    if(!proto)
        return;

    if (!Target || Target == this )
        return;

    for (int i = 0; i < 5; i++)
    {
        if(!proto->Spells[i].SpellId ) continue;

        SpellEntry const *spellInfo = sSpellStore.LookupEntry(proto->Spells[i].SpellId);
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown Item spellid %i", proto->Spells[i].SpellId);
            continue;
        }

        if(proto->Spells[i].SpellTrigger != CHANCE_ON_HIT) continue;

        float chance = spellInfo->procChance <= 100 ? float(spellInfo->procChance) : GetWeaponProcChance();
        if (roll_chance_f(chance))
            this->CastSpell(Target, spellInfo->Id, true, item);
    }

    // item combat enchantments
    for(int e_slot = 0; e_slot < MAX_ENCHANTMENT_SLOT; ++e_slot)
    {
        uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(e_slot));
        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant) continue;
        for (int s=0;s<3;s++)
        {
            uint32 enchant_display = pEnchant->display_type[s];
            float chance = pEnchant->amount[s] != 0 ? float(pEnchant->amount[s]) : GetWeaponProcChance();
            uint32 enchant_spell_id = pEnchant->spellid[s];
            SpellEntry const *enchantSpell_info = sSpellStore.LookupEntry(enchant_spell_id);

            if(!enchantSpell_info) continue;

            if(enchant_display!=4 && enchant_display!=2 && enchant_display!=5 && IsItemSpellToCombat(enchantSpell_info))
                if (roll_chance_f(chance))
                    this->CastSpell(Target, enchantSpell_info->Id, true);
        }
    }
}

// only some item spell/auras effects can be executed when item is equiped.
// If not you can have unexpected beaviur. like item giving damage to player when equip.
bool Player::IsItemSpellToEquip(SpellEntry const *spellInfo)
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
bool Player::IsItemSpellToCombat(SpellEntry const *spellInfo)
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
        {
            if(m_items[i]->IsBroken())
                continue;

            ItemPrototype const *proto = m_items[i]->GetProto();
            if(!proto)
                continue;

            if(proto->ItemSet)
                RemoveItemsSetItem(this,proto);

            for (int m = 0; m < 5; m++)
            {
                if(proto->Spells[m].SpellId)
                    RemoveAurasDueToSpell(proto->Spells[m].SpellId );
            }

            ApplyEnchantment(m_items[i], false);
        }
    }

    _RemoveStatsMods();

    // additional bonuses from ammo
    if(GetItemByPos(INVENTORY_SLOT_BAG_0,EQUIPMENT_SLOT_RANGED))
        _ApplyAmmoBonuses(false);

    AuraList& mModBaseResistancePct = GetAurasByType(SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
    AuraList& mModHaste             = GetAurasByType(SPELL_AURA_MOD_HASTE);
    AuraList& mModRangedHaste       = GetAurasByType(SPELL_AURA_MOD_RANGED_HASTE);
    AuraList& mModRangedAmmoHaste   = GetAurasByType(SPELL_AURA_MOD_RANGED_AMMO_HASTE);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(false);

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            if(m_items[i]->IsBroken())
                continue;
            ItemPrototype const *proto = m_items[i]->GetProto();
            if(!proto)
                continue;
            _ApplyItemBonuses(proto,i, false);
        }
    }

    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(true);

    _ApplyStatsMods();

    sLog.outDebug("_RemoveAllItemMods complete.");
}

void Player::_ApplyAllItemMods()
{
    sLog.outDebug("_ApplyAllItemMods start.");

    _RemoveStatsMods();

    AuraList& mModBaseResistancePct = GetAurasByType(SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
    AuraList& mModHaste             = GetAurasByType(SPELL_AURA_MOD_HASTE);
    AuraList& mModRangedHaste       = GetAurasByType(SPELL_AURA_MOD_RANGED_HASTE);
    AuraList& mModRangedAmmoHaste   = GetAurasByType(SPELL_AURA_MOD_RANGED_AMMO_HASTE);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(false);
    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(false);

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            if(m_items[i]->IsBroken())
                continue;

            ItemPrototype const *proto = m_items[i]->GetProto();
            if(!proto)
                continue;

            _ApplyItemBonuses(proto,i, true);
        }
    }

    for(AuraList::iterator i = mModRangedAmmoHaste.begin(); i != mModRangedAmmoHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModRangedHaste.begin(); i != mModRangedHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModHaste.begin(); i != mModHaste.end(); ++i)
        (*i)->ApplyModifier(true);
    for(AuraList::iterator i = mModBaseResistancePct.begin(); i != mModBaseResistancePct.end(); ++i)
        (*i)->ApplyModifier(true);

    // additional bonuses from ammo
    if(GetItemByPos(INVENTORY_SLOT_BAG_0,EQUIPMENT_SLOT_RANGED))
        _ApplyAmmoBonuses(true);

    _ApplyStatsMods();

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            if(m_items[i]->IsBroken())
                continue;

            ItemPrototype const *proto = m_items[i]->GetProto();
            if(!proto)
                continue;

            if(proto->ItemSet)
                AddItemsSetItem(this,m_items[i]);

            CastItemEquipSpell(m_items[i]);

            ApplyEnchantment(m_items[i], true);
        }
    }

    sLog.outDebug("_ApplyAllItemMods complete.");
}

void Player::_ApplyAmmoBonuses(bool apply)
{
    // check ammo 
    uint32 ammo_id = GetUInt32Value(PLAYER_AMMO_ID);
    if(!ammo_id)
        return;

    ItemPrototype const *ammo_proto = objmgr.GetItemPrototype( ammo_id );
    if( !ammo_proto || ammo_proto->Class!=ITEM_CLASS_PROJECTILE )
        return;

    // check ranged weapon
    Item *weapon = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED );
    if(!weapon  || weapon->IsBroken() )
        return;

    ItemPrototype const* weapon_proto = weapon->GetProto();
    if(!weapon_proto || weapon_proto->Class!=ITEM_CLASS_WEAPON )
        return;

    // check ammo ws. weapon compatibility
    switch(weapon_proto->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            if(ammo_proto->SubClass!=ITEM_SUBCLASS_ARROW)
                return;
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            if(ammo_proto->SubClass!=ITEM_SUBCLASS_BULLET)
                return;
            break;
        default:
            return;
    }

    // all ok
    std::string applystr = apply ? "Add" : "Remove";

    float minDamage = ammo_proto->Damage[0].DamageMin*GetAttackTime(RANGED_ATTACK)/1000;
    float maxDamage = ammo_proto->Damage[0].DamageMax*GetAttackTime(RANGED_ATTACK)/1000;

    ApplyModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, minDamage, apply);
    sLog.outDetail("%s Ranged mindam: %f, now is: %f", applystr.c_str(), minDamage, GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE));
    ApplyModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, maxDamage, apply);
    sLog.outDetail("%s Ranged mindam: %f, now is: %f", applystr.c_str(), maxDamage, GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
}

/*Loot type MUST be
1-corpse, go
2-skinning
3-Fishing
*/

void Player::SendLoot(uint64 guid, LootType loot_type)
{
    Loot    *loot = 0;
    PermissionTypes permission = ALL_PERMISSION;

    sLog.outDebug("Player::SendLoot");
    if (IS_GAMEOBJECT_GUID(guid))
    {
        sLog.outDebug("       IS_GAMEOBJECT_GUID(guid)");
        GameObject *go =
            ObjectAccessor::Instance().GetGameObject(*this, guid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        if (!go || (loot_type != LOOT_FISHING || go->GetOwnerGUID() != GetGUID()) && !go->IsWithinDistInMap(this,OBJECT_ITERACTION_DISTANCE))
            return;

        loot = &go->loot;

        if(go->getLootState() == GO_CLOSED)
        {
            uint32 lootid =  go->lootid;

            if(lootid)
            {
                sLog.outDebug("       if(lootid)");
                FillLoot(loot, lootid, LootTemplates_Gameobject);
            }

            if(loot_type == LOOT_FISHING)
                go->getFishLoot(loot);

            go->SetLootState(GO_OPEN);
        }
    }
    else if (IS_ITEM_GUID(guid))
    {
        Item *item = this->GetItemByPos( this->GetPosByGuid( guid ));

        if (!item)
            return;

        if(loot_type == LOOT_DISENCHANTING)
        {
            loot = &item->loot;

            if(!item->m_lootGenerated)
            {
                item->m_lootGenerated = true;
                FillLoot(loot, item->GetProto()->DisenchantID, LootTemplates_Disenchant);
            }
        }
        else
        {
            loot = &item->loot;

            if(!item->m_lootGenerated)
            {
                item->m_lootGenerated = true;
                FillLoot(loot, item->GetEntry(), LootTemplates_Item);
            }
        }
    }
    else
    {
        Creature *creature =
            ObjectAccessor::Instance().GetCreature(*this, guid);

        // must be in range and creature must be alive for pickpocket and must be dead for another loot
        if (!creature || creature->isAlive()!=(loot_type == LOOT_PICKPOCKETING) || !creature->IsWithinDistInMap(this,OBJECT_ITERACTION_DISTANCE))
            return;

        if(loot_type == LOOT_PICKPOCKETING && IsFriendlyTo(creature))
            return;

        loot   = &creature->loot;

        if(loot_type == LOOT_PICKPOCKETING)
        {
            uint32 lootid = creature->GetCreatureInfo()->pickpocketLootId;

            if ( !creature->lootForPickPocketed )
            {
                creature->lootForPickPocketed = true;
                loot->clear();

                if (!creature->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_VENDOR) && lootid)
                    FillLoot(loot, lootid, LootTemplates_Pickpocketing);
                // Generate extra money for pick pocket loot
                const uint32 a = urand(0, creature->getLevel()/2);
                const uint32 b = urand(0, getLevel()/2);
                loot->gold = uint32(10 * (a + b) * sWorld.getRate(RATE_DROP_MONEY));
            }
        }
        else
        {
            uint32 lootid = creature->GetCreatureInfo()->lootid;

            // the player whose group may loot the corpse
            Player *recipient = creature->GetLootRecipient();
            if (!recipient)
            {
                creature->SetLootRecipient(this);
                recipient = this;
            }

            if (creature->lootForPickPocketed)
            {
                creature->lootForPickPocketed = false;
                loot->clear();
            }

            if(!creature->lootForBody)
            {
                creature->lootForBody = true;
                if (!creature->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_VENDOR) && lootid)
                    FillLoot(loot, lootid, LootTemplates_Creature);

                creature->generateMoneyLoot();

                if (recipient->groupInfo.group)
                {
                    // round robin style looting applies for all low
                    // quality items in each loot method except free for all
                    Group *group = recipient->groupInfo.group;

                    // next by circle
                    uint64 next_guid = group->GetNextGuidAfter(group->GetLooterGuid());
                    if(next_guid==0)
                        next_guid = group->GetMembers().front().guid;

                    group->SetLooterGuid(next_guid);

                    switch (group->GetLootMethod())
                    {
                        case GROUP_LOOT:
                            // GroupLoot delete items over threshold (threshold even not implemented), and roll them. Items with quality<threshold, round robin
                            group->GroupLoot(recipient->GetGUID(), loot, creature);
                            break;
                        case NEED_BEFORE_GREED:
                            group->NeedBeforeGreed(recipient->GetGUID(), loot, creature);
                            break;
                        default:
                            break;
                    }
                }
            }

            if (loot_type == LOOT_SKINNING)
                FillLoot(loot, creature->GetCreatureInfo()->SkinLootId, LootTemplates_Skinning);

            if (!groupInfo.group && recipient == this)
                permission = ALL_PERMISSION;
            else
            {
                if (groupInfo.group)
                {
                    Group *group = groupInfo.group;
                    if ((group == recipient->groupInfo.group) && (group->GetLooterGuid() == GetGUID() || loot->released || group->GetLootMethod() == FREE_FOR_ALL))
                        permission = ALL_PERMISSION;
                    else
                    if (group == recipient->groupInfo.group)
                        permission = GROUP_PERMISSION;
                    else
                        permission = NONE_PERMISSION;
                }
                else
                    permission = NONE_PERMISSION;
            }
        }
    }

    m_lootGuid = guid;

    QuestItemList *q_list = 0;
    if (permission != NONE_PERMISSION)
    {
        QuestItemMap::iterator itr = loot->PlayerQuestItems.find(this);
        if (itr == loot->PlayerQuestItems.end())
            q_list = FillQuestLoot(this, loot);
        else
            q_list = itr->second;
    }

    // LOOT_PICKPOCKETING and LOOT_DISENCHANTING unsupported by client, sending LOOT_SKINNING instead
    if(loot_type == LOOT_PICKPOCKETING || loot_type == LOOT_DISENCHANTING)
        loot_type = LOOT_SKINNING;

    WorldPacket data(SMSG_LOOT_RESPONSE, (9+50));           // we guess size

    data << guid;
    data << uint8(loot_type);
    data << LootView(*loot, q_list, permission);

    SendDirectMessage(&data);

    // add 'this' player as one of the players that are looting 'loot'
    if (permission != NONE_PERMISSION)
        loot->AddLooter(GetGUID());
}

void Player::SendNotifyLootMoneyRemoved()
{
    WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
    GetSession()->SendPacket( &data );
}

void Player::SendNotifyLootItemRemoved(uint8 lootSlot)
{
    WorldPacket data(SMSG_LOOT_REMOVED, 1);
    data << uint8(lootSlot);
    GetSession()->SendPacket( &data );
}

void Player::SendUpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << Field;
    data << Value;
    SendMessageToSet(&data, true);
    //GetSession()->SendPacket(&data);
}

void Player::SendInitWorldStates()
{
    // data depends on zoneid/mapid...
    uint16 NumberOfFields = 0;
    uint32 mapid = GetMapId();
    uint32 zoneid = GetZoneId();
    sLog.outDebug("Sending SMSG_INIT_WORLD_STATES to Map:%u, Zone: %u", mapid, zoneid);
    // may be exist better way to do this...
    switch(zoneid)
    {
        case 0:
        case 1:
        case 4:
        case 8:
        case 10:
        case 11:
        case 12:
        case 36:
        case 38:
        case 40:
        case 41:
        case 51:
        case 267:
        case 1519:
        case 1537:
        case 2257:
        case 2918:
            NumberOfFields = 6;
            break;
        case 2597:
            NumberOfFields = 81;
            break;
        case 3277:
            NumberOfFields = 14;
            break;
        case 3358:
            NumberOfFields = 38;
            break;
        case 3483:
            NumberOfFields = 22;
            break;
        case 3519:
            NumberOfFields = 36;
            break;
        case 3521:
            NumberOfFields = 35;
            break;
        case 3703:
            NumberOfFields = 9;
            break;
        default:
            NumberOfFields = 10;
            break;
    }

    WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+2+(NumberOfFields*8)));
    data << mapid;                                          // mapid
    data << zoneid;                                         // zone id
    data << NumberOfFields;                                 // count of uint64 blocks
    data << uint32(0x8d8) << uint32(0x0);                   // 1
    data << uint32(0x8d7) << uint32(0x0);                   // 2
    data << uint32(0x8d6) << uint32(0x0);                   // 3
    data << uint32(0x8d5) << uint32(0x0);                   // 4
    data << uint32(0x8d4) << uint32(0x0);                   // 5
    data << uint32(0x8d3) << uint32(0x0);                   // 6
    if(mapid == 530)                                        // Outland
    {
        data << uint32(0x9bf) << uint32(0x0);               // 7
        data << uint32(0x9bd) << uint32(0xF);               // 8
        data << uint32(0x9bb) << uint32(0xF);               // 9
    }
    switch(zoneid)
    {
        case 1:
        case 11:
        case 12:
        case 38:
        case 40:
        case 51:
        case 1519:
        case 1537:
        case 2257:
            break;
        case 2597:                                          // AV
            data << uint32(0x7ae) << uint32(0x1);           // 7
            data << uint32(0x532) << uint32(0x1);           // 8
            data << uint32(0x531) << uint32(0x0);           // 9
            data << uint32(0x52e) << uint32(0x0);           // 10
            data << uint32(0x571) << uint32(0x0);           // 11
            data << uint32(0x570) << uint32(0x0);           // 12
            data << uint32(0x567) << uint32(0x1);           // 13
            data << uint32(0x566) << uint32(0x1);           // 14
            data << uint32(0x550) << uint32(0x1);           // 15
            data << uint32(0x544) << uint32(0x0);           // 16
            data << uint32(0x536) << uint32(0x0);           // 17
            data << uint32(0x535) << uint32(0x1);           // 18
            data << uint32(0x518) << uint32(0x0);           // 19
            data << uint32(0x517) << uint32(0x0);           // 20
            data << uint32(0x574) << uint32(0x0);           // 21
            data << uint32(0x573) << uint32(0x0);           // 22
            data << uint32(0x572) << uint32(0x0);           // 23
            data << uint32(0x56f) << uint32(0x0);           // 24
            data << uint32(0x56e) << uint32(0x0);           // 25
            data << uint32(0x56d) << uint32(0x0);           // 26
            data << uint32(0x56c) << uint32(0x0);           // 27
            data << uint32(0x56b) << uint32(0x0);           // 28
            data << uint32(0x56a) << uint32(0x1);           // 29
            data << uint32(0x569) << uint32(0x1);           // 30
            data << uint32(0x568) << uint32(0x1);           // 13
            data << uint32(0x565) << uint32(0x0);           // 32
            data << uint32(0x564) << uint32(0x0);           // 33
            data << uint32(0x563) << uint32(0x0);           // 34
            data << uint32(0x562) << uint32(0x0);           // 35
            data << uint32(0x561) << uint32(0x0);           // 36
            data << uint32(0x560) << uint32(0x0);           // 37
            data << uint32(0x55f) << uint32(0x0);           // 38
            data << uint32(0x55e) << uint32(0x0);           // 39
            data << uint32(0x55d) << uint32(0x0);           // 40
            data << uint32(0x3c6) << uint32(0x4);           // 41
            data << uint32(0x3c4) << uint32(0x6);           // 42
            data << uint32(0x3c2) << uint32(0x4);           // 43
            data << uint32(0x516) << uint32(0x1);           // 44
            data << uint32(0x515) << uint32(0x0);           // 45
            data << uint32(0x3b6) << uint32(0x6);           // 46
            data << uint32(0x55c) << uint32(0x0);           // 47
            data << uint32(0x55b) << uint32(0x0);           // 48
            data << uint32(0x55a) << uint32(0x0);           // 49
            data << uint32(0x559) << uint32(0x0);           // 50
            data << uint32(0x558) << uint32(0x0);           // 51
            data << uint32(0x557) << uint32(0x0);           // 52
            data << uint32(0x556) << uint32(0x0);           // 53
            data << uint32(0x555) << uint32(0x0);           // 54
            data << uint32(0x554) << uint32(0x1);           // 55
            data << uint32(0x553) << uint32(0x1);           // 56
            data << uint32(0x552) << uint32(0x1);           // 57
            data << uint32(0x551) << uint32(0x1);           // 58
            data << uint32(0x54f) << uint32(0x0);           // 59
            data << uint32(0x54e) << uint32(0x0);           // 60
            data << uint32(0x54d) << uint32(0x1);           // 61
            data << uint32(0x54c) << uint32(0x0);           // 62
            data << uint32(0x54b) << uint32(0x0);           // 63
            data << uint32(0x545) << uint32(0x0);           // 64
            data << uint32(0x543) << uint32(0x1);           // 65
            data << uint32(0x542) << uint32(0x0);           // 66
            data << uint32(0x540) << uint32(0x0);           // 67
            data << uint32(0x53f) << uint32(0x0);           // 68
            data << uint32(0x53e) << uint32(0x0);           // 69
            data << uint32(0x53d) << uint32(0x0);           // 70
            data << uint32(0x53c) << uint32(0x0);           // 71
            data << uint32(0x53b) << uint32(0x0);           // 72
            data << uint32(0x53a) << uint32(0x1);           // 73
            data << uint32(0x539) << uint32(0x0);           // 74
            data << uint32(0x538) << uint32(0x0);           // 75
            data << uint32(0x537) << uint32(0x0);           // 76
            data << uint32(0x534) << uint32(0x0);           // 77
            data << uint32(0x533) << uint32(0x0);           // 78
            data << uint32(0x530) << uint32(0x0);           // 79
            data << uint32(0x52f) << uint32(0x0);           // 80
            data << uint32(0x52d) << uint32(0x1);           // 81
        case 3277:                                          // WSG
            data << uint32(0x62d) << uint32(0x0);           // 7 1581 alliance flag captures
            data << uint32(0x62e) << uint32(0x0);           // 8 1582 horde flag captures
            data << uint32(0x609) << uint32(0x0);           // 9 1545 unk, set to 1 on alliance flag pickup...
            data << uint32(0x60a) << uint32(0x0);           // 10 1546 unk, set to 1 on horde flag pickup, after drop it's -1
            data << uint32(0x60b) << uint32(0x2);           // 11 1547 unk
            data << uint32(0x641) << uint32(0x3);           // 12 1601 unk (max flag captures?)
            data << uint32(0x922) << uint32(0x1);           // 13 2338 horde (0 - hide, 1 - flag ok, 2 - flag picked up (flashing), 3 - flag picked up (not flashing)
            data << uint32(0x923) << uint32(0x1);           // 14 2339 alliance (0 - hide, 1 - flag ok, 2 - flag picked up (flashing), 3 - flag picked up (not flashing)
            break;
        case 3358:                                          // AB
            data << uint32(0x6e7) << uint32(0x0);           // 7 1767 stables alliance
            data << uint32(0x6e8) << uint32(0x0);           // 8 1768 stables horde
            data << uint32(0x6e9) << uint32(0x0);           // 9 1769 unk, ST?
            data << uint32(0x6ea) << uint32(0x0);           // 10 1770 stables (show/hide)
            data << uint32(0x6ec) << uint32(0x0);           // 11 1772 farm (0 - horde controlled, 1 - alliance controlled)
            data << uint32(0x6ed) << uint32(0x0);           // 12 1773 farm (show/hide)
            data << uint32(0x6ee) << uint32(0x0);           // 13 1774 farm color
            data << uint32(0x6ef) << uint32(0x0);           // 14 1775 gold mine color, may be FM?
            data << uint32(0x6f0) << uint32(0x0);           // 15 1776 alliance resources
            data << uint32(0x6f1) << uint32(0x0);           // 16 1777 horde resources
            data << uint32(0x6f2) << uint32(0x0);           // 17 1778 horde bases
            data << uint32(0x6f3) << uint32(0x0);           // 18 1779 alliance bases
            data << uint32(0x6f4) << uint32(0x7d0);         // 19 1780 max resources (2000)
            data << uint32(0x6f6) << uint32(0x0);           // 20 1782 blacksmith color
            data << uint32(0x6f7) << uint32(0x0);           // 21 1783 blacksmith (show/hide)
            data << uint32(0x6f8) << uint32(0x0);           // 22 1784 unk, bs?
            data << uint32(0x6f9) << uint32(0x0);           // 23 1785 unk, bs?
            data << uint32(0x6fb) << uint32(0x0);           // 24 1787 gold mine (0 - horde contr, 1 - alliance contr)
            data << uint32(0x6fc) << uint32(0x0);           // 25 1788 gold mine (0 - conflict, 1 - horde)
            data << uint32(0x6fd) << uint32(0x0);           // 26 1789 gold mine (1 - show/0 - hide)
            data << uint32(0x6fe) << uint32(0x0);           // 27 1790 gold mine color
            data << uint32(0x700) << uint32(0x0);           // 28 1792 gold mine color, wtf?, may be LM?
            data << uint32(0x701) << uint32(0x0);           // 29 1793 lumber mill color (0 - conflict, 1 - horde contr)
            data << uint32(0x702) << uint32(0x0);           // 30 1794 lumber mill (show/hide)
            data << uint32(0x703) << uint32(0x0);           // 31 1795 lumber mill color color
            data << uint32(0x732) << uint32(0x1);           // 32 1842 stables (1 - uncontrolled)
            data << uint32(0x733) << uint32(0x1);           // 33 1843 gold mine (1 - uncontrolled)
            data << uint32(0x734) << uint32(0x1);           // 34 1844 lumber mill (1 - uncontrolled)
            data << uint32(0x735) << uint32(0x1);           // 35 1845 farm (1 - uncontrolled)
            data << uint32(0x736) << uint32(0x1);           // 36 1846 blacksmith (1 - uncontrolled)
            data << uint32(0x745) << uint32(0x2);           // 37 1861 unk
            data << uint32(0x7a3) << uint32(0x708);         // 38 1955 warning limit (1800)
            break;
        case 3483:                                          // Hellfire Peninsula
            data << uint32(0x9ba) << uint32(0x1);           // 10
            data << uint32(0x9b9) << uint32(0x1);           // 11
            data << uint32(0x9b5) << uint32(0x0);           // 12
            data << uint32(0x9b4) << uint32(0x1);           // 13
            data << uint32(0x9b3) << uint32(0x0);           // 14
            data << uint32(0x9b2) << uint32(0x0);           // 15
            data << uint32(0x9b1) << uint32(0x1);           // 16
            data << uint32(0x9b0) << uint32(0x0);           // 17
            data << uint32(0x9ae) << uint32(0x0);           // 18 horde pvp objectives captured
            data << uint32(0x9ac) << uint32(0x0);           // 19
            data << uint32(0x9a8) << uint32(0x0);           // 20
            data << uint32(0x9a7) << uint32(0x0);           // 21
            data << uint32(0x9a6) << uint32(0x1);           // 22
        case 3519:                                          // Terokkar Forest
            data << uint32(0xa41) << uint32(0x0);           // 10
            data << uint32(0xa40) << uint32(0x14);          // 11
            data << uint32(0xa3f) << uint32(0x0);           // 12
            data << uint32(0xa3e) << uint32(0x0);           // 13
            data << uint32(0xa3d) << uint32(0x5);           // 14
            data << uint32(0xa3c) << uint32(0x0);           // 15
            data << uint32(0xa87) << uint32(0x0);           // 16
            data << uint32(0xa86) << uint32(0x0);           // 17
            data << uint32(0xa85) << uint32(0x0);           // 18
            data << uint32(0xa84) << uint32(0x0);           // 19
            data << uint32(0xa83) << uint32(0x0);           // 20
            data << uint32(0xa82) << uint32(0x0);           // 21
            data << uint32(0xa81) << uint32(0x0);           // 22
            data << uint32(0xa80) << uint32(0x0);           // 23
            data << uint32(0xa7e) << uint32(0x0);           // 24
            data << uint32(0xa7d) << uint32(0x0);           // 25
            data << uint32(0xa7c) << uint32(0x0);           // 26
            data << uint32(0xa7b) << uint32(0x0);           // 27
            data << uint32(0xa7a) << uint32(0x0);           // 28
            data << uint32(0xa79) << uint32(0x0);           // 29
            data << uint32(0x9d0) << uint32(0x5);           // 30
            data << uint32(0x9ce) << uint32(0x0);           // 31
            data << uint32(0x9cd) << uint32(0x0);           // 32
            data << uint32(0x9cc) << uint32(0x0);           // 33
            data << uint32(0xa88) << uint32(0x0);           // 34
            data << uint32(0xad0) << uint32(0x0);           // 35
            data << uint32(0xacf) << uint32(0x1);           // 36
        case 3521:                                          // Zangarmarsh
            data << uint32(0x9e1) << uint32(0x0);           // 10
            data << uint32(0x9e0) << uint32(0x0);           // 11
            data << uint32(0x9df) << uint32(0x0);           // 12
            data << uint32(0xa5d) << uint32(0x1);           // 13
            data << uint32(0xa5c) << uint32(0x0);           // 14
            data << uint32(0xa5b) << uint32(0x1);           // 15
            data << uint32(0xa5a) << uint32(0x0);           // 16
            data << uint32(0xa59) << uint32(0x1);           // 17
            data << uint32(0xa58) << uint32(0x0);           // 18
            data << uint32(0xa57) << uint32(0x0);           // 19
            data << uint32(0xa56) << uint32(0x0);           // 20
            data << uint32(0xa55) << uint32(0x1);           // 21
            data << uint32(0xa54) << uint32(0x0);           // 22
            data << uint32(0x9e7) << uint32(0x0);           // 23
            data << uint32(0x9e6) << uint32(0x0);           // 24
            data << uint32(0x9e5) << uint32(0x0);           // 25
            data << uint32(0xa00) << uint32(0x0);           // 26
            data << uint32(0x9ff) << uint32(0x1);           // 27
            data << uint32(0x9fe) << uint32(0x0);           // 28
            data << uint32(0x9fd) << uint32(0x0);           // 29
            data << uint32(0x9fc) << uint32(0x1);           // 30
            data << uint32(0x9fb) << uint32(0x0);           // 31
            data << uint32(0xa62) << uint32(0x0);           // 32
            data << uint32(0xa61) << uint32(0x1);           // 33
            data << uint32(0xa60) << uint32(0x1);           // 34
            data << uint32(0xa5f) << uint32(0x0);           // 35
        case 3703:                                          // Shattrath City
            break;
        default:
            data << uint32(0x914) << uint32(0x0);           // 7
            data << uint32(0x913) << uint32(0x0);           // 8
            data << uint32(0x912) << uint32(0x0);           // 9
            data << uint32(0x915) << uint32(0x0);           // 10
            break;
    }
    GetSession()->SendPacket(&data);
}

uint32 Player::GetXPRestBonus(uint32 xp)
{
    uint32 rested_bonus = (uint32)GetRestBonus();           //xp for each rested bonus

    if(rested_bonus > xp)                                   // max rested_bonus == xp or (r+x) = 200% xp
        rested_bonus = xp;

    SetRestBonus( GetRestBonus() - rested_bonus);

    sLog.outDetail("Player gain %u xp (+ %u Rested Bonus). Rested points=%f",xp+rested_bonus,rested_bonus,GetRestBonus());
    return rested_bonus;
}

int32 Player::FishingMinSkillForCurrentZone() const
{
    // special areas (subzones)
    switch(GetAreaId())
    {
        case 297:
            return 205;

        case 1112:
        case 1222:
        case 1227:
        case 3140:
            return 330;
    }

    // zones
    switch(GetZoneId())
    {
        case 1:
        case 12:
        case 14:
        case 85:
        case 141:
        case 215:
            return -70;

        case 17:
        case 38:
        case 40:
        case 130:
        case 148:
        case 206:
        case 718:
        case 719:
        case 1519:
        case 1581:
        case 1637:
        case 1638:
        case 1657:
            return -20;

        case 10:
        case 11:
        case 44:
        case 367:
        case 331:
        case 406:
            return 55;

        case 8:
        case 15:
        case 33:
        case 36:
        case 45:
        case 400:
        case 405:
        case 796:
            return 130;

        case 16:
        case 28:
        case 47:
        case 357:
        case 361:
        case 440:
        case 490:
        case 493:
        case 1477:
        case 2100:
            return 205;

        case 41:
        case 46:
        case 139:
        case 618:
        case 1377:
        case 1977:
        case 2017:
        case 2057:
            return 330;
    }

    // impossable or unknown
    return 9999;
}

void Player::SetBindPoint(uint64 guid)
{
    WorldPacket data(SMSG_BINDER_CONFIRM, 8);
    data << guid;
    GetSession()->SendPacket( &data );
}

void Player::SendTalentWipeConfirm(uint64 guid)
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, (8+4));
    data << guid;
    data << (uint32)resetTalentsCost();
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
    if(i < 2 && item)
    {
        if(!item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            return;
        uint32 charges = item->GetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT);
        if(charges == 0)
            return;
        if(charges > 1)
            item->SetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT,charges-1);
        else if(charges <= 1)
        {
            ApplyEnchantment(item,TEMP_ENCHANTMENT_SLOT,false);
            item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
        }
    }
}

void Player::SetSheath( uint32 sheathed )
{
    Item* item;
    switch (sheathed)
    {
        case 0:                                             // no prepeared weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
        case 1:                                             // prepeared melee weapon
        {
            item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            SetVirtualItemSlot(0,item && !item->IsBroken() && IsUseEquipedWeapon() ? item : NULL);
            item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            SetVirtualItemSlot(1,item && !item->IsBroken() && IsUseEquipedWeapon() ? item : NULL);
            SetVirtualItemSlot(2,NULL);
        };  break;
        case 2:                                             // prepeared ranged weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            SetVirtualItemSlot(2,item && !item->IsBroken() && IsUseEquipedWeapon() ? item : NULL);
            break;
        default:
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
    }
    SetUInt32Value(UNIT_FIELD_BYTES_2, 0x2800+sheathed);    // this must visualize Sheath changing for other players...
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
        {
            slots[0] = EQUIPMENT_SLOT_MAINHAND;

            // suggest offhand slot only if know dual wielding
            // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual weilding" ...
            if(CanDualWield())
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
        };break;
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
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
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
        case INVTYPE_RELIC:
            slots[0] = EQUIPMENT_SLOT_RANGED;
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
        // search empty slot at first
        for (int i = 0; i < 4; i++)
        {
            if ( slots[i] != NULL_SLOT && !GetItemByPos( INVENTORY_SLOT_BAG_0, slots[i] ) )
                return slots[i];
        }

        // if not found empty and can swap return first appropriate
        for (int i = 0; i < 4; i++)
        {
            if ( slots[i] != NULL_SLOT && swap )
                return slots[i];
        }
    }

    // no free position
    return NULL_SLOT;
}

Item* Player::CreateItem( uint32 item, uint32 count ) const
{
    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
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

uint32 Player::GetFreeSlots() const
{
    uint32 count = 0;
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( !pItem )
            ++count;
    }

    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
            count += pBag->GetFreeSlots();
    }

    return count;
}

uint32 Player::GetItemCount( uint32 item, Item* eItem ) const
{
    Item *pItem;
    uint32 count = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem != eItem &&  pItem->GetEntry() == item )
            count += pItem->GetCount();
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem != eItem && pItem->GetEntry() == item )
            count += pItem->GetCount();
    }
    Bag *pBag;
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
            count += pBag->GetItemCount(item,eItem);
    }

    if(eItem && eItem->GetProto()->GemProperties)
    {
        for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
            if( pItem && pItem != eItem && pItem->GetProto()->Socket[0].Color )
                count += pItem->GetGemCountWithID(item);
        }
    }

    return count;
}

uint32 Player::GetBankItemCount( uint32 item, Item* eItem ) const
{
    Item *pItem;
    uint32 count = 0;
    for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem != eItem && pItem->GetEntry() == item )
            count += pItem->GetCount();
    }
    Bag *pBag;
    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
            count += pBag->GetItemCount(item,eItem);
    }
    
    if(eItem && eItem->GetProto()->GemProperties)
    {
        for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
            if( pItem && pItem != eItem && pItem->GetProto()->Socket[0].Color )
                count += pItem->GetGemCountWithID(item);
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
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        pItem = GetItemByPos( pos );
        if( pItem && pItem->GetGUID() == guid )
            return pos;
    }
    Bag *pBag;
    ItemPrototype const *pBagProto;
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

    // In this case GUID is trade slot (enchanting fix)
    if (guid < TRADE_SLOT_COUNT && GetTrader())
    {
        Item *item = GetItemByPos(tradeItems[guid]);
        if (item)
            return GetPosByGuid(item->GetGUID());
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
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot < BANK_SLOT_BAG_END || slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END ) )
        return m_items[slot];
    else if(bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END
        || bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END )
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

bool Player::IsInventoryPos( uint8 bag, uint8 slot )
{
    if( bag == INVENTORY_SLOT_BAG_0 && slot == NULL_SLOT )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END ) )
        return true;
    if( bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END ) )
        return true;
    return false;
}

bool Player::IsEquipmentPos( uint8 bag, uint8 slot )
{
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot < EQUIPMENT_SLOT_END ) )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END ) )
        return true;
    return false;
}

bool Player::IsBankPos( uint8 bag, uint8 slot )
{
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END ) )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END ) )
        return true;
    if( bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END )
        return true;
    return false;
}

bool Player::IsBagPos( uint16 pos )
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END ) )
        return true;
    if( bag == INVENTORY_SLOT_BAG_0 && ( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END ) )
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
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
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
    ItemPrototype const *pBagProto;
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

uint8 Player::CanTakeMoreSimilarItems(Item* pItem) const
{
    ItemPrototype const *pProto = pItem->GetProto();
    if( !pProto )
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;

    // no maximum
    if(pProto->MaxCount == 0)
        return EQUIP_ERR_OK;

    uint32 curcount = GetItemCount(pProto->ItemId,pItem) + GetBankItemCount(pProto->ItemId,pItem);

    if( curcount + pItem->GetCount() > pProto->MaxCount )
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;

    return EQUIP_ERR_OK;
}

uint8 Player::CanStoreItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanStoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();
        if( pProto )
        {
            Item *pItem2;
            Bag *pBag;
            ItemPrototype const *pBagProto;
            uint16 pos;
            if(pItem->IsBindedNotWith(GetGUID()))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            uint8 res = CanTakeMoreSimilarItems(pItem);
            if(res != EQUIP_ERR_OK)
                return res;

            if( bag == NULL_BAG )
            {
                // search stack for merge to (ignore keyring - keys not merged)
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

                // search free slot - special bag case
                if( pProto->BagFamily != BAG_FAMILY_NONE )
                {
                    if(pProto->BagFamily == BAG_FAMILY_KEYS)
                    {
                        uint32 keyringSize = GetMaxKeyringSize();
                        for(uint32 j = KEYRING_SLOT_START; j < KEYRING_SLOT_START+keyringSize; j++)
                        {
                            pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, j );
                            if( !pItem2 )
                            {
                                dest = ( (INVENTORY_SLOT_BAG_0 << 8) | j );
                                return EQUIP_ERR_OK;
                            }
                        }
                    }

                    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();

                            // not plain container check
                            if( pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) &&
                                pItem->CanGoIntoBag(pBagProto) )
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

                // search free slot
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
                        if( pBagProto && pItem->CanGoIntoBag(pBagProto))
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
            else                                            // in specific bag
            {
                if( slot == NULL_SLOT )
                {
                    if( pProto->InventoryType == INVTYPE_BAG )
                    {
                        Bag *pBag = (Bag*)pItem;
                        if( pBag && !pBag->IsEmpty() )
                            return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
                    }

                    // search stack in bag for merge to (ignore keyring - keys not merged)
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
                        // search free slot - keyring case
                        if(pProto->BagFamily == BAG_FAMILY_KEYS)
                        {
                            uint32 keyringSize = GetMaxKeyringSize();
                            for(uint32 j = KEYRING_SLOT_START; j < KEYRING_SLOT_START+keyringSize; j++)
                            {
                                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, j );
                                if( !pItem2 )
                                {
                                    dest = ( (INVENTORY_SLOT_BAG_0 << 8) | j );
                                    return EQUIP_ERR_OK;
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
                                if( !pItem->CanGoIntoBag(pBagProto) )
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
                else                                        // specific bag and slot
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

                            // keyring case
                            if(slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_START+GetMaxKeyringSize() && pProto->BagFamily != BAG_FAMILY_KEYS)
                                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

                            // prevent cheating
                            if(slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END || slot >= PLAYER_SLOT_END)
                                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

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
                                    if( !pItem->CanGoIntoBag(pBagProto) )
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

//////////////////////////////////////////////////////////////////////////
uint8 Player::CanStoreItems( Item **pItems,int count) const
{
    Item    *pItem2;

    // fill space table
    int inv_slot_items[INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START];
    int inv_bags[INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START][MAX_BAG_SIZE];
    int inv_keys[KEYRING_SLOT_END-KEYRING_SLOT_START];

    memset(inv_slot_items,0,sizeof(int)*(INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START));
    memset(inv_bags,0,sizeof(int)*(INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START)*MAX_BAG_SIZE);
    memset(inv_keys,0,sizeof(int)*(KEYRING_SLOT_END-KEYRING_SLOT_START));

    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_slot_items[i-INVENTORY_SLOT_ITEM_START] = pItem2->GetCount();
        }
    }

    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag     *pBag;
        ItemPrototype const *pBagProto;

        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            pBagProto = pBag->GetProto();

            if( pBagProto )
            {
                for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                {
                    pItem2 = GetItemByPos( i, j );
                    if (pItem2 && !pItem2->IsInTrade())
                    {
                        inv_bags[i-INVENTORY_SLOT_BAG_START][j] = pItem2->GetCount();
                    }
                }
            }
        }
    }

    // check free space for all items
    for (int k=0;k<count;k++)
    {
        Item  *pItem = pItems[k];

        // no item
        if (!pItem)  continue;

        sLog.outDebug( "STORAGE: CanStoreItems %i. item = %u, count = %u", k+1, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();

        // strange item
        if( !pProto )
            return EQUIP_ERR_ITEM_NOT_FOUND;

        // item it 'bind'
        if(pItem->IsBindedNotWith(GetGUID()))
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;

        Bag *pBag;
        ItemPrototype const *pBagProto;

        // item is 'one item only'
        uint8 res = CanTakeMoreSimilarItems(pItem);
        if(res != EQUIP_ERR_OK)
            return res;

        // search stack for merge to (ignore keyring - keys not merged)
        if( pProto->Stackable > 1 )
        {
            bool b_found = false;
            for(int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
            {
                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_slot_items[t-INVENTORY_SLOT_ITEM_START] + pItem->GetCount() <= pProto->Stackable )
                {
                    inv_slot_items[t-INVENTORY_SLOT_ITEM_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pBag )
                {
                    pBagProto = pBag->GetProto();
                    if( pBagProto )
                    {
                        for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                        {
                            pItem2 = GetItemByPos( t, j );
                            if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_bags[t-INVENTORY_SLOT_BAG_START][j] + pItem->GetCount() <= pProto->Stackable )
                            {
                                inv_bags[t-INVENTORY_SLOT_BAG_START][j] += pItem->GetCount();
                                b_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // special bag case
        if( pProto->BagFamily != BAG_FAMILY_NONE )
        {
            bool b_found = false;
            if(pProto->BagFamily == BAG_FAMILY_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                for(uint32 t = KEYRING_SLOT_START; t < KEYRING_SLOT_START+keyringSize; ++t)
                {
                    if( inv_keys[t-KEYRING_SLOT_START] == 0 )
                    {
                        inv_keys[t-KEYRING_SLOT_START] = 1;
                        b_found = true;
                        break;
                    }
                }
            }

            if (b_found) continue;

            for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pBag )
                {
                    pBagProto = pBag->GetProto();

                    // not plain container check
                    if( pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) && pItem->CanGoIntoBag(pBagProto) )
                    {
                        for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                        {
                            if( inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0 )
                            {
                                inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                                b_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // search free slot
        bool b_found = false;
        for(int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
        {
            if( inv_slot_items[t-INVENTORY_SLOT_ITEM_START] == 0 )
            {
                inv_slot_items[t-INVENTORY_SLOT_ITEM_START] = 1;
                b_found = true;
                break;
            }
        }
        if (b_found) continue;

        // search free slot in bags
        for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
        {
            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
            if( pBag )
            {
                pBagProto = pBag->GetProto();
                if( pBagProto && pItem->CanGoIntoBag(pBagProto))
                {
                    for(uint32 j = 0; j < pBagProto->ContainerSlots; j++)
                    {
                        if( inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0 )
                        {
                            inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                            b_found = true;
                            break;
                        }
                    }
                }
            }
        }

        // no free slot found?
        if (!b_found)
            return EQUIP_ERR_INVENTORY_FULL;
    }

    return EQUIP_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
uint8 Player::CanEquipNewItem( uint8 slot, uint16 &dest, uint32 item, uint32 count, bool swap ) const
{
    dest = 0;
    Item *pItem = CreateItem( item, count );
    if( pItem )
    {
        uint8 result = CanEquipItem(slot, dest, pItem, swap );
        delete pItem;
        return result;
    }

    return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 Player::CanEquipItem( uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();
        if( pProto )
        {
            if(pItem->IsBindedNotWith(GetGUID()))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            uint8 res = CanTakeMoreSimilarItems(pItem);
            if(res != EQUIP_ERR_OK)
                return res;

            if( isInCombat()&& pProto->Class != ITEM_CLASS_WEAPON && pProto->Class != ITEM_CLASS_PROJECTILE &&
                pProto->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD && pProto->InventoryType != INVTYPE_RELIC)
                return EQUIP_ERR_CANT_DO_IN_COMBAT;

            if(isInCombat()&& pProto->Class == ITEM_CLASS_WEAPON && m_weaponChangeTimer != 0)
                return EQUIP_ERR_CANT_DO_RIGHT_NOW;         // maybe exist better err

            uint32 type = pProto->InventoryType;
            uint8 eslot = FindEquipSlot( type, slot, swap );
            if( eslot == NULL_SLOT )
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            uint8 msg = CanUseItem( pItem , not_loading );
            if( msg != EQUIP_ERR_OK )
                return msg;
            if( !swap && GetItemByPos( INVENTORY_SLOT_BAG_0, eslot ) )
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;

            if(eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                if( type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND )
                {
                    if(!CanDualWield())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }

                Item *mainItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND );
                if(mainItem)
                {
                    if(mainItem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                        return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
                }
                else if(type != INVTYPE_HOLDABLE)
                {
                    // not let equip offhand non-holdable item if mainhand not equipped
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
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

uint8 Player::CanUnequipItem( uint16 pos, bool swap ) const
{
    // Applied only to equipped items and bank bags
    if(!IsEquipmentPos(pos) && !IsBagPos(pos))
        return EQUIP_ERR_OK;

    Item* pItem = GetItemByPos(pos);

    // Applied only to existed equipped item
    if( !pItem )
        return EQUIP_ERR_OK;

    sLog.outDebug( "STORAGE: CanUnequipItem slot = %u, item = %u, count = %u", pos, pItem->GetEntry(), pItem->GetCount());

    ItemPrototype const *pProto = pItem->GetProto();
    if( !pProto )
        return EQUIP_ERR_ITEM_NOT_FOUND;

    if( isInCombat()&& pProto->Class != ITEM_CLASS_WEAPON && pProto->Class != ITEM_CLASS_PROJECTILE &&
        pProto->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD && pProto->InventoryType != INVTYPE_RELIC )
        return EQUIP_ERR_CANT_DO_IN_COMBAT;

    if(!swap && pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    // All equiped items can swapped (not in combat case)
    if(swap)
        return EQUIP_ERR_OK;

    uint8 slot = pos & 255;

    // can't unequip mainhand item if offhand item equiped (weapon or shield)
    if(slot == EQUIPMENT_SLOT_MAINHAND)
    {
        Item * offhand = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if(offhand)
        {
            ItemPrototype const *offProto = offhand->GetProto();
            if(offProto && (offProto->Class == ITEM_CLASS_WEAPON || offProto->InventoryType == INVTYPE_SHIELD))
                return EQUIP_ERR_CANT_DO_RIGHT_NOW;
        }
    }

    return EQUIP_ERR_OK;
}

uint8 Player::CanBankItem( uint8 bag, uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading ) const
{
    dest = 0;
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanBankItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();
        if( pProto )
        {
            Item *pItem2;
            Bag *pBag;
            ItemPrototype const *pBagProto;
            uint16 pos;
            if( pItem->IsBindedNotWith(GetGUID()) )
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            uint8 res = CanTakeMoreSimilarItems(pItem);
            if(res != EQUIP_ERR_OK)
                return res;

            if( bag == NULL_BAG )
            {
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

                // search place in special bag
                if( pProto->BagFamily != BAG_FAMILY_NONE )
                {
                    for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
                    {
                        pos = ( (INVENTORY_SLOT_BAG_0 << 8) | i );
                        pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
                        if( pBag )
                        {
                            pBagProto = pBag->GetProto();
                            // not plain container check
                            if( pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) &&
                                pItem->CanGoIntoBag(pBagProto) )
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

                // search free space
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
                        if( pBagProto && pItem->CanGoIntoBag(pBagProto) )
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
                                if( !pItem->CanGoIntoBag(pBagProto) )
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
                                if( uint8 cantuse = CanUseItem( pItem, not_loading ) != EQUIP_ERR_OK )
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
                                    if( !pItem->CanGoIntoBag(pBagProto) )
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

uint8 Player::CanUseItem( Item *pItem, bool not_loading ) const
{
    if( pItem )
    {
        sLog.outDebug( "STORAGE: CanUseItem item = %u", pItem->GetEntry());
        if( !isAlive() && not_loading )
            return EQUIP_ERR_YOU_ARE_DEAD;
        //if( isStunned() )
        //    return EQUIP_ERR_YOU_ARE_STUNNED;
        ItemPrototype const *pProto = pItem->GetProto();
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
            if( pProto->RequiredReputationFaction && GetReputationRank(pProto->RequiredReputationFaction) < pProto->RequiredReputationRank )
                return EQUIP_ITEM_REPUTATION_NOT_ENOUGH;
            if( getLevel() < pProto->RequiredLevel )
                return EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;
            return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

bool Player::CanUseItem( ItemPrototype const *pProto )
{
    // Used by group, function NeedBeforeGreed, to know if a prototype can be used by a player

    if( pProto )
    {
        if( (pProto->AllowableClass & getClassMask()) == 0 || (pProto->AllowableRace & getRaceMask()) == 0 )
            return false;
        if( pProto->RequiredSkill != 0  )
        {
            if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                return false;
            else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                return false;
        }
        if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
            return false;
        if( getLevel() < pProto->RequiredLevel )
            return false;
        return true;
    }
    return false;
}

uint8 Player::CanUseAmmo( uint32 item ) const
{
    sLog.outDebug( "STORAGE: CanUseAmmo item = %u", item);
    if( !isAlive() )
        return EQUIP_ERR_YOU_ARE_DEAD;
    //if( isStunned() )
    //    return EQUIP_ERR_YOU_ARE_STUNNED;
    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        if( pProto->InventoryType!= INVTYPE_AMMO )
            return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
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
        /*if( GetReputation() < pProto->RequiredReputation )
        return EQUIP_ITEM_REPUTATION_NOT_ENOUGH;
        */
        if( getLevel() < pProto->RequiredLevel )
            return EQUIP_ERR_YOU_MUST_REACH_LEVEL_N;
        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

void Player::SetAmmo( uint32 item )
{
    // already set
    if( GetUInt32Value(PLAYER_AMMO_ID) == item )
        return;

    // check ammo
    if(item)
    {
        uint8 msg = CanUseAmmo( item );
        if( msg != EQUIP_ERR_OK )
        {
            SendEquipError( msg, NULL, NULL );
            return;
        }
    }

    _RemoveAllItemMods();
    SetUInt32Value(PLAYER_AMMO_ID, item);
    _ApplyAllItemMods();
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::StoreNewItem( uint16 pos, uint32 item, uint32 count, bool update ,uint32 randomPropertyId )
{
    Item *pItem = CreateItem( item, count );
    if( pItem )
    {
        ItemPrototype const *pProto = pItem->GetProto();
        ItemAddedQuestCheck( item, count );
        if(randomPropertyId)
            pItem->SetItemRandomProperties(randomPropertyId);
        Item * retItem = StoreItem( pos, pItem, update );

        return retItem;
    }
    return NULL;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::StoreItem( uint16 pos, Item *pItem, bool update )
{
    if( pItem )
    {
        if( pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetProto()->Class == ITEM_CLASS_QUEST)
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
                pItem->SetContainer( NULL );

                if( IsInWorld() && update )
                {
                    pItem->AddToWorld();
                    pItem->SendUpdateToPlayer( this );
                }

                pItem->SetState(ITEM_CHANGED, this);
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
                    pItem->SetState(ITEM_CHANGED, this);
                    pBag->SetState(ITEM_CHANGED, this);
                }
            }

            AddEnchantmentDurations(pItem);
        }
        else
        {
            pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
            if( IsInWorld() && update )
                pItem2->SendUpdateToPlayer( this );

            // delete item (it not in any slot currently)
            //pItem->DeleteFromDB();
            if( IsInWorld() && update )
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer( this );
            }

            pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
            pItem2->SetState(ITEM_CHANGED, this);

            RemoveEnchantmentDurations(pItem);
            AddEnchantmentDurations(pItem2);
            return pItem2;
        }
    }

    return pItem;
}

Item* Player::EquipNewItem( uint16 pos, uint32 item, uint32 count, bool update )
{
    Item *pItem = CreateItem( item, count );
    if( pItem )
    {
        ItemPrototype const *pProto = pItem->GetProto();
        ItemAddedQuestCheck( item, count );
        Item * retItem = EquipItem( pos, pItem, update );

        return retItem;
    }
    return NULL;
}

Item* Player::EquipItem( uint16 pos, Item *pItem, bool update )
{
    if( pItem )
    {
        AddEnchantmentDurations(pItem);

        uint8 bag = pos >> 8;
        uint8 slot = pos & 255;

        Item *pItem2 = GetItemByPos( bag, slot );

        if( !pItem2 )
        {
            VisualizeItem( pos, pItem);
            uint8 slot = pos & 255;

            if(isAlive())
            {
                _ApplyItemMods(pItem, slot, true);

                ItemPrototype const *pProto = pItem->GetProto();
                if(pProto && isInCombat()&& pProto->Class == ITEM_CLASS_WEAPON && m_weaponChangeTimer == 0)
                {
                    m_weaponChangeTimer = DEFAULT_SWITCH_WEAPON;
                    if (getClass() == CLASS_ROGUE)
                        m_weaponChangeTimer = ROGUE_SWITCH_WEAPON;
                }
            }

            if( IsInWorld() && update )
            {
                pItem->AddToWorld();
                pItem->SendUpdateToPlayer( this );
            }
        }
        else
        {
            pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
            if( IsInWorld() && update )
                pItem2->SendUpdateToPlayer( this );

            // delete item (it not in any slot currently)
            //pItem->DeleteFromDB();
            if( IsInWorld() && update )
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer( this );
            }

            pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
            pItem2->SetState(ITEM_CHANGED, this);

            RemoveEnchantmentDurations(pItem);
            AddEnchantmentDurations(pItem2);
            return pItem2;
        }
    }
    return pItem;
}

void Player::QuickEquipItem( uint16 pos, Item *pItem)
{
    if( pItem )
    {
        VisualizeItem( pos, pItem);

        if( IsInWorld() )
        {
            pItem->AddToWorld();
            pItem->SendUpdateToPlayer( this );
        }
    }
}

void Player::VisualizeItem( uint16 pos, Item *pItem)
{
    if(!pItem)
        return;

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
    pItem->SetContainer( NULL );

    if( slot < EQUIPMENT_SLOT_END )
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
        SetUInt32Value(VisibleBase, pItem->GetEntry());

        for(int i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, pItem->GetEnchantmentId(EnchantmentSlot(i)));

        SetUInt32Value(VisibleBase + 8, pItem->GetItemRandomPropertyId());
    }

    pItem->SetState(ITEM_CHANGED, this);
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::BankItem( uint16 pos, Item *pItem, bool update )
{
    return StoreItem( pos, pItem, update);
}

void Player::RemoveItem( uint8 bag, uint8 slot, bool update )
{
    // note: removeitem does not actualy change the item
    // it only takes the item out of storage temporarily
    // note2: if removeitem is to be used for delinking
    // the item must be removed from the player's updatequeue

    Item *pItem = GetItemByPos( bag, slot );
    if( pItem )
    {
        sLog.outDebug( "STORAGE: RemoveItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        RemoveEnchantmentDurations(pItem);

        if( bag == INVENTORY_SLOT_BAG_0 )
        {
            if ( slot < INVENTORY_SLOT_BAG_END )
            {
                _ApplyItemMods(pItem, slot, false);

                // and remove held enchantments
                if ( slot == EQUIPMENT_SLOT_MAINHAND )
                {
                    pItem->ClearEnchantment(HELD_PERM_ENCHANTMENT_SLOT);
                    pItem->ClearEnchantment(HELD_TEMP_ENCHANTMENT_SLOT);
                }
            }


            m_items[slot] = NULL;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if ( slot < EQUIPMENT_SLOT_END )
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
                for (int i = VisibleBase; i < VisibleBase + 12; ++i)
                    SetUInt32Value(i, 0);
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

                if(remcount >=count)
                    return;
            }
            else
            {
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() && update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                RemoveItem( INVENTORY_SLOT_BAG_0, i, update );

                if(remcount >=count)
                    return;
            }
            else
            {
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() && update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
    Bag *pBag;
    ItemPrototype const *pBagProto;
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

                            if(remcount >=count)
                                return;
                        }
                        else
                        {
                            pItem->SetCount( pItem->GetCount() - count + remcount );
                            if( IsInWorld() && update )
                                pItem->SendUpdateToPlayer( this );
                            pItem->SetState(ITEM_CHANGED, this);
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

        if(pItem->HasFlag(ITEM_FIELD_FLAGS, 8))
            sDatabase.Execute("DELETE FROM `character_gifts` WHERE `item_guid` = '%u'", pItem->GetGUIDLow());

        //pItem->SetOwnerGUID(0);
        pItem->SetSlot( NULL_SLOT );
        pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, 0 );
        ItemPrototype const *pProto = pItem->GetProto();

        RemoveEnchantmentDurations(pItem);

        if( bag == INVENTORY_SLOT_BAG_0 )
        {
            ItemRemovedQuestCheck( pItem->GetEntry(), pItem->GetCount() );

            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if ( slot < EQUIPMENT_SLOT_END )
            {
                _ApplyItemMods(pItem, slot, false);
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
                for (int i = VisibleBase; i < VisibleBase + 12; ++i)
                    SetUInt32Value(i, 0);
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
                    ItemRemovedQuestCheck( pItem->GetEntry(), pItem->GetCount() );

                pBag->RemoveItem(slot, update);

                if( IsInWorld() && update )
                {
                    pItem->RemoveFromWorld();
                    pItem->DestroyForPlayer(this);
                }
            }
        }

        if (pItem->IsBag())
        {
            for (int i = 0; i < MAX_BAG_SIZE; i++)
            {
                Item *bagItem = ((Bag*)pItem)->GetItemByPos(i);
                if (bagItem) bagItem->SetState(ITEM_REMOVED, this);
            }
        }
        pItem->SetState(ITEM_REMOVED, this);
    }
}

void Player::DestroyItemCount( uint32 item, uint32 count, bool update )
{
    sLog.outDebug( "STORAGE: DestroyItemCount item = %u, count = %u", item, count);
    Item *pItem;
    ItemPrototype const *pProto;
    uint32 remcount = 0;

    // in inventory
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                if(remcount >=count)
                    return;
            }
            else
            {
                pProto = pItem->GetProto();
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                if(remcount >=count)
                    return;
            }
            else
            {
                pProto = pItem->GetProto();
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }

    // in inventory bags
    Bag *pBag;
    ItemPrototype const *pBagProto;
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

                            if(remcount >=count)
                                return;
                        }
                        else
                        {
                            pProto = pItem->GetProto();
                            ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                            pItem->SetCount( pItem->GetCount() - count + remcount );
                            if( IsInWorld() && update )
                                pItem->SendUpdateToPlayer( this );
                            pItem->SetState(ITEM_CHANGED, this);
                            return;
                        }
                    }
                }
            }
        }
    }

    // in equipment and bag list
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                if(remcount >=count)
                    return;
            }
            else
            {
                pProto = pItem->GetProto();
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
}

void Player::DestroyItemCount( Item* pItem, uint32 &count, bool update )
{
    if(!pItem)
        return;

    sLog.outDebug( "STORAGE: DestroyItemCount item (GUID: %u, Entry: %u) count = %u", pItem->GetGUIDLow(),pItem->GetEntry(), count);

    if( pItem->GetCount() <= count )
    {
        count-= pItem->GetCount();

        uint16 pos = GetPosByGuid(pItem->GetGUID());
        DestroyItem( (pos >> 8),(pos & 255), update);
    }
    else
    {
        ItemPrototype const* pProto  = pItem->GetProto();
        ItemRemovedQuestCheck( pItem->GetEntry(), count);
        pItem->SetCount( pItem->GetCount() - count );
        count = 0;
        if( IsInWorld() & update )
            pItem->SendUpdateToPlayer( this );
        pItem->SetState(ITEM_CHANGED, this);
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
        // not let split all items (can be only at cheating)
        if(pSrcItem->GetCount() == count)
        {
            SendEquipError( EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL );
            return;
        }

        // not let split more existed items (can be only at cheating)
        if(pSrcItem->GetCount() < count)
        {
            SendEquipError( EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT, pSrcItem, NULL );
            return;
        }

        sLog.outDebug( "STORAGE: SplitItem bag = %u, slot = %u, item = %u, count = %u", dstbag, dstslot, pSrcItem->GetEntry(), count);
        Item *pNewItem = CreateItem( pSrcItem->GetEntry(), count );
        if( pNewItem )
        {
            uint16 dest;
            uint8 msg;
            if( IsInventoryPos( dst ) )
            {
                // change item amount before check (for unique max count check)
                pSrcItem->SetCount( pSrcItem->GetCount() - count );
                msg = CanStoreItem( dstbag, dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    Item *pDstItem = GetItemByPos(dstbag, dstslot);
                    if (!pDstItem || pDstItem && pDstItem->GetCount() + pSrcItem->GetCount() <= pSrcItem->GetProto()->Stackable)
                    {
                        if( IsInWorld() )
                            pSrcItem->SendUpdateToPlayer( this );
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        StoreItem( dest, pNewItem, true);
                    }
                    else
                    {
                        delete pNewItem;
                        pSrcItem->SetCount( pSrcItem->GetCount() + count );
                        SendEquipError( EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL );
                    }
                }
                else
                {
                    delete pNewItem;
                    pSrcItem->SetCount( pSrcItem->GetCount() + count );
                    SendEquipError( msg, pSrcItem, NULL );
                }
            }
            else if( IsBankPos ( dst ) )
            {
                // change item amount before check (for unique max count check)
                pSrcItem->SetCount( pSrcItem->GetCount() - count );
                msg = CanBankItem( dstbag, dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    Item *pDstItem = GetItemByPos(dstbag, dstslot);
                    if (!pDstItem || pDstItem && pDstItem->GetCount() + pSrcItem->GetCount() <= pSrcItem->GetProto()->Stackable)
                    {
                        if( IsInWorld() )
                            pSrcItem->SendUpdateToPlayer( this );
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        BankItem( dest, pNewItem, true);
                    }
                    else
                    {
                        delete pNewItem;
                        pSrcItem->SetCount( pSrcItem->GetCount() + count );
                        SendEquipError( EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL );
                    }
                }
                else
                {
                    delete pNewItem;
                    pSrcItem->SetCount( pSrcItem->GetCount() + count );
                    SendEquipError( msg, pSrcItem, NULL );
                }
            }
            else if( IsEquipmentPos ( dst ) )
            {
                // change item amount before check (for unique max count check)
                pSrcItem->SetCount( pSrcItem->GetCount() + count );
                msg = CanEquipItem( dstslot, dest, pNewItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    Item *pDstItem = GetItemByPos(dstbag, dstslot);
                    if (!pDstItem || pDstItem && pDstItem->GetCount() + pSrcItem->GetCount() <= pSrcItem->GetProto()->Stackable)
                    {
                        if( IsInWorld() )
                            pSrcItem->SendUpdateToPlayer( this );
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        EquipItem( dest, pNewItem, true);
                    }
                    else
                    {
                        delete pNewItem;
                        pSrcItem->SetCount( pSrcItem->GetCount() + count );
                        SendEquipError( EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL );
                    }
                }
                else
                {
                    delete pNewItem;
                    pSrcItem->SetCount( pSrcItem->GetCount() + count );
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

        // check unequip posability for equipped items and bank bags
        if(IsEquipmentPos ( src ) || IsBagPos ( src ))
        {
            // bags can be swapped with empty bag slots
            uint8 msg = CanUnequipItem( src, pDstItem != NULL || IsBagPos ( src ) && IsBagPos ( dst ));
            if(msg != EQUIP_ERR_OK)
            {
                SendEquipError( msg, pSrcItem, pDstItem );
                return;
            }

        }

        // prevent put equipped/bank bag in self
        if( IsBagPos ( src ) && srcslot == dstbag)
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
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        pDstItem->SetState(ITEM_CHANGED, this);
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
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        pDstItem->SetState(ITEM_CHANGED, this);
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
                        pSrcItem->SetState(ITEM_CHANGED, this);
                        pDstItem->SetState(ITEM_CHANGED, this);
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
            {
                msg = CanEquipItem( dstslot, dest, pSrcItem, true );
                if( msg == EQUIP_ERR_OK )
                    msg = CanUnequipItem( dest, true );
            }

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
                    if( msg == EQUIP_ERR_OK )
                        msg = CanUnequipItem( dest2, true);

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

void Player::AddItemToBuyBackSlot( Item *pItem )
{
    if( pItem )
    {
        uint32 slot = m_currentBuybackSlot;
        // if current back slot non-empty search oldest or free
        if(m_items[slot])
        {
            uint32 oldest_time = GetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 );
            uint32 oldest_slot = BUYBACK_SLOT_START;

            for(uint32 i = BUYBACK_SLOT_START+1; i < BUYBACK_SLOT_END; ++i )
            {
                // found empty
                if(!m_items[i])
                {
                    slot = i;
                    break;
                }

                uint32 i_time = GetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + i - BUYBACK_SLOT_START);

                if(oldest_time > i_time)
                {
                    oldest_time = i_time;
                    oldest_slot = i;
                }
            }

            // find oldest
            slot = oldest_slot;
        }

        RemoveItemFromBuyBackSlot( slot, true );
        sLog.outDebug( "STORAGE: AddItemToBuyBackSlot item = %u, slot = %u", pItem->GetEntry(), slot);

        m_items[slot] = pItem;
        time_t base = time(NULL);
        uint32 etime = uint32(base - m_logintime + (30 * 3600));
        uint32 eslot = slot - BUYBACK_SLOT_START;

        SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, pItem->GetGUID() );
        ItemPrototype const *pProto = pItem->GetProto();
        if( pProto )
            SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, pProto->SellPrice * pItem->GetCount() );
        else
            SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0 );
        SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, (uint32)etime );

        // move to next (for non filled list is move most optimized choice)
        if(m_currentBuybackSlot < BUYBACK_SLOT_END-1)
            ++m_currentBuybackSlot;
    }
}

Item* Player::GetItemFromBuyBackSlot( uint32 slot )
{
    sLog.outDebug( "STORAGE: GetItemFromBuyBackSlot slot = %u", slot);
    if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
        return m_items[slot];
    return NULL;
}

void Player::RemoveItemFromBuyBackSlot( uint32 slot, bool del )
{
    sLog.outDebug( "STORAGE: RemoveItemFromBuyBackSlot slot = %u", slot);
    if( slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END )
    {
        Item *pItem = m_items[slot];
        if( pItem )
        {
            pItem->RemoveFromWorld();
            if(del) pItem->SetState(ITEM_REMOVED, this);
        }

        m_items[slot] = NULL;

        uint32 eslot = slot - BUYBACK_SLOT_START;
        SetUInt64Value( PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, 0 );
        SetUInt32Value( PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0 );
        SetUInt32Value( PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, 0 );

        // if current backslot is filled set to now free slot
        if(m_items[m_currentBuybackSlot])
            m_currentBuybackSlot = slot;
    }
}

void Player::SendEquipError( uint8 msg, Item* pItem, Item *pItem2 )
{
    sLog.outDetail( "WORLD: Sent SMSG_INVENTORY_CHANGE_FAILURE" );
    WorldPacket data( SMSG_INVENTORY_CHANGE_FAILURE, ((msg == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N)?22:18) );
    data << msg;
    if( msg == EQUIP_ERR_YOU_MUST_REACH_LEVEL_N )
        data << (pItem && pItem->GetProto() ? pItem->GetProto()->RequiredLevel : uint32(0));
    data << (pItem ? pItem->GetGUID() : uint64(0));
    data << (pItem2 ? pItem2->GetGUID() : uint64(0));
    data << uint8(0);                                       // not 0 there...
    GetSession()->SendPacket(&data);
}

void Player::SendBuyError( uint8 msg, Creature* pCreature, uint32 item, uint32 param )
{
    sLog.outDetail( "WORLD: Sent SMSG_BUY_FAILED" );
    WorldPacket data( SMSG_BUY_FAILED, (8+4+4+1) );
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
    WorldPacket data( SMSG_SELL_ITEM, (8+4+4+1) );          // last check 2.0.10
    data << (pCreature ? pCreature->GetGUID() : uint64(0));
    data << guid;
    if( param > 0 )
        data << param;
    data << msg;
    GetSession()->SendPacket(&data);
}

void Player::ClearTrade()
{
    tradeGold = 0;
    acceptTrade = false;
    for(int i = 0; i < TRADE_SLOT_COUNT; i++)
        tradeItems[i] = NULL_SLOT;
}

void Player::TradeCancel(bool sendback)
{
    if(pTrader)
    {
        // prevent loop cancel message (already processed)
        if(!sendback)
            pTrader->pTrader = NULL;

        WorldSession* ws = pTrader->GetSession();
        pTrader = NULL;
        ws->SendCancelTrade();
    }
    ClearTrade();
}

void Player::UpdateEnchantTime(uint32 time)
{
    for(EnchantDurationList::iterator itr = m_enchantDuration.begin(),next;itr != m_enchantDuration.end();itr=next)
    {
        assert(itr->item);
        next=itr;
        if(!itr->item->GetEnchantmentId(itr->slot))
        {
            next = m_enchantDuration.erase(itr);
        }
        else if(itr->leftduration <= time)
        {
            ApplyEnchantment(itr->item,itr->slot,false,false);
            itr->item->ClearEnchantment(itr->slot);
            next = m_enchantDuration.erase(itr);
        }
        else if(itr->leftduration > time)
        {
            itr->leftduration -= time;
            ++next;
        }
    }
}

void Player::AddEnchantmentDurations(Item *item)
{
    for(int x=0;x<MAX_ENCHANTMENT_SLOT;++x)
    {
        uint32 duration = item->GetEnchantmentDuration(EnchantmentSlot(x));
        if( duration == 0 )
            continue;
        else if( duration > 0 )
            AddEnchantmentDuration(item,EnchantmentSlot(x+1),duration);
    }
}

void Player::RemoveEnchantmentDurations(Item *item)
{
    for(EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();)
    {
        if(itr->item == item)
        {
            // save duration in item
            item->SetEnchantmentDuration(EnchantmentSlot(itr->slot),itr->leftduration);
            itr = m_enchantDuration.erase(itr);
        }
        else
            ++itr;
    }
}

// duration == 0 will remove item enchant
void Player::AddEnchantmentDuration(Item *item,EnchantmentSlot slot,uint32 duration)
{
    if(!item)
        return;

    if(slot >= MAX_ENCHANTMENT_SLOT)
        return;

    for(EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();++itr)
    {
        if(itr->item == item && itr->slot == slot)
        {
            m_enchantDuration.erase(itr);
            break;
        }
    }
    if(item && duration > 0 )
    {
        GetSession()->SendItemEnchantTimeUpdate(GetGUID(), item->GetGUID(),slot,uint32(duration/1000));
        m_enchantDuration.push_back(EnchantDuration(item,slot,duration));
    }
}

void Player::ApplyEnchantment(Item *item,bool apply)
{
    for(uint32 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
        ApplyEnchantment(item, EnchantmentSlot(slot), apply);
}

void Player::ApplyEnchantment(Item *item,EnchantmentSlot slot,bool apply, bool apply_dur, bool ignore_condition)
{
    if(!item)
        return;

    if(!item->IsEquipped())
        return;

    if(slot > MAX_ENCHANTMENT_SLOT)
        return;

    uint32 enchant_id = item->GetEnchantmentId(slot);
    if(!enchant_id)
        return;

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
        return;

    if(!ignore_condition && pEnchant->EnchantmentCondition && !((Player*)this)->EnchantmentFitsRequirements(pEnchant->EnchantmentCondition, -1))
        return;

    for (int s=0; s<3; s++)
    {
        uint32 enchant_display_type = pEnchant->display_type[s];
        uint32 enchant_amount = pEnchant->amount[s];
        uint32 enchant_spell_id = pEnchant->spellid[s];

        if (enchant_display_type == 0)
        {
            // Nothing
        }
        else if(enchant_display_type ==4)
        {
            ApplyArmorMod(enchant_amount,apply);
        }
        else if(enchant_display_type == 6) // Shaman Rockbiter Weapon
        {
            // enchant_amount is then containing the number of damage per second to add to the weapon
            if(getClass() == CLASS_SHAMAN)
            {
                ApplyModFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,enchant_amount,apply);
                //ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,enchant_amount,apply);
            }
        }
        else if(enchant_display_type == 5) // 
        {
            sLog.outDebug("Adding %u to stat nb %u",enchant_amount,enchant_spell_id);
            switch (enchant_spell_id)
            {
            case ITEM_STAT_AGILITY:
                sLog.outDebug("+ %u AGILITY",enchant_amount);
                ApplyPosStatMod(STAT_AGILITY, enchant_amount, apply);
                ApplyStatMod(STAT_AGILITY, enchant_amount, apply);
                break;
            case ITEM_STAT_STRENGTH:
                sLog.outDebug("+ %u STRENGTH",enchant_amount);
                ApplyPosStatMod(STAT_STRENGTH, enchant_amount, apply);
                ApplyStatMod(STAT_STRENGTH, enchant_amount, apply);
                break;
            case ITEM_STAT_INTELLECT:
                sLog.outDebug("+ %u INTELLECT",enchant_amount);
                ApplyPosStatMod(STAT_INTELLECT, enchant_amount, apply);
                ApplyStatMod(STAT_INTELLECT, enchant_amount, apply);
                break;
            case ITEM_STAT_SPIRIT:
                sLog.outDebug("+ %u SPIRIT",enchant_amount);
                ApplyPosStatMod(STAT_SPIRIT, enchant_amount, apply);
                ApplyStatMod(STAT_SPIRIT, enchant_amount, apply);
                break;
            case ITEM_STAT_STAMINA:
                sLog.outDebug("+ %u STAMINA",enchant_amount);
                ApplyPosStatMod(STAT_STAMINA, enchant_amount, apply);
                ApplyStatMod(STAT_STAMINA, enchant_amount, apply);
                break;
            case ITEM_STAT_DEFENCE_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_DEFENCE_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u DEFENCE", enchant_amount);
                break;
            case ITEM_STAT_DODGE_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_DODGE_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u DODGE", enchant_amount);
                break;
            case ITEM_STAT_PARRY_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_PARRY_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u PARRY", enchant_amount);
                break;
            case ITEM_STAT_SHIELD_BLOCK_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_BLOCK_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u SHIELD_BLOCK", enchant_amount);
                break;
            case ITEM_STAT_MELEE_HIT_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_MELEE_HIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u MELEE_HIT", enchant_amount);
                break;
            case ITEM_STAT_RANGED_HIT_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_RANGED_HIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u RANGED_HIT", enchant_amount);
                break;
            case ITEM_STAT_SPELL_HIT_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_SPELL_HIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u SPELL_HIT", enchant_amount);
                break;
            case ITEM_STAT_MELEE_CS_RATING: // CS = Critical Strike
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_MELEE_CRIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u MELEE_CRIT", enchant_amount);
                break;
            case ITEM_STAT_RANGED_CS_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_RANGED_CRIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u RANGED_CRIT", enchant_amount);
                break;
            case ITEM_STAT_SPELL_CS_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_SPELL_CRIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u SPELL_CRIT", enchant_amount);
                break;
                // Values from ITEM_STAT_MELEE_HA_RATING to ITEM_STAT_SPELL_HASTE_RATING are never used
                // in Enchantments
            case ITEM_STAT_HIT_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_HIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u HIT", enchant_amount);
                break;
            case ITEM_STAT_CS_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_CRIT_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u CRITICAL", enchant_amount);
                break;
                // Values ITEM_STAT_HA_RATING and ITEM_STAT_CA_RATING are never used in Enchantment
            case ITEM_STAT_RESILIENCE_RATING:
                ((Player*)this)->ApplyRatingMod(PLAYER_FIELD_RESILIENCE_RATING, enchant_amount, apply);
                sLog.outDebug("+ %u RESILIENCE", enchant_amount);
                break;
                // Value ITEM_STAT_HASTE_RATING is never used in Enchantment
            }
        }
        else if(enchant_display_type ==2)
        {
            if(getClass() == CLASS_HUNTER)
            {
                ApplyModFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,enchant_amount,apply);
                ApplyModFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,enchant_amount,apply);
            }
            else
            {
                ApplyModFloatValue(UNIT_FIELD_MINDAMAGE,enchant_amount,apply);
                ApplyModFloatValue(UNIT_FIELD_MAXDAMAGE,enchant_amount,apply);
            }
        }

        if(enchant_spell_id)
        {
            if(apply)
            {
                if(enchant_display_type == 3)
                    CastSpell(this,enchant_spell_id,true, NULL);
            }
            else
                RemoveAurasDueToSpell(enchant_spell_id);
        }

    }

    // visualize enchantment at player and equipped items
    int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * 16);
    SetUInt32Value(VisibleBase+1 + slot +1, apply? item->GetEnchantmentId(slot) : 0);


    if(apply_dur)
    {
        if(apply)
        {
            // set duration
            uint32 duration = item->GetEnchantmentDuration(slot);
            if(duration)
                AddEnchantmentDuration(item,TEMP_ENCHANTMENT_SLOT,duration);
        }
        else
        {
            // duration == 0 will remove EnchantDuration
            AddEnchantmentDuration(item,slot,0);
        }
    }
}

void Player::ReducePoisonCharges(uint32 enchantId)
{
    if(!enchantId)
        return;
    uint32 pEnchantId = 0;
    uint32 charges = 0;
    Item *pItem;
    uint16 pos;

    for(int i = EQUIPMENT_SLOT_MAINHAND; i < EQUIPMENT_SLOT_RANGED; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);

        pItem = GetItemByPos( pos );
        if(!pItem)
            continue;
        for(int x=0;x<MAX_ENCHANTMENT_SLOT;x++)
        {
            charges = pItem->GetEnchantmentCharges(EnchantmentSlot(x));
            if(charges == 0)
                continue;
            if(charges <= 1)
            {
                ApplyEnchantment(pItem,EnchantmentSlot(x),false);
                pItem->ClearEnchantment(EnchantmentSlot(x));
                break;
            }
            else
            {
                pItem->SetEnchantmentCharges(EnchantmentSlot(x),charges-1);
                break;
            }
        }
    }
}

void Player::SaveEnchant()
{
    for(EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();++itr)
    {
        assert(itr->item);
        if(itr->leftduration > 0)
            itr->item->SetEnchantmentDuration(itr->slot,itr->leftduration);
    }
}

void Player::LoadEnchant()
{
    uint32 duration = 0;
    Item *pItem;
    uint16 pos;

    // ignore keyring, keys can't be enchanted
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);

        pItem = GetItemByPos( pos );
        if(!pItem)
            continue;
        if(pItem->GetProto()->Class != ITEM_CLASS_WEAPON && pItem->GetProto()->Class != ITEM_CLASS_ARMOR)
            continue;

        AddEnchantmentDurations(pItem);
    }
    Bag *pBag;
    ItemPrototype const *pBagProto;
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
                    if(!pItem)
                        continue;
                    if(pItem->GetProto()->Class != ITEM_CLASS_WEAPON && pItem->GetProto()->Class != ITEM_CLASS_ARMOR)
                        continue;
                    AddEnchantmentDurations(pItem);
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
                    if(!pItem)
                        continue;
                    if(pItem->GetProto()->Class != ITEM_CLASS_WEAPON && pItem->GetProto()->Class != ITEM_CLASS_ARMOR)
                        continue;
                    AddEnchantmentDurations(pItem);
                }
            }
        }
    }
}

void Player::SendNewItem(Item *item, uint32 count, bool received, bool created, bool broadcast)
{
    if(!item)                                               // prevent crash
        return;

                                                            // last check 2.0.10
    WorldPacket data( SMSG_ITEM_PUSH_RESULT, (8+4+4+4+1+4+4+4+4+4) );
    data << GetGUID();                                      // player GUID
    data << uint32(received);                               // 0=looted, 1=from npc
    data << uint32(created);                                // 0=received, 1=created
    data << uint32(1);                                      // always 0x01 (propably meant to be count of listed items)
    data << (uint8)item->GetBagSlot();                      // bagslot
                                                            // item slot, but when added to stack: 0xFFFFFFFF
    data << (uint32) ((item->GetCount()==count) ? item->GetSlot() : -1);
    data << uint32(item->GetEntry());                       // item id
    data << (uint32)urand(0, 255);                          // 0 when bought from npc otherwise ???
    data << uint32(item->GetItemRandomPropertyId());
    data << uint32(count);                                  // count of items
    data << GetItemCount(item->GetEntry());                 // count of items in inventory

    if (broadcast && groupInfo.group)
        groupInfo.group->BroadcastPacket(&data);
    else
        GetSession()->SendPacket(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void Player::PrepareQuestMenu( uint64 guid )
{
    Object *pObject;
    QuestRelations* pObjectQR;
    QuestRelations* pObjectQIR;
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
    if( pCreature )
    {
        pObject = (Object*)pCreature;
        pObjectQR  = &objmgr.mCreatureQuestRelations;
        pObjectQIR = &objmgr.mCreatureQuestInvolvedRelations;
    }
    else
    {
        GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*this, guid);
        if( pGameObject )
        {
            pObject = (Object*)pGameObject;
            pObjectQR  = &objmgr.mGOQuestRelations;
            pObjectQIR = &objmgr.mGOQuestInvolvedRelations;
        }
        else
            return;
    }

    QuestMenu *qm = PlayerTalkClass->GetQuestMenu();
    qm->ClearMenu();

    for(QuestRelations::const_iterator i = pObjectQIR->lower_bound(pObject->GetEntry()); i != pObjectQIR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        QuestStatus status = GetQuestStatus( quest_id );
        if ( status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus( quest_id ) )
            qm->AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if ( status == QUEST_STATUS_INCOMPLETE )
            qm->AddMenuItem(quest_id, DIALOG_STATUS_INCOMPLETE);
        else if (status == QUEST_STATUS_AVAILABLE )
            qm->AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
    }

    for(QuestRelations::const_iterator i = pObjectQR->lower_bound(pObject->GetEntry()); i != pObjectQR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        Quest* pQuest = objmgr.QuestTemplates[quest_id];
        if(!pQuest) continue;

        QuestStatus status = GetQuestStatus( quest_id );

        if (pQuest->IsAutoComplete() && CanTakeQuest(pQuest, false))
            qm->AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if ( status == QUEST_STATUS_NONE && CanTakeQuest( pQuest, false ) )
            qm->AddMenuItem(quest_id, DIALOG_STATUS_AVAILABLE);
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
        // Auto open -- maybe also should verify there is no greeting
        uint32 quest_id = pQuestMenu->GetItem(0).m_qId;
        Quest *pQuest = objmgr.QuestTemplates[quest_id];
        if ( pQuest )
        {
            if( status == DIALOG_STATUS_REWARD_REP && !GetQuestRewardStatus( quest_id ) )
                PlayerTalkClass->SendQuestGiverRequestItems( pQuest, guid, CanRewardQuest(pQuest,false), true );
            else if( status == DIALOG_STATUS_INCOMPLETE )
                PlayerTalkClass->SendQuestGiverRequestItems( pQuest, guid, false, true );
            else
                PlayerTalkClass->SendQuestGiverQuestDetails( pQuest, guid, true );
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
                qe._Delay = 0;                              //TEXTEMOTE_MESSAGE;              //zyg: player emote
                qe._Emote = 0;                              //TEXTEMOTE_HELLO;                //zyg: NPC emote
                title = "";
            }
            else
            {
                qe = gossiptext->Options[0].Emotes[0];
                title = gossiptext->Options[0].Text_0;
                if( &title == NULL )
                    title = "";
            }
        }
        PlayerTalkClass->SendQuestGiverQuestList( qe, title, guid );
    }
}

Quest *Player::GetActiveQuest( uint32 quest_id ) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);

    return itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE ?  itr->second.m_quest : NULL;
}

Quest* Player::GetNextQuest( uint64 guid, Quest *pQuest )
{
    if( pQuest )
    {
        Object *pObject;
        QuestRelations* pObjectQR;
        QuestRelations* pObjectQIR;

        Creature *pCreature = ObjectAccessor::Instance().GetCreature(*this, guid);
        if( pCreature )
        {
            pObject = (Object*)pCreature;
            pObjectQR  = &objmgr.mCreatureQuestRelations;
            pObjectQIR = &objmgr.mCreatureQuestInvolvedRelations;
        }
        else
        {
            GameObject *pGameObject = ObjectAccessor::Instance().GetGameObject(*this, guid);
            if( pGameObject )
            {
                pObject = (Object*)pGameObject;
                pObjectQR  = &objmgr.mGOQuestRelations;
                pObjectQIR = &objmgr.mGOQuestInvolvedRelations;
            }
            else
                return NULL;
        }

        uint32 nextQuestID = pQuest->GetNextQuestInChain();
        for(QuestRelations::const_iterator itr = pObjectQR->lower_bound(pObject->GetEntry()); itr != pObjectQR->upper_bound(pObject->GetEntry()); ++itr)
        {
            if (itr->second == nextQuestID)
                return objmgr.QuestTemplates[nextQuestID];
        }
    }
    return NULL;
}

bool Player::CanSeeStartQuest( uint32 quest_id )
{
    if( quest_id )
    {
        if( SatisfyQuestRace( quest_id, false ) && SatisfyQuestClass( quest_id, false ) && SatisfyQuestExclusiveGroup( quest_id, false )
            && SatisfyQuestSkill( quest_id, false ) && SatisfyQuestReputation( quest_id, false )
            && SatisfyQuestPreviousQuest( quest_id, false ) && SatisfyQuestNextChain( quest_id, false ) && SatisfyQuestPrevChain( quest_id, false ) )
            return ( getLevel() + 7 >= objmgr.QuestTemplates[quest_id]->GetMinLevel() );
    }
    return false;
}

bool Player::CanTakeQuest( Quest *pQuest, bool msg )
{
    if( pQuest)
    {
        uint32 quest_id = pQuest->GetQuestId();
        return ( SatisfyQuestStatus( quest_id, msg ) && SatisfyQuestExclusiveGroup( quest_id, msg )
            && SatisfyQuestRace( quest_id, msg ) && SatisfyQuestLevel( quest_id, msg ) && SatisfyQuestClass( quest_id, msg )
            && SatisfyQuestSkill( quest_id, msg ) && SatisfyQuestReputation( quest_id, msg )
            && SatisfyQuestPreviousQuest( quest_id, msg ) && SatisfyQuestTimed( quest_id, msg )
            && SatisfyQuestNextChain( quest_id, msg ) && SatisfyQuestPrevChain( quest_id, msg ) );
    }
    return false;
}

bool Player::CanAddQuest( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        if( !SatisfyQuestLog( msg ) )
            return false;

        uint32 srcitem = pQuest->GetSrcItemId();
        if( srcitem > 0 )
        {
            uint32 count = pQuest->GetSrcItemCount();
            uint16 dest;
            if( count <= 0 )
                count = 1;
            uint8 msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, srcitem, count, false );

            // player already have max number (in most case 1) source item, no additional item needed and quest can be added.
            if( msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS )
                return true;
            else if( msg != EQUIP_ERR_OK )
            {
                SendEquipError( msg, NULL, NULL );
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Player::CanCompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        QuestStatus qStatus = mQuestStatus[quest_id].m_status;
        if( qStatus == QUEST_STATUS_COMPLETE )
            return true;

        Quest* qInfo = objmgr.QuestTemplates[quest_id];

        if(!qInfo)
            return false;

        // auto complete quest
        if (qInfo->IsAutoComplete() && CanTakeQuest(qInfo, false))
            return true;

        if ( mQuestStatus[quest_id].m_status == QUEST_STATUS_INCOMPLETE )
        {

            if ( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( qInfo->ReqItemCount[i]!= 0 && mQuestStatus[quest_id].m_itemcount[i] < qInfo->ReqItemCount[i] )
                        return false;
                }
            }

            if ( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL_OR_CAST ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( qInfo->ReqCreatureOrGOId[i] == 0 )
                        continue;

                    if( qInfo->ReqCreatureOrGOCount[i] != 0 && mQuestStatus[quest_id].m_creatureOrGOcount[i] < qInfo->ReqCreatureOrGOCount[i] )
                        return false;
                }
            }

            if ( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_EXPLORATION ) && !mQuestStatus[quest_id].m_explored )
                return false;

            if ( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) && mQuestStatus[quest_id].m_timer == 0 )
                return false;

            if ( qInfo->GetRewOrReqMoney() < 0 )
            {
                if ( GetMoney() < uint32(-qInfo->GetRewOrReqMoney()) )
                    return false;
            }

            return true;
        }
    }
    return false;
}

bool Player::CanRewardQuest( Quest *pQuest, bool msg )
{
    if( pQuest )
    {
        // not auto complete quest and not completed quest (only cheating case, then ignore without message)
        if(!pQuest->IsAutoComplete() && GetQuestStatus(pQuest->GetQuestId()) != QUEST_STATUS_COMPLETE)
            return false;

        // rewarded and not repeatable quest (only cheating case, then ignore without message)
        if(GetQuestRewardStatus(pQuest->GetQuestId()))
            return false;

        // prevent receive reward with quest items in bank
        if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
        {
            for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            {
                if( pQuest->ReqItemCount[i]!= 0 &&
                    GetItemCount(pQuest->ReqItemId[i]) < pQuest->ReqItemCount[i] )
                {
                    if(msg)
                        SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
                    return false;
                }
            }
        }

        return true;
    }
    return false;
}

bool Player::CanRewardQuest( Quest *pQuest, uint32 reward, bool msg )
{
    if( pQuest )
    {
        // prevent receive reward with quest items in bank or for not completed quest
        if(!CanRewardQuest(pQuest,msg))
            return false;

        uint16 dest;
        uint8 msg;

        if ( pQuest->GetRewChoiceItemsCount() > 0 )
        {
            if( pQuest->RewChoiceItemId[reward] )
            {
                msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewChoiceItemId[reward], pQuest->RewChoiceItemCount[reward], false );
                if( msg != EQUIP_ERR_OK )
                {
                    SendEquipError( msg, NULL, NULL );
                    return false;
                }
            }
        }

        if ( pQuest->GetRewItemsCount() > 0 )
        {
            for (int i = 0; i < pQuest->GetRewItemsCount(); i++)
            {
                if( pQuest->RewItemId[i] )
                {
                    msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewItemId[i], pQuest->RewItemCount[i], false );
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
        uint16 log_slot = GetQuestSlot( 0 );
        assert(log_slot);

        uint32 quest_id = pQuest->GetQuestId();

        // check for repeatable quests status
        QuestUpdateState uState;
        if (mQuestStatus.find(quest_id)==mQuestStatus.end())
        {
            uState = QUEST_NEW;
            mQuestStatus[quest_id].m_rewarded = false;
        }
        else
        {
            if (mQuestStatus[quest_id].uState != QUEST_NEW)
                uState = QUEST_CHANGED;
            else
                uState = QUEST_NEW;
        }

        mQuestStatus[quest_id].m_quest = pQuest;
        mQuestStatus[quest_id].m_status = QUEST_STATUS_INCOMPLETE;
        mQuestStatus[quest_id].m_explored = false;

        mQuestStatus[quest_id].uState = uState;             // mark quest as new or changed

        if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
        {
            for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                mQuestStatus[quest_id].m_itemcount[i] = 0;
        }
        if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL_OR_CAST ) )
        {
            for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                mQuestStatus[quest_id].m_creatureOrGOcount[i] = 0;
        }

        GiveQuestSourceItem( quest_id );
        AdjustQuestReqItemCount( quest_id );

        SetUInt32Value(log_slot + 0, quest_id);
        SetUInt32Value(log_slot + 1, 0);

        if( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
        {
            uint32 limittime = pQuest->GetLimitTime();
            AddTimedQuest( quest_id );
            mQuestStatus[quest_id].m_timer = limittime * 1000;
            uint32 qtime = static_cast<uint32>(time(NULL)) + limittime;
            SetUInt32Value( log_slot + 2, qtime );
        }
        else
        {
            mQuestStatus[quest_id].m_timer = 0;
            SetUInt32Value( log_slot + 2, 0 );
        }
    }
}

void Player::CompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        SetQuestStatus( quest_id, QUEST_STATUS_COMPLETE);

        uint16 log_slot = GetQuestSlot( quest_id );
        if( log_slot )
        {
            uint32 state = GetUInt32Value( log_slot + 1 );
            state |= 1 << 24;
            SetUInt32Value( log_slot + 1, state );
        }

        SendQuestComplete( quest_id );
    }
}

void Player::IncompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        SetQuestStatus( quest_id, QUEST_STATUS_INCOMPLETE );

        uint16 log_slot = GetQuestSlot( quest_id );
        if( log_slot )
        {
            uint32 state = GetUInt32Value( log_slot + 1 );
            state &= ~(1 << 24);
            SetUInt32Value( log_slot + 1, state );
        }
    }
}

void Player::RewardQuest( Quest *pQuest, uint32 reward, Object* questGiver )
{
    if( pQuest )
    {
        uint32 quest_id = pQuest->GetQuestId();

        uint16 dest;
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++ )
        {
            if ( pQuest->ReqItemId[i] )
                DestroyItemCount( pQuest->ReqItemId[i], pQuest->ReqItemCount[i], true);
        }

        //if( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
        //    SetTimedQuest( 0 );
        m_timedquests.erase(pQuest->GetQuestId());

        Item * item;
        if ( pQuest->GetRewChoiceItemsCount() > 0 )
        {
            if( pQuest->RewChoiceItemId[reward] )
            {
                if( CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewChoiceItemId[reward], pQuest->RewChoiceItemCount[reward], false ) == EQUIP_ERR_OK )
                {
                    item = StoreNewItem( dest, pQuest->RewChoiceItemId[reward], pQuest->RewChoiceItemCount[reward], true);
                    SendNewItem(item, pQuest->RewChoiceItemCount[reward], true, false);
                }
            }
        }

        if ( pQuest->GetRewItemsCount() > 0 )
        {
            for (int i=0; i < pQuest->GetRewItemsCount(); i++)
            {
                if( pQuest->RewItemId[i] )
                {
                    if( CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewItemId[i], pQuest->RewItemCount[i], false ) == EQUIP_ERR_OK )
                    {
                        item = StoreNewItem( dest, pQuest->RewItemId[i], pQuest->RewItemCount[i], true);
                        SendNewItem(item, pQuest->RewItemCount[i], true, false);
                    }
                }
            }
        }

        if( pQuest->GetRewSpell() > 0 )
            CastSpell( this, pQuest->GetRewSpell(), true);

        uint16 log_slot = GetQuestSlot( quest_id );
        if( log_slot )
        {
            SetUInt32Value(log_slot + 0, 0);
            SetUInt32Value(log_slot + 1, 0);
            SetUInt32Value(log_slot + 2, 0);
        }

        // Not give XP in case already completed once repeatable quest
        uint32 XP = mQuestStatus[quest_id].m_rewarded ? 0 : uint32(pQuest->XPValue( this )*sWorld.getRate(RATE_XP_QUEST));

        if ( getLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
            GiveXP( XP , NULL );
        else
            ModifyMoney( MaNGOS::XP::xp_to_money(XP) );

        ModifyMoney( pQuest->GetRewOrReqMoney() );

        if ( !pQuest->IsRepeatable() )
            SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);
        else
            SetQuestStatus(quest_id, QUEST_STATUS_NONE);

        mQuestStatus[quest_id].m_rewarded = true;
        SendQuestReward( pQuest, XP, questGiver );
        if (mQuestStatus[quest_id].uState != QUEST_NEW) mQuestStatus[quest_id].uState = QUEST_CHANGED;
    }
}

void Player::FailQuest( uint32 quest_id )
{
    if( quest_id )
    {
        IncompleteQuest( quest_id );

        uint16 log_slot = GetQuestSlot( quest_id );
        if( log_slot )
        {
            SetUInt32Value( log_slot + 2, 1 );

            uint32 state = GetUInt32Value( log_slot + 1 );
            state |= 1 << 25;
            SetUInt32Value( log_slot + 1, state );
        }
        SendQuestFailed( quest_id );
    }
}

void Player::FailTimedQuest( uint32 quest_id )
{
    if( quest_id )
    {
        if (mQuestStatus[quest_id].uState != QUEST_NEW) mQuestStatus[quest_id].uState = QUEST_CHANGED;
        mQuestStatus[quest_id].m_timer = 0;

        IncompleteQuest( quest_id );

        uint16 log_slot = GetQuestSlot( quest_id );
        if( log_slot )
        {
            SetUInt32Value( log_slot + 2, 1 );

            uint32 state = GetUInt32Value( log_slot + 1 );
            state |= 1 << 25;
            SetUInt32Value( log_slot + 1, state );
        }
        SendQuestTimerFailed( quest_id );
    }
}

bool Player::SatisfyQuestClass( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        int32 zoneOrSort = qInfo->GetZoneOrSort();

        // skip zone case
        if ( zoneOrSort >= 0 )
            return true;

        int32 questSort = -zoneOrSort;

        uint8 reqClass = ClassByQuestSort(questSort);

        if(reqClass != 0 && getClass() != reqClass)
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }

        return true;
    }
    return false;
}

bool Player::SatisfyQuestLevel( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if( getLevel() < qInfo->GetMinLevel() )
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
    if( GetQuestSlot(0) )
        return true;
    else
    {
        if( msg )
        {
            WorldPacket data( SMSG_QUESTLOG_FULL, 0 );
            GetSession()->SendPacket( &data );
            sLog.outDebug( "WORLD: Sent QUEST_LOG_FULL_MESSAGE" );
        }
        return false;
    }
}

bool Player::SatisfyQuestPreviousQuest( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        // No previous quest (might be first quest in a series)
        if( qInfo->prevQuests.size() == 0 )
            return true;

        for(vector<int32>::iterator iter = qInfo->prevQuests.begin(); iter != qInfo->prevQuests.end(); ++iter )
        {
            uint32 prevId = abs(*iter);

            QuestStatusMap::iterator i_prevstatus = mQuestStatus.find( prevId );

            if( i_prevstatus != mQuestStatus.end() )
            {
                // If any of the positive previous quests completed, return true
                if( *iter > 0 && i_prevstatus->second.m_rewarded )
                    return true;
                // If any of the negative previous quests active, return true
                if( *iter < 0 && (i_prevstatus->second.m_status == QUEST_STATUS_INCOMPLETE
                    || (i_prevstatus->second.m_status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId))))
                    return true;
            }
        }

        // Has only positive prev. quests in non-rewarded state
        // and negative prev. quests in non-active state
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
    }
    return false;
}

bool Player::SatisfyQuestRace( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        uint32 reqraces = qInfo->GetRequiredRaces();
        if ( reqraces == 0 )
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

bool Player::SatisfyQuestReputation( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        uint32 faction_id = qInfo->GetRequiredRepFaction();
        if(!faction_id)
            return true;

        return GetReputation(faction_id) >= qInfo->GetRequiredRepValue();
    }
    return false;
}

bool Player::SatisfyQuestSkill( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        int32 zoneOrSort = qInfo->GetZoneOrSort();

        // skip zone case
        if ( zoneOrSort >= 0 )
            return true;

        int32 questSort = -zoneOrSort;

        uint8 reqskill = SkillByQuestSort(questSort);

        if( reqskill != 0 && GetSkillValue( reqskill ) < qInfo->GetRequiredSkillValue() )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestStatus( uint32 quest_id, bool msg )
{
    if( quest_id )
    {
        QuestStatusMap::iterator itr = mQuestStatus.find( quest_id );
        if  ( itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_HAVE_QUEST );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestTimed( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if ( (find(m_timedquests.begin(), m_timedquests.end(), quest_id) != m_timedquests.end()) && qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED) )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_HAVE_TIMED_QUEST );
            return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestExclusiveGroup( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if(!qInfo->GetExclusiveGroup())
            return true;

        multimap<uint32, uint32>::iterator iter = objmgr.ExclusiveQuestGroups.lower_bound(qInfo->GetExclusiveGroup());
        multimap<uint32, uint32>::iterator end  = objmgr.ExclusiveQuestGroups.upper_bound(qInfo->GetExclusiveGroup());

        assert(iter!=end);                                  // always must be found if qInfo->ExclusiveGroup != 0

        for(; iter != end; ++iter)
        {
            uint32 exclude_Id = iter->second;

            // skip checked quest id, only state of other quests in group is interesting
            if(exclude_Id == quest_id)
                continue;

            QuestStatusMap::iterator i_exstatus = mQuestStatus.find( exclude_Id );

            // alternative quest already started or completed
            if( i_exstatus != mQuestStatus.end()
                && (i_exstatus->second.m_status == QUEST_STATUS_COMPLETE || i_exstatus->second.m_status == QUEST_STATUS_INCOMPLETE) )
                return false;
        }
        return true;
    }
    return false;
}

bool Player::SatisfyQuestNextChain( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if(!qInfo->GetNextQuestInChain())
            return true;

        // next quest in chain already started or completed
        QuestStatusMap::iterator itr = mQuestStatus.find( qInfo->GetNextQuestInChain() );
        if( itr != mQuestStatus.end()
            && (itr->second.m_status == QUEST_STATUS_COMPLETE || itr->second.m_status == QUEST_STATUS_INCOMPLETE) )
            return false;

        // check for all quests further up the chain
        // only necessary if there are quest chains with more than one quest that can be skipped
        //return SatisfyQuestNextChain( qInfo->GetNextQuestInChain(), msg );
        return true;
    }
    return false;
}

bool Player::SatisfyQuestPrevChain( uint32 quest_id, bool msg )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        // No previous quest in chain
        if( qInfo->prevChainQuests.size() == 0 )
            return true;

        for(vector<uint32>::iterator iter = qInfo->prevChainQuests.begin(); iter != qInfo->prevChainQuests.end(); ++iter )
        {
            uint32 prevId = *iter;

            QuestStatusMap::iterator i_prevstatus = mQuestStatus.find( prevId );

            if( i_prevstatus != mQuestStatus.end() )
            {
                // If any of the previous quests in chain active, return false
                if( i_prevstatus->second.m_status == QUEST_STATUS_INCOMPLETE
                    || (i_prevstatus->second.m_status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId)))
                    return false;
            }

            // check for all quests further down the chain
            // only necessary if there are quest chains with more than one quest that can be skipped
            //if( !SatisfyQuestPrevChain( prevId, msg ) )
            //    return false;
        }

        // No previous quest in chain active
        return true;
    }
    return false;
}

bool Player::GiveQuestSourceItem( uint32 quest_id )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        uint32 srcitem = qInfo->GetSrcItemId();
        if( srcitem > 0 )
        {
            uint16 dest;
            uint32 count = qInfo->GetSrcItemCount();
            if( count <= 0 )
                count = 1;
            uint8 msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, srcitem, count, false );
            if( msg == EQUIP_ERR_OK )
            {
                Item * item = StoreNewItem(dest, srcitem, count, true);
                SendNewItem(item, count, true, false);
                return true;
            }
            // player already have max amount required item, just report success
            else if( msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS )
                return true;
            else
                SendEquipError( msg, NULL, NULL );
            return false;
        }
    }
    return true;
}

void Player::TakeQuestSourceItem( uint32 quest_id )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        uint32 srcitem = qInfo->GetSrcItemId();
        if( srcitem > 0 )
        {
            uint32 count = qInfo->GetSrcItemCount();
            if( count <= 0 )
                count = 1;
            DestroyItemCount(srcitem, count, true);
        }
    }
}

bool Player::GetQuestRewardStatus( uint32 quest_id )
{
    Quest* qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        // for repeatable quests: rewarded field is set after first reward only to prevent getting XP more than once
        QuestStatusMap::iterator itr = mQuestStatus.find( quest_id );
        if( itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE
            && !qInfo->IsRepeatable() )
            return mQuestStatus[quest_id].m_rewarded;

        return false;
    }
    return false;
}

QuestStatus Player::GetQuestStatus( uint32 quest_id )
{
    if( quest_id )
    {
        if( mQuestStatus.find( quest_id ) != mQuestStatus.end() )
            return mQuestStatus[quest_id].m_status;
    }
    return QUEST_STATUS_NONE;
}

bool Player::CanShareQuest(uint32 quest_id)
{
    if( quest_id )
    {
        if( mQuestStatus.find( quest_id ) != mQuestStatus.end() )
            return mQuestStatus[quest_id].m_status == QUEST_STATUS_NONE
                || mQuestStatus[quest_id].m_status == QUEST_STATUS_INCOMPLETE;
    }

    return false;
}

void Player::SetQuestStatus( uint32 quest_id, QuestStatus status )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if( status == QUEST_STATUS_NONE || status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE )
        {
            if( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) )
                m_timedquests.erase(qInfo->GetQuestId());
        }

        mQuestStatus[quest_id].m_status = status;
        if (mQuestStatus[quest_id].uState != QUEST_NEW) mQuestStatus[quest_id].uState = QUEST_CHANGED;
    }
}

void Player::AdjustQuestReqItemCount( uint32 quest_id )
{
    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        if ( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
        {
            for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            {
                uint32 reqitemcount = qInfo->ReqItemCount[i];
                if( reqitemcount != 0 )
                {
                    uint32 curitemcount = GetItemCount(qInfo->ReqItemId[i]) + GetBankItemCount(qInfo->ReqItemId[i]);
                    mQuestStatus[quest_id].m_itemcount[i] = min(curitemcount, reqitemcount);
                    if (mQuestStatus[quest_id].uState != QUEST_NEW) mQuestStatus[quest_id].uState = QUEST_CHANGED;
                }
            }
        }
    }
}

uint16 Player::GetQuestSlot( uint32 quest_id )
{
    for ( uint16 i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        if ( GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i) == quest_id )
            return PLAYER_QUEST_LOG_1_1 + 3*i;
    }
    return 0;
}

void Player::AreaExplored( uint32 questId )
{
    if( questId )
    {
        uint16 log_slot = GetQuestSlot( questId );
        if( log_slot )
        {
            mQuestStatus[questId].m_explored = true;
            if (mQuestStatus[questId].uState != QUEST_NEW) mQuestStatus[questId].uState = QUEST_CHANGED;
        }
        if( CanCompleteQuest( questId ) )
            CompleteQuest( questId );
    }
}

void Player::ItemAddedQuestCheck( uint32 entry, uint32 count )
{
    uint32 questid;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 additemcount;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        questid = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        if ( questid != 0 && mQuestStatus[questid].m_status == QUEST_STATUS_INCOMPLETE )
        {
            Quest * qInfo = objmgr.QuestTemplates[questid];
            if( qInfo && qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    reqitem = qInfo->ReqItemId[j];
                    if ( reqitem == entry )
                    {
                        reqitemcount = qInfo->ReqItemCount[j];
                        curitemcount = mQuestStatus[questid].m_itemcount[j];
                        if ( curitemcount < reqitemcount )
                        {
                            additemcount = ( curitemcount + count <= reqitemcount ? count : reqitemcount - curitemcount);
                            mQuestStatus[questid].m_itemcount[j] += additemcount;
                            if (mQuestStatus[questid].uState != QUEST_NEW) mQuestStatus[questid].uState = QUEST_CHANGED;

                            SendQuestUpdateAddItem( questid, j, additemcount );
                        }
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );
                        return;
                    }
                }
            }
        }
    }
}

void Player::ItemRemovedQuestCheck( uint32 entry, uint32 count )
{
    uint32 questid;
    uint32 reqitem;
    uint32 reqitemcount;
    uint32 curitemcount;
    uint32 remitemcount;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        questid = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        Quest * qInfo = objmgr.QuestTemplates[questid];
        if ( qInfo )
        {
            if( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_DELIVER ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    reqitem = qInfo->ReqItemId[j];
                    if ( reqitem == entry )
                    {
                        reqitemcount = qInfo->ReqItemCount[j];
                        if( mQuestStatus[questid].m_status != QUEST_STATUS_COMPLETE )
                            curitemcount = mQuestStatus[questid].m_itemcount[j];
                        else
                            curitemcount = GetItemCount(entry) + GetBankItemCount(entry);
                        if ( curitemcount < reqitemcount + count )
                        {
                            remitemcount = ( curitemcount <= reqitemcount ? count : count + reqitemcount - curitemcount);
                            mQuestStatus[questid].m_itemcount[j] = curitemcount - remitemcount;
                            if (mQuestStatus[questid].uState != QUEST_NEW) mQuestStatus[questid].uState = QUEST_CHANGED;

                            IncompleteQuest( questid );
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
    uint32 questid;
    uint32 reqkill;
    uint32 reqkillcount;
    uint32 curkillcount;
    uint32 addkillcount = 1;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        questid = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);

        if(!questid)
            continue;

        Quest * qInfo = objmgr.QuestTemplates[questid];
        // just if !ingroup || !noraidgroup || raidgroup
        if ( qInfo && mQuestStatus[questid].m_status == QUEST_STATUS_INCOMPLETE && (!groupInfo.group || !groupInfo.group->isRaidGroup() || qInfo->GetType() == 62))
        {
            if( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL_OR_CAST ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip GO activate objective or none
                    if(qInfo->ReqCreatureOrGOId[j] <=0)
                        continue;

                    // skip Cast at creature objective
                    if(qInfo->ReqSpell[j] !=0 )
                        continue;

                    reqkill = qInfo->ReqCreatureOrGOId[j];

                    if ( reqkill == entry )
                    {
                        reqkillcount = qInfo->ReqCreatureOrGOCount[j];
                        curkillcount = mQuestStatus[questid].m_creatureOrGOcount[j];
                        if ( curkillcount < reqkillcount )
                        {
                            mQuestStatus[questid].m_creatureOrGOcount[j] = curkillcount + addkillcount;
                            if (mQuestStatus[questid].uState != QUEST_NEW) mQuestStatus[questid].uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreature( questid, guid, j, curkillcount, addkillcount);
                        }
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );
                        return;
                    }
                }
            }
        }
    }
}

void Player::CastedCreatureOrGO( uint32 entry, uint64 guid, uint32 spell_id )
{
    uint32 questid;
    uint32 reqCastCount;
    uint32 curCastCount;
    uint32 addCastCount = 1;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        questid = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);

        if(!questid)
            continue;

        Quest * qInfo = objmgr.QuestTemplates[questid];
        if ( qInfo && mQuestStatus[questid].m_status == QUEST_STATUS_INCOMPLETE )
        {
            if( qInfo->HasSpecialFlag( QUEST_SPECIAL_FLAGS_KILL_OR_CAST ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip kill creature objective (0) or wrong spell casts
                    if(qInfo->ReqSpell[j] != spell_id )
                        continue;

                    uint32 reqTarget = 0;
                    // GO activate objective
                    if(qInfo->ReqCreatureOrGOId[j] < 0)
                    {
                        // checked at quest_template loading
                        reqTarget = - qInfo->ReqCreatureOrGOId[j];
                    }
                    // creature acivate objectives
                    else if(qInfo->ReqCreatureOrGOId[j] > 0)
                    {
                        // checked at quest_template loading
                        reqTarget = qInfo->ReqCreatureOrGOId[j];
                    }
                    // other not creature/GO related obejctives
                    else
                        continue;

                    if ( reqTarget == entry )
                    {
                        reqCastCount = qInfo->ReqCreatureOrGOCount[j];
                        curCastCount = mQuestStatus[questid].m_creatureOrGOcount[j];
                        if ( curCastCount < reqCastCount )
                        {
                            mQuestStatus[questid].m_creatureOrGOcount[j] = curCastCount + addCastCount;
                            if (mQuestStatus[questid].uState != QUEST_NEW) mQuestStatus[questid].uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreature( questid, guid, j, curCastCount, addCastCount);
                        }
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );
                        return;
                    }
                }
            }
        }
    }
}

void Player::MoneyChanged( uint32 count )
{
    uint32 questid;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        questid = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i);
        if ( questid != 0 )
        {
            Quest * qInfo = objmgr.QuestTemplates[questid];
            if( qInfo && qInfo->GetRewOrReqMoney() < 0 )
            {
                if( mQuestStatus[questid].m_status == QUEST_STATUS_INCOMPLETE )
                {
                    if(int32(count) >= -qInfo->GetRewOrReqMoney())
                    {
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );
                    }
                }
                else if( mQuestStatus[questid].m_status == QUEST_STATUS_COMPLETE )
                {
                    if(int32(count) < -qInfo->GetRewOrReqMoney())
                        IncompleteQuest( questid );
                }
            }
        }
    }
}

bool Player::HasQuestForItem( uint32 itemid )
{
    for( QuestStatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++i )
    {
        quest_status qs=i->second;

        if (qs.m_status == QUEST_STATUS_INCOMPLETE)
        {
            if (!qs.m_quest) continue;
            Quest * qinfo = qs.m_quest;

            // hide quest if player is in raid-group and quest is no raid quest
            if(groupInfo.group && groupInfo.group->isRaidGroup() && qinfo->GetType() != 62)
                continue;

            // There should be no mixed ReqItem/ReqSource drop
            // This part for ReqItem drop
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if(itemid == qinfo->ReqItemId[j] && qs.m_itemcount[j] < qinfo->ReqItemCount[j] )
                    return true;
            }
            // This part - for ReqSource
            for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; j++)
            {
                // examined item is a source item
                if (qinfo->ReqSourceId[j] == itemid && qinfo->ReqSourceRef[j] > 0 && qinfo->ReqSourceRef[j] <= QUEST_OBJECTIVES_COUNT)
                {
                    uint32 idx = qinfo->ReqSourceRef[j]-1;
                    // total count of created ReqItems and SourceItems is less than ReqItemCount
                    if(qinfo->ReqItemId[idx] != 0 &&
                        qs.m_itemcount[idx] * qinfo->ReqSourceCount[j] + GetItemCount(itemid) + GetBankItemCount(itemid) < qinfo->ReqItemCount[idx] * qinfo->ReqSourceCount[j])
                        return true;

                    // total count of casted ReqCreatureOrGOs and SourceItems is less than ReqCreatureOrGOCount
                    if (qinfo->ReqCreatureOrGOId[idx] != 0 &&
                        qs.m_creatureOrGOcount[idx] * qinfo->ReqSourceCount[j] + GetItemCount(itemid) + GetBankItemCount(itemid) < qinfo->ReqCreatureOrGOCount[idx] * qinfo->ReqSourceCount[j])
                        return true;
                }
            }
        }
    }
    return false;
}

void Player::SendQuestComplete( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTUPDATE_COMPLETE, 4 );
        data << quest_id;
        GetSession()->SendPacket( &data );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_COMPLETE quest = %u", quest_id );
    }
}

void Player::SendQuestReward( Quest *pQuest, uint32 XP, Object * questGiver )
{
    if( pQuest )
    {
        uint32 questid = pQuest->GetQuestId();
        sLog.outDebug( "WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE quest = %u", questid );
        WorldPacket data( SMSG_QUESTGIVER_QUEST_COMPLETE, (8+8+4+pQuest->GetRewItemsCount()*8) );
        data << questid;
        data << uint32(0x03);

        if ( getLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) )
        {
            data << XP;
            data << uint32(pQuest->GetRewOrReqMoney());
        }
        else
        {
            data << uint32(0);
            data << uint32(pQuest->GetRewOrReqMoney() + XP);
        }
        data << uint32( pQuest->GetRewItemsCount() );

        for (int i = 0; i < pQuest->GetRewItemsCount(); i++)
        {
            if ( pQuest->RewItemId[i] > 0 )
                data << pQuest->RewItemId[i] << pQuest->RewItemCount[i];
        }

        GetSession()->SendPacket( &data );

        if (pQuest->GetQuestCompleteScript() != 0)
            sWorld.ScriptsStart(sScripts, pQuest->GetQuestCompleteScript(), questGiver, this);
    }
}

void Player::SendQuestFailed( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTGIVER_QUEST_FAILED, 4 );
        data << quest_id;
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_FAILED");
    }
}

void Player::SendQuestTimerFailed( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTUPDATE_FAILEDTIMER, 4 );
        data << quest_id;
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_FAILEDTIMER");
    }
}

void Player::SendCanTakeQuestResponse( uint32 msg )
{
    WorldPacket data( SMSG_QUESTGIVER_QUEST_INVALID, 4 );
    data << msg;
    GetSession()->SendPacket( &data );
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void Player::SendPushToPartyResponse( Player *pPlayer, uint32 msg )
{
    if( pPlayer )
    {
        WorldPacket data( MSG_QUEST_PUSH_RESULT, (8+4+1) );
        data << pPlayer->GetGUID();
        data << msg;
        data << uint8(0);
        GetSession()->SendPacket( &data );
        sLog.outDebug("WORLD: Sent MSG_QUEST_PUSH_RESULT");
    }
}

void Player::SendQuestUpdateAddItem( uint32 quest_id, uint32 item_idx, uint32 count )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTUPDATE_ADD_ITEM, (4+4) );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_ADD_ITEM" );
        data << objmgr.QuestTemplates[quest_id]->ReqItemId[item_idx];
        data << count;
        GetSession()->SendPacket( &data );
    }
}

void Player::SendQuestUpdateAddCreature( uint32 quest_id, uint64 guid, uint32 creature_idx, uint32 old_count, uint32 add_count )
{
    assert(old_count + add_count < 64 && "mob/GO count store in 6 bits 2^6 = 64 (0..63)");

    Quest * qInfo = objmgr.QuestTemplates[quest_id];
    if( qInfo )
    {
        WorldPacket data( SMSG_QUESTUPDATE_ADD_KILL, (24) );
        sLog.outDebug( "WORLD: Sent SMSG_QUESTUPDATE_ADD_KILL" );
        data << qInfo->GetQuestId();
        data << uint32(qInfo->ReqCreatureOrGOId[ creature_idx ]);
        data << old_count + add_count;
        data << qInfo->ReqCreatureOrGOCount[ creature_idx ];
        data << guid;
        GetSession()->SendPacket(&data);

        uint16 log_slot = GetQuestSlot( quest_id );
        uint32 kills = GetUInt32Value( log_slot + 1 );
        kills = kills + (add_count << ( 6 * creature_idx ));
        SetUInt32Value( log_slot + 1, kills );
    }
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/

bool Player::MinimalLoadFromDB( uint32 guid )
{
    QueryResult *result = sDatabase.Query("SELECT `data`,`name`,`position_x`,`position_y`,`position_z`,`map`,`totaltime`,`leveltime`,`rename` FROM `character` WHERE `guid` = '%u'",guid);
    if(!result)
        return false;

    Field *fields = result->Fetch();

    if(!LoadValues( fields[0].GetString()))
    {
        sLog.outError("ERROR: Player #%d have broken data in `data` field. Can't be loaded.",GUID_LOPART(guid));
        delete result;
        return false;
    }

    m_name = fields[1].GetCppString();

    Relocate(fields[2].GetFloat(),fields[3].GetFloat(),fields[4].GetFloat());
    SetMapId(fields[5].GetUInt32());

    m_Played_time[0] = fields[6].GetUInt32();
    m_Played_time[1] = fields[7].GetUInt32();

    m_needRename = fields[8].GetBool();

    _LoadGroup();

    _LoadBoundInstances();

    delete result;

    for (int i = 0; i < PLAYER_SLOTS_COUNT; i++)
        m_items[i] = NULL;

    if( HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST) )
        m_deathState = DEAD;

    return true;
}

bool Player::LoadPositionFromDB(uint32& mapid, float& x,float& y,float& z,float& o, uint64 guid)
{
    QueryResult *result = sDatabase.Query("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `character` WHERE `guid` = '%u'",GUID_LOPART(guid));
    if(!result)
        return false;

    Field *fields = result->Fetch();

    x = fields[0].GetFloat();
    y = fields[1].GetFloat();
    z = fields[2].GetFloat();
    o = fields[3].GetFloat();
    mapid = fields[4].GetUInt32();

    delete result;
    return true;
}

bool Player::LoadValuesArrayFromDB(vector<string> & data, uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT `data` FROM `character` WHERE `guid`='"<<GUID_LOPART(guid)<<"'";
    QueryResult *result = sDatabase.Query( ss.str().c_str() );
    if( !result )
        return false;

    Field *fields = result->Fetch();

    data = StrSplit(fields[0].GetString(), " ");

    delete result;

    return true;
}

uint32 Player::GetUInt32ValueFromArray(vector<string> const& data, uint16 index)
{
    return (uint32)atoi(data[index].c_str());
}

float Player::GetFloatValueFromArray(vector<string> const& data, uint16 index)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromArray(data,index);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

uint32 Player::GetUInt32ValueFromDB(uint16 index, uint64 guid)
{
    vector<string> data;
    if(!LoadValuesArrayFromDB(data,guid))
        return 0;

    return GetUInt32ValueFromArray(data,index);
}

float Player::GetFloatValueFromDB(uint16 index, uint64 guid)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromDB(index, guid);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

bool Player::LoadFromDB( uint32 guid )
{
    //                                                0        1       2      3      4      5         6            7            8         9        10            11         12          13         14            15          16               17                  18                 19               20        21        22       23         24          25      26            27
    QueryResult *result = sDatabase.Query("SELECT `guid`,`account`,`data`,`name`,`race`,`class`,`position_x`,`position_y`,`position_z`,`map`,`orientation`,`taximask`,`cinematic`,`totaltime`,`leveltime`,`rest_bonus`,`logout_time`,`is_logout_resting`,`resettalents_cost`,`resettalents_time`,`trans_x`,`trans_y`,`trans_z`,`trans_o`, `transguid`,`gmstate`,`stable_slots`,`rename` FROM `character` WHERE `guid` = '%u'", guid);

    if(!result)
    {
        sLog.outError("ERROR: Player (GUID: %u) not found in table `character`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();

    uint32 dbAccountId = fields[1].GetUInt32();

    // check if the character's account in the db and the logged in account match.
    // player should be able to load/delete character only with correct account!
    if( dbAccountId != GetSession()->GetAccountId() )
    {
        sLog.outError("ERROR: Player (GUID: %u) loading from wrong account (is: %u, should be: %u)",guid,GetSession()->GetAccountId(),dbAccountId);
        delete result;
        return false;
    }

    Object::_Create( guid, HIGHGUID_PLAYER );

    if(!LoadValues( fields[2].GetString()))
    {
        sLog.outError("ERROR: Player #%d have broken data in `data` field. Can't be loaded.",GUID_LOPART(guid));
        delete result;
        return false;
    }

    // cleanup inventory related item value fields (its will be filled correctly in _LoadInventory)
    for(uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), 0 );

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
        for(int i = 0; i < 11; ++i )
            SetUInt32Value(VisibleBase + i, 0);

        if (m_items[slot])
        {
            delete m_items[slot];
            m_items[slot] = NULL;
        }
    }

    m_drunk = GetUInt32Value(PLAYER_BYTES_3) & 0xFFFE;

    m_name = fields[3].GetCppString();

    sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());
    outDebugValues();

    m_race = fields[4].GetUInt8();
    //Need to call it to initialize m_team (m_team can be calculated from m_race)
    //Other way is to saves m_team into characters table.
    setFactionForRace(m_race);
    SetCharm(0);

    m_class = fields[5].GetUInt8();

    PlayerInfo const *info = objmgr.GetPlayerInfo(m_race, m_class);
    if(!info)
    {
        sLog.outError("Player have incorrect race/class pair. Can't be loaded.");
        delete result;
        return false;
    }

    uint32 transGUID = fields[24].GetUInt32();
    Relocate(fields[6].GetFloat(),fields[7].GetFloat(),fields[8].GetFloat(),fields[10].GetFloat());
    SetMapId(fields[9].GetUInt32());

    _LoadGroup();

    _LoadBoundInstances();

    SetRecallPosition(GetMapId(),GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());

    if (transGUID != 0)
    {
        m_transX = fields[20].GetFloat();
        m_transY = fields[21].GetFloat();
        m_transZ = fields[22].GetFloat();
        m_transO = fields[23].GetFloat();

        for (int i = 0; i < MapManager::Instance().m_Transports.size(); i++)
        {
            if ((MapManager::Instance().m_Transports[i])->GetGUIDLow() == transGUID)
            {
                m_transport = MapManager::Instance().m_Transports[i];
                m_transport->AddPassenger(this);
                SetMapId(m_transport->GetMapId());
            }
        }
    }

    // since last logout (in seconds)
    uint32 time_diff = (time(NULL) - fields[16].GetUInt32());

    m_rest_bonus = fields[15].GetFloat();
    //speed collect rest bonus in offline, in logout, far from tavern, city (section/in hour)
    float bubble0 = 0.0416;
    //speed collect rest bonus in offline, in logout, in tavern, city (section/in hour)
    float bubble1 = 0.083;

    if((int32)fields[16].GetUInt32() > 0)
    {
        float bubble = fields[17].GetUInt32() > 0
            ? bubble1*sWorld.getRate(RATE_REST_OFFLINE_IN_TAVERN_OR_CITY)
            : bubble0*sWorld.getRate(RATE_REST_OFFLINE_IN_WILDERNESS);

        SetRestBonus(GetRestBonus()+ time_diff*((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)/144000)*bubble);
    }

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Player (guidlow %d) have invalid coordinates (X: %f Y: %f). Teleport to default race/class locations.",guid,GetPositionX(),GetPositionY());

        SetMapId(info->mapId);
        Relocate(info->positionX,info->positionY,info->positionZ);
    }

    m_cinematic = fields[12].GetUInt32();
    m_Played_time[0]= fields[13].GetUInt32();
    m_Played_time[1]= fields[14].GetUInt32();

    m_resetTalentsCost = fields[18].GetUInt32();
    m_resetTalentsTime = fields[19].GetUInt64();

    // reserve some flags
    uint32 old_safe_flags = GetUInt32Value(PLAYER_FLAGS) & ( PLAYER_FLAGS_HIDE_CLOAK | PLAYER_FLAGS_HIDE_HELM );

    if( HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM) )
        SetUInt32Value(PLAYER_FLAGS, 0 | old_safe_flags);

    _LoadTaxiMask( fields[11].GetString() );

    uint32 gmstate = fields[25].GetUInt32();

    m_stableSlots = fields[26].GetUInt32();
    if(m_stableSlots > 2)
    {
        sLog.outError("Player can have not more 2 stable slots, but have in DB %u",uint32(m_stableSlots));
        m_stableSlots = 2;
    }

    m_needRename = fields[27].GetBool();

    delete result;

    // clear channel spell data (if saved at channel spell casting)
    SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
    SetUInt32Value(UNIT_CHANNEL_SPELL,0);

    // clear charm/summon related fields
    SetUInt64Value(UNIT_FIELD_CHARM,0);
    SetUInt64Value(UNIT_FIELD_SUMMON,0);
    SetUInt64Value(UNIT_FIELD_CHARMEDBY,0);
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY,0);
    SetUInt64Value(UNIT_FIELD_CREATEDBY,0);

    // reset skill modifiers
    for (uint32 i = 0; i < PLAYER_MAX_SKILLS; i++)
        SetUInt32Value(PLAYER_SKILL(i)+2,0);

    // make sure the unit is considered out of combat for proper loading
    ClearInCombat(true);

    // make sure the unit is considered not in duel for proper loading
    SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    // remember loaded power/health values to restore after stats initialization and modifier applying
    uint32 savedHealth = GetHealth();
    uint32 savedPower[MAX_POWERS];
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        savedPower[i] = GetPower(Powers(i));

    // reset stats before loading any modifiers
    InitStatsForLevel(getLevel(),false,false);

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()
    _ApplyStatsMods();

    //mails are loaded only when needed ;-) - when player in game click on mailbox.
    //_LoadMail();

    _LoadAuras(time_diff);

    // add ghost flag (must be after aura load: PLAYER_FLAGS_GHOST set in aura)
    if( HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST) )
        m_deathState = DEAD;

    _LoadSpells(time_diff);

    // after spell load
    InitTalentForLevel();

    _LoadQuestStatus();

    _LoadTutorials();

    _LoadReputation();                                      // must be before inventory (some items required reputation check)

    _LoadInventory(time_diff);

    _LoadActions();

    // Skip _ApplyAllAuraMods(); -- applied in _LoadAuras by AddAura calls at aura load
    // Skip _ApplyAllItemMods(); -- already applied in _LoadInventory()

    // restore remembered power/health values (but not more max values)
    SetHealth(savedHealth > GetMaxHealth() ? GetMaxHealth() : savedHealth);
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        SetPower(Powers(i),savedPower[i] > GetMaxPower(Powers(i)) ? GetMaxPower(Powers(i)) : savedPower[i]);

    sLog.outDebug("The value of player %s after load item and aura is: ", m_name.c_str());
    outDebugValues();

    // GM state
    if(GetSession()->GetSecurity() > 0)
    {
        switch(sWorld.getConfig(CONFIG_GM_LOGIN_STATE))
        {
            case 0:                                         // disable
                break;
            case 1:                                         // enable
                SetGameMaster(true);
                break;
            case 2:                                         // save state
                if(gmstate)
                    SetGameMaster(true);
                break;
            default:
                break;
        }
    }
    //Unmount Player from previous mount, so speed bug with mount is no more...
    if(IsMounted())
    {
        Unmount();
        RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    }

    m_Loaded = true;

    return true;
}

void Player::_LoadActions()
{
    m_actionButtons.clear();

    QueryResult *result = sDatabase.Query("SELECT `button`,`action`,`type`,`misc` FROM `character_action` WHERE `guid` = '%u' ORDER BY `button`",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint8 button = fields[0].GetUInt8();

            addActionButton(button, fields[1].GetUInt16(), fields[2].GetUInt8(), fields[3].GetUInt8());

            m_actionButtons[button].uState = ACTIONBUTTON_UNCHANGED;
        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_LoadAuras(uint32 timediff)
{
    m_Auras.clear();
    for (int i = 0; i < TOTAL_AURAS; i++)
        m_modAuras[i].clear();

    for(uint8 i = 0; i < 48; i++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURA + i), 0);
    for(uint8 j = 0; j < 6; j++)
        SetUInt32Value((uint16)(UNIT_FIELD_AURAFLAGS + j), 0);

    QueryResult *result = sDatabase.Query("SELECT `spell`,`effect_index`,`remaintime` FROM `character_aura` WHERE `guid` = '%u'",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 spellid = fields[0].GetUInt32();
            uint32 effindex = fields[1].GetUInt32();
            int32 remaintime = (int32)fields[2].GetUInt32();

            SpellEntry const* spellproto = sSpellStore.LookupEntry(spellid);
            if(!spellproto)
            {
                sLog.outError("Unknown aura (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            if(effindex >= 3)
            {
                sLog.outError("Invalid effect index (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && !IsPositiveEffect(spellid, effindex))
            {
                remaintime -= timediff;
                if(remaintime <= 0) continue;
            }

            // FIXME: real caster not stored in DB currently

            Aura* aura = new Aura(spellproto, effindex, this, this/*caster*/);
            aura->SetAuraDuration(remaintime);
            AddAura(aura);
        }
        while( result->NextRow() );

        delete result;
    }

    if(m_class == CLASS_WARRIOR)
        CastSpell(this,SPELL_PASSIVE_BATTLE_STANCE,true);
}

void Player::LoadCorpse()
{
    if( isAlive() )
    {
        ObjectAccessor::Instance().ConvertCorpseForPlayer(GetGUID());
    }
    else
    {
        if(CorpsePtr corpse = GetCorpse())
        {
            corpse->UpdateForPlayer(this,true);

            if( corpse->GetType() == CORPSE_RESURRECTABLE && IsWithinDistInMap(&*corpse,0.0))
                RepopAtGraveyard();
        }
        else
        {
            //Prevent Dead Player login without corpse
            ResurrectPlayer();
        }
    }
}

void Player::_LoadInventory(uint32 timediff)
{
    QueryResult *result = sDatabase.Query("SELECT `slot`,`item`,`item_template` FROM `character_inventory` WHERE `guid` = '%u' AND `bag` = '0' ORDER BY `slot`",GetGUIDLow());

    uint16 dest;
    if (result)
    {
        // prevent items from being added to the queue when stored
        m_itemUpdateQueueBlocked = true;
        do
        {
            Field *fields = result->Fetch();
            uint8  slot      = fields[0].GetUInt8();
            uint32 item_guid = fields[1].GetUInt32();
            uint32 item_id   = fields[2].GetUInt32();

            ItemPrototype const * proto = objmgr.GetItemPrototype(item_id);

            if(!proto)
            {
                sLog.outError( "Player::_LoadInventory: Player %s have unknown item (id: #%u) in inventory, skipped.", GetName(),item_id );
                continue;
            }

            Item *item = NewItemOrBag(proto);
            item->SetSlot( slot );
            item->SetContainer( NULL );

            if(!item->LoadFromDB(item_guid, GetGUID()))
            {
                delete item;
                continue;
            }

            bool success = true;
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | slot);
            if( IsInventoryPos( dest ) )
            {
                if( !CanStoreItem( INVENTORY_SLOT_BAG_0, slot, dest, item, false ) == EQUIP_ERR_OK )
                {
                    success = false;
                    continue;
                }

                item = StoreItem(dest, item, true);
            }
            else if( IsEquipmentPos( dest ) )
            {
                if( !CanEquipItem( slot, dest, item, false, false ) == EQUIP_ERR_OK )
                {
                    success = false;
                    continue;
                }

                QuickEquipItem(dest, item);
            }
            else if( IsBankPos( dest ) )
            {
                if( !CanBankItem( INVENTORY_SLOT_BAG_0, slot, dest, item, false, false ) == EQUIP_ERR_OK )
                {
                    success = false;
                    continue;
                }

                item = BankItem(dest, item, true);
            }

            // item's state may have changed after stored
            if (success) item->SetState(ITEM_UNCHANGED, this);
            else delete item;
        } while (result->NextRow());

        delete result;
        m_itemUpdateQueueBlocked = false;
    }
    if(isAlive())
        _ApplyAllItemMods();
}

// load mailed items which should receive current player
void Player::_LoadMailedItems()
{
    QueryResult *result = sDatabase.Query( "SELECT `item_guid`,`item_template` FROM `mail` WHERE `receiver` = '%u' AND `item_guid` > 0", GetGUIDLow());

    if( !result )
        return;

    Field *fields;
    do
    {
        fields = result->Fetch();
        uint32 item_guid = fields[0].GetUInt32();
        uint32 item_template = fields[1].GetUInt32();

        ItemPrototype const *proto = objmgr.GetItemPrototype(item_template);

        if(!proto)
        {
            sLog.outError( "Player %u have unknown item_template (ProtoType) in mailed items(GUID: %u template: %u) in mail, skipped.", GetGUIDLow(), item_guid, item_template);
            continue;
        }
        Item *item = NewItemOrBag(proto);
        if(!item->LoadFromDB(item_guid, 0))
        {
            sLog.outError( "Player::_LoadMailedItems - Mailed Item doesn't exist!!!! - item guid: %u", item_guid);
            delete item;
            continue;
        }
        AddMItem(item);
    }
    while( result->NextRow() );

    delete result;
}

void Player::_LoadMail()
{
    time_t base = time(NULL);

    _LoadMailedItems();

    m_mail.clear();
    //mails are in right order
    QueryResult *result = sDatabase.Query("SELECT `id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked` FROM `mail` WHERE `receiver` = '%u' ORDER BY `id` DESC",GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            Mail *m = new Mail;
            m->messageID = fields[0].GetUInt32();
            m->messageType = fields[1].GetUInt8();
            m->sender = fields[2].GetUInt32();
            m->receiver = fields[3].GetUInt32();
            m->subject = fields[4].GetCppString();
            m->itemTextId = fields[5].GetUInt32();
            m->item_guid = fields[6].GetUInt32();
            m->item_template = fields[7].GetUInt32();
            m->expire_time = (time_t)fields[8].GetUInt64();
            m->deliver_time = (time_t)fields[9].GetUInt64();
            m->money = fields[10].GetUInt32();
            m->COD = fields[11].GetUInt32();
            m->checked = fields[12].GetUInt32();
            m->state = MAIL_STATE_UNCHANGED;
            m_mail.push_back(m);
        } while( result->NextRow() );
        delete result;
    }
    m_mailsLoaded = true;
}

void Player::LoadPet()
{
    Pet *pet = new Pet(this, getClass()==CLASS_HUNTER?HUNTER_PET:SUMMON_PET);
    if(!pet->LoadPetFromDB(this,0,0,true))
        delete pet;
}

void Player::_LoadQuestStatus()
{
    mQuestStatus.clear();

    uint32 slot = 0;

    QueryResult *result = sDatabase.Query("SELECT `quest`,`status`,`rewarded`,`explored`,`timer`,`mobcount1`,`mobcount2`,`mobcount3`,`mobcount4`,`itemcount1`,`itemcount2`,`itemcount3`,`itemcount4` FROM `character_queststatus` WHERE `guid` = '%u'", GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();
            Quest* pQuest = objmgr.QuestTemplates[quest_id];// used to be new, no delete?
            if( pQuest )
            {
                mQuestStatus[quest_id].m_quest = pQuest;

                uint32 qstatus = fields[1].GetUInt32();
                if(qstatus < MAX_QUEST_STATUS)
                    mQuestStatus[quest_id].m_status = QuestStatus(qstatus);
                else
                {
                    mQuestStatus[quest_id].m_status = QUEST_STATUS_NONE;
                    sLog.outError("Player %s have invalid quest %d status (%d), replaced by QUEST_STATUS_NONE(0).",GetName(),quest_id,qstatus);
                }

                mQuestStatus[quest_id].m_rewarded = ( fields[2].GetUInt8() > 0 );
                mQuestStatus[quest_id].m_explored = ( fields[3].GetUInt8() > 0 );

                uint32 quest_time = fields[4].GetUInt32();

                if( objmgr.QuestTemplates[quest_id]->HasSpecialFlag( QUEST_SPECIAL_FLAGS_TIMED ) && !GetQuestRewardStatus(quest_id) )
                {
                    AddTimedQuest( quest_id );

                    if (quest_time <= sWorld.GetGameTime())
                        mQuestStatus[quest_id].m_timer = 1;
                    else
                        mQuestStatus[quest_id].m_timer = (quest_time - sWorld.GetGameTime()) * 1000;
                }
                else
                    quest_time = 0;

                mQuestStatus[quest_id].m_creatureOrGOcount[0] = fields[5].GetUInt32();
                mQuestStatus[quest_id].m_creatureOrGOcount[1] = fields[6].GetUInt32();
                mQuestStatus[quest_id].m_creatureOrGOcount[2] = fields[7].GetUInt32();
                mQuestStatus[quest_id].m_creatureOrGOcount[3] = fields[8].GetUInt32();
                mQuestStatus[quest_id].m_itemcount[0] = fields[9].GetUInt32();
                mQuestStatus[quest_id].m_itemcount[1] = fields[10].GetUInt32();
                mQuestStatus[quest_id].m_itemcount[2] = fields[11].GetUInt32();
                mQuestStatus[quest_id].m_itemcount[3] = fields[12].GetUInt32();

                mQuestStatus[quest_id].uState = QUEST_UNCHANGED;

                // add to quest log
                if( slot < MAX_QUEST_LOG_SIZE &&
                    ( mQuestStatus[quest_id].m_status==QUEST_STATUS_INCOMPLETE ||
                    mQuestStatus[quest_id].m_status==QUEST_STATUS_COMPLETE && !mQuestStatus[quest_id].m_rewarded ) )
                {
                    uint32 state = 0;
                    if(mQuestStatus[quest_id].m_status == QUEST_STATUS_COMPLETE)
                        state |= 1 << 24;

                    for(uint8 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
                    {
                        if(mQuestStatus[quest_id].m_creatureOrGOcount[idx])
                            state += (mQuestStatus[quest_id].m_creatureOrGOcount[idx] << ( 6 * idx ));
                    }

                    SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot+0,quest_id);
                    SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot+1,state);
                    SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*slot+2,quest_time);

                    ++slot;
                }

                sLog.outDebug("Quest status is {%u} for quest {%u}", mQuestStatus[quest_id].m_status, quest_id);
            }
        }
        while( result->NextRow() );

        delete result;
    }

    // clear quest log tail
    for ( uint16 i = slot; i < MAX_QUEST_LOG_SIZE; ++i )
    {
        SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i+0,0);
        SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i+1,0);
        SetUInt32Value(PLAYER_QUEST_LOG_1_1 + 3*i+2,0);
    }
}

void Player::_LoadReputation()
{
    m_factions.clear();

    QueryResult *result = sDatabase.Query("SELECT `faction`,`reputation`,`standing`,`flags` FROM `character_reputation` WHERE `guid` = '%u'",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            Faction newFaction;
            newFaction.ID               = fields[0].GetUInt32();
            newFaction.ReputationListID = fields[1].GetUInt32();
            newFaction.Standing         = int32(fields[2].GetUInt32());
            newFaction.Flags            = fields[3].GetUInt32();
            newFaction.uState           = FACTION_UNCHANGED;

            m_factions.push_back(newFaction);
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

void Player::_LoadSpells(uint32 timediff)
{
    for (PlayerSpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        delete itr->second;
    m_spells.clear();

    QueryResult *result = sDatabase.Query("SELECT `spell`,`slot`,`active` FROM `character_spell` WHERE `guid` = '%u'",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            addSpell(fields[0].GetUInt16(), fields[2].GetUInt8(), PLAYERSPELL_UNCHANGED, fields[1].GetUInt16());
        }
        while( result->NextRow() );

        delete result;
    }
}

void Player::_LoadTaxiMask(const char* data)
{
    vector<string> tokens = StrSplit(data, " ");

    int index;
    vector<string>::iterator iter;

    for (iter = tokens.begin(), index = 0;
        (index < TaxiMaskSize) && (iter != tokens.end()); ++iter, ++index)
    {
        // load and set bits only for existed taxi nodes
        m_taximask[index] = sTaxiNodesMask[index] & uint32(atol((*iter).c_str()));
    }
}

void Player::_LoadTutorials()
{
    QueryResult *result = sDatabase.Query("SELECT `tut0`,`tut1`,`tut2`,`tut3`,`tut4`,`tut5`,`tut6`,`tut7` FROM `character_tutorial` WHERE `guid` = '%u'",GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            for (int iI=0; iI<8; iI++)
                m_Tutorials[iI] = fields[iI].GetUInt32();
        }
        while( result->NextRow() );

        delete result;
    }
}

/*********************************************************/
/***                   SAVE SYSTEM                     ***/
/*********************************************************/

void Player::SaveToDB(bool first_save)
{
    // delay auto save at any saves (manual, in code, or autosave)
    m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

    // saved before flight
    if (isInFlight())
        return;

    // Must saved before enter into BattleGround
    if(InBattleGround())
        return;

    int is_save_resting = HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ? 1 : 0;
                                                            //save, far from tavern/city
                                                            //save, but in tavern/city
    sLog.outDebug("The value of player %s before unload item and aura is: ", m_name.c_str());
    outDebugValues();

    // remember current power/health values with all auras/item/stats mods to save and restore at load
    uint32 currentHealth = GetHealth();
    uint32 currentPower[MAX_POWERS];
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        currentPower[i] = GetPower(Powers(i));

    if(isAlive())
    {
        _RemoveAllItemMods();
        _RemoveAllAuraMods();
    }

    // not required: all stats mods recalculated at load
    //_RemoveStatsMods();

    // save state (after auras removing), if aura remove some flags then it must set it back by self)
    uint32 tmp_bytes = GetUInt32Value(UNIT_FIELD_BYTES_1);
    uint32 tmp_flags = GetUInt32Value(UNIT_FIELD_FLAGS);
    uint32 tmp_pflags = GetUInt32Value(PLAYER_FLAGS);
    uint32 tmp_displayid = GetUInt32Value(UNIT_FIELD_DISPLAYID);

    // Set player sit state to standing on save, also stealth and shifted form
    RemoveFlag(UNIT_FIELD_BYTES_1,PLAYER_STATE_SIT | PLAYER_STATE_FORM_ALL | PLAYER_STATE_FLAG_STEALTH);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    SetUInt32Value(UNIT_FIELD_DISPLAYID,GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));

    // remove restflag when save
    //this is because of the rename char stuff
    //RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

    bool inworld = IsInWorld();

    // remember base (exactly) power/health values before temporary set to saved currentPower/currentHealth data
    uint32 baseHealth = GetUInt32Value(UNIT_FIELD_HEALTH);
    float basePower[MAX_POWERS];
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        basePower[i] = GetFloatValue(UNIT_FIELD_POWER1+i);

    // temporary set current power/health values to save
    SetUInt32Value(UNIT_FIELD_HEALTH,currentHealth);
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        SetFloatValue(UNIT_FIELD_POWER1+i,float(currentPower[i]));

    //sDatabase.BeginTransaction();

    //sDatabase.Execute("DELETE FROM `character` WHERE `guid` = '%u'",GetGUIDLow());
    if(!first_save)
        sDatabase.Execute("DELETE FROM `character` WHERE `guid` = '%u'",GetGUIDLow());

    std::ostringstream ss;
    ss << "INSERT INTO `character` (`guid`,`account`,`name`,`race`,`class`,"
        "`map`,`position_x`,`position_y`,`position_z`,`orientation`,`data`,"
        "`taximask`,`online`,`cinematic`,"
        "`totaltime`,`leveltime`,`rest_bonus`,`logout_time`,`is_logout_resting`,`resettalents_cost`,`resettalents_time`,"
        "`trans_x`, `trans_y`, `trans_z`, `trans_o`, `transguid`, `gmstate`, `stable_slots`,`rename`) VALUES ("
        << GetGUIDLow() << ", "
        << GetSession()->GetAccountId() << ", '"
        << m_name << "', "
        << m_race << ", "
        << m_class << ", "
        << GetMapId() << ", "
        << GetPositionX() << ", "
        << GetPositionY() << ", "
        << GetPositionZ() << ", "
        << GetOrientation() << ", '";

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
    ss << m_cinematic;

    ss << ", ";
    ss << m_Played_time[0];
    ss << ", ";
    ss << m_Played_time[1];

    ss << ", ";
    ss << m_rest_bonus;
    ss << ", ";
    ss << time(NULL);
    ss << ", ";
    ss << is_save_resting;
    ss << ", ";
    ss << m_resetTalentsCost;
    ss << ", ";
    ss << (uint64)m_resetTalentsTime;

    ss << ", ";
    ss << m_transX;
    ss << ", ";
    ss << m_transY;
    ss << ", ";
    ss << m_transZ;
    ss << ", ";
    ss << m_transO;
    ss << ", ";
    if (m_transport)
        ss << m_transport->GetGUIDLow();
    else
        ss << "0";

    ss << ", ";
    ss << (isGameMaster()? 1 : 0);

    ss << ", ";
    ss << uint32(m_stableSlots);                            // to prevent save uint8 as char

    ss << ", ";
    ss << (isNeedRename()? 1 : 0);

    ss << " )";

    //sDatabase.Execute( ss.str().c_str() );
    if(!first_save)
        sDatabase.Execute( ss.str().c_str() );
    else
        sDatabase.WaitExecute(ss.str().c_str());

    SaveEnchant();

    if(m_mailsUpdated)                                      //save mails only when needed
        _SaveMail();

    //_SaveInventory();
    _SaveInventory(first_save);
    _SaveQuestStatus();
    _SaveTutorials();
    _SaveSpells();
    _SaveSpellCooldowns();
    _SaveActions();
    _SaveAuras();
    _SaveReputation();
    _SaveBoundInstances();

    //sDatabase.CommitTransaction();

    // restore base power/health values before restore mods
    SetUInt32Value(UNIT_FIELD_HEALTH,baseHealth);
    for(uint32 i = 0; i < MAX_POWERS; ++i)
        SetFloatValue(UNIT_FIELD_POWER1+i,basePower[i]);

    sLog.outDebug("Save Basic value of player %s is: ", m_name.c_str());
    outDebugValues();

    // restore state (before aura apply, if aura remove flag then aura must set it ack by self)
    SetUInt32Value(UNIT_FIELD_DISPLAYID, tmp_displayid);
    SetUInt32Value(UNIT_FIELD_BYTES_1, tmp_bytes);
    SetUInt32Value(UNIT_FIELD_FLAGS, tmp_flags);
    SetUInt32Value(PLAYER_FLAGS, tmp_pflags);

    // not required: all stats mods recalculated at load and _RemoveStatsMods not called early in code
    // _ApplyStatsMods();

    if(isAlive())
    {
        _ApplyAllAuraMods();
        _ApplyAllItemMods();
    }

    // save pet (hunter pet level and experience and all type pets health/mana).
    if(Pet* pet = GetPet())
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

// fast save function for item/money cheating preventing - save only inventory and money state
void Player::SaveInventoryAndGoldToDB()
{
    _SaveInventory(false);
    SetUInt32ValueInDB(PLAYER_FIELD_COINAGE,GetMoney(),GetGUID());
}

void Player::_SaveActions()
{
    for(ActionButtonList::iterator itr = m_actionButtons.begin(); itr != m_actionButtons.end(); )
    {
        switch (itr->second.uState)
        {
            case ACTIONBUTTON_NEW:
                sDatabase.Execute("INSERT INTO `character_action` (`guid`,`button`,`action`,`type`,`misc`) VALUES ('%u', '%u', '%u', '%u', '%u')",
                    GetGUIDLow(), (uint32)itr->first, (uint32)itr->second.action, (uint32)itr->second.type, (uint32)itr->second.misc );
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            case ACTIONBUTTON_CHANGED:
                sDatabase.Execute("UPDATE `character_action`  SET `action` = '%u', `type` = '%u', `misc`= '%u' WHERE `guid`= '%u' AND `button`= '%u' ",
                    (uint32)itr->second.action, (uint32)itr->second.type, (uint32)itr->second.misc, GetGUIDLow(), (uint32)itr->first );
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            case ACTIONBUTTON_DELETED:
                sDatabase.Execute("DELETE FROM `character_action` WHERE `guid` = '%u' and button = '%u'", GetGUIDLow(), (uint32)itr->first );
                m_actionButtons.erase(itr++);
                break;
            default:
                ++itr;
                break;
        };
    }
}

void Player::_SaveAuras()
{
    sDatabase.Execute("DELETE FROM `character_aura` WHERE `guid` = '%u'",GetGUIDLow());

    AuraMap const& auras = GetAuras();
    for(AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        // skip all auras from spell that apply at cast SPELL_AURA_MOD_SHAPESHIFT or SPELL_AURA_MOD_STEALTH auras.
        SpellEntry const *spellInfo = itr->second->GetSpellProto();
        uint8 i;
        for (i = 0; i < 3; i++)
            if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT ||
            spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_STEALTH)
                break;

        if (i == 3 && !itr->second->IsPassive())
        {
            sDatabase.Execute("DELETE FROM `character_aura` WHERE `guid` = '%u' and `spell` = '%u' and  `effect_index`= '%u'",GetGUIDLow(),(uint32)(*itr).second->GetId(), (uint32)(*itr).second->GetEffIndex());
            sDatabase.Execute("INSERT INTO `character_aura` (`guid`,`spell`,`effect_index`,`remaintime`) VALUES ('%u', '%u', '%u', '%d')", GetGUIDLow(), (uint32)(*itr).second->GetId(), (uint32)(*itr).second->GetEffIndex(), int((*itr).second->GetAuraDuration()));
        }
    }
}

void Player::_SaveInventory(bool first_save)
{
    // force items in buyback slots to new state
    // and remove those that aren't already
    for (uint8 i = BUYBACK_SLOT_START; i < BUYBACK_SLOT_END; i++)
    {
        Item *item = m_items[i];
        if (!item || item->GetState() == ITEM_NEW) continue;
        sDatabase.Execute("DELETE FROM `character_inventory` WHERE `item` = '%u'", item->GetGUIDLow());
        sDatabase.Execute("DELETE FROM `item_instance` WHERE `guid` = '%u'", item->GetGUIDLow());
        m_items[i]->FSetState(ITEM_NEW);
    }

    if (m_itemUpdateQueue.empty()) return;

    // do not save if the update queue is corrupt
    bool error = false;
    for(int i = 0; i < m_itemUpdateQueue.size(); i++)
    {
        Item *item = m_itemUpdateQueue[i];
        if(!item || item->GetState() == ITEM_REMOVED) continue;
        Item *test = GetItemByPos( item->GetBagSlot(), item->GetSlot());

        if (test == NULL)
        {
            sLog.outError("Player::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow());
            error = true;
        }
        else if (test != item)
        {
            sLog.outError("Player::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), test->GetGUIDLow());
            error = true;
        }
    }

    if (error)
    {
        sLog.outError("Player::_SaveInventory - one or more errors occured save aborted!");
        sChatHandler.SendSysMessage(GetSession(), "Item save failed!");
        return;
    }

    for(int i = 0; i < m_itemUpdateQueue.size(); i++)
    {
        Item *item = m_itemUpdateQueue[i];
        if(!item) continue;

        Bag *container = item->GetContainer();
        uint32 bag_guid = container ? container->GetGUIDLow() : 0;

        switch(item->GetState())
        {
            case ITEM_NEW:
                //sDatabase.Execute("INSERT INTO `character_inventory` (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', '%u', '%u', '%u', '%u')", GetGUIDLow(), bag_guid, item->GetSlot(), item->GetGUIDLow(), item->GetEntry());
                if(!first_save)
                        sDatabase.Execute("INSERT INTO `character_inventory` (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', '%u', '%u', '%u', '%u')", GetGUIDLow(), bag_guid, item->GetSlot(), item->GetGUIDLow(), item->GetEntry());
                    else
                        sDatabase.WaitExecute("INSERT INTO `character_inventory` (`guid`,`bag`,`slot`,`item`,`item_template`) VALUES ('%u', '%u', '%u', '%u', '%u')", GetGUIDLow(), bag_guid, item->GetSlot(), item->GetGUIDLow(), item->GetEntry());
                break;
            case ITEM_CHANGED:
                sDatabase.Execute("UPDATE `character_inventory` SET `guid`='%u', `bag`='%u', `slot`='%u', `item_template`='%u' WHERE `item`='%u'", GetGUIDLow(), bag_guid, item->GetSlot(), item->GetEntry(), item->GetGUIDLow());
                break;
            case ITEM_REMOVED:
                sDatabase.Execute("DELETE FROM `character_inventory` WHERE `item` = '%u'", item->GetGUIDLow());
        }

        //item->SaveToDB();
        item->SaveToDB(first_save);
    }
    m_itemUpdateQueue.clear();
}

void Player::_SaveMail()
{
    if (!m_mailsLoaded)
        return;

    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); itr++)
    {
        Mail *m = (*itr);
        if (m->state == MAIL_STATE_CHANGED)
        {
            sDatabase.Execute("UPDATE `mail` SET `itemTextId` = '%u',`item_guid` = '%u',`item_template` = '%u',`expire_time` = '" I64FMTD "', `deliver_time` = '" I64FMTD "',`money` = '%u',`cod` = '%u',`checked` = '%u' WHERE `id` = '%u'",
                m->itemTextId, m->item_guid, m->item_template, (uint64)m->expire_time, (uint64)m->deliver_time, m->money, m->COD, m->checked, m->messageID);
            m->state = MAIL_STATE_UNCHANGED;
        }
        else if (m->state == MAIL_STATE_DELETED)
        {
            if (m->item_guid)
                sDatabase.Execute("DELETE FROM `item_instance` WHERE `guid` = '%u'", m->item_guid);
            if (m->itemTextId)
            {
                sDatabase.Execute("DELETE FROM `item_text` WHERE `id` = '%u'", m->itemTextId);
            }
            sDatabase.Execute("DELETE FROM `mail` WHERE `id` = '%u'", m->messageID);
        }
    }
    //deallocate deleted mails...
    bool continueDeleting = true;
    while ( continueDeleting )
    {
        continueDeleting = false;
        for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); itr++)
        {
            if ((*itr)->state == MAIL_STATE_DELETED)
            {
                Mail* m = *itr;
                m_mail.erase(itr);
                continueDeleting = true;
                delete m;
                break;                                      //break only from for cycle
            }
        }
    }
    m_mailsUpdated = false;
}

void Player::_SaveQuestStatus()
{
    // we don't need transactions here.
    for( QuestStatusMap::iterator i = mQuestStatus.begin( ); i != mQuestStatus.end( ); ++i )
    {
        switch (i->second.uState)
        {
            case QUEST_NEW :
                sDatabase.Execute("INSERT INTO `character_queststatus` (`guid`,`quest`,`status`,`rewarded`,`explored`,`timer`,`mobcount1`,`mobcount2`,`mobcount3`,`mobcount4`,`itemcount1`,`itemcount2`,`itemcount3`,`itemcount4`) VALUES ('%u', '%u', '%u', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')",
                    GetGUIDLow(), i->first, i->second.m_status, i->second.m_rewarded, i->second.m_explored, uint64(i->second.m_timer / 1000 + sWorld.GetGameTime()), i->second.m_creatureOrGOcount[0], i->second.m_creatureOrGOcount[1], i->second.m_creatureOrGOcount[2], i->second.m_creatureOrGOcount[3], i->second.m_itemcount[0], i->second.m_itemcount[1], i->second.m_itemcount[2], i->second.m_itemcount[3]);
                break;
            case QUEST_CHANGED :
                sDatabase.Execute("UPDATE `character_queststatus` SET `status` = '%u',`rewarded` = '%u',`explored` = '%u',`timer` = '" I64FMTD "',`mobcount1` = '%u',`mobcount2` = '%u',`mobcount3` = '%u',`mobcount4` = '%u',`itemcount1` = '%u',`itemcount2` = '%u',`itemcount3` = '%u',`itemcount4` = '%u'  WHERE `guid` = '%u' AND `quest` = '%u' ",
                    i->second.m_status, i->second.m_rewarded, i->second.m_explored, uint64(i->second.m_timer / 1000 + sWorld.GetGameTime()), i->second.m_creatureOrGOcount[0], i->second.m_creatureOrGOcount[1], i->second.m_creatureOrGOcount[2], i->second.m_creatureOrGOcount[3], i->second.m_itemcount[0], i->second.m_itemcount[1], i->second.m_itemcount[2], i->second.m_itemcount[3], GetGUIDLow(), i->first );
                break;
        };
        i->second.uState = QUEST_UNCHANGED;
    }
}

void Player::_SaveReputation()
{
    for(FactionsList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        switch(itr->uState)
        {
            case FACTION_UNCHANGED:
                break;
            case FACTION_CHANGED:
                sDatabase.Execute("UPDATE `character_reputation` SET `reputation`='%u', `standing`='%i', `flags`='%u' WHERE `guid` = '%u' AND `faction`='%u'",
                    itr->ReputationListID, itr->Standing, itr->Flags, GetGUIDLow(), itr->ID );
                itr->uState = FACTION_UNCHANGED;
                break;
            case FACTION_NEW:
                sDatabase.Execute("INSERT INTO `character_reputation` (`guid`,`faction`,`reputation`,`standing`,`flags`) VALUES ('%u', '%u', '%u', '%i', '%u')", GetGUIDLow(), itr->ID, itr->ReputationListID, itr->Standing, itr->Flags);
                itr->uState = FACTION_UNCHANGED;
                break;
            default:
                sLog.outError("Unknown faction lists entry state: %d",itr->uState);
                break;
        }
    }
}

void Player::_SaveSpells()
{
    for (PlayerSpellMap::const_iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        next++;
        if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->state == PLAYERSPELL_CHANGED)
            sDatabase.Execute("DELETE FROM `character_spell` WHERE `guid` = '%u' and `spell` = '%u'", GetGUIDLow(), itr->first);
        if (itr->second->state == PLAYERSPELL_NEW || itr->second->state == PLAYERSPELL_CHANGED)
            sDatabase.Execute("INSERT INTO `character_spell` (`guid`,`spell`,`slot`,`active`) VALUES ('%u', '%u', '%u','%u')", GetGUIDLow(), itr->first, itr->second->slotId,itr->second->active);

        if (itr->second->state == PLAYERSPELL_REMOVED)
            _removeSpell(itr->first);
        else
            itr->second->state = PLAYERSPELL_UNCHANGED;
    }
}

void Player::_SaveTutorials()
{
    // it's better than rebuilding indexes multiple times
    QueryResult *result = sDatabase.Query("SELECT count(*) AS r FROM `character_tutorial` WHERE `guid` = '%u'", GetGUIDLow() );
    Field *fields = result->Fetch();
    uint32 Rows = fields[0].GetUInt32();
    delete result;

    if (Rows)
    {
        sDatabase.Execute("UPDATE `character_tutorial` SET `tut0`='%u', `tut1`='%u', `tut2`='%u', `tut3`='%u', `tut4`='%u', `tut5`='%u', `tut6`='%u', `tut7`='%u' WHERE `guid` = '%u'",
            m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7], GetGUIDLow() );
    } else
    {
        sDatabase.Execute("INSERT INTO `character_tutorial` (`guid`,`tut0`,`tut1`,`tut2`,`tut3`,`tut4`,`tut5`,`tut6`,`tut7`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')", GetGUIDLow(), m_Tutorials[0], m_Tutorials[1], m_Tutorials[2], m_Tutorials[3], m_Tutorials[4], m_Tutorials[5], m_Tutorials[6], m_Tutorials[7]);
    };
}

void Player::outDebugValues() const
{
    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetMaxHealth(), GetMaxPower(POWER_MANA));
    sLog.outDebug("AGILITY is: \t\t%f\t\tSTRENGTH is: \t\t%f",GetStat(STAT_AGILITY), GetStat(STAT_STRENGTH));
    sLog.outDebug("INTELLECT is: \t\t%f\t\tSPIRIT is: \t\t%f",GetStat(STAT_INTELLECT), GetStat(STAT_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%f\t\tSPIRIT is: \t\t%f",GetStat(STAT_STAMINA), GetStat(STAT_SPIRIT));
    sLog.outDebug("Armor is: \t\t%f\t\tBlock is: \t\t%f",GetArmor(), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%f\t\tFireRes is: \t\t%f",GetResistance(SPELL_SCHOOL_HOLY), GetResistance(SPELL_SCHOOL_FIRE));
    sLog.outDebug("NatureRes is: \t\t%f\t\tFrostRes is: \t\t%f",GetResistance(SPELL_SCHOOL_NATURE), GetResistance(SPELL_SCHOOL_FROST));
    sLog.outDebug("ShadowRes is: \t\t%f\t\tArcaneRes is: \t\t%f",GetResistance(SPELL_SCHOOL_SHADOW), GetResistance(SPELL_SCHOOL_ARCANE));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetAttackTime(BASE_ATTACK), GetAttackTime(RANGED_ATTACK));
}

/*********************************************************/
/***              LOW LEVEL FUNCTIONS:Notifiers        ***/
/*********************************************************/

void Player::SendOutOfRange(Object* obj)
{
    UpdateData his_data;
    WorldPacket his_pk;
    obj->BuildOutOfRangeUpdateBlock(&his_data);
    his_data.BuildPacket(&his_pk);
    GetSession()->SendPacket(&his_pk);
}

inline void Player::SendAttackSwingNotInRange()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTINRANGE, 0);
    GetSession()->SendPacket( &data );
}

void Player::SavePositionInDB(uint32 mapid, float x,float y,float z,float o,uint64 guid)
{
    std::ostringstream ss2;
    ss2 << "UPDATE `character` SET `position_x`='"<<x<<"',`position_y`='"<<y
        << "',`position_z`='"<<z<<"',`orientation`='"<<o<<"',`map`='"<<mapid
        << "' WHERE `guid`='"<< GUID_LOPART(guid) <<"'";
    sDatabase.Execute(ss2.str().c_str());
}

bool Player::SaveValuesArrayInDB(vector<string> const& tokens, uint64 guid)
{
    std::ostringstream ss2;
    ss2<<"UPDATE `character` SET `data`='";
    vector<string>::const_iterator iter;
    int i=0;
    for (iter = tokens.begin(); iter != tokens.end(); ++iter, ++i)
    {
        ss2<<tokens[i]<<" ";
    }
    ss2<<"' WHERE `guid`='"<< GUID_LOPART(guid) <<"'";

    return sDatabase.Execute(ss2.str().c_str());
}

void Player::SetUInt32ValueInArray(vector<string>& tokens,uint16 index, uint32 value)
{
    char buf[11];
    snprintf(buf,11,"%u",value);
    tokens[index] = buf;
}

void Player::SetUInt32ValueInDB(uint16 index, uint32 value, uint64 guid)
{
    vector<string> tokens;
    if(!LoadValuesArrayFromDB(tokens,guid))
        return;

    char buf[11];
    snprintf(buf,11,"%u",value);
    tokens[index] = buf;

    SaveValuesArrayInDB(tokens,guid);
}

void Player::SetFloatValueInDB(uint16 index, float value, uint64 guid)
{
    uint32 temp;
    memcpy(&temp, &value, sizeof(value));
    Player::SetUInt32ValueInDB(index, temp, guid);
}

inline void Player::SendAttackSwingNotStanding()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTSTANDING, 0);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingDeadTarget()
{
    WorldPacket data(SMSG_ATTACKSWING_DEADTARGET, 0);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingCantAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_CANT_ATTACK, 0);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingCancelAttack()
{
    WorldPacket data(SMSG_CANCEL_COMBAT, 0);
    GetSession()->SendPacket( &data );
}

inline void Player::SendAttackSwingBadFacingAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_BADFACING, 0);
    GetSession()->SendPacket( &data );
}

void Player::PlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    if (OnlySelf)
        GetSession()->SendPacket( &data );
    else
        SendMessageToSet( &data, true );
}

void Player::SendExplorationExperience(uint32 Area, uint32 Experience)
{
    WorldPacket data( SMSG_EXPLORATION_EXPERIENCE, 8 );
    data << Area;
    data << Experience;
    GetSession()->SendPacket(&data);
}

void Player::SendDungeonDifficulty()
{
    WorldPacket data(MSG_SET_DUNGEON_DIFFICULTY, 12);
    data << m_dungeonDifficulty;
    data << uint32(0x00000001);
    data << uint32(0x00000000);
    GetSession()->SendPacket(&data);
}

/*********************************************************/
/***              Update timers                        ***/
/*********************************************************/

void Player::UpdatePvPFlag(time_t currTime)
{
    if(!IsPvP() || pvpInfo.endTimer == 0) return;
    if(currTime < (pvpInfo.endTimer + 300)) return;

    UpdatePvP(false);
}

void Player::UpdateDuelFlag(time_t currTime)
{
    if(!duel || duel->startTimer == 0) return;
    if(currTime < duel->startTimer + 3) return;

    SetUInt32Value(PLAYER_DUEL_TEAM, 1);
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 2);

    duel->startTimer = 0;
    duel->startTime  = currTime;
    duel->opponent->duel->startTimer = 0;
    duel->opponent->duel->startTime  = currTime;
}

void Player::RemovePet(Pet* pet, PetSaveMode mode)
{
    if(!pet)
        pet = GetPet();

    if(!pet || pet->GetOwnerGUID()!=GetGUID())
        return;

    // only if current pet in slot
    if(GetPetGUID()==pet->GetGUID())
        SetPet(0);

    pet->CombatStop(true);

    pet->SavePetToDB(mode);

    SendDestroyObject(pet->GetGUID());

    pet->CleanupCrossRefsBeforeDelete();
    ObjectAccessor::Instance().AddObjectToRemoveList(pet);
    pet->m_removed = true;

    if(pet->isControlled())
    {
        WorldPacket data(SMSG_PET_SPELLS, 8);
        data << uint64(0);
        GetSession()->SendPacket(&data);
    }
}

void Player::Uncharm()
{
    Creature* charm = GetCharm();
    if(!charm) return;

    SetCharm(0);

    CreatureInfo const *cinfo = charm->GetCreatureInfo();
    charm->SetUInt64Value(UNIT_FIELD_CHARMEDBY,0);
    charm->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);

    charm->AIM_Initialize();
    WorldPacket data(SMSG_PET_SPELLS, 8);
    data << uint64(0);
    GetSession()->SendPacket(&data);
}

void Player::Say(const std::string text, const uint32 language)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_SAY;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint64)GetGUID();
    data << (uint32)(text.length()+1);
    data << text;
    data << (uint8)chatTag();

    SendMessageToSet(&data, true);
}

void Player::Yell(const std::string text, const uint32 language)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_YELL;
    data << (uint32)language;
    data << (uint64)GetGUID();
    data << (uint64)GetGUID();
    data << (uint32)(text.length()+1);
    data << text;
    data << (uint8)chatTag();

    SendMessageToSet(&data, true);
}

void Player::TextEmote(const std::string text)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_EMOTE;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)GetGUID();
    data << (uint32)(text.length()+1);
    data << text;
    data << (uint8)chatTag();

    if(sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
        SendMessageToSet(&data, true);
    else
        SendMessageToOwnTeamSet(&data,true);
}

void Player::Whisper(const uint64 receiver, const std::string text, const uint32 language)
{
    Player *player = objmgr.GetPlayer(receiver);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_WHISPER;
    data << (uint32)LANG_UNIVERSAL;
    data << (uint64)GetGUID();
    data << (uint32)(text.length()+1);
    data << text;
    data << (uint8)chatTag();
    player->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_MESSAGECHAT, 200);
    data << (uint8)CHAT_MSG_WHISPER_INFORM;
    data << (uint32)language;
    data << (uint64)player->GetGUID();
    data << (uint32)(text.length()+1);
    data << text;
    data << (uint8)chatTag();
    GetSession()->SendPacket(&data);

    if(player->isAFK())
    {
    }
    if(player->isDND())
    {
    }

    if(!isAcceptWhispers())
    {
        SetAcceptWhispers(true);
        sChatHandler.SendSysMessage(GetSession() ,"Whispers accepting now: ON");
    }
}

void Player::PetSpellInitialize()
{
    Pet* pet = GetPet();

    // FIXME: charmed case
    //if(!pet)
    // pet = GetCharm();

    if(pet)
    {
        uint16 Command = 7;
        uint16 State = 6;
        uint8 addlist = 0;
        uint8 act_state = 0;

        sLog.outDebug("Pet Spells Groups");

        if (pet->HasActState(STATE_RA_PASSIVE))
            act_state = 0;
        if (pet->HasActState(STATE_RA_REACTIVE))
            act_state = 1;
        if (pet->HasActState(STATE_RA_PROACTIVE))
            act_state = 2;

        WorldPacket data(SMSG_PET_SPELLS, 100);             // we guess size

        data << (uint64)pet->GetGUID() << uint32(0x00000000) << uint8(act_state) << uint8(1) << uint16(0);

        data << uint16 (2) << uint16(Command << 8);         // 2 command from 0x700 group place to 1 slot
        data << uint16 (1) << uint16(Command << 8);         // 1 command from 0x700 group place to 2 slot
        data << uint16 (0) << uint16(Command << 8);         // 0 command from 0x700 group place to 3 slot

        for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
        {
            if(!pet->m_spells[i])
            {
                data << uint16(pet->m_spells[i]) << uint16((i+2) << 8);
            }
            else
            {
                if (pet->HasActState(STATE_RA_SPELL1 << i))
                                                            // Spell enabled
                    data << uint16 (pet->m_spells[i]) << uint16 (0xC100);
                else
                                                            // Spell disabled
                    data << uint16 (pet->m_spells[i]) << uint16 (0x8100);
            }
        }

        data << uint16 (2) << uint16(State << 8);           // 2 command from 0x600 group place to 8 slot
        data << uint16 (1) << uint16(State << 8);           // 1 command from 0x600 group place to 9 slot
        data << uint16 (0) << uint16(State << 8);           // 0 command from 0x600 group place to 10 slot

        if(pet->isControlled())
        {
            for(PlayerSpellMap::iterator itr = m_spells.begin();itr != m_spells.end();itr++)
            {
                if(itr->second->active == 4)
                    addlist++;
            }
        }

        data << uint8(addlist);
        /*data << uint8(0x08); // just for testing pet spell book
        data << uint32(0x0100433a);
        data << uint32(0x0100105f);
        data << uint32(0x01005fe8);
        data << uint32(0x01005f77);
        data << uint32(0x01005f7f);
        data << uint32(0x01005fb6);
        data << uint32(0x01005fb1);
        data << uint32(0x01005fb9);*/

        if(pet->isControlled())
        {
            for(PlayerSpellMap::iterator itr = m_spells.begin();itr != m_spells.end();itr++)
            {
                if(itr->second->active == 4)
                {
                    bool hasthisspell = false;

                    SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                    data << uint16(spellInfo->EffectTriggerSpell[0]);
                    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
                    {
                        if(pet->m_spells[i] == spellInfo->EffectTriggerSpell[0])
                        {
                            if (pet->HasActState(STATE_RA_SPELL1 << i))
                                data << uint16(0xC100);     // Spell enabled
                            else
                                data << uint16(0x8100);     // Spell disabled
                            hasthisspell = true;
                            break;
                        }
                    }
                    if(!hasthisspell)
                        data << uint16(0x0100);
                }
            }
        }

        uint8 count = 3;
        // if count = 0, then end of packet...
        data << count;
        // uint32 value is spell id...
        // uint64 value is constant 0, unknown...
        data << uint32(0x6010) << uint64(0);                // if count = 1, 2 or 3
        //data << uint32(0x5fd1) << uint64(0);  // if count = 2
        data << uint32(0x8e8c) << uint64(0);                // if count = 3
        data << uint32(0x8e8b) << uint64(0);                // if count = 3

        GetSession()->SendPacket(&data);
    }
}

int32 Player::GetTotalFlatMods(uint32 spellId, uint8 op)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
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
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
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

void Player::RemoveAreaAurasFromGroup()
{
    Group* pGroup = groupInfo.group;
    if(!pGroup)
        return;

    Group::MemberList const& members = pGroup->GetMembers();
    for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
    {
        if(!pGroup->SameSubGroup(GetGUID(), &*itr))
            continue;

        Unit* Member = objmgr.GetPlayer(itr->guid);
        if(!Member)
            continue;

        Member->RemoveAreaAurasByOthers(GetGUID());
        for (uint8 i = 0; i < 4; i++)
            if (m_TotemSlot[i])
                Member->RemoveAreaAurasByOthers(m_TotemSlot[i]);
    }
}

// send Proficiency
void Player::SendProficiency(uint8 pr1, uint32 pr2)
{
    WorldPacket data(SMSG_SET_PROFICIENCY, 8);
    data << pr1 << pr2;
    GetSession()->SendPacket (&data);
}

void Player::RemovePetitionsAndSigns(uint64 guid)
{
    QueryResult *result = sDatabase.Query("SELECT `ownerguid`,`charterguid` FROM `guild_charter_sign` WHERE `playerguid` = '%u'", guid);
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint64 ownerguid   = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);
            uint64 charterguid = MAKE_GUID(fields[1].GetUInt32(),HIGHGUID_ITEM);

            // send update  if charter owner in game
            Player* owner = objmgr.GetPlayer(ownerguid);
            if(owner)
                owner->GetSession()->SendPetitionQueryOpcode(charterguid);

        } while ( result->NextRow() );

        delete result;

        sDatabase.Execute("DELETE FROM `guild_charter_sign` WHERE `playerguid` = '%u'",guid);
    }

    //sDatabase.BeginTransaction();
    sDatabase.Execute("DELETE FROM `guild_charter` WHERE `ownerguid` = '%u'",guid);
    sDatabase.Execute("DELETE FROM `guild_charter_sign` WHERE `ownerguid` = '%u'",guid);
    //sDatabase.CommitTransaction();
}

void Player::SetRestBonus (float rest_bonus_new)
{
    if(rest_bonus_new < 0)
        rest_bonus_new = 0;

    float rest_bonus_max = (float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)/2;

    if(rest_bonus_new > rest_bonus_max)
        m_rest_bonus = rest_bonus_max;
    else
        m_rest_bonus = rest_bonus_new;

    // update data for client
    if(m_rest_bonus>10)
    {
        SetFlag(PLAYER_BYTES_2, 0x1000000);                 // Set Reststate = Rested
        RemoveFlag(PLAYER_BYTES_2, 0x2000000);              // Remove Reststate = Normal
    }
    else if(m_rest_bonus<=1)
    {
        SetFlag(PLAYER_BYTES_2, 0x2000000);                 // Set Reststate = Normal
        RemoveFlag(PLAYER_BYTES_2, 0x1000000);              // Remove Reststate = Rested
    }

    //RestTickUpdate
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, uint32(m_rest_bonus));
}

void Player::HandleInvisiblePjs()
{
    Map *m = MapManager::Instance().GetMap(GetMapId(), this);

    //this is to be sure that InvisiblePjsNear vector has active pjs only.
    m->PlayerRelocation(this, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation(), true);

    for (std::vector<Player *>::iterator i = InvisiblePjsNear.begin(); i != InvisiblePjsNear.end(); i++)
    {
        if ((*i)->isVisibleFor(this,true))
        {
            m_DiscoveredPj = *i;
            m_enableDetect = false;
            m->PlayerRelocation(this, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation(), true);
            m_enableDetect = true;
            m_DiscoveredPj = 0;
        }
    }
    if (!InvisiblePjsNear.size())
        m_DetectInvTimer = 0;
    InvisiblePjsNear.clear();
}

bool Player::ActivateTaxiPathTo(std::vector<uint32> const& nodes )
{
    if(nodes.size() < 2)
        return false;

    // not let cheating with start flight mounted
    if(IsMounted())
    {
        WorldPacket data(SMSG_CAST_RESULT, 6);
        data << uint32(0);
        data << uint8(2);
        data << uint8(SPELL_FAILED_NOT_MOUNTED);
        GetSession()->SendPacket(&data);
        return false;
    }

    // not let cheating with start flight in time of logout process
    if(GetSession()->isLogingOut())
    {
        WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
        data << uint32( 7 );                                // you can't used taxi service now
        GetSession()->SendPacket( &data );
        return false;
    }

    if( HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE ))
        return false;

    // not let flight if casting not finished
    if(m_currentSpell)
    {
        WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
        data << uint32( 7 );                                // you can't used taxi service now
        GetSession()->SendPacket( &data );
        return false;
    }

    uint32 curloc = objmgr.GetNearestTaxiNode( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId() );

    uint32 sourcenode = nodes[0];

    // starting node != nearest node (cheat?)
    if(curloc != sourcenode)
    {
        WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
        data << uint32( 4 );
        GetSession()->SendPacket( &data );
        return false;
    }

    uint32 sourcepath = 0;
    uint32 totalcost = 0;

    uint32 prevnode = sourcenode;
    uint32 lastnode = 0;

    ClearTaxiDestinations();

    for(uint32 i = 1; i < nodes.size(); ++i)
    {
        uint32 path, cost;

        lastnode =  nodes[i];
        objmgr.GetTaxiPath( prevnode, lastnode, path, cost);

        if(!path)
            break;

        totalcost += cost;

        if(prevnode == sourcenode)
            sourcepath = path;

        AddTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    uint16 MountId = objmgr.GetTaxiMount(sourcenode, GetTeam());

    if ( MountId == 0 || sourcepath == 0)
    {
        WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
        data << uint32( 1 );
        GetSession()->SendPacket( &data );
        ClearTaxiDestinations();
        return false;
    }

    uint32 money = GetMoney();
    if(money < totalcost )
    {
        WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
        data << uint32( 3 );
        GetSession()->SendPacket( &data );
        ClearTaxiDestinations();
        return false;
    }

    // Save before flight (player must loaded in start taxinode is disconnected at flight,etc)
    SaveToDB(false);

    // unsummon pet, it will be lost anyway
    RemovePet(NULL,PET_SAVE_NOT_IN_SLOT);

    //Checks and preparations done, DO FLIGHT
    setDismountCost( money - totalcost);

    WorldPacket data( SMSG_ACTIVATETAXIREPLY, 4 );
    data << uint32( 0 );
    GetSession()->SendPacket( &data );

    sLog.outDebug( "WORLD: Sent SMSG_ACTIVATETAXIREPLY" );

    GetSession()->SendDoFlight( MountId, sourcepath );

    return true;
}

void Player::ProhibitSpellScholl(uint32 idSchool /* from SpellSchools */, uint32 unTimeMs )
{
                                                            // last check 2.0.10
    WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+m_spells.size()*8);
    data << GetGUID();
    data << uint8(0x0);
    time_t curTime = time(NULL);
    for(PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        // WTF, we can send multiple cooldowns in one packet? I don't think so...
        if (itr->second->state == PLAYERSPELL_REMOVED)
            continue;
        uint32 unSpellId = itr->first;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(unSpellId);
        if (!spellInfo)
        {
            ASSERT(spellInfo);
            continue;
        }
        if(idSchool == spellInfo->School && GetSpellCooldownDelay(unSpellId) < unTimeMs )
        {
            data << unSpellId;
            data << unTimeMs;                               // in m.secs
            AddSpellCooldown(unSpellId, 0, curTime + unTimeMs/1000);
        }
    }
    GetSession()->SendPacket(&data);
}

void Player::InitDataForForm()
{
    switch(m_form)
    {
        case FORM_CAT:
        {
            SetAttackTime(BASE_ATTACK,1000);                //Speed 1
            SetAttackTime(OFF_ATTACK,1000);                 //Speed 1
            uint32 tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
            float val = tem_att_power/14.0f + getLevel();
                                                            // Damage in cat form (Correct ???)
            SetFloatValue(UNIT_FIELD_MINDAMAGE, val*0.9);
            SetFloatValue(UNIT_FIELD_MAXDAMAGE, val*1.1);
            SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, val*0.9);
            SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, val*1.1);
            if(getPowerType()!=POWER_ENERGY) setPowerType(POWER_ENERGY);
            break;
        }
        case FORM_BEAR:
        case FORM_DIREBEAR:
        {
            SetAttackTime(BASE_ATTACK,2500);                //Speed 2.5
            SetAttackTime(OFF_ATTACK,2500);                 //Speed 2.5
            uint32 tem_att_power = GetUInt32Value(UNIT_FIELD_ATTACK_POWER) + GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
            float val = tem_att_power/14.0f + getLevel();
                                                            // Damage in Bear forms (Correct ???)
            SetFloatValue(UNIT_FIELD_MINDAMAGE, val*0.9);
            SetFloatValue(UNIT_FIELD_MAXDAMAGE, val*1.1);
            SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, val*0.9);
            SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, val*1.1);
            if(getPowerType()!=POWER_RAGE) setPowerType(POWER_RAGE);
            break;
        }
        default:                                            // 0, for example
        {
            SetAttackTime(BASE_ATTACK,   2000 );
            SetAttackTime(OFF_ATTACK,    2000 );

            SetFloatValue(UNIT_FIELD_MINDAMAGE, 0 );
            SetFloatValue(UNIT_FIELD_MAXDAMAGE, 0 );
            SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, 0 );
            SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, 0 );

            ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(getClass());
            if(cEntry && cEntry->powerType < MAX_POWERS && uint32(getPowerType()) != cEntry->powerType)
                setPowerType(Powers(cEntry->powerType));
            break;
        }
    }

    SetAttackTime(RANGED_ATTACK, 2000 );
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, 0 );
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, 0 );

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER,            0 );
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,       0 );
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER,     0 );
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,0 );

    SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, 1.00);
    SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, 0);
    SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS, 0);
}

void Player::ApplySpeedMod(UnitMoveType mtype, float rate, bool forced, bool apply)
{
    if(forced)
        ++m_forced_speed_changes[mtype];                    // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
    Unit::ApplySpeedMod(mtype,rate,forced,apply);
}

void Player::BuyItemFromVendor(uint64 vendorguid, uint32 item, uint8 count, uint64 bagguid, uint8 slot)
{
    // cheating attempt
    if(count < 1) count = 1;

    if(!isAlive())
        return;

    ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
    if( pProto )
    {
        Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*this, vendorguid,UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
        {
            sLog.outDebug( "WORLD: BuyItemFromVendor - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(vendorguid)) );
            SendBuyError( BUY_ERR_DISTANCE_TOO_FAR, NULL, item, 0);
            return;
        }

        // load vendor items if not yet
        pCreature->LoadGoods();

        CreatureItem* crItem = pCreature->FindItem(item);
        if(!crItem)
        {
            SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
            return;
        }

        if( crItem->maxcount != 0 && crItem->count < count )
        {
            SendBuyError( BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
            return;
        }
        if( getLevel() < pProto->RequiredLevel )
        {
            SendBuyError( BUY_ERR_LEVEL_REQUIRE, pCreature, item, 0);
            return;
        }
        if(pProto->ExtendedCost)
        {
            ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(pProto->ExtendedCost);
            if(iece)
            {
                if(GetHonorPoints() < iece->reqhonorpoints)
                {
                    SendEquipError(EQUIP_DONT_HAVE_ENOUGHT_HONOR_POINTS, NULL, NULL);
                    return;
                }
                if(GetArenaPoints() < iece->reqarenapoints)
                {
                    SendEquipError(EQUIP_DONT_HAVE_ENOUGHT_ARENA_POINTS, NULL, NULL);
                    return;
                }
                if( (iece->reqitem1 && !HasItemCount(iece->reqitem1, iece->reqitemcount1)) ||
                    (iece->reqitem2 && !HasItemCount(iece->reqitem2, iece->reqitemcount2)) ||
                    (iece->reqitem3 && !HasItemCount(iece->reqitem3, iece->reqitemcount3)) )
                {
                    SendEquipError(EQUIP_DONT_HAVE_REQITEMS_FOR_THAT_PURCHASE, NULL, NULL);
                    return;
                }
            }
            else
            {
                sLog.outError("Item %u have wrong ExtendedCost field value %u", pProto->ItemId, pProto->ExtendedCost);
                return;
            }
        }

        // 10% reputation discount
        uint32 price  = pProto->BuyPrice * count;
        FactionTemplateEntry const* vendor_faction = pCreature->getFactionTemplateEntry();
        if (vendor_faction && GetReputationRank(vendor_faction->faction) >= REP_HONORED)
            price = 9 * price / 10;

        if( GetMoney() < price )
        {
            SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
            return;
        }

        uint8 bag = 0;                                      // init for case invalid bagGUID
        uint16 dest = 0;

        if (bagguid != NULL_BAG && slot != NULL_SLOT)
        {
            Bag *pBag;
            if( bagguid == GetGUID() )
            {
                bag = INVENTORY_SLOT_BAG_0;
            }
            else
            {
                for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END;i++)
                {
                    pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0,i);
                    if( pBag )
                    {
                        if( bagguid == pBag->GetGUID() )
                        {
                            bag = i;
                            break;
                        }
                    }
                }
            }
            dest = ((bag << 8) | slot);
        }

        uint8 msg;
        if( IsInventoryPos( dest ) || (bagguid == NULL_BAG && slot == NULL_SLOT) )
        {
            msg = CanStoreNewItem( bag, slot, dest, item, pProto->BuyCount * count, false );
            if( msg == EQUIP_ERR_OK )
            {
                ModifyMoney( -(int32)price );
                if(pProto->ExtendedCost)                    // case for new honor system
                {
                    ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(pProto->ExtendedCost);
                    if(iece->reqhonorpoints)
                        SetHonorPoints(GetHonorPoints() - iece->reqhonorpoints);
                    if(iece->reqarenapoints)
                        SetArenaPoints(GetArenaPoints() - iece->reqarenapoints);
                    if(iece->reqitem1)
                        DestroyItemCount(iece->reqitem1, iece->reqitemcount1, true);
                    if(iece->reqitem2)
                        DestroyItemCount(iece->reqitem2, iece->reqitemcount2, true);
                    if(iece->reqitem3)
                        DestroyItemCount(iece->reqitem3, iece->reqitemcount3, true);
                }
                Item *it = StoreNewItem( dest, item, pProto->BuyCount * count, true );
                if( crItem->maxcount != 0 )
                    crItem->count -= pProto->BuyCount * count;

                WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
                data << pCreature->GetGUID();
                data << (uint32)crItem->id;                 // entry
                data << (uint32)crItem->count;
                data << (uint32)count;
                GetSession()->SendPacket(&data);

                SendNewItem(it, count, true, false, false);
            }
            else
                SendEquipError( msg, NULL, NULL );
        }
        else if( IsEquipmentPos( dest ) )
        {
            msg = CanEquipNewItem( slot, dest, item, pProto->BuyCount * count, false );
            if( msg == EQUIP_ERR_OK )
            {
                ModifyMoney( -(int32)price );
                if(pProto->ExtendedCost)                    // case for new honor system
                {
                    ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(pProto->ExtendedCost);
                    if(iece->reqhonorpoints)
                        SetHonorPoints(GetHonorPoints() - iece->reqhonorpoints);
                    if(iece->reqarenapoints)
                        SetArenaPoints(GetArenaPoints() - iece->reqarenapoints);
                    if(iece->reqitem1)
                        DestroyItemCount(iece->reqitem1, iece->reqitemcount1, true);
                    if(iece->reqitem2)
                        DestroyItemCount(iece->reqitem2, iece->reqitemcount2, true);
                    if(iece->reqitem3)
                        DestroyItemCount(iece->reqitem3, iece->reqitemcount3, true);
                }
                Item *it = EquipNewItem( dest, item, pProto->BuyCount * count, true );
                if( crItem->maxcount != 0 )
                    crItem->count -= pProto->BuyCount * count;

                WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
                data << pCreature->GetGUID();
                data << (uint32)crItem->id;                 // entry
                data << (uint32)crItem->count;
                data << (uint32)count;
                GetSession()->SendPacket(&data);

                SendNewItem(it, count, true, false, false);
            }
            else
                SendEquipError( msg, NULL, NULL );
        }
        else
            SendEquipError( EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, NULL, NULL );
    }
    else
        SendBuyError( BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
}

void Player::_LoadGroup()
{
    QueryResult *result = sDatabase.Query("SELECT `leaderGuid` FROM `group_member` WHERE `memberGuid`='%u'", GetGUIDLow());
    if(result)
    {
        uint64 leaderGuid = MAKE_GUID(result->Fetch()[0].GetUInt32(),HIGHGUID_PLAYER);
        delete result;
        groupInfo.group = objmgr.GetGroupByLeader(leaderGuid);
    }
}

void Player::_LoadBoundInstances()
{
    m_BoundInstances.clear();

    QueryResult *result = sDatabase.Query("SELECT `map`,`instance`,`leader` FROM `character_instance` WHERE `guid` = '%u'", GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            m_BoundInstances[fields[0].GetUInt32()] = std::pair< uint32, uint32 >(fields[1].GetUInt32(), fields[2].GetUInt32());
        } while(result->NextRow());
        delete result;
    }

    // correctly set current instance (if needed)
    BoundInstancesMap::iterator i = m_BoundInstances.find(GetMapId());
    if (i != m_BoundInstances.end()) SetInstanceId(i->second.first); else SetInstanceId(0);
}

void Player::_SaveBoundInstances()
{
    sDatabase.Execute("DELETE FROM `character_instance` WHERE (`guid` = '%u')", GetGUIDLow());
    for(BoundInstancesMap::iterator i = m_BoundInstances.begin(); i != m_BoundInstances.end(); i++)
    {
        sDatabase.Execute("INSERT INTO `character_instance` (`guid`,`map`,`instance`,`leader`) VALUES ('%u', '%u', '%u', '%u')", GetGUIDLow(), i->first, i->second.first, i->second.second);
    }
}

void Player::UpdateHomebindTime(uint32 time)
{
    // GMs never get homebind timer online
    if (m_InstanceValid || isGameMaster())
    {
        if(m_HomebindTimer)                                 // instance valid, but timer not reset
        {
            // hide reminder
            WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
            data << uint32(0);
            data << uint32(0);
            GetSession()->SendPacket(&data);
        }
        // instance is valid, reset homebind timer
        m_HomebindTimer = 0;
    }
    else if (m_HomebindTimer > 0)
    {
        if (time >= m_HomebindTimer)
        {
            // teleport to homebind location
            TeleportTo(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, GetOrientation());
        }
        else
        {
            uint32 oldTimer = m_HomebindTimer;
            m_HomebindTimer -= time;
        }
    }
    else
    {
        // instance is invalid, start homebind timer
        m_HomebindTimer = 60000;
        // send message to player
        WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
        data << m_HomebindTimer;
        data << uint32(1);
        GetSession()->SendPacket(&data);
        sLog.outDebug("PLAYER: Player '%s' will be teleported to homebind in 60 seconds", GetName());
    }
}

void Player::UpdatePvP(bool state, bool ovrride)
{
    if(!state || ovrride)
    {
        SetPvP(state);
        if(Pet* pet = GetPet())
            pet->SetPvP(state);
        if(Creature* charmed = GetCharm())
            charmed->SetPvP(state);

        pvpInfo.endTimer = 0;
    }
    else
    {
        if(pvpInfo.endTimer != 0)
            pvpInfo.endTimer = time(NULL);
        else
        {
            SetPvP(state);

            if(Pet* pet = GetPet())
                pet->SetPvP(state);
            if(Creature* charmed = GetCharm())
                charmed->SetPvP(state);
        }
    }
}

void Player::SendAllowMove()
{
    WorldPacket data(SMSG_ALLOW_MOVE, 4);                   // new 2.0.x, enable movement
    data << uint32(0x00000000);                             // on blizz it increments periodically
    GetSession()->SendPacket(&data);
}

void Player::AddSpellCooldown(uint32 spellid, uint32 itemid, time_t end_time)
{
    SpellCooldown sc;
    sc.end = end_time;
    sc.itemid = itemid;
    m_spellCooldowns[spellid] = sc;
}

uint32 Player::GetBlockValue() const
{
    if(m_AuraModifiers[SPELL_AURA_MOD_SHIELD_BLOCKVALUE] <= 0)
        return 0;

    if(m_AuraModifiers[SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT] <= -100)
        return 0;

    return m_AuraModifiers[SPELL_AURA_MOD_SHIELD_BLOCKVALUE]*(m_AuraModifiers[SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT]+100)/100;
}

bool Player::EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot)    //slot to be excluded while counting
{
    if(!enchantmentcondition)
        return true;

    SpellItemEnchantmentConditionEntry const *Condition = sSpellItemEnchantmentConditionStore.LookupEntry(enchantmentcondition);

    if(!Condition)
            return true;

    uint8 curcount[4] = {0, 0, 0, 0};

    //counting current equipped gem colors
    for(uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if(i == slot)
                continue;
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i );
        Item *pItem2 = GetItemByPos( pos );
        if(pItem2 && pItem2->GetProto()->Socket[0].Color)
        {
            for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
            {
                uint32 enchant_id = pItem2->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if(!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if(!enchantEntry)
                    continue;

                uint32 gemid = enchantEntry->GemID;
                if(!gemid)
                    continue;

                ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
                if(!gemProto)
                    continue;

                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if(!gemProperty)
                    continue;

                uint8 GemColor = gemProperty->color;

                for(uint8 b = 0, tmpcolormask = 1; b < 4; b++, tmpcolormask <<= 1)
                {
                    if(tmpcolormask & GemColor)
                        curcount[b]++;
                }
            }
        }
    }

    bool activate = true;

    for(int i = 0; i < 5; i++)
    {
        if(!Condition->Color[i])
            continue;

        uint32 _cur_gem = curcount[Condition->Color[i] - 1];

        // if have <CompareColor> use them as count, else use <value> from Condition
        uint32 _cmp_gem = Condition->CompareColor[i] ? curcount[Condition->CompareColor[i] - 1]: Condition->Value[i];

        switch(Condition->Comparator[i]) {
            case 2: // requires less <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem < _cmp_gem) ? true : false;
                break;
            case 3: // requires more <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem > _cmp_gem) ? true : false;
                break;
            case 5: // requires at least <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem >= _cmp_gem) ? true : false;
                break;
        }
    }

    sLog.outDebug("Checking Condition %u, there are %u Meta Gems, %u Red Gems, %u Yellow Gems and %u Blue Gems, Activate:%s", enchantmentcondition, curcount[0], curcount[1], curcount[2], curcount[3], activate ? "yes" : "no");

    return activate;
}


void Player::CorrectMetaGemEnchants(uint8 exceptslot, bool apply)
{

    for(uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)    //cycle all equipped items
    {
        //enchants for the slot being socketed are handeled by Player::ApplyItemMods
        if(slot == exceptslot)
            continue;

        uint16 pos = ( (INVENTORY_SLOT_BAG_0 << 8) | slot );
        Item* pItem = GetItemByPos( pos );

        if(!pItem || !pItem->GetProto()->Socket[0].Color)
            continue;

        for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if(!enchant_id)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if(!enchantEntry)
                continue;

            uint32 condition = enchantEntry->EnchantmentCondition;
            if(condition)
            {
                bool wasactive = EnchantmentFitsRequirements(condition, apply ? exceptslot : -1);    //was enchant active with/without item?
                if(wasactive ^ EnchantmentFitsRequirements(condition, apply ? -1 : exceptslot))    //should it now be?
                {
                    // ignore item gem conditions
                    ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot),!wasactive,true,true);                            //if state changed, (dis)apply enchant
                }
            }
        }
    }
}

void Player::ToggleMetaGemsActive(uint16 exceptslot, bool apply)        //if false -> then toggled off if was on| if true -> toggled on if was off AND meets requirements
{
    for(int slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)    //cycle all equipped items
    {
        //enchants for the slot being socketed are handeled by WorldSession::HandleSocketOpcode(WorldPacket& recv_data)
        if(slot == exceptslot) continue;

        uint16 pos = ( (INVENTORY_SLOT_BAG_0 << 8) | slot );
        Item *pItem = GetItemByPos( pos );

        if(!pItem || !pItem->GetProto()->Socket[0].Color)    //if item has no sockets or no item is equipped go to next item
            continue;

        //cycle all (gem)enchants
        for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if(!enchant_id)                                    //if no enchant go to next enchant(slot)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if(!enchantEntry)
                continue;

            //only metagems to be (de)activated, so only enchants with condition
            uint32 condition = enchantEntry->EnchantmentCondition;
            if(condition)
                ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot), apply);
        }
    }
}

bool Player::DropBattleGroundFlag()
{
    if(InBattleGround())
        return false;

    BattleGround *bg = sBattleGroundMgr.GetBattleGround(GetBattleGroundId());
    if(!bg)
        return false;

    if(GetTeam() == HORDE)
    {
        if(bg->IsAllianceFlagPickedup())
        {
            if(bg->GetAllianceFlagPickerGUID() == GetGUID())
            {
                bg->SetAllianceFlagPicker(0);
                RemoveAurasDueToSpell(23335);
                CastSpell(this,23336,true,NULL);
                return true;
            }
        }
    }
    else                                                    // ALLIANCE
    {
        if(bg->IsHordeFlagPickedup())
        {
            if(bg->GetHordeFlagPickerGUID() == GetGUID())
            {
                bg->SetHordeFlagPicker(0);
                RemoveAurasDueToSpell(23333);
                CastSpell(this,23334,true,NULL);
                return true;
            }
        }
    }
    return false;
}
