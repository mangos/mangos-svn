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
#include "Config/ConfigEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSocket.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Group.h"
#include "UpdateData.h"
#include "Chat.h"
#include "Database/DBCStores.h"
#include "ChannelMgr.h"
#include "LootMgr.h"
#include "ProgressBar.hpp"
#include "MapManager.h"
#include "ScriptCalls.h"
#include "CreatureAIRegistry.h" // need for Game::Initialize()
#include "Policies/SingletonImp.h"
#include "EventSystem.h"

INSTANTIATE_SINGLETON_1( World );

extern bool LoadScriptingModule();



World::World()
{
    m_playerLimit = 0;
    m_allowMovement = true;
}


World::~World()
{
}


WorldSession* World::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if(itr != m_sessions.end())
        return itr->second;
    else
        return 0;
}


void World::RemoveSession(uint32 id)
{
    SessionMap::iterator itr = m_sessions.find(id);

    if(itr != m_sessions.end())
    {
        delete itr->second;
        m_sessions.erase(itr);
    }
}


void World::AddSession(WorldSession* s)
{
    ASSERT(s);
    m_sessions[s->GetAccountId()] = s;
}

void World::SetInitialWorldSettings()
{
    std::string dataPath="./";
    std::string tmpPath="";

    srand((unsigned int)time(NULL));
    m_lastTick = time(NULL);
    
    time_t tiempo;
    char hour[3];
    char minute[3];
    char second[3];
    struct tm *tmPtr;
    tiempo = time(NULL);
    tmPtr = localtime(&tiempo);
    strftime( hour, 3, "%H", tmPtr );
    strftime( minute, 3, "%M", tmPtr );
    strftime( second, 3, "%S", tmPtr );
    
    if(!sConfig.GetString("DataDir",&dataPath))
	dataPath="./";
    else
    {
	if(dataPath.at(dataPath.length()-1)!='/')
	    dataPath.append("/");
    }

    sLog.outString("Using DataDir %s ...",dataPath.c_str());

    m_gameTime = (3600*atoi(hour))+(atoi(minute)*60)+(atoi(second));
    
    sDatabase.PExecute("UPDATE characters set online=0;");
    
    new ChannelMgr;


    sLog.outString("Initialize data stores...");
    barGoLink bar( 12 );
    bar.step();
    
    tmpPath=dataPath;
    tmpPath.append("dbc/EmotesText.dbc");
    sEmoteStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/Spell.dbc");
    sSpellStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellRange.dbc");
    sSpellRange.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellCastTimes.dbc");
    sCastTime.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellDuration.dbc");
    sSpellDuration.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/SpellRadius.dbc");
    sSpellRadius.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/Talent.dbc");
    sTalentStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/Faction.dbc");
    sFactionStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/FactionTemplate.dbc");
    sFactionTemplateStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/ItemDisplayInfo.dbc");
    sItemDisplayTemplateStore.Load((char *)(tmpPath.c_str()));
    bar.step();
	
    tmpPath=dataPath;
    tmpPath.append("dbc/ItemSet.dbc");
    sItemSetStore.Load((char *)(tmpPath.c_str()));
    bar.step();

    tmpPath=dataPath;
    tmpPath.append("dbc/AreaTable.dbc");
    sAreaStore.Load((char *)(tmpPath.c_str()));

    sLog.outString( "" );
    sLog.outString( ">> Loaded 12 data stores" );
    sLog.outString( "" );



    sLog.outString( "Loading Quests..." );
    objmgr.LoadQuests();

    
    sLog.outString( "Loading NPC Texts..." );
    objmgr.LoadGossipText();

    
    sLog.outString( "Loading Quest Area Triggers..." );
    objmgr.LoadAreaTriggerPoints();

    
    sLog.outString( "Loading Items..." );
    objmgr.LoadItemPrototypes();
    objmgr.LoadAuctions();
    objmgr.LoadAuctionItems();
    objmgr.LoadMailedItems();


    sLog.outString( "Loading Creature templates..." );
    objmgr.LoadCreatureTemplates();

    
    sLog.outString( "Loading Guilds..." );
    objmgr.LoadGuilds();
    
    sLog.outString( "Loading Teleport Coords..." );
    objmgr.LoadTeleportCoords();
    
    objmgr.SetHighestGuids();

    LoadCreaturesLootTables();

    sLog.outString( "Loading Game Object Templates..." );
    objmgr.LoadGameobjectInfo();
    
    if(!LoadScriptingModule())
			exit(1);
				
		sLog.outString( "Initializing Scripts..." );
		Script->ScriptsInit();

    m_timers[WUPDATE_OBJECTS].SetInterval(0);
    m_timers[WUPDATE_SESSIONS].SetInterval(0);
    m_timers[WUPDATE_AUCTIONS].SetInterval(1000);

    MaNGOS::Game::Initialize();
    sLog.outString( "WORLD: SetInitialWorldSettings done" );

    StartEventSystem();
    sLog.outString( "WORLD: Starting Event System" );
}

void World::Update(time_t diff)
{
    for(int i = 0; i < WUPDATE_COUNT; i++)
        if(m_timers[i].GetCurrent()>=0)
            m_timers[i].Update(diff);
    else m_timers[i].SetCurrent(0);

    _UpdateGameTime();

    if (m_timers[WUPDATE_AUCTIONS].Passed())
    {
        m_timers[WUPDATE_AUCTIONS].Reset();
        ObjectMgr::AuctionEntryMap::iterator itr,next;
        for (itr = objmgr.GetAuctionsBegin(); itr != objmgr.GetAuctionsEnd();itr = next)
        {
            next = itr;
            next++;
            if (time(NULL) > (itr->second->time))
            {
                if (itr->second->bidder == 0)
                {
                    Mail *m = new Mail;
                    m->reciever = itr->second->owner;
                    m->body = "";
                    m->sender = itr->second->owner;
                    m->checked = 0;
                    m->COD = 0;
                    m->messageID = objmgr.GenerateMailID();
                    m->money = 0;
                    m->time = time(NULL) + (29 * 3600);
                    m->subject = "Your item failed to sell";
                    m->item = itr->second->item;
                    Item *it = objmgr.GetAItem(m->item);
                    objmgr.AddMItem(it);

                    std::stringstream ss;
                    ss << "INSERT INTO mailed_items (guid, data) VALUES ("
                        << it->GetGUIDLow() << ", '";
                    for(uint16 i = 0; i < it->GetValuesCount(); i++ )
                    {
                        ss << it->GetUInt32Value(i) << " ";
                    }
                    ss << "' )";
                    sDatabase.Execute( ss.str().c_str() );

		    sDatabase.PExecute("DELETE FROM mail WHERE mailID = '%d'",m->messageID);

                    sDatabase.PExecute("INSERT INTO mail (mailId,sender,reciever,subject,body,item,time,money,COD,checked) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->reciever, m->subject.c_str(), m->body.c_str(), m->item, m->time, m->money, 0,  m->checked);

                    uint64 rcpl = m->reciever;
                    std::string pname;
                    objmgr.GetPlayerNameByGUID(rcpl,pname);
                    Player *rpl = objmgr.GetPlayer(pname.c_str());
                    if (rpl)
                    {
                        rpl->AddMail(m);
                    }
		    sDatabase.PExecute("DELETE FROM auctionhouse WHERE itemowner = '%d'",m->reciever);
		    sDatabase.PExecute("DELETE FROM auctioned_items WHERE guid = '%d'",m->item);
		    sDatabase.PExecute("DELETE FROM bids WHERE Id = '%d'",itr->second->Id);

                    objmgr.RemoveAuction(itr->second->Id);
                }
                else
                {
                    Mail *m = new Mail;
                    m->reciever = itr->second->owner;
                    m->body = "";
                    m->sender = itr->second->bidder;
                    m->checked = 0;
                    m->COD = 0;
                    m->messageID = objmgr.GenerateMailID();
                    m->money = itr->second->bid;
                    m->time = time(NULL) + (29 * 3600);
                    m->subject = "Your item sold!";
                    m->item = 0;
                   
		    sDatabase.PExecute("DELETE FROM mail WHERE mailID = '%d'",m->messageID);

                    sDatabase.PExecute("INSERT INTO mail (mailId,sender,reciever, subject,body,item,time,money,COD,checked) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->reciever, m->subject.c_str(), m->body.c_str(), m->item, m->time, m->money, 0, m->checked);

                    uint64 rcpl = m->reciever;
                    std::string pname;
                    objmgr.GetPlayerNameByGUID(rcpl,pname);
                    Player *rpl = objmgr.GetPlayer(pname.c_str());
                    if (rpl)
                    {
                        rpl->AddMail(m);
                    }

                    Mail *mn = new Mail;
                    mn->reciever = itr->second->bidder;
                    mn->body = "";
                    mn->sender = itr->second->owner;
                    mn->checked = 0;
                    mn->COD = 0;
                    mn->messageID = objmgr.GenerateMailID();
                    mn->money = 0;
                    mn->time = time(NULL) + (29 * 3600);
                    mn->subject = "Your won an item!";
                    mn->item = itr->second->item;
                    Item *it = objmgr.GetAItem(itr->second->item);
                    objmgr.AddMItem(it);

                    std::stringstream ss;
                    ss << "INSERT INTO mailed_items (guid, data) VALUES ("
                        << it->GetGUIDLow() << ", '";
                    for(uint16 i = 0; i < it->GetValuesCount(); i++ )
                    {
                        ss << it->GetUInt32Value(i) << " ";
                    }
                    ss << "' )";
                    sDatabase.Execute( ss.str().c_str() );

		    sDatabase.PExecute("DELETE FROM mail WHERE mailID = '%d'", mn->messageID);

                    sDatabase.PExecute("INSERT INTO mail (mailId,sender,reciever,subject,body,item,time,money,COD,checked) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", mn->messageID, mn->sender, mn->reciever, mn->subject.c_str(), mn->body.c_str(), mn->item, mn->time, mn->money, 0, mn->checked);

                    uint64 rcpl1 = mn->reciever;
                    std::string pname1;
                    objmgr.GetPlayerNameByGUID(rcpl1,pname1);
                    Player *rpl1 = objmgr.GetPlayer(pname1.c_str());
                    if (rpl1)
                    {
                        rpl1->AddMail(mn);
                    }
                    objmgr.RemoveAItem(itr->second->item);
                    objmgr.RemoveAuction(itr->second->Id);
                }
            }
        }
    }
    if (m_timers[WUPDATE_SESSIONS].Passed())
    {
        m_timers[WUPDATE_SESSIONS].Reset();

        SessionMap::iterator itr, next;
        for (itr = m_sessions.begin(); itr != m_sessions.end(); itr = next)
        {
            next = itr;
            next++;

            if(!itr->second->Update(diff))
            {
                delete itr->second;
                m_sessions.erase(itr);
            }
        }
    }

    if (m_timers[WUPDATE_OBJECTS].Passed())
    {
			m_timers[WUPDATE_OBJECTS].Reset();
			MapManager::Instance().Update(diff);
    }
}


void World::SendGlobalMessage(WorldPacket *packet, WorldSession *self)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld()
            && itr->second != self)               
        {
            itr->second->SendPacket(packet);
        }
    }
}


void World::SendWorldText(const char* text, WorldSession *self)
{
    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, 0, text);
    SendGlobalMessage(&data, self);
}

