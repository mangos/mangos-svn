/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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
#include "SpellMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "WaypointMovementGenerator.h"
#include "InstanceData.h"
#include "BattleGround.h"

// apply implementation of the singletons
#include "Policies/SingletonImp.h"

Creature::Creature( WorldObject *instantiator ) :
Unit( instantiator ), i_AI(NULL),
lootForPickPocketed(false), lootForBody(false), m_groupLootTimer(0), lootingGroupLeaderGUID(0),
m_itemsLoaded(false), m_trainerSpellsLoaded(false), m_trainer_type(0), m_lootMoney(0), m_lootRecipient(0),
m_deathTimer(0), m_respawnTime(0), m_respawnDelay(25), m_corpseDelay(60), m_respawnradius(0.0f),
m_gossipOptionLoaded(false),m_NPCTextId(0), m_emoteState(0), m_isPet(false), m_isTotem(false),
m_regenTimer(2000), m_defaultMovementType(IDLE_MOTION_TYPE), m_regenHealth(true), m_equipmentId(0),
m_AI_locked(false)
{
    m_valuesCount = UNIT_END;

    for(int i =0; i<3; ++i) respawn_cord[i] = 0.0f;

    for(int i =0; i<4; ++i)
        m_spells[i] = 0;

    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_GlobalCooldown = 0;

    m_AlreadyCallAssistence = false;
}

Creature::~Creature()
{
    CleanupsBeforeDelete();

    m_trainer_spells.clear();
    m_vendor_items.clear();

    delete i_AI;
    i_AI = NULL;
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if(!IsInWorld()) ObjectAccessor::Instance().AddObject(this);
    Object::AddToWorld();
}

void Creature::RemoveFromWorld()
{
    ///- Remove the creature from the accessor
    if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
    Object::RemoveFromWorld();
}

void Creature::LoadTrainerSpells()
{
    if(m_trainerSpellsLoaded)
        return;

    m_trainer_spells.clear();
    m_trainer_type = 0;

    Field *fields;
    QueryResult *result = WorldDatabase.PQuery("SELECT `spell`,`spellcost`,`reqskill`,`reqskillvalue`,`reqlevel` FROM `npc_trainer` WHERE `entry` = '%u'", GetEntry());

    if(!result) return;

    do
    {
        fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();
        SpellEntry const *spellinfo = sSpellStore.LookupEntry(spellid);

        if(!spellinfo)
        {
            sLog.outErrorDb("Trainer (Entry: %u ) have in list non existed spell %u",GetEntry(),spellid);
            continue;
        }

        if(spellinfo->Effect[0]!=SPELL_EFFECT_LEARN_SPELL)
        {
            sLog.outErrorDb("LoadTrainerSpells: Trainer (Entry: %u) has not learning spell(%u).", GetEntry(), spellid);
            continue;
        }

        if(!sSpellStore.LookupEntry(spellinfo->EffectTriggerSpell[0]))
        {
            sLog.outErrorDb("LoadTrainerSpells: Trainer (Entry: %u) has learning spell(%u) without triggered spell (bad dbc?).", GetEntry(), spellid);
            continue;
        }

        if(!SpellMgr::IsSpellValid(spellinfo))
        {
            sLog.outErrorDb("LoadTrainerSpells: Trainer (Entry: %u) has broken learning spell %u.", GetEntry(), spellid);
            continue;
        }

        if(SpellMgr::IsProfessionSpell(spellinfo->EffectTriggerSpell[0]))
            m_trainer_type = 2;

        TrainerSpell tspell;
        tspell.spell        = spellinfo;
        tspell.spellcost    = fields[1].GetUInt32();
        tspell.reqskill     = fields[2].GetUInt32();
        tspell.reqskillvalue= fields[3].GetUInt32();
        tspell.reqlevel     = fields[4].GetUInt32();

        m_trainer_spells.push_back(tspell);

    } while( result->NextRow() );

    delete result;

    m_trainerSpellsLoaded = true;
}

void Creature::RemoveCorpse()
{
    if(getDeathState()!=CORPSE)
        return;

    m_deathTimer = 0;
    ObjectAccessor::UpdateObjectVisibility(this);
    loot.clear();
    setDeathState(DEAD);
    m_respawnTime = time(NULL) + m_respawnDelay;

    float x,y,z;
    GetRespawnCoord(x, y, z);
    MapManager::Instance().GetMap(GetMapId(), this)->CreatureRelocation(this,x,y,z,GetOrientation());
}

void Creature::Update(uint32 diff)
{
    if(m_GlobalCooldown <= diff)
        m_GlobalCooldown = 0;
    else
        m_GlobalCooldown -= diff;

    switch( m_deathState )
    {
        case JUST_DIED:
            // Dont must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_DEAD (1)",GetGUIDLow(),GetEntry());
            break;
        case DEAD:
        {
            if( m_respawnTime <= time(NULL) )
            {
                DEBUG_LOG("Respawning...");
                m_respawnTime = 0;
                lootForPickPocketed = false;
                lootForBody         = false;

                CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(this->GetEntry());

                SelectLevel(cinfo);
                SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
                setMoveRunFlag(false);

                SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
                SetHealth(GetMaxHealth());
                setDeathState( ALIVE );
                clearUnitState(UNIT_STAT_ALL_STATE);
                i_motionMaster.Clear();
                LoadCreaturesAddon(true);
                MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
            }
            break;
        }
        case CORPSE:
        {
            if( m_deathTimer <= diff )
            {
                RemoveCorpse();
                DEBUG_LOG("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
            }
            else
            {
                m_deathTimer -= diff;
                if (m_groupLootTimer && lootingGroupLeaderGUID)
                {
                    if(diff <= m_groupLootTimer)
                    {
                        m_groupLootTimer -= diff;
                    }
                    else
                    {
                        Group* group = objmgr.GetGroupByLeader(lootingGroupLeaderGUID);
                        if (group)
                            group->EndRoll();
                        m_groupLootTimer = 0;
                        lootingGroupLeaderGUID = 0;
                    }
                }
            }

            break;
        }
        case ALIVE:
        {
            Unit::Update( diff );

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if(!isAlive())
                break;

            if(!IsInEvadeMode())
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }

            if(m_regenTimer > 0)
            {
                if(diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }
            if (m_regenTimer != 0)
                break;

            if (!isInCombat() || IsPolymorphed())
            {
                RegenerateHealth();
                if (!isInCombat())
                    RegenerateMana();
            }
            m_regenTimer = 2000;
            break;
        }
        default:
            break;
    }
}

void Creature::RegenerateMana()
{
    uint32 curValue = GetPower(POWER_MANA);
    uint32 maxValue = GetMaxPower(POWER_MANA);

    if (curValue >= maxValue)   return;

    float ManaIncreaseRate = sWorld.getRate(RATE_POWER_MANA);

    float Spirit = GetStat(STAT_SPIRIT);

    uint32 addvalue = 0;

    if (isInCombat() || isPet())
        addvalue = uint32((Spirit/5 + 17) * ManaIncreaseRate);
    else
        addvalue = maxValue/3;

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH);

    float Spirit = GetStat(STAT_SPIRIT);

    uint32 addvalue = 0;

    if(isPet())
    {
        if( GetPower(POWER_MANA) > 0 )
            addvalue = uint32(Spirit * 0.25 * HealthIncreaseRate);
        else
            addvalue = uint32(Spirit * 0.80 * HealthIncreaseRate);
    }
    else
        addvalue = maxValue/3;

    ModifyHealth(addvalue);
}

bool Creature::AIM_Initialize()
{
    // make sure nothing can change the AI during AI update
    if(m_AI_locked)
    {
        sLog.outDebug("AIM_Initialize: failed to init, locked.");
        return false;
    }

    CreatureAI * oldAI = i_AI;
    i_motionMaster.Initialize();
    i_AI = FactorySelector::selectAI(this);
    if (oldAI)
        delete oldAI;
    return true;
}

bool Creature::Create (uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry, uint32 team, const CreatureData *data)
{
    respawn_cord[0] = x;
    respawn_cord[1] = y;
    respawn_cord[2] = z;
    SetMapId(mapid);
    Relocate(x,y,z);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",guidlow,Entry,x,y);
        return false;
    }

    SetOrientation(ang);
    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    return  CreateFromProto(guidlow, Entry, team, data);
}

bool Creature::isCanTrainingOf(Player* pPlayer, bool msg) const
{
    if(!isTrainer())
        return false;

    if(m_trainer_spells.empty())
    {
        sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
            GetGUIDLow(),GetCreatureInfo()->Entry);
        return false;
    }

    switch(GetCreatureInfo()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if(pPlayer->getClass()!=GetCreatureInfo()->classNum)
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch(GetCreatureInfo()->classNum)
                    {
                        case CLASS_DRUID:  pPlayer->PlayerTalkClass->SendGossipMenu( 4913,GetGUID()); break;
                        case CLASS_HUNTER: pPlayer->PlayerTalkClass->SendGossipMenu(10090,GetGUID()); break;
                        case CLASS_MAGE:   pPlayer->PlayerTalkClass->SendGossipMenu(  328,GetGUID()); break;
                        case CLASS_PALADIN:pPlayer->PlayerTalkClass->SendGossipMenu( 1635,GetGUID()); break;
                        case CLASS_PRIEST: pPlayer->PlayerTalkClass->SendGossipMenu( 4436,GetGUID()); break;
                        case CLASS_ROGUE:  pPlayer->PlayerTalkClass->SendGossipMenu( 4797,GetGUID()); break;
                        case CLASS_SHAMAN: pPlayer->PlayerTalkClass->SendGossipMenu( 5003,GetGUID()); break;
                        case CLASS_WARLOCK:pPlayer->PlayerTalkClass->SendGossipMenu( 5836,GetGUID()); break;
                        case CLASS_WARRIOR:pPlayer->PlayerTalkClass->SendGossipMenu( 4985,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if(pPlayer->getClass()!=CLASS_HUNTER)
            {
                pPlayer->PlayerTalkClass->ClearMenus();
                pPlayer->PlayerTalkClass->SendGossipMenu(3620,GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if(GetCreatureInfo()->race && pPlayer->getRace() != GetCreatureInfo()->race)
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch(GetCreatureInfo()->classNum)
                    {
                        case RACE_DWARF:        pPlayer->PlayerTalkClass->SendGossipMenu(5865,GetGUID()); break;
                        case RACE_GNOME:        pPlayer->PlayerTalkClass->SendGossipMenu(4881,GetGUID()); break;
                        case RACE_HUMAN:        pPlayer->PlayerTalkClass->SendGossipMenu(5861,GetGUID()); break;
                        case RACE_NIGHTELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_ORC:          pPlayer->PlayerTalkClass->SendGossipMenu(5863,GetGUID()); break;
                        case RACE_TAUREN:       pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                        case RACE_TROLL:        pPlayer->PlayerTalkClass->SendGossipMenu(5816,GetGUID()); break;
                        case RACE_UNDEAD_PLAYER:pPlayer->PlayerTalkClass->SendGossipMenu( 624,GetGUID()); break;
                        case RACE_BLOODELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_DRAENEI:      pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if(GetCreatureInfo()->trainer_spell && !pPlayer->HasSpell(GetCreatureInfo()->trainer_spell))
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->PlayerTalkClass->SendGossipMenu(11031,GetGUID());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::isCanIneractWithBattleMaster(Player* pPlayer, bool msg) const
{
    if(!isBattleMaster())
        return false;

    uint32 bgTypeId = objmgr.GetBattleMasterBG(GetEntry());
    if(!msg)
        return pPlayer->GetBGAccessByLevel(bgTypeId);

    if(!pPlayer->GetBGAccessByLevel(bgTypeId))
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch(bgTypeId)
        {
            case BATTLEGROUND_AV:  pPlayer->PlayerTalkClass->SendGossipMenu(7616,GetGUID()); break;
            case BATTLEGROUND_WS:  pPlayer->PlayerTalkClass->SendGossipMenu(7599,GetGUID()); break;
            case BATTLEGROUND_AB:  pPlayer->PlayerTalkClass->SendGossipMenu(7642,GetGUID()); break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:  pPlayer->PlayerTalkClass->SendGossipMenu(10024,GetGUID()); break;
            break;
        }
        return false;
    }
    return true;
}

bool Creature::isCanTrainingAndResetTalentsOf(Player* pPlayer) const
{
    return pPlayer->getLevel() >= 10
        && GetCreatureInfo()->trainer_type == TRAINER_TYPE_CLASS
        && pPlayer->getClass() == GetCreatureInfo()->classNum;
}

void Creature::prepareGossipMenu( Player *pPlayer,uint32 gossipid )
{
    PlayerMenu* pm=pPlayer->PlayerTalkClass;
    pm->ClearMenus();

    // lazy loading single time at use
    LoadGossipOptions();

    GossipOption* gso;
    GossipOption* ingso;

    for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        gso=&*i;
        if(gso->GossipId == gossipid)
        {
            bool cantalking=true;
            if(gso->Id==1)
            {
                uint32 textid=GetNpcTextId();
                GossipText * gossiptext=objmgr.GetGossipText(textid);
                if(!gossiptext)
                    cantalking=false;
            }
            else
            {
                switch (gso->Action)
                {
                    case GOSSIP_OPTION_QUESTGIVER:
                        pPlayer->PrepareQuestMenu(GetGUID());
                        //if (pm->GetQuestMenu()->MenuItemCount() == 0)
                        cantalking=false;
                        //pm->GetQuestMenu()->ClearMenu();
                        break;
                    case GOSSIP_OPTION_ARMORER:
                        cantalking=false;                   // added in special mode
                        break;
                    case GOSSIP_OPTION_SPIRITHEALER:
                        if( !pPlayer->isDead() )
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_VENDOR:
                        // load vendor items if not yet
                        LoadGoods();

                        if(!GetItemCount())
                        {
                            sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_VENDOR but have empty trading item list.",
                                GetGUIDLow(),GetCreatureInfo()->Entry);
                            cantalking=false;
                        }
                        break;
                    case GOSSIP_OPTION_TRAINER:
                        // Lazy loading at first access
                        LoadTrainerSpells();

                        if(!isCanTrainingOf(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNTALENTS:
                        if(!isCanTrainingAndResetTalentsOf(pPlayer))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNPETSKILLS:
                        if(!pPlayer->GetPet() || pPlayer->GetPet()->getPetType() != HUNTER_PET || pPlayer->GetPet()->m_spells.size() <= 1 || GetCreatureInfo()->trainer_type != TRAINER_TYPE_PETS || GetCreatureInfo()->classNum != CLASS_HUNTER)
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_TAXIVENDOR:
                        if ( pPlayer->GetSession()->SendLearnNewTaxiNode(GetGUID()) )
                            return;
                        break;
                    case GOSSIP_OPTION_BATTLEFIELD:
                        if(!isCanIneractWithBattleMaster(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_SPIRITGUIDE:
                    case GOSSIP_OPTION_INNKEEPER:
                    case GOSSIP_OPTION_BANKER:
                    case GOSSIP_OPTION_PETITIONER:
                    case GOSSIP_OPTION_STABLEPET:
                    case GOSSIP_OPTION_TABARDDESIGNER:
                    case GOSSIP_OPTION_AUCTIONEER:
                        break;                              // no checks
                    default:
                        sLog.outErrorDb("Creature %u (entry: %u) have unknown gossip option %u",GetGUIDLow(),GetCreatureInfo()->Entry,gso->Action);
                        break;
                }
            }

            if(!gso->Option.empty() && cantalking )
            {                                               //note for future dev: should have database fields for BoxMessage & BoxMoney
                pm->GetGossipMenu()->AddMenuItem((uint8)gso->Icon,gso->Option, gossipid,gso->Action,"",0,false);
                ingso=gso;
            }
        }
    }

    ///some gossips aren't handled in normal way ... so we need to do it this way .. TODO: handle it in normal way ;-)
    if(pm->GetGossipMenu()->MenuItemCount()==0 && !pm->GetQuestMenu()->MenuItemCount())
    {
        if(HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER))
        {
            LoadTrainerSpells();                            // Lazy loading at first access
            isCanTrainingOf(pPlayer,true);                  // output error message if need
        }
        if(HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_BATTLEMASTER))
        {
            isCanIneractWithBattleMaster(pPlayer,true);     // output error message if need
        }
    }
}

void Creature::sendPreparedGossip(Player* player)
{
    if(!player)
        return;

    GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();

    // in case empty gossip menu open quest menu if any
    if (gossipmenu->MenuItemCount() == 0 && GetNpcTextId() == 0)
    {
        player->SendPreparedQuest(GetGUID());
        return;
    }

    // in case non empty gossip menu (that not included quests list size) show it
    // (quest entries from quest menu wiill be included in list)
    player->PlayerTalkClass->SendGossipMenu(GetNpcTextId(), GetGUID());
}

void Creature::OnGossipSelect(Player* player, uint32 option)
{
    GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();
    uint32 action=gossipmenu->GetItem(option).m_gAction;
    uint32 zoneid=GetZoneId();
    uint64 guid=GetGUID();
    GossipOption const *gossip=GetGossipOption( action );
    uint32 textid;
    if(!gossip)
    {
        zoneid=0;
        gossip=GetGossipOption( action );
        if(!gossip)
            return;
    }
    textid=GetGossipTextId( action, zoneid);
    if(textid==0)
        textid=GetNpcTextId();

    switch (gossip->Action)
    {
        case GOSSIP_OPTION_GOSSIP:
            player->PlayerTalkClass->CloseGossip();
            player->PlayerTalkClass->SendTalking( textid );
            break;
        case GOSSIP_OPTION_SPIRITHEALER:
            if( player->isDead() )
                CastSpell(this,17251,true,NULL,NULL,player->GetGUID());
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            player->PrepareQuestMenu( guid );
            player->SendPreparedQuest( guid );
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
            player->GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_STABLEPET:
            player->GetSession()->SendStablePet(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            player->GetSession()->SendTrainerList(guid);
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            player->PlayerTalkClass->CloseGossip();
            player->SendTalentWipeConfirm(guid);
            break;
        case GOSSIP_OPTION_UNLEARNPETSKILLS:
            player->PlayerTalkClass->CloseGossip();
            player->SendPetSkillWipeConfirm();
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            player->GetSession()->SendTaxiMenu(guid);
            break;
        case GOSSIP_OPTION_INNKEEPER:
            player->PlayerTalkClass->CloseGossip();
            player->SetBindPoint( guid );
            break;
        case GOSSIP_OPTION_BANKER:
            player->GetSession()->SendShowBank( guid );
            break;
        case GOSSIP_OPTION_PETITIONER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendPetitionShowList( guid );
            break;
        case GOSSIP_OPTION_TABARDDESIGNER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendTabardVendorActivate( guid );
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            player->GetSession()->SendAuctionHello( guid, this );
            break;
        case GOSSIP_OPTION_SPIRITGUIDE:
        case GOSSIP_GUARD_SPELLTRAINER:
        case GOSSIP_GUARD_SKILLTRAINER:
            prepareGossipMenu( player,gossip->Id );
            sendPreparedGossip( player );
            break;
        case GOSSIP_OPTION_BATTLEFIELD:
        {
            uint32 bgTypeId = objmgr.GetBattleMasterBG(GetEntry());
            player->GetSession()->SendBattlegGroundList( GetGUID(), bgTypeId );
            break;
        }
        default:
            OnPoiSelect( player, gossip );
            break;
    }

}

void Creature::OnPoiSelect(Player* player, GossipOption const *gossip)
{
    if(gossip->GossipId==GOSSIP_GUARD_SPELLTRAINER || gossip->GossipId==GOSSIP_GUARD_SKILLTRAINER)
    {
        float x,y;
        bool findnpc=false;
        Poi_Icon icon = ICON_POI_0;
        QueryResult *result;
        Field *fields;
        uint32 mapid=GetMapId();
        Map const* map=MapManager::Instance().GetBaseMap( mapid );
        uint16 areaflag=map->GetAreaFlag(GetPositionX(),GetPositionY());
        uint32 zoneid=map->GetZoneId(areaflag);
        std::string areaname= gossip->Option;
        uint16 pflag;

        // use the action relate to creaturetemplate.trainer_type ?
        result= WorldDatabase.PQuery("SELECT `creature`.`position_x`,`creature`.`position_y` FROM `creature`,`creature_template` WHERE `creature`.`map` = '%u' AND `creature`.`id` = `creature_template`.`entry` AND `creature_template`.`trainer_type` = '%u'", mapid, gossip->Action );
        if(!result)
            return;
        do
        {
            fields = result->Fetch();
            x=fields[0].GetFloat();
            y=fields[1].GetFloat();
            pflag=map->GetAreaFlag(GetPositionX(),GetPositionY());
            if(pflag==areaflag)
            {
                findnpc=true;
                break;
            }
        }while(result->NextRow());

        delete result;

        if(!findnpc)
        {
            player->PlayerTalkClass->SendTalking( "$NSorry", "Here no this person.");
            return;
        }

        //need add more case.
        switch(gossip->Action)
        {
            case GOSSIP_GUARD_BANK:
                icon=ICON_POI_HOUSE;
                break;
            case GOSSIP_GUARD_RIDE:
                icon=ICON_POI_RWHORSE;
                break;
            case GOSSIP_GUARD_GUILD:
                icon=ICON_POI_BLUETOWER;
                break;
            default:
                icon=ICON_POI_TOWER;
                break;
        }
        uint32 textid=GetGossipTextId( gossip->Action, zoneid );
        player->PlayerTalkClass->SendTalking( textid );
        player->PlayerTalkClass->SendPointOfInterest( x, y, icon, 2, 15, areaname.c_str() );
    }
}

uint32 Creature::GetGossipTextId(uint32 action, uint32 zoneid)
{
    QueryResult *result= WorldDatabase.PQuery("SELECT `textid` FROM `npc_gossip_textid` WHERE `action` = '%u' AND `zoneid` ='%u'", action, zoneid );

    if(!result)
        return 0;

    Field *fields = result->Fetch();
    uint32 id = fields[0].GetUInt32();

    delete result;

    return id;
}

uint32 Creature::GetGossipCount( uint32 gossipid )
{
    uint32 count=0;
    for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        if(i->GossipId == gossipid )
            count++;
    }
    return count;
}

uint32 Creature::GetNpcTextId()
{
    // already loaded and cached
    if(m_NPCTextId)
        return m_NPCTextId;

    QueryResult* result = WorldDatabase.PQuery("SELECT `textid` FROM `npc_gossip` WHERE `npc_guid`= '%u'", m_DBTableGuid);
    if(result)
    {
        Field *fields = result->Fetch();
        m_NPCTextId = fields[0].GetUInt32();
        delete result;
    }
    else
        m_NPCTextId = DEFAULT_GOSSIP_MESSAGE;

    return m_NPCTextId;
}

std::string Creature::GetGossipTitle(uint8 type,uint32 id)
{
    for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        if(i->Id==id && i->NpcFlag==(uint32)type)
            return i->Option;
    }
    return "";
}

GossipOption const* Creature::GetGossipOption( uint32 id ) const
{
    for( GossipOptionList::const_iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        if(i->Action==id )
            return &*i;
    }
    return NULL;
}

void Creature::LoadGossipOptions()
{
    if(m_gossipOptionLoaded)
        return;

    uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

    QueryResult *result = WorldDatabase.PQuery( "SELECT `id`,`gossip_id`,`npcflag`,`icon`,`action`,`option` FROM `npc_option` WHERE (npcflag & %u)<>0", npcflags );

    if(!result)
        return;

    GossipOption go;
    do
    {
        Field *fields = result->Fetch();
        go.Id= fields[0].GetUInt32();
        go.GossipId = fields[1].GetUInt32();
        go.NpcFlag=fields[2].GetUInt32();
        go.Icon=fields[3].GetUInt32();
        go.Action=fields[4].GetUInt32();
        go.Option=fields[5].GetCppString();
        addGossipOption(go);
    }while( result->NextRow() );
    delete result;

    m_gossipOptionLoaded = true;
}

void Creature::AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run, uint8 type)
{
    /*    uint32 timeElap = getMSTime();
        if ((timeElap - m_startMove) < m_moveTime)
        {
            oX = (dX - oX) * ( (timeElap - m_startMove) / m_moveTime );
            oY = (dY - oY) * ( (timeElap - m_startMove) / m_moveTime );
        }
        else
        {
            oX = dX;
            oY = dY;
        }

        dX = x;
        dY = y;
        m_orientation = atan2((oY - dY), (oX - dX));

        m_startMove = getMSTime();
        m_moveTime = time;*/
    SendMonsterMove(x,y,z,type,run,time);
}

Player *Creature::GetLootRecipient() const
{
    if (!m_lootRecipient) return NULL;
    else return ObjectAccessor::FindPlayer(m_lootRecipient);
}

void Creature::SetLootRecipient(Player *player)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears
    if (!player) m_lootRecipient = 0;
    else m_lootRecipient = player->GetGUID();
}

void Creature::SaveToDB()
{
    // update in loaded data
    CreatureData& data = objmgr.NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);

    // check if it's a custom model and if not, use 0 for displayId
    CreatureInfo const *cinfo = GetCreatureInfo();
    if(cinfo)
    {
        if(displayId != cinfo->DisplayID_A && displayId != cinfo->DisplayID_H)
        {
            CreatureModelInfo const *minfo = objmgr.GetCreatureModelInfo(cinfo->DisplayID_A);
            if(!minfo || displayId != minfo->modelid_other_gender)
            {
                minfo = objmgr.GetCreatureModelInfo(cinfo->DisplayID_H);
                if(minfo && displayId == minfo->modelid_other_gender)
                    displayId = 0;
            }
            else
                displayId = 0;
        }
        else
            displayId = 0;
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = GetMapId();
    data.displayid = displayId;
    data.equipmentId = GetEquipmentId();
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.spawntimesecs = m_respawnDelay;
    data.spawndist = m_respawnradius;
    data.currentwaypoint = 0;
    data.spawn_posX = respawn_cord[0];
    data.spawn_posY = respawn_cord[1];
    data.spawn_posZ = respawn_cord[2];
    data.spawn_orientation = GetOrientation();
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    data.deathState = m_deathState;
    data.movementType = GetDefaultMovementType();

    // updated in DB
    WorldDatabase.BeginTransaction();

    WorldDatabase.PExecuteLog("DELETE FROM `creature` WHERE `guid` = '%u'", m_DBTableGuid);

    std::ostringstream ss;
    ss << "INSERT INTO `creature` VALUES ("
        << m_DBTableGuid << ","
        << GetEntry() << ","
        << GetMapId() <<","
        << displayId <<","
        << GetEquipmentId() <<","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << m_respawnDelay << ","                            //respawn time
        << (float) 0  << ","                                //spawn distance (float)
        << (uint32) (0) << ","                              //currentwaypoint
        << respawn_cord[0] << ","                           //spawn_position_x
        << respawn_cord[1] << ","                           //spawn_position_y
        << respawn_cord[2] << ","                           //spawn_position_z
        << (float)(0) << ","                                //spawn_orientation
        << GetHealth() << ","                               //curhealth
        << GetPower(POWER_MANA) << ","                      //curmana
        << (uint32)(m_deathState) << ","                    //DeathState (0 or 65)
        << GetDefaultMovementType() << ")";                 // default movement generator type

    WorldDatabase.PExecuteLog( ss.str( ).c_str( ) );

    WorldDatabase.CommitTransaction();
}

void Creature::SelectLevel(const CreatureInfo *cinfo)
{
    uint32 rank = isPet()? 0 : cinfo->rank;

    // level
    uint32 minlevel = std::min(cinfo->maxlevel, cinfo->minlevel);
    uint32 maxlevel = std::max(cinfo->maxlevel, cinfo->minlevel);
    uint32 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel))/(maxlevel - minlevel);

    // health
    float healthmod = _GetHealthMod(rank);

    uint32 minhealth = std::min(cinfo->maxhealth, cinfo->minhealth);
    uint32 maxhealth = std::max(cinfo->maxhealth, cinfo->minhealth);
    uint32 health = uint32(healthmod * (minhealth + uint32(rellevel*(maxhealth - minhealth))));

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);

    // mana
    uint32 minmana = std::min(cinfo->maxmana, cinfo->minmana);
    uint32 maxmana = std::max(cinfo->maxmana, cinfo->minmana);
    uint32 mana = minmana + uint32(rellevel*(maxmana - minmana));

    SetCreateMana(mana);
    SetMaxPower(POWER_MANA, mana);                          //MAX Mana
    SetPower(POWER_MANA, mana);

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, health);
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, mana);

    // damage
    float damagemod = _GetDamageMod(rank);

    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg * damagemod);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg * damagemod);

    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,cinfo->minrangedmg * damagemod);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,cinfo->maxrangedmg * damagemod);

    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, cinfo->attackpower * damagemod);
}

float Creature::_GetHealthMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getRate(RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getRate(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

float Creature::_GetDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getRate(RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getRate(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

float Creature::GetSpellDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getRate(RATE_CREATURE_NORMAL_SPELLDAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getRate(RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld.getRate(RATE_CREATURE_ELITE_RARE_SPELLDAMAGE);
        default:
            return sWorld.getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
    }
}

bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data)
{
    Object::_Create(guidlow, HIGHGUID_UNIT);

    m_DBTableGuid = guidlow;

    SetUInt32Value(OBJECT_FIELD_ENTRY,Entry);
    CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(Entry);
    if(!cinfo)
    {
        sLog.outErrorDb("Error: creature entry %u does not exist.",Entry);
        return false;
    }

    if (cinfo->DisplayID_A == 0 || cinfo->DisplayID_H == 0) // Cancel load if no model defined
    {
        sLog.outErrorDb("Creature (Entry: %u) has no model defined for Horde or Alliance in table `creature_template`, can't load. ",Entry);
        return false;
    }

    m_regenHealth = cinfo->RegenHealth;

    uint32 display_id = objmgr.ChooseDisplayId(team,cinfo,data);

    CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(display_id);
    if (!minfo)
    {
        sLog.outErrorDb("Creature (Entry: %u) has model %u not found in table `creature_model_info`, can't load. ", Entry, display_id);
        return false;
    }
    else
        display_id = minfo->modelid;                        // it can be different (for another gender)

    SetUInt32Value(UNIT_FIELD_DISPLAYID, display_id);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, display_id);
    SetUInt32Value(UNIT_FIELD_BYTES_2, 1);                  // let creature use equiped weapon in fight
    SetUInt32Value(UNIT_FIELD_BYTES_0, ( minfo->gender << 16 ));

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,minfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH,minfo->combat_reach );

    // Load creature equipment
    if(!data || data->equipmentId == 0)
    {                                                       // use default from the template
        if(!LoadEquipment(cinfo->equipmentId))
            sLog.outErrorDb("Creature (Entry: %u) has equipment_id %u (default from creature template) not found in table `creature_equip_template`.", Entry, cinfo->equipmentId);
    }
    else if(data && data->equipmentId != -1)
    {                                                       // override, -1 means no equipment
        if(!LoadEquipment(data->equipmentId))
            sLog.outErrorDb("Creature (GUID: %u Entry: %u) has equipment_id %u (override from creature data) not found in table `equipment`. ", guidlow, data->id, data->equipmentId);
    }

    SetName(GetCreatureInfo()->Name);

    SelectLevel(cinfo);

    if (team == HORDE)
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, cinfo->faction_H);
    else
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, cinfo->faction_A);

    SetUInt32Value(UNIT_NPC_FLAGS,cinfo->npcflag);

    SetAttackTime(BASE_ATTACK,  cinfo->baseattacktime);
    SetAttackTime(RANGED_ATTACK,cinfo->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS,cinfo->Flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS,cinfo->dynamicflags);

    SetModifierValue(UNIT_MOD_ARMOR,             BASE_VALUE, float(cinfo->armor));
    SetModifierValue(UNIT_MOD_RESISTANCE_HOLY,   BASE_VALUE, float(cinfo->resistance1));
    SetModifierValue(UNIT_MOD_RESISTANCE_FIRE,   BASE_VALUE, float(cinfo->resistance2));
    SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(cinfo->resistance3));
    SetModifierValue(UNIT_MOD_RESISTANCE_FROST,  BASE_VALUE, float(cinfo->resistance4));
    SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(cinfo->resistance5));
    SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(cinfo->resistance6));

    SetCanModifyStats(true);
    UpdateAllStats();

    CreatureDisplayInfoEntry const* ScaleEntry = sCreatureDisplayInfoStore.LookupEntry(display_id);
    SetFloatValue(OBJECT_FIELD_SCALE_X, ScaleEntry ? ScaleEntry->scale : 1);

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cinfo->faction_A);
    if (factionTemplate)                                    // check and error show at loading templates
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction);
        if (factionEntry)
            if (!cinfo->civilian && (factionEntry->team == ALLIANCE || factionEntry->team == HORDE))
                SetPvP(true);
    }

    m_spells[0] = cinfo->spell1;
    m_spells[1] = cinfo->spell2;
    m_spells[2] = cinfo->spell3;
    m_spells[3] = cinfo->spell4;

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    SetSpeed(MOVE_WALK,     cinfo->speed );
    SetSpeed(MOVE_RUN,      cinfo->speed );
    SetSpeed(MOVE_WALKBACK, cinfo->speed );
    SetSpeed(MOVE_SWIM,     cinfo->speed);
    SetSpeed(MOVE_SWIMBACK, cinfo->speed);

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);

    //Notify the map's instance data.
    //Only works if you create the object in it, not if it is moves to that map.
    //Normally non-players do not teleport to other maps.
    Map *map = MapManager::Instance().GetMap(GetMapId(), this);
    if(map && map->GetInstanceData())
    {
        map->GetInstanceData()->OnCreatureCreate(this, Entry);
    }

    return true;
}

bool Creature::LoadFromDB(uint32 guid, uint32 InstanceId)
{
    CreatureData const* data = objmgr.GetCreatureData(guid);

    if(!data)
    {
        sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load. ",guid);
        return false;
    }

    uint32 stored_guid = guid;

    if (InstanceId != 0) guid = objmgr.GenerateLowGuid(HIGHGUID_UNIT);
    SetInstanceId(InstanceId);

    uint16 team = 0;
    if(!Create(guid,data->mapid,data->posX,data->posY,data->posZ,data->orientation,data->id,team,data))
        return false;

    m_DBTableGuid = stored_guid;
    LoadCreaturesAddon();
    if(GetCreatureInfo()->rank > 0)
        this->m_corpseDelay *= 3;                           //if creature is elite, then remove corpse later

    uint32 curhealth = data->curhealth;
    if(curhealth)
    {
        curhealth = uint32(curhealth*_GetHealthMod(GetCreatureInfo()->rank));
        if(curhealth < 1)
            curhealth = 1;
    }

    SetHealth(curhealth);
    SetPower(POWER_MANA,data->curmana);

    m_respawnradius = data->spawndist;
    respawn_cord[0] = data->spawn_posX;
    respawn_cord[1] = data->spawn_posY;
    respawn_cord[2] = data->spawn_posZ;

    m_respawnDelay = data->spawntimesecs;
    m_deathState = (DeathState)data->deathState;
    if(m_deathState == JUST_DIED)                           // Don't must be set to JUST_DEAD, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
    {
        sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_DEAD (1). State set to ALIVE.",GetGUIDLow(),GetEntry());
        m_deathState = ALIVE;
    }
    else
    if(m_deathState < ALIVE || m_deathState > DEAD)
    {
        sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) in wrong state: %d. State set to ALIVE.",GetGUIDLow(),GetEntry(),m_deathState);
        m_deathState = ALIVE;
    }

    m_respawnTime  = objmgr.GetCreatureRespawnTime(m_DBTableGuid,GetInstanceId());
    if(m_respawnTime > time(NULL))                          // not ready to respawn
        m_deathState = DEAD;
    else if(m_respawnTime)                                  // respawn time set but expired
    {
        m_respawnTime = 0;
        objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),0);
    }

    // checked at creature_template loading
    m_defaultMovementType = MovementGeneratorType(data->movementType);

    AIM_Initialize();
    return true;
}

bool Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    if(equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i=0;i<3;i++)
            {
                SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, 0);
                SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2), 0);
                SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, 0);
            }
            m_equipmentId = 0;
        }
        return true;
    }

    EquipmentInfo const *einfo = objmgr.GetEquipmentInfo(equip_entry);
    if (!einfo)
        return false;

    m_equipmentId = equip_entry;
    for (uint8 i=0;i<3;i++)
    {
        SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, einfo->equipmodel[i]);
        SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2), einfo->equipinfo[i]);
        SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, einfo->equipslot[i]);
    }
    return true;
}

void Creature::LoadGoods()
{
    // already loaded;
    if(m_itemsLoaded)
        return;

    m_vendor_items.clear();

    QueryResult *result = WorldDatabase.PQuery("SELECT `item`, `maxcount`,`incrtime` FROM `npc_vendor` WHERE `entry` = '%u'", GetEntry());

    if(!result) return;

    do
    {
        Field *fields = result->Fetch();

        if (GetItemCount() >= MAX_VENDOR_ITEMS)
        {
            sLog.outErrorDb( "Vendor %u has too many items (%u >= %i). Check the DB!", GetEntry(), GetItemCount(), MAX_VENDOR_ITEMS );
            break;
        }

        uint32 item_id = fields[0].GetUInt32();
        if(!sItemStorage.LookupEntry<ItemPrototype>(item_id))
        {
            sLog.outErrorDb("Vendor %u have in item list non-existed item %u",GetEntry(),item_id);
            continue;
        }

        AddItem( item_id, fields[1].GetUInt32(), fields[2].GetUInt32());
    }
    while( result->NextRow() );

    delete result;

    m_itemsLoaded = true;
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mCreatureQuestRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mCreatureQuestInvolvedRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

void Creature::DeleteFromDB()
{
    objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),0);
    objmgr.DeleteCreatureData(m_DBTableGuid);

    WorldDatabase.BeginTransaction();
    WorldDatabase.PExecuteLog("DELETE FROM `creature` WHERE `guid` = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog("DELETE FROM `creature_addon` WHERE `guid` = '%u'", m_DBTableGuid);
    WorldDatabase.PExecuteLog("DELETE FROM `creature_movement` WHERE `id` = '%u'", m_DBTableGuid);
    WorldDatabase.CommitTransaction();
}

float Creature::GetAttackDistance(Unit *pl) const
{
    float aggroRate = sWorld.getRate(RATE_CREATURE_AGGRO);
    if(aggroRate==0)
        return 0.0f;

    int32 playerlevel   = pl->getLevel();
    int32 creaturelevel = getLevel();

    int32 leveldif       = playerlevel - creaturelevel;

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if ( leveldif < - 25)
        leveldif = -25;

    // "The aggro radius of a mob having the same level as the player is roughly 20 yards"
    float RetDistance = 20;

    // "Aggro Radius varries with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    if(getLevel()+5 <= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        // decrease aggro range auras
        AuraList const& modDectectRangeList = GetAurasByType(SPELL_AURA_MOD_DETECT_RANGE);
        for(AuraList::const_iterator itr = modDectectRangeList.begin(); itr != modDectectRangeList.end(); ++itr)
            RetDistance += (*itr)->GetModifier()->m_amount;
    }

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if(RetDistance < 5)
        RetDistance = 5;

    return (RetDistance*aggroRate);
}

CreatureInfo const *Creature::GetCreatureInfo() const
{
    return objmgr.GetCreatureTemplate(GetEntry());
}

void Creature::setDeathState(DeathState s)
{
    if(s == JUST_DIED)
    {
        m_deathTimer = m_corpseDelay*1000;

        // always save boss respawn time at death to prevent crash cheating
        if(sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATLY) || isWorldBoss())
            SaveRespawnTime();

        if(!IsStopped()) StopMoving();
    }
    Unit::setDeathState(s);

    if(s == JUST_DIED)
    {
        SetUInt64Value (UNIT_FIELD_TARGET,0);               // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, 0);
        if(!isPet() && GetCreatureInfo()->SkinLootId)
        {
            LootStore skinStore = LootTemplates_Skinning;
            LootStore::iterator tab = skinStore.find(GetCreatureInfo()->SkinLootId);
            if ( tab != skinStore.end() )
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        }
        Unit::setDeathState(CORPSE);
    }
}

void Creature::Respawn()
{
    if(getDeathState()==CORPSE)
    {
        m_deathTimer = 0;
        Update(0);                                          // despawn corpse
    }
    if(getDeathState()==DEAD)
    {
        objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),0);
        m_respawnTime = time(NULL);                         // respawn at next tick
    }
}

bool Creature::IsBossImmunedToMechanic(uint32 mechanic) const
{
    if( isWorldBoss() && (
        (MECHANIC_CHARM         == mechanic) ||
        (MECHANIC_CONFUSED      == mechanic) ||
        (MECHANIC_DISTRACT      == mechanic) ||
        (MECHANIC_FEAR          == mechanic) ||
        (MECHANIC_ROOT          == mechanic) ||
        (MECHANIC_SILENCE       == mechanic) ||
        (MECHANIC_SLEEP         == mechanic) ||
        (MECHANIC_SNARE         == mechanic) ||
        (MECHANIC_STUN          == mechanic) ||
        (MECHANIC_FREEZE        == mechanic) ||
        (MECHANIC_KNOCKOUT      == mechanic) ||
        (MECHANIC_POLYMORPH     == mechanic) ||
        (MECHANIC_BANISH        == mechanic) ||
        (MECHANIC_SHACKLE       == mechanic) ||
        (MECHANIC_TURN          == mechanic) ||
        (MECHANIC_HORROR        == mechanic) ||
        (MECHANIC_DAZE          == mechanic) ))
        return true;

    return false;
}

bool Creature::IsImmunedToSpell(SpellEntry const* spellInfo) const
{
    if (!spellInfo)
        return false;

    if( IsBossImmunedToMechanic(spellInfo->Mechanic) )
        return true;

    return Unit::IsImmunedToSpell(spellInfo);
}

bool Creature::IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const
{
    if( IsBossImmunedToMechanic(mechanic) )
        return true;

    return Unit::IsImmunedToSpellEffect(effect, mechanic);
}

SpellEntry const *Creature::reachWithSpellAttack(Unit *pVictim)
{
    if(!pVictim)
        return NULL;

    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if(!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spells[i] );
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for(uint32 j=0;j<3;j++)
        {
            if( (spellInfo->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE )       ||
                (spellInfo->Effect[j] == SPELL_EFFECT_INSTAKILL)            ||
                (spellInfo->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                (spellInfo->Effect[j] == SPELL_EFFECT_HEALTH_LEECH )
                )
            {
                bcontinue = false;
                break;
            }
        }
        if(bcontinue) continue;

        if(spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetMaxRange(srange);
        float minrange = GetMinRange(srange);
        float dist = GetDistance(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range || dist < minrange )
            continue;
        if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

SpellEntry const *Creature::reachWithSpellCure(Unit *pVictim)
{
    if(!pVictim)
        return NULL;

    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if(!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(m_spells[i] );
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for(uint32 j=0;j<3;j++)
        {
            if( (spellInfo->Effect[j] == SPELL_EFFECT_HEAL ) )
            {
                bcontinue = false;
                break;
            }
        }
        if(bcontinue) continue;

        if(spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetMaxRange(srange);
        float minrange = GetMinRange(srange);
        float dist = GetDistance(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range || dist < minrange )
            continue;
        if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Creature::IsVisibleInGridForPlayer(Player* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if(pl->isGameMaster())
        return true;

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if(pl->isAlive() || pl->GetDeathTimer() > 0)
    {
        if( GetEntry() == VISUAL_WAYPOINT && !pl->isGameMaster() )
            return false;
        return isAlive() || m_deathTimer > 0;
    }

    // Dead player see live creatures near own corpse
    if(isAlive())
    {
        Corpse *corpse = pl->GetCorpse();
        if(corpse)
        {
            // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
            if(corpse->IsWithinDistInMap(this,(20+25)*sWorld.getRate(RATE_CREATURE_AGGRO)))
                return true;
        }
    }

    // Dead player see Spirit Healer or Spirit Guide
    if(isSpiritService())
        return true;

    // and not see any other
    return false;
}

void Creature::CallAssistence()
{
    if( !m_AlreadyCallAssistence && getVictim() )
    {
        CastSpell(this,SPELL_ID_AGGRO, true, NULL, NULL, getVictim()->GetGUID());
    }
}

void Creature::SaveRespawnTime()
{
    if(isPet())
        return;

    if(m_respawnTime > time(NULL))                          // dead (no corpse)
        objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),m_respawnTime);
    else if(m_deathTimer > 0)                               // dead (corpse)
        objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),time(NULL)+m_respawnDelay+m_deathTimer/1000);
}

bool Creature::IsOutOfThreatArea(Unit* pVictim) const
{
    if(!pVictim)
        return true;

    if(!pVictim->IsInMap(this))
        return true;

    if(!pVictim->isTargetableForAttack())
        return true;

    if(!pVictim->isInAccessablePlaceFor(this))
        return true;

    // we not need get instance map, base map provide all info
    if(MapManager::Instance().GetBaseMap(GetMapId())->Instanceable())
        return false;

    float rx,ry,rz;
    GetRespawnCoord(rx, ry, rz);

    float length = pVictim->GetDistanceSq(rx,ry,rz);
    return ( length > 10000.0f);                            // real value unknown
}

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
    if(CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(m_DBTableGuid))
        return addon;

    return ObjectMgr::GetCreatureTemplateAddon(GetEntry());
}

//creature_addon table
bool Creature::LoadCreaturesAddon(bool reload)
{
    CreatureDataAddon const *cainfo = GetCreatureAddon();
    if(!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->bytes0 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);

    if (cainfo->bytes1 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_1, cainfo->bytes1);

    if (cainfo->bytes2 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_2, cainfo->bytes2);

    if (cainfo->emote != 0)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, cainfo->emote);

    if(cainfo->auras)
    {
        for (CreatureDataAddonAura const* cAura = cainfo->auras; cAura->spell_id; ++cAura)
        {
            SpellEntry const *AdditionalSpellInfo = sSpellStore.LookupEntry(cAura->spell_id);
            if (!AdditionalSpellInfo)
            {
                sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) has wrong spell %u defined in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id);
                continue;
            }

            // skip already applied aura
            if(GetAura(cAura->spell_id,cAura->effect_idx))
            {
                if(!reload)
                    sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) has duplicate aura (spell %u effect %u) in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id,cAura->effect_idx);

                continue;
            }

            Aura* AdditionalAura = new Aura(AdditionalSpellInfo, cAura->effect_idx, NULL, this, this, 0);
            AddAura(AdditionalAura);
            sLog.outDebug("Spell: %u with Aura %u added to creature (GUIDLow: %u Entry: %u )", cAura->spell_id, AdditionalSpellInfo->EffectApplyAuraName[0],GetGUIDLow(),GetEntry());
        }
    }
    return true;
}

/// Send a message to LocalDefense channel for players oposition team in the zone
void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPacket data(SMSG_ZONE_UNDER_ATTACK,4);
    data << (uint32)GetZoneId();
    sWorld.SendGlobalMessage(&data,NULL,(enemy_team==ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::_AddCreatureSpellCooldown(uint32 spell_id, time_t end_time)
{
    m_CreatureSpellCooldowns[spell_id] = end_time;
}

void Creature::_AddCreatureCategoryCooldown(uint32 category, time_t apply_time)
{
    m_CreatureCategoryCooldowns[category] = apply_time;
}

void Creature::AddCreatureSpellCooldown(uint32 spellid)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellid);
    if(!spellInfo)
        return;

    uint32 cooldown = GetRecoveryTime(spellInfo);
    if(cooldown)
        _AddCreatureSpellCooldown(spellid, time(NULL) + cooldown/1000);

    if(spellInfo->Category)
        _AddCreatureCategoryCooldown(spellInfo->Category, time(NULL));

    m_GlobalCooldown = 1500;
}

bool Creature::HasCategoryCooldown(uint32 spell_id) const
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if(!spellInfo)
        return false;

    CreatureSpellCooldowns::const_iterator itr = m_CreatureCategoryCooldowns.find(spellInfo->Category);
    return(itr != m_CreatureCategoryCooldowns.end() && time_t(itr->second + (spellInfo->CategoryRecoveryTime / 1000)) > time(NULL));
}

bool Creature::HasSpellCooldown(uint32 spell_id) const
{
    CreatureSpellCooldowns::const_iterator itr = m_CreatureSpellCooldowns.find(spell_id);
    return (itr != m_CreatureSpellCooldowns.end() && itr->second > time(NULL)) || m_GlobalCooldown > 0 || HasCategoryCooldown(spell_id);
}

bool Creature::IsInEvadeMode() const
{
    return !i_motionMaster.empty() && i_motionMaster.top()->GetMovementGeneratorType() == HOME_MOTION_TYPE;
}

bool Creature::HasSpell(uint32 spellID) const
{
    uint8 i;
    for(i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if(spellID == m_spells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         //broke before end of iteration of known spells
}
