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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Log.h"
#include "LootMgr.h"
#include "Chat.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"
#include "Formulas.h"
#include "SpellAuras.h"

// apply implementation of the singletons
#include "Policies/SingletonImp.h"

uint32 CreatureInfo::randomDisplayID() const
{
    if(DisplayID_f==0)
        return DisplayID_m;
    else if(DisplayID_m==0)
        return DisplayID_f;
    else
        return urand(0,1) ? DisplayID_m : DisplayID_f;
}

Creature::Creature() :
Unit(), i_AI(NULL), lootForPickPocketed(false), lootForBody(false), m_lootMoney(0),
m_deathTimer(0), m_respawnTime(0), m_respawnDelay(25), m_corpseDelay(60), m_respawnradius(0.0),
itemcount(0), mTaxiNode(0), m_moveBackward(false), m_moveRandom(false),
m_moveRun(false), m_emoteState(0), m_isPet(false), m_isTotem(false),
m_regenTimer(2000), m_defaultMovementType(IDLE_MOTION_TYPE)
{
    m_valuesCount = UNIT_END;

    memset(item_list, 0, sizeof(CreatureItem)*MAX_CREATURE_ITEMS);
    for(int i =0; i<3; ++i) respawn_cord[i] = 0.0;

    m_spells[0] = 0;
    m_spells[1] = 0;
    m_spells[2] = 0;
    m_spells[3] = 0;

    m_AlreadyCallAssistence = false;
}

Creature::~Creature()
{
    if(m_uint32Values)                                      // only for fully created object
    {
        CombatStop();
        RemoveAllAuras();
    }

    for( SpellsList::iterator i = m_tspells.begin( ); i != m_tspells.end( ); i++ )
        delete (*i);
    m_tspells.clear();

    delete i_AI;
    i_AI = NULL;
}

void Creature::CreateTrainerSpells()
{
    for(SpellsList::iterator i = m_tspells.begin(); i != m_tspells.end(); ++i)
        delete *i;
    m_tspells.clear();

    TrainerSpell *tspell;
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT `spell`,`spellcost`,`reqskill`,`reqskillvalue`,`reqlevel` FROM `npc_trainer` WHERE `entry` = '%u'", GetCreatureInfo()->Entry);

    if(!result) return;

    do
    {
        fields = result->Fetch();

        uint32 spellid = fields[0].GetUInt32();
        SpellEntry const *spellinfo = sSpellStore.LookupEntry(spellid);

        if(!spellinfo)
        {
            sLog.outErrorDb("Trainer (GUID: %u ID: %u ) have in list non existed spell %u",GetGUIDLow(),GetEntry(),spellid);
            continue;
        }

        tspell = new TrainerSpell;
        tspell->spell = spellinfo;
        tspell->spellcost = fields[1].GetUInt32();
        tspell->reqskill = fields[2].GetUInt32();
        tspell->reqskillvalue = fields[3].GetUInt32();
        tspell->reqlevel = fields[4].GetUInt32();

        m_tspells.push_back(tspell);

    } while( result->NextRow() );

    delete result;
}

bool Creature::isCanSwimOrFly() const
{
    uint32 family = GetCreatureInfo()->family;

    // Creature that can swim in water or fly above water or walk by bottom (maybe wrong family filter - please fix)
    return (family != 3 && family != 10 && family != 11 && family != 12 && family != 20 && family != 21 && family != 27 );
}

bool Creature::isCanWalkOrFly() const
{
    //uint32 family = GetCreatureInfo()->family;

    // Creature that can walk by ground or fly above ground (maybe wrong family filter - please fix)
    // I not found special family for fish :(
    return true;
}

void Creature::Update(uint32 diff)
{
    switch( m_deathState )
    {
        case JUST_DIED:
            assert(false);                                  // Dont must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            break;
        case DEAD:
        {
            if( m_respawnTime <= time(NULL) )
            {
                DEBUG_LOG("Respawning...");
                m_respawnTime = 0;

                CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(this->GetEntry());

                SelectLevel(cinfo);
                SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

                SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
                SetHealth(GetMaxHealth());
                setDeathState( ALIVE );
                clearUnitState(UNIT_STAT_ALL_STATE);
                i_motionMaster.Clear();
                MapManager::Instance().GetMap(GetMapId())->Add(this);
            }
            break;
        }
        case CORPSE:
        {
            if( m_deathTimer <= diff )
            {
                m_deathTimer = 0;
                DEBUG_LOG("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
                ObjectAccessor::Instance().RemoveCreatureCorpseFromPlayerView(this);
                lootForPickPocketed = false;
                lootForBody         = false;
                loot.clear();
                setDeathState(DEAD);
                m_respawnTime = time(NULL) + m_respawnDelay;

                float x,y,z;
                GetRespawnCoord(x, y, z);
                MapManager::Instance().GetMap(GetMapId())->CreatureRelocation(this,x,y,z,GetOrientation());
            }
            else
                m_deathTimer -= diff;

            break;
        }
        case ALIVE:
        {
            Unit::Update( diff );
            i_motionMaster.UpdateMotion(diff);
            i_AI->UpdateAI(diff);
            if(m_regenTimer > 0)
            {
                if(diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }
            if (m_regenTimer != 0)
                break;
            if (!isInCombat())
            {
                RegenerateHealth();
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

    if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;

    uint32 addvalue = 0;

    if (isInCombat() || isPet())
        addvalue = uint32((Spirit/5 + 17) * ManaIncreaseRate);
    else
        addvalue = maxValue/3;

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld.getRate(RATE_HEALTH);

    float Spirit = GetStat(STAT_SPIRIT);

    if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;

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

void Creature::AIM_Initialize()
{
    i_motionMaster.Initialize(this);
    i_AI = FactorySelector::selectAI(this);
}

bool Creature::Create (uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry)
{
    respawn_cord[0] = x;
    respawn_cord[1] = y;
    respawn_cord[2] = z;
    SetMapId(mapid);
    Relocate(x,y,z);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %d Y: ^%d)",guidlow,Entry,x,y);
        return false;
    }

    SetOrientation(ang);
    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    return  CreateFromProto(guidlow, Entry);

}

uint32 Creature::getDialogStatus(Player *pPlayer, uint32 defstatus)
{
    uint32 result = DIALOG_STATUS_NONE;
    QuestStatus status;
    uint32 quest_id;
    Quest *pQuest;

    for( std::list<uint32>::iterator i = mInvolvedQuests.begin( ); i != mInvolvedQuests.end( ); i++ )
    {
        quest_id = *i;
        pQuest = objmgr.QuestTemplates[quest_id];
        status = pPlayer->GetQuestStatus( quest_id );
        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(quest_id)) ||
            ((strlen(pQuest->GetObjectives()) == 0) && pPlayer->CanTakeQuest(pQuest, false)))
        {
            SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);

            if ( pQuest->IsRepeatable() )
                return DIALOG_STATUS_REWARD_REP;
            else
                return DIALOG_STATUS_REWARD;
        }
        else if ( status == QUEST_STATUS_INCOMPLETE )
            result = DIALOG_STATUS_INCOMPLETE;
    }

    RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);

    if ( result == DIALOG_STATUS_INCOMPLETE )
        return result;

    for( std::list<uint32>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        pQuest = objmgr.QuestTemplates[*i];
        if ( !pQuest )
            continue;

        quest_id = pQuest->GetQuestId();
        status = pPlayer->GetQuestStatus( quest_id );
        if ( status == QUEST_STATUS_NONE )
        {
            if ( pPlayer->CanSeeStartQuest( quest_id ) )
            {
                if ( pPlayer->SatisfyQuestLevel(quest_id, false) )
                {
                    if ( pQuest->IsRepeatable() )
                        return DIALOG_STATUS_REWARD_REP;
                    else
                        return DIALOG_STATUS_AVAILABLE;
                }
                result = DIALOG_STATUS_UNAVAILABLE;
            }
        }
    }

    // can train and help unlearn talentes (2 action -> chat menu)
    if( isCanTrainingAndResetTalentsOf(pPlayer) )
        return DIALOG_STATUS_CHAT;

    if ( result == DIALOG_STATUS_UNAVAILABLE )
        return result;

    if ( defstatus == DIALOG_STATUS_NONE )
        return DIALOG_STATUS_NONE;
    else
        return DIALOG_STATUS_CHAT;
}

bool Creature::isCanTrainingOf(Player* pPlayer, bool msg) const
{
    if(!isTrainer())
        return false;

    if(m_tspells.empty())
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
            sLog.outErrorDb("Creature %u (entry: %u) have trainer type %u",GetGUIDLow(),GetCreatureInfo()->Entry,GetCreatureInfo()->trainer_type);
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

    if(!m_goptions.size())
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
                        if(!GetItemCount())
                        {
                            sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_VENDOR but have empty trading item list.",
                                GetGUIDLow(),GetCreatureInfo()->Entry);
                            cantalking=false;
                        }
                        break;
                    case GOSSIP_OPTION_TRAINER:
                        if(!isCanTrainingOf(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNTALENTS:
                        if(!isCanTrainingAndResetTalentsOf(pPlayer))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_TAXIVENDOR:
                        if ( pPlayer->GetSession()->SendLearnNewTaxiNode(GetGUID()) )
                            return;
                    case GOSSIP_OPTION_GUARD:
                    case GOSSIP_OPTION_INNKEEPER:
                    case GOSSIP_OPTION_BANKER:
                    case GOSSIP_OPTION_PETITIONER:
                    case GOSSIP_OPTION_STABLEPET:
                    case GOSSIP_OPTION_TABARDVENDOR:
                    case GOSSIP_OPTION_BATTLEFIELD:
                    case GOSSIP_OPTION_AUCTIONEER:
                        break;                              // no checks
                    default:
                        sLog.outErrorDb("Creature %u (entry: %u) have unknown gossip option %u",GetGUIDLow(),GetCreatureInfo()->Entry,gso->Action);
                        break;
                }
            }

            if(gso->Option!="" && cantalking )
            {
                pm->GetGossipMenu()->AddMenuItem((uint8)gso->Icon,gso->Option.c_str(), gossipid,gso->Action,false);
                ingso=gso;
            }
        }
    }

    if(pm->GetGossipMenu()->MenuItemCount()==0 && HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER))
        isCanTrainingOf(pPlayer,true);

    /*
    if(pm->GetGossipMenu()->MenuItemCount()==1 && ingso->Id==8 && GetGossipCount( ingso->GossipId )>0)
    {
        pm->ClearMenus();

        for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
        {
            gso=&*i;
            if(gso->GossipId==ingso->Id)
            {
                if(gso->Option!="")
                    pm->GetGossipMenu()->AddMenuItem((uint8)gso->Icon,gso->Option.c_str(),ingso->GossipId,gso->Action,false);
            }
        }
    }
    */
}

void Creature::sendPreparedGossip(Player* player)
{
    if(!player)
        return;

    GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();

    // in case empty gossip menu open quest menu if any
    if (gossipmenu->MenuItemCount() == 0)
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
            player->PlayerTalkClass->SendTalking( textid );
            break;
        case GOSSIP_OPTION_SPIRITHEALER:
            if( player->isDead() )
                player->GetSession()->SendSpiritResurrect();
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
        case GOSSIP_OPTION_TABARDVENDOR:
            player->GetSession()->SendTabardVendorActivate( guid );
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            player->GetSession()->SendAuctionHello( guid, this );
            break;
        case GOSSIP_OPTION_GUARD:
        case GOSSIP_GUARD_SPELLTRAINER:
        case GOSSIP_GUARD_SKILLTRAINER:
            prepareGossipMenu( player,gossip->Id );
            sendPreparedGossip( player );
            break;
        default:
            OnPoiSelect( player, gossip );
            break;
    }

}

void Creature::OnPoiSelect(Player* player, GossipOption const *gossip)
{
    if(gossip->GossipId==GOSSIP_OPTION_GUARD || gossip->GossipId==GOSSIP_GUARD_SPELLTRAINER || gossip->GossipId==GOSSIP_GUARD_SKILLTRAINER)
    {
        float x,y;
        bool findnpc=false;
        Poi_Icon icon = ICON_POI_0;
        QueryResult *result;
        Field *fields;
        uint32 mapid=GetMapId();
        Map* map=MapManager::Instance().GetMap( mapid );
        uint16 areaflag=map->GetAreaFlag(GetPositionX(),GetPositionY());
        uint32 zoneid=map->GetZoneId(areaflag);
        std::string areaname= gossip->Option;
        uint16 pflag;

        // use the action relate to creaturetemplate.trainer_type ?
        result= sDatabase.PQuery("SELECT `creature`.`position_x`,`creature`.`position_y` FROM `creature`,`creature_template` WHERE `creature`.`map` = '%u' AND `creature`.`id` = `creature_template`.`entry` AND `creature_template`.`trainer_type` = '%u'", mapid, gossip->Action );
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
    QueryResult *result= sDatabase.PQuery("SELECT `textid` FROM `npc_gossip_textid` WHERE `action` = '%u' AND `zoneid` ='%u'", action, zoneid );

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
    QueryResult *result = sDatabase.PQuery("SELECT `textid` FROM `npc_gossip` WHERE `npc_guid`= '%u'",GetGUIDLow());
    if(result)
    {
        Field *fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();
        delete result;
        return id;
    }
    return DEFAULT_GOSSIP_MESSAGE;
}

std::string Creature::GetGossipTitle(uint8 type,uint32 id)
{
    for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        if(i->Id==id && i->NpcFlag==(uint32)type)
            return i->Option;
    }
    return NULL;
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
    uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

    QueryResult *result = sDatabase.PQuery( "SELECT `id`,`gossip_id`,`npcflag`,`icon`,`action`,`option` FROM `npc_option` WHERE (npcflag & %u)!=0", npcflags );

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
}

void Creature::generateMoneyLoot()
{
    if (GetCreatureInfo()->maxgold > 0)
    {
        uint32 diff = GetCreatureInfo()->maxgold - GetCreatureInfo()->mingold + 1;

        // rand() result (RAND_MAX) small for large gold loots
        if(diff > 10000)
        {
            uint32 gold_part              = rand() % (diff / 10000);
            uint32 silver_and_copper_part = rand() % (diff % 10000);
            loot.gold = uint32((gold_part * 10000 + silver_and_copper_part + GetCreatureInfo()->mingold) * sWorld.getRate(RATE_DROP_MONEY));
        }
        else
        {
            loot.gold = uint32((rand() % diff + GetCreatureInfo()->mingold) * sWorld.getRate(RATE_DROP_MONEY));
        }
    }
}

void Creature::AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run, bool WalkBack)
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
    SendMonsterMove(x,y,z,WalkBack,run,time);
}

Player *Creature::GetLootRecipient() const
{
    if (!m_lootRecipient) return NULL;
    else return ObjectAccessor::Instance().FindPlayer(m_lootRecipient);
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
    sDatabase.BeginTransaction();

    sDatabase.PExecute("DELETE FROM `creature` WHERE `guid` = '%u'", GetGUIDLow());

    std::ostringstream ss;
    ss << "INSERT INTO `creature` VALUES ("
        << GetGUIDLow () << ","
        << GetEntry() << ","
        << GetMapId() <<","
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

        << (uint32)(m_deathState) << ","                    // is it really death state or just state?
    //or
    //<< (uint32)(m_state) << ","                     // is it really death state or just state?

        << GetDefaultMovementType() << ","                  // default movement generator type
        << "'')";                                           // should save auras

    sDatabase.Execute( ss.str( ).c_str( ) );

    sDatabase.CommitTransaction();
}

void Creature::SelectLevel(const CreatureInfo *cinfo)
{
    uint32 minlevel = min(cinfo->maxlevel, cinfo->minlevel);
    uint32 maxlevel = max(cinfo->maxlevel, cinfo->minlevel);
    uint32 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel))/(maxlevel - minlevel);

    uint32 minhealth = min(cinfo->maxhealth, cinfo->minhealth);
    uint32 maxhealth = max(cinfo->maxhealth, cinfo->minhealth);
    uint32 health = uint32(_GetHealthMod(isPet() ? 0 : cinfo->rank) * (minhealth + uint32(rellevel*(maxhealth - minhealth))));

    SetMaxHealth(health);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH,health);
    SetHealth(health);

    uint32 minmana = min(cinfo->maxmana, cinfo->minmana);
    uint32 maxmana = max(cinfo->maxmana, cinfo->minmana);
    uint32 mana = minmana + uint32(rellevel*(maxmana - minmana));

    SetMaxPower(POWER_MANA, mana);                          //MAX Mana
    SetUInt32Value(UNIT_FIELD_BASE_MANA, mana);
    SetPower(POWER_MANA, mana);
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

bool Creature::CreateFromProto(uint32 guidlow,uint32 Entry)
{
    Object::_Create(guidlow, HIGHGUID_UNIT);
    SetUInt32Value(OBJECT_FIELD_ENTRY,Entry);
    CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(Entry);
    if(!cinfo)
    {
        sLog.outError("Error: creature entry %u does not exist.",Entry);
        return false;
    }
    uint32 rank = isPet()? 0 : cinfo->rank;
    float damagemod = _GetDamageMod(rank);;

    uint32 display_id = cinfo->randomDisplayID();

    SetUInt32Value(UNIT_FIELD_DISPLAYID,display_id );
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID,display_id );
    SetUInt32Value(UNIT_FIELD_BYTES_2,1);                   // let creature used equiped weapon in fight

    SetName(GetCreatureInfo()->Name);

    SelectLevel(cinfo);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);

    SetUInt32Value(UNIT_NPC_FLAGS,cinfo->npcflag);

    SetFloatValue(UNIT_FIELD_MINDAMAGE,cinfo->mindmg * damagemod);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE,cinfo->maxdmg * damagemod);

    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,cinfo->minrangedmg * damagemod);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,cinfo->maxrangedmg * damagemod);

    SetAttackTime(BASE_ATTACK,  cinfo->baseattacktime);
    SetAttackTime(RANGED_ATTACK,cinfo->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS,cinfo->Flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS,cinfo->dynamicflags);

    SetArmor(cinfo->armor);
    SetResistance(SPELL_SCHOOL_HOLY,cinfo->resistance1);
    SetResistance(SPELL_SCHOOL_FIRE,cinfo->resistance2);
    SetResistance(SPELL_SCHOOL_NATURE,cinfo->resistance3);
    SetResistance(SPELL_SCHOOL_FROST,cinfo->resistance4);
    SetResistance(SPELL_SCHOOL_SHADOW,cinfo->resistance5);
    SetResistance(SPELL_SCHOOL_ARCANE,cinfo->resistance6);

    //this is probably wrong
    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, cinfo->equipmodel[0]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO , cinfo->equipinfo[0]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO  + 1, cinfo->equipslot[0]);

    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, cinfo->equipmodel[1]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2, cinfo->equipinfo[1]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 2 + 1, cinfo->equipslot[1]);

    SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+2, cinfo->equipmodel[2]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 4, cinfo->equipinfo[2]);
    SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + 4 + 1, cinfo->equipslot[2]);

    SetFloatValue(OBJECT_FIELD_SCALE_X, cinfo->size);

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,cinfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH,cinfo->combat_reach );

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cinfo->faction);
    if (factionTemplate)
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction);
        if (factionEntry)
            if (cinfo->civilian != 1 && (factionEntry->team == ALLIANCE || factionEntry->team == HORDE))
                SetPvP(true);
    } else
    sLog.outErrorDb("Error: invalid faction (%u) for creature (GUIDLow: %u Entry: %u)", cinfo->faction, GetGUIDLow(),Entry);

    if (cinfo->mount != 0)
        Mount(cinfo->mount);

    m_spells[0] = cinfo->spell1;
    m_spells[1] = cinfo->spell2;
    m_spells[2] = cinfo->spell3;
    m_spells[3] = cinfo->spell4;

    SetSpeed(MOVE_WALK,     cinfo->speed );
    SetSpeed(MOVE_RUN,      cinfo->speed );
    SetSpeed(MOVE_WALKBACK, cinfo->speed );
    SetSpeed(MOVE_SWIM,     cinfo->speed);
    SetSpeed(MOVE_SWIMBACK, cinfo->speed);

    if(cinfo->MovementType < MAX_DB_MOTION_TYPE)
        m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    else
    {
        m_defaultMovementType = IDLE_MOTION_TYPE;
        sLog.outErrorDb("Creature template %u have wrong movement generator type value %u, ignore and set to IDLE.",Entry,cinfo->MovementType);
    }

    return true;
}

bool Creature::LoadFromDB(uint32 guid, QueryResult *result)
{
    bool external = (result != NULL);
    if (!external)
        //                                0    1     2            3            4            5             6               7           8                  9                  10                 11          12        13            14      15             16
        result = sDatabase.PQuery("SELECT `id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawndist`,`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`curhealth`,`curmana`,`respawntime`,`state`,`MovementType`,`auras` "
            "FROM `creature` LEFT JOIN `creature_respawn` ON `creature`.`guid`=`creature_respawn`.`guid` WHERE `guid` = '%u'", guid);

    if(!result)
    {
        sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();

    if(!Create(guid,fields[1].GetUInt32(),fields[2].GetFloat(),fields[3].GetFloat(),
        fields[4].GetFloat(),fields[5].GetFloat(),fields[0].GetUInt32()))
    {
        if (!external) delete result;
        return false;
    }

    if(GetCreatureInfo()->rank > 0)
        this->m_corpseDelay *= 3;                           //if creature is elite, then remove corpse later

    SetHealth(fields[11].GetUInt32());
    SetPower(POWER_MANA,fields[12].GetUInt32());

    m_respawnradius = fields[7].GetFloat();
    respawn_cord[0] = fields[8].GetFloat();
    respawn_cord[1] = fields[9].GetFloat();
    respawn_cord[2] = fields[10].GetFloat();

    m_respawnDelay = fields[6].GetUInt32();
    m_deathState = (DeathState)fields[14].GetUInt32();

    m_respawnTime  = (time_t)fields[13].GetUInt64();
    if(m_respawnTime > time(NULL))                          // not ready to respawn
        m_deathState = DEAD;
    else                                                    // ready to respawn
        m_respawnTime = 0;

    {
        uint32 mtg = fields[15].GetUInt32();
        if(mtg < MAX_DB_MOTION_TYPE)
            m_defaultMovementType = MovementGeneratorType(mtg);
        else
        {
            m_defaultMovementType = IDLE_MOTION_TYPE;
            sLog.outErrorDb("Creature (GUID: %u ID: %u) have wrong movement generator type value %u, ignore and set to IDLE.",guid,GetEntry(),mtg);
        }
    }

    if(!external) delete result;

    if (HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER ) )
        CreateTrainerSpells();

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ) )
        _LoadGoods();

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ) )
        _LoadQuests();

    AIM_Initialize();
    return true;
}

void Creature::_LoadGoods()
{
    itemcount = 0;

    QueryResult *result = sDatabase.PQuery("SELECT `item`, `maxcount`,`incrtime` FROM `npc_vendor` WHERE `entry` = '%u'", GetEntry());

    if(!result) return;

    do
    {
        Field *fields = result->Fetch();

        if (GetItemCount() >= MAX_CREATURE_ITEMS)
        {
            sLog.outErrorDb( "Vendor %u has too many items (%u >= %i). Check the DB!", GetUInt32Value(OBJECT_FIELD_ENTRY), GetItemCount(), MAX_CREATURE_ITEMS );
            break;
        }

        AddItem( fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32());
    }
    while( result->NextRow() );

    delete result;

}

void Creature::_LoadQuests()
{
    mQuests.clear();
    mInvolvedQuests.clear();

    Field *fields;

    QueryResult *result = sDatabase.PQuery("SELECT `quest` FROM `creature_questrelation` WHERE `id` = '%u'", GetEntry());

    if(result)
    {
        do
        {
            fields = result->Fetch();
            Quest* qInfo = objmgr.QuestTemplates[fields[0].GetUInt32()];
            if (!qInfo) continue;

            addQuest(qInfo->GetQuestId());
        }
        while( result->NextRow() );

        delete result;
    }

    QueryResult *result1 = sDatabase.PQuery("SELECT `quest` FROM `creature_involvedrelation` WHERE `id` = '%u'", GetEntry());

    if(!result1) return;

    do
    {
        fields = result1->Fetch();
        Quest* qInfo = objmgr.QuestTemplates[fields[0].GetUInt32()];
        if (!qInfo) continue;

        addInvolvedQuest(qInfo->GetQuestId());
    }
    while( result1->NextRow() );

    delete result1;
}

void Creature::DeleteFromDB()
{
    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `creature` WHERE `guid` = '%u'", GetGUIDLow());
    sDatabase.PExecute("DELETE FROM `creature_involvedrelation` WHERE `id` = '%u'", GetGUIDLow());
    sDatabase.CommitTransaction();
}

float Creature::GetAttackDistance(Unit *pl) const
{
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

    if(getLevel() <= 55)
    {
        // decrease aggro range auras
        AuraList const& modDectectRangeList = GetAurasByType(SPELL_AURA_MOD_DETECT_RANGE);
        for(AuraList::const_iterator itr = modDectectRangeList.begin(); itr != modDectectRangeList.end(); ++itr)
            RetDistance += (*itr)->GetModifier()->m_amount;
    }

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if(RetDistance < 5 && sWorld.getRate(RATE_CREATURE_AGGRO) != 0)
        RetDistance = 5;

    return (RetDistance*sWorld.getRate(RATE_CREATURE_AGGRO));
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

        if(!IsStopped()) StopMoving();
    }
    Unit::setDeathState(s);

    if(s == JUST_DIED)
    {
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

void Creature::Say(char const* message, uint32 language)
{
    WorldPacket data;

    sChatHandler.FillMessageData( &data, NULL, CHAT_MSG_SAY, language, NULL, 0, message );
    SendMessageToSet( &data, false );
}

void Creature::MonsterSay(char const* message, uint32 language, uint64 targetGUID)
{
    WorldPacket data;

    sChatHandler.FillMessageData( &data, NULL, CHAT_MSG_MONSTER_SAY, language, NULL, targetGUID, message, this );
    SendMessageToSet( &data, false );
}

bool Creature::IsImmunedToSpell(SpellEntry const* spellInfo) const
{
    if (!spellInfo)
        return false;
    if( CREATURE_ELITE_WORLDBOSS == GetCreatureInfo()->rank )
        if( (IMMUNE_MECHANIC_FEAR == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_STUNDED == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_DAZED == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_BANISH == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_FREEZE == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_ROOT == spellInfo->Mechanic) ||
        (IMMUNE_MECHANIC_CONFUSED == spellInfo->Mechanic))
            return true;

    return Unit::IsImmunedToSpell(spellInfo);
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
        float dist = GetDistanceSq(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range * range || dist < minrange * minrange )
            continue;
        if(m_silenced)
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
        float dist = GetDistanceSq(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range * range || dist < minrange * minrange )
            continue;
        if(m_silenced)
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Creature::IsVisibleInGridForPlayer(Player* pl) const
{
    // Live player see live creatures or death creatures with corpse dissappiring time > 0
    if(pl->isAlive())
    {
        return isAlive() || m_deathTimer > 0;
    }

    // Dead player see live creatures near own corpse
    if(pl->getDeathState() == CORPSE)
    {
        if(isAlive())
        {
            if(Corpse* corpse = pl->GetCorpse())
            {
                // 20 - aggro distance for same level, 25 - max additinal distance if player level less that creature level
                if(corpse->IsWithinDistInMap(this,(20+25)*sWorld.getRate(RATE_CREATURE_AGGRO)))
                    return true;
            }
        }
    }

    // Dead player see Spirit Healer
    if(pl->isDead() && isSpiritHealer())
        return true;

    // and not see any other
    return false;
}

void Creature::CallAssistence()
{
    if (!m_AlreadyCallAssistence)
    {
        CastSpell(this,SPELL_ID_AGGRO, true);
    }
}

void Creature::SaveRespawnTime()
{
    sDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `guid` = '%u'", GetGUIDLow());
    if(m_respawnTime > time(NULL))                           // dead (no corpse)
        sDatabase.PExecute("INSERT INTO `creature_respawn` VALUES ( '%u', '" I64FMTD "' )", GetGUIDLow(),uint64(m_respawnTime));
    else if(m_deathTimer > 0)                               // dead (corpse)
        sDatabase.PExecute("INSERT INTO `creature_respawn` VALUES ( '%u', '" I64FMTD "' )", GetGUIDLow(),uint64(time(NULL)+m_respawnDelay+m_deathTimer/1000));
}