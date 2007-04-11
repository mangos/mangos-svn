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
#include "Transports.h"
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

ScriptMapMap sScripts;
ScriptMapMap sSpellScripts;

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid = 1;
    m_hiCreatureGuid = 1;
    m_hiItemGuid = 1;
    m_hiGoGuid = 1;
    m_hiDoGuid = 1;
    m_hiCorpseGuid=1;
}

ObjectMgr::~ObjectMgr()
{

    for( QuestMap::iterator i = QuestTemplates.begin( ); i != QuestTemplates.end( ); ++ i )
    {
        delete i->second;
    }
    QuestTemplates.clear( );

    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    for( AreaTriggerMap::iterator i = mAreaTriggerMap.begin( ); i != mAreaTriggerMap.end( ); ++ i )
    {
        delete i->second;
    }
    mAreaTriggerMap.clear( );

    for( TeleportMap::iterator i = mTeleports.begin( ); i != mTeleports.end( ); ++ i )
    {
        delete i->second;
    }
    mTeleports.clear();

    for(PetLevelInfoMap::iterator i = petInfo.begin( ); i != petInfo.end( ); ++ i )
    {
        delete[] i->second;
    }
    petInfo.clear();

    for (int race = 0; race < MAX_RACES; ++race)
    {
        for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
        {
            delete[] playerInfo[race][class_].levelInfo;
        }
        delete[] playerInfo[race];
    }
    delete[] playerInfo;

    mRepOnKill.clear();
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
        return (uint32) (0.15 * highBid);
    else
        return (uint32) (0.05 * highBid);
}

uint32 ObjectMgr::GetAuctionDeposit(uint32 location, uint32 time, Item *pItem)
{
    uint32 percentance;
    if ( location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        percentance = 25;
    else
        percentance = 5;
    return (uint32) ( ((percentance * pItem->GetProto()->SellPrice * pItem->GetCount() ) / 100 ) * (time / 120 ) );
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
    msgAuctionWonBody << right << hex << auction->owner;
    msgAuctionWonBody << dec << ":" << auction->bid << ":" << auction->buyout;
    sLog.outDebug( "AuctionWon body string : %s", msgAuctionWonBody.str().c_str() );

    //prepare mail data... :
    uint32 mailId = this->GenerateMailID();
    uint32 itemTextId = this->CreateItemText( msgAuctionWonBody.str() );
    time_t etime = time(NULL) + (30 * DAY);

    uint64 bidder_guid = MAKE_GUID(auction->bidder,HIGHGUID_PLAYER);
    Player *bidder = objmgr.GetPlayer(bidder_guid);

    // data for gm.log
    if( sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
    {
        uint32 accid = 0;
        uint32 security = 0;
        std::string name;
        if (bidder)
        {
            accid = bidder->GetSession()->GetAccountId();
            security = bidder->GetSession()->GetSecurity();
            name = bidder->GetName();
        }
        else
        {
            security = GetPlayerAccountIdByGUID(bidder_guid);
            
            if(security > 0)                                // not do redundant DB requests
            {
                GetPlayerNameByGUID(bidder_guid,name);
                accid = GetPlayerAccountIdByGUID(bidder_guid);
            }
        }

        if( security > 0 )
            sLog.outCommand("GM auction, won item: %s (Entry: %u Count: %u) and pay money: %u GM: %s (Account: %u)",
                pItem->GetProto()->Name1,pItem->GetEntry(),pItem->GetCount(),auction->bid,name.c_str(),accid);
    }

    if (bidder)
    {
        bidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, bidder_guid, 0, 0, auction->item_template);

        bidder->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionWonSubject.str(), itemTextId, auction->item_guid, auction->item_template, etime, 0, 0, AUCTION_CHECKED, pItem);
    }
    else
        delete pItem;

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "', '0', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->bidder, msgAuctionWonSubject.str().c_str(), itemTextId, auction->item_guid, auction->item_template, (uint64)etime, AUCTION_CHECKED);
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
    auctionSuccessfulBody << right << hex << auction->bidder;
    auctionSuccessfulBody << dec << ":" << auction->bid << ":0:";
    auctionSuccessfulBody << auction->deposit << ":" << auctionCut;
    sLog.outDebug("AuctionSuccessful body string : %s", auctionSuccessfulBody.str().c_str());

    uint32 itemTextId = this->CreateItemText( auctionSuccessfulBody.str() );

    uint32 mailId = this->GenerateMailID();
    time_t etime = time(NULL) + (30 * DAY);
    uint32 profit = auction->bid + auction->deposit - auctionCut;

    Player *owner = objmgr.GetPlayer((uint64) auction->owner);
    if (owner)
    {
        //send auctionowner notification, bidder must be current!
        owner->GetSession()->SendAuctionOwnerNotification( auction );

        owner->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionSuccessfulSubject.str(), itemTextId, 0, 0, etime, profit, 0, AUCTION_CHECKED, NULL);
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '0', '0', '" I64FMTD "', '%u', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->owner, msgAuctionSuccessfulSubject.str().c_str(), itemTextId, (uint64)etime, profit, AUCTION_CHECKED);
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
        time_t etime = time(NULL) + 30 * DAY;

        sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
            "VALUES ('%u', '2', '%u', '%u', '%s', '0', '%u', '%u', '" I64FMTD "', '0', '0', '0')",
            messageId, auction->location, auction->owner, subject.str().c_str(), auction->item_guid, auction->item_template, (uint64)etime );
        if ( seller )
        {
            seller->GetSession()->SendAuctionOwnerNotification( auction );

            seller->CreateMail(messageId, AUCTIONHOUSE_MAIL, auction->location, subject.str(), 0, auction->item_guid, auction->item_template, etime,0,0,NOT_READ,pItem);
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

void ObjectMgr::LoadSpellProcEvents()
{
    sSpellProcEventStore.Load();

    sLog.outString( ">> Loaded %u spell proc event conditions", sSpellProcEventStore.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::LoadSpellThreats()
{
    sSpellThreatStore.Load();

    sLog.outString( ">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount );
    sLog.outString( "" );
}

// name must be checked to correctness (if received) before call this function
uint64 ObjectMgr::GetPlayerGUIDByName(const char *name) const
{

    uint64 guid = 0;

    // Player name safe to sending to DB (checked at login) and this function using
    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name);

    if(result)
    {
        guid = MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER);

        delete result;
    }

    return guid;
}

bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{

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
        aItem->outBid = ( aItem->bid > 0 );                 //when bid = 0 then 0 else 1
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
        //                                              0               1       2    3      4     5     6     7     8
        QueryResult *result  = sDatabase.Query("SELECT `creature_entry`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi` FROM `pet_levelstats`");

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
                if(current_level > 255)                     // harcoded level maximum
                    sLog.outErrorDb("Wrong (> 255) level %u in `pet_levelstats` table, ignoring.",current_level);
                else
                    sLog.outDetail("Unused (> MaxPlayerLevel in mangosd.conf) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }

            PetLevelInfo*& pInfoMapEntry = petInfo[creature_id];

            if(pInfoMapEntry==NULL)
                pInfoMapEntry =  new PetLevelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            // data for level 1 stored in [0] array element, ...
            PetLevelInfo* pLevelInfo = &pInfoMapEntry[current_level-1];

            pLevelInfo->health = fields[2].GetUInt16();
            pLevelInfo->mana   = fields[3].GetUInt16();

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
        //                                            0      1       2     3      4            5            6            7
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`map`,`zone`,`position_x`,`position_y`,`position_z`,`displayID` FROM `playercreateinfo`");

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
            pInfo->displayId = fields[7].GetUInt16();

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
            uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();

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

    // Load playercreate skills
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
                if(current_level > 255)                     // harcoded level maximum
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
            if(!pInfo->displayId)
                continue;

            // fatal error if no level 1 data
            if(!pInfo->levelInfo || pInfo->levelInfo[0].health == 0 )
            {
                sLog.outErrorDb("Race %i Class %i Level 1 not have stats data!",race,class_);
                exit(1);
            }

            // fill level gaps
            for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
            {
                if(pInfo->levelInfo[level].health == 0)
                {
                    sLog.outErrorDb("Race %i Class %i Level %i not have stats data, using data for %i.",race,class_,level+1, level);
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
    if(pInfo->displayId==0) return;

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
        newguild->LoadGuildFromDB(fields[0].GetUInt32());
        AddGuild(newguild);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u guild definitions", count );
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
        group->LoadGroupFromDB(MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER));
        AddGroup(group);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u group definitions", count );
}

void ObjectMgr::LoadQuests()
{
    //                                            0       1        2           3          4            5
    QueryResult *result = sDatabase.Query("SELECT `entry`,`ZoneId`,`QuestSort`,`MinLevel`,`QuestLevel`,`Type`,"
    //   6               7               8               9                    10                   11
        "`RequiredRaces`,`RequiredClass`,`RequiredSkill`,`RequiredSkillValue`,`RequiredRepFaction`,`RequiredRepValue`,"
    //   12          13             14            15            16               17                 18          19             20
        "`LimitTime`,`SpecialFlags`,`PrevQuestId`,`NextQuestId`,`ExclusiveGroup`,`NextQuestInChain`,`SrcItemId`,`SrcItemCount`,`SrcSpell`,"
    //   21      22        23           24                25                 26        27               28               29               30
        "`Title`,`Details`,`Objectives`,`OfferRewardText`,`RequestItemsText`,`EndText`,`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,"
    //   31           32           33           34           35              36              37              38
        "`ReqItemId1`,`ReqItemId2`,`ReqItemId3`,`ReqItemId4`,`ReqItemCount1`,`ReqItemCount2`,`ReqItemCount3`,`ReqItemCount4`,"
    //   39             40             41             42             43                44                45                46                47              48              49              50
        "`ReqSourceId1`,`ReqSourceId2`,`ReqSourceId3`,`ReqSourceId4`,`ReqSourceCount1`,`ReqSourceCount2`,`ReqSourceCount3`,`ReqSourceCount4`,`ReqSourceRef1`,`ReqSourceRef2`,`ReqSourceRef3`,`ReqSourceRef4`,"
    //   51                   52                   53                   54                   55                      56                      57                      58
        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOId2`,`ReqCreatureOrGOId3`,`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount1`,`ReqCreatureOrGOCount2`,`ReqCreatureOrGOCount3`,`ReqCreatureOrGOCount4`,"
    //   59              60              61              62
        "`ReqSpellCast1`,`ReqSpellCast2`,`ReqSpellCast3`,`ReqSpellCast4`,"
    //   63                 64                 65                 66                 67                 68
        "`RewChoiceItemId1`,`RewChoiceItemId2`,`RewChoiceItemId3`,`RewChoiceItemId4`,`RewChoiceItemId5`,`RewChoiceItemId6`,"
    //   69                    70                    71                    72                    73                    74
        "`RewChoiceItemCount1`,`RewChoiceItemCount2`,`RewChoiceItemCount3`,`RewChoiceItemCount4`,`RewChoiceItemCount5`,`RewChoiceItemCount6`,"
    //   75           76           77           78           79              80              81              82
        "`RewItemId1`,`RewItemId2`,`RewItemId3`,`RewItemId4`,`RewItemCount1`,`RewItemCount2`,`RewItemCount3`,`RewItemCount4`,"
    //   83               84               85               86               87               88             89             90             91             92
        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
    //   93              94      95         96           97       98       99         100                101                 102              103
        "`RewOrReqMoney`,`RewXP`,`RewSpell`,`PointMapId`,`PointX`,`PointY`,`PointOpt`,`OfferRewardEmote`,`RequestItemsEmote`,`CompleteScript`,`Repeatable`"
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
    // some quests can have many previous maps setted by NextQuestId in previouse quest
    // for example set of race quests can lead to single not race specific quest
    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        Quest * newQuest = new Quest(fields);
        QuestTemplates[newQuest->GetQuestId()] = newQuest;
    } while( result->NextRow() );

    delete result;

    // Post processing
    for (QuestMap::iterator iter = QuestTemplates.begin(); iter != QuestTemplates.end(); iter++)
    {
        Quest * qinfo = iter->second;

        // additional quest integrity checks (GO, creature_template and item_template must be loaded already)
        if(qinfo->RequiredSkill && !sSkillLineStore.LookupEntry(qinfo->RequiredSkill))
        {
            sLog.outErrorDb("Quest %u has `RequiredSkill` = %u but skill %u doesn't exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredSkill,qinfo->RequiredSkill);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredSkillValue && qinfo->RequiredSkillValue > sWorld.GetConfigMaxSkillValue())
        {
            sLog.outErrorDb("Quest %u has `RequiredSkillValue` = %u but max possible skill is %u, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredSkillValue,sWorld.GetConfigMaxSkillValue());
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredRepFaction && !sFactionTemplateStore.LookupEntry(qinfo->RequiredRepFaction))
        {
            sLog.outErrorDb("Quest %u has `RequiredRepFaction` = %u but faction template %u doesn't exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredRepFaction,qinfo->RequiredRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredRepValue && qinfo->RequiredRepValue > Player::Reputation_Cap)
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
            if(id && !sItemStorage.LookupEntry<ItemPrototype>(id))
            {
                sLog.outErrorDb("Quest %u has `ReqItemId%d` = %u but item with entry %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,id);
                // no changes, quest can't be done for this requirement
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
            if(QuestTemplates.find(qinfo->NextQuestInChain) == QuestTemplates.end())
            {
                sLog.outErrorDb("Quest %u has `NextQuestInChain` = %u but quest %u doesn't exist, quest chain will not work.",
                    qinfo->GetQuestId(),qinfo->NextQuestInChain ,qinfo->NextQuestInChain );
                qinfo->NextQuestInChain = 0;
            }
            else
                QuestTemplates[qinfo->NextQuestInChain]->prevChainQuests.push_back(qinfo->GetQuestId());
        }

        // fill additional data stores
        if(qinfo->PrevQuestId)
            qinfo->prevQuests.push_back(qinfo->PrevQuestId);

        if(qinfo->NextQuestId)
        {
            if (QuestTemplates.find(abs(qinfo->GetNextQuestId())) == QuestTemplates.end())
            {
                sLog.outErrorDb("Quest %d has NextQuestId %i, but no such quest", qinfo->GetQuestId(), qinfo->GetNextQuestId());
            }
            else
            {
                int32 signedQuestId = qinfo->NextQuestId < 0 ? -int32(qinfo->GetQuestId()) : int32(qinfo->GetQuestId());
                QuestTemplates[abs(qinfo->GetNextQuestId())]->prevQuests.push_back(signedQuestId);
            }
        }

        if(qinfo->ExclusiveGroup)
            ExclusiveQuestGroups.insert(pair<uint32, uint32>(qinfo->ExclusiveGroup, qinfo->GetQuestId()));
    }

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u quests definitions", QuestTemplates.size() );
}

void ObjectMgr::LoadSpellChains()
{
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
            sLog.outErrorDb("Spell %u listed in `spell_chain` have not compatiable chain data (prev: %u, first: %u, rank: %d)",spell_id,node.prev,node.first,node.rank);
            continue;
        }

        SpellChains[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    // additinal integrity checks
    for(SpellChainMap::iterator i = SpellChains.begin(); i != SpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::iterator i_prev = SpellChains.find(i->second.prev);
            if(i_prev == SpellChains.end())
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
            multimap<uint32, ScriptInfo> emptyMap;
            scripts[tmp.id] = emptyMap;
        }
        scripts[tmp.id].insert(pair<uint32, ScriptInfo>(tmp.delay, tmp));

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
        sDatabase.PExecute("DELETE FROM `mail` WHERE `time` < '%u' AND `item_guid` = '0' AND `itemTextId` = 0", basetime);
    QueryResult* result = sDatabase.PQuery("SELECT `id`,`messageType`,`sender`,`receiver`,`itemTextId`,`item_guid`,`time`,`cod`,`checked` FROM `mail` WHERE `time` < '%u'", basetime);
    if ( !result )
        return;                                             // any mails need to be returned or deleted
    Field *fields;
    //std::ostringstream delitems, delmails; //will be here for optimalization
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
        m->time = fields[6].GetUInt32();
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
                sDatabase.PExecute("UPDATE `mail` SET `sender` = '%u', `receiver` = '%u', `time` = '" I64FMTD "', `checked` = '16' WHERE `id` = '%u'", m->receiver, m->sender, (uint64)(basetime + 30*DAY), m->messageID);
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

void ObjectMgr::AddAreaTriggerPoint(AreaTriggerPoint *pArea)
{
    ASSERT( pArea->Trigger_ID );
    ASSERT( mAreaTriggerMap.find(pArea->Trigger_ID) == mAreaTriggerMap.end() );

    mAreaTriggerMap[pArea->Trigger_ID] = pArea;
}

AreaTriggerPoint *ObjectMgr::GetAreaTriggerQuestPoint(uint32 Trigger_ID)
{
    AreaTriggerMap::const_iterator itr;
    for (itr = mAreaTriggerMap.begin(); itr != mAreaTriggerMap.end(); itr++)
    {
        if(itr->second->Trigger_ID == Trigger_ID)
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::LoadAreaTriggerPoints()
{
    QueryResult *result = sDatabase.Query( "SELECT `id`,`quest` FROM `areatrigger_involvedrelation`" );
    AreaTriggerPoint *pArea;

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

        pArea = new AreaTriggerPoint;

        Field *fields = result->Fetch();

        pArea->Trigger_ID      = fields[0].GetUInt32();
        pArea->Quest_ID        = fields[1].GetUInt32();

        AddAreaTriggerPoint( pArea );

    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u quest trigger points", count );
}

uint32 ObjectMgr::GetNearestTaxiNode( float x, float y, float z, uint32 mapid )
{
    bool found = false;
    float dist;
    uint32 id = 0;
    for(uint32 i = 1; i <= sTaxiNodesStore.nCount; ++i)
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

uint16 ObjectMgr::GetTaxiMount( uint32 id, uint32 team  )
{
    uint16 mount_id = 0;

    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
    if(node)
    {
        if (team == ALLIANCE)
            switch(node->alliance_mount_type)
            {
                case 541:
                    mount_id = 1147;                        // alliance
                    break;
                case 3837:
                    mount_id = 479;                         // nightelf
                    break;
                //case 17760:
                //  unknown outer bg mount?
            }
            else if (team == HORDE)
                switch(node->horde_mount_type)
                {
                    case 2224:
                        mount_id = 295;                     // horde
                        break;
                    case 3574:
                        mount_id = 1566;                    // undead
                        break;
                //case 17760:
                //  unknown outer bg mount?
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
        sLog.outErrorDb("Table `game_graveyard_zone` incomplite: Zone %u Map %u Team %u not have linked graveyard.",zoneId,MapId,team);
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

        // skip enimy faction graveyard at same map (normal area, city, or battleground)
        if(g_team != 0 && g_team != team)
            continue;

        // find now nearest graveyrd at same map
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

AreaTrigger *ObjectMgr::GetAreaTrigger(uint32 Trigger_ID)
{
    if( !Trigger_ID )
        return NULL;
    QueryResult *result = sDatabase.PQuery("SELECT `required_level`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `areatrigger_template` WHERE `id` = '%u'", Trigger_ID);
    if ( !result )
        return NULL;
    Field *fields = result->Fetch();
    AreaTrigger *at = new AreaTrigger;

    at->requiredLevel = fields[0].GetUInt8();
    at->mapId = fields[1].GetUInt32();
    at->X = fields[2].GetFloat();
    at->Y = fields[3].GetFloat();
    at->Z = fields[4].GetFloat();
    at->Orientation = fields[5].GetFloat();

    delete result;

    if(at->X==0&&at->Y==0&&at->Z==0)
        return NULL;

    return at;
}

void ObjectMgr::LoadTeleportCoords()
{
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `id`,`target_position_x`,`target_position_y`,`target_position_z`,`target_map` FROM `areatrigger_template`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u teleport definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    TeleportCoords *pTC;

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        pTC = new TeleportCoords;
        pTC->id = fields[0].GetUInt32();
        //pTC->Name = fields[6].GetString();
        pTC->mapId = fields[4].GetUInt32();
        pTC->x = fields[1].GetFloat();
        pTC->y = fields[2].GetFloat();
        pTC->z = fields[3].GetFloat();

        AddTeleportCoords(pTC);

    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u teleport definitions", count );
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

std::string ObjectMgr::GeneratePetName(uint32 entry)
{
    std::vector<std::string> & list0 = PetHalfName0[entry];
    std::vector<std::string> & list1 = PetHalfName1[entry];

    if(list0.empty() || list1.empty())
    {
        CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(entry);
        char* petname = GetPetName(cinfo->family);
        if(!petname)
            petname = cinfo->Name;
        return std::string(petname);
    }

    return *(list0.begin()+urand(0, list0.size()-1)) + *(list1.begin()+urand(0, list1.size()-1));
}

bool ObjectMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2)
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool ObjectMgr::canStackSpellRank(SpellEntry const *spellInfo)
{
    if(GetSpellRank(spellInfo->Id) == 0)
        return true;

    if(spellInfo->powerType == 0)
    {
        if(spellInfo->manaCost > 0)
            return true;
        if(spellInfo->ManaCostPercentage > 0)
            return true;
        if(spellInfo->manaCostPerlevel > 0)
            return true;
        if(spellInfo->manaPerSecond > 0)
            return true;
        if(spellInfo->manaPerSecondPerLevel > 0)
            return true;
    }
    return false;
}

void ObjectMgr::LoadCorpses()
{
    uint32 count = 0;
    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance`,`guid` FROM `corpse` WHERE `bones_flag` = 0");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u corpses", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();

        uint32 guid = fields[result->GetFieldCount()-1].GetUInt32();

        Corpse* corpse = new Corpse(NULL);
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

    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == 107 || spellInfo_1->EffectApplyAuraName[i] == 108 ||
                spellInfo_2->EffectApplyAuraName[i] == 107 || spellInfo_2->EffectApplyAuraName[i] == 108)
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
void ObjectMgr::LoadReputationOnKill()
{
    uint32 count = 0; 

    //                                             0             1                      2
    QueryResult *result = sDatabase.Query("SELECT `creature_id`,`RewOnKillRepFaction1`,`RewOnKillRepFaction2`,"
        //3              4              5                    6              7              8         
        "`IsTeamAward1`,`MaxStanding1`,`RewOnKillRepValue1`,`IsTeamAward2`,`MaxStanding2`,`RewOnKillRepValue2`"
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
        repOnKill.is_teamaward1        = fields[3].GetUInt32();
        repOnKill.reputration_max_cap1 = fields[4].GetUInt32();
        repOnKill.repvalue1            = fields[5].GetInt32();
        repOnKill.is_teamaward2        = fields[6].GetUInt32();
        repOnKill.reputration_max_cap2 = fields[7].GetUInt32();
        repOnKill.repvalue2            = fields[8].GetInt32();

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
