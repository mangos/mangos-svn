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
#include "Chat.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Unit.h"

void WorldSession::HandleSendMail(WorldPacket & recv_data )
{
    time_t base = time(NULL);
    uint64 mailbox,itemId;
    std::string receiver,subject,body;
    uint32 unk1,unk2,money,COD,mailId;
    recv_data >> mailbox;
    recv_data >> receiver >> subject >> body;
    recv_data >> unk1 >> unk2;                              // 0x29 and 0x00
    recv_data >> itemId;
    recv_data >> money >> COD;                              //then there are two (uint32) 0;

    if (receiver.size() == 0)
        return;
    normalizePlayerName(receiver);
    sDatabase.escape_string(receiver);                      // prevent SQL injection - normal name don't must changed by this call

    Player* pl = _player;

    sLog.outDetail("Player %u is sending mail to %s with subject %s and body %s includes item %u, %u copper and %u COD copper with unk1 = %u, unk2 = %u",pl->GetGUIDLow(),receiver.c_str(),subject.c_str(),body.c_str(),GUID_LOPART(itemId),money,COD,unk1,unk2);

    uint64 rc = objmgr.GetPlayerGUIDByName(receiver.c_str());
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
        QueryResult* result = sDatabase.PQuery("SELECT COUNT(*) FROM `mail` WHERE `receiver` = '%u'", GUID_LOPART(rc));
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
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && pl->GetTeam() != rc_team)
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

    uint32 itemPageId = 0;
    if (body.size() > 0)
    {
        itemPageId = objmgr.CreateItemPage( body );
    }
    if (pItem)
    {
        //item reminds in item_instance table already, used it in mail now
        pl->RemoveItem( (item_pos >> 8), (item_pos & 255), true );
        pItem->RemoveFromUpdateQueueOf( pl );

        sDatabase.BeginTransaction();
        pItem->DeleteFromInventoryDB();                     //deletes item from character's inventory
        pItem->SaveToDB();                                  // recursive and not have transaction guard into self
        sDatabase.CommitTransaction();
    }
    uint32 messagetype = 0;
    pl->ModifyMoney( -30 - money );
    uint32 item_template = pItem ? pItem->GetEntry() : 0;   //item prototype
    mailId = objmgr.GenerateMailID();
    time_t etime = time(NULL) + DAY * ((COD > 0)? 3 : 30);  //time if COD 3 days, if no COD 30 days
    if (receive)
    {
        receive->CreateMail(mailId, messagetype, pl->GetGUIDLow(), subject, itemPageId, GUID_LOPART(itemId), item_template, etime, money, COD, NOT_READ, pItem);
    }
    else if (pItem)
        delete pItem;                                       //item is sent, but receiver isn't online .. so remove it from RAM
    // backslash all '
    sDatabase.escape_string(subject);
    //not needed : sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",mID);
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%u', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "', '%u', '%u', '0')",
        mailId, messagetype, pl->GetGUIDLow(), GUID_LOPART(rc), subject.c_str(), itemPageId, GUID_LOPART(itemId), item_template, (uint64)etime, money, COD);
    //there is small problem:
    //one player sends big sum of money to another player, then server crashes, but mail with money is
    //is DB .. so receiver will receive money, BUT sender has also that money..., we will have to save also players money...

    // save to db
    pl->SaveToDB();
}

void WorldSession::HandleMarkAsRead(WorldPacket & recv_data )
{
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
        m->time = time(NULL) + (3 * DAY);
        pl->m_mailsUpdated = true;
        m->state = CHANGED;
    }
}

void WorldSession::HandleMailDelete(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player* pl = _player;
    pl->m_mailsUpdated = true;
    Mail *m = pl->GetMail(mailId);
    if(m)
        m->state = DELETED;
    pl->SendMailResult(mailId, MAIL_DELETED, 0);
}

void WorldSession::HandleReturnToSender(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player *pl = _player;
    Mail *m = pl->GetMail(mailId);
    if(!m || m->state == DELETED)
    {
        pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    //we can return mail now
    //so firstly delete the old one
    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", mailId);
    pl->RemoveMail(mailId);

    uint32 messageID = objmgr.GenerateMailID();

    Item *pItem = NULL;
    if (m->item_guid)
    {
        pItem = pl->GetMItem(m->item_guid);
        pl->RemoveMItem(m->item_guid);
    }
    time_t etime = time(NULL) + 30*DAY;
    Player *receiver = objmgr.GetPlayer((uint64)m->sender);
    if(receiver)
        receiver->CreateMail(messageID,0,m->receiver,m->subject,m->itemPageId,m->item_guid,m->item_template,etime,m->money,0,RETURNED_CHECKED,pItem);
    else if ( pItem )
        delete pItem;

    std::string subject;
    subject = m->subject;
    sDatabase.escape_string(subject);
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '0', '%u', '%u', '%s', '%u', '%u', '%u', '" I64FMTD "', '%u', '0', '16')",
        messageID, m->receiver, m->sender, subject.c_str(), m->itemPageId, m->item_guid, m->item_template, (uint64)etime, m->money);
    delete m;                                               //we can deallocate old mail
    pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, 0);
}

void WorldSession::HandleTakeItem(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 mailId;
    uint16 dest;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player* pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == DELETED)
    {
                                                            //not sure
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

        if (m->COD > 0)
        {
            Player *receive = objmgr.GetPlayer(MAKE_GUID(m->sender,HIGHGUID_PLAYER));
            time_t etime = time(NULL) + (30 * DAY);
            uint32 newMailId = objmgr.GenerateMailID();
            if (receive)
                receive->CreateMail(newMailId, 0, m->receiver, m->subject, 0, 0, 0, etime, m->COD, 0, COD_PAYMENT_CHECKED, NULL);

            //escape apostrophes
            std::string subject = m->subject;
            sDatabase.escape_string(subject);
            sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemPageId`,`item_guid`,`item_template`,`time`,`money`,`cod`,`checked`) "
                "VALUES ('%u', '0', '%u', '%u', '%s', '0', '0', '0', '" I64FMTD "', '%u', '0', '8')",
                newMailId, m->receiver, m->sender, subject.c_str(), (uint64)etime, m->COD);

            pl->ModifyMoney( -int32(m->COD) );
        }
        m->COD = 0;
        m->state = CHANGED;
        pl->m_mailsUpdated = true;
        pl->RemoveMItem(it->GetGUIDLow());
        Item* it2 = pl->StoreItem( dest, it, true);
        if(it2 == it)                                       // only set if not merged to existed stack (*it can be deleted already but we can compare pointers any way)
            it->SetState(ITEM_NEW, pl);
        pl->ItemAddedQuestCheck(it2->GetEntry(),it2->GetCount());

        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, 0);
    } else
                                                            //works great
    pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_BAG_FULL, msg);
}

void WorldSession::HandleTakeMoney(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 mailId;
    recv_data >> mailbox;
    recv_data >> mailId;
    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == DELETED)
        return;

    pl->SendMailResult(mailId, MAIL_MONEY_TAKEN, 0);

    pl->ModifyMoney(m->money);
    m->money = 0;
    m->state = CHANGED;
    pl->m_mailsUpdated = true;
}

void WorldSession::HandleGetMail(WorldPacket & recv_data )
{
    uint64 mailbox;
    recv_data >> mailbox;

    Player* pl = _player;

    //load players mails, and mailed items
    if(!pl->m_mailsLoaded)
        pl ->_LoadMail();

    WorldPacket data(SMSG_MAIL_LIST_RESULT, (200));         // guess size
    data << uint8(0);
    uint8 mails_count = 0;
    std::deque<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        if ((*itr)->state == DELETED)
            continue;
        mails_count++;
        data << (uint32) (*itr)->messageID;
        data << (uint8)  (*itr)->messageType;               // Message Type, once = 3
        data << (uint32) (*itr)->sender;                    // SenderID
        if ((*itr)->messageType == 0)
            data << (uint32) 0;                             // HIGHGUID_PLAYER
        data << (*itr)->subject.c_str();                    // Subject string - once 00, when mail type = 3
        data << (uint32) (*itr)->itemPageId;                // sure about this
        data << (uint32) 0;                                 // Constant
        if ((*itr)->messageType == 0)
            data << (uint32) 0x29;                          // Constant
        else
            data << (uint32) 0x3E;
        uint8 icount = 1;
        Item* it = NULL;
        if ((*itr)->item_guid != 0)
        {
            if(it = pl->GetMItem((*itr)->item_guid))
            {
                                                            //item prototype
                data << (uint32) it->GetUInt32Value(OBJECT_FIELD_ENTRY);
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

        data << (uint32) 0;                                 // Unknown Constant 0
        data << (uint32) 0;                                 // Unknown
        data << (uint32) 0;                                 // not item->creator, it is enchating?
        data << (uint8)  icount;                            // Attached item stack count
                                                            //sometimes more than zero, not sure when
        int32 charges = (it) ? int32(it->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES)) : 0;
        data << (uint32) charges;                           // item -> charges sure
        uint32 maxDurability = (it)? it->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) : 0;
        uint32 curDurability = (it)? it->GetUInt32Value(ITEM_FIELD_DURABILITY) : 0;
        data << (uint32) maxDurability;                     // MaxDurability
        data << (uint32) curDurability;                     // Durability
        data << (uint32) (*itr)->money;                     // Gold
        data << (uint32) (*itr)->COD;                       // COD
        data << (uint32) (*itr)->checked;                   // checked
        data << (float)  ((*itr)->time - time(NULL)) / DAY; // Time
        data << (uint32) 0;                                 // Constant, something like end..
    }
    data.put<uint8>(0, mails_count);
    SendPacket(&data);
}

//TO DO FIXME, for mails and auction mails this function works
void WorldSession::HandleItemTextQuery(WorldPacket & recv_data )
{
    uint32 itemPageId;
    uint32 mailId;                                          //this value can be item id in bag, but it is also mail id
    uint32 unk;                                             //maybe something like state - 0x70000000

    recv_data >> itemPageId >> mailId >> unk;

    Player* pl = _player;

    //some check needed, if player has item with guid mailId, or has mail with id mailId

    sLog.outDebug("CMSG_ITEM_TEXT_QUERY itemguid: %u, mailId: %u, unk: %u", itemPageId, mailId, unk);

    WorldPacket data(SMSG_ITEM_TEXT_QUERY_RESPONSE, (4+10));// guess size
    data << itemPageId;
    /*QueryResult *result = sDatabase.PQuery( "SELECT `text` FROM `item_page` WHERE `id` = '%u'", itemPageId );
    if (result) {
        Field *fields = result->Fetch();
        data << fields[0].GetCppString();
    } else {
        data << "There is no info for this item.";
    }*/
    data << objmgr.GetItemPage( itemPageId );
    SendPacket(&data);
}

void WorldSession::HandleMailCreateTextItem(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 mailId;

    recv_data >> mailbox >> mailId;

    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || !m->itemPageId || m->state == DELETED)
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

    bodyItem->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , m->itemPageId );
    bodyItem->SetUInt32Value( ITEM_FIELD_CREATOR, m->sender);

    sLog.outDetail("HandleMailCreateTextItem mailid=%u",mailId);

    uint16 dest;
    uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, bodyItem, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->itemPageId = 0;
        m->state = CHANGED;
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
