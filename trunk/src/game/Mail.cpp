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
    time_t etime = base + (30 * 3600);
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
    uint32 tmpMoney = pl->GetUInt32Value(PLAYER_FIELD_COINAGE);

    if (tmpMoney - money < 30)
    {
        tmpData.Initialize(SMSG_SEND_MAIL_RESULT);
        tmpData << uint32(0);
        tmpData << uint32(0);
        tmpData << uint32(3);
        SendPacket(&tmpData);
    }
    else
    {

        QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s';", receiver.c_str());

        Player *receive = objmgr.GetPlayer(receiver.c_str());
        uint64 rc = objmgr.GetPlayerGUIDByName(receiver.c_str());
        if (result)
        {
            delete result;
            data.Initialize(SMSG_SEND_MAIL_RESULT);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            SendPacket(&data);

            if (item != 0)
            {
                uint16 pos = pl->GetPosByGuid(item);
                Item *it = pl->GetItemByPos( pos );

                objmgr.AddMItem(it);

                std::stringstream ss;
                ss << "INSERT INTO `mail_item` (`guid`,`data`) VALUES ("
                    << it->GetGUIDLow() << ", '";
                for(uint16 i = 0; i < it->GetValuesCount(); i++ )
                {
                    ss << it->GetUInt32Value(i) << " ";
                }
                ss << "' )";
                sDatabase.Execute( ss.str().c_str() );

                pl->RemoveItem( (pos >> 8), (pos & 255), true );
            }
            uint32 playerGold = pl->GetUInt32Value(PLAYER_FIELD_COINAGE);
            pl->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - 30 - money );

            if (receive)
            {
                Mail* m = new Mail;
                m->messageID = mID;
                m->sender =   pl->GetGUIDLow();
                m->receiver = GUID_LOPART(rc);
                m->subject = subject;
                m->body = body;
                m->item = GUID_LOPART(item);
                m->money = money;
                m->time = etime;
                m->COD = 0;
                m->checked = 0;
                receive->AddMail(m);

                data.Initialize(SMSG_RECEIVED_MAIL);
                data << uint32(0);
                SendPacket(&data);
                receive->GetSession()->SendPacket(&data);
            }

            sDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'",mID);
            sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '%u', '%u');", mID, pl->GetGUIDLow(), GUID_LOPART(rc), subject.c_str(), body.c_str(), GUID_LOPART(item), (long)etime, money, 0, 0);

        }
        else
        {
            data.Initialize(SMSG_SEND_MAIL_RESULT);
            data << uint32(0);
            data << uint32(0);
            data << uint32(4);
            SendPacket(&data);
        }
    }
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
    data << uint32(4);
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
    pl->RemoveMail(message);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(message);
    data << uint32(3);
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
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`sender`,`receiver`,`subject`,`body`,`item`,`time`,`money`,`cod`,`checked`) VALUES ('%u', '%u','%u', '%s', '%s', '%u','%u','%u','%u','%u');", m->messageID, pl->GetGUIDLow(), m->receiver, m->subject.c_str(), m->body.c_str(), m->item, (long)m->time, m->money, 0, m->checked);

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
        pl->AddMail(m);
        _player->StoreItem( dest, it, true);
        objmgr.RemoveMItem(it->GetGUIDLow());
        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(message);
        data << uint32(2);
        data << uint32(0);
        SendPacket(&data);
    }
    else
        _player->SendEquipError( msg, it, NULL );
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
    uint32 money = pl->GetUInt32Value(PLAYER_FIELD_COINAGE);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(id);
    data << uint32(1);
    data << uint32(0);
    SendPacket(&data);

    pl->SetUInt32Value(PLAYER_FIELD_COINAGE,money + m->money);
    m->money = 0;
    pl->AddMail(m);

}

void WorldSession::HandleGetMail(WorldPacket & recv_data )
{
    uint32 info;
    recv_data >> info;
    WorldPacket data;
    Player* pl = _player;
    data.Initialize(SMSG_MAIL_LIST_RESULT);
    data << uint8(pl->GetMailSize());
    std::list<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        data << uint32((*itr)->messageID);
        data << uint8(0);
        data << uint32((*itr)->sender);
        data << uint32(0);

        data << (*itr)->subject.c_str();
        if((*itr)->body.c_str()!=NULL)
            data << uint32((*itr)->messageID);
        else
            data << uint32(0);
        data << uint32(0);
        data << uint32(41);
        if ((*itr)->item != 0)
        {
            Item* i = objmgr.GetMItem((*itr)->item);
            data << uint32(i->GetUInt32Value(OBJECT_FIELD_ENTRY));
        }
        else
        {
            data << uint32(0);
        }
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint8(1);
        data << uint32(0xFFFFFFFF);
        data << uint32(0);
        data << uint32(0);
        data << uint32((*itr)->money);
        data << uint32((*itr)->COD);
        data << uint32((*itr)->checked);

        data << float(((*itr)->time - time(NULL)) / 3600);
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
    bool resist_added = false;
    bool names_added = false;

    if (realID < 0)
    {
        sLog.outError( "WORLD: Unknown item id 0x%.8X", realID );
        return false;
    }

    ItemPrototype *itemProto = objmgr.GetItemPrototype(realID);

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
        QueryResult *result = sDatabase.PQuery( "SELECT * FROM `item_page` WHERE `id` = '%u'", mailguid );

        if( result )
        {
            data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
            data << mailguid;

            uint32 nextpage = mailguid;

            while (nextpage)
            {
                data << (*result)[1].GetString();
                nextpage = (*result)[2].GetUInt32();
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

    uint32 sbit2=5;
    bool   slotfree=false;
    WorldPacket Data;

    Item *item = new Item();

    if(!item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), 889, _player))
        return;
    item->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , mailid );

    uint16 dest;
    uint8 msg = _player->CanStoreItem( 0, NULL_SLOT, dest, item, false );
    if( msg == EQUIP_ERR_OK )
        _player->StoreItem(dest, item, true);
    else
        _player->SendEquipError( msg, item, NULL );
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
