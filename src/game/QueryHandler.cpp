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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "ObjectAccessor.h"
#include "Pet.h"

void WorldSession::SendNameQueryOpcode(Player *p)
{
    if(!p)
        return;

                                                            // guess size
    WorldPacket data( SMSG_NAME_QUERY_RESPONSE, (8+1+4+4+4+10) );
    data << p->GetGUID();
    data << p->GetName();
    data << uint8(0);                                       // realm name for cross realm BG usage
    data << uint32(p->getRace());
    data << uint32(p->getGender());
    data << uint32(p->getClass());

    SendPacket(&data);
}

void WorldSession::SendNameQueryOpcodeFromDB(uint64 guid)
{
    std::string name;
    if(!objmgr.GetPlayerNameByGUID(guid, name))
        name = "<non-existing character>";
    uint32 field = Player::GetUInt32ValueFromDB(UNIT_FIELD_BYTES_0, guid);

                                                            // guess size
    WorldPacket data( SMSG_NAME_QUERY_RESPONSE, (8+1+4+4+4+10) );
    data << guid;
    data << name;
    data << (uint8)0;
    data << (uint32)(field & 0xFF);
    data << (uint32)((field >> 16) & 0xFF);
    data << (uint32)((field >> 8) & 0xFF);

    SendPacket( &data );
}

void WorldSession::HandleNameQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    recv_data >> guid;

    Player *pChar = objmgr.GetPlayer(guid);

    if (pChar)
        SendNameQueryOpcode(pChar);
    else
        SendNameQueryOpcodeFromDB(guid);
}

void WorldSession::HandleQueryTimeOpcode( WorldPacket & recv_data )
{
    WorldPacket data( SMSG_QUERY_TIME_RESPONSE, 4 );

    data << (uint32)time(NULL);
    //data << (uint32)getMSTime();
    SendPacket( &data );
}

void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+8);

    uint32 entry;
    uint64 guid;

    recv_data >> entry;
    recv_data >> guid;

    SendCreatureQuery(entry, guid);
}

void WorldSession::SendCreatureQuery( uint32 entry, uint64 guid )
{
    Creature *unit = ObjectAccessor::Instance().GetCreatureOrPet(*_player, guid);

    if (unit == NULL)
    {
        sLog.outDebug( "WORLD: SendCreatureQuery - (%u) NO SUCH UNIT! (GUID: %u, ENTRY: %u)", uint32(GUID_LOPART(guid)), guid, entry );
        //    return;
    }

    //CreatureInfo const *ci = unit->GetCreatureInfo();

    CreatureInfo const *ci = objmgr.GetCreatureTemplate(entry);

    if (!ci)
    {
        sLog.outDebug( "WORLD: SendCreatureQuery - (%u) NO CREATUREINFO! (GUID: %u, ENTRY: %u)", uint32(GUID_LOPART(guid)), guid, entry );
        return;
    }

    sLog.outDetail("WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u - GUID: %u.", ci->Name, entry, guid);

    WorldPacket data( SMSG_CREATURE_QUERY_RESPONSE, 100 );  // guess size
    data << (uint32)entry;                                  // creature entry
    data << (unit ? unit->GetName() : ci->Name);            // creature name

    data << uint8(0) << uint8(0) << uint8(0);               // name2, name3, name4, always empty

    if (unit)
        data << ((unit->isPet()) ? "Pet" : ci->SubName);    // subname
    else
        data << ci->Name;                                   // may be it's typo? must be subname I think...

    data << ci->flag1;                                      // flags          wdbFeild7=wad flags1

    if (unit)
        data << (uint32)((unit->isPet()) ? 0 : ci->type);   // creatureType   wdbFeild8
    else
        data << (uint32) ci->type;

    data << (uint32)ci->family;                             // family         wdbFeild9
    data << (uint32)(unit && unit->isPet() ? 0 : ci->rank); // rank           wdbFeild10
    data << uint32(0);                                      // unknown        wdbFeild11
    data << uint32(0);                                      // unknown        wdbFeild12

    if (unit)
        data << unit->GetUInt32Value(UNIT_FIELD_DISPLAYID); //DisplayID      wdbFeild13
    else
        data << (uint32)ci->randomDisplayID();

    data << (float)1;                                       // unk
    data << (float)1;                                       // unk
    data << uint8(ci->civilian);
    data << uint8(0);                                       // boss flag?
    SendPacket( &data );
}

void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+8);

    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;

    const GameObjectInfo *info = objmgr.GetGameObjectInfo(entryID);
                                                            // guess size
    WorldPacket data ( SMSG_GAMEOBJECT_QUERY_RESPONSE, 150 );
    if(!info)
    {
        sLog.outDebug( "WORLD: CMSG_GAMEOBJECT_QUERY - Missing game object info for entry %u", entryID);
        data << uint32(entryID | 0x80000000);
    }
    else
    {
        sLog.outDetail("WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u - GUID: %u.", info->name, entryID, guid);
        data << entryID;
        data << (uint32)info->type;
        data << (uint32)info->displayId;
        data << info->name;
        data << uint8(0) << uint8(0) << uint8(0);               // name2, name3, name4
        data << uint8(0);                                       // 2.0.3, string
        data << uint8(0);                                       // 2.0.3, string
        data << uint8(0);                                       // 2.0.3, probably string
        data << uint32(info->sound0);
        data << uint32(info->sound1);
        data << uint32(info->sound2);
        data << uint32(info->sound3);
        data << uint32(info->sound4);
        data << uint32(info->sound5);
        data << uint32(info->sound6);
        data << uint32(info->sound7);
        data << uint32(info->sound8);
        data << uint32(info->sound9);
        data << uint32(info->sound10);
        data << uint32(info->sound11);
        data << uint32(info->sound12);
        data << uint32(info->sound13);
        data << uint32(info->sound14);
        data << uint32(info->sound15);
        data << uint32(info->sound16);
        data << uint32(info->sound17);
        data << uint32(info->sound18);
        data << uint32(info->sound19);
        data << uint32(info->sound20);
        data << uint32(info->sound21);
        data << uint32(info->sound22);
        data << uint32(info->sound23);
    }
    SendPacket( &data );
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
    sLog.outDetail("WORLD: Received MSG_CORPSE_QUERY");

    CorpsePtr corpse = GetPlayer()->GetCorpse();

    if(!corpse) return;

    WorldPacket data(MSG_CORPSE_QUERY, (5*4+1));
    data << uint8(0x01);
    data << corpse->GetMapId();
    data << corpse->GetPositionX();
    data << corpse->GetPositionY();
    data << corpse->GetPositionZ();
    data << _player->GetMapId();
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+4+4);

    uint32 textID;
    uint64 guid;
    GossipText *pGossip;
    std::string GossipStr;

    recv_data >> textID;
    sLog.outDetail("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    recv_data >> guid;
    GetPlayer()->SetUInt64Value(UNIT_FIELD_TARGET, guid);

    pGossip = objmgr.GetGossipText(textID);

    WorldPacket data( SMSG_NPC_TEXT_UPDATE, 100 );          // guess size
    data << textID;

    if (!pGossip)
    {
        data << uint32( 0 );
        data << "Greetings $N";
        data << "Greetings $N";
    } else

    for (int i=0; i<8; i++)
    {
        data << pGossip->Options[i].Probability;
        data << pGossip->Options[i].Text_0;

        if ( pGossip->Options[i].Text_1 == "" )
            data << pGossip->Options[i].Text_0; else
            data << pGossip->Options[i].Text_1;

        data << pGossip->Options[i].Language;

        data << pGossip->Options[i].Emotes[0]._Delay;
        data << pGossip->Options[i].Emotes[0]._Emote;

        data << pGossip->Options[i].Emotes[1]._Delay;
        data << pGossip->Options[i].Emotes[1]._Emote;

        data << pGossip->Options[i].Emotes[2]._Delay;
        data << pGossip->Options[i].Emotes[2]._Emote;
    }

    SendPacket( &data );

    sLog.outDetail( "WORLD: Sent SMSG_NPC_TEXT_UPDATE " );
}

void WorldSession::HandlePageQueryOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 pageID;

    recv_data >> pageID;
    sLog.outDetail("WORLD: Received CMSG_PAGE_TEXT_QUERY for pageID '%u'", pageID);

    while (pageID)
    {
        PageText const *pPage = sPageTextStore.LookupEntry<PageText>( pageID );
                                                            // guess size
        WorldPacket data( SMSG_PAGE_TEXT_QUERY_RESPONSE, 50 );
        data << pageID;

        if (!pPage)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            data << pPage->Text;
            data << uint32(pPage->Next_Page);
            pageID = pPage->Next_Page;
        }
        SendPacket( &data );

        sLog.outDetail( "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE " );
    }
}
