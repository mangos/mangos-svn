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

//void called when player click on auctionhouse npc
void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;                                            //NPC guid
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_AUCTIONEER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleAuctionHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendAuctionHello(guid, unit);
}

static uint8 AuctioneerFactionToLocation(uint32 faction)
{
    if(sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        return 7;                                           // neutral

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
        default:                                            // 85 and so on ... neutral
            return 7;
    }
}

//this void causes that auction window is opened
void WorldSession::SendAuctionHello( uint64 guid, Creature* unit )
{
    WorldPacket data( MSG_AUCTION_HELLO, 12 );
    data << (uint64) guid;
    data << (uint32) AuctioneerFactionToLocation(unit->getFaction());
    SendPacket( &data );
}

//this function inserts to WorldPacket auction's data
bool WorldSession::SendAuctionInfo(WorldPacket & data, AuctionEntry* auction)
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if (!pItem)
    {
        sLog.outError("auction to item, that doesn't exist !!!!");
        return false;
    }
    data << auction->Id;
    data << pItem->GetUInt32Value(OBJECT_FIELD_ENTRY);
    
    // Permanent enchantment
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+0*3+0);
                                                            //Enchanting 0 (green)
    data << (uint32) 0;                                     //(duration, but not show?)
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+0*3+2);
                                                            //(charges)

    // Temporary enchantment
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+1*3+0);
                                                            //Enchanting 1 (green)
    data << (uint32) 0;                                     //(duration, but not show?)
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_ENCHANTMENT+1*3+2);
                                                            //(charges)

    data << (uint32) 0;                                     //Socket 0? (white)
    data << (uint32) 0;                                     //(field of Socket 0?)
    data << (uint32) 0;                                     //(charges)

    data << (uint32) 0;                                     //Socket 1? (white)
    data << (uint32) 0;                                     //(field of Socket 1?)
    data << (uint32) 0;                                     //(charges)

    data << (uint32) 0;                                     //Socket 2? (white)
    data << (uint32) 0;                                     //(field of Socket 2?)
    data << (uint32) 0;                                     //(charges)

    data << (uint32) 0;                                     //Bonus ? (green)
    data << (uint32) 0;                                     //(field of Bonus?)
    data << (uint32) 0;                                     //(charges)

    data << (uint32) pItem->GetItemRandomPropertyId();      //random item property id

    data << (uint32) 0;                                     //not pItem->GetCreator();// 4a d0 64 02, 0, unknown, maybe enchating
    data << (uint32) pItem->GetCount();                     //item->count
                                                            //item->charge FFFFFFF
    data << (uint32) pItem->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES);
    data << (uint32) 0;                                     //unknown
    data << (uint32) auction->owner;                        //Auction->owner
    data << (uint32) 0;                                     //player_high_guid
    data << (uint32) auction->startbid;                     //Auction->startbid
    data << (uint32) auction->outBid;                       //minimal outbid...
    data << (uint32) auction->buyout;                       //auction->buyout
    data << (uint32) (auction->time - time(NULL)) * 1000;   //time ---- has changed!
    data << (uint32) auction->bidder;                       //auction->bidder current
    data << (uint32) 0;                                     //player highguid
    data << (uint32) auction->bid;                          //current bid
    return true;
}

//call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(uint32 auctionId, uint32 Action, uint32 ErrorCode, uint32 bidError )
{
    WorldPacket data( SMSG_AUCTION_COMMAND_RESULT, 16 );
    data << auctionId;
    data << Action;
    data << ErrorCode;
    if ( !ErrorCode && Action )
        data << bidError;                                   //when bid, then send 0, once...
    SendPacket(&data);
}

//this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification( uint32 location, uint32 auctionId, uint64 bidder, uint32 bidSum, uint32 diff, uint32 item_template)
{
    WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, (8*4));
    data << location;
    data << auctionId;
    data << (uint64) bidder;
    data << bidSum;
    data << (uint32) diff;
    data << item_template;
    data << (uint32) 0;
    SendPacket(&data);
}

//this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification( AuctionEntry* auction)
{
    WorldPacket data(SMSG_AUCTION_OWNER_NOTIFICATION, (7*4));
    data << auction->Id;
    data << auction->bid;
    data << (uint32) 0;                                     //unk
    data << (uint32) 0;                                     //unk
    data << (uint32) 0;                                     //unk
    data << auction->item_template;
    data << (uint32) 0;                                     //unk
    SendPacket(&data);
}

//this function sends mail to old bidder
void WorldSession::SendAuctionOutbiddedMail(AuctionEntry *auction, uint32 newPrice)
{
    uint32 mailId = objmgr.GenerateMailID();
    time_t etime = time(NULL) + (30 * DAY);

    std::ostringstream msgAuctionOutbiddedSubject;
    msgAuctionOutbiddedSubject << auction->item_template << ":0:" << AUCTION_OUTBIDDED;

    uint64 oldBidder_guid = MAKE_GUID(auction->bidder,HIGHGUID_PLAYER);
    Player *oldBidder = objmgr.GetPlayer(oldBidder_guid);
    if (oldBidder)
    {
        oldBidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, _player->GetGUID(), newPrice, auction->outBid, auction->item_template);
        oldBidder->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionOutbiddedSubject.str(), 0, 0, 0, etime, auction->bid, 0, NOT_READ, NULL);
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '0', '0', '0', '" I64FMTD "', '%u', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->bidder, msgAuctionOutbiddedSubject.str().c_str(), (uint64)etime, auction->bid, NOT_READ);
}

//this function sends mail, when auction is cancelled to old bidder
void WorldSession::SendAuctionCancelledToBidderMail( AuctionEntry* auction )
{
    uint32 mailId = objmgr.GenerateMailID();
    time_t etime = time(NULL) + (30 * DAY);

    std::ostringstream msgAuctionCancelledSubject;
    msgAuctionCancelledSubject << auction->item_template << ":0:" << AUCTION_CANCELLED_TO_BIDDER;

    uint64 bidder_guid = MAKE_GUID(auction->bidder,HIGHGUID_PLAYER);
    Player *bidder = objmgr.GetPlayer(bidder_guid);
    if (bidder)
    {
        // unknown : bidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, _player->GetGUID(), newPrice, newPrice - auction->bid, auction->item_template);
        bidder->CreateMail(mailId, AUCTIONHOUSE_MAIL, auction->location, msgAuctionCancelledSubject.str(), 0, 0, 0, etime, auction->bid, 0, NOT_READ, NULL);
    }

    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%d', '%u', '%u', '%s', '0', '0', '0', '" I64FMTD "', '%u', '0', '%d')",
        mailId, AUCTIONHOUSE_MAIL, auction->location, auction->bidder, msgAuctionCancelledSubject.str().c_str(), (uint64)etime, auction->bid, NOT_READ);
}

//this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+8+4+4+4);

    uint64 auctioneer, item;
    uint32 etime, bid, buyout;
    recv_data >> auctioneer >> item;
    recv_data >> bid >> buyout >> etime;
    Player *pl = GetPlayer();

    if (!item || !bid || !etime)
        return;                                             //check for cheaters

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, auctioneer,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionSellItem - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)) );
        return;
    }

    uint16 pos = pl->GetPosByGuid(item);
    Item *it = pl->GetItemByPos( pos );
    //do not allow to sell already auctioned items
    if(objmgr.GetAItem(GUID_LOPART(item)))
    {
        sLog.outError("AuctionError, player %s is sending item id: %u, but item is already in another auction", pl->GetName(), GUID_LOPART(item));
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR);
        return;
    }
    // prevent sending bag with items (cheat: can be placed in bag after adding equiped empty bag to auction)
    if(!it || !it->CanBeTraded())
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR);
        return;
    }

    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());
    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    //we have to take deposit :
    uint32 deposit = objmgr.GetAuctionDeposit( location, etime, it );
    if ( pl->GetMoney() < deposit )
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_NOT_ENOUGHT_MONEY);
        return;
    }

    if( GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
    {
        sLog.outCommand("GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
            GetPlayerName(),GetAccountId(),it->GetProto()->Name1,it->GetEntry(),it->GetCount());
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
    AH->outBid = 0;
    AH->buyout = buyout;
    time_t base = time(NULL);
    AH->time = ((time_t)(etime * 60)) + base;
    AH->deposit = deposit;
    AH->location = location;

    sLog.outDetail("selling item %u to auctioneer %u with inital bid %u with buyout %u and with time %u (in minutes) in location: %u", GUID_LOPART(item), GUID_LOPART(auctioneer), bid, buyout, GUID_LOPART(time), location);
    mAuctions->AddAuction(AH);

    objmgr.AddAItem(it);
    pl->RemoveItem( (pos >> 8),(pos & 255), true);
    it->RemoveFromUpdateQueueOf(pl);

    sDatabase.BeginTransaction();
    it->DeleteFromInventoryDB();
    it->SaveToDB();                                         // recursive and not have transaction guard into self
    sDatabase.PExecute("INSERT INTO `auctionhouse` (`id`,`auctioneerguid`,`itemguid`,`item_template`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`startbid`,`deposit`,`location`) "
        "VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '" I64FMTD "', '%u', '%u', '%u', '%u', '%u')",
        AH->Id, AH->auctioneer, AH->item_guid, AH->item_template, AH->owner, AH->buyout, (uint64)AH->time, AH->bidder, AH->bid, AH->startbid, AH->deposit, AH->location);
    pl->SaveInventoryAndGoldToDB();
    sDatabase.CommitTransaction();

    SendAuctionCommandResult(AH->Id, AUCTION_SELL_ITEM, AUCTION_OK);
}

//this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    uint64 auctioneer;
    uint32 auctionId;
    uint32 price;
    recv_data >> auctioneer;
    recv_data >> auctionId >> price;

    if (!auctionId || !price)
        return;                                             //check for cheaters

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, auctioneer,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionPlaceBid - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)) );
        return;
    }

    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    AuctionEntry *auction = mAuctions->GetAuction(auctionId);
    Player *pl = GetPlayer();
    if ((auction) && (auction->owner != pl->GetGUIDLow()))
    {
        if (price < (auction->bid + auction->outBid))
        {
            //auction has already higher bid, client tests it!
            //SendAuctionCommandResult(auction->auctionId, AUCTION_PLACE_BID, ???);
            return;
        }
        if (price > pl->GetMoney())
        {
            //you don't have enought money!, client tests!
            //SendAuctionCommandResult(auction->auctionId, AUCTION_PLACE_BID, ???);
            return;
        }
        if ((price < auction->buyout) || (auction->buyout == 0))
        {
            auction->outBid += 5;                           //this line must be here

            if (auction->bidder > 0)
            {
                if ( auction->bidder == pl->GetGUIDLow() )
                {
                    pl->ModifyMoney(((uint32)(price - auction->bid)) * -1);
                }
                else
                {
                    // mail to last bidder and return money
                    SendAuctionOutbiddedMail( auction , price );
                    pl->ModifyMoney(((int32) price) * -1);
                }
            }
            else
            {
                pl->ModifyMoney(((int32) price) * -1);
            }
            auction->bidder = pl->GetGUIDLow();
            auction->bid = price;
            if ( auction->outBid > 10000 )                  //one gold
                auction->outBid = 5;

            // after this update we should save player's money ...
            sDatabase.PExecute("UPDATE `auctionhouse` SET `buyguid` = '%u',`lastbid` = '%u' WHERE `id` = '%u';", auction->bidder, auction->bid, auction->Id);

            SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK, 0 );
        }
        else
        {
            //buyout:
            if (pl->GetGUIDLow() == auction->bidder )
            {
                pl->ModifyMoney(-int32(auction->buyout - auction->bid));
            }
            else
            {
                pl->ModifyMoney(-int32(auction->buyout));
                if ( auction->bidder )                      //buyout for bidded auction ..
                {
                    SendAuctionOutbiddedMail( auction, auction->buyout );
                }
            }
            auction->bidder = pl->GetGUIDLow();
            auction->bid = auction->buyout;

            objmgr.SendAuctionSuccessfulMail( auction );
            objmgr.SendAuctionWonMail( auction );

            SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK);

            objmgr.RemoveAItem(auction->item_guid);
            mAuctions->RemoveAuction(auction->Id);
            sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",auction->Id);

            delete auction;
        }
        sDatabase.BeginTransaction();
        pl->SaveInventoryAndGoldToDB();
        sDatabase.CommitTransaction();
    }
    else
    {
        //you cannot bid your own auction:
        SendAuctionCommandResult( 0, AUCTION_PLACE_BID, CANNOT_BID_YOUR_AUCTION_ERROR );
    }
}

//this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 auctioneer;
    uint32 auctionId;
    recv_data >> auctioneer;
    recv_data >> auctionId;
    //sLog.outDebug( "Cancel AUCTION AuctionID: %u", auctionId);

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, auctioneer,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionRemoveItem - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)) );
        return;
    }

    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    AuctionEntry *auction = mAuctions->GetAuction(auctionId);
    Player *pl = GetPlayer();

    if (auction && auction->owner == pl->GetGUIDLow())
    {
        Item *pItem = objmgr.GetAItem(auction->item_guid);
        if (pItem)
        {
            if (auction->bidder > 0)                        // If we have a bidder, we have to send him the money he paid
            {
                uint32 auctionCut = objmgr.GetAuctionCut( auction->location, auction->bid);
                if ( pl->GetMoney() < auctionCut )          //player doesn't have enought money, maybe message needed
                    return;
                //some auctionBidderNotification would be needed, but don't know that parts..
                SendAuctionCancelledToBidderMail( auction );
                pl->ModifyMoney( ((int32) auctionCut) * -1 );
            }
            // Return the item by mail
            std::ostringstream msgAuctionCanceledOwner;
            msgAuctionCanceledOwner << auction->item_template << ":0:" << AUCTION_CANCELED;

            uint32 messageID = objmgr.GenerateMailID();
            time_t etime = time(NULL) + (30 * DAY);

            pl->CreateMail( messageID, AUCTIONHOUSE_MAIL, auction->location, msgAuctionCanceledOwner.str(), 0, auction->item_guid, auction->item_template, etime, 0, 0, 0, pItem);
            sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
                "VALUES ('%u', '%d', '%u', '%u', '%s', '0', '%u', '%u', '" I64FMTD "', '0', '0', '0')",
                messageID, AUCTIONHOUSE_MAIL, auction->location , pl->GetGUIDLow() , msgAuctionCanceledOwner.str().c_str(), auction->item_guid, auction->item_template, (uint64)etime);

        }
        else
        {
            sLog.outError("Auction id: %u has non-existed item (item guid : %u)!!!", auction->Id, auction->item_guid);
            SendAuctionCommandResult( 0, AUCTION_CANCEL, AUCTION_INTERNAL_ERROR );
            return;
        }
    }
    else
    {
        SendAuctionCommandResult( 0, AUCTION_CANCEL, AUCTION_INTERNAL_ERROR );
        //this code isn't possible ... maybe there should be assert
        sLog.outError("CHEATER : %u, he tried to cancel auction (id: %u) of another player, or auction is NULL", pl->GetGUIDLow(), auctionId );
        return;
    }

    //inform player, that auction is removed
    SendAuctionCommandResult( auction->Id, AUCTION_CANCEL, AUCTION_OK );
    // Now remove the auction
    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",auction->Id);
    pl->SaveInventoryAndGoldToDB();
    sDatabase.CommitTransaction();
    objmgr.RemoveAItem( auction->item_guid );
    mAuctions->RemoveAuction( auction->Id );
    delete auction;
}

//called when player lists his bids
void WorldSession::HandleAuctionListBidderItems( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    uint64 guid;                                            //NPC guid
    uint32 listfrom;                                        //page of auctions
    uint32 outbiddedCount;                                  //count of outbidded auctions

    recv_data >> guid;
    recv_data >> listfrom;
    recv_data >> outbiddedCount;
    if (recv_data.size() != (16 + outbiddedCount * 4 ))
    {
        sLog.outError("Client sent bad opcode!!! with count: %u and size : %d (mustbe: %d", outbiddedCount, recv_data.size(),(16 + outbiddedCount * 4 ));
        outbiddedCount = 0;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionListBidderItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    WorldPacket data( SMSG_AUCTION_BIDDER_LIST_RESULT, (4+4+4) );
    Player *pl = GetPlayer();
    data << (uint32) 0;                                     //add 0 as count
    uint32 count = 0;
    uint32 totalcount = 0;
    while ( outbiddedCount > 0)                             //add all data, which client requires
    {
        outbiddedCount--;
        uint32 outbiddedAuctionId;
        recv_data >> outbiddedAuctionId;
        AuctionEntry * auction = mAuctions->GetAuction( outbiddedAuctionId );
        if ( auction && SendAuctionInfo(data, auction))
        {
            totalcount++;
            count++;
        }
    }
    for (AuctionHouseObject::AuctionEntryMap::iterator itr = mAuctions->GetAuctionsBegin();itr != mAuctions->GetAuctionsEnd();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->bidder == pl->GetGUIDLow() )
        {
            if ((count < 50) && (totalcount >= listfrom) && SendAuctionInfo(data, itr->second))
                count++;
            totalcount++;
        }
    }
    data.put( 0, count );                                   // add count to placeholder
    data << totalcount;
    SendPacket(&data);
}

//this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint32 listfrom;
    uint64 guid;

    recv_data >> guid;
    recv_data >> listfrom;                                  // page of auctions

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionListOwnerItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    uint32 location = AuctioneerFactionToLocation(pCreature->getFaction());

    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    WorldPacket data( SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4) );
    data << (uint32) 0;
    uint32 count = 0;
    uint32 totalcount = 0;
    for (AuctionHouseObject::AuctionEntryMap::iterator itr = mAuctions->GetAuctionsBegin();itr != mAuctions->GetAuctionsEnd();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->owner == _player->GetGUIDLow() )
        {
            if ((count < 50) && (totalcount >= listfrom) && SendAuctionInfo(data, itr->second))
                count++;
            totalcount++;
        }
    }
    data.put<uint32>(0, count);
    data << (uint32) totalcount;
    SendPacket(&data);
}

//this void is called when player clicks on search button
void WorldSession::HandleAuctionListItems( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4+1+1+1+4+4+4+4+1);

    std::string searchedname, name;
    uint8 levelmin, levelmax, usable, location;
    uint32 count, totalcount, listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality;
    uint64 guid;

    recv_data >> guid;
    recv_data >> listfrom;
    recv_data >> searchedname;

    // recheck with known string size
    CHECK_PACKET_SIZE(recv_data,8+4+(searchedname.size()+1)+1+1+4+4+4+4+1);

    recv_data >> levelmin >> levelmax;
    recv_data >> auctionSlotID >> auctionMainCategory >> auctionSubCategory;
    recv_data >> quality >> usable;

    Creature *pCreature = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_AUCTIONEER);
    if (!pCreature)
    {
        sLog.outDebug( "WORLD: HandleAuctionListItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    location = AuctioneerFactionToLocation(pCreature->getFaction());
    AuctionHouseObject * mAuctions;
    mAuctions = objmgr.GetAuctionsMap( location );

    //sLog.outDebug("Auctionhouse search guid: " I64FMTD ", list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u", guid, listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data( SMSG_AUCTION_LIST_RESULT, (4+4+4) );
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
