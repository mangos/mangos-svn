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
#include "Chat.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Unit.h"

void WorldSession::HandleSendMail(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+1+1+1+4+4+8+4+4);

    time_t base = time(NULL);
    uint64 mailbox,itemId;
    std::string receiver,subject,body;
    uint32 unk1,unk2,money,COD,mailId;
    recv_data >> mailbox;
    recv_data >> receiver;

    // recheck
    CHECK_PACKET_SIZE(recv_data,8+(receiver.size()+1)+1+1+4+4+8+4+4);

    recv_data >> subject;

    // recheck
    CHECK_PACKET_SIZE(recv_data,8+(receiver.size()+1)+(subject.size()+1)+1+4+4+8+4+4);

    recv_data >> body;

    // recheck
    CHECK_PACKET_SIZE(recv_data,8+(receiver.size()+1)+(subject.size()+1)+(body.size()+1)+4+4+8+4+4);

    recv_data >> unk1 >> unk2;                              // 0x29 and 0x00
    recv_data >> itemId;
    recv_data >> money >> COD;                              //then there are two (uint32) 0;

    if (receiver.size() == 0)
        return;
    normalizePlayerName(receiver);
    //sDatabase.escape_string(receiver);                      // prevent SQL injection

    Player* pl = _player;

    sLog.outDetail("Player %u is sending mail to %s with subject %s and body %s includes item %u, %u copper and %u COD copper with unk1 = %u, unk2 = %u",pl->GetGUIDLow(),receiver.c_str(),subject.c_str(),body.c_str(),GUID_LOPART(itemId),money,COD,unk1,unk2);

    uint64 rc = objmgr.GetPlayerGUIDByName(receiver);
    if(pl->GetGUID() == rc)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_CANNOT_SEND_TO_SELF);
        return;
    }
    if (pl->GetMoney() < money + 30)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }
    if (!rc)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_RECIPIENT_NOT_FOUND);
        return;
    }

    Player *receive = objmgr.GetPlayer(rc);

    uint32 rc_team = 0;
    uint8 mails_count = 0;                                  //do not allow to send to one player more than 100 mails

    if(receive)
    {
        rc_team = receive->GetTeam();
        mails_count = receive->GetMailSize();
    }
    else
    {
        rc_team = objmgr.GetPlayerTeamByGUID(rc);
        QueryResult* result = sDatabase.Query("SELECT COUNT(*) FROM `mail` WHERE `receiver` = '%u'", GUID_LOPART(rc));
        Field *fields = result->Fetch();
        mails_count = fields[0].GetUInt32();
        delete result;
    }
    //do not allow to have more than 100 mails in mailbox.. mails count is in opcode uint8!!! - so max can be 255..
    if (mails_count > 100)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    // test the receiver's Faction...
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE) && pl->GetTeam() != rc_team)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_NOT_YOUR_TEAM);
        return;
    }

    uint16 item_pos;
    Item *pItem = 0;
    if (itemId)
    {
        item_pos = pl->GetPosByGuid(itemId);
        pItem = pl->GetItemByPos( item_pos );
        // prevent sending bag with items (cheat: can be placed in bag after adding equiped empty bag to mail)
        if(!pItem || !pItem->CanBeTraded())
        {
            pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
            return;
        }
        /*if (pItem->GetUInt32Value(???) { //todo find conjured_item index to item->data array
            pl->SendMailResult(0, 0, MAIL_ERR_CANNOT_MAIL_CONJURED_ITEM);
            return;
        }*/
    }
    pl->SendMailResult(0, 0, MAIL_OK);

    uint32 itemTextId = 0;
    if (body.size() > 0)
    {
        itemTextId = objmgr.CreateItemText( body );
    }

    pl->ModifyMoney( -30 - money );

    if (pItem)
    {
        if( GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
        {
            uint32 accId = 0;
            if(receive)
                accId = receive->GetSession()->GetAccountId();
            else
            {
                accId = objmgr.GetPlayerAccountIdByGUID(rc);
            }
            sLog.outCommand("GM %s (Account: %u) mail item: %s (Entry: %u Count: %u) and money: %u to player: %s (Account: %u)",
                GetPlayerName(),GetAccountId(),pItem->GetProto()->Name1,pItem->GetEntry(),pItem->GetCount(),money,receiver.c_str(),accId);
        }

        //item reminds in item_instance table already, used it in mail now
        pl->RemoveItem( (item_pos >> 8), (item_pos & 255), true );
        pItem->RemoveFromUpdateQueueOf( pl );

        //sDatabase.BeginTransaction();
        pItem->DeleteFromInventoryDB();                     //deletes item from character's inventory
        pItem->SaveToDB(false);                                  // recursive and not have transaction guard into self
        //sDatabase.CommitTransaction();
    }
    uint32 messagetype = MAIL_NORMAL;
    uint32 item_template = pItem ? pItem->GetEntry() : 0;   //item prototype
    mailId = objmgr.GenerateMailID();

    // If theres is an item, there is a one hour delivery delay.
    time_t dtime = (itemId != 0) ? time(NULL) + sWorld.getConfig(CONFIG_MAIL_DELIVERY_DELAY) : time(NULL);

    time_t etime = dtime + DAY * ((COD > 0)? 3 : 30);       //time if COD 3 days, if no COD 30 days

    if (receive)
    {
        receive->CreateMail(mailId, messagetype, pl->GetGUIDLow(), subject, itemTextId, GUID_LOPART(itemId), item_template, etime, dtime, money, COD, NOT_READ, pItem);
    }
    else if (pItem)
        delete pItem;                                       //item is sent, but receiver isn't online .. so remove it from RAM
    // backslash all '
    sDatabase.escape_string(subject);
    //not needed : sDatabase.Execute("DELETE FROM `mail` WHERE `id` = '%u'",mID);
    sDatabase.Execute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%u', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "','" I64FMTD "', '%u', '%u', '%d')",
        mailId, messagetype, pl->GetGUIDLow(), GUID_LOPART(rc), subject.c_str(), itemTextId, GUID_LOPART(itemId), item_template, (uint64)etime, (uint64)dtime, money, COD, NOT_READ);
    //sDatabase.BeginTransaction();
    pl->SaveInventoryAndGoldToDB();
    //sDatabase.CommitTransaction();
}

//called when mail is read
void WorldSession::HandleMarkAsRead(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player *pl = _player;
    Mail *m = pl->GetMail(mailId);
    if (m)
    {
        if (pl->unReadMails)
            pl->unReadMails--;
        m->checked = m->checked | READ;
        m->expire_time = time(NULL) + (3 * DAY);
        pl->m_mailsUpdated = true;
        m->state = MAIL_STATE_CHANGED;
    }
}

//called when client deletes mail
void WorldSession::HandleMailDelete(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player* pl = _player;
    pl->m_mailsUpdated = true;
    Mail *m = pl->GetMail(mailId);
    if(m)
        m->state = MAIL_STATE_DELETED;
    pl->SendMailResult(mailId, MAIL_DELETED, 0);
}

void WorldSession::HandleReturnToSender(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player *pl = _player;
    Mail *m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    //we can return mail now
    //so firstly delete the old one
    sDatabase.Execute("DELETE FROM `mail` WHERE `id` = '%u'", mailId);
    pl->RemoveMail(mailId);

    uint32 messageID = objmgr.GenerateMailID();

    Item *pItem = NULL;
    if (m->item_guid)
    {
        pItem = pl->GetMItem(m->item_guid);
        pl->RemoveMItem(m->item_guid);
    }

    // If theres is an item, there is a one hour delivery delay.
    time_t dtime = (pItem != 0) ? time(NULL) + sWorld.getConfig(CONFIG_MAIL_DELIVERY_DELAY) : time(NULL);

    time_t etime = dtime + 30*DAY;

    Player *receiver = objmgr.GetPlayer((uint64)m->sender);
    if(receiver)
        receiver->CreateMail(messageID,0,m->receiver,m->subject,m->itemTextId,m->item_guid,m->item_template,etime, dtime, m->money,0,RETURNED_CHECKED,pItem);
    else if ( pItem )
        delete pItem;

    std::string subject;
    subject = m->subject;
    sDatabase.escape_string(subject);                       //we cannot forget to delete COD, if returning mail with COD
    sDatabase.Execute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '0', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "', '" I64FMTD "', '%u', '0', '%d')",
        messageID, m->receiver, m->sender, subject.c_str(), m->itemTextId, m->item_guid, m->item_template, (uint64)etime, (uint64)dtime, m->money,RETURNED_CHECKED);
    delete m;                                               //we can deallocate old mail
    pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, 0);
}

//called when player takes item attached in mail
void WorldSession::HandleTakeItem(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;
    uint16 dest;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player* pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    // prevent cheating with skip client money check
    if(pl->GetMoney() < m->COD)
    {
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    Item *it = pl->GetMItem(m->item_guid);

    uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, it, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->item_guid = 0;
        m->item_template = 0;

        if (m->COD > 0)                                     //if there is COD, take COD money from player and send them to sender by mail
        {
            Player *receive = objmgr.GetPlayer(MAKE_GUID(m->sender,HIGHGUID_PLAYER));

            // If theres is an item, there is a one hour delivery delay.
            time_t dtime = (it != 0) ? time(NULL) + sWorld.getConfig(CONFIG_MAIL_DELIVERY_DELAY) : time(NULL);

            time_t etime = dtime + (30 * DAY);
            uint32 newMailId = objmgr.GenerateMailID();
            if (receive)
                receive->CreateMail(newMailId, 0, m->receiver, m->subject, 0, 0, 0, etime, dtime, m->COD, 0, COD_PAYMENT_CHECKED, NULL);

            //escape apostrophes
            std::string subject = m->subject;
            sDatabase.escape_string(subject);
            sDatabase.Execute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
                "VALUES ('%u', '0', '%u', '%u', '%s', '0', '0', '0', '" I64FMTD "', '" I64FMTD "', '%u', '0', '%d')",
                newMailId, m->receiver, m->sender, subject.c_str(), (uint64)etime, (uint64)dtime, m->COD, COD_PAYMENT_CHECKED);

            pl->ModifyMoney( -int32(m->COD) );

            if( GetSecurity() > 0 && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
            {
                uint32 sender_accId = 0;
                std::string sender_name;
                if(receive)
                {
                    sender_accId = receive->GetSession()->GetAccountId();
                    sender_name = receive->GetName();
                }
                else
                {
                    uint64 sender_guid = MAKE_GUID(m->sender,HIGHGUID_PLAYER);
                    sender_accId = objmgr.GetPlayerAccountIdByGUID(sender_guid);
                    objmgr.GetPlayerNameByGUID(sender_guid,sender_name);
                }
                sLog.outCommand("GM %s (Account: %u) receive mail item: %s (Entry: %u Count: %u) and send COD money: %u to player: %s (Account: %u)",
                    GetPlayerName(),GetAccountId(),it->GetProto()->Name1,it->GetEntry(),it->GetCount(),m->COD,sender_name.c_str(),sender_accId);
            }
        }
        m->COD = 0;
        m->state = MAIL_STATE_CHANGED;
        pl->m_mailsUpdated = true;
        pl->RemoveMItem(it->GetGUIDLow());
        Item* it2 = pl->StoreItem( dest, it, true);
        if(it2 == it)                                       // only set if not merged to existed stack (*it can be deleted already but we can compare pointers any way)
            it->SetState(ITEM_NEW, pl);
        pl->ItemAddedQuestCheck(it2->GetEntry(),it2->GetCount());

        //sDatabase.BeginTransaction();
        pl->SaveInventoryAndGoldToDB();
        pl->_SaveMail();
        //sDatabase.CommitTransaction();

        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, 0);
    }
    else                                                    //works great
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_BAG_FULL, msg);
}

void WorldSession::HandleTakeMoney(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    pl->SendMailResult(mailId, MAIL_MONEY_TAKEN, 0);

    pl->ModifyMoney(m->money);
    m->money = 0;
    m->state = MAIL_STATE_CHANGED;
    pl->m_mailsUpdated = true;

    // save money and mail to prevent cheating
    //sDatabase.BeginTransaction();
    pl->SetUInt32ValueInDB(PLAYER_FIELD_COINAGE,pl->GetMoney(),pl->GetGUID());
    pl->_SaveMail();
    //sDatabase.CommitTransaction();
}

//called when player lists his received mails
void WorldSession::HandleGetMail(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 mailbox;
    recv_data >> mailbox;

    //GameObject* obj = ObjectAccessor::Instance().GetGameObject(_player, mailbox);
    //if(!obj || !obj->IsMailBox())
    //    return;

    Player* pl = _player;

    //load players mails, and mailed items
    if(!pl->m_mailsLoaded)
        pl ->_LoadMail();

    WorldPacket data(SMSG_MAIL_LIST_RESULT, (200));         // guess size
    data << uint8(0);                                       // mail's count
    uint8 mails_count = 0;
    time_t cur_time = time(NULL);
    std::deque<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        // skip deleted or not delivered (deliver delay not expired) mails
        if ((*itr)->state == MAIL_STATE_DELETED || (*itr)->item_guid != 0 && cur_time < (*itr)->deliver_time)
            continue;

        mails_count++;
        data << (uint32) (*itr)->messageID;                 // Message ID

        if((*itr)->messageType == MAIL_GM)
            data << (uint8) 0;
        else
            data << (uint8) (*itr)->messageType;            // Message Type, once = 3

        if((*itr)->messageType == MAIL_AUCTION)
            data << uint32(2);                              // probably auctionhouse id(2==alliance?)
        /*else if((*itr)->messageType == MAIL_CREATURE)
            data << uint32(0);                              // creature entry
        else if((*itr)->messageType == MAIL_GAMEOBJECT)
            data << uint32(0);                              // gameobject entry
        else if((*itr)->messageType == MAIL_ITEM)
            data << uint32(0);                              // item entry*/
        else
            data << (*itr)->sender;                         // SenderID

        if ((*itr)->messageType == MAIL_NORMAL || (*itr)->messageType == MAIL_GM)
            data << (uint32) 0;                             // HIGHGUID_PLAYER

        data << (*itr)->subject.c_str();                    // Subject string - once 00, when mail type = 3
        data << (uint32) (*itr)->itemTextId;                // sure about this
        data << (uint32) 0;                                 // Constant, unknown

        if ((*itr)->messageType == MAIL_NORMAL)
            data << (uint32) 0x29;                          // normal mail
        else if((*itr)->messageType == MAIL_GM)
            data << (uint32) 0x3D;                          // customer support mail
        else
            data << (uint32) 0x3E;                          // auction mail
        // 0x40 - valentine mail?

        uint8 icount = 1;
        Item* it = NULL;
        if ((*itr)->item_guid != 0)
        {
            if(it = pl->GetMItem((*itr)->item_guid))
            {
                                                            //item prototype
                data << it->GetUInt32Value(OBJECT_FIELD_ENTRY);
                icount = it->GetCount();
            }
            else
            {
                sLog.outError("Mail to %s marked as having item (mail item idx: %u), but item not found.",pl->GetName(),(*itr)->item_guid);
                data << (uint32) 0;
            }
        }
        else
            data << (uint32) 0;                             // Any item attached

        // ?? strange code: maybe start from this 6 * (enchant_id,duration,charges) and GetItemRandomPropertyId finally
        data << (uint32) 0;                                 // items count?

        for(uint8 i = 0; i < 6; i++)                        // new 2.0.1
        {
            data << getMSTime();                            // probably time
            if(i == 5)
            {
                data << (uint32) (it ? it->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) : 0);
                data << (uint32) (it ? it->GetItemRandomPropertyId() : 0);
            }
            else
            {
                data << (uint32) 0;
                data << (uint32) 0;
            }
        }

        data << (uint32) 0;                                 // not item->creator, it is enchating?
        data << (uint8)  icount;                            // Attached item stack count
                                                            // sometimes more than zero, not sure when
        int32 charges = (it) ? int32(it->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES)) : 0;
        data << (uint32) charges;                           // item -> charges sure
        uint32 maxDurability = (it) ? it->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) : 0;
        uint32 curDurability = (it) ? it->GetUInt32Value(ITEM_FIELD_DURABILITY) : 0;
        data << maxDurability;                              // MaxDurability
        data << curDurability;                              // Durability
        data << (*itr)->money;                              // Gold
        data << (*itr)->COD;                                // COD
        data << (*itr)->checked;                            // checked
        data << (float)((*itr)->expire_time-time(NULL))/DAY;// Time
        data << (uint32) 0;                                 // Constant, something like end..
    }
    data.put<uint8>(0, mails_count);
    SendPacket(&data);

    // recalculate m_nextMailDelivereTime and unReadMails
    _player->UpdateNextMailTimeAndUnreads();
}

///this function is called when client needs mail message body, or when player clicks on item which has ITEM_FIELD_ITEM_TEXT_ID > 0
void WorldSession::HandleItemTextQuery(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+4+4);

    uint32 itemTextId;
    uint32 mailId;                                          //this value can be item id in bag, but it is also mail id
    uint32 unk;                                             //maybe something like state - 0x70000000

    recv_data >> itemTextId >> mailId >> unk;

    Player* pl = _player;

    //some check needed, if player has item with guid mailId, or has mail with id mailId

    sLog.outDebug("CMSG_ITEM_TEXT_QUERY itemguid: %u, mailId: %u, unk: %u", itemTextId, mailId, unk);

    WorldPacket data(SMSG_ITEM_TEXT_QUERY_RESPONSE, (4+10));// guess size
    data << itemTextId;
    data << objmgr.GetItemText( itemTextId );
    SendPacket(&data);
}

//used when player copies mail body to his inventory
void WorldSession::HandleMailCreateTextItem(WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 mailbox;
    uint32 mailId;

    recv_data >> mailbox >> mailId;

    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || !m->itemTextId || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    Item *bodyItem = new Item;
    if(!bodyItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), MAIL_BODY_ITEM_TEMPLATE, pl))
    {
        delete bodyItem;
        return;
    }

    bodyItem->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , m->itemTextId );
    bodyItem->SetUInt32Value( ITEM_FIELD_CREATOR, m->sender);

    sLog.outDetail("HandleMailCreateTextItem mailid=%u",mailId);

    uint16 dest;
    uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, bodyItem, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->itemTextId = 0;
        m->state = MAIL_STATE_CHANGED;
        pl->m_mailsUpdated = true;

        pl->StoreItem(dest, bodyItem, true);
        //bodyItem->SetState(ITEM_NEW, pl); is set automatically
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, 0);
    }
    else
    {
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_BAG_FULL, msg);
        delete bodyItem;
    }
}

//TODO Fix me! ... this void has probably bad condition, but good data are sent
void WorldSession::HandleMsgQueryNextMailtime(WorldPacket & recv_data )
{
    WorldPacket data;

    data.Initialize(MSG_QUERY_NEXT_MAIL_TIME,4);
    if( _player->unReadMails > 0 )
    {
        data << (uint32) 0;
    }
    else
    {
        data << (uint8) 0x00;
        data << (uint8) 0xC0;
        data << (uint8) 0xA8;
        data << (uint8) 0xC7;
    }
    SendPacket(&data);
}
