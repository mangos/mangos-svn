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
#include "Database/SQLStorage.h"

#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "World.h"
#include "WorldSession.h"
#include "Group.h"
#include "Guild.h"
#include "ArenaTeam.h"
#include "Transports.h"
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"
#include "Language.h"
#include "GameEvent.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

ScriptMapMap sQuestEndScripts;
ScriptMapMap sQuestStartScripts;
ScriptMapMap sSpellScripts;

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid        = 1;
    m_hiCreatureGuid    = 1;
    m_hiItemGuid        = 1;
    m_hiGoGuid          = 1;
    m_hiDoGuid          = 1;
    m_hiCorpseGuid      = 1;

    m_hiPetNumber       = 1;

    playerInfo          = NULL;
}

ObjectMgr::~ObjectMgr()
{

    for( QuestMap::iterator i = mQuestTemplates.begin( ); i != mQuestTemplates.end( ); ++ i )
    {
        delete i->second;
    }
    mQuestTemplates.clear( );

    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    mAreaTriggers.clear();

    for(PetLevelInfoMap::iterator i = petInfo.begin( ); i != petInfo.end( ); ++ i )
    {
        delete[] i->second;
    }
    petInfo.clear();

    // free only if loaded
    if(playerInfo)
    {
        for (int race = 0; race < MAX_RACES; ++race)
        {
            for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
            {
                delete[] playerInfo[race][class_].levelInfo;
            }
            delete[] playerInfo[race];
        }
        delete[] playerInfo;
    }

    // free group and guild objects
    for (GroupSet::iterator itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
        delete (*itr);
    for (GuildSet::iterator itr = mGuildSet.begin(); itr != mGuildSet.end(); ++itr)
        delete (*itr);
}

Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    GroupSet::const_iterator itr;
    for (itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
    {
        if ((*itr)->GetLeaderGUID() == guid)
        {
            return *itr;
        }
    }

    return NULL;
}

Guild * ObjectMgr::GetGuildById(const uint32 GuildId) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetId() == GuildId)
        {
            return *itr;
        }
    }

    return NULL;
}

Guild * ObjectMgr::GetGuildByName(std::string guildname) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetName() == guildname)
        {
            return *itr;
        }
    }

    return NULL;
}

std::string ObjectMgr::GetGuildNameById(const uint32 GuildId) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetId() == GuildId)
        {
            return (*itr)->GetName();
        }
    }

    return "";
}

ArenaTeam * ObjectMgr::GetArenaTeamById(const uint32 ArenaTeamId) const
{
    ArenaTeamSet::const_iterator itr;
    for (itr = mArenaTeamSet.begin(); itr != mArenaTeamSet.end(); itr++)
    {
        if ((*itr)->GetId() == ArenaTeamId)
        {
            return *itr;
        }
    }

    return NULL;
}

ArenaTeam * ObjectMgr::GetArenaTeamByName(std::string arenateamname) const
{
    ArenaTeamSet::const_iterator itr;
    for (itr = mArenaTeamSet.begin(); itr != mArenaTeamSet.end(); itr++)
    {
        if ((*itr)->GetName() == arenateamname)
        {
            return *itr;
        }
    }

    return NULL;
}

AuctionHouseObject * ObjectMgr::GetAuctionsMap( uint32 location )
{
    switch ( location )
    {
        case 6:                                             //horde
            return & mHordeAuctions;
            break;
        case 2:                                             //alliance
            return & mAllianceAuctions;
            break;
        default:                                            //neutral
            return & mNeutralAuctions;
    }
}

uint32 ObjectMgr::GetAuctionCut(uint32 location, uint32 highBid)
{
    if (location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        return (uint32) (0.15 * highBid * sWorld.getRate(RATE_AUCTION_CUT));
    else
        return (uint32) (0.05 * highBid * sWorld.getRate(RATE_AUCTION_CUT));
}

uint32 ObjectMgr::GetAuctionDeposit(uint32 location, uint32 time, Item *pItem)
{
    uint32 percentance;
    if ( location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        percentance = 25;
    else
        percentance = 5;
    return (uint32) ( ((percentance * sWorld.getRate(RATE_AUCTION_DEPOSIT) * pItem->GetProto()->SellPrice * pItem->GetCount() ) / 100 ) * (time / 120 ) );
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 ObjectMgr::GetAuctionOutBid(uint32 currentBid)
{
    uint32 outbid = (currentBid / 100) * 5;
    if (!outbid)
        outbid = 1;
    return outbid;
}

//does not clear ram
void ObjectMgr::SendAuctionWonMail( AuctionEntry *auction )
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    std::ostringstream msgAuctionWonSubject;
    msgAuctionWonSubject << auction->item_template << ":0:" << AUCTION_WON;

    std::ostringstream msgAuctionWonBody;
    msgAuctionWonBody.width(16);
    msgAuctionWonBody << std::right << std::hex << auction->owner;
    msgAuctionWonBody << std::dec << ":" << auction->bid << ":" << auction->buyout;
    sLog.outDebug( "AuctionWon body string : %s", msgAuctionWonBody.str().c_str() );

    //prepare mail data... :
    uint32 mailId = this->GenerateMailID();
    uint32 itemTextId = this->CreateItemText( msgAuctionWonBody.str() );
    time_t dtime = time(NULL);                              //Will always be instant when from Auction House.
    time_t etime = dtime + (30 * DAY);

    uint64 bidder_guid = MAKE_GUID(auction->bidder,HIGHGUID_PLAYER);
    Player *bidder = objmgr.GetPlayer(bidder_guid);

    // data for gm.log
    if( sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
    {
        uint32 gm_accid = 0;
        uint32 gm_security = 0;
        std::string gm_name;
        if (bidder)
        {
            gm_accid = bidder->GetSession()->GetAccountId();
            gm_security = bidder->GetSession()->GetSecurity();
            gm_name = bidder->GetName();
        }
        else
        {
            gm_accid = GetPlayerAccountIdByGUID(bidder_guid);
            gm_security = GetSecurityByAccount(gm_accid);
            
            if(gm_security > SEC_PLAYER )                                // not do redundant DB requests
            {
                if(!GetPlayerNameByGUID(bidder_guid,gm_name))
                    gm_name = LANG_UNKNOWN;
            }
        }

        if( gm_security > SEC_PLAYER )
        {
            std::string owner_name;
            if(!GetPlayerNameByGUID(auction->owner,owner_name))
                owner_name = LANG_UNKNOWN;

            uint32 owner_accid = GetPlayerAccountIdByGUID(auction->owner);

            sLog.outCommand("GM %s (Account: %u) won item in auction: %s (Entry: %u Count: %u) and pay money: %u. Original owner %s (Account: %u)",
                gm_name.c_str(),gm_accid,pItem->GetProto()->Name1,pItem->GetEntry(),pItem->GetCount(),auction->bid,owner_name.c_str(),owner_accid);
        }
    }

    if (bidder)
    {
        bidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, bidder_guid, 0, 0, auction->item_template);

        bidder->CreateMail(mailId, MAIL_AUCTION, auction->location, msgAuctionWonSubject.str(), itemTextId, auction->item_guid, auction->item_template, etime, dtime, 0, 0, AUCTION_CHECKED, pItem);
    }
    else
        delete pItem;

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "','" I64FMTD "', '0', '0', '%d')",
        mailId, MAIL_AUCTION, auction->location, auction->bidder, msgAuctionWonSubject.str().c_str(), itemTextId, auction->item_guid, auction->item_template, (uint64)etime,(uint64)dtime, AUCTION_CHECKED);
}

//call this method to send mail to auctionowner, when auction is successful, it does not clear ram
void ObjectMgr::SendAuctionSuccessfulMail( AuctionEntry * auction )
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    std::ostringstream msgAuctionSuccessfulSubject;
    msgAuctionSuccessfulSubject << auction->item_template << ":0:" << AUCTION_SUCCESSFUL;

    std::ostringstream auctionSuccessfulBody;
    uint32 auctionCut = this->GetAuctionCut(auction->location, auction->bid);

    auctionSuccessfulBody.width(16);
    auctionSuccessfulBody << std::right << std::hex << auction->bidder;
    auctionSuccessfulBody << std::dec << ":" << auction->bid << ":0:";
    auctionSuccessfulBody << auction->deposit << ":" << auctionCut;
    sLog.outDebug("AuctionSuccessful body string : %s", auctionSuccessfulBody.str().c_str());

    uint32 itemTextId = this->CreateItemText( auctionSuccessfulBody.str() );

    uint32 mailId = this->GenerateMailID();
    time_t dtime = time(NULL);                              //Instant because it's Auction House
    time_t etime = dtime + (30 * DAY);
    uint32 profit = auction->bid + auction->deposit - auctionCut;

    Player *owner = objmgr.GetPlayer((uint64) auction->owner);
    if (owner)
    {
        //send auctionowner notification, bidder must be current!
        owner->GetSession()->SendAuctionOwnerNotification( auction );

        owner->CreateMail(mailId, MAIL_AUCTION, auction->location, msgAuctionSuccessfulSubject.str(), itemTextId, 0, 0, etime, dtime, profit, 0, AUCTION_CHECKED, NULL);
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '0', '0', '" I64FMTD "', '" I64FMTD "', '%u', '0', '%d')",
        mailId, MAIL_AUCTION, auction->location, auction->owner, msgAuctionSuccessfulSubject.str().c_str(), itemTextId, (uint64)etime, (uint64)dtime, profit, AUCTION_CHECKED);
}

//does not clear ram
void ObjectMgr::SendAuctionExpiredMail( AuctionEntry * auction )
{                                                           //return an item in auction to its owner by mail
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(pItem)
    {

        Player *seller = objmgr.GetPlayer((uint64)auction->owner);

        uint32 messageId = objmgr.GenerateMailID();
        std::ostringstream subject;
        subject << auction->item_template << ":0:" << AUCTION_EXPIRED;
        time_t dtime = time(NULL);                          //Instant since it's Auction House
        time_t etime = dtime + 30 * DAY;

        sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
            "VALUES ('%u', '2', '%u', '%u', '%s', '0', '%u', '%u', '" I64FMTD "','" I64FMTD "', '0', '0', '%d')",
            messageId, auction->location, auction->owner, subject.str().c_str(), auction->item_guid, auction->item_template, (uint64)etime, (uint64)dtime, NOT_READ);
        if ( seller )
        {
            seller->GetSession()->SendAuctionOwnerNotification( auction );

            seller->CreateMail(messageId, MAIL_AUCTION, auction->location, subject.str(), 0, auction->item_guid, auction->item_template, etime,dtime,0,0,NOT_READ,pItem);
        }
        else
        {
            delete pItem;
        }
    }
    else
    {
        sLog.outError("Auction item (GUID: %u) not found, and lost.",auction->item_guid);
    }
}

CreatureInfo const* ObjectMgr::GetCreatureTemplate(uint32 id)
{
    return sCreatureStorage.LookupEntry<CreatureInfo>(id);
}

void ObjectMgr::LoadCreatureTemplates()
{
    sCreatureStorage.Load();

    sLog.outString( ">> Loaded %u creature definitions", sCreatureStorage.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadCreatureAddons()
{
    sCreatureDataAddonStorage.Load();

    sLog.outString( ">> Loaded %u creature addons", sCreatureDataAddonStorage.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadCreatures()
{
    uint32 count = 0; 

    //                                            0      1    2     3            4            5            6             7               8           9                 
    QueryResult *result = sDatabase.Query("SELECT `creature`.`guid`,`id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawndist`,`currentwaypoint`,"
    //   10                 11                 12                 13                  14          15        16           17             18
        "`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`DeathState`,`MovementType`,`auras`,`event` "
        "FROM `creature` LEFT OUTER JOIN `game_event_creature` ON `creature`.`guid`=`game_event_creature`.`guid`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 creature. DB table `creature` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 guid = fields[0].GetUInt32();

        CreatureData& data = mCreatureDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.posX           = fields[ 3].GetFloat();
        data.posY           = fields[ 4].GetFloat();
        data.posZ           = fields[ 5].GetFloat();
        data.orientation    = fields[ 6].GetFloat();
        data.spawntimesecs  = fields[ 7].GetUInt32();
        data.spawndist      = fields[ 8].GetFloat();
        data.currentwaypoint= fields[ 9].GetUInt32();
        data.spawn_posX     = fields[10].GetFloat();
        data.spawn_posY     = fields[11].GetFloat();
        data.spawn_posZ     = fields[12].GetFloat();
        data.spawn_orientation = fields[13].GetFloat();
        data.curhealth      = fields[14].GetUInt32();
        data.curmana        = fields[15].GetUInt32();
        data.deathState     = fields[16].GetUInt8();
        data.movementType   = fields[17].GetUInt8();
        data.auras          = fields[18].GetCppString();
        int16 gameEvent     = fields[19].GetInt16();

        if (gameEvent==0) // if not this is to be managed by GameEvent System
            AddCreatureToGrid(guid, &data);
        count++;

    } while (result->NextRow());

    delete result;
    
    sLog.outString( "" );
    sLog.outString( ">> Loaded %u creatures", mCreatureDataMap.size() );
}

void ObjectMgr::AddCreatureToGrid(uint32 guid, CreatureData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.creatures.insert(guid);
}

void ObjectMgr::RemoveCreatureFromGrid(uint32 guid, CreatureData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.creatures.erase(guid);
}

void ObjectMgr::LoadGameobjects()
{
    uint32 count = 0; 

    //                                            0      1    2     3            4            5            6             
    QueryResult *result = sDatabase.Query("SELECT `gameobject`.`guid`,`id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,"
    //   7           8           9           10          11     12              13             14
        "`rotation0`,`rotation1`,`rotation2`,`rotation3`,`loot`,`spawntimesecs`,`animprogress`,`dynflags`,`event` "
        "FROM `gameobject` LEFT OUTER JOIN `game_event_gameobject` ON `gameobject`.`guid`=`game_event_gameobject`.`guid`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 gameobjects. DB table `gameobject` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 guid = fields[0].GetUInt32();

        GameObjectData& data = mGameObjectDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.posX           = fields[ 3].GetFloat();
        data.posY           = fields[ 4].GetFloat();
        data.posZ           = fields[ 5].GetFloat();
        data.orientation    = fields[ 6].GetFloat();
        data.rotation0      = fields[ 7].GetFloat();
        data.rotation1      = fields[ 8].GetFloat();
        data.rotation2      = fields[ 9].GetFloat();
        data.rotation3      = fields[10].GetFloat();
        data.lootid         = fields[11].GetUInt32();
        data.spawntimesecs  = fields[12].GetUInt32();
        data.animprogress   = fields[13].GetUInt32();
        data.dynflags       = fields[14].GetUInt32();
        int16 gameEvent     = fields[15].GetInt16();

        if (gameEvent==0) // if not this is to be managed by GameEvent System
            AddGameobjectToGrid(guid, &data);
        count++;

    } while (result->NextRow());

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u gameobjects", mGameObjectDataMap.size());
}

void ObjectMgr::AddGameobjectToGrid(uint32 guid, GameObjectData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.gameobjects.insert(guid);
}

void ObjectMgr::RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.gameobjects.erase(guid);
}

void ObjectMgr::LoadCreatureRespawnTimes()
{
    // remove outdated data
    sDatabase.Execute("DELETE FROM `creature_respawn` WHERE `respawntime` <= UNIX_TIMESTAMP(NOW())");

    uint32 count = 0; 

    QueryResult *result = sDatabase.Query("SELECT `guid`,`respawntime`,`instance` FROM `creature_respawn`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 creature respawn times.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mCreatureRespawnTimes[MAKE_GUID(loguid,instance)] = time_t(respawn_time);

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString( ">> Loaded %u creature respawn times", mCreatureRespawnTimes.size() );
    sLog.outString( "" );
}

void ObjectMgr::LoadGameobjectRespawnTimes()
{
    // remove outdated data
    sDatabase.Execute("DELETE FROM `gameobject_respawn` WHERE `respawntime` <= UNIX_TIMESTAMP(NOW())");

    uint32 count = 0; 

    QueryResult *result = sDatabase.Query("SELECT `guid`,`respawntime`,`instance` FROM `gameobject_respawn`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 gameobject respawn times.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mGORespawnTimes[MAKE_GUID(loguid,instance)] = time_t(respawn_time);

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString( ">> Loaded %u gameobject respawn times", mGORespawnTimes.size() );
    sLog.outString( "" );
}

void ObjectMgr::LoadSpellThreats()
{
    sSpellThreatStore.Load();

    sLog.outString( ">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount );
    sLog.outString( "" );
}

// name must be checked to correctness (if received) before call this function
uint64 ObjectMgr::GetPlayerGUIDByName(std::string name) const
{
    uint64 guid = 0;

    sDatabase.escape_string(name);

    // Player name safe to sending to DB (checked at login) and this function using
    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name.c_str());
    if(result)
    {
        guid = MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER);

        delete result;
    }

    return guid;
}

bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{
    // prevent DB access for online player
    if(Player* player = GetPlayer(guid))
    {
        name  = player->GetName();
        return true;
    }

    QueryResult *result = sDatabase.PQuery("SELECT `name` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

    if(result)
    {
        name = (*result)[0].GetCppString();
        delete result;
        return true;
    }

    return false;
}

uint32 ObjectMgr::GetPlayerTeamByGUID(const uint64 &guid) const
{
    QueryResult *result = sDatabase.PQuery("SELECT `race` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

    if(result)
    {
        uint8 race = (*result)[0].GetUInt8();
        delete result;
        return Player::TeamForRace(race);
    }

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByGUID(const uint64 &guid) const
{
    QueryResult *result = sDatabase.PQuery("SELECT `account` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));
    if(result)
    {
        uint32 acc = (*result)[0].GetUInt32();
        delete result;
        return acc;
    }

    return 0;
}

uint32 ObjectMgr::GetSecurityByAccount(uint32 acc_id) const
{
    QueryResult *result = loginDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `id` = '%u'", acc_id);
    if(result)
    {
        uint32 sec = (*result)[0].GetUInt32();
        delete result;
        return sec;
    }

    return 0;
}

void ObjectMgr::LoadAuctions()
{
    QueryResult *result = sDatabase.Query("SELECT COUNT(*) FROM `auctionhouse`");

    Field *fields = result->Fetch();
    uint32 AuctionCount=fields[0].GetUInt32();
    delete result;

    if(!AuctionCount)
        return;

    result = sDatabase.Query( "SELECT `id`,`auctioneerguid`,`itemguid`,`item_template`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`startbid`,`deposit`,`location` FROM `auctionhouse`" );

    if( !result )
        return;

    barGoLink bar( AuctionCount );

    AuctionEntry *aItem;

    do
    {
        fields = result->Fetch();

        bar.step();

        aItem = new AuctionEntry;
        aItem->Id = fields[0].GetUInt32();
        aItem->auctioneer = fields[1].GetUInt32();
        aItem->item_guid = fields[2].GetUInt32();
        aItem->item_template = fields[3].GetUInt32();
        aItem->owner = fields[4].GetUInt32();
        aItem->buyout = fields[5].GetUInt32();
        aItem->time = fields[6].GetUInt32();
        aItem->bidder = fields[7].GetUInt32();
        aItem->bid = fields[8].GetUInt32();
        aItem->startbid = fields[9].GetUInt32();
        aItem->deposit = fields[10].GetUInt32();
        aItem->location = fields[11].GetUInt8();
        //check if sold item exists
        if ( this->GetAItem( aItem->item_guid ) )
        {
            GetAuctionsMap( aItem->location )->AddAuction(aItem);
        }
        else
        {
            sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",aItem->Id);
            sLog.outError("Auction %u have not existing item : %u", aItem->Id, aItem->item_guid);
            delete aItem;
        }
    } while (result->NextRow());
    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u auctions", AuctionCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadItemPrototypes()
{
    sItemStorage.Load ();
    sLog.outString( ">> Loaded %u item prototypes", sItemStorage.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadAuctionItems()
{
    QueryResult *result = sDatabase.Query( "SELECT `itemguid`,`item_template` FROM `auctionhouse`" );

    if( !result )
        return;

    barGoLink bar( result->GetRowCount() );

    uint32 count = 0;

    Field *fields;
    do
    {
        bar.step();

        fields = result->Fetch();
        uint32 item_guid        = fields[0].GetUInt32();
        uint32 item_template    = fields[1].GetUInt32();

        ItemPrototype const *proto = objmgr.GetItemPrototype(item_template);

        if(!proto)
        {
            sLog.outError( "ObjectMgr::LoadAuctionItems: Unknown item (GUID: %u id: #%u) in auction, skipped.", item_guid,item_template);
            continue;
        }

        Item *item = NewItemOrBag(proto);

        if(!item->LoadFromDB(item_guid,0))
        {
            delete item;
            continue;
        }
        AddAItem(item);

        ++count;
    }
    while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u auction items", count );
}

void ObjectMgr::LoadPetLevelInfo()
{
    // Loading levels data
    {
        //                                              0               1       2    3      4     5     6     7     8     9
        QueryResult *result  = sDatabase.Query("SELECT `creature_entry`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi`,`armor` FROM `pet_levelstats`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u level pet stats definitions", count );
            sLog.outErrorDb( "Error loading pet_levelstats table or table empty.");
            return;
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 creature_id = fields[0].GetUInt32();
            if(!sCreatureStorage.LookupEntry<CreatureInfo>(creature_id))
            {
                sLog.outErrorDb("Wrong creature id %u in `pet_levelstats` table, ignoring.",creature_id);
                continue;
            }

            uint32 current_level = fields[1].GetUInt32();
            if(current_level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > 255)                     // hardcoded level maximum
                    sLog.outErrorDb("Wrong (> 255) level %u in `pet_levelstats` table, ignoring.",current_level);
                else
                    sLog.outDetail("Unused (> MaxPlayerLevel in mangosd.conf) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }
            else if(current_level < 1)
            {
                sLog.outErrorDb("Wrong (<1) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }

            PetLevelInfo*& pInfoMapEntry = petInfo[creature_id];

            if(pInfoMapEntry==NULL)
                pInfoMapEntry =  new PetLevelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            // data for level 1 stored in [0] array element, ...
            PetLevelInfo* pLevelInfo = &pInfoMapEntry[current_level-1];

            pLevelInfo->health = fields[2].GetUInt16();
            pLevelInfo->mana   = fields[3].GetUInt16();
            pLevelInfo->armor  = fields[9].GetUInt16();

            for (int i = 0; i < MAX_STATS; i++)
            {
                pLevelInfo->stats[i] = fields[i+4].GetUInt16();
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u level pet stats definitions", count );
    }

    // Fill gaps and check integrity
    for (PetLevelInfoMap::iterator itr = petInfo.begin(); itr != petInfo.end(); ++itr)
    {
        PetLevelInfo* pInfo = itr->second;

        // fatal error if no level 1 data
        if(!pInfo || pInfo[0].health == 0 )
        {
            sLog.outErrorDb("Creature %u Level 1 not have pet stats data!",itr->first);
            exit(1);
        }

        // fill level gaps
        for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
        {
            if(pInfo[level].health == 0)
            {
                sLog.outErrorDb("Creature %u Level %i not have pet stats data, using data for %i.",itr->first,level+1, level);
                pInfo[level] = pInfo[level-1];
            }
        }
    }
}

PetLevelInfo const* ObjectMgr::GetPetLevelInfo(uint32 creature_id, uint32 level) const
{
    if(level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        level = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL);

    PetLevelInfoMap::const_iterator itr = petInfo.find(creature_id);
    if(itr == petInfo.end())
        return NULL;

    return &itr->second[level-1];                           // data for level 1 stored in [0] array element, ...
}

void ObjectMgr::LoadPlayerInfo()
{
    // allocate dynamic array
    playerInfo = new PlayerInfo*[MAX_RACES];
    for (uint8 race = 0; race < MAX_RACES; ++race)
    {
        playerInfo[race] = new PlayerInfo[MAX_CLASSES];
    }

    // Load playercreate
    {
        //                                              0      1       2     3      4            5            6
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`map`,`zone`,`position_x`,`position_y`,`position_z` FROM `playercreateinfo`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create definitions", count );
            sLog.outErrorDb( "Error loading `playercreateinfo` table or table empty.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outErrorDb("Wrong race %u in `playercreateinfo` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outErrorDb("Wrong class %u in `playercreateinfo` table, ignoring.",current_class);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            pInfo->mapId     = fields[2].GetUInt32();
            pInfo->zoneId    = fields[3].GetUInt32();
            pInfo->positionX = fields[4].GetFloat();
            pInfo->positionY = fields[5].GetFloat();
            pInfo->positionZ = fields[6].GetFloat();

            ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(fields[0].GetUInt32());
            if(rEntry)
            {
                pInfo->displayId_m = rEntry->model_m;
                pInfo->displayId_f = rEntry->model_f;
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u player create definitions", count );
    }

    // Load playercreate items
    {
        //                                            0      1       2        3
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`itemid`,`amount` FROM `playercreateinfo_item`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create items", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_item` table or table empty.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_item` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_item` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];

                pInfo->item.push_back(PlayerCreateInfoItem( fields[2].GetUInt32(), fields[3].GetUInt32()));

                bar.step();
                count++;
            }
            while(result->NextRow());

            delete result;

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create items", count );
        }
    }

    // Load playercreate spells
    {
        //                                            0      1       2       3
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`Spell`,`Active` FROM `playercreateinfo_spell`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create spells", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_spell` table or table empty.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_spell` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_spell` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->spell.push_back(CreateSpellPair(fields[2].GetUInt16(), fields[3].GetBool()));

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create spells", count );
        }
    }

    // Load playercreate skills
    {
        //                                            0      1       2
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`Skill` FROM `playercreateinfo_skill`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create skills", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_skill` table or table empty.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_skill` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_skill` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];

                uint32 skill = fields[2].GetUInt16();

                pInfo->skill.push_back(skill);

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create skills", count );
        }
    }

    // Load playercreate actions
    {
        //                                                    0      1       2        3        4      5
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`button`,`action`,`type`,`misc` FROM `playercreateinfo_action`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create actions", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_action` table or table empty.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_action` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_action` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->action[0].push_back(fields[2].GetUInt16());
                pInfo->action[1].push_back(fields[3].GetUInt16());
                pInfo->action[2].push_back(fields[4].GetUInt16());
                pInfo->action[3].push_back(fields[5].GetUInt16());

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create actions", count );
        }
    }

    // Loading levels data
    {
        //                                              0      1       2       3    4      5     6     7     8     9
        QueryResult *result  = sDatabase.Query("SELECT `race`,`class`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi` FROM `player_levelstats`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u level stats definitions", count );
            sLog.outErrorDb( "Error loading player_levelstats table or table empty.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outErrorDb("Wrong race %u in `player_levelstats` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outErrorDb("Wrong class %u in `player_levelstats` table, ignoring.",current_class);
                continue;
            }

            uint32 current_level = fields[2].GetUInt32();
            if(current_level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > 255)                     // hardcoded level maximum
                    sLog.outErrorDb("Wrong (> 255) level %u in `player_levelstats` table, ignoring.",current_level);
                else
                    sLog.outDetail("Unused (> MaxPlayerLevel in mangosd.conf) level %u in `player_levelstats` table, ignoring.",current_level);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            if(!pInfo->levelInfo)
                pInfo->levelInfo = new PlayerLevelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            PlayerLevelInfo* pLevelInfo = &pInfo->levelInfo[current_level-1];

            pLevelInfo->health = fields[3].GetUInt16();
            pLevelInfo->mana   = fields[4].GetUInt16();

            for (int i = 0; i < MAX_STATS; i++)
            {
                pLevelInfo->stats[i] = fields[i+5].GetUInt8();
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u level stats definitions", count );
    }

    // Fill gaps and check integrity
    for (int race = 0; race < MAX_RACES; ++race)
    {
        // skip non existed races
        if(!sChrRacesStore.LookupEntry(race))
            continue;

        for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
        {
            // skip non existed classes
            if(!sChrClassesStore.LookupEntry(class_))
                continue;

            PlayerInfo* pInfo = &playerInfo[race][class_];

            // skip non loaded combinations
            if(!pInfo->displayId_m || !pInfo->displayId_f)
                continue;
            
            // skip expansion races if not playing with expansion
            if (!sWorld.getConfig(CONFIG_EXPANSION) && (race == RACE_BLOODELF || race == RACE_DRAENEI))
                continue;

            // fatal error if no level 1 data
            if(!pInfo->levelInfo || pInfo->levelInfo[0].health == 0 )
            {
                sLog.outErrorDb("Race %i Class %i Level 1 does not have stats data!",race,class_);
                exit(1);
            }

            // fill level gaps
            for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
            {
                if(pInfo->levelInfo[level].health == 0)
                {
                    sLog.outErrorDb("Race %i Class %i Level %i does not have stats data. Using stats data of level %i.",race,class_,level+1, level);
                    pInfo->levelInfo[level] = pInfo->levelInfo[level-1];
                }
            }
        }
    }
}

void ObjectMgr::GetPlayerLevelInfo(uint32 race, uint32 class_, uint32 level, PlayerLevelInfo* info) const
{
    if(level < 1) return;
    if(race   >= MAX_RACES)   return;
    if(class_ >= MAX_CLASSES) return;
    PlayerInfo const* pInfo = &playerInfo[race][class_];
    if(pInfo->displayId_m==0 || pInfo->displayId_f==0) return;

    if(level <= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        *info = pInfo->levelInfo[level-1];
    else
        BuildPlayerLevelInfo(race,class_,level,info);
}

void ObjectMgr::BuildPlayerLevelInfo(uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info) const
{
    // base data (last known level)
    *info = playerInfo[race][_class].levelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)-1];

    for(int lvl = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)-1; lvl < level; ++lvl)
    {
        switch(_class)
        {
            case CLASS_WARRIOR:
                info->stats[STAT_STRENGTH]  += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 9 && !(lvl%2) ? 1: 0);
                break;
            case CLASS_PALADIN:
                info->stats[STAT_STRENGTH]  += (lvl > 3  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 6 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 7 ? 1: 0);
                break;
            case CLASS_HUNTER:
                info->stats[STAT_STRENGTH]  += (lvl > 4  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_ROGUE:
                info->stats[STAT_STRENGTH]  += (lvl > 5  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_PRIEST:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 3  ? 1: 0);
                break;
            case CLASS_SHAMAN:
                info->stats[STAT_STRENGTH]  += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 4 ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 7 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 5 ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 4 ? 1: 0);
                break;
            case CLASS_MAGE:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                break;
            case CLASS_WARLOCK:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                break;
            case CLASS_DRUID:
                info->stats[STAT_STRENGTH]  += (lvl > 38 ? 2: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 32 ? 2: (lvl > 4 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 2: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 38 ? 3: (lvl > 4 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 3: (lvl > 5 ? 1: 0));
        }

        info->mana   += (_class == CLASS_WARRIOR || _class == CLASS_ROGUE) ? 0 : info->stats[STAT_SPIRIT] / 2;
        info->health += info->stats[STAT_STAMINA] / 2;
    }
}

void ObjectMgr::LoadGuilds()
{
    Guild *newguild;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `guildid` FROM `guild`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u guild definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();
        count++;

        newguild = new Guild;
        if(!newguild->LoadGuildFromDB(fields[0].GetUInt32()))
        {
            newguild->Disband();
            delete newguild;
            continue;
        }
        AddGuild(newguild);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u guild definitions", count );
}

void ObjectMgr::LoadArenaTeams()
{
    ArenaTeam *newarenateam;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `arenateamid` FROM `arena_team`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u arenateam definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();
        count++;

        newarenateam = new ArenaTeam;
        newarenateam->LoadArenaTeamFromDB(fields[0].GetUInt32());
        AddArenaTeam(newarenateam);
    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u arenateam definitions", count );
}

void ObjectMgr::LoadGroups()
{
    Group *group;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `leaderGuid` FROM `group`" );

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u group definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();
        count++;

        group = new Group;
        if(!group->LoadGroupFromDB(MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER)))
        {
            group->Disband();
            delete group;
            continue;
        }
        AddGroup(group);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u group definitions", count );
}

void ObjectMgr::LoadQuests()
{
    //                                            0       1         2          3            4
    QueryResult *result = sDatabase.Query("SELECT `entry`,`ZoneOrSort`, `MinLevel`,`QuestLevel`,`Type`,"
    //   5               6                    7                    8                  9                  10
        "`RequiredRaces`,`RequiredSkillValue`,`RequiredRepFaction`,`RequiredRepValue`,`SuggestedPlayers`,`LimitTime`,"
    //   11             12            13            14               15                 16          17             18
        "`SpecialFlags`,`PrevQuestId`,`NextQuestId`,`ExclusiveGroup`,`NextQuestInChain`,`SrcItemId`,`SrcItemCount`,`SrcSpell`,"
    //   19      20        21           22                23                 24        25               26               27               28
        "`Title`,`Details`,`Objectives`,`OfferRewardText`,`RequestItemsText`,`EndText`,`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,"
    //   29           30           31           32           33              34              35              36
        "`ReqItemId1`,`ReqItemId2`,`ReqItemId3`,`ReqItemId4`,`ReqItemCount1`,`ReqItemCount2`,`ReqItemCount3`,`ReqItemCount4`,"
    //   37             38             39             40             41                42                43                44                45              46              47              48
        "`ReqSourceId1`,`ReqSourceId2`,`ReqSourceId3`,`ReqSourceId4`,`ReqSourceCount1`,`ReqSourceCount2`,`ReqSourceCount3`,`ReqSourceCount4`,`ReqSourceRef1`,`ReqSourceRef2`,`ReqSourceRef3`,`ReqSourceRef4`,"
    //   49                   50                   51                   52                   53                      54                      55                      56
        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOId2`,`ReqCreatureOrGOId3`,`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount1`,`ReqCreatureOrGOCount2`,`ReqCreatureOrGOCount3`,`ReqCreatureOrGOCount4`,"
    //   57              58              59              60
        "`ReqSpellCast1`,`ReqSpellCast2`,`ReqSpellCast3`,`ReqSpellCast4`,"
    //   61                 62                 63                 64                 65                 66
        "`RewChoiceItemId1`,`RewChoiceItemId2`,`RewChoiceItemId3`,`RewChoiceItemId4`,`RewChoiceItemId5`,`RewChoiceItemId6`,"
    //   67                    68                    69                    70                    71                    72
        "`RewChoiceItemCount1`,`RewChoiceItemCount2`,`RewChoiceItemCount3`,`RewChoiceItemCount4`,`RewChoiceItemCount5`,`RewChoiceItemCount6`,"
    //   73           74           75           76           77              78              79              80
        "`RewItemId1`,`RewItemId2`,`RewItemId3`,`RewItemId4`,`RewItemCount1`,`RewItemCount2`,`RewItemCount3`,`RewItemCount4`,"
    //   81               82               83               84               85               86             87             88             89             90
        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
    //   91              92      93         94           95       96       97         98              99              100             101
        "`RewOrReqMoney`,`RewXpOrMoney`,`RewSpell`,`PointMapId`,`PointX`,`PointY`,`PointOpt`,`DetailsEmote1`,`DetailsEmote2`,`DetailsEmote3`,`DetailsEmote4`,"
    //   102               103             104                 105                 106                 107                 108           109          110     
        "`IncompleteEmote`,`CompleteEmote`,`OfferRewardEmote1`,`OfferRewardEmote2`,`OfferRewardEmote3`,`OfferRewardEmote4`,`StartScript`,`CompleteScript`,`Repeatable`"
        " FROM `quest_template`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 quests definitions" );
        sLog.outErrorDb("`quest_template` table is empty!");
        return;
    }

    // create multimap previous quest for each existed quest
    // some quests can have many previous maps set by NextQuestId in previous quest
    // for example set of race quests can lead to single not race specific quest
    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        Quest * newQuest = new Quest(fields);
        mQuestTemplates[newQuest->GetQuestId()] = newQuest;
    } while( result->NextRow() );

    delete result;

    // Post processing
    for (QuestMap::iterator iter = mQuestTemplates.begin(); iter != mQuestTemplates.end(); iter++)
    {
        Quest * qinfo = iter->second;

        // additional quest integrity checks (GO, creature_template and item_template must be loaded already)

        if(qinfo->RequiredSkillValue && qinfo->RequiredSkillValue > sWorld.GetConfigMaxSkillValue())
        {
            sLog.outErrorDb("Quest %u has `RequiredSkillValue` = %u but max possible skill is %u, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredSkillValue,sWorld.GetConfigMaxSkillValue());
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredRepFaction))
        {
            sLog.outErrorDb("Quest %u has `RequiredRepFaction` = %u but faction template %u doesn't exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredRepFaction,qinfo->RequiredRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredRepValue && int32(qinfo->RequiredRepValue) > Player::Reputation_Cap)
        {
            sLog.outErrorDb("Quest %u has `RequiredRepValue` = %u but max reputation is %u, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredRepValue,Player::Reputation_Cap);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->SrcItemId && !sItemStorage.LookupEntry<ItemPrototype>(qinfo->SrcItemId))
        {
            sLog.outErrorDb("Quest %u has `SrcItemId` = %u but item with entry %u doesn't exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->SrcItemId,qinfo->SrcItemId);
            qinfo->SrcItemId = 0;                           // quest can't be done for this requirement
        }

        if(qinfo->SrcSpell && !sSpellStore.LookupEntry(qinfo->SrcSpell))
        {
            sLog.outErrorDb("Quest %u has `SrcSpell` = %u but spell %u doesn't exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->SrcSpell,qinfo->SrcSpell);
            qinfo->SrcSpell = 0;                            // quest can't be done for this requirement
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqItemId[j];
            if(id)
            {
                if((qinfo->SpecialFlags & QUEST_SPECIAL_FLAGS_DELIVER)==0)
                {
                    sLog.outErrorDb("Quest %u has `ReqItemId%d` = %u but `SpecialFlags` not have set delivery flag bit, quest can be done without item delivery!",
                        qinfo->GetQuestId(),j+1,id);
                    // no changes, quest can be incorrectly done, but we already report this
                }

                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `ReqItemId%d` = %u but item with entry %u doesn't exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSourceId[j];
            if(id)
            {
                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = %u but item with entry %u doesn't exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }
                if(!qinfo->ReqSourceCount[j])
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = %u but `ReqSourceCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can't be done for this requirement
                }
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 ref = qinfo->ReqSourceRef[j];
            if(ref)
            {
                if(ref > QUEST_OBJECTIVES_COUNT)
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceRef%d` = %u but max value in `ReqSourceRef%d` is %u, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,j+1,QUEST_OBJECTIVES_COUNT);
                    // no changes, quest can't be done for this requirement
                }
                else
                if(qinfo->ReqSourceId[j] && !qinfo->ReqItemId[ref-1] && !qinfo->ReqSpell[ref-1])
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceRef%d` = %u but `ReqItemId%u` = 0 and `ReqSpellCast%u` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,ref,ref);
                    // no changes, quest can't be done for this requirement
                }
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            int32 id = qinfo->ReqCreatureOrGOId[j];
            if(id < 0 && !sGOStorage.LookupEntry<GameObject>(-id))
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %i but gameobject %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(-id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id > 0 && !sCreatureStorage.LookupEntry<CreatureInfo>(id))
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %i but creature with entry %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id && (qinfo->SpecialFlags & QUEST_SPECIAL_FLAGS_KILL_OR_CAST)==0)
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %u but `SpecialFlags` not have set killOrCast flag bit, quest can be done without creature/go kill/cast!",
                    qinfo->GetQuestId(),j+1,id);
                // no changes, quest can be incorrectly done, but we already report this
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSpell[j];
            if(id && !sSpellStore.LookupEntry(id))
            {
                sLog.outErrorDb("Quest %u has `ReqSpellCast%d` = %u but spell %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,id);
                // no changes, quest can't be done for this requirement
            }
        }

        for(int j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j )
        {
            uint32 id = qinfo->RewChoiceItemId[j];
            if(id && !sItemStorage.LookupEntry<ItemPrototype>(id))
            {
                sLog.outErrorDb("Quest %u has `RewChoiceItemId%d` = %u but item with entry %u doesn't exist, quest will not reward this item.",
                    qinfo->GetQuestId(),j+1,id,id);
                qinfo->RewChoiceItemId[j] = 0;              // no changes, quest will not reward this
            }
        }

        for(int j = 0; j < QUEST_REWARDS_COUNT; ++j )
        {
            uint32 id = qinfo->RewItemId[j];
            if(id && !sItemStorage.LookupEntry<ItemPrototype>(id))
            {
                sLog.outErrorDb("Quest %u has `RewItemId%d` = %u but item with entry %u doesn't exist, quest will not reward this item.",
                    qinfo->GetQuestId(),j+1,id,id);
                qinfo->RewItemId[j] = 0;                    // no changes, quest will not reward this item
            }
        }

        for(int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        {
            if(qinfo->RewRepFaction[j] && !sFactionStore.LookupEntry(qinfo->RewRepFaction[j]))
            {
                sLog.outErrorDb("Quest %u has `RewRepFaction%d` = %u but raw faction (faction.dbc) %u doesn't exist, quest will not reward reputation for this faction.",
                    qinfo->GetQuestId(),j+1,qinfo->RewRepFaction[j] ,qinfo->RewRepFaction[j] );
                qinfo->RewRepFaction[j] = 0;                // no changes, quest will not reward this
            }
        }

        if(qinfo->RewSpell && !sSpellStore.LookupEntry(qinfo->RewSpell))
        {
            sLog.outErrorDb("Quest %u has `RewSpell` = %u but spell %u doesn't exist, quest will not have a spell reward.",
                qinfo->GetQuestId(),qinfo->RewSpell,qinfo->RewSpell);
            qinfo->RewSpell = 0;                            // no changes, quest will not reward this
        }

        if(qinfo->NextQuestInChain)
        {
            if(mQuestTemplates.find(qinfo->NextQuestInChain) == mQuestTemplates.end())
            {
                sLog.outErrorDb("Quest %u has `NextQuestInChain` = %u but quest %u doesn't exist, quest chain will not work.",
                    qinfo->GetQuestId(),qinfo->NextQuestInChain ,qinfo->NextQuestInChain );
                qinfo->NextQuestInChain = 0;
            }
            else
                mQuestTemplates[qinfo->NextQuestInChain]->prevChainQuests.push_back(qinfo->GetQuestId());
        }

        // fill additional data stores
        if(qinfo->PrevQuestId)
            qinfo->prevQuests.push_back(qinfo->PrevQuestId);

        if(qinfo->NextQuestId)
        {
            if (mQuestTemplates.find(abs(qinfo->GetNextQuestId())) == mQuestTemplates.end())
            {
                sLog.outErrorDb("Quest %d has NextQuestId %i, but no such quest", qinfo->GetQuestId(), qinfo->GetNextQuestId());
            }
            else
            {
                int32 signedQuestId = qinfo->NextQuestId < 0 ? -int32(qinfo->GetQuestId()) : int32(qinfo->GetQuestId());
                mQuestTemplates[abs(qinfo->GetNextQuestId())]->prevQuests.push_back(signedQuestId);
            }
        }

        if(qinfo->ExclusiveGroup)
            mExclusiveQuestGroups.insert(std::pair<uint32, uint32>(qinfo->ExclusiveGroup, qinfo->GetQuestId()));
    }

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u quests definitions", mQuestTemplates.size() );
}

void ObjectMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    QueryResult *result = sDatabase.PQuery("SELECT `spell_id`, `prev_spell`, `first_spell`, `rank` FROM `spell_chain`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 spell chain records" );
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` not exist",spell_id);
            continue;
        }

        if(node.prev!=0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` have non existed previous rank spell: %u",spell_id,node.prev);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` have non existed first rank spell: %u",spell_id,node.first);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) ||
            (spell_id == node.first) != (node.prev == 0) ||
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` have not compatible chain data (prev: %u, first: %u, rank: %d)",spell_id,node.prev,node.first,node.rank);
            continue;
        }

        mSpellChains[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    // additional integrity checks
    for(SpellChainMap::iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::iterator i_prev = mSpellChains.find(i->second.prev);
            if(i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` have not found in table previous rank spell.",
                    i->first,i->second.prev,i->second.first,i->second.rank);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` have different first spell in chain in comparison with previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` have different rank in comparison with previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
        }
    }

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell chain records", count );
}

void ObjectMgr::LoadSpellLearnSkills()
{
    QueryResult *result = sDatabase.PQuery("SELECT `entry`, `SkillID`, `Value`, `MaxValue` FROM `spell_learn_skill`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 spell learn skills" );
        sLog.outErrorDb("`spell_learn_skill` table is empty!");
        return;
    }

    uint32 count = 0;

    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        int32 skill_val = fields[2].GetInt32();
        int32 skill_max = fields[3].GetInt32();

        SpellLearnSkillNode node;
        node.skill    = fields[1].GetUInt32();
        node.value    = skill_val < 0 ? maxconfskill : skill_val;
        node.maxvalue = skill_max < 0 ? maxconfskill : skill_max;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_skill` not exist",spell_id);
            continue;
        }

        if(!sSkillLineStore.LookupEntry(node.skill))
        {
            sLog.outErrorDb("Skill %u listed in `spell_learn_skill` not exist",node.skill);
            continue;
        }

        SpellLearnSkills[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.value    = 1;
                dbc_node.maxvalue = (entry->EffectBasePoints[i]+1)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                if(db_node)
                {
                    if(db_node->skill != dbc_node.skill)
                        sLog.outErrorDb("Spell %u auto-learn skill %u in spell.dbc but learn skill %u in `spell_learn_skill`, fix DB.",
                            spell,dbc_node.skill,db_node->skill);

                    continue;                               // skip already added spell-skill pair
                }

                SpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell learn skills + found in DBC %u", count, dbc_count );
}

void ObjectMgr::LoadSpellLearnSpells()
{
    QueryResult *result = sDatabase.PQuery("SELECT `entry`, `SpellID`,`IfNoSpell` FROM `spell_learn_spell`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 spell learn spells" );
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.ifNoSpell  = fields[2].GetUInt32();
        node.autoLearned= false;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` not exist",spell_id);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` not exist",node.spell);
            continue;
        }

        if(node.ifNoSpell && !sSpellStore.LookupEntry(node.ifNoSpell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` not exist",node.ifNoSpell);
            continue;
        }

        SpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.ifNoSpell   = 0;
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if(itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc and record in `spell_learn_spell` redundant, fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if(!found)                                  // add new spell-spell pair if not found
                {
                    SpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell learn spells + found in DBC %u", count, dbc_count );
}

void ObjectMgr::LoadPetCreateSpells()
{
    QueryResult *result = sDatabase.PQuery("SELECT `entry`, `Spell1`, `Spell2`, `Spell3`, `Spell4`,`FamilyPassive` FROM `petcreateinfo_spell`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 pet create spells" );
        sLog.outErrorDb("`petcreateinfo_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );

    mPetCreateSpell.clear();

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 creature_id = fields[0].GetUInt32();
        
        if(!creature_id || !sCreatureStorage.LookupEntry<CreatureInfo>(creature_id))
            continue;

        PetCreateSpellEntry PetCreateSpell;
        for(int i = 0; i < 4; i++)
        {
            PetCreateSpell.spellid[i] = fields[i + 1].GetUInt32();

            if(PetCreateSpell.spellid[i] && !sSpellStore.LookupEntry(PetCreateSpell.spellid[i]))
                sLog.outErrorDb("Spell %u listed in `petcreateinfo_spell` not exist",PetCreateSpell.spellid[i]);
        }
        
        PetCreateSpell.familypassive = fields[5].GetUInt32();
        
        if(PetCreateSpell.familypassive && !sSpellStore.LookupEntry(PetCreateSpell.familypassive))
            sLog.outErrorDb("Spell %u listed in `petcreateinfo_spell` not exist",PetCreateSpell.familypassive);

        mPetCreateSpell[creature_id] = PetCreateSpell;

        count++;
    }
    while (result->NextRow());

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u pet create spells", count );
}

void ObjectMgr::LoadScripts(ScriptMapMap& scripts, char const* tablename)
{
    sLog.outString( "%s :", tablename);

    QueryResult *result = sDatabase.PQuery( "SELECT `id`,`delay`,`command`,`datalong`,`datalong2`,`datatext`, `x`, `y`, `z`, `o` FROM `%s`", tablename );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u script definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        ScriptInfo tmp;
        tmp.id = fields[0].GetUInt32();
        tmp.delay = fields[1].GetUInt32();
        tmp.command = fields[2].GetUInt32();
        tmp.datalong = fields[3].GetUInt32();
        tmp.datalong2 = fields[4].GetUInt32();
        tmp.datatext = fields[5].GetString();
        tmp.x = fields[6].GetFloat();
        tmp.y = fields[7].GetFloat();
        tmp.z = fields[8].GetFloat();
        tmp.o = fields[9].GetFloat();

        if (scripts.find(tmp.id) == scripts.end())
        {
            ScriptMap emptyMap;
            scripts[tmp.id] = emptyMap;
        }
        scripts[tmp.id].insert(std::pair<uint32, ScriptInfo>(tmp.delay, tmp));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u script definitions", count );
}

void ObjectMgr::LoadItemTexts()
{
    QueryResult *result = sDatabase.PQuery("SELECT `id`, `text` FROM `item_text`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u item pages", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    Field* fields;
    do
    {
        bar.step();

        fields = result->Fetch();

        mItemTexts[ fields[0].GetUInt32() ] = fields[1].GetCppString();

        ++count;

    } while ( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u item texts", count );
}

void ObjectMgr::LoadPageTexts()
{
    sPageTextStore.Load ();
    sLog.outString( ">> Loaded %u page texts", sPageTextStore.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::AddGossipText(GossipText *pGText)
{
    ASSERT( pGText->Text_ID );
    ASSERT( mGossipText.find(pGText->Text_ID) == mGossipText.end() );
    mGossipText[pGText->Text_ID] = pGText;
}

GossipText *ObjectMgr::GetGossipText(uint32 Text_ID)
{
    GossipTextMap::const_iterator itr;
    for (itr = mGossipText.begin(); itr != mGossipText.end(); itr++)
    {
        if(itr->second->Text_ID == Text_ID)
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::LoadGossipText()
{
    GossipText *pGText;
    QueryResult *result = sDatabase.Query( "SELECT * FROM `npc_text`" );

    int count = 0;
    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u npc texts", count );
        return;
    }

    int cic;

    barGoLink bar( result->GetRowCount() );

    do
    {
        count++;
        cic = 0;

        Field *fields = result->Fetch();

        bar.step();

        pGText = new GossipText;
        pGText->Text_ID    = fields[cic++].GetUInt32();

        for (int i=0; i< 8; i++)
        {
            pGText->Options[i].Text_0           = fields[cic++].GetCppString();
            pGText->Options[i].Text_1           = fields[cic++].GetCppString();

            pGText->Options[i].Language         = fields[cic++].GetUInt32();
            pGText->Options[i].Probability      = fields[cic++].GetFloat();

            pGText->Options[i].Emotes[0]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[0]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[1]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[1]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[2]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[2]._Emote  = fields[cic++].GetUInt32();
        }

        if ( !pGText->Text_ID ) continue;
        AddGossipText( pGText );

    } while( result->NextRow() );

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u npc texts", count );
    delete result;
}

//not very fast function but it is called only once a day, or on starting-up
void ObjectMgr::ReturnOrDeleteOldMails(bool serverUp)
{
    time_t basetime = time(NULL);
    sLog.outDebug("Returning mails current time: hour: %d, minute: %d, second: %d ", localtime(&basetime)->tm_hour, localtime(&basetime)->tm_min, localtime(&basetime)->tm_sec);
    //delete all old mails without item and without body immediately, if starting server
    if (!serverUp)
        sDatabase.PExecute("DELETE FROM `mail` WHERE `expire_time` < '" I64FMTD "' AND `item_guid` = '0' AND `itemTextId` = 0", (uint64)basetime);
    QueryResult* result = sDatabase.PQuery("SELECT `id`,`messageType`,`sender`,`receiver`,`itemTextId`,`item_guid`,`expire_time`,`cod`,`checked` FROM `mail` WHERE `expire_time` < '" I64FMTD "'", (uint64)basetime);
    if ( !result )
        return;                                             // any mails need to be returned or deleted
    Field *fields;
    //std::ostringstream delitems, delmails; //will be here for optimization
    //bool deletemail = false, deleteitem = false;
    //delitems << "DELETE FROM `item_instance` WHERE `guid` IN ( ";
    //delmails << "DELETE FROM `mail` WHERE `id` IN ( "
    do
    {
        fields = result->Fetch();
        Mail *m = new Mail;
        m->messageID = fields[0].GetUInt32();
        m->messageType = fields[1].GetUInt8();
        m->sender = fields[2].GetUInt32();
        m->receiver = fields[3].GetUInt32();
        m->itemTextId = fields[4].GetUInt32();
        m->item_guid = fields[5].GetUInt32();
        m->expire_time = (time_t)fields[6].GetUInt64();
        m->deliver_time = 0;
        m->COD = fields[7].GetUInt32();
        m->checked = fields[8].GetUInt32();
        Player *pl = 0;
        if (serverUp)
            pl = objmgr.GetPlayer((uint64)m->receiver);
        if (pl && pl->m_mailsLoaded)
        {                                                   //this code will run very improbably (the time is between 4 and 5 am, in game is online a player, who has old mail
            //his in mailbox and he has already listed his mails )
            delete m;
            continue;
        }
        //delete or return mail:
        if (m->item_guid)
        {
            if (m->checked < 4)
            {
                //mail will be returned:
                sDatabase.PExecute("UPDATE `mail` SET `sender` = '%u', `receiver` = '%u', `expire_time` = '" I64FMTD "', `deliver_time` = '" I64FMTD "',`cod` = '0', `checked` = '16' WHERE `id` = '%u'", m->receiver, m->sender, (uint64)(basetime + 30*DAY), (uint64)basetime, m->messageID);
                delete m;
                continue;
            }
            else
            {
                //deleteitem = true;
                //delitems << m->item_guid << ", ";
                sDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'", m->item_guid);
            }
        }
        if (m->itemTextId)
        {
            sDatabase.PExecute("DELETE FROM `item_text` WHERE `id` = '%u'", m->itemTextId);
        }
        //deletemail = true;
        //delmails << m->messageID << ", ";
        sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", m->messageID);
        delete m;
    } while (result->NextRow());
    delete result;
}

void ObjectMgr::LoadQuestAreaTriggers()
{
    mQuestAreaTriggerMap.clear();                           // need for reload case

    QueryResult *result = sDatabase.Query( "SELECT `id`,`quest` FROM `areatrigger_involvedrelation`" );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u quest trigger points", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 Trigger_ID = fields[0].GetUInt32();
        uint32 Quest_ID   = fields[1].GetUInt32();

        if(!mQuestTemplates[Quest_ID])
        {
            sLog.outErrorDb("Table `areatrigger_involvedrelation` have record (id: %u) for non-existed quest %u",Trigger_ID,Quest_ID);
            continue;
        }

        mQuestAreaTriggerMap[Trigger_ID] = Quest_ID;

    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u quest trigger points", count );
}

void ObjectMgr::LoadTavernAreaTriggers()
{
    mTavernAreaTriggerSet.clear();                          // need for reload case

    QueryResult *result = sDatabase.Query("SELECT `id` FROM `areatrigger_tavern`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u tavern triggers", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 Trigger_ID      = fields[0].GetUInt32();

        if(!GetAreaTrigger(Trigger_ID))
        {
            sLog.outErrorDb("Table `areatrigger_tavern` have record for non-existed area trigger %u",Trigger_ID);
            continue;
        }

        mTavernAreaTriggerSet.insert(Trigger_ID);
    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u tavern triggers", count );
}

uint32 ObjectMgr::GetNearestTaxiNode( float x, float y, float z, uint32 mapid )
{
    bool found = false;
    float dist;
    uint32 id = 0;
    for(uint32 i = 1; i < sTaxiNodesStore.nCount; ++i)
    {
        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
        if(node && node->map_id == mapid)
        {
            float dist2 = (node->x - x)*(node->x - x)+(node->y - y)*(node->y - y)+(node->z - z)*(node->z - z);
            if(found)
            {
                if(dist2 < dist)
                {
                    dist = dist2;
                    id = i;
                }
            }
            else
            {
                found = true;
                dist = dist2;
                id = i;
            }
        }
    }

    return id;
}

void ObjectMgr::GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(source);
    if(src_i==sTaxiPathSetBySource.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    TaxiPathSetForSource& pathSet = src_i->second;

    TaxiPathSetForSource::iterator dest_i = pathSet.find(destination);
    if(dest_i==pathSet.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint16 ObjectMgr::GetTaxiMount( uint32 id, uint32 team )
{
    uint16 mount_id = 0;

    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
    if(node)
    {
        if (team == ALLIANCE)
        {
            CreatureInfo const *ci = objmgr.GetCreatureTemplate(node->alliance_mount_type);
            if(ci)
                mount_id = ci->randomDisplayID();
        }
        if (team == HORDE)
        {
            CreatureInfo const *ci = objmgr.GetCreatureTemplate(node->horde_mount_type);
            if(ci)
                mount_id = ci->randomDisplayID();
        }
    }

    return mount_id;
}

void ObjectMgr::GetTaxiPathNodes( uint32 path, Path &pathnodes )
{
    if(path >= sTaxiPathNodesByPath.size())
        return;

    TaxiPathNodeList& nodeList = sTaxiPathNodesByPath[path];

    pathnodes.Resize(nodeList.size());

    for(size_t i = 0; i < nodeList.size(); ++i)
    {
        pathnodes[ i ].x = nodeList[i].x;
        pathnodes[ i ].y = nodeList[i].y;
        pathnodes[ i ].z = nodeList[i].z;
    }
}

void ObjectMgr::GetTransportPathNodes( uint32 path, TransportPath &pathnodes )
{
    if(path >= sTaxiPathNodesByPath.size())
        return;

    TaxiPathNodeList& nodeList = sTaxiPathNodesByPath[path];

    pathnodes.Resize(nodeList.size());

    for(size_t i = 0; i < nodeList.size(); ++i)
    {
        pathnodes[ i ].mapid = nodeList[i].mapid;
        pathnodes[ i ].x = nodeList[i].x;
        pathnodes[ i ].y = nodeList[i].y;
        pathnodes[ i ].z = nodeList[i].z;
        pathnodes[ i ].actionFlag = nodeList[i].actionFlag;
        pathnodes[ i ].delay = nodeList[i].delay;
    }
}

WorldSafeLocsEntry const *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team)
{
    // search for zone associated closest graveyard
    uint32 zoneId = MapManager::Instance().GetZoneId(MapId,x,y);

    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone,ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check `faction`
    //   if mapId != graveyard.mapId (ghost in instance) and search ANY graveyard associated
    //     then skip check `faction`
    QueryResult *result = sDatabase.PQuery("SELECT `id`,`faction` FROM `game_graveyard_zone` WHERE  `ghost_map` = %u AND `ghost_zone` = %u", MapId, zoneId);

    if(! result)
    {
        sLog.outErrorDb("Table `game_graveyard_zone` incomplete: Zone %u Map %u Team %u not have linked graveyard.",zoneId,MapId,team);
        return NULL;
    }

    bool foundNear = false;
    float distNear;
    WorldSafeLocsEntry const* entryNear = NULL;
    WorldSafeLocsEntry const* entryFar = NULL;

    do
    {
        Field *fields = result->Fetch();
        uint32 g_id   = fields[0].GetUInt32();
        uint32 g_team = fields[1].GetUInt32();

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(g_id);
        if(!entry)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` have record for not existed graveyard (WorldSafeLocs.dbc id) %u, skipped.",g_id);
            continue;
        }

        // remember first graveyard at another map and ignore other
        if(MapId != entry->map_id)
        {
            if(!entryFar)
                entryFar = entry;
            continue;
        }

        // skip enemy faction graveyard at same map (normal area, city, or battleground)
        if(g_team != 0 && g_team != team)
            continue;

        // find now nearest graveyard at same map
        float dist2 = (entry->x - x)*(entry->x - x)+(entry->y - y)*(entry->y - y)+(entry->z - z)*(entry->z - z);
        if(foundNear)
        {
            if(dist2 < distNear)
            {
                distNear = dist2;
                entryNear = entry;
            }
        }
        else
        {
            foundNear = true;
            distNear = dist2;
            entryNear = entry;
        }
    } while( result->NextRow() );

    delete result;

    if(entryNear)
        return entryNear;

    return entryFar;
}

void ObjectMgr::LoadAreaTriggers()
{
    mAreaTriggers.clear();                                  // need for reload case

    uint32 count = 0;

    //                                            0    1                2               3             4                    5                    6                    7            8                   9                   10                  11
    QueryResult *result = sDatabase.Query("SELECT `id`,`required_level`,`required_item`,`trigger_map`,`trigger_position_x`,`trigger_position_y`,`trigger_position_z`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `areatrigger_template`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u area trigger definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        uint32 Trigger_ID = fields[0].GetUInt32();

        AreaTrigger at;

        at.requiredLevel      = fields[1].GetUInt8();
        at.requiredItem       = fields[2].GetUInt32();
        at.trigger_mapId      = fields[3].GetUInt32();
        at.trigger_X          = fields[4].GetFloat();
        at.trigger_Y          = fields[5].GetFloat();
        at.trigger_Z          = fields[6].GetFloat();
        at.target_mapId       = fields[7].GetUInt32();
        at.target_X           = fields[8].GetFloat();
        at.target_Y           = fields[9].GetFloat();
        at.target_Z           = fields[10].GetFloat();
        at.target_Orientation = fields[11].GetFloat();

        if(at.requiredItem)
        {
            ItemPrototype const *pProto = objmgr.GetItemPrototype(at.requiredItem);
            if(!pProto)
            {
                sLog.outError("Key item %u not exist for trigger %u, remove key requirement.", at.requiredItem, Trigger_ID);
                at.requiredItem = 0;
            }
        }

        mAreaTriggers[Trigger_ID] = at;

    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u area trigger definitions", count );
}

void ObjectMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                            0        1          2         3            4          5         6             7                 8
    QueryResult *result = sDatabase.Query("SELECT `entry`,`effectId`,`SpellId`,`SchoolMask`,`Category`,`SkillID`,`SpellFamily`,`SpellFamilyMask`,`Charges` FROM `spell_affect`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u spell affect definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        if (!sSpellStore.LookupEntry(entry))
        {
         sLog.outErrorDb("Spell %u listed in `spell_affect` not exist", entry);
         continue;
        }

        SpellAffection sa;

        sa.SpellId = fields[2].GetUInt16();
        sa.SchoolMask = fields[3].GetUInt8();
        sa.Category = fields[4].GetUInt16();
        sa.SkillId = fields[5].GetUInt16();
        sa.SpellFamily = fields[6].GetUInt8();
        sa.SpellFamilyMask = fields[7].GetUInt64();
        sa.Charges = fields[8].GetUInt16();

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,sa));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell affect definitions", count );
}

bool ObjectMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 const& familyFlags)
{
    if (!spellInfo) 
        return false;

    SpellAffection const *spellAffect = GetSpellAffection(spellId,effectId);

    if (spellAffect )
    {
        if (spellAffect->SpellId && (spellAffect->SpellId == spellInfo->Id))
            return true;
        if (spellAffect->SchoolMask && (spellAffect->SchoolMask & spellInfo->School))
            return true;
        if (spellAffect->Category && (spellAffect->Category == spellInfo->Category))
            return true;
        if (spellAffect->SkillId)
        {
            SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(spellInfo->Id);
            if(skillLineEntry && skillLineEntry->skillId == spellAffect->SkillId)
                return true;
        }
        if (spellAffect->SpellFamily && spellAffect->SpellFamily == spellInfo->SpellFamilyName)
            return true;

        if (spellAffect->SpellFamilyMask && (spellAffect->SpellFamilyMask & spellInfo->SpellFamilyFlags))
            return true;
    }
    else if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    return false;
}


void ObjectMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                            0       1            2          3         4          5      6                 7           8
    QueryResult *result = sDatabase.Query("SELECT `entry`,`SchoolMask`,`Category`,`SkillID`,`SpellFamilyName`,`SpellFamilyMask`,`procFlags`,`ppmRate` FROM `spell_proc_event`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u spell proc event conditions", count  );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();

        if (!sSpellStore.LookupEntry(entry))
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.category        = fields[2].GetUInt32();
        spe.skillId         = fields[3].GetUInt32();
        spe.spellFamilyName = fields[4].GetUInt32();
        spe.spellFamilyMask = fields[5].GetUInt64();
        spe.procFlags       = fields[6].GetUInt32();
        spe.ppmRate         = fields[7].GetFloat();

        mSpellProcEventMap[entry] = spe;

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell proc event conditions", count  );
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult *result = sDatabase.Query( "SELECT MAX(`guid`) FROM `character`" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `creature`" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `item_instance`" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query("SELECT MAX(`guid`) FROM `gameobject`" );
    if( result )
    {
        m_hiGoGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query("SELECT MAX(`id`) FROM `auctionhouse`" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_auctionid = 0;
    }
    result = sDatabase.Query( "SELECT MAX(`id`) FROM `mail`" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_mailid = 0;
    }
    result = sDatabase.Query( "SELECT MAX(`id`) FROM `item_text`" );
    if( result )
    {
        m_ItemTextId = (*result)[0].GetUInt32();

        delete result;
    }
    else
        m_ItemTextId = 0;

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `corpse`" );
    if( result )
    {
        m_hiCorpseGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }
}

uint32 ObjectMgr::GenerateAuctionID()
{
    return ++m_auctionid;
}

uint32 ObjectMgr::GenerateMailID()
{
    return ++m_mailid;
}

uint32 ObjectMgr::GenerateItemTextID()
{
    return ++m_ItemTextId;
}

uint32 ObjectMgr::CreateItemText(std::string text)
{
    uint32 newItemTextId = GenerateItemTextID();
    //insert new itempage to container
    mItemTexts[ newItemTextId ] = text;
    //save new itempage
    sDatabase.escape_string(text);
    //any Delete query needed, itemTextId is maximum of all ids
    std::ostringstream query;
    query << "INSERT INTO `item_text` (`id`,`text`) VALUES ( '" << newItemTextId << "', '" << text << "')";
    sDatabase.Execute(query.str().c_str());                 //needs to be run this way, because mail body may be more than 1024 characters
    return newItemTextId;
}

uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
    switch(guidhigh)
    {
        case HIGHGUID_ITEM          : return ++m_hiItemGuid;
        case HIGHGUID_UNIT          : return ++m_hiCreatureGuid;
        case HIGHGUID_PLAYER        : return ++m_hiCharGuid;
        case HIGHGUID_GAMEOBJECT    : return ++m_hiGoGuid;
        case HIGHGUID_CORPSE        : return ++m_hiCorpseGuid;
        case HIGHGUID_DYNAMICOBJECT : return ++m_hiDoGuid;
        default                     : ASSERT(0);
    }

    return 0;
}

void ObjectMgr::LoadGameobjectInfo()
{
    sGOStorage.Load();

    sLog.outString( ">> Loaded %u game object templates", sGOStorage.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadExplorationBaseXP()
{
    uint32 count = 0;
    QueryResult *result = sDatabase.Query("SELECT `level`,`basexp` FROM `exploration_basexp`");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u BaseXP definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        uint32 level  = fields[0].GetUInt32();
        uint32 basexp = fields[1].GetUInt32();
        mBaseXPTable[level] = basexp;
        ++count;
    }
    while (result->NextRow());

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u BaseXP definitions", count );
}

uint32 ObjectMgr::GetBaseXP(uint32 level)
{
    return mBaseXPTable[level] ? mBaseXPTable[level] : 0;
}

void ObjectMgr::LoadPetNames()
{
    uint32 count = 0;
    QueryResult *result = sDatabase.Query("SELECT `word`,`entry`,`half` FROM `pet_name_generation`");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u pet name parts", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        std::string word = fields[0].GetString();
        uint32 entry     = fields[1].GetUInt32();
        bool   half      = fields[2].GetBool();
        if(half)
            PetHalfName1[entry].push_back(word);
        else
            PetHalfName0[entry].push_back(word);
        count++;
    }
    while (result->NextRow());
    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u pet name parts", count );
}

void ObjectMgr::LoadPetNumber()
{
    QueryResult* result = sDatabase.Query("SELECT MAX(`id`) FROM `character_pet`");
    if(result)
    {
        Field *fields = result->Fetch();
        m_hiPetNumber = fields[0].GetUInt32()+1;
        delete result;
    }
    
    barGoLink bar( 1 );
    bar.step();

    sLog.outString( "" );
    sLog.outString( ">> Loaded the max pet number: %d", m_hiPetNumber-1);
}

std::string ObjectMgr::GeneratePetName(uint32 entry)
{
    std::vector<std::string> & list0 = PetHalfName0[entry];
    std::vector<std::string> & list1 = PetHalfName1[entry];

    if(list0.empty() || list1.empty())
    {
        CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(entry);
        char* petname = GetPetName(cinfo->family, sWorld.GetDBClang());
        if(!petname)
            petname = cinfo->Name;
        return std::string(petname);
    }

    return *(list0.begin()+urand(0, list0.size()-1)) + *(list1.begin()+urand(0, list1.size()-1));
}

uint32 ObjectMgr::GeneratePetNumber()
{
    return ++m_hiPetNumber;
}

bool ObjectMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2)
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool ObjectMgr::canStackSpellRanks(SpellEntry const *spellInfo,SpellEntry const *spellInfo2)
{
    if(spellInfo->powerType == 0)
    {
        if(spellInfo->manaCost > 0 && spellInfo->manaCost != spellInfo2->manaCost)
            return true;
        if(spellInfo->ManaCostPercentage > 0 && spellInfo->ManaCostPercentage != spellInfo2->ManaCostPercentage)
            return true;
        if(spellInfo->manaCostPerlevel > 0 && spellInfo->manaCostPerlevel != spellInfo2->manaCostPerlevel)
            return true;
        if(spellInfo->manaPerSecond > 0 && spellInfo->manaPerSecond != spellInfo2->manaPerSecond)
            return true;
        if(spellInfo->manaPerSecondPerLevel > 0 && spellInfo->manaPerSecondPerLevel != spellInfo2->manaPerSecondPerLevel)
            return true;
    }
    return false;
}

void ObjectMgr::LoadCorpses()
{
    uint32 count = 0;
    //                                             0            1            2            3             4     5      6            7          8        9
    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance`,`player`,`guid` FROM `corpse` WHERE `bones_flag` = 0");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u corpses", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    Corpse *corpse;
    do
    {
        bar.step();

        Field *fields = result->Fetch();

        uint32 guid = fields[result->GetFieldCount()-1].GetUInt32();

        corpse = new Corpse(NULL);
        if(!corpse->LoadFromDB(guid,fields))
        {
            delete corpse;
            continue;
        }

        ObjectAccessor::Instance().AddCorpse(corpse);

        count++;
    }
    while (result->NextRow());
    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u corpses", count );
}

bool ObjectMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2)
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);

    if(!spellInfo_1 || !spellInfo_2)
        return false;

    if(spellInfo_1->Id == spellId_2)
        return false;

    //I think we don't check this correctly because i need a exception for spell:
    //72,11327,18461...(called from 1856,1857...) Call Aura 16,31, after trigger another spell who call aura 77 and 77 remove 16 and 31, this should not happen.
    if(spellInfo_2->SpellFamilyFlags == 2048)
        return false;

    // Paladin Seals
    if( IsSealSpell(spellId_1) && IsSealSpell(spellId_2) )
        return true;

    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER || 
                spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER || 
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER )
                isModifier = true;
        }
        if (!isModifier)
            return true;
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
        return true;

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    for (int i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
        spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
        spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
        spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

    return true;
}

bool ObjectMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionSkill(skill);
}

bool ObjectMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsPrimaryProfessionSkill(skill);
}

bool ObjectMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId)
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

void ObjectMgr::LoadReputationOnKill()
{
    uint32 count = 0; 

    //                                             0             1                      2
    QueryResult *result = sDatabase.Query("SELECT `creature_id`,`RewOnKillRepFaction1`,`RewOnKillRepFaction2`,"
        //3              4              5                    6              7              8                   9
        "`IsTeamAward1`,`MaxStanding1`,`RewOnKillRepValue1`,`IsTeamAward2`,`MaxStanding2`,`RewOnKillRepValue2`,`TeamDependent` "
        "FROM `creature_onkill_reputation`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 creature award reputation definitions. DB table `creature_onkill_reputation` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 creature_id = fields[0].GetUInt32();

        ReputationOnKillEntry repOnKill;
        repOnKill.repfaction1          = fields[1].GetUInt32();
        repOnKill.repfaction2          = fields[2].GetUInt32();
        repOnKill.is_teamaward1        = fields[3].GetUInt8();
        repOnKill.reputration_max_cap1 = fields[4].GetUInt32();
        repOnKill.repvalue1            = fields[5].GetInt32();
        repOnKill.is_teamaward2        = fields[6].GetUInt8();
        repOnKill.reputration_max_cap2 = fields[7].GetUInt32();
        repOnKill.repvalue2            = fields[8].GetInt32();
        repOnKill.team_dependent       = fields[9].GetUInt8();

        if(repOnKill.repfaction1)
        {
            FactionEntry const *factionEntry1 = sFactionStore.LookupEntry(repOnKill.repfaction1); 
            if(!factionEntry1)
            {
                sLog.outErrorDb("Faction (faction.dbc) %u not existed but used in `creature_onkill_reputation`",repOnKill.repfaction1);
                continue;
            }
        }

        if(repOnKill.repfaction2)
        {
            FactionEntry const *factionEntry2 = sFactionStore.LookupEntry(repOnKill.repfaction2); 
            if(!factionEntry2)
            {
                sLog.outErrorDb("Faction (faction.dbc) %u not existed but used in `creature_onkill_reputation`",repOnKill.repfaction2);
                continue;
            }
        }

        mRepOnKill[creature_id] = repOnKill;

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString("");
    sLog.outString(">> Loaded %u creature award reputation definitions", count);
}

void ObjectMgr::CleanupInstances()
{
    // this routine cleans up old instances from all the tables before server start

    uint32 okcount = 0;
    uint32 delcount = 0;

    QueryResult *result;

    // first, obtain total instance set
    std::set< uint32 > InstanceSet;

    // creature_respawn
    result = sDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `creature_respawn` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // gameobject_respawn
    result = sDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `gameobject_respawn` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // character_instance
    result = sDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `character_instance`");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // instance
    result = sDatabase.PQuery("SELECT `id` FROM `instance`");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // now remove all valid instances from instance list (it will become list of instances to delete)
    // instances considered valid:
    //   1) reset time > current time
    //   2) bound to at least one character (id is found in `character_instance` table)
    result = sDatabase.PQuery("SELECT DISTINCT(`instance`.`id`) AS `id` FROM `instance` LEFT JOIN `character_instance` ON (`character_instance`.`instance` = `instance`.`id`) WHERE (`instance`.`id` = `character_instance`.`instance`) AND (`instance`.`resettime` > " I64FMTD ")", (uint64)time(NULL));
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            if (InstanceSet.find(fields[0].GetUInt32()) != InstanceSet.end())
            {
                InstanceSet.erase(fields[0].GetUInt32());
                okcount++;
            }
        }
        while (result->NextRow());
        delete result;
    }

    delcount = InstanceSet.size();

    barGoLink bar( delcount + 1 );
    bar.step();

    // remove all old instances from tables
    for (std::set< uint32 >::iterator i = InstanceSet.begin(); i != InstanceSet.end(); i++)
    {
        sDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `instance` = '%u'", *i);
        sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `instance` = '%u'", *i);
        sDatabase.PExecute("DELETE FROM `corpse` WHERE `instance` = '%u'", *i);
        sDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'", *i);

        bar.step();
    }

    sLog.outString( "" );
    sLog.outString( ">> Initialized %u instances, deleted %u old instances", okcount, delcount );
}

void ObjectMgr::PackInstances()
{
    // this routine renumbers player instance associations in such a way so they start from 1 and go up

    // obtain set of all associations
    std::set< uint32 > InstanceSet;

    // the check in query allows us to prevent table destruction in case of a bug we must never encounter
    QueryResult *result = sDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `character_instance` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    barGoLink bar( InstanceSet.size() + 1);
    bar.step();

    uint32 InstanceNumber = 1;

    // we do assume std::set is sorted properly on integer value
    for (std::set< uint32 >::iterator i = InstanceSet.begin(); i != InstanceSet.end(); i++)
    {
        if (*i != InstanceNumber)
        {
            // remap instance id
            sDatabase.PExecute("UPDATE `creature_respawn` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            sDatabase.PExecute("UPDATE `gameobject_respawn` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            sDatabase.PExecute("UPDATE `corpse` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            sDatabase.PExecute("UPDATE `character_instance` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            sDatabase.PExecute("UPDATE `instance` SET `id` = '%u' WHERE `id` = '%u'", InstanceNumber, *i);
        }

        InstanceNumber++;
        bar.step();
    }

    sLog.outString( "" );
    sLog.outString( ">> Instance numbers remapped, next instance id is %u", InstanceNumber );
}

void ObjectMgr::LoadWeatherZoneChances()
{
    uint32 count = 0; 

    //                                             0      1                    2                    3                     4                    5                    6                     7                  8                  9                   10                   11                   12
    QueryResult *result = sDatabase.Query("SELECT `zone`,`spring_rain_chance`,`spring_snow_chance`,`spring_storm_chance`,`summer_rain_chance`,`summer_snow_chance`,`summer_storm_chance`,`fall_rain_chance`,`fall_snow_chance`,`fall_storm_chance`,`winter_rain_chance`,`winter_snow_chance`,`winter_storm_chance` FROM `game_weather`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 weather definitions. DB table `game_weather` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 zone_id = fields[0].GetUInt32();

        WeatherZoneChances& wzc = mWeatherZoneMap[zone_id];

        for(int season = 0; season < WEATHER_SEASONS; ++season)
        {
            wzc.data[season].rainChance  = fields[season * (MAX_WEATHER_TYPE-1) + 1].GetUInt32();
            wzc.data[season].snowChance  = fields[season * (MAX_WEATHER_TYPE-1) + 2].GetUInt32();
            wzc.data[season].stormChance = fields[season * (MAX_WEATHER_TYPE-1) + 3].GetUInt32();

            if(wzc.data[season].rainChance > 100)
            {
                wzc.data[season].rainChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u have wrong rain chance > 100%",zone_id,season);
            }

            if(wzc.data[season].snowChance > 100)
            {
                wzc.data[season].snowChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u have wrong snow chance > 100%",zone_id,season);
            }


            if(wzc.data[season].stormChance > 100)
            {
                wzc.data[season].stormChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u have wrong storm chance > 100%",zone_id,season);
            }
        }

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString("");
    sLog.outString(">> Loaded %u weather definitions", count);
}

void ObjectMgr::SaveCreatureRespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    mCreatureRespawnTimes[MAKE_GUID(loguid,instance)] = t;
    sDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", loguid, instance);
    if(t)
        sDatabase.PExecute("INSERT INTO `creature_respawn` VALUES ( '%u', '" I64FMTD "', '%u' )", loguid, uint64(t), instance);
}

void ObjectMgr::DeleteCreatureData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    CreatureData const* data = GetCreatureData(guid);
    if(data)
    {
        CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
        uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

        CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
        cell_guids.creatures.erase(guid);
    }

    mCreatureDataMap.erase(guid);
}

void ObjectMgr::SaveGORespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    mGORespawnTimes[MAKE_GUID(loguid,instance)] = t;
    sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", loguid, instance);
    if(t)
        sDatabase.PExecute("INSERT INTO `gameobject_respawn` VALUES ( '%u', '" I64FMTD "', '%u' )", loguid, uint64(t), instance);
}

void ObjectMgr::DeleteRespawnTimeForInstance(uint32 instance)
{
    RespawnTimes::iterator next;

    for(RespawnTimes::iterator itr = mGORespawnTimes.begin(); itr != mGORespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mGORespawnTimes.erase(itr);
    }

    for(RespawnTimes::iterator itr = mCreatureRespawnTimes.begin(); itr != mCreatureRespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mCreatureRespawnTimes.erase(itr);
    }

    sDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `instance` = '%u'", instance);
    sDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `instance` = '%u'", instance);
}

void ObjectMgr::DeleteGOData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    GameObjectData const* data = GetGOData(guid);
    if(data)
    {
        CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
        uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

        CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
        cell_guids.gameobjects.erase(guid);
    }

    mGameObjectDataMap.erase(guid);
}

void ObjectMgr::AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance)
{
    CellObjectGuids& cell_guids = mMapObjectGuids[mapid][cellid];
    cell_guids.corpses[player_guid] = instance;
}

void ObjectMgr::DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid)
{
    CellObjectGuids& cell_guids = mMapObjectGuids[mapid][cellid];
    cell_guids.corpses.erase(player_guid);
}

void ObjectMgr::LoadQuestRelationsHelper(QuestRelations& map,char const* table)
{
    map.clear();                                            // need for reload case

    uint32 count = 0; 

    QueryResult *result = sDatabase.PQuery("SELECT `id`,`quest` FROM `%s`",table);

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 quest relations from %s. DB table `%s` is empty.",table,table);
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 id    = fields[0].GetUInt32();
        uint32 quest = fields[1].GetUInt32();

        if ( !mQuestTemplates[quest] )
        {
            sLog.outErrorDb("Table %s: Quest %u listed for entry %u not exist.",table,quest,id);
            continue;
        }

        map.insert(QuestRelations::value_type(id,quest));

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString("");
    sLog.outString(">> Loaded %u quest relations from %s", count,table);
}

// Character Dumps (Write/Load)

#define NUM_TBLS 13

struct tbl
{
    char name[30];
    uint8 type;
};

tbl tbls[NUM_TBLS] = {
    { "character", 1 },
    { "character_action", 0 },
    { "character_aura", 0 },
    { "character_homebind", 0 },
    { "character_inventory", 2 },
    { "character_kill", 0 },
    { "character_queststatus", 0 },
    { "character_reputation", 0 },
    { "character_spell", 0 },
    { "character_spell_cooldown", 0 },
    { "item_instance", 3 },
    { "character_ticket", 0 },
    { "character_tutorial", 0 }
};

std::string CreateDumpString(char *table, QueryResult *result)
{
    if(!table || !result) return "";
    std::ostringstream ss;
    ss << "INSERT INTO `" << table << "` VALUES (";
    Field *fields = result->Fetch();
    for(uint32 i = 0; i < result->GetFieldCount(); i++)
    {
        if (i == 0) ss << "'";
        else ss << ", '";

        std::string s = fields[i].GetCppString();
        sDatabase.escape_string(s);
        ss << s;

        ss << "'";
    }
    ss << ");";
    return ss.str();
}

bool DumpPlayerTable(FILE *file, uint32 guid, char *tableFrom, char *tableTo, int8 type = 0)
{
    if (!file || !tableFrom || !tableTo) return false;
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `%s` WHERE `%s` = '%d'", tableFrom, type != 3 ? "guid" : "owner_guid", guid);
    if (!result) return false;
    do
    {
        std::string dump = CreateDumpString(tableTo, result);
        fprintf(file, "%s\n", dump.c_str());
    }
    while (result->NextRow());
    delete result;
    return true;
}

bool ObjectMgr::WritePlayerDump(std::string file, uint32 guid)
{
    FILE *fout = fopen(file.c_str(), "w");
    if (!fout) { sLog.outError("Failed to open file!\r\n"); return false; }

    for(int i = 0; i < NUM_TBLS; i++)
        DumpPlayerTable(fout, guid, tbls[i].name, tbls[i].name, tbls[i].type);

    // TODO: Add pets/instance/group/gifts..
    // TODO: Add a dump level option to skip some non-important tables
    
    fclose(fout);
    return true;
}

bool findnth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    s = str.find("VALUES ('")+9;
    if (s == std::string::npos) return false;

    do {
        e = str.find("'",s);
        if (e == std::string::npos) return false;
    } while(str[e-1] == '\\');

    for(int i = 1; i < n; i++)
    {
        do {
            s = e+4;
            e = str.find("'",s);
            if (e == std::string::npos) return false;
        } while (str[e-1] == '\\');
    }
    return true;
}

std::string gettablename(std::string &str)
{
    std::string::size_type s = 13, e = str.find('`', s);
    if (e == std::string::npos) return "";
    return str.substr(s, e-s);
}

bool changenth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s, e;
    if(!findnth(str,n,s,e)) return false;
    if(nonzero && str.substr(s,e-s) == "0") return true;    // not an error
    if(!insert) str.replace(s,e-s, with);
    else str.insert(s, with);
    return true;
}

std::string getnth(std::string &str, int n)
{
    std::string::size_type s, e;
    if(!findnth(str,n,s,e)) return "";
    return str.substr(s, e-s);
}

bool findtoknth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    int i; s = e = 0;
    std::string::size_type size = str.size();
    for(i = 1; s < size && i < n; s++) if(str[s] == ' ') i++;
    if (i < n) return false;
    e = str.find(' ', s);
    return e != std::string::npos;
}

std::string gettoknth(std::string &str, int n)
{
    std::string::size_type s = 0, e = 0;
    if(!findtoknth(str, n, s, e)) return "";
    return str.substr(s, e-s);
}

bool changetoknth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s = 0, e = 0;
    if(!findtoknth(str, n, s, e)) return false;
    if(nonzero && str.substr(s,e-s) == "0") return true;    // not an error
    if(!insert) str.replace(s, e-s, with);
    else str.insert(s, with);
    return true;
}

uint32 registerItem(uint32 oldGuid, std::map<uint32, uint32> &items, uint32 hiGuid)
{
    if(items[oldGuid] == 0) items[oldGuid] = hiGuid + items.size();
    return items[oldGuid];
}

bool changeItem(std::string &str, int n, std::map<uint32, uint32> &items, uint32 hiGuid, bool nonzero = false)
{
    char chritem[20];
    uint32 oldGuid = atoi(getnth(str, n).c_str());
    if (nonzero && oldGuid == 0) return true;               // not an error
    uint32 newGuid = registerItem(oldGuid, items, hiGuid);
    snprintf(chritem, 20, "%d", newGuid);
    return changenth(str, n, chritem, false, nonzero);
}

bool changetokItem(std::string &str, int n, std::map<uint32, uint32> &items, uint32 hiGuid, bool nonzero = false)
{
    char chritem[20];
    uint32 oldGuid = atoi(gettoknth(str, n).c_str());
    if (nonzero && oldGuid == 0) return true;               // not an error
    uint32 newGuid = registerItem(oldGuid, items, hiGuid);
    snprintf(chritem, 20, "%d", newGuid);
    return changetoknth(str, n, chritem, false, nonzero);
}

#define ROLLBACK {sDatabase.RollbackTransaction(); fclose(fin); return false;}

extern std::string notAllowedChars;

bool ObjectMgr::LoadPlayerDump(std::string file, uint32 account, std::string name, uint32 guid)
{
    FILE *fin = fopen(file.c_str(), "r");
    if(!fin) return false;

    QueryResult * result = NULL;
    char newguid[20], chraccount[20];

    // make sure the same guid doesn't already exist and is safe to use
    bool incHighest = true;
    if(guid != 0 && guid < m_hiCharGuid)
    {
        result = sDatabase.PQuery("SELECT * FROM `character` WHERE `guid` = '%d'", guid);
        if (result)
        {
            guid = m_hiCharGuid;                            // use first free if exists
            delete result;
        }
        else incHighest = false;
    }
    else guid = m_hiCharGuid;

    // normalize the name if specified and check if it exists
    if(name != "" && name.find_first_of(notAllowedChars) == name.npos)
    {
        sDatabase.escape_string(name);
        normalizePlayerName(name);
        result = sDatabase.PQuery("SELECT * FROM `character` WHERE `name` = '%s'", name.c_str());
        if (result)
        {
            name = "";                                      // use the one from the dump
            delete result;
        }
    }
    else name = "";

    snprintf(newguid, 20, "%d", guid);
    snprintf(chraccount, 20, "%d", account);

    std::map<uint32, uint32> items;
    char buf[32000] = "";

    sDatabase.BeginTransaction();
    while(!feof(fin))
    {
        if(!fgets(buf, 32000, fin))
        {
            if(feof(fin)) break;
            sLog.outError("LoadPlayerDump: File read error!");
            ROLLBACK;
        }

        std::string line; line.assign(buf);

        // determine table name and load type
        std::string tn = gettablename(line);
        uint8 type, i;
        for(i = 0; i < NUM_TBLS; i++)
            if (tn == tbls[i].name) { type = tbls[i].type; break; }
        if (i == NUM_TBLS)
        {
            sLog.outError("LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            ROLLBACK;
        }
        
        // change the data to server values
        if(type <= 2)                                       // most tables
            if(!changenth(line, 1, newguid)) ROLLBACK;
        if(type == 1)                                       // character t.
        {
            // guid, data field:guid, items
            if(!changenth(line, 2, chraccount)) ROLLBACK;
            std::string vals = getnth(line, 3);
            if(!changetoknth(vals, OBJECT_FIELD_GUID+1, newguid)) ROLLBACK;
            for(uint16 field = PLAYER_FIELD_INV_SLOT_HEAD; field < PLAYER_FARSIGHT; field++)
                if(!changetokItem(vals, field+1, items, m_hiItemGuid, true)) ROLLBACK;
            if(!changenth(line, 3, vals.c_str())) ROLLBACK;
            if (name == "")
            {
                // check if the original name already exists
                name = getnth(line, 4);
                result = sDatabase.PQuery("SELECT * FROM `character` WHERE `name` = '%s'", name.c_str());
                if (result)
                {
                    delete result;
                    if(!changenth(line, 29, "1")) ROLLBACK;     // rename on login
                }
            }
            else if(!changenth(line, 4, name.c_str())) ROLLBACK;
        }
        else if(type == 2)                                  // character_inventory t.
        {
            // bag, item
            if(!changeItem(line, 2, items, m_hiItemGuid, true)) ROLLBACK;
            if(!changeItem(line, 4, items, m_hiItemGuid)) ROLLBACK;
        }
        else if(type == 3)                                  // item_instance t.
        {
            // item, owner, data field:item, owner guid
            if(!changeItem(line, 1, items, m_hiItemGuid)) ROLLBACK;
            if(!changenth(line, 2, newguid)) ROLLBACK;
            std::string vals = getnth(line,3);
            if(!changetokItem(vals, OBJECT_FIELD_GUID+1, items, m_hiItemGuid)) ROLLBACK;
            if(!changetoknth(vals, ITEM_FIELD_OWNER+1, newguid)) ROLLBACK;
            if(!changenth(line, 3, vals.c_str())) ROLLBACK;
        }

        if(!sDatabase.Execute(line.c_str())) ROLLBACK;
    }
    sDatabase.CommitTransaction();
    m_hiItemGuid += items.size();
    if(incHighest) m_hiCharGuid++;
    fclose(fin);
    return true;
}
