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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "AuctionHouseObject.h"

//pls DO NOT use iterator++, because it is slowlier than ++iterator!!!
//post-incrementation is always slowlier than pre-incrementation !

void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    uint64 guid; //NPC guid
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!unit) {
        sLog.outDebug( "WORLD: HandleAuctionHelloOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }
    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;
    if( !unit->isAuctioner())                               // it's not auctioner
        return;

    SendAuctionHello(guid, unit);
}

static uint8 AuctioneerFactionToLocation(uint32 faction)
{
    switch (faction)
    {
        case 29:                                            //orc
        case 68:                                            //undead
        case 104:                                           //tauren
            return 6;
            break;
        case 12:                                            //human
        case 55:                                            //dwarf
        case 79:                                            //Nightelf
            return 2;
            break;
        default:                                            /* 85 and so on ... neutral*/
            return 7;
    }
}

void WorldSession::SendAuctionHello( uint64 guid, Creature* unit )
{
    WorldPacket data;
    data.Initialize( MSG_AUCTION_HELLO );
    data << (uint64) guid;
    data << (uint32) AuctioneerFactionToLocation(unit->getFaction());
    SendPacket( &data );
}
//this function inserts to WorldPacket auction's data 
bool WorldSession::SendAuctionInfo(WorldPacket & data, AuctionEntry* auction)
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if (!pItem) {
        sLog.outError("auction to item, that doesn't exist !!!!");		
        return false;
    }
    data << auction->Id;
    data << pItem->GetUInt32Value(OBJECT_FIELD_ENTRY);
    data << (uint32) 0;                                                 //0 - HighBidder, 1 - outbid, BID TYPE - not sure
    data << (uint32) 0;                                                 //unknown constant 0 ?
    data << (uint32) 0;                                                 //not pItem->GetCreator();// 4a d0 64 02, 0, unknown
    data << (uint32) pItem->GetCount();                                 //item->count
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES);   //item->charge FFFFFFF
    data << (uint32) auction->owner;                                    //Auction->owner
    data << (uint32) 0;                                                 //player_high_guid
    data << (uint32) auction->startbid;                                 //Auction->startbid
    data << (uint32) 0;                                                 //minimal outbid... not fixed now... wait a moment ;-)
    data << (uint32) auction->buyout;                                   //auction->buyout
    data << (uint32) (auction->time - time(NULL)) * 1000;               //time
    data << (uint32) auction->bidder;                                   //auction->bidder current
    data << (uint32) 0;                                                 //0 ? .. player highguid
    data << (uint32) auction->bid;                                      //current bid
    return true;
}
//call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(uint32 auctionId, uint32 Action, uint32 ErrorCode, uint32 bidError )
{
    WorldPacket data;
    data.Initialize( SMSG_AUCTION_COMMAND_RESULT );
    data << auctionId;
    data << Action;
    data << ErrorCode;
    if ( Action )
        data << bidError; //when bid, then send 0, once...
    SendPacket(&data);
}
//this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification( uint32 location, uint32 auctionId, uint64 bidder, uint32 bidSum, uint32 diff, uint32 item_template)
{
    WorldPacket data;
    data.Initialize(SMSG_AUCTION_BIDDER_NOTIFICATION);
    data << location;
    data << auctionId;
    data << (uint64) bidder;
    data << bidSum;
    data << (uint32) diff;
    data << item_template;
    data << (uint32) 0;
    SendPacket(&data);
}

void WorldSession::SendAuctionOwnerNotification( AuctionEntry* auction)
{
    WorldPacket data;
    data.Initialize(SMSG_AUCTION_OWNER_NOTIFICATION);
    data << auction->Id;
    data << auction->bid;
    data << (uint32) 0; //unk
    data << (uint32) 0; //unk
    data << (uint32) 0; //unk
    data << auction->item_template;
    data << (uint32) 0; //unk
    SendPacket(&data);
}
//this function sends mail to old bidder 
void WorldSession::SendAuctionOutbiddedMail(AuctionEntry *auction, uint32 newPrice)
{
    uint32 mailId = objmgr.GenerateMailID();
    time_t etime = time(NULL) + (30 * DAY);

    std::ostringstream msgAuctionExpiredSubject;
    msgAuctionExpiredSubject << auction->item_template << ":0:" << AUCTION_OUTBIDDED;

    Player *oldBidder = objmgr.GetPlayer((uint64) auction->bidder);
    if (oldBidder) {
        oldBidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, _player->GetGUID(), newPrice, newPrice - auction->bid, auction->item_template);
        oldBidder->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionExpiredSubject.str(), 0, 0, 0, etime, auction->bid, 0, NOT_READ, NULL); 
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '0', '0', '0', '" I64FMTD "', '%u', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->bidder, msgAuctionExpiredSubject.str().c_str(), (uint64)etime, auction->bid, NOT_READ);
}

void WorldSession::HandleAuctionSellItem( WorldPacket & recv_data )
{
    uint64 auctioneer, item;
    uint32 etime, bid, buyout;
    recv_data >> auctioneer >> item;
    recv_data >> bid >> buyout >> etime;
    Player *pl = GetPlayer();

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, auctioneer);
    if(!pCreature||!pCreature->isAuctioner()) {
        return;
    }

    uint16 pos = pl->GetPosByGuid(item);
    Item *it = pl->GetItemByPos( pos );

    // prevent sending bag with items (cheat: can be placed in bag after adding equiped empty bag to auction)
    if(!it || !it->CanBeTraded())
    {
        //5b 02 00 00 00 00 00 00 00 00 02 00 00 00 -- -- : [.............
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR); //sure
        return;
    }
    
    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());
    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    //we have to take deposit :
    uint32 deposit = objmgr.GetAuctionDeposit( location, etime, it );
    if ( pl->GetMoney() < deposit ) {
        //SendAuctionCommandResult(0, AUCTION_SELL_ITEM, some error code , maybe 1?);
        return;
    }
    pl->ModifyMoney( ((int32) deposit) * -1 );

    AuctionEntry *AH = new AuctionEntry;
    AH->Id = objmgr.GenerateAuctionID();
    AH->auctioneer = GUID_LOPART(auctioneer);
    AH->item_guid = GUID_LOPART(item);
    AH->item_template = it->GetEntry();
    AH->owner = pl->GetGUIDLow();
    AH->startbid = bid;
    AH->bidder = 0;
    AH->bid = 0;
    AH->buyout = buyout;
    time_t base = time(NULL);
    AH->time = ((time_t)(etime * 60)) + base;
    AH->deposit = deposit;
    AH->location = location;

    sLog.outDetail("selling item %u to auctioneer %u with inital bid %u with buyout %u and with time %u (in minutes) in location: %u", GUID_LOPART(item), GUID_LOPART(auctioneer), bid, buyout, GUID_LOPART(time), location);
    mAuctions->AddAuction(AH);

    // DB can have outdate auction item with same guid
    objmgr.RemoveAItem(GUID_LOPART(item));

    objmgr.AddAItem(it);
    pl->RemoveItem( (pos >> 8),(pos & 255), true);
    it->RemoveFromUpdateQueueOf(pl);
    it->DeleteFromInventoryDB();
    it->SaveToDB();
    sDatabase.PExecute("INSERT INTO `auctionhouse` (`id`,`auctioneerguid`,`itemguid`,`item_template`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`startbid`,`deposit`,`location`) "
        "VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u', '%u', '%u')",
        AH->Id, AH->auctioneer, AH->item_guid, AH->item_template, AH->owner, AH->buyout, AH->time, AH->bidder, AH->bid, AH->startbid, AH->deposit, AH->location);

    SendAuctionCommandResult(AH->Id, AUCTION_SELL_ITEM, AUCTION_OK);
    //pl->SaveToDB() - isn't needed, because item will be removed from inventory now, only money are problem
}

void WorldSession::HandleAuctionPlaceBid( WorldPacket & recv_data )
{
    uint64 auctioneer;
    uint32 auctionId;
    uint32 price;
    WorldPacket data;
    recv_data >> auctioneer;
    recv_data >> auctionId >> price;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, auctioneer);
    if(!pCreature||!pCreature->isAuctioner())
        return;
    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    AuctionEntry *auction = mAuctions->GetAuction(auctionId);
    Player *pl = GetPlayer();
    if ((auction) && (auction->owner != pl->GetGUIDLow()))
    {
        if (price < auction->bid) {
            //auction has already higher bid, client tests it!
            //SendAuctionCommandResult(auction->auctionId, AUCTION_PLACE_BID, ???);
            return;
        }
        if (price > pl->GetMoney()) {
            //you don't have enought money!, client tests!
            //SendAuctionCommandResult(auction->auctionId, AUCTION_PLACE_BID, ???);
            return;
        }
        if ((price < auction->buyout) || (auction->buyout == 0))
        {
            if (auction->bidder > 0) {
                if ( auction->bidder == pl->GetGUIDLow() ) {
                    pl->ModifyMoney(((uint32)(price - auction->bid)) * -1);
                } else {
                    // mail to last bidder if there's one... + return money
                    SendAuctionOutbiddedMail( auction , price );
                    pl->ModifyMoney(((int32) price) * -1);
                }
            } else {
                pl->ModifyMoney(((int32) price) * -1);
            }
            auction->bidder = pl->GetGUIDLow();
            auction->bid = price;

            // after this update we should save player's money ...
            sDatabase.PExecute("UPDATE `auctionhouse` SET `buyguid` = '%u',`lastbid` = '%u' WHERE `id` = '%u';", auction->bidder, auction->bid, auction->Id);

            SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK, 0 );
        } else {
            //buyout:
            if (pl->GetGUIDLow() == auction->bidder ) {
                pl->ModifyMoney(-int32(auction->buyout - auction->bid));
            } else {
                pl->ModifyMoney(-int32(auction->buyout));
                if ( auction->bidder ) {                                         //buyout for bidded auction ..
                    SendAuctionOutbiddedMail( auction, auction->buyout );
                }
            }
            auction->bidder = pl->GetGUIDLow();
            auction->bid = auction->buyout;
            
            objmgr.SendAuctionSuccessfulMail( auction );
            objmgr.SendAuctionWonMail( auction );

            SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK);
        }
    } else {
        //send auction database error..., or somethig like that
        SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_INTERNAL_ERROR, 0); //maybe would work
    }
}
//will be fixed soon , but now it's not used....
void WorldSession::HandleAuctionRemoveItem( WorldPacket & recv_data )
{
    uint64 auctioneer;
    uint32 auctionID;
    recv_data >> auctioneer;
    recv_data >> auctionID;
    sLog.outDebug( "DELETE AUCTION AuctionID: %u", auctionID);
    // Fetches the AH auction
    //AuctionEntry *ah = objmgr.GetAuction(auctionID);
    /*Player *pl = GetPlayer();
    //will be fixed soon
    if ((ah))                                               // && (ah->owner != pl->GetGUIDLow()))
    {
        Item *it = objmgr.GetAItem(ah->item_guidlow);
        ItemPrototype const *proto = it->GetProto();
        if (it)
        {

            if (ah->bidder > 0)                             // If we have a bidder, we have to send him the money he paid
            {
                Mail *m = new Mail;
                m->messageID = objmgr.GenerateMailID();
                m->sender = ah->owner;
                m->receiver = ah->bidder;
                std::ostringstream msgAuctionCanceled;
                msgAuctionCanceled << "Auction Was Canceled: "  << proto->Name1;
                m->subject = msgAuctionCanceled.str();
                m->body = "";
                m->item_guidlow = 0;
                m->item_id = 0;
                m->time = time(NULL) + (29 * DAY);
                m->money = ah->bid;
                m->COD = 0;
                m->checked = 0;

                //escape apostrophes
                std::string subject = m->subject;
                std::string body = m->body;
                sDatabase.escape_string(body);
                sDatabase.escape_string(subject);

                sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", m->messageID);
                sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`item_template`,`time`,`money`,`cod`,`checked`) "
                    "VALUES( '%u', '%u', '%u', '%s', '%s', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u')",
                    m->messageID, m->sender, m->receiver, subject.c_str(), body.c_str(), m->item_guidlow, m->item_id, (uint64)m->time, m->money, m->COD, m->checked);

                uint64 mrcpl = MAKE_GUID(m->receiver,HIGHGUID_PLAYER);
                Player *mrpln = objmgr.GetPlayer(mrcpl);
                if (mrpln)
                {
                    mrpln->AddMail(m);
                }
            }
            // Return the item
            Mail *mn2 = new Mail;
            mn2->messageID = objmgr.GenerateMailID();
            mn2->sender = ah->owner;
            mn2->receiver = pl->GetGUIDLow();
            std::ostringstream msgAuctionCanceledOwner;
            msgAuctionCanceledOwner << "Auction Canceled: " << proto->Name1;
            mn2->body = "";
            mn2->subject = msgAuctionCanceledOwner.str();
            mn2->item_guidlow = ah->item_guidlow;
            mn2->item_id = ah->item_id;
            mn2->time = time(NULL) + (29 * DAY);
            mn2->money = 0;
            mn2->COD = 0;
            mn2->checked = 0;

            //escape apostrophes
            std::string subject = mn2->subject;
            std::string body = mn2->body;
            sDatabase.escape_string(body);
            sDatabase.escape_string(subject);

            sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", mn2->messageID);
            sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`item_template`,`time`,`money`,`cod`,`checked`) "
                "VALUES( '%u', '%u', '%u', '%s', '%s', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u')",
                mn2->messageID , mn2->sender , mn2->receiver , subject.c_str() , body.c_str(), mn2->item_guidlow , mn2->item_id , (uint64)mn2->time ,mn2->money ,mn2->COD ,mn2->checked);

            uint64 mrcpl2 = MAKE_GUID(mn2->receiver,HIGHGUID_PLAYER);
            Player *mrpln2 = objmgr.GetPlayer(mrcpl2);
            if (mrpln2)
            {
                // ItemPrototype
                mrpln2->AddMItem(it);
                mrpln2->AddMail(mn2);
            }
        }
        else
        {
            // TODO Item not found - Error handling
            return;
        }
    }
    else
    {
        return;
    }

    // Now remove the auction
    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",ah->Id);
    objmgr.RemoveAItem(ah->item_guidlow);
    objmgr.RemoveAuction(ah->Id);

    // And return an updated list of items
    WorldPacket data;
    data.Initialize( SMSG_AUCTION_OWNER_LIST_RESULT );

    uint32 count = 0;
    data << uint32(0);
    for (ObjectMgr::AuctionEntryMap::iterator itr = objmgr.GetAuctionsBegin();itr != objmgr.GetAuctionsEnd();itr++)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry )
        {
            if( Aentry->owner == _player->GetGUIDLow() )
            {
                Item *item = objmgr.GetAItem(Aentry->item_guidlow);
                if( item )
                {
                    ItemPrototype const *proto = item->GetProto();
                    if( proto )
                    {
                        count++;
                        data << Aentry->Id;
                        data << proto->ItemId;
                        data << uint32(0);
                        data << uint32(0);
                        data << uint32(0);
                        data << uint32(item->GetCount());
                        data << uint32(0);
                        data << item->GetOwnerGUID();
                        data << Aentry->bid;
                        data << uint32(0);
                        data << Aentry->buyout;
                                                            // May be need fixing
                        data << uint32((Aentry->time - time(NULL)) * 1000);
                        data << uint32(Aentry->bidder);
                        data << uint32(0);
                        data << Aentry->bid;
                        if( count == 50 )
                            break;
                    }
                }
            }
        }
    }
    data.put<uint32>(0, count);
    data << uint32(count);
    SendPacket(&data);
    //    WorldPacket data;
    //    data << unk1 << auctionID;                              //need fix here, this code does nothing
    //    SendPacket(&data);*/
}

void WorldSession::HandleAuctionListBidderItems( WorldPacket & recv_data )
{
    uint64 guid;            //NPC guid
    float unknownAuction;   //0 Constant ?

    recv_data >> guid;
    recv_data >> unknownAuction;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if(!pCreature||!pCreature->isAuctioner())
        return;
    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    WorldPacket data;
    data.Initialize( SMSG_AUCTION_BIDDER_LIST_RESULT );
    Player *pl = GetPlayer();
    data << (uint32) 0; //add 0 as count
	uint32 cnt = 0;
    for (AuctionHouseObject::AuctionEntryMap::iterator itr = mAuctions->GetAuctionsBegin();itr != mAuctions->GetAuctionsEnd();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->bidder == pl->GetGUIDLow() && (cnt < 51))
        {
            if (SendAuctionInfo(data, itr->second))
                cnt++;            
        }
    }
    data.put( 0, cnt );                                   // add count to placeholder
    data << cnt;                                          //not sure
    SendPacket(&data);
}

void WorldSession::HandleAuctionListOwnerItems( WorldPacket & recv_data )
{
    uint32 count;
    uint64 guid;

    recv_data >> guid;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if(!pCreature||!pCreature->isAuctioner())
        return;
    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    WorldPacket data;
    data.Initialize( SMSG_AUCTION_OWNER_LIST_RESULT );
    count = 0;
    data << (uint32) 0;
    for (AuctionHouseObject::AuctionEntryMap::iterator itr = mAuctions->GetAuctionsBegin();itr != mAuctions->GetAuctionsEnd();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry )
        {
            if( Aentry->owner == _player->GetGUIDLow() )
            {
                Item *item = objmgr.GetAItem(Aentry->item_guid);
                if( item )
                {
                    ItemPrototype const *proto = item->GetProto();
                    if( proto )
                    {
                        if (SendAuctionInfo(data, itr->second))
                            count++;
                        if( count == 50 )
                            break;
                    }
                }
            }
        }
    }
    data.put<uint32>(0, count);
    data << (uint32) count;
    SendPacket(&data);
}

void WorldSession::HandleAuctionListItems( WorldPacket & recv_data )
{
    std::string searchedname, name;
    uint8 levelmin, levelmax, usable, location;
    uint32 count, totalcount, listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality;
    uint64 guid;

    recv_data >> guid;
    recv_data >> listfrom;
    recv_data >> searchedname;
    recv_data >> levelmin >> levelmax;
    recv_data >> auctionSlotID >> auctionMainCategory >> auctionSubCategory;
    recv_data >> quality >> usable;

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if(!pCreature||!pCreature->isAuctioner())
        return;

    location = AuctioneerFactionToLocation(pCreature->getFaction());
    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    sLog.outDebug("Auctionhouse search guid: " I64FMTD ", list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u", guid, listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data;
    data.Initialize( SMSG_AUCTION_LIST_RESULT );
    count = 0;
    totalcount = 0;
    data << (uint32) 0;
    for (AuctionHouseObject::AuctionEntryMap::iterator itr = mAuctions->GetAuctionsBegin();itr != mAuctions->GetAuctionsEnd();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        Item *item = objmgr.GetAItem(Aentry->item_guid);
        if( item )
        {
            ItemPrototype const *proto = item->GetProto();
            if( proto )
            {
                if( auctionMainCategory == (0xffffffff) || proto->Class == auctionMainCategory )
                {
                    if( auctionSubCategory == (0xffffffff) || proto->SubClass == auctionSubCategory )
                    {
                        if( auctionSlotID == (0xffffffff) || proto->InventoryType == auctionSlotID )
                        {
                            if( quality == (0xffffffff) || proto->Quality == quality )
                            {
                                if( usable == (0x00) || _player->CanUseItem( item ) == EQUIP_ERR_OK )
                                {
                                    if( ( levelmin == (0x00) || proto->RequiredLevel >= levelmin ) && ( levelmax == (0x00) || proto->RequiredLevel <= levelmax ) )
                                    {
                                        name = proto->Name1;
                                        std::transform( name.begin(), name.end(), name.begin(), ::tolower );
                                        std::transform( searchedname.begin(), searchedname.end(), searchedname.begin(), ::tolower );
                                        if( searchedname.empty() || name.find( searchedname ) != std::string::npos )
                                        {
                                            if ((count < 50) && (totalcount >= listfrom))
                                            {
                                                count++;
                                                SendAuctionInfo( data, Aentry);                                                
                                            }
                                            totalcount++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    data.put<uint32>(0, count);
    data << (uint32) totalcount;
    SendPacket(&data);
}
