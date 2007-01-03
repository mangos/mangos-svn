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

extern SQLStorage sItemStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sCreatureStorage;
ScriptMapMap sScripts;

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
}

Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    GroupSet::const_iterator itr;
    for (itr = mGroupSet.begin(); itr != mGroupSet.end(); itr++)
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
    if (location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION))
        return (uint32) (0.15 * highBid);
    else
        return (uint32) (0.05 * highBid);
}

uint32 ObjectMgr::GetAuctionDeposit(uint32 location, uint32 time, Item *pItem)
{
    uint32 percentance;
    if ( location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION))
        percentance = 25;
    else
        percentance = 5;
    return (uint32) ( ((percentance * pItem->GetProto()->SellPrice * pItem->GetCount() ) / 100 ) * (time / 120 ) );
}

void ObjectMgr::SendAuctionWonMail( AuctionEntry *auction ) //clears ram :D
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    std::ostringstream msgAuctionWonSubject;
    msgAuctionWonSubject << auction->item_template << ":0:" << AUCTION_WON;

    std::ostringstream msgAuctionWonBody;
    std::ostringstream temp;
    msgAuctionWonBody << "        ";                        //8 times " "
    temp << hex << auction->owner;
    for (int i = temp.str().size();i < 8; i++)
        msgAuctionWonBody << " ";

    msgAuctionWonBody << temp.str();
    msgAuctionWonBody << ":" << auction->bid << ":" << auction->buyout;
    sLog.outError( "AuctionWon body string : %s", msgAuctionWonBody.str().c_str() );

    //prepare mail data... :
    uint32 mailId = this->GenerateMailID();
    uint32 itemPage = this->CreateItemPage( msgAuctionWonBody.str() );
    time_t etime = time(NULL) + (30 * DAY);

    Player *bidder = objmgr.GetPlayer((uint64) auction->bidder);
    if (bidder)
    {
        bidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, (uint64) auction->bidder, 0, 0, auction->item_template);

        bidder->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionWonSubject.str(), itemPage, auction->item_guid, auction->item_template, etime, 0, 0, AUCTION_CHECKED, pItem);
    }
    else
        delete pItem;

    sDatabase.BeginTransaction();
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "', '0', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->bidder, msgAuctionWonSubject.str().c_str(), itemPage, auction->item_guid, auction->item_template, (uint64)etime, AUCTION_CHECKED);

    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",auction->Id);
    sDatabase.CommitTransaction();

    this->RemoveAItem(auction->item_guid);
    this->GetAuctionsMap(auction->location)->RemoveAuction(auction->Id);

    delete auction;
}

//call this method to send mail to auctionowner, when auction is successful
                                                            //this finction doesn't clear ram...
void ObjectMgr::SendAuctionSuccessfulMail( AuctionEntry * auction )
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    std::ostringstream msgAuctionSuccessfulSubject;
    msgAuctionSuccessfulSubject << auction->item_template << ":0:" << AUCTION_SUCCESSFUL;

    std::ostringstream auctionSuccessfulBody;
    std::ostringstream temp;
    auctionSuccessfulBody << "        ";                    //8 times " "
    temp << hex << auction->bidder;
    for (int i = temp.str().size();i < 8; i++)
        auctionSuccessfulBody << " ";                       //fill " " before id
    uint32 auctionCut = this->GetAuctionCut(auction->location, auction->bid);

    auctionSuccessfulBody << temp.str();
    auctionSuccessfulBody << ":" << auction->bid << ":0:";
    auctionSuccessfulBody << auction->deposit << ":" << auctionCut;
    sLog.outError("AuctionSuccessful body string : %s", auctionSuccessfulBody.str().c_str());

                                                            //:D
    uint32 itemPage = this->CreateItemPage( auctionSuccessfulBody.str() );

    uint32 mailId = this->GenerateMailID();
    time_t etime = time(NULL) + (30 * DAY);
    uint32 profit = auction->bid + auction->deposit - auctionCut;

    Player *owner = objmgr.GetPlayer((uint64) auction->owner);
    if (owner)
    {
        //send auctionowner notification, bidder must be current!
        owner->GetSession()->SendAuctionOwnerNotification( auction );

        owner->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionSuccessfulSubject.str(), itemPage, 0, 0, etime, profit, 0, AUCTION_CHECKED, NULL);
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '0', '0', '" I64FMTD "', '%u', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->owner, msgAuctionSuccessfulSubject.str().c_str(), itemPage, (uint64)etime, profit, AUCTION_CHECKED);
}

                                                            //clears ram
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
        //remove pointers from containers
        this->RemoveAItem(auction->item_guid);
        this->GetAuctionsMap(auction->location)->RemoveAuction(auction->Id);

        sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
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
    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",auction->Id);
    delete auction;
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

// name must be checked to correctness (if recived) before call this function
uint64 ObjectMgr::GetPlayerGUIDByName(const char *name) const
{

    uint64 guid = 0;

    // Player name safe to sending to DB (checked at login) and this function using
    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name);

    if(result)
    {
        guid = (*result)[0].GetUInt32();

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
            sLog.outError( "Error loading `playercreateinfo` table or table empty.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outError("Wrong race %u in `playercreateinfo` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outError("Wrong class %u in `playercreateinfo` table, ignoring.",current_class);
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
            sLog.outError( "Error loading `playercreateinfo_item` table or table empty.");
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
                    sLog.outError("Wrong race %u in `playercreateinfo_item` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outError("Wrong class %u in `playercreateinfo_item` table, ignoring.",current_class);
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
            sLog.outError( "Error loading `playercreateinfo_spell` table or table empty.");
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
                    sLog.outError("Wrong race %u in `playercreateinfo_spell` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outError("Wrong class %u in `playercreateinfo_spell` table, ignoring.",current_class);
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
        //                                            0      1       2       3           4
        QueryResult *result = sDatabase.Query("SELECT `race`,`class`,`Skill`,`SkillMin`, `SkillMax` FROM `playercreateinfo_skill`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString( "" );
            sLog.outString( ">> Loaded %u player create skills", count );
            sLog.outError( "Error loading `playercreateinfo_skill` table or table empty.");
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
                    sLog.outError("Wrong race %u in `playercreateinfo_skill` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outError("Wrong class %u in `playercreateinfo_skill` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->skill[0].push_back(fields[2].GetUInt16());
                pInfo->skill[1].push_back(fields[3].GetUInt16());
                pInfo->skill[2].push_back(fields[4].GetUInt16());

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
            sLog.outError( "Error loading `playercreateinfo_action` table or table empty.");
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
                    sLog.outError("Wrong race %u in `playercreateinfo_action` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outError("Wrong class %u in `playercreateinfo_action` table, ignoring.",current_class);
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
            sLog.outError( "Error loading player_levelstats table or table empty.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outError("Wrong race %u in `player_levelstats` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outError("Wrong class %u in `player_levelstats` table, ignoring.",current_class);
                continue;
            }

            uint32 current_level = fields[2].GetUInt32();
            if(current_level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > 255)                     // harcoded level maximum
                    sLog.outError("Wrong (> 255) level %u in `player_levelstats` table, ignoring.",current_level);
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
                sLog.outError("Race %i Class %i Level 1 not have stats data!",race,class_);
                exit(1);
            }

            // fill level gaps
            for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
            {
                if(pInfo->levelInfo[level].health == 0)
                {
                    sLog.outError("Race %i Class %i Level %i not have stats data, using data for %i.",race,class_,level+1, level);
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

void ObjectMgr::LoadRaidGroups()
{
    Group *group;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `leaderGuid` FROM `raidgroup`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u raidgroup definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();
        count++;

        group = new Group;
        group->LoadRaidGroupFromDB((*result)[0].GetUInt64());
        AddGroup(group);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u raidgroup definitions", count );
}

void ObjectMgr::LoadQuests()
{
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `quest_template`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded 0 quests definitions" );
        sLog.outError("`quest_template` table is empty!");
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
        if(qinfo->PrevQuestId)
            qinfo->prevQuests.push_back(qinfo->PrevQuestId);

        if(qinfo->NextQuestId)
        {
            if (QuestTemplates.find(qinfo->NextQuestId) == QuestTemplates.end())
            {
                sLog.outError("Quest %d has NextQuestId %d, but no such quest", qinfo->GetQuestId(), qinfo->NextQuestId);
            }
            else
                QuestTemplates[qinfo->NextQuestId]->prevQuests.push_back(qinfo->GetQuestId());
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
        sLog.outError("`spell_chains` table is empty!");
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
            sLog.outError("Spell %u not exist",spell_id);
            continue;
        }

        if(node.prev!=0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outError("Spell %u have non existed previous rank spell: %u",spell_id,node.prev);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outError("Spell %u have non existed first rank spell: %u",spell_id,node.first);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) || 
            (spell_id == node.first) != (node.prev == 0) || 
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outError("Spell %u have not compatiable chain data (prev: %u, first: %u, rank: %d)",spell_id,node.prev,node.first,node.rank);
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
                sLog.outError("Spell %u (prev: %u, first: %u, rank: %d) have not found in table previous rank spell.",
                    i->first,i->second.prev,i->second.first,i->second.rank);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outError("Spell %u (prev: %u, first: %u, rank: %d) have different firsdy spell in chain in comparison with previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outError("Spell %u (prev: %u, first: %u, rank: %d) have different rank in comparison with previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
        }
    }

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u spell chain records", count );
}

void ObjectMgr::LoadScripts()
{

    QueryResult *result = sDatabase.Query( "SELECT `id`,`delay`,`command`,`datalong`,`datalong2`,`datatext`, `x`, `y`, `z`, `o` FROM `scripts`" );

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

        if (sScripts.find(tmp.id) == sScripts.end())
        {
            multimap<uint32, ScriptInfo> emptyMap;
            sScripts[tmp.id] = emptyMap;
        }
        sScripts[tmp.id].insert(pair<uint32, ScriptInfo>(tmp.delay, tmp));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u script definitions", count );
}

void ObjectMgr::LoadItemPages()
{
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `item_page`");

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

        mItemPages[ fields[0].GetUInt32() ] = fields[1].GetCppString();

        ++count;

    } while ( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u item pages", count );
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
        sDatabase.PExecute("DELETE FROM `mail` WHERE `time` < '%u' AND `item_guid` = '0' AND `itemPageId` = 0", basetime);
    QueryResult* result = sDatabase.PQuery("SELECT `id`,`messageType`,`sender`,`receiver`,`itemPageId`,`item_guid`,`time`,`cod`,`checked` FROM `mail` WHERE `time` < '%u'", basetime);
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
        m->itemPageId = fields[4].GetUInt32();
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
        if (m->itemPageId)
        {
            sDatabase.PExecute("DELETE FROM `item_page` WHERE `id` = '%u'", m->itemPageId);
        }
        //deletemail = true;
        //delmails << m->messageID << ", ";
        sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", m->messageID);
        delete m;
    } while (result->NextRow());
    delete result;
}

ItemPage *ObjectMgr::RetreiveItemPageText(uint32 Page_ID)
{
    ItemPage *pIText;
    QueryResult *result = sDatabase.PQuery("SELECT `id`,`text`,`next_page` FROM `item_page` WHERE `id` = '%u'", Page_ID);

    if( !result ) return NULL;
    int cic, count = 0;
    pIText = new ItemPage;

    do
    {
        count++;
        cic = 0;

        Field *fields = result->Fetch();

        pIText->Page_ID    = fields[cic++].GetUInt32();

        pIText->PageText   = fields[cic++].GetCppString();
        pIText->Next_Page  = fields[cic++].GetUInt32();

        if ( !pIText->Page_ID ) break;
    } while( result->NextRow() );

    delete result;
    return pIText;
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

bool ObjectMgr::GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask )
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(curloc);

    if(src_i==sTaxiPathSetBySource.end())
        return false;

    TaxiPathSetForSource& pathSet = src_i->second;

    for(TaxiPathSetForSource::iterator  path_i = pathSet.begin(); path_i != pathSet.end(); ++path_i)
    {
        uint8 destination = path_i->first;
        uint8 field = (uint8)((destination - 1) / 32);
        Mask[field] |= 1 << ( (destination - 1 ) % 32 );
    }

    return true;
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
    uint32 zoneId = MapManager::Instance().GetMap(MapId)->GetZoneId(x,y);

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
        sLog.outError("DB incomplite: Zone %u Map %u Team %u not have linked graveyard.",zoneId,MapId,team);
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
            sLog.outError("Table `game_graveyard_zone` have record for not existed graveyard (WorldSafeLocs.dbc id) %u, skipped.",g_id);
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
    QueryResult *result = sDatabase.PQuery("SELECT `target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `areatrigger_template` WHERE `id` = '%u'", Trigger_ID);
    if ( !result )
        return NULL;
    Field *fields = result->Fetch();
    AreaTrigger *at = new AreaTrigger;

    at->mapId = fields[0].GetUInt32();
    at->X = fields[1].GetFloat();
    at->Y = fields[2].GetFloat();
    at->Z = fields[3].GetFloat();
    at->Orientation = fields[4].GetFloat();
    if(at->X==0&&at->Y==0&&at->Z==0)return NULL;

    delete result;
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
    result = sDatabase.Query( "SELECT MAX(`id`) FROM `item_page`" );
    if( result )
    {
        m_ItemPageId = (*result)[0].GetUInt32();

        delete result;
    }
    else
        m_ItemPageId = 0;

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

uint32 ObjectMgr::GenerateItemPageID()
{
    return ++m_ItemPageId;
}

uint32 ObjectMgr::CreateItemPage(std::string text)
{
    uint32 newItemPageId = GenerateItemPageID();
    //insert new itempage to container
    mItemPages[ newItemPageId ] = text;
    //save new itempage
    sDatabase.escape_string(text);
    //any Delete query needed, itempageId is maximum of all ids
    std::ostringstream query;
    query << "INSERT INTO `item_page` (`id`,`text`,`next_page`) VALUES ( '" << newItemPageId << "', '" << text << "', '0' )";
    sDatabase.Execute(query.str().c_str());                 //needs to be run this way, because mail body may be more than 1024 characters
    return newItemPageId;
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
