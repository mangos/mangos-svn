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
#include "TemporarySummon.h"
#include "TargetedMovementGenerator.h"
#include "zlib/zlib.h"

#include <signal.h>

INSTANTIATE_SINGLETON_1( World );

volatile bool World::m_stopEvent = false;

World::World()
{
    m_playerLimit = 0;
    m_allowMovement = true;
    m_Last_tick = time(NULL);
    m_ShutdownIdleMode = false;
    m_ShutdownTimer = 0;
    internalGameTime = 0;
    m_logFilter = 0;
}

World::~World()
{
    for (WeatherMap::iterator itr = m_weathers.begin(); itr != m_weathers.end(); ++itr)
        delete itr->second;

    m_weathers.clear();
}

Player* World::FindPlayerInZone(uint32 zone)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
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
        return itr->second;                                 // also can return NULL for kicked session
    else
        return NULL;
}

bool World::RemoveSession(uint32 id)
{
    SessionMap::iterator itr = m_sessions.find(id);

    if(itr != m_sessions.end() && itr->second)
    {
        if (itr->second->PlayerLoading())
            return false;
        itr->second->KickPlayer();

        // session can't be erased or delected currently (to prevent iterator invalidation and socket problems)
        m_kicked_sessions.insert(itr->second);
        itr->second = NULL;
        return true;
    }

    return true;
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

    SetPlayerLimit( sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
    SetMotd( sConfig.GetStringDefault("Motd", "Welcome to the Massive Network Game Object Server." ).c_str() );

    rate_values[RATE_HEALTH]      = sConfig.GetFloatDefault("Rate.Health", 1);
    rate_values[RATE_POWER_MANA]  = sConfig.GetFloatDefault("Rate.Power1", 1);
    rate_values[RATE_POWER_RAGE]  = sConfig.GetFloatDefault("Rate.Power2", 1);
    rate_values[RATE_POWER_FOCUS] = sConfig.GetFloatDefault("Rate.Power3", 1);
    rate_values[RATE_DROP_ITEMS]  = sConfig.GetFloatDefault("Rate.Drop.Items", 1);
    rate_values[RATE_DROP_MONEY]  = sConfig.GetFloatDefault("Rate.Drop.Money", 1);
    rate_values[RATE_XP_KILL]     = sConfig.GetFloatDefault("Rate.XP.Kill", 1);
    rate_values[RATE_XP_QUEST]    = sConfig.GetFloatDefault("Rate.XP.Quest", 1);
    rate_values[RATE_XP_EXPLORE]  = sConfig.GetFloatDefault("Rate.XP.Explore", 1);
    rate_values[RATE_CREATURE_NORMAL_DAMAGE]          = sConfig.GetFloatDefault("Rate.Creature.Normal.Damage", 1);
    rate_values[RATE_CREATURE_ELITE_ELITE_DAMAGE]     = sConfig.GetFloatDefault("Rate.Creature.Elite.Elite.Damage", 1);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.RAREELITE.Damage", 1);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.Damage", 1);
    rate_values[RATE_CREATURE_ELITE_RARE_DAMAGE]      = sConfig.GetFloatDefault("Rate.Creature.Elite.RARE.Damage", 1);
    rate_values[RATE_CREATURE_NORMAL_HP]          = sConfig.GetFloatDefault("Rate.Creature.Normal.HP", 1);
    rate_values[RATE_CREATURE_ELITE_ELITE_HP]     = sConfig.GetFloatDefault("Rate.Creature.Elite.Elite.HP", 1);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.RAREELITE.HP", 1);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_HP] = sConfig.GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.HP", 1);
    rate_values[RATE_CREATURE_ELITE_RARE_HP]      = sConfig.GetFloatDefault("Rate.Creature.Elite.RARE.HP", 1);
    rate_values[RATE_CREATURE_AGGRO]  = sConfig.GetFloatDefault("Rate.Creature.Aggro", 1);
    rate_values[RATE_REST_INGAME]                    = sConfig.GetFloatDefault("Rate.Rest.InGame", 1);
    rate_values[RATE_REST_OFFLINE_IN_TAVERN_OR_CITY] = sConfig.GetFloatDefault("Rate.Rest.Offline.InTavernOrCity", 1);
    rate_values[RATE_REST_OFFLINE_IN_WILDERNESS]     = sConfig.GetFloatDefault("Rate.Rest.Offline.InWilderness", 1);

    m_configs[CONFIG_LOG_LEVEL] = sConfig.GetIntDefault("LogLevel", 0);
    m_configs[CONFIG_LOG_WORLD] = sConfig.GetIntDefault("LogWorld", 0);

    if(sConfig.GetIntDefault("LogFilter_TransportMoves", 0)!=0)
        m_logFilter |= LOG_FILTER_TRANSPORT_MOVES;
    if(sConfig.GetIntDefault("LogFilter_CreatureMoves", 0)!=0)
        m_logFilter |= LOG_FILTER_CREATURE_MOVES;

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
    m_configs[CONFIG_PORT_WORLD] = sConfig.GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);
    m_configs[CONFIG_PORT_REALM] = sConfig.GetIntDefault("RealmServerPort", DEFAULT_REALMSERVER_PORT);
    m_configs[CONFIG_SOCKET_SELECTTIME] = sConfig.GetIntDefault("SocketSelectTime", DEFAULT_SOCKET_SELECT_TIME);
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
    m_configs[CONFIG_IGNORE_AT_LEVEL_REQUIREMENT] = sConfig.GetBoolDefault("IgnoreATLevelRequirement", 0);

    m_configs[CONFIG_MAX_PRIMARY_TRADE_SKILL] = sConfig.GetIntDefault("MaxPrimaryTradeSkill", 2);
    m_configs[CONFIG_MIN_PETITION_SIGNS] = sConfig.GetIntDefault("MinPetitionSigns", 9);
    if(m_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        sLog.outError("MinPetitionSigns (%i) must be in range 0..9. Set to 9.",m_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    m_configs[CONFIG_GM_WISPERING_TO] = sConfig.GetIntDefault("GM.WhisperingTo",0);
    m_configs[CONFIG_GM_IN_GM_LIST]  = sConfig.GetIntDefault("GM.InGMList",0);
    m_configs[CONFIG_GM_IN_WHO_LIST]  = sConfig.GetIntDefault("GM.InWhoList",0);
    m_configs[CONFIG_GM_LOGIN_STATE]  = sConfig.GetIntDefault("GM.LoginState",2);
    m_configs[CONFIG_GM_LOG_TRADE] = sConfig.GetIntDefault("GM.LogTrade", 1);

    m_configs[CONFIG_GROUP_VISIBILITY] = sConfig.GetIntDefault("GroupVisibility",0);

    m_configs[CONFIG_SKILL_CHANCE_ORANGE] = sConfig.GetIntDefault("SkillChance.Orange",100);
    m_configs[CONFIG_SKILL_CHANCE_YELLOW] = sConfig.GetIntDefault("SkillChance.Yellow",75);
    m_configs[CONFIG_SKILL_CHANCE_GREEN]  = sConfig.GetIntDefault("SkillChance.Green",25);
    m_configs[CONFIG_SKILL_CHANCE_GREY]   = sConfig.GetIntDefault("SkillChance.Grey",0);

    m_configs[CONFIG_MAX_OVERSPEED_PINGS] = sConfig.GetIntDefault("MaxOverspeedPings",10);
    if(m_configs[CONFIG_MAX_OVERSPEED_PINGS] != 0 && m_configs[CONFIG_MAX_OVERSPEED_PINGS] < 2)
    {
        sLog.outError("MaxOverspeedPings (%i) must be in range 2..infinity (or 0 for disbale check. Set to 2.",m_configs[CONFIG_MAX_OVERSPEED_PINGS]);
        m_configs[CONFIG_MAX_OVERSPEED_PINGS] = 2;
    }

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

    sLog.outString( "Loading player create info & level stats..." );
    objmgr.LoadPlayerInfo();

    sLog.outString( "Loading Quests..." );
    objmgr.LoadQuests();

    sLog.outString( "Loading Spell Chain Data..." );
    objmgr.LoadSpellChains();

    sLog.outString( "Loading Scripts..." );
    objmgr.LoadScripts(sScripts,      "scripts");           // quest scripts
    objmgr.LoadScripts(sSpellScripts, "spell_scripts");     // spell casting scripts

    sLog.outString( "Loading NPC Texts..." );
    objmgr.LoadGossipText();

    sLog.outString( "Loading Quest Area Triggers..." );
    objmgr.LoadAreaTriggerPoints();

    sLog.outString( "Loading Items..." );
    objmgr.LoadItemPrototypes();
    objmgr.LoadAuctionItems();
    objmgr.LoadAuctions();

    sLog.outString( "Returning old mails..." );
    objmgr.ReturnOrDeleteOldMails(false);

    sLog.outString( "Loading item_pages..." );
    objmgr.LoadItemPages();

    sLog.outString( "Loading Creature templates..." );
    objmgr.LoadCreatureTemplates();

    sLog.outString( "Loading Guilds..." );
    objmgr.LoadGuilds();

    sLog.outString( "Loading RaidGroups.." );
    objmgr.LoadRaidGroups();

    sLog.outString( "Loading Teleport Coords..." );
    objmgr.LoadTeleportCoords();

    sLog.outString( "Loading Pet Name Parts..." );
    objmgr.LoadPetNames();

    sLog.outString( "Loading pet level stats..." );
    objmgr.LoadPetLevelInfo();

    objmgr.SetHighestGuids();

    sLog.outString( "Loading Loot Tables..." );
    LoadLootTables();

    sLog.outString( "Initializing Scripts..." );
    if(!LoadScriptingModule())
        exit(1);

    m_timers[WUPDATE_OBJECTS].SetInterval(0);
    m_timers[WUPDATE_SESSIONS].SetInterval(0);
    m_timers[WUPDATE_WEATHERS].SetInterval(1000);
    m_timers[WUPDATE_AUCTIONS].SetInterval(60000);          //set auction update interval to 1 minute

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    mail_timer = ((((localtime( &m_gameTime )->tm_hour + 20) % 24)* HOUR * 1000) / m_timers[WUPDATE_AUCTIONS].GetInterval() );
                                                            //1440
    mail_timer_expires = ( (DAY * 1000) / (m_timers[WUPDATE_AUCTIONS].GetInterval()));
    sLog.outDebug("Mail timer set to: %u, mail return is called every %u minutes", mail_timer, mail_timer_expires);

    sLog.outString( "WORLD: Starting BattleGround System" );
    sBattleGroundMgr.CreateInitialBattleGrounds();

    MaNGOS::Game::Initialize();
    sLog.outString( "WORLD: SetInitialWorldSettings done" );

    sLog.outString( "Loading Transports..." );
    MapManager::Instance().LoadTransports();

    sLog.outString( "WORLD: Starting Event System" );
    StartEventSystem();

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
    internalGameTime += diff;

    if (m_timers[WUPDATE_AUCTIONS].Passed())
    {
        m_timers[WUPDATE_AUCTIONS].Reset();
        //update mails (return old mails with item, or delete it) (tested... works on win)
        if (++mail_timer > mail_timer_expires)
        {
            mail_timer = 0;
            objmgr.ReturnOrDeleteOldMails(true);
        }
        //update auctions
        for (int i = 0; i < 3; i++)
        {
            AuctionHouseObject* AuctionMap;
            switch (i)
            {
                case 0:
                    AuctionMap = objmgr.GetAuctionsMap( 6 );//horde
                    break;
                case 1:
                    AuctionMap = objmgr.GetAuctionsMap( 2 );//ali
                    break;
                case 2:
                    AuctionMap = objmgr.GetAuctionsMap( 7 );//neutral
                    break;
            }

            AuctionHouseObject::AuctionEntryMap::iterator itr,next;
            for (itr = AuctionMap->GetAuctionsBegin(); itr != AuctionMap->GetAuctionsEnd();itr = next)
            {
                next = itr;
                ++next;
                if (time(NULL) > (itr->second->time))
                {
                    // Auction time Expired!
                    if (itr->second->bidder == 0)           // if no one bidded auction...
                    {
                        objmgr.SendAuctionExpiredMail( itr->second );
                    }
                    else
                    {
                        //we should send "item sold"- message if seller is online
                        //we should send item to winner
                        //we should send money to seller
                        objmgr.SendAuctionSuccessfulMail( itr->second );
                                                            //- this functions cleans ram!
                        objmgr.SendAuctionWonMail( itr->second );
                    }
                    //this is better way to dealocate memory, than to do it in some called functions..

                    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",itr->second->Id);
                    objmgr.RemoveAItem(itr->second->item_guid);
                    delete itr->second;
                    AuctionMap->RemoveAuction(itr->first);
                }
            }
        }
    }
    if (m_timers[WUPDATE_SESSIONS].Passed())
    {
        m_timers[WUPDATE_SESSIONS].Reset();

        for (std::set<WorldSession*>::iterator itr = m_kicked_sessions.begin(); itr != m_kicked_sessions.end(); ++itr)
            delete *itr;
        m_kicked_sessions.clear();

        for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
        {
            next = itr;
            next++;

            if(!itr->second)
                continue;

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

        if (scriptSchedule.size() > 0)
            ScriptsProcess();
    }

    // move all creatures with delayed move and remove and delete all objects with delayed remove
    ObjectAccessor::Instance().DoDelayedMovesAndRemoves();
}

void World::ScriptsStart(ScriptMapMap scripts, uint32 id, Object* source, Object* target)
{
    ScriptMapMap::iterator s = scripts.find(id);
    if (s == scripts.end())
        return;

    ScriptMap *s2 = &(s->second);
    ScriptMap::iterator iter;
    bool immedScript = false;
    for (iter = s2->begin(); iter != s2->end(); iter++)
    {
        if (iter->first == 0)
        {
            ScriptAction sa;
            sa.source = source;
            sa.script = &iter->second;
            sa.target = target;
            sWorld.scriptSchedule.insert(pair<uint64, ScriptAction>(0, sa));
            immedScript = true;
        }
        else
        {
            ScriptAction sa;
            sa.source = source;
            sa.script = &iter->second;
            sa.target = target;
            sWorld.scriptSchedule.insert(pair<uint64, ScriptAction>(sWorld.internalGameTime + iter->first, sa));
        }
    }
    if (immedScript)
        sWorld.ScriptsProcess();
}

void World::ScriptsProcess()
{
    if (scriptSchedule.size() == 0)
        return;

    multimap<uint64, ScriptAction>::iterator iter = scriptSchedule.begin();
    while ((scriptSchedule.size() > 0) && (iter->first < internalGameTime))
    {
        ScriptAction const& step = iter->second;
        switch (step.script->command)
        {
            case SCRIPT_COMMAND_SAY:
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_SAY call for NULL creature.");
                    break;
                }

                if(step.source->GetTypeId()!=TYPEID_UNIT)
                {
                    sLog.outError("SCRIPT_COMMAND_SAY call for non-creature (TypeId: %u), skipping.",step.source->GetTypeId());
                    break;
                }

                ((Creature *)iter->second.source)->MonsterSay(step.script->datatext.c_str(), 0, step.target->GetGUID());
                break;
            case SCRIPT_COMMAND_EMOTE:
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_EMOTE call for NULL creature.");
                    break;
                }

                if(step.source->GetTypeId()!=TYPEID_UNIT)
                {
                    sLog.outError("SCRIPT_COMMAND_EMOTE call for non-creature (TypeId: %u), skipping.",step.source->GetTypeId());
                    break;
                }

                ((Creature *)step.source)->HandleEmoteCommand(step.script->datalong);
                break;
            case SCRIPT_COMMAND_FIELD_SET:
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_FIELD_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= step.source->GetValuesCount())
                {
                    sLog.outError("SCRIPT_COMMAND_FIELD_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,step.source->GetValuesCount(),step.source->GetTypeId());
                    break;
                }

                step.source->SetUInt32Value(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_MOVE_TO:
                if(iter->second.source && iter->second.source->isType(TYPE_UNIT))
                {
                    ((Unit *)iter->second.source)->SendMoveToPacket(iter->second.script->x, iter->second.script->y, iter->second.script->z, false, iter->second.script->datalong2 );
                    MapManager::Instance().GetMap(((Unit *)iter->second.source)->GetMapId())->CreatureRelocation(((Creature *)iter->second.source), iter->second.script->x, iter->second.script->y, iter->second.script->z, 0);
                    //char buffff[255];
                    //sprintf(buffff, "M:%d", iter->second.script->datalong2);
                    //((Creature *)iter->second.source)->MonsterSay(buffff, 0, iter->second.target->GetGUID());
                }
                break;
            case SCRIPT_COMMAND_FLAG_SET:
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_FLAG_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= step.source->GetValuesCount())
                {
                    sLog.outError("SCRIPT_COMMAND_FLAG_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,step.source->GetValuesCount(),step.source->GetTypeId());
                    break;
                }

                step.source->SetFlag(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_FLAG_REMOVE:
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_FLAG_REMOVE call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= step.source->GetValuesCount())
                {
                    sLog.outError("SCRIPT_COMMAND_FLAG_REMOVE call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,step.source->GetValuesCount(),step.source->GetTypeId());
                    break;
                }

                step.source->RemoveFlag(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_TEMP_SUMMON:
            {
                if(!step.source)
                {
                    sLog.outError("SCRIPT_COMMAND_TEMP_SUMMON call for NULL unit.");
                    break;
                }

                if(!step.source->isType(TYPE_UNIT))
                {
                    sLog.outError("SCRIPT_COMMAND_TEMP_SUMMON call for non-unit (TypeId: %u), skipping.",step.source->GetTypeId());
                    break;
                }

                if(!sCreatureStorage.LookupEntry<CreatureInfo>(step.script->datalong))
                {
                    sLog.outError("SCRIPT_COMMAND_TEMP_SUMMON call for non existed summon (entry: %u).",step.script->datalong);
                    break;
                }

                float x = step.script->x;
                float y = step.script->y;
                float z = step.script->z;
                float o = step.script->o;

                TemporarySummon* pCreature = new TemporarySummon;
                if (!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), ((Unit*)step.source)->GetMapId(), x, y, z, o, step.script->datalong))
                {
                    delete pCreature;
                    return;
                }

                pCreature->Summon(TEMPSUMMON_REMOVE_DEAD, step.script->datalong2);

                if( pCreature->Attack((Unit *)step.source) )
                {
                    (*pCreature)->Mutate(new TargetedMovementGenerator(*((Unit *)step.source)));
                }
                break;
            }
            default:
                sLog.outError("Unknown script command %u called.",step.script->command);
                break;
        }

        scriptSchedule.erase(iter);

        iter = scriptSchedule.begin();
    }
    return;
}

void World::SendGlobalMessage(WorldPacket *packet, WorldSession *self)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second && itr->second->GetPlayer() &&
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
        if(!itr->second)
            continue;

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

bool World::KickPlayer(std::string playerName)
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
        if( player->IsInWorld() )
        {
            if (playerName == player->GetName())
            {
                itr->second->KickPlayer();
                return true;
            }
        }
    }
    return false;
}

bool World::BanAccount(std::string nameOrIP)
{
    bool is_ip = IsIPAddress(nameOrIP.c_str());

    loginDatabase.escape_string(nameOrIP);

    QueryResult *resultAccounts;

    if(is_ip)
    {
        resultAccounts = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE `last_ip` = '%s'",nameOrIP.c_str());

        loginDatabase.PExecute("INSERT INTO `ip_banned` VALUES ('%s')",nameOrIP.c_str());
    }
    else
    {
        resultAccounts = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE `username` = '%s'",nameOrIP.c_str());

        loginDatabase.PExecute("UPDATE `account` SET `banned` = '1' WHERE `username` = '%s'",nameOrIP.c_str());
    }

    // disconnect all affected players (for IP it's can be many)
    if(resultAccounts)
    {
        do
        {
            Field* fieldsAccount = resultAccounts->Fetch();
            uint32 account = fieldsAccount->GetUInt32();

            WorldSession* s = FindSession(account);
            if(s)
                s->KickPlayer();
        }
        while( resultAccounts->NextRow() );

        delete resultAccounts;
        return true;
    }

    return is_ip;                                           // if not ip and no accounts found then mark as fail
}

bool World::RemoveBanAccount(std::string nameOrIP)
{
    if(IsIPAddress(nameOrIP.c_str()))
    {
        loginDatabase.escape_string(nameOrIP);
        loginDatabase.PExecute("DELETE FROM `ip_banned` WHERE `ip` = '%s'",nameOrIP.c_str());
    }
    else
    {
        loginDatabase.escape_string(nameOrIP);
        loginDatabase.PExecute("UPDATE `account` SET `banned` = '0' WHERE `username` = '%s'",nameOrIP.c_str());
    }
    return true;
}

time_t World::_UpdateGameTime()
{
    time_t thisTime = time(NULL);

    uint32 elapsed = uint32(thisTime - m_Last_tick);

    if(m_ShutdownTimer > 0 && elapsed > 0)
    {
        if( m_ShutdownTimer <= elapsed )
        {
            if(!m_ShutdownIdleMode || GetSessionCount()==0)
                m_stopEvent = true;
            else
                m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
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

void World::ShutdownServ(uint32 time, bool idle)
{
    m_ShutdownIdleMode = idle;

    if(time==0)
    {
        if(!idle || GetSessionCount()==0)
            m_stopEvent = true;
    }
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true);
    }
}

void World::ShutdownMsg(bool show, Player* player)
{
    // not show messages for idle shutdown mode
    if(m_ShutdownIdleMode)
        return;

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

    m_ShutdownIdleMode = false;
    m_ShutdownTimer = 0;
    SendServerMessage(SERVER_MSG_SHUTDOWN_CANCELLED);

    DEBUG_LOG("Server shuttingdown cancelled.");
}

void World::SendServerMessage(ServerMessageType type, const char *text, Player* player)
{
    WorldPacket data(SMSG_SERVER_MESSAGE, 50);              // guess size
    data << uint32(type);
    if(type <= SERVER_MSG_STRING)
        data << text;

    if(player)
        player->GetSession()->SendPacket(&data);
    else
        SendGlobalMessage( &data );
}
