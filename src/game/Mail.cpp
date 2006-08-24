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
    WorldPacket data;
    uint64 sender,item;
    std::string receiver,subject,body;
    uint32 unk1,unk2,money,COD,mID;
    recv_data >> sender;
    recv_data >> receiver >> subject >> body;
    recv_data >> unk1 >> unk2;
    recv_data >> item;
    recv_data >> money >> COD;

    sLog.outString("Player %u is sending mail to %s with subject %s and body %s includes item %u and %u copper and %u COD copper",GUID_LOPART(sender),receiver.c_str(),subject.c_str(),body.c_str(),GUID_LOPART(item),money,COD);
    mID = objmgr.GenerateMailID();

    Player* pl = _player;

    WorldPacket tmpData;

    uint64 rc = objmgr.GetPlayerGUIDByName(receiver.c_str());
    if(pl->GetGUID() == rc)
    {
        tmpData.Initialize(SMSG_SEND_MAIL_RESULT);
        tmpData << uint32(0);
        tmpData << uint32(0);
        tmpData << uint32(MAIL_ERR_CANNOT_SEND_TO_SELF);
        SendPacket(&tmpData);
        return;
    }

    if (pl->GetMoney() < money + 30)
    {
        tmpData.Initialize(SMSG_SEND_MAIL_RESULT);
        tmpData << uint32(0);
        tmpData << uint32(0);
        tmpData << uint32(MAIL_ERR_NOT_ENOUGH_MONEY);
        SendPacket(&tmpData);
        return;
    }

    if (!rc)
    {
        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(0);
        data << uint32(0);
        data << uint32(MAIL_ERR_RECIPIENT_NOT_FOUND);
        SendPacket(&data);
        return;
    }

    Player *receive = objmgr.GetPlayer(rc);

    uint32 rc_team = 0;

    if(receive)
        rc_team = receive->GetTeam();
    else
        rc_team = objmgr.GetPlayerTeamByGUID(rc);

    // test the receiver's Faction...
    if (pl->GetTeam() != rc_team)
    {
        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(0);
        data << uint32(0);
        data << uint32(MAIL_ERR_NOT_YOUR_TEAM);
        SendPacket(&data);
        return;
    }

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(0);
    data << uint32(0);
    data << uint32(MAIL_OK);
    SendPacket(&data);

    if (item != 0)
    {
        uint16 pos = pl->GetPosByGuid(item);
        Item *it = pl->GetItemByPos( pos );

        objmgr.RemoveMItem(it->GetGUIDLow());
        objmgr.AddMItem(it);

        std::ostringstream ss;
        ss  << "INSERT INTO `mail_item` (`guid`,`data`) VALUES ("
            << it->GetGUIDLow() << ", '";
        for(uint16 i = 0; i < it->GetValuesCount(); i++ )
        {
            ss << it->GetUInt32Value(i) << " ";
        }
        ss << "' )";
        sDatabase.Execute( ss.str().c_str() );

        pl->RemoveItem( (pos >> 8), (pos & 255), true );
    }
    pl->ModifyMoney( -30 - money );

    time_t etime = base + 3600 * ((COD > 0)? 3 : 30);       //time if COD 3 days, if no COD 30 days
    if (receive)
    {
        Mail* m = new Mail;
        m->messageID = mID;
        m->sender = pl->GetGUIDLow();
        m->receiver = GUID_LOPART(rc);
        m->subject = subject;
        m->body = body;
        m->item = GUID_LOPART(item);
        m->time = etime;
        m->money = money;
        m->COD = COD;
        m->checked = 0;

        receive->AddMail(m);

        data.Initialize(SMSG_RECEIVED_MAIL);
        data << uint32(0);
        SendPacket(&data);
        receive->GetSession()->SendPacket(&data);
    }

    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",mID);
                                                            // there was (long)etime ;-) COD added
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%I64d', '%u', '%u', '%u')", mID, pl->GetGUIDLow(), GUID_LOPART(rc), subject.c_str(), body.c_str(), GUID_LOPART(item), etime, money, COD, 0);
}

void WorldSession::HandleMarkAsRead(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 message;
    recv_data >> mailbox;
    recv_data >> message;
    Player *pl = _player;
    Mail *m = pl->GetMail(message);
    m->checked = 1;
    m->time = time(NULL) + (3 * 3600);
    pl->AddMail(m);
}

void WorldSession::HandleMailDelete(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 message;
    WorldPacket data;
    recv_data >> mailbox;
    recv_data >> message;
    Player *pl = _player;
    Mail *m = pl->GetMail(message);
    if (m->item != 0)
    {
        objmgr.RemoveMItem(m->item);
    }
    pl->RemoveMail(message);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(message);
    data << uint32(MAIL_DELETED);
    data << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleReturnToSender(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 message;
    WorldPacket data;
    recv_data >> mailbox;
    recv_data >> message;
    Player *pl = _player;
    Mail *m = pl->GetMail(message);
    m->receiver = m->sender;
    m->sender = pl->GetGUIDLow();
    m->time = sWorld.GetGameTime() + (30 * 3600);
    m->checked = 0;
    m->COD = 0;
    pl->RemoveMail(message);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(message);
    data << uint32(MAIL_RETURNED_TO_SENDER);
    data << uint32(0);
    SendPacket(&data);

    uint64 rc = m->receiver;
    std::string name;
    objmgr.GetPlayerNameByGUID(rc,name);
    Player *receive = objmgr.GetPlayer(name.c_str());
    if (receive)
    {
        receive->AddMail(m);
    }

    sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",m->messageID);
                                                            //m->checked); //there was (long)m->time...
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u','%u', '%s', '%s', '%u','%I64d','%u','%u','%u')", m->messageID, pl->GetGUIDLow(), m->receiver, m->subject.c_str(), m->body.c_str(), m->item, m->time, m->money, 0, 0);
}

void WorldSession::HandleTakeItem(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 message;
    uint16 dest;
    WorldPacket data;
    recv_data >> mailbox;
    recv_data >> message;
    Player* pl = _player;
    Mail* m = pl->GetMail(message);
    Item *it = objmgr.GetMItem(m->item);

    uint8 msg = _player->CanStoreItem( 0, NULL_SLOT, dest, it, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->item = 0;

        if (m->COD > 0)
        {
            Mail* mn = new Mail;
            mn->messageID = objmgr.GenerateMailID();
            mn->sender = m->receiver;
            mn->receiver = m->sender;
            mn->subject = m->subject;
            mn->body = "Your item sold, player paid a COD";
            mn->item = 0;
            mn->time = time(NULL) + (30 * 3600);
            mn->money = m->COD;
            mn->COD = 0;
            mn->checked = 0;

            Player *receive = objmgr.GetPlayer((uint64)m->sender);

            if (receive)
            {
                WorldPacket data2;
                data2.Initialize(SMSG_RECEIVED_MAIL);       //move this code to function Player::AddMail ;-)
                data2 << uint32(0);
                SendPacket(&data2);
                receive->GetSession()->SendPacket(&data2);
                receive->AddMail(mn);
            }
            sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",mn->messageID);
                                                            //added
            sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%I64d', '%u', '%u', '%u')", mn->messageID, mn->sender, mn->receiver, mn->subject.c_str(), mn->body.c_str(), 0, mn->time, mn->money, 0, 0);
            // client tests, if player has enought money !!!
            pl->ModifyMoney( -int32(m->COD) );
        }
        m->COD = 0;
        pl->AddMail(m);
        _player->StoreItem( dest, it, true);
        objmgr.RemoveMItem(it->GetGUIDLow());
        sDatabase.PExecute("DELETE FROM `mail_item` WHERE `guid` = '%u'", it->GetGUIDLow());
        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(message);
        data << uint32(MAIL_ITEM_TAKEN);
        data << uint32(0);
        SendPacket(&data);
    }
    else
    {
        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(message);
        data << uint32(0);
        data << uint32(MAIL_ERR_BAG_FULL);
        SendPacket(&data);
    }
}

void WorldSession::HandleTakeMoney(WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 mailbox;
    uint32 id;
    recv_data >> mailbox;
    recv_data >> id;
    Player *pl = _player;
    Mail* m = pl->GetMail(id);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(id);
    data << uint32(MAIL_MONEY_TAKEN);
    data << uint32(0);
    SendPacket(&data);

    pl->ModifyMoney(m->money);
    m->money = 0;
    pl->AddMail(m);

}

void WorldSession::HandleGetMail(WorldPacket & recv_data )
{
    uint64 mailbox;
    recv_data >> mailbox;                                   // from mailbox in Storm near bank it was 0x000068A2 (dec 26786) and 0xF0004000 (dec 4026548224)

    WorldPacket data;
    Player* pl = _player;
    data.Initialize(SMSG_MAIL_LIST_RESULT);
    data << uint8(pl->GetMailSize());
    std::list<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        data << uint32((*itr)->messageID);                  // Correct..., also message id , if you change it, server crashes
        data << uint8(0);                                   // Message Type 0 = Default, maybe there will be also Reply
        data << uint32((*itr)->sender);                     // SenderID
        data << uint32(0);                                  // Constant

        data << (*itr)->subject.c_str();                    // Subject string
        if((*itr)->body.c_str()!=NULL)
            data << uint32((*itr)->messageID);              // MessageID!!
        else
            data << uint32(0);                              // No messageID
        data << uint32(0);                                  // Unknown - 0x00000029
        data << uint32(0);                                  // Unknown
        if ((*itr)->item != 0)
        {
            if(Item* i = objmgr.GetMItem((*itr)->item))
                data << uint32(i->GetUInt32Value(OBJECT_FIELD_ENTRY));
            else
            {
                sLog.outError("Mail to %s marked as having item (mail item idx: %u), but item not found.",pl->GetName(),(*itr)->item);
                data << uint32(0);
            }
        }
        else
            data << uint32(0);

        data << uint32(0);                                  // Unknown
        data << uint32(0);                                  // Unknown
        data << uint32(0);                                  // Unknown
        data << uint8(1);                                   // Unknown - Count
        data << uint32(0xFFFFFFFF);                         // Unknown - Charges
        data << uint32(0);                                  // Unknown - MaxDurability
        data << uint32(0);                                  // Unknown - Durability
        data << uint32((*itr)->money);                      // Gold
        data << uint32((*itr)->COD);                        // COD
        data << uint32((*itr)->checked);                    // flags 0: not checked 1: checked 8: COD Payment: "Subject"
        data << float(((*itr)->time - time(NULL)) / 3600);  // Time
        data << uint32(0);                                  // Unknown

    }
    SendPacket(&data);
}

extern char *fmtstring( char *format, ... );

uint32 GetItemGuidFromDisplayID ( uint32 displayID, Player* pl )
{

    uint8 i = 0;
    Item * srcitem;

    for (i = EQUIPMENT_SLOT_START; i < BANK_SLOT_BAG_END; i++)
    {
        srcitem = pl->GetItemByPos( INVENTORY_SLOT_BAG_0, i );

        if (srcitem)
        {
            if (srcitem->GetProto()->DisplayInfoID == displayID)
            {
                break;
            }
        }
    }

    if( i >= BANK_SLOT_BAG_END )
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `entry` FROM `item_template` WHERE `displayid`='%u'", displayID );

        if( !result )
        {
            return (uint8)-1;
        }

        uint32 id = (*result)[0].GetUInt32();
        return id;
        delete result;
    }

    return srcitem->GetProto()->ItemId;
}

extern char *GetInventoryImageFilefromObjectClass(uint32 classNum, uint32 subclassNum, uint32 type, uint32 DisplayID);

bool WorldSession::SendItemInfo( uint32 itemid, WorldPacket data )
{
    //    int i;
    uint32 realID = GetItemGuidFromDisplayID(itemid, _player);
    char const *itemInfo;

    if (realID < 0)
    {
        sLog.outError( "WORLD: Unknown item id 0x%.8X", realID );
        return false;
    }

    ItemPrototype const *itemProto = objmgr.GetItemPrototype(realID);

    if(!itemProto)
    {
        sLog.outError( "WORLD: Unknown item id 0x%.8X", realID );
        return false;
    }

    sLog.outDebug( "WORLD: Real item id is %u. Name %s.", realID, itemProto->Name1 );

    data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
    data << itemid;

    itemInfo = fmtstring("<HTML>\n<BODY>\n");

    itemInfo = fmtstring("%s</P></BODY>\n</HTML>\n", itemInfo);

    data << itemInfo;
    data << uint32(0);
    SendPacket(&data);

    return true;
}

void WorldSession::HandleItemTextQuery(WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 mailguid;
    uint64 unk1;

    recv_data >> mailguid >> unk1;

    Player* pl = _player;

    Mail *itr;

    itr = pl->GetMail(mailguid);
    if(itr)
    {
        sLog.outDebug("We got mailguid: %u with unk: %u", mailguid, unk1);

        data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
        data << mailguid;
        data << itr->body.c_str();
        data << uint32(0);
        SendPacket(&data);
    }

    else
    {
        QueryResult *result = sDatabase.PQuery( "SELECT `text`,`next_page` FROM `item_page` WHERE `id` = '%u'", mailguid );

        if( result )
        {
            data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
            data << mailguid;

            uint32 nextpage = mailguid;

            while (nextpage)
            {
                data << (*result)[0].GetString();
                nextpage = (*result)[1].GetUInt32();
                data << nextpage;
            }

            data << uint32(0);
            SendPacket(&data);
            delete result;
            return;
        }

        if (!SendItemInfo( mailguid, data ))
        {
            data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
            data << mailguid;

            data << "There is no info for this item.";

            data << uint32(0);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleMailCreateTextItem(WorldPacket & recv_data )
{
    uint32 unk1,unk2,mailid;

    recv_data >> unk1 >> unk2 >> mailid;

    sLog.outString("HandleMailCreateTextItem unk1=%u,unk2=%u,mailid=%u",unk1,unk2,mailid);

    WorldPacket Data;

    Item *item = new Item();

    if(!item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), 889, _player))
    {
        delete item;
        return;
    }
    item->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , mailid );

    WorldPacket data;
    data.Initialize(SMSG_SEND_MAIL_RESULT);

    uint16 dest;
    uint8 msg = _player->CanStoreItem( 0, NULL_SLOT, dest, item, false );
    if( msg == EQUIP_ERR_OK )
    {
        _player->StoreItem(dest, item, true);
        data << uint32(mailid);
        data << uint32(MAIL_MADE_PERMANENT);
        data << uint32(0);
    }
    else
    {
        delete item;
        _player->SendEquipError( msg, item, NULL );
        data << uint32(mailid);
        data << uint32(0);
        data << uint32(MAIL_ERR_INTERNAL_ERROR);
    }

    SendPacket(&data);
}

void WorldSession::HandleMsgQueryNextMailtime(WorldPacket & recv_data )
{

    WorldPacket Data;
    bool checkmail=false;
    Player *pl=_player;

    std::list<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        if(!(*itr)->checked)
        {
            checkmail = true;
            break;
        }
    }

    if ( checkmail )
    {
        Data.Initialize(MSG_QUERY_NEXT_MAIL_TIME);
        Data << uint32(0);
        SendPacket(&Data);
    }
    else
    {
        Data.Initialize(MSG_QUERY_NEXT_MAIL_TIME);
        Data << uint8(0x00);
        Data << uint8(0xC0);
        Data << uint8(0xA8);
        Data << uint8(0xC7);
        SendPacket(&Data);
    }

}
