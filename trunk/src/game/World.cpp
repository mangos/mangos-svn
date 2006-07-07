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
#include "Weather.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "UpdateData.h"
#include "Chat.h"
#include "Database/DBCStores.h"
#include "ChannelMgr.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "ScriptCalls.h"
#include "CreatureAIRegistry.h"                             // need for Game::Initialize()
#include "Policies/SingletonImp.h"
#include "EventSystem.h"
#include "GlobalEvents.h"
#include "BattleGroundMgr.h"
#include "SystemConfig.h"
#include "AddonHandler.h"
#include "zlib/zlib.h"

INSTANTIATE_SINGLETON_1( World );

World::World()
{
    m_playerLimit = 0;
    m_allowMovement = true;
}

World::~World()
{
}

Player* World::FindPlayerInZone(uint32 zone)
{
    SessionMap::iterator itr, next;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr = next)
    {
        next = itr;
        next++;

        if(!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if(!player)
            continue;
        if( player->IsInWorld() && player->GetZoneId() == zone )
        {
            return player;
        }
    }
    return NULL;
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

Weather* World::FindWeather(uint32 id) const
{
    WeatherMap::const_iterator itr = m_weathers.find(id);

    if(itr != m_weathers.end())
        return itr->second;
    else
        return 0;
}

void World::RemoveWeather(uint32 id)
{
    WeatherMap::iterator itr = m_weathers.find(id);

    if(itr != m_weathers.end())
    {
        delete itr->second;
        m_weathers.erase(itr);
    }
}

void World::AddWeather(Weather* w)
{
    ASSERT(w);
    m_weathers[w->GetZone()] = w;
}

void World::SetInitialWorldSettings()
{
    std::string dataPath="./";

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
        if((dataPath.at(dataPath.length()-1)!='/') && (dataPath.at(dataPath.length()-1)!='\\'))
            dataPath.append("/");
    }
    sLog.outString("Using DataDir %s ...",dataPath.c_str());

    // Non-critical warning about conf file version
    uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
    if(!confVersion)
    {
        sLog.outString("*****************************************************************************");
        sLog.outString(" WARNING: mangosd.conf does not include a ConfVersion variable.");
        sLog.outString("          Your conf file may be out of date!");
        sLog.outString("*****************************************************************************");
        clock_t pause = 3000 + clock();
        while (pause > clock());
    }
    else
    {
        if (confVersion < _MANGOSDCONFVERSION)
        {
            sLog.outString("*****************************************************************************");
            sLog.outString(" WARNING: Your mangosd.conf version indicates your conf file is out of date!");
            sLog.outString("          Please check for updates, as your current default values may cause");
            sLog.outString("          strange behavior.");
            sLog.outString("*****************************************************************************");
            clock_t pause = 3000 + clock();
            while (pause > clock());
        }
    }

    regen_values[RATE_HEALTH] = sConfig.GetFloatDefault("Rate.Health", 1);
    regen_values[RATE_POWER1] = sConfig.GetFloatDefault("Rate.Power1", 1);
    regen_values[RATE_POWER2] = sConfig.GetFloatDefault("Rate.Power2", 1);
    regen_values[RATE_POWER3] = sConfig.GetFloatDefault("Rate.Power3", 1);
    regen_values[RATE_DROP] = sConfig.GetFloatDefault("Rate.Drop", 1);
    regen_values[RATE_XP] = sConfig.GetFloatDefault("Rate.XP", 1);
    m_configs[CONFIG_LOG_LEVEL] = sConfig.GetIntDefault("LogLevel", 0);
    m_configs[CONFIG_LOG_WORLD] = sConfig.GetIntDefault("LogWorld", 0);
    m_configs[CONFIG_INTERVAL_SAVE] = sConfig.GetIntDefault("PlayerSaveInterval", 900000);
    m_configs[CONFIG_INTERVAL_GRIDCLEAN] = sConfig.GetIntDefault("GridCleanUpDelay", 300000);
    m_configs[CONFIG_INTERVAL_MAPUPDATE] = sConfig.GetIntDefault("MapUpdateInterval", 100);
    m_configs[CONFIG_INTERVAL_CHANGEWEATHER] = sConfig.GetIntDefault("ChangeWeatherInterval", 600000);
    m_configs[CONFIG_PORT_WORLD] = sConfig.GetIntDefault("WorldServerPort", 8085);
    m_configs[CONFIG_PORT_REALM] = sConfig.GetIntDefault("RealmServerPort", 3724);
    m_configs[CONFIG_SOCKET_SELECTTIME] = sConfig.GetIntDefault("SocketSelectTime", 10000);
    m_configs[CONFIG_GETXP_DISTANCE] = sConfig.GetIntDefault("MaxDistance", 5500);
    m_configs[CONFIG_GETXP_LEVELDIFF] = sConfig.GetIntDefault("MaxLevelDiff", 10);
    m_configs[CONFIG_SIGHT_MONSTER] = sConfig.GetIntDefault("MonsterSight", 400);
    m_configs[CONFIG_SIGHT_GUARDER] = sConfig.GetIntDefault("GuarderSight", 500);
    m_configs[CONFIG_GAME_TYPE] = sConfig.GetIntDefault("GameType", 0);
    m_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfig.GetIntDefault("MaxPlayerLevel", 60);

    m_gameTime = (3600*atoi(hour))+(atoi(minute)*60)+(atoi(second));

    //duplicate
    //sDatabase.PExecute("UPDATE `character` SET `online` = 0;");

    //Update realm list
    loginDatabase.PExecute("UPDATE `realmlist` SET `icon` = %u WHERE `id` = %d;", m_configs[CONFIG_GAME_TYPE],sConfig.GetIntDefault("RealmID", 0));

    // remove bones after restart
    sDatabase.PExecute("DELETE FROM `game_corpse` WHERE `bones_flag` = '1';");

    new ChannelMgr;

    sLog.outString("Initialize data stores...");
    LoadDBCStores(dataPath);


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

    sLog.outString( "Loading Loot Tables..." );
    LoadLootTables();

    sLog.outString( "Loading Game Object Templates..." );
    objmgr.LoadGameobjectInfo();

    sLog.outString( "Initializing Scripts..." );
    if(!LoadScriptingModule())
        exit(1);

    m_timers[WUPDATE_OBJECTS].SetInterval(0);
    m_timers[WUPDATE_SESSIONS].SetInterval(0);
    m_timers[WUPDATE_WEATHERS].SetInterval(1000);
    m_timers[WUPDATE_AUCTIONS].SetInterval(1000);

    sLog.outString( "WORLD: Starting BattleGround System" );
    sBattleGroundMgr.CreateInitialBattleGrounds();

    MaNGOS::Game::Initialize();
    sLog.outString( "WORLD: SetInitialWorldSettings done" );

    sLog.outString( "WORLD: Starting Event System" );
    StartEventSystem();

    //Start Addon stuff
    bool temp = sConfig.GetBoolDefault("AddonDefault", 1);
    sLog.outString( "WORLD: Starting Addon System, AddonDefault:%d (%s all addons not registared in DB)", temp, temp? "Enabled" : "Disabled"  );
    sAddOnHandler.SetAddonDefault(temp);
    sAddOnHandler._LoadFromDB();

    sLog.outString( "WORLD: Starting Corpse Handler" );
    // global event to erase corpses/bones
    // deleting expired bones time > 20 minutes and corpses > 3 days
    // it is run each 20 minutes
    // need good tests on windows
    
    //uint32 m_CorpsesEventID = 
    AddEvent(&HandleCorpsesErase,NULL,1200000,false,true);
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
                    m->receiver = itr->second->owner;
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
                    ss << "INSERT INTO `mail_item` (`guid`,`data`) VALUES ("
                        << it->GetGUIDLow() << ", '";
                    for(uint16 i = 0; i < it->GetValuesCount(); i++ )
                    {
                        ss << it->GetUInt32Value(i) << " ";
                    }
                    ss << "' )";
                    sDatabase.Execute( ss.str().c_str() );

                    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",m->messageID);

                    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->receiver, m->subject.c_str(), m->body.c_str(), m->item, m->time, m->money, 0,  m->checked);

                    uint64 rcpl = m->receiver;
                    std::string pname;
                    objmgr.GetPlayerNameByGUID(rcpl,pname);
                    Player *rpl = objmgr.GetPlayer(pname.c_str());
                    if (rpl)
                    {
                        rpl->AddMail(m);
                    }
                    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `itemowner` = '%u'",m->receiver);
                    sDatabase.PExecute("DELETE FROM `auctionhouse_item` WHERE `guid` = '%u'",m->item);
                    sDatabase.PExecute("DELETE FROM `auctionhouse_bid` WHERE `id` = '%u'",itr->second->Id);

                    objmgr.RemoveAuction(itr->second->Id);
                }
                else
                {
                    Mail *m = new Mail;
                    m->receiver = itr->second->owner;
                    m->body = "";
                    m->sender = itr->second->bidder;
                    m->checked = 0;
                    m->COD = 0;
                    m->messageID = objmgr.GenerateMailID();
                    m->money = itr->second->bid;
                    m->time = time(NULL) + (29 * 3600);
                    m->subject = "Your item sold!";
                    m->item = 0;

                    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",m->messageID);

                    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", m->messageID, m->sender, m->receiver, m->subject.c_str(), m->body.c_str(), m->item, m->time, m->money, 0, m->checked);

                    uint64 rcpl = m->receiver;
                    std::string pname;
                    objmgr.GetPlayerNameByGUID(rcpl,pname);
                    Player *rpl = objmgr.GetPlayer(pname.c_str());
                    if (rpl)
                    {
                        rpl->AddMail(m);
                    }

                    Mail *mn = new Mail;
                    mn->receiver = itr->second->bidder;
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
                    ss << "INSERT INTO `mail_item` (`guid`,`data`) VALUES ("
                        << it->GetGUIDLow() << ", '";
                    for(uint16 i = 0; i < it->GetValuesCount(); i++ )
                    {
                        ss << it->GetUInt32Value(i) << " ";
                    }
                    ss << "' )";
                    sDatabase.Execute( ss.str().c_str() );

                    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", mn->messageID);

                    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", mn->messageID, mn->sender, mn->receiver, mn->subject.c_str(), mn->body.c_str(), mn->item, mn->time, mn->money, 0, mn->checked);

                    uint64 rcpl1 = mn->receiver;
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

    if (m_timers[WUPDATE_WEATHERS].Passed())
    {
        m_timers[WUPDATE_WEATHERS].Reset();

        WeatherMap::iterator itr, next;
        for (itr = m_weathers.begin(); itr != m_weathers.end(); itr = next)
        {
            next = itr;
            next++;

            if(!itr->second->Update(diff))
            {
                delete itr->second;
                m_weathers.erase(itr);
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

void World::SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        Player *player = itr->second->GetPlayer();
        if ( player && player->IsInWorld() && player->GetZoneId() == zone && itr->second != self)
        {
            itr->second->SendPacket(packet);
        }
    }
}

void World::SendZoneText(uint32 zone, const char* text, WorldSession *self)
{
    WorldPacket data;
    sChatHandler.FillSystemMessageData(&data, 0, text);
    SendZoneMessage(zone, &data, self);
}

void World::KickPlayer(char* playerName)
{
    SessionMap::iterator itr, next;
    WorldSession *playerToKick = 0;

    int y = 0;
    while (!playerName[y] == 0)
    {
        if ((playerName[y] >= 'a') && (playerName[y] <= 'z'))
            playerName[y] -= 'a' - 'A';
        y++;
    }

    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr = next)
    {
        next = itr;
        next++;
        if(!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if(!player)
            continue;
        if( player->IsInWorld() )
        {
            char *tmpPlayerName = new char[strlen(player->GetName()) + 1];
            strcpy(tmpPlayerName, player->GetName());
            y = 0;
            while (!tmpPlayerName[y] == 0)
            {
                if ((tmpPlayerName[y] >= 'a') && (tmpPlayerName[y] <= 'z'))
                    tmpPlayerName[y] -= 'a' - 'A';
                y++;
            }
            if (strcmp(playerName, tmpPlayerName) == 0)
                playerToKick = itr->second;
            delete tmpPlayerName;
        }
    }
    if (playerToKick)
    {
        playerToKick->LogoutPlayer(true);
    }
}
