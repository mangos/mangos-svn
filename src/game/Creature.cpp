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
m_respawnDelay(25000), m_corpseDelay(120000), m_respawnradius(0.0),
itemcount(0), mTaxiNode(0),
m_moveBackward(false), m_moveRandom(false), m_moveRun(false),
i_creatureState(UNIT_STAT_STOPPED), m_faction(0),m_emoteState(0),m_isPet(false)
{
    m_valuesCount = UNIT_END;

    memset(item_list, 0, sizeof(CreatureItem)*MAX_CREATURE_ITEMS);
    for(int i =0; i<3; ++i) respawn_cord[i] = 0.0;
    m_isPet = false;
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
    QueryResult *result = sDatabase.PQuery("SELECT * FROM trainers WHERE guid = %d", GetCreatureInfo()->Entry);

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
            SetUInt32Value(UNIT_NPC_FLAGS, 0);
            setDeathState( CORPSE );

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
                ClearState(UNIT_STAT_ALL_STATE);
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
                DEBUG_LOG("Removing corpse... %d ", GetUInt32Value(OBJECT_FIELD_ENTRY));
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

    return  CreateFromProto(guidlow, Entry);

}

uint32 Creature::getDialogStatus(Player *pPlayer, uint32 defstatus)
{
    bool wasReward  = false;
    bool wasRewardRep  = false;
    bool wasAvail   = false;
    bool wasIncompl = false;

    bool wasAvailShow   = false;
    bool wasUnavailShow = false;

    uint32 status;
    QuestInfo *questinfo;
    Quest *pQuest;
    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        pQuest=*i;
        questinfo = pQuest->GetQuestInfo();
        if ( pQuest == NULL ) continue;
        status = pPlayer->getQuestStatus(questinfo->QuestId);

        if ( !pQuest->PreReqSatisfied( pPlayer ) ||
            !pQuest->IsCompatible( pPlayer ) ||
            pQuest->RewardIsTaken( pPlayer )
            ) continue;
        if ( status == QUEST_STATUS_AVAILABLE )
        {
            if ( pQuest->LevelSatisfied( pPlayer ) )
            {
                pPlayer->PlayerTalkClass->SendPointOfInterest(GetPositionX(), GetPositionY(), 6, 6, 30, questinfo->Title);
                wasAvailShow = true;
            }
            wasAvail   = true;
        }

        if ( status == QUEST_STATUS_UNAVAILABLE )
        {
            if ( pQuest->CanShowUnsatified( pPlayer ) )
                wasUnavailShow = true;
        }

        if ( status == QUEST_STATUS_NONE )
        {
            if ( pQuest->PreReqSatisfied( pPlayer ) && pQuest->IsCompatible( pPlayer ) && !pQuest->RewardIsTaken( pPlayer ))
            {
                if ( pQuest->LevelSatisfied( pPlayer ) )
                {
                    pPlayer->addNewQuest(pQuest, QUEST_STATUS_AVAILABLE );
                    wasAvailShow = true;
                }
                else if(pQuest->CanShowUnsatified( pPlayer ))
                {
                    pPlayer->addNewQuest(pQuest, QUEST_STATUS_UNAVAILABLE );
                    wasUnavailShow = true;
                }
                //wasAvail = true;
            }
            else if (pQuest->IsCompatible( pPlayer ) && pQuest->CanShowUnsatified( pPlayer ) )
            {
                pPlayer->addNewQuest(pQuest, QUEST_STATUS_UNAVAILABLE );
                wasUnavailShow = true;
            }
        }
    }

    for( std::list<Quest*>::iterator i = mInvolvedQuests.begin( ); i != mInvolvedQuests.end( ); i++ )
    {
        pQuest=*i;
        questinfo = pQuest->GetQuestInfo();
        if ( pQuest == NULL ) continue;
        status = pPlayer->getQuestStatus(questinfo->QuestId);

        if ( status == QUEST_STATUS_INCOMPLETE )
        {
            if ( pPlayer->isQuestComplete(pQuest ) )
            {
                pPlayer->PlayerTalkClass->SendPointOfInterest(GetPositionX(), GetPositionY(), 4, 6, 30, questinfo->Title);
                wasReward = true;
            }
            else
                wasIncompl = true;
        }
        if ( status == QUEST_STATUS_COMPLETE)
        {
            if (pQuest->HasFlag( QUEST_SPECIAL_FLAGS_REPEATABLE ))
                wasRewardRep = true;
            else if( !pPlayer->getQuestRewardStatus(questinfo->QuestId))
            {
                pPlayer->PlayerTalkClass->SendPointOfInterest(GetPositionX(), GetPositionY(), 4, 6, 30, questinfo->Title);
                wasReward = true;
            }
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

    //if (wasAnavail)
    //{
    if (wasUnavailShow)
        return DIALOG_STATUS_UNAVAILABLE;
    //}

    return DIALOG_STATUS_NONE;
}

Quest *Creature::getNextAvailableQuest(Player *pPlayer, Quest *prevQuest)
{
    uint32 qid=prevQuest->GetQuestInfo()->NextQuestId;
    if(qid<=0)
        return NULL;
    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        Quest* nextquest=*i;
        if(nextquest->GetQuestInfo()->QuestId==qid)
            if( nextquest->CanBeTaken(pPlayer))
                return nextquest;
    }
    return NULL;
}

void Creature::prepareQuestMenu( Player *pPlayer )
{
    uint32 quest_id;
    uint32 status;
    Quest *pQuest;
    QuestMenu *qm=pPlayer->PlayerTalkClass->GetQuestMenu();
    qm->ClearMenu();
    for( std::list<Quest*>::iterator i = mQuests.begin( ); i != mQuests.end( ); i++ )
    {
        pQuest = *i;
        quest_id=pQuest->GetQuestInfo()->QuestId;
        status = pPlayer->getQuestStatus(quest_id);
        if ( ( status == QUEST_STATUS_AVAILABLE ) && ( pQuest->CanBeTaken(pPlayer) ) )
            qm->AddMenuItem( quest_id, DIALOG_STATUS_AVAILABLE, true );
    }

    for( std::list<Quest*>::iterator i = mInvolvedQuests.begin( ); i != mInvolvedQuests.end( ); i++ )
    {
        pQuest = *i;
        quest_id=pQuest->GetQuestInfo()->QuestId;
        status = pPlayer->getQuestStatus(quest_id);
        if(qm->HasItem(quest_id))
            continue;
        if ( status == QUEST_STATUS_INCOMPLETE )
            qm->AddMenuItem( quest_id, DIALOG_STATUS_INCOMPLETE, false );
        if ( status == QUEST_STATUS_COMPLETE && !pPlayer->getQuestRewardStatus(quest_id))
        {
            qm->AddMenuItem( quest_id, DIALOG_STATUS_REWARD, false );
            float x,y,z;
            GetRespawnCoord(x,y,z);
            std::string mapname=pQuest->GetQuestInfo()->Title;
            pPlayer->PlayerTalkClass->SendPointOfInterest(x, y, 4, 2, 30, mapname.c_str());
            //player->PlayerTalkClass->SendGossipMenu(gossip->TextId, GetGUID());
        }
    }
}

void Creature::sendPreparedQuest(Player *player)
{
    QuestMenu* _QuestMenu   = player->PlayerTalkClass->GetQuestMenu();
    if( !_QuestMenu || _QuestMenu->MenuItemCount() <1 )
        return;
    uint64 guid=GetGUID();
    if ( _QuestMenu->MenuItemCount() == 1 )
    {
        Quest *pQuest = objmgr.GetQuest( _QuestMenu->GetItem(0).m_qId );
        if (pQuest)
        {
            if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_REWARD )
                player->PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0 );

            if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_AVAILABLE )
                player->PlayerTalkClass->SendQuestDetails( pQuest, guid, true );

            if ( _QuestMenu->GetItem(0).m_qIcon == DIALOG_STATUS_INCOMPLETE )
            {
                if ( player->isQuestComplete( pQuest ) )
                    player->PlayerTalkClass->SendQuestReward( pQuest, guid, true, NULL, 0);
                else
                    player->PlayerTalkClass->SendRequestedItems(pQuest, guid, false);
            }
        }
    }
    else if (_QuestMenu->MenuItemCount() > 1 )
    {
        QEmote qe;
        std::string title="";
        uint32 textid=GetNpcTextId();
        GossipText * gossiptext=objmgr.GetGossipText(textid);
        if(!gossiptext)
        {
            qe._Delay=TEXTEMOTE_MASSAGE;                    //zyg: player emote
            qe._Emote=TEXTEMOTE_HELLO;                      //zyg: NPC emote
            title="Do Quest?";
        }
        else
        {
            qe=gossiptext->Options[0].Emotes[0];
            title=gossiptext->Options[0].Text_0;
            if(&title==NULL)
                title="";
        }
        player->PlayerTalkClass->SendQuestMenu( qe, title, guid );
    }
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
            prepareQuestMenu( player );
            sendPreparedQuest( player );
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
        AreaTableEntry* area=sAreaStore.LookupEntry(areaflag);
        uint32 zoneid=area->zone;
        std::string areaname= gossip->Option;
        uint16 pflag;
        //use the action relate to creaturetemplate.trainer_type ?
        result= sDatabase.PQuery("SELECT creatures.positionx,creatures.positiony FROM creatures,creaturetemplate WHERE creatures.mapid=%u AND creatures.entry=creaturetemplate.entry AND creaturetemplate.trainer_type=%u;", mapid, gossip->Action );
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
            player->PlayerTalkClass->SendTalking( "$N£¬Sorry", "Here no this person.");
            return;
        }
        //need add more case.
        switch(gossip->Action)
        {
            case GOSSIP_GUARD_BANK:
                icon=ICON_POI_4;
                break;
            case GOSSIP_GUARD_RIDE:
                icon=ICON_POI_40;
                break;
            case GOSSIP_GUARD_GUILD:
                icon=ICON_POI_18;
                break;
            default:
                icon=ICON_POI_5;
                break;
        }
        uint32 textid=GetGossipTextId( gossip->Action, zoneid );
        player->PlayerTalkClass->SendTalking( textid );
        player->PlayerTalkClass->SendPointOfInterest( x, y, icon, 2, 15, areaname.c_str() );
    }
}

uint32 Creature::GetGossipTextId(uint32 action, uint32 zoneid)
{
    QueryResult *result= sDatabase.PQuery("SELECT textid FROM gossip_textid WHERE action=%u and zoneid=%u;", action, zoneid );
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
    QueryResult *result = sDatabase.PQuery("SELECT * FROM npc_gossip where npc_guid=%u;",GetGUIDLow());
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
    uint32 zoneid=GetZoneId();
    QueryResult *result = sDatabase.PQuery( "SELECT * FROM npc_options where (npcflag & %u)!=0;", npcflags );
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

    uint32 level = getLevel();

    loot.gold = rand() % (level * level * 5);
}

void Creature::AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run)
{
    WorldPacket data;
    data.Initialize( SMSG_MONSTER_MOVE );
    data << uint8(0xFF);
    data << GetGUID();
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    data << (uint32)getMSTime();
    data << uint8(0);
    data << uint32(run ? 0x00000100 : 0x00000000);
    data << time;
    data << uint32(1);
    data << x << y << z;
    //WPAssert( data.size() == 49 );
    SendMessageToSet( &data, false );
}

void Creature::setItemId(int slot, uint32 tempitemid) { item_list[slot].ItemId=tempitemid; }
void Creature::setItemAmount(int slot, int tempamount) { item_list[slot].amount = tempamount; }

void Creature::setItemAmountById(uint32 tempitemid, int tempamount)
{
    int i;
    for(i=0;i<itemcount;i++)
    {
        if(item_list[i].ItemId == tempitemid)
            item_list[i].amount = tempamount;
    }
}

void Creature::addItem(uint32 itemid, uint32 amount)
{
    item_list[itemcount].ItemId = itemid;
    item_list[itemcount++].amount = amount;

}

int Creature::getItemSlotById(uint32 itemid)
{
    int i;
    for(i=0;i<itemcount;i++)
    {
        if(item_list[i].ItemId == itemid)
            return i;
    }
    return -1;
}

void Creature::SaveToDB()
{
    std::stringstream ss;
    sDatabase.PExecute("DELETE FROM creatures WHERE guid = '%u'", GetGUIDLow());

    ss << "INSERT INTO creatures VALUES (";

    ss << GetGUIDLow () << ","
        << GetEntry() << ","
        << m_mapId <<","
        << m_positionX << ","
        << m_positionY << ","
        << m_positionZ << ","
        << m_orientation << ","
        << m_respawnDelay << ","                            //fix me: store x-y delay but not 1
        << m_respawnDelay << ","
        << (float) 0  << ","
        << (uint32) (0) << ","
        << respawn_cord[0] << ","
        << respawn_cord[1] << ","
        << respawn_cord[2] << ","
        << (float)(0) << ","
        << GetUInt32Value(UNIT_FIELD_HEALTH) << ","
        << (uint32)(0) << ","
        << m_respawnTimer << ","
        << (uint32)(m_deathState) << ","
        << GetUInt32Value(UNIT_NPC_FLAGS) << ","
        << GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) << ","
        << "'')";

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

    SetUInt32Value(UNIT_FIELD_BASE_MANA, cinfo->maxmana);
    //fix me : max mana
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

    m_spells[0] = cinfo->spell1;
    m_spells[1] = cinfo->spell2;
    m_spells[2] = cinfo->spell3;
    m_spells[3] = cinfo->spell4;

    SetSpeed( cinfo->speed ) ;
    return true;
}

void Creature::LoadFromDB(uint32 guid)
{

    QueryResult *result = sDatabase.PQuery("SELECT * FROM creatures WHERE guid = '%u';", guid);
    ASSERT(result);

    Field *fields = result->Fetch();

    Create(guid,fields[2].GetUInt32(),fields[3].GetFloat(),fields[4].GetFloat(),
        fields[5].GetFloat(),fields[6].GetFloat(),fields[1].GetUInt32());

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
}

void Creature::_LoadGoods()
{

    itemcount = 0;
    //this needs fix, it's wrong to save goods by vendor GUID, as it may change del-spawn op
    //it should be stored by 'entry' field id (c) Phantomas
    //Use entry instead of GUID, fixed by Ant009 need more test

    QueryResult *result = sDatabase.PQuery("SELECT * FROM vendors WHERE entry = '%u';", GetEntry());

    if(!result) return;

    do
    {
        Field *fields = result->Fetch();

        if (getItemCount() >= MAX_CREATURE_ITEMS)
        {
            sLog.outError( "Vendor %u has too many items (%u >= %i). Check the DB!", GetUInt32Value(OBJECT_FIELD_ENTRY), getItemCount(), MAX_CREATURE_ITEMS );
            break;
        }

        setItemId(getItemCount() , fields[1].GetUInt32());
        setItemAmount(getItemCount() , fields[2].GetUInt32());
        increaseItemCount();
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

    QueryResult *result = sDatabase.PQuery("SELECT * FROM creaturequestrelation WHERE creatureid = '%u' ORDER BY questid;", GetEntry ());

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

    QueryResult *result1 = sDatabase.PQuery("SELECT * FROM creatureinvolvedrelation WHERE creatureid = '%u' ORDER BY questid;", GetUInt32Value (OBJECT_FIELD_ENTRY));

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

    sDatabase.PExecute("DELETE FROM creatures WHERE guid = '%u'", GetGUIDLow());
    sDatabase.PExecute("DELETE FROM vendors WHERE entry = '%u'", GetEntry());
    sDatabase.PExecute("DELETE FROM trainers WHERE guid = '%u'", GetGUIDLow());
    sDatabase.PExecute("DELETE FROM creaturequestrelation WHERE creatureid = '%u'", GetGUIDLow());
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

ItemPrototype *Creature::getProtoByslot(uint32 slot)
{ return objmgr.GetItemPrototype(item_list[slot].ItemId); }

CreatureInfo *Creature::GetCreatureInfo()
{
    return objmgr.GetCreatureTemplate(GetEntry());
}
