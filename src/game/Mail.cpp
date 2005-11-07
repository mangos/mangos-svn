/* Mail.cpp
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#include "Unit.h"

void WorldSession::HandleSendMail(WorldPacket & recv_data )
{
    time_t base = time(NULL);
    time_t etime = base + (30 * 3600);
    WorldPacket data;
    uint64 sender,item;
    std::string reciever,subject,body;
    uint32 unk1,unk2,money,COD,mID;
    recv_data >> sender;
    recv_data >> reciever >> subject >> body;
    recv_data >> unk1 >> unk2;
    recv_data >> item;
    recv_data >> money >> COD;

    Log::getSingleton().outString("Player %u is sending mail to %s with subject %s and body %s includes item %u and %u copper and %u COD copper",GUID_LOPART(sender),reciever.c_str(),subject.c_str(),body.c_str(),GUID_LOPART(item),money,COD);
    mID = objmgr.GenerateMailID();

    Player* pl = GetPlayer();

    WorldPacket tmpData;                    
    uint32 tmpMoney = pl->GetUInt32Value(PLAYER_FIELD_COINAGE); //get player money

    if (tmpMoney - money < 30)    //add by vendy
    {             
        
        tmpData.Initialize(SMSG_SEND_MAIL_RESULT);
        tmpData << uint32(0);
        tmpData << uint32(0);
        tmpData << uint32(3);
        SendPacket(&tmpData);    //not enough money
           
    }
    else
    {

        data.Initialize(SMSG_SEND_MAIL_RESULT);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        SendPacket(&data);
        
        if (item != 0)
        {
            uint32 slot = pl->GetSlotByItemGUID(item);
            Item *it = pl->GetItemBySlot((uint8)slot);
            objmgr.AddMItem(it);

            std::stringstream ss;
            ss << "INSERT INTO mailed_items (guid, data) VALUES ("
               << it->GetGUIDLow() << ", '";         // TODO: use full guids
            for(uint16 i = 0; i < it->GetValuesCount(); i++ )
            {
               ss << it->GetUInt32Value(i) << " ";
            }
            ss << "' )";
            sDatabase.Execute( ss.str().c_str() );

            pl->RemoveItemFromSlot((uint8)slot);
        }
        uint32 playerGold = pl->GetUInt32Value(PLAYER_FIELD_COINAGE);
        pl->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - 30 - money );
        uint64 rc = objmgr.GetPlayerGUIDByName(reciever.c_str());
        Player *recieve = objmgr.GetPlayer(reciever.c_str());
        if (recieve)
        {
            Mail* m = new Mail;
            m->messageID = mID;
            m->sender =   pl->GetGUIDLow();  
            m->reciever = GUID_LOPART(rc);
            m->subject = subject;
            m->body = body;
            m->item = GUID_LOPART(item);
            m->money = money;
            m->time = etime;
            m->COD = 0;
            m->checked = 0;
            recieve->AddMail(m);
        }

        std::stringstream delinvq;
        // TODO: use full guids
        delinvq << "DELETE FROM mail WHERE mailID = " << mID;
        sDatabase.Execute( delinvq.str().c_str( ) );
        std::stringstream ss;
        ss << "INSERT INTO mail (mailId,sender,reciever,subject,body,item,time,money,COD,checked) VALUES ( " <<
            mID << ", " << pl->GetGUIDLow() << ", " << GUID_LOPART(rc) << ",' " << subject.c_str() << "' ,' " <<
        body.c_str() << "', " << GUID_LOPART(item) << ", " << (long)etime << ", " << money << ", " << 0 << ", " << 0 << " )";
        sDatabase.Execute( ss.str().c_str( ) );
    }
}


void WorldSession::HandleMarkAsRead(WorldPacket & recv_data )
{
    uint64 mailbox;
    uint32 message;
    recv_data >> mailbox;
    recv_data >> message;
    Player *pl = GetPlayer();
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
    Player *pl = GetPlayer();
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
    Player *pl = GetPlayer();
    Mail *m = pl->GetMail(message);
    m->reciever = m->sender;
    m->sender = pl->GetGUIDLow();
    m->time = sWorld.GetGameTime() + (30 * 3600);
    m->checked = 0;
    pl->RemoveMail(message);

    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(message);
    data << uint32(3);
    data << uint32(0);
    SendPacket(&data);

    uint64 rc;
    GUID_LOPART(rc) = m->reciever;
    GUID_HIPART(rc) = 0;
    std::string name;
    objmgr.GetPlayerNameByGUID(rc,name);
    Player *recieve = objmgr.GetPlayer(name.c_str());
    if (recieve)
    {
        recieve->AddMail(m);
    }

    std::stringstream delinvq;
    // TODO: use full guids
    delinvq << "DELETE FROM mail WHERE mailID = " << m->messageID;
    sDatabase.Execute( delinvq.str().c_str( ) );
    std::stringstream ss;
    ss << "INSERT INTO mail (mailId,sender,reciever,subject,body,item,time,money,COD,checked) VALUES ( " <<
        m->messageID << ", " << pl->GetGUIDLow() << ", " << m->reciever << ",' " << m->subject.c_str() << "' ,' " <<
        m->body.c_str() << "', " << m->item << ", " << (long)m->time << ", " << m->money << ", " << 0 << ", " << m->checked << " )";
    sDatabase.Execute( ss.str().c_str( ) );

}


void WorldSession::HandleTakeItem(WorldPacket & recv_data )
{
    uint8 i,slot;
    uint64 mailbox;
    uint32 message;
    WorldPacket data;
    recv_data >> mailbox;
    recv_data >> message;
    Player* pl = GetPlayer();
    Mail* m = pl->GetMail(message);
    Item *it = objmgr.GetMItem(m->item);
    m->item = 0;
    pl->AddMail(m);
    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if (GetPlayer()->GetItemBySlot(i) == NULL)
        {
            slot = i;
            break;
        }
    }
    it->SetUInt64Value(ITEM_FIELD_CONTAINED,pl->GetGUID());
    it->SetUInt64Value(ITEM_FIELD_OWNER,pl->GetGUID());
    GetPlayer()->AddItemToSlot(slot, it);
    objmgr.RemoveMItem(it->GetGUIDLow());
    data.Initialize(SMSG_SEND_MAIL_RESULT);
    data << uint32(message);
    data << uint32(2);
    data << uint32(0);
    SendPacket(&data);
}


void WorldSession::HandleTakeMoney(WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 mailbox;
    uint32 id;
    recv_data >> mailbox;
    recv_data >> id;
    Player *pl = GetPlayer();
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
    Player* pl = GetPlayer();
    data.Initialize(SMSG_MAIL_LIST_RESULT);
    data << uint8(pl->GetMailSize());
    std::list<Mail*>::iterator itr;
    for (itr = pl->GetmailBegin(); itr != pl->GetmailEnd();itr++)
    {
        data << uint32((*itr)->messageID);
        data << uint8(0);
        data << uint32((*itr)->sender);
        data << uint32(0);                        //sender high GUID
      
        data << (*itr)->subject.c_str();
        if((*itr)->body.c_str()!=NULL)            //do we have a body?
            data << uint32((*itr)->messageID);
        else
            data << uint32(0);                    //guess not
        data << uint32(0);                        //gift 2 = gift
        data << uint32(41);                       //unk4
        if ((*itr)->item != 0)
        {
            Item* i = objmgr.GetMItem((*itr)->item);
            data << uint32(i->GetUInt32Value(OBJECT_FIELD_ENTRY));
        }
        else
        {
            data << uint32(0);
        }
        data << uint32(0);                        //item random property 1
        data << uint32(0);                        //item random property 2
        data << uint32(0);                        //unk7
        data << uint8(1);                         //count
        data << uint32(0xFFFFFFFF);               //charges
        data << uint32(0);                        //max durability
        data << uint32(0);                        //durability
        data << uint32((*itr)->money);            //money
        data << uint32((*itr)->COD);              //COD check
        data << uint32((*itr)->checked);          //read check
        //time
        data << float(((*itr)->time - time(NULL)) / 3600);
    }
    SendPacket(&data);
}

extern char *fmtstring( char *format, ... );

uint32 GetItemGuidFromDisplayID ( uint32 displayID, Player* pl )
{
	// UQ1: I will extend this to actually look for the item in inventory.. In case we have an item that shares a 
	// displayID with another item...

	/*
	QueryResult *result = sDatabase.Query( fmtstring("SELECT entry FROM items WHERE displayid='%u'", displayID) );

    if( !result )
        return -1;

	uint32 id = (*result)[0].GetUInt32();
	*/

	uint8 i = 0;
	Item * srcitem;

	for (i = EQUIPMENT_SLOT_START; i < BANK_SLOT_BAG_END; i++)
	{
		srcitem = pl->GetItemBySlot(i);

		if (srcitem)
		{
			if (srcitem->GetItemProto()->DisplayInfoID == displayID)
			{// We found the item... (I hope hehehe)
				break;
			}
		}
	}

    if( i >= BANK_SLOT_BAG_END )
	{// Didn't find it! Try a normal DB lookup...
        QueryResult *result = sDatabase.Query( fmtstring("SELECT entry FROM items WHERE displayid='%u'", displayID) );

		if( !result )
		{// Big time failure! There is no item matching this!!!
	        return -1;
		}

		uint32 id = (*result)[0].GetUInt32();
		return id;
	}

	return srcitem->GetItemProto()->ItemId;
}

bool WorldSession::SendItemInfo( uint32 itemid, WorldPacket data )
{// UQ1: Generate item info page text for an item...
    int i;
	Player* pl = GetPlayer();
	uint32 realID = GetItemGuidFromDisplayID(itemid, pl);
	char *itemInfo;
	bool resist_added = false;
	bool names_added = false;

	if (realID < 0)
	{
        Log::getSingleton( ).outError( "WORLD: Unknown item id 0x%.8X", realID );
        return false;
    }

    ItemPrototype *itemProto = objmgr.GetItemPrototype(realID);
    
	if(!itemProto)
    {
        Log::getSingleton( ).outError( "WORLD: Unknown item id 0x%.8X", realID );
        return false;
    }

	Log::getSingleton( ).outDebug( "WORLD: Real item id is %u. Name %s.", realID, itemProto->Name1.c_str() );

	data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
    data << itemid;

	//itemInfo = (fmtstring("<HTML>\n\r<BODY>\n\r<H1>Name: %s</H1>\n\n", itemProto->Name1.c_str()));
	itemInfo = (fmtstring("Name: %s\n\n", itemProto->Name1.c_str()));

    if (stricmp(itemProto->Name2.c_str(), "") && stricmp(itemProto->Name2.c_str(), itemProto->Name1.c_str()))
    {
        itemInfo = (fmtstring("%s%s\n", itemInfo, itemProto->Name2.c_str()));
		names_added = true;
    }

    if (stricmp(itemProto->Name3.c_str(), "") && stricmp(itemProto->Name3.c_str(), itemProto->Name2.c_str()))
    {
        itemInfo = (fmtstring("%s%s\n", itemInfo, itemProto->Name3.c_str()));
		names_added = true;
    }

    if (stricmp(itemProto->Name4.c_str(), "") && stricmp(itemProto->Name4.c_str(), itemProto->Name3.c_str()))
    {
        itemInfo = (fmtstring("%s%s\n", itemInfo, itemProto->Name4.c_str()));
		names_added = true;
    }

	if (names_added)
		itemInfo = (fmtstring("%s\n", itemInfo)); //New line..

    if (stricmp(itemProto->Description.c_str(), ""))
    {
		itemInfo = (fmtstring("%sDescription: %s\n", itemInfo, itemProto->Description.c_str()));
		itemInfo = (fmtstring("%s\n", itemInfo)); //New line..
    }

	if (itemProto->Bonding)
		itemInfo = (fmtstring("%sThis is a bonding item.\n", itemInfo));

    itemInfo = (fmtstring("%sQuality: %u out of 5.\n", itemInfo, itemProto->Quality));
	itemInfo = (fmtstring("%sMaximum Durability: %u.\n", itemInfo, itemProto->MaxDurability));

	uint32 min_damage = 0, max_damage = 0;

    for(i = 0; i < 5; i++)
    {// UQ1: Need to add a damage type here...
        min_damage += (uint32)itemProto->DamageMin[i];
        max_damage += (uint32)itemProto->DamageMax[i];
    }

	if (min_damage > 0 || max_damage > 0)
		itemInfo = (fmtstring("%sMinimum Damage: %u.\nMaximum Damage: %u.\n", itemInfo, min_damage, max_damage));

    itemInfo = (fmtstring("%sSell Price: %u.\n", itemInfo, itemProto->SellPrice));
	itemInfo = (fmtstring("%s\n", itemInfo)); //New line..

    itemInfo = (fmtstring("%sLevel: %u.\n", itemInfo, itemProto->ItemLevel));
	itemInfo = (fmtstring("%sRequired Character Level: %u.\n", itemInfo, itemProto->RequiredLevel));
	itemInfo = (fmtstring("%s\n", itemInfo)); //New line..

    if (itemProto->ContainerSlots)
	{
		itemInfo = (fmtstring("%sThis item is a container, and will hold %u items.\n", itemInfo, itemProto->ContainerSlots));
		itemInfo = (fmtstring("%s\n", itemInfo)); //New line..
	}

	//itemInfo = (fmtstring("%s\n", itemInfo)); //New line..
    
	if (itemProto->Armor > 0)
	{
		itemInfo = (fmtstring("%sArmor Bonus: %u.\n", itemInfo, itemProto->Armor));
		itemInfo = (fmtstring("%s\n", itemInfo)); //New line..
	}

	if (itemProto->HolyRes > 0)
	{
		itemInfo = (fmtstring("%sHoly Resistance Bonus: %u.\n", itemInfo, itemProto->HolyRes));
		resist_added = true;
	}

	if (itemProto->FireRes > 0)
	{
		itemInfo = (fmtstring("%sFire Resistance Bonus: %u.\n", itemInfo, itemProto->FireRes));
		resist_added = true;
	}

	if (itemProto->NatureRes > 0)
	{
		itemInfo = (fmtstring("%sNature Resistance Bonus: %u.\n", itemInfo, itemProto->NatureRes));
		resist_added = true;
	}

	if (itemProto->FrostRes > 0)
	{
		itemInfo = (fmtstring("%sFrost Resistance Bonus: %u.\n", itemInfo, itemProto->FrostRes));
		resist_added = true;
	}

	if (itemProto->ShadowRes > 0)
	{
		itemInfo = (fmtstring("%sShadow Resistance Bonus: %u.\n", itemInfo, itemProto->ShadowRes));
		resist_added = true;
	}

	if (itemProto->ArcaneRes > 0)
	{
		itemInfo = (fmtstring("%sArcane Resistance Bonus: %u.\n", itemInfo, itemProto->ArcaneRes));
		resist_added = true;
	}

	if (resist_added)
		itemInfo = (fmtstring("%s\n", itemInfo)); //New line..
    
	itemInfo = (fmtstring("%sAttack Delay: %u.\n", itemInfo, itemProto->Delay));

	//itemInfo = (fmtstring("%s\n\r</BODY>\n\r</HTML>\n\r", itemInfo)); //New line..

	data << itemInfo;
	data << uint32(0);
    SendPacket(&data);  

	//Log::getSingleton( ).outDebug( "Item %u info is:\n%s", realID, itemInfo );

	return true;
}

void WorldSession::HandleItemTextQuery(WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 mailguid;
    uint64 unk1;

    recv_data >> mailguid >> unk1;

    Player* pl = GetPlayer();
    // std::list<Mail*>::iterator itr;
    Mail *itr;
    // for (itr = pl->GetmailBegin(); (*itr)->messageID != mailguid && itr != pl->GetmailEnd() ;itr++) ;
    itr = pl->GetMail(mailguid);
    if(itr)   
    {
		Log::getSingleton().outDebug("We got mailguid: %d with unk: %d", mailguid, unk1);

        data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
        data << mailguid;
        data << itr->body.c_str();
        data << uint32(0);
        SendPacket(&data);
    }
    else
	{// UQ1: This is an item info text page...
        //Log::getSingleton().outError("We got mailguid: %d but there is no such mail.",mailguid);
		//data << "Hi, We got a mailguid,  but there is no such mail";
		QueryResult *result = sDatabase.Query( fmtstring("SELECT * FROM item_pages WHERE id='%u'", mailguid) );

		if( result )
		{// UQ1: We have a text page for this item in the DB... Use it!
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
	        return;
		}
		
		// UQ1: Make a generic item info text page for the item instead...
		if (!SendItemInfo( mailguid, data ))
		{// If SendItemInfo failed, send a generic text...
			data.Initialize(SMSG_ITEM_TEXT_QUERY_RESPONSE);
			data << mailguid;

			data << "There is no info for this item.";

			data << uint32(0);
			SendPacket(&data);  
		}
    }
}
//add by  vendy
void WorldSession::HandleMailCreateTextItem(WorldPacket & recv_data )
{
    uint32 unk1,unk2,mailid;

    recv_data >> unk1 >> unk2 >> mailid;

    Log::getSingleton().outString("HandleMailCreateTextItem unk1=%d,unk2=%d,mailid=%d",unk1,unk2,mailid);

    uint32 sbit2=5;
    bool   slotfree=false;
    WorldPacket Data;
    uint8 i,slot;

    Player* pl = GetPlayer();

    for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if (GetPlayer()->GetItemBySlot(i) == NULL)
        {
            slot = i;
            slotfree=true;
            break;
        }
    }
    if (slotfree)
    {
        Item *item = new Item();
        
        item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), 889, GetPlayer());
        item->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , mailid );

        GetPlayer()->AddItemToSlot( slot, item );

        Data.Initialize(SMSG_SEND_MAIL_RESULT);
        Data << uint32(mailid);
        Data << uint32(sbit2);
        Data << uint32(0);
        SendPacket(&Data);    //delete mail copy
    }
    else
    { 
        Data.Initialize(SMSG_SEND_MAIL_RESULT);
        Data << uint32(mailid);
        Data << uint32(0);
        Data << uint32(1);
        SendPacket(&Data);   //error ,bag is full
    }
    
}

