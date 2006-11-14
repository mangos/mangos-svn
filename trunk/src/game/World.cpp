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

#include <signal.h>

INSTANTIATE_SINGLETON_1( World );

volatile bool World::m_stopEvent = false;

World::World()
{
    m_playerLimit = 0;
    m_allowMovement = true;
    m_Last_tick = time(NULL);
    m_ShutdownTimer = 0;
}

World::~World()
{
    for (WeatherMap::iterator itr = m_weathers.begin(); itr != m_weathers.end(); ++itr)
        delete itr->second;
    m_weathers.clear();
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
        sLog.outError("*****************************************************************************");
        sLog.outError(" WARNING: mangosd.conf does not include a ConfVersion variable.");
        sLog.outError("          Your conf file may be out of date!");
        sLog.outError("*****************************************************************************");
        clock_t pause = 3000 + clock();
        while (pause > clock());
    }
    else
    {
        if (confVersion < _MANGOSDCONFVERSION)
        {
            sLog.outError("*****************************************************************************");
            sLog.outError(" WARNING: Your mangosd.conf version indicates your conf file is out of date!");
            sLog.outError("          Please check for updates, as your current default values may cause");
            sLog.outError("          strange behavior.");
            sLog.outError("*****************************************************************************");
            clock_t pause = 3000 + clock();
            while (pause > clock());
        }
    }

    regen_values[RATE_HEALTH]      = sConfig.GetFloatDefault("Rate.Health", 1);
    regen_values[RATE_POWER_MANA]  = sConfig.GetFloatDefault("Rate.Power1", 1);
    regen_values[RATE_POWER_RAGE]  = sConfig.GetFloatDefault("Rate.Power2", 1);
    regen_values[RATE_POWER_FOCUS] = sConfig.GetFloatDefault("Rate.Power3", 1);
    regen_values[RATE_DROP_ITEMS]  = sConfig.GetFloatDefault("Rate.Drop.Items", 1);
    regen_values[RATE_DROP_MONEY]  = sConfig.GetFloatDefault("Rate.Drop.Money", 1);
    regen_values[RATE_XP_KILL]     = sConfig.GetFloatDefault("Rate.XP.Kill", 1);
    regen_values[RATE_XP_QUEST]    = sConfig.GetFloatDefault("Rate.XP.Quest", 1);
    regen_values[RATE_XP_EXPLORE]  = sConfig.GetFloatDefault("Rate.XP.Explore", 1);
    regen_values[RATE_CREATURE_NORMAL_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Normal.Damage", 1);
    regen_values[RATE_CREATURE_ELITE_ELITE_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.Elite.Damage", 1);
    regen_values[RATE_CREATURE_ELITE_RAREELITE_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.RAREELITE.Damage", 1);
    regen_values[RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.Damage", 1);
    regen_values[RATE_CREATURE_ELITE_RARE_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.RARE.Damage", 1);
    regen_values[RATE_CREATURE_NORMAL_HP] = sConfig.GetFloatDefault("Rate.Creature.Normal.HP", 1);
    regen_values[RATE_CREATURE_ELITE_ELITE_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.Elite.HP", 1);
    regen_values[RATE_CREATURE_ELITE_RAREELITE_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.RAREELITE.HP", 1);
    regen_values[RATE_CREATURE_ELITE_WORLDBOSS_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.HP", 1);
    regen_values[RATE_CREATURE_ELITE_RARE_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.RARE.HP", 1);
    regen_values[RATE_CREATURE_AGGRO]  = sConfig.GetFloatDefault("Rate.Creature.Aggro", 1);
    m_configs[CONFIG_LOG_LEVEL] = sConfig.GetIntDefault("LogLevel", 0);
    m_configs[CONFIG_LOG_WORLD] = sConfig.GetIntDefault("LogWorld", 0);
    m_configs[CONFIG_COMPRESSION] = sConfig.GetIntDefault("Compression", 1);
    if(m_configs[CONFIG_COMPRESSION] < 1 || m_configs[CONFIG_COMPRESSION] > 9)
    {
        sLog.outError("Compression level (%i) must be in range 1..9. Using default compression level (1).",m_configs[CONFIG_COMPRESSION]);
        m_configs[CONFIG_COMPRESSION] = 1;
    }
    m_configs[CONFIG_GRID_UNLOAD] = sConfig.GetIntDefault("GridUnload", 1);
    m_configs[CONFIG_INTERVAL_SAVE] = sConfig.GetIntDefault("PlayerSaveInterval", 900000);
    m_configs[CONFIG_INTERVAL_GRIDCLEAN] = sConfig.GetIntDefault("GridCleanUpDelay", 300000);
    m_configs[CONFIG_INTERVAL_MAPUPDATE] = sConfig.GetIntDefault("MapUpdateInterval", 100);
    m_configs[CONFIG_INTERVAL_CHANGEWEATHER] = sConfig.GetIntDefault("ChangeWeatherInterval", 600000);
    m_configs[CONFIG_PORT_WORLD] = sConfig.GetIntDefault("WorldServerPort", 8085);
    m_configs[CONFIG_PORT_REALM] = sConfig.GetIntDefault("RealmServerPort", 3724);
    m_configs[CONFIG_SOCKET_SELECTTIME] = sConfig.GetIntDefault("SocketSelectTime", 10000);
    m_configs[CONFIG_GROUP_XP_DISTANCE] = sConfig.GetIntDefault("MaxGroupXPDistance", 74);
    m_configs[CONFIG_GROUP_XP_DISTANCE] = m_configs[CONFIG_GROUP_XP_DISTANCE]*m_configs[CONFIG_GROUP_XP_DISTANCE];
    m_configs[CONFIG_GROUP_XP_LEVELDIFF] = sConfig.GetIntDefault("MaxGroupXPLevelDiff", 10);
    m_configs[CONFIG_SIGHT_MONSTER] = sConfig.GetIntDefault("MonsterSight", 400);
    m_configs[CONFIG_SIGHT_GUARDER] = sConfig.GetIntDefault("GuarderSight", 500);
    m_configs[CONFIG_GAME_TYPE] = sConfig.GetIntDefault("GameType", 0);
    m_configs[CONFIG_ALLOW_TWO_SIDE_ACCOUNTS] = sConfig.GetIntDefault("AllowTwoSide.Accounts", 0);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION] = sConfig.GetIntDefault("AllowTwoSide.Interaction",0);
    m_configs[CONFIG_ALLOW_TWO_SIDE_WHO_LIST] = sConfig.GetIntDefault("AllowTwoSide.WhoList", 0);
    m_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfig.GetIntDefault("MaxPlayerLevel", 60);
    if(m_configs[CONFIG_MAX_PLAYER_LEVEL] > 255)
    {
        sLog.outError("MaxPlayerLevel (%i) must be in range 1..255. Set to 255.",m_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_configs[CONFIG_MAX_PLAYER_LEVEL] = 255;
    }

    m_configs[CONFIG_MAX_PRIMARY_TRADE_SKILL] = sConfig.GetIntDefault("MaxPrimaryTradeSkill", 2);
    m_configs[CONFIG_MIN_PETITION_SIGNS] = sConfig.GetIntDefault("MinPetitionSigns", 9);
    if(m_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        sLog.outError("MinPetitionSigns (%i) must be in range 0..9. Set to 9.",m_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    m_configs[CONFIG_GM_WISPERING_TO] = sConfig.GetIntDefault("GM.WhisperingTo",0);
    m_configs[CONFIG_GM_IN_WHO_LIST]  = sConfig.GetIntDefault("GM.InWhoList",0);

    m_gameTime = time(NULL);

    // check existance map files (startup race areas).
    if(   !MapManager::ExistMAP(0,-6240.32, 331.033)
        ||!MapManager::ExistMAP(0,-8949.95,-132.493)
        ||!MapManager::ExistMAP(0,-8949.95,-132.493)
        ||!MapManager::ExistMAP(1,-618.518,-4251.67)
        ||!MapManager::ExistMAP(0, 1676.35, 1677.45)
        ||!MapManager::ExistMAP(1, 10311.3, 832.463)
        ||!MapManager::ExistMAP(1,-2917.58,-257.98))
    {
        sLog.outError("Correct *.map files not found by path '%smaps'. Please place *.map files in directory by this path or correct DataDir value in mangosd.conf file.",dataPath.c_str());
        exit(1);
    }

    //Update realm list
    loginDatabase.PExecute("UPDATE `realmlist` SET `icon` = %u WHERE `id` = '%d'", m_configs[CONFIG_GAME_TYPE],sConfig.GetIntDefault("RealmID", 1));

    // remove bones after restart
    sDatabase.PExecute("DELETE FROM `corpse` WHERE `bones_flag` = '1'");

    new ChannelMgr;

    sLog.outString("Initialize data stores...");
    LoadDBCStores(dataPath);

    sLog.outString( "Loading Game Object Templates..." );
    objmgr.LoadGameobjectInfo();

    sLog.outString( "Loading levelup stat gains..." );
    objmgr.LoadLvlUpGains();

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

    sLog.outString( "Loading Creature templates..." );
    objmgr.LoadCreatureTemplates();

    sLog.outString( "Loading Guilds..." );
    objmgr.LoadGuilds();

    sLog.outString( "Loading Teleport Coords..." );
    objmgr.LoadTeleportCoords();

    objmgr.SetHighestGuids();

    sLog.outString( "Loading Loot Tables..." );
    LoadLootTables();

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

    sLog.outString( "Loading Transports..." );
    MapManager::Instance().LoadTransports();

    sLog.outString( "WORLD: Starting Event System" );
    StartEventSystem();

    //Start Addon stuff
    bool temp = sConfig.GetBoolDefault("AddonDefault", 1);
    sLog.outString( "WORLD: Starting Addon System, AddonDefault:%d (%s all addons not registered in DB)", temp, temp? "Enabled" : "Disabled"  );
    sAddOnHandler.SetAddonDefault(temp);
    sAddOnHandler._LoadFromDB();

    sLog.outString( "WORLD: Starting Corpse Handler" );
    // global event to erase corpses/bones
    // deleting expired bones time > 20 minutes and corpses > 3 days
    // it is run each 20 minutes
    // need good tests on windows
    AddEvent(&HandleCorpsesErase,NULL,20*MINUTE*1000,false,true);
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
                // Auction Expired!
                if (itr->second->bidder == 0)               // if noone bidded auction...
                {
                    Item *it = objmgr.GetAItem(itr->second->item_guidlow);
                    if(it)
                    {
                        uint64 plGuid = itr->second->owner;
                        Player *receive = objmgr.GetPlayer(plGuid);

                        Mail *m = new Mail;
                        m->messageID = objmgr.GenerateMailID();
                        m->sender = 0;                      //there should be horde/ali AuctionHouse
                        m->receiver = itr->second->owner;

                        std::ostringstream msgAuctionExpiredSubject;
                        msgAuctionExpiredSubject << "Auction Expired: ";
                        Item *theitem = objmgr.GetAItem(itr->second->item_guidlow);
                        if (theitem)
                        {
                            ItemPrototype const *theitemproto = theitem->GetProto();
                            msgAuctionExpiredSubject << theitemproto->Name1;
                        }
                        else
                            msgAuctionExpiredSubject << "Unknown";

                        m->subject = msgAuctionExpiredSubject.str().c_str();
                        m->body = "";
                        m->item_guidlow = itr->second->item_guidlow;
                        m->item_id = itr->second->item_id;
                        m->time = time(NULL) + (29 * DAY);
                        m->money = 0;
                        m->COD = 0;                         // there might be deposit
                        m->checked = 0;
                        if (receive)
                        {
                            receive->AddMail(m);
                            receive->AddMItem(it);
                        }

                        //escape apostrophes
                        std::string subject = m->subject;
                        std::string body = m->body;
                        sDatabase.escape_string(body);
                        sDatabase.escape_string(subject);

                        sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",m->messageID);
                        sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`item_template`,`time`,`money`,`cod`,`checked`) "
                            "VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u')",
                            m->messageID, m->sender, m->receiver, subject.c_str(), body.c_str(), m->item_guidlow, m->item_id, (uint64)m->time, 0, 0, 0);

                        delete m;
                    }
                    else
                        sLog.outError("Auction item (GUID: %u) not found, and lost.",itr->second->item_guidlow);

                    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",itr->second->Id);

                    objmgr.RemoveAItem(itr->second->item_guidlow);
                    objmgr.RemoveAuction(itr->second->Id);
                }
                else
                {
                    Mail *m = new Mail;
                    m->messageID = objmgr.GenerateMailID();
                    m->sender = itr->second->bidder;
                    m->receiver = itr->second->owner;

                    std::ostringstream msgAuctionSuccessfullSubject;
                    std::ostringstream msgAuctionSuccessfullBody;
                    msgAuctionSuccessfullSubject << "Auction successfull: ";
                    msgAuctionSuccessfullBody << "Item Sold: ";

                    Item *theitem = objmgr.GetAItem(itr->second->item_guidlow);
                    if (theitem)
                    {
                        ItemPrototype const *theitemproto = theitem->GetProto();
                        msgAuctionSuccessfullSubject << theitemproto->Name1;
                        msgAuctionSuccessfullBody << theitemproto->Name1;
                    }
                    else
                    {
                        msgAuctionSuccessfullSubject << "Unknown";
                        msgAuctionSuccessfullBody << "Unknown";
                    }

                    // If it was a buyout, show it so
                    if (itr->second->bid == itr->second->buyout)
                        msgAuctionSuccessfullSubject << " (buyout)";

                    msgAuctionSuccessfullBody << "$B" << "Sold By: ";
                    Player *auctionWinner = objmgr.GetPlayer(itr->second->bidder);
                    if (auctionWinner)
                    {
                        // the auctionOwner is currently online, so lets get the name
                        msgAuctionSuccessfullBody << auctionWinner->GetName() << "$B";
                    }
                    else
                    {
                        // the auctionOwner is currently offline, so lets get the name from the database
                        std::string ownerName;
                        if(objmgr.GetPlayerNameByGUID(itr->second->bidder,ownerName))
                            msgAuctionSuccessfullBody << ownerName << "$B";
                        else
                            msgAuctionSuccessfullBody << "Unknown$B";
                    }
                    m->subject = msgAuctionSuccessfullSubject.str().c_str();
                    m->body = msgAuctionSuccessfullBody.str().c_str();
                    m->item_guidlow = 0;
                    m->item_id = 0;
                    m->time = time(NULL) + (29 * DAY);
                    m->money = itr->second->bid;
                    m->COD = 0;
                    m->checked = 0;

                    {
                        //escape apostrophes
                        std::string subject = m->subject;
                        std::string body = m->body;
                        sDatabase.escape_string(body);
                        sDatabase.escape_string(subject);

                        sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",m->messageID);
                        sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`item_template`,`time`,`money`,`cod`,`checked`) "
                            "VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u')",
                            m->messageID, m->sender, m->receiver, subject.c_str(), body.c_str(), m->item_guidlow, m->item_id, (uint64)m->time, m->money, 0, 0);
                    }

                    Player *rpl = objmgr.GetPlayer(MAKE_GUID(m->receiver,HIGHGUID_PLAYER));
                    if (rpl)
                    {
                        rpl->AddMail(m);
                    }
                    else
                        delete m;

                    Mail *mn = new Mail;
                    mn->messageID = objmgr.GenerateMailID();
                    mn->sender = itr->second->owner;
                    mn->receiver = itr->second->bidder;
                    mn->subject = "Your won an item!";
                    mn->body = "";
                    mn->item_guidlow = itr->second->item_guidlow;
                    mn->item_id = itr->second->item_id;
                    mn->time = time(NULL) + (29 * 3600);
                    mn->money = 0;
                    mn->COD = 0;
                    mn->checked = 0;

                    Item *it = objmgr.GetAItem(itr->second->item_guidlow);

                    {
                        //escape apostrophes
                        std::string subject = mn->subject;
                        std::string body = mn->body;
                        sDatabase.escape_string(body);
                        sDatabase.escape_string(subject);

                        sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", mn->messageID);
                        sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`, `item_template`, `time`,`money`,`cod`,`checked`) "
                            "VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u')",
                            mn->messageID, mn->sender, mn->receiver, subject.c_str(), body.c_str(), mn->item_guidlow, mn->item_id, (uint64)mn->time, 0, 0, 0);
                    }

                    Player *rpl1 = objmgr.GetPlayer(MAKE_GUID(mn->receiver,HIGHGUID_PLAYER));
                    if (rpl1)
                    {
                        rpl1->AddMItem(it);
                        rpl1->AddMail(mn);
                    }
                    else
                        delete mn;

                    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",itr->second->Id);

                    objmgr.RemoveAItem(itr->second->item_guidlow);
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

    // move all creatures with delayed move and remove and delete all objects with delayed remove
    ObjectAccessor::Instance().DoDelayedMovesAndRemoves();
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
            delete[] tmpPlayerName;
        }
    }
    if (playerToKick)
    {
        playerToKick->LogoutPlayer(true);
    }
}

time_t World::_UpdateGameTime()
{
    time_t thisTime = time(NULL);

    uint32 elapsed = uint32(thisTime - m_Last_tick);

    if(m_ShutdownTimer > 0 && elapsed > 0)
    {
        if( m_ShutdownTimer <= elapsed )
        {
            m_stopEvent = true;
        }
        else
        {

            m_ShutdownTimer -= elapsed;

            ShutdownMsg();
        }
    }

    m_gameTime = thisTime;
    m_Last_tick = thisTime;

    return m_gameTime;
}

void World::ShutdownServ(uint32 time)
{
    if(time==0)
        m_stopEvent = true;
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true);
    }
}

void World::ShutdownMsg(bool show, Player* player)
{
    if ( show ||
        (m_ShutdownTimer < 10) ||
                                                            // < 30 sec; every 5 sec
        (m_ShutdownTimer<30        && (m_ShutdownTimer % 5         )==0) ||
                                                            // < 5 min ; every 1 min
        (m_ShutdownTimer<5*MINUTE  && (m_ShutdownTimer % MINUTE    )==0) ||
                                                            // < 30 min ; every 5 min
        (m_ShutdownTimer<30*MINUTE && (m_ShutdownTimer % (5*MINUTE))==0) ||
                                                            // < 12 h ; every 1 h
        (m_ShutdownTimer<12*HOUR   && (m_ShutdownTimer % HOUR      )==0) ||
                                                            // > 12 h ; every 12 h
        (m_ShutdownTimer>12*HOUR   && (m_ShutdownTimer % (12*HOUR) )==0))
    {
        uint32 secs    = m_ShutdownTimer % MINUTE;
        uint32 minutes = m_ShutdownTimer % HOUR / MINUTE;
        uint32 hours   = m_ShutdownTimer % DAY  / HOUR;
        uint32 days    = m_ShutdownTimer / DAY;

        std::ostringstream ss;
        if(days)
            ss << days << " Day(s) ";
        if(hours)
            ss << hours << " Hour(s) ";
        if(minutes)
            ss << minutes << " Minute(s) ";
        if(secs)
            ss << secs << " Second(s).";

        SendServerMessage(SERVER_MSG_SHUTDOWN_TIME,ss.str().c_str(),player);
        DEBUG_LOG("Server is shuttingdown in %s",ss.str().c_str());
    }
}

void World::ShutdownCancel()
{
    if(!m_ShutdownTimer)
        return;

    m_ShutdownTimer = 0;
    SendServerMessage(SERVER_MSG_SHUTDOWN_CANCELLED);

    DEBUG_LOG("Server shuttingdown cancelled.");
}

void World::SendServerMessage(ServerMessageType type, const char *text, Player* player)
{
    WorldPacket data;

    data.Initialize(SMSG_SERVER_MESSAGE);
    data << uint32(type);
    if(type <= SERVER_MSG_STRING)
        data << text;

    if(player)
        player->GetSession()->SendPacket(&data);
    else
        SendGlobalMessage( &data );
}
