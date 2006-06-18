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
#include "Stats.h"
#include "Log.h"
#include "LootMgr.h"
#include "Chat.h"
#include "MapManager.h"
#include "FactionTemplateResolver.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"

// apply implementation of the singletons
#include "Policies/SingletonImp.h"

Creature::Creature() :
Unit(), i_AI(NULL), m_lootMoney(0), m_deathTimer(0), m_respawnTimer(0),
m_respawnDelay(25000), m_corpseDelay(60000), m_respawnradius(0.0),
itemcount(0), mTaxiNode(0), m_regenTimer(2000),
m_moveBackward(false), m_moveRandom(false), m_moveRun(false),
m_faction(0),m_emoteState(0),m_isPet(false)
{
    m_valuesCount = UNIT_END;

    memset(item_list, 0, sizeof(CreatureItem)*MAX_CREATURE_ITEMS);
    for(int i =0; i<3; ++i) respawn_cord[i] = 0.0;
}

Creature::~Creature()
{
    mQuests.clear( );
    for( SpellsList::iterator i = m_tspells.begin( ); i != m_tspells.end( ); i++ )
        delete (*i);

    m_tspells.clear();

    delete i_AI;
    i_AI = NULL;
}

void Creature::CreateTrainerSpells()
{
    TrainerSpell *tspell;
    Field *fields;
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `npc_trainer` WHERE `entry` = '%u'", GetCreatureInfo()->Entry);

    if(!result) return;

    SpellEntry * spellinfo;

    do
    {
        fields = result->Fetch();

        spellinfo = sSpellStore.LookupEntry(fields[2].GetUInt32());

        if(!spellinfo) continue;

        tspell = new TrainerSpell;
        tspell->spell = spellinfo;
        tspell->spellcost = fields[3].GetUInt32();
        tspell->reqspell = fields[4].GetUInt32();
        tspell->reqskill = fields[5].GetUInt32();
        tspell->reqskillvalue = fields[6].GetUInt32();

        m_tspells.push_back(tspell);

    } while( result->NextRow() );

}

//---------------------------------------------------------------//
/* with compiler optimzation, switch statement differs
 * from if statement.  Going to a switch statement can
 * be perform in order(1) where as an if statement has
 * an order(N) performances.
 */
void Creature::AIM_Update(const uint32 &diff)
{
    switch( m_deathState )
    {
        case JUST_DIED:
        {
            setDeathState( JUST_DIED );
            SetUInt32Value(UNIT_NPC_FLAGS, 0);
            i_AI->UpdateAI(diff);
            break;
        }
        case DEAD:
        {
            if( m_respawnTimer <= diff )
            {
                DEBUG_LOG("Respawning...");

                RemoveFlag (UNIT_FIELD_FLAGS, 0x4000000);
                SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
                setDeathState( ALIVE );
                clearUnitState(UNIT_STAT_ALL_STATE);
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

                DEBUG_LOG("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
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
                Regenerate( UNIT_FIELD_HEALTH, UNIT_FIELD_MAXHEALTH );
            Regenerate( UNIT_FIELD_POWER1, UNIT_FIELD_MAXPOWER1);
            m_regenTimer = 2000;
            break;
        }
        default:
            break;
    }
}

void Creature::Regenerate(uint16 field_cur, uint16 field_max)
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

    uint16 Spirit = GetUInt32Value(UNIT_FIELD_SPIRIT);

    if( HealthIncreaseRate <= 0 ) HealthIncreaseRate = 1;
    if( ManaIncreaseRate <= 0 ) ManaIncreaseRate = 1;

    uint32 addvalue = 0;

    switch (field_cur)
    {
        case UNIT_FIELD_HEALTH:
        {
            if( GetUInt32Value(UNIT_FIELD_POWER1)>0 )
                addvalue = uint32((Spirit*0.25) * HealthIncreaseRate);
            else
                addvalue = uint32((Spirit*0.80) * HealthIncreaseRate);
        }break;
        case UNIT_FIELD_POWER1:
            addvalue = uint32((Spirit/5 + 17) * ManaIncreaseRate);
            break;
    }

    curValue += addvalue;
    if (curValue > maxValue) curValue = maxValue;
    SetUInt32Value(field_cur, curValue);
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

bool Creature::Create (uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry)
{
    respawn_cord[0] = x;
    respawn_cord[1] = y;
    respawn_cord[2] = z;
    m_mapId =mapid;
    m_positionX =x;
    m_positionY=y;
    m_positionZ=z;
    m_orientation=ang;
    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    return  CreateFromProto(guidlow, Entry);

}

uint32 Creature::getDialogStatus(Player *pPlayer, uint32 defstatus)
{
    uint32 result = DIALOG_STATUS_NONE;
    uint32 status;
    uint32 quest_id;
    Quest *pQuest;

    for( std::list<Quest*>::iterator i = mInvolvedQuests.begin( ); i != mInvolvedQuests.end( ); i++ )
    {
        pQuest = *i;
        if ( !pQuest )
            continue;

        quest_id = pQuest->GetQuestInfo()->QuestId;
        status = pPlayer->GetQuestStatus( pQuest );
        if ( status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus( pQuest ) )
        {
            SetFlag(UNIT_DYNAMIC_FLAGS ,2);
            if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ) )
                return DIALOG_STATUS_REWARD_REP;
            else
                return DIALOG_STATUS_REWARD;
        }
        else if ( status == QUEST_STATUS_INCOMPLETE )
            result = DIALOG_STATUS_INCOMPLETE;
    }

    if( GetFlag(UNIT_DYNAMIC_FLAGS, 2) )
        RemoveFlag(UNIT_DYNAMIC_FLAGS, 2);

    if ( result == DIALOG_STATUS_INCOMPLETE )
        return result;

    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        pQuest = *i;
        if ( !pQuest )
            continue;

        quest_id = pQuest->GetQuestInfo()->QuestId;
        status = pPlayer->GetQuestStatus( pQuest );
        if ( status == QUEST_STATUS_NONE )
        {
            if ( pPlayer->CanSeeStartQuest( pQuest ) )
            {
                if ( pPlayer->SatisfyQuestLevel(pQuest, false) )
                {
                    if ( pQuest->HasSpecialFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ) )
                        return DIALOG_STATUS_REWARD_REP;
                    else
                        return DIALOG_STATUS_AVAILABLE;
                }
                result = DIALOG_STATUS_UNAVAILABLE;
            }
        }
    }

    if ( result == DIALOG_STATUS_UNAVAILABLE )
        return result;

    if ( defstatus == DIALOG_STATUS_NONE )
        return DIALOG_STATUS_NONE;
    else
        return DIALOG_STATUS_CHAT;
}

Quest *Creature::getNextAvailableQuest(Player *pPlayer, Quest *prevQuest)
{
    if ( !prevQuest )
        return NULL;
    uint32 quest_id = prevQuest->GetQuestInfo()->NextQuestId;
    Quest *pQuest;

    if( quest_id <= 0 )
        return NULL;

    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        pQuest = *i;
        if( pQuest->GetQuestInfo()->QuestId == quest_id )
        {
            if ( pPlayer->CanTakeQuest( pQuest, false ) )
                return pQuest;
            else
                return NULL;
        }
    }
    return NULL;
}

void Creature::prepareGossipMenu( Player *pPlayer,uint32 gossipid )
{
    PlayerMenu* pm=pPlayer->PlayerTalkClass;
    pm->ClearMenus();
    if(!m_goptions.size())
        LoadGossipOptions();
    GossipOption* gso;
    GossipOption* ingso;
    for( std::list<GossipOption*>::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        gso=*i;
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
            if(gso->Option!="" && cantalking )
            {
                pm->GetGossipMenu()->AddMenuItem((uint8)gso->Icon,gso->Option.c_str(), gossipid,gso->Action,false);
                ingso=gso;
            }
        }
    }
    if(pm->GetGossipMenu()->MenuItemCount()==1 && ingso->Id==8 && GetGossipCount( ingso->GossipId )>0)
    {
        pm->ClearMenus();
        for( std::list<GossipOption*>::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
        {
            gso=*i;
            if(gso->GossipId==ingso->Id)
            {
                if(gso->Option!="")
                    pm->GetGossipMenu()->AddMenuItem((uint8)gso->Icon,gso->Option.c_str(),ingso->GossipId,gso->Action,false);
            }
        }
    }
}

void Creature::sendPreparedGossip( Player* player)
{
    if(!player)
        return;
    GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();
    if ( !gossipmenu || gossipmenu->MenuItemCount() == 0 )
        return;

    if ( gossipmenu->MenuItemCount() == 1 )
        OnGossipSelect( player, 0 );
    else
        player->PlayerTalkClass->SendGossipMenu( GetNpcTextId(), GetGUID() );
}

void Creature::OnGossipSelect(Player* player, uint32 option)
{
    GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();
    uint32 action=gossipmenu->GetItem(option).m_gAction;
    uint32 zoneid=GetZoneId();
    uint64 guid=GetGUID();
    GossipOption *gossip=GetGossipOption( action );
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
                player->GetSession()->SendSpiritRessurect();
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            player->PrepareQuestMenu( guid );
            player->SendPreparedQuest( guid );
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
        case GOSSIP_OPTION_STABLEPET:
            player->GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            player->GetSession()->SendTrainerList(guid);
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            player->GetSession()->SendTaxiStatus(guid);
            break;
        case GOSSIP_OPTION_INNKEEPER:
            //_player->SetBindPoint( guid );
            player->GetSession()->SendBindPoint();
            break;
        case GOSSIP_OPTION_BANKER:
            player->GetSession()->SendShowBank( guid );
            break;
        case GOSSIP_OPTION_PETITIONER:
        case GOSSIP_OPTION_TABARDVENDOR:
            player->GetSession()->SendTabardVendorActivate( guid );
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            player->GetSession()->SendAuctionHello( guid );
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

void Creature::OnPoiSelect(Player* player, GossipOption *gossip)
{
    if(gossip->GossipId==GOSSIP_OPTION_GUARD || gossip->GossipId==GOSSIP_GUARD_SPELLTRAINER || gossip->GossipId==GOSSIP_GUARD_SKILLTRAINER)
    {
        float x,y;
        bool findnpc=false;
        uint32 icon=0;
        QueryResult *result;
        Field *fields;
        uint32 mapid=GetMapId();
        Map* map=MapManager::Instance().GetMap( mapid );
        uint16 areaflag=map->GetAreaFlag(m_positionX,m_positionY);
        uint32 zoneid=map->GetZoneId(areaflag);
        std::string areaname= gossip->Option;
        uint16 pflag;

        // use the action relate to creaturetemplate.trainer_type ?
        result= sDatabase.PQuery("SELECT `creature`.`position_x`,`creature`.`position_y` FROM `creature`,`creature_template` WHERE `creature`.`map` = '%u' AND `creature`.`id` = `creature_template`.`entry` AND `creature_template`.`trainer_type` = '%u';", mapid, gossip->Action );
        if(!result)
            return;
        do
        {
            fields = result->Fetch();
            x=fields[0].GetFloat();
            y=fields[1].GetFloat();
            pflag=map->GetAreaFlag(m_positionX,m_positionY);
            if(pflag==areaflag)
            {
                findnpc=true;
                break;
            }
        }while(result->NextRow());
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
    QueryResult *result= sDatabase.PQuery("SELECT `textid` FROM `npc_gossip_textid` WHERE `action` = '%u' AND `zoneid` ='%u';", action, zoneid );
    if(!result)
        return 0;
    Field *fields = result->Fetch();
    return fields[0].GetUInt32();
}

uint32 Creature::GetGossipCount( uint32 gossipid )
{
    uint32 count=0;
    GossipOption* gso;
    for( std::list<GossipOption*>::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        gso=*i;
        if(gso->GossipId == gossipid )
            count++;
    }
    return count;
}

uint32 Creature::GetNpcTextId()
{
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `npc_gossip` WHERE `npc_guid`= '%u';",GetGUIDLow());
    if(result)
    {
        Field *fields = result->Fetch();
        return fields[3].GetUInt32();
    }
    return DEFAULT_GOSSIP_MESSAGE;
}

std::string Creature::GetGossipTitle(uint8 type,uint32 id)
{
    GossipOption* gso;
    for( std::list<GossipOption*>::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        gso=*i;
        if(gso->Id==id && gso->NpcFlag==(uint32)type)
            return gso->Option;
    }
    return NULL;
}

GossipOption* Creature::GetGossipOption( uint32 id )
{
    GossipOption* gso;
    for( std::list<GossipOption*>::iterator i = m_goptions.begin( ); i != m_goptions.end( ); i++ )
    {
        gso=*i;
        if(gso->Action==id )
            return gso;
    }
    return NULL;
}

void Creature::LoadGossipOptions()
{
    uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

    QueryResult *result = sDatabase.PQuery( "SELECT * FROM `npc_option` WHERE (npcflag & %u)!=0;", npcflags );
    if(!result)
        return;
    GossipOption *go;
    do
    {
        Field *fields = result->Fetch();
        go=new GossipOption;
        go->Id= fields[0].GetUInt32();
        go->GossipId = fields[1].GetUInt32();
        go->NpcFlag=fields[2].GetUInt32();
        go->Icon=fields[3].GetUInt32();
        go->Action=fields[4].GetUInt32();
        go->Option=fields[5].GetString();
        addGossipOption(go);
    }while( result->NextRow() );
}

bool Creature::hasQuest(uint32 quest_id)
{
    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        if ((*i)->GetQuestInfo()->QuestId == quest_id)
            return true;
    }

    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id)
{
    for( std::list<Quest*>::iterator i = mInvolvedQuests.begin( ); i != mInvolvedQuests.end( ); i++ )
    {
        if ((*i)->GetQuestInfo()->QuestId == quest_id)
            return true;
    }

    return false;
}

void Creature::generateLoot()
{
    //DO NOT GENERATE LOOT IF IT'S NOT SPECIFIED!
    //ESPECIALLY USING RND, IT'S COMPLETELY WRONG
    //Looit is used as goods id for vendors
    //they have no loot but have items to trade

    uint32 lootid = GetCreatureInfo()->lootid;

    if (!HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_VENDOR) && lootid)
        FillLoot(&loot, lootid);

    if (GetCreatureInfo()->maxgold > 0)
    {
        uint32 diff = GetCreatureInfo()->maxgold - GetCreatureInfo()->mingold + 1;

        // rand() result (RAND_MAX) small for large gold loots
        if(diff > 10000)
        {
            uint32 gold_part              = rand() % (diff / 10000);
            uint32 silver_and_copper_part = rand() % (diff % 10000);
            loot.gold = gold_part * 10000 + silver_and_copper_part + GetCreatureInfo()->mingold;
        }
        else
        {
            loot.gold = rand() % diff + GetCreatureInfo()->mingold;
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
    //SendMonsterMove(x,y,z,false,run,time);

    WorldPacket data;
    data.Initialize( SMSG_MONSTER_MOVE );
    data << uint8(0xFF);
    data << GetGUID();
                                                            //point A
    data << GetPositionX() << GetPositionY() << GetPositionZ();
                                                            //little trick related to orientation
    data << (uint32)((*((uint32*)&GetOrientation())) & 0x30000000);
    data << uint8(WalkBack);                                //run / walk back only when still moving to point B
    data << uint32(run ? 0x00000100 : 0x00000000);
    data << time;
    data << uint32(1);
    data << x << y << z;                                    //point B
    WPAssert( data.size() == 50 );
    SendMessageToSet( &data, false );
}

void Creature::getSkinLoot()
{
    CreatureInfo *cinfo = GetCreatureInfo();

    if(cinfo->type == CREATURE_TYPE_DRAGON)
        FillSkinLoot(&loot,8165);
    if(cinfo->family ==CREATURE_FAMILY_TURTLE)
        FillSkinLoot(&loot,8167);
    uint32 level = getLevel();
    if(level > 48)
        FillSkinLoot(&loot,(urand(0,10)<8 ? 8170 :8171));
    else if(level > 36)
        FillSkinLoot(&loot,(urand(0,10)<8 ? 4304 :8169));
    else if(level > 25)
        FillSkinLoot(&loot,(urand(0,10)<8 ? 4234 :4235));
    else if(level > 15)
        FillSkinLoot(&loot,(urand(0,10)<8 ? 2319 :4232));
    else if(level > 3)
        FillSkinLoot(&loot,(urand(0,10)<8 ? 2318 :783));
    else if(level <= 3)
        FillSkinLoot(&loot,2934);
}

void Creature::SaveToDB()
{
    std::stringstream ss;
    sDatabase.PExecute("DELETE FROM `creature` WHERE `guid` = '%u'", GetGUIDLow());

    ss << "INSERT INTO `creature` VALUES (";

    ss << GetGUIDLow () << ","
        << GetEntry() << ","
        << m_mapId <<","
        << m_positionX << ","
        << m_positionY << ","
        << m_positionZ << ","
        << m_orientation << ","
        << m_respawnDelay << ","                            //spawn time min                          // fix me: store x-y delay but not 1
        << m_respawnDelay << ","                            //spawn time max
        << (float) 0  << ","                                //spawn distance (float)
        << (uint32) (0) << ","                              //currentwaypoint
        << respawn_cord[0] << ","                           //spawn_position_x
        << respawn_cord[1] << ","                           //spawn_position_y
        << respawn_cord[2] << ","                           //spawn_position_z
        << (float)(0) << ","                                //spawn_orientation
        << GetUInt32Value(UNIT_FIELD_HEALTH) << ","         //curhealth
        << GetUInt32Value(UNIT_FIELD_POWER2) << ","         //curmana
        << m_respawnTimer << ","                            //respawntimer

        << (uint32)(m_deathState) << ","                    // is it really death state or just state?
    //or
    //<< (uint32)(m_state) << ","                     // is it really death state or just state?

        << GetUInt32Value(UNIT_NPC_FLAGS) << ","            //npcflags
        << GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) << ","
        << "'')";                                           // should save auras

    sDatabase.Execute( ss.str( ).c_str( ) );
}

bool Creature::CreateFromProto(uint32 guidlow,uint32 Entry)
{
    Object::_Create(guidlow, HIGHGUID_UNIT);
    SetUInt32Value(OBJECT_FIELD_ENTRY,Entry);
    CreatureInfo *cinfo = objmgr.GetCreatureTemplate(Entry);
    if(!cinfo)
    {
        sLog.outString("Error: creature entry %u does not exist.",Entry);
        return false;
    }
    SetUInt32Value(UNIT_FIELD_DISPLAYID,cinfo->DisplayID );
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID,cinfo->DisplayID );

    SetUInt32Value(UNIT_FIELD_MAXHEALTH,cinfo->maxhealth );
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH,cinfo->maxhealth );

    SetUInt32Value(UNIT_FIELD_MAXPOWER2, cinfo->maxmana);   //MAX Mana
    SetUInt32Value(UNIT_FIELD_BASE_MANA, cinfo->maxmana);

    SetUInt32Value(UNIT_FIELD_LEVEL,cinfo->level);
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction);
    SetUInt32Value(UNIT_NPC_FLAGS,cinfo->npcflag);

    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,cinfo->baseattacktime);

    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME,cinfo->rangeattacktime);
    SetUInt32Value(UNIT_FIELD_FLAGS,cinfo->Flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS,cinfo->dynamicflags);

    SetUInt32Value(UNIT_FIELD_ARMOR,cinfo->armor);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_01,cinfo->resistance1);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_02,cinfo->resistance2);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_03,cinfo->resistance3);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_04,cinfo->resistance4);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_05,cinfo->resistance5);
    SetUInt32Value(UNIT_FIELD_RESISTANCES_06,cinfo->resistance6);

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

    SetFloatValue(UNIT_FIELD_MINDAMAGE,cinfo->mindmg);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE,cinfo->maxdmg);

    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,cinfo->minrangedmg );
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,cinfo->maxrangedmg);

    if (cinfo->mount != 0)
    {
        SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, cinfo->mount);
    }

    m_spells[0] = cinfo->spell1;
    m_spells[1] = cinfo->spell2;
    m_spells[2] = cinfo->spell3;
    m_spells[3] = cinfo->spell4;

    SetSpeed( cinfo->speed ) ;
    return true;
}

bool Creature::LoadFromDB(uint32 guid)
{

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `creature` WHERE `guid` = '%u';", guid);
    if(!result)
        return false;

    Field *fields = result->Fetch();

    if(!Create(guid,fields[2].GetUInt32(),fields[3].GetFloat(),fields[4].GetFloat(),
        fields[5].GetFloat(),fields[6].GetFloat(),fields[1].GetUInt32()))
        return false;

    SetUInt32Value(UNIT_FIELD_HEALTH,fields[15].GetUInt32());
    //fix me current mana

    SetUInt32Value(UNIT_NPC_FLAGS,fields[19].GetUInt32());
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,fields[20].GetUInt32());

    respawn_cord[0] = fields[11].GetFloat();
    respawn_cord[1] = fields[12].GetFloat();
    respawn_cord[2] = fields[13].GetFloat();

    m_respawnDelay =(fields[7].GetUInt32()+fields[8].GetUInt32())*1000/2;
    m_respawnTimer = fields[17].GetUInt32();
    m_deathState = (DeathState)fields[18].GetUInt32();

    delete result;

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

    QueryResult *result = sDatabase.PQuery("SELECT `item`,`buycount`,`maxcount`,`incrtime` FROM `npc_vendor` WHERE `entry` = '%u';", GetEntry());

    if(!result) return;

    do
    {
        Field *fields = result->Fetch();

        if (GetItemCount() >= MAX_CREATURE_ITEMS)
        {
            sLog.outError( "Vendor %u has too many items (%u >= %i). Check the DB!", GetUInt32Value(OBJECT_FIELD_ENTRY), GetItemCount(), MAX_CREATURE_ITEMS );
            break;
        }

        AddItem( fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32());
    }
    while( result->NextRow() );

    delete result;

}

void Creature::_LoadQuests()
{
    mQuests.clear();
    mInvolvedQuests.clear();

    Field *fields;
    Quest *pQuest;

    QueryResult *result = sDatabase.PQuery("SELECT * FROM `creature_questrelation` WHERE `id` = '%u';", GetEntry ());

    if(result)
    {
        do
        {
            fields = result->Fetch();
            pQuest = objmgr.GetQuest( fields[1].GetUInt32() );
            if (!pQuest) continue;

            addQuest(pQuest);
        }
        while( result->NextRow() );

        delete result;
    }

    QueryResult *result1 = sDatabase.PQuery("SELECT * FROM `creature_involvedrelation` WHERE `id` = '%u';", GetEntry ());

    if(!result1) return;

    do
    {
        fields = result1->Fetch();
        pQuest = objmgr.GetQuest( fields[1].GetUInt32() );
        if (!pQuest) continue;

        addInvolvedQuest(pQuest);
    }
    while( result1->NextRow() );

    delete result1;
}

void Creature::DeleteFromDB()
{

    sDatabase.PExecute("DELETE FROM `creature` WHERE `guid` = '%u'", GetGUIDLow());
    sDatabase.PExecute("DELETE FROM `npc_vendor` WHERE `entry` = '%u'", GetEntry());
    sDatabase.PExecute("DELETE FROM `npc_trainer` WHERE `entry` = '%u'", GetEntry());
    sDatabase.PExecute("DELETE FROM `creature_involvedrelation` WHERE `id` = '%u'", GetGUIDLow());
}

float Creature::GetAttackDistance(Unit *pl)
{
    uint16 playlevel     = (uint16)pl->GetUInt32Value(UNIT_FIELD_LEVEL);
    uint16 creaturelevel = (uint16)GetUInt32Value(UNIT_FIELD_LEVEL);
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

CreatureInfo *Creature::GetCreatureInfo()
{
    return objmgr.GetCreatureTemplate(GetEntry());
}

void Creature::Say(char const* message, uint32 language)
{
    WorldPacket data;

    sChatHandler.FillMessageData( &data, NULL, CHAT_MSG_SAY, language, NULL, message );
    SendMessageToSet( &data, false );
}
