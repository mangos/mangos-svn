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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "UpdateData.h"
#include "Chat.h"
#include "Guild.h"
#include "MapManager.h"
#include "GossipDef.h"

// Guild Charter ID in item_template
#define GUILD_CHARTER_ITEM_ID 5863

//to create guild from game you should create petition
void WorldSession::HandlePetitionBuyOpcode( WorldPacket & recv_data )
{
    /*
    4D 18 00 00 00 10 00 F0 | 00 00 00 00 00 00 00 00
    00 00 00 00 74 65 73 74 | 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 | 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 | 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 | 01 00 00 00
    */
    if (_player->GetGuildId())
        return;
    sLog.outDebug("Received opcode CMSG_PETITION_BUY");
    uint64 guidNPC;
    uint64 unk3;
    uint32 unk4;
    std::string guildname;
    uint8 unk5;
    uint16 unk6;
    uint32 unk7;
    uint64 unk8;
    uint64 unk9;
    uint64 unk10;
    uint64 unk11;
    uint64 unk12;
    uint32 unk13;
    recv_data >> guidNPC;                                   // NPC GUID
    recv_data >> unk3;                                      // 0
    recv_data >> unk4;                                      // 0
    recv_data >> guildname;                                 // Guild name
    recv_data >> unk5;                                      // 0
    recv_data >> unk6;                                      // 0
    recv_data >> unk7;                                      // 0
    recv_data >> unk8;                                      // 0
    recv_data >> unk9;                                      // 0
    recv_data >> unk10;                                     // 0
    recv_data >> unk11;                                     // 0
    recv_data >> unk12;                                     // 0
    recv_data >> unk13;                                     // 1
    sLog.outDebug("Guildmaster with GUID %u tried sell petition: guildname %s", GUID_LOPART(guidNPC), guildname.c_str());

    // prevent cheating
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*_player, guidNPC);
    if(!pCreature||!pCreature->isGuildMaster())
        return;

    if(objmgr.GetGuildByName(guildname))
    {
        SendCommandResult(GUILD_CREATE_S,guildname,GUILD_NAME_EXISTS);
        return;
    }

    ItemPrototype const *pProto = objmgr.GetItemPrototype( GUILD_CHARTER_ITEM_ID );
    if( !pProto )
    {
        _player->SendBuyError( BUY_ERR_CANT_FIND_ITEM, NULL, GUILD_CHARTER_ITEM_ID, 0);
        return;
    }

    //if player hasn't enought money.. maybe another opdoce needed ..
    uint32 price = 1000;                                    // 10 silver
    if( _player->GetMoney() < price)
    {                                                       //player hasn't got enought money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, GUILD_CHARTER_ITEM_ID, 0);
        return;
    }

    uint16 dest;
    uint8 msg = _player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, GUILD_CHARTER_ITEM_ID, pProto->BuyCount, false );
    if( msg != EQUIP_ERR_OK )
    {
        _player->SendBuyError(msg, pCreature, GUILD_CHARTER_ITEM_ID, 0);
        return;
    }

    _player->ModifyMoney( -(int32)price );
    _player->StoreNewItem( dest, GUILD_CHARTER_ITEM_ID, 1, true );
    Item *charter = _player->GetItemByPos(dest);
    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, charter->GetGUIDLow());
    charter->SetState(ITEM_CHANGED, _player);

    sDatabase.escape_string(guildname);
    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `guild_charter` WHERE `ownerguid` = '%u' OR `charterguid` = '%u'", _player->GetGUIDLow(), charter->GetGUIDLow());
    sDatabase.PExecute("INSERT INTO `guild_charter` (`ownerguid`, `charterguid`, `guildname`) VALUES ('%u', '%u', '%s')",
        _player->GetGUIDLow(), charter->GetGUIDLow(), guildname.c_str());
    sDatabase.CommitTransaction();
}

void WorldSession::HandlePetitionShowSignOpcode( WorldPacket & recv_data )
{
    /*
    Client >>> [10 bytes] CMSG_PETITION_SHOW_SIGNATURES=0x1BE -- dump/001287.c
    0000: 00 00 8A E7 37 54 65 00  00 00                   | ....7Te. ..
    Server >>> [21 bytes] SMSG_PETITION_SHOW_SIGNATURES=0x1BF -- dump/001288.s
    0000: 8A E7 37 54 65 00 00 00  AE 60 80 00 00 00 00 00 | ..7Te... .`......
    0010: 01 00 00 00 00                                  | .....
    */
    sLog.outDebug("Received opcode CMSG_PETITION_SHOW_SIGNATURES");
    if(_player->GetGuildId())
        return;
    uint8 signs = 0;
    uint64 petitionguid;
    recv_data >> petitionguid;                              // petition guid

    QueryResult *result = sDatabase.PQuery("SELECT `charterguid` FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    if(!result)
    {
        sLog.outError("any charter on server...");
        return;
    }

    delete result;

    result = sDatabase.PQuery("SELECT `playerguid` FROM `guild_charter_sign` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));

    // result==NULL also correct in case no sign yet
    if(result)
        signs = result->GetRowCount();

    sLog.outDebug("CMSG_PETITION_SHOW_SIGNATURES petition entry: '%u'", GUID_LOPART(petitionguid));

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+1+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << GUID_LOPART(petitionguid);                      // guild guid (in mangos always same as GUID_LOPART(petitionguid)
    data << signs;                                          // sign's count

    for(uint8 i = 1; i <= signs; i++)
    {
        Field *fields = result->Fetch();
        uint64 plguid = fields[0].GetUInt64();

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }
    delete result;
    SendPacket( &data );
}

void WorldSession::HandlePetitionQueryOpcode( WorldPacket & recv_data )
{
    /*
    Client >>> [14 bytes] CMSG_PETITION_QUERY=0x1C6 -- dump/001229.c
    0000: 00 00 01 00 00 00 8A E7  37 54 65 00 00 00       | ........ 7Te...
    Server >>> [73 bytes] SMSG_PETITION_QUERY_RESPONSE=0x1C7 -- dump/001237.s
    0000: 01 00 00 00 AE 60 80 00  00 00 00 00 74 65 73 74 | .....`.. ....test
    0010: 67 75 69 6C 64 00 00 01  00 00 00 00 00 00 00 09 | guild... ........
    0020: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 | ........ ........
    0030: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 | ........ ........
    0040: 00 00 00 00 00 00 00 00  00                      | ........ .
    */
    sLog.outDebug("Received opcode CMSG_PETITION_QUERY");
    uint32 guildguid;
    uint64 petitionguid;
    recv_data >> guildguid;                                 // in mangos always same as GUID_LOPART(petitionguid)
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("CMSG_PETITION_QUERY Petition GUID %u Guild GUID %u", GUID_LOPART(petitionguid), guildguid);

    SendPetitionQueryOpcode(petitionguid);
}

void WorldSession::SendPetitionQueryOpcode( uint64 petitionguid)
{
    uint64 ownerguid = 0;
    std::string guildname = "NO_GUILD_NAME_FOR_GUID";
    uint8 signs = 0;

    unsigned char tdata[51] =
    {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    QueryResult *result = sDatabase.PQuery(
        "SELECT `ownerguid`, `guildname`, "
        "  (SELECT COUNT(`playerguid`) FROM `guild_charter_sign` WHERE `guild_charter_sign`.`charterguid` = '%u') AS signs "
        "FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if(result)
    {
        Field* fields = result->Fetch();
        ownerguid = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);
        guildname = fields[1].GetCppString();
        signs     = fields[2].GetUInt8();
        delete result;
    }
    else
    {
        sLog.outDebug("CMSG_PETITION_QUERY failed for petition (GUID: %u)", (uint32)petitionguid);
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, (4+8+guildname.size()+1+sizeof(tdata)));
    data << GUID_LOPART(petitionguid);                      // guild guid (in mangos always same as GUID_LOPART(petition guid)
    data << (uint64)ownerguid;                              // charter owner guid
    data << guildname;                                      // guildname
    data.append(tdata, sizeof(tdata));
    SendPacket( &data );
}

void WorldSession::HandlePetitionRenameOpcode( WorldPacket & recv_data )
{
    /*
    06 00 00 00 00 00 00 00 | 64 61 61 64 61 77 64 00
    */
    sLog.outDebug("Received opcode MSG_PETITION_RENAME");

    uint64 petitionguid;
    std::string newguildname;

    recv_data >> petitionguid;                              // guid
    recv_data >> newguildname;                              // new guild name

    uint16 pos = _player->GetPosByGuid(petitionguid);
    Item *item = _player->GetItemByPos( pos );
    if(!item)
        return;

    if(objmgr.GetGuildByName(newguildname))
    {
        SendCommandResult(GUILD_CREATE_S,newguildname,GUILD_NAME_EXISTS);
        return;
    }

    std::string db_newguildname = newguildname;
    sDatabase.escape_string(db_newguildname);
    sDatabase.PExecute("UPDATE `guild_charter` SET `guildname` = '%s' WHERE `charterguid` = '%u'",
        db_newguildname.c_str(), GUID_LOPART(petitionguid));

    sLog.outDebug("Petition (GUID: %u) renamed to '%s'", GUID_LOPART(petitionguid), newguildname.c_str());
    WorldPacket data(MSG_PETITION_RENAME, (8+newguildname.size()+1));
    data << petitionguid;
    data << newguildname;
    SendPacket( &data );
}

void WorldSession::HandlePetitionSignOpcode( WorldPacket & recv_data )
{
    /*
    Client >>> [11 bytes] CMSG_PETITION_SIGN=0x1C0 -- dump/000676.c
    0000: 00 00 8A E7 37 54 65 00  00 00 01                | ....7Te. ...
    Server >>> [20 bytes] SMSG_PETITION_SIGN_RESULTS=0x1C1 -- dump/000677.s
    0000: 8A E7 37 54 65 00 00 00  45 F5 7D 00 00 00 00 00 | ..7Te... E.}.....
    0010: 00 00 00 00                                     | ....
    */
    sLog.outDebug("Received opcode CMSG_PETITION_SIGN");
    Field *fields;
    uint64 petitionguid;
    uint64 ownerguid;
    recv_data >> petitionguid;                              // petition guid

    uint8 signs = 0;

    QueryResult *result = sDatabase.PQuery(
        "SELECT `ownerguid`, "
        "  (SELECT COUNT(`playerguid`) FROM `guild_charter_sign` WHERE `guild_charter_sign`.`charterguid` = '%u') AS signs "
        "FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if(!result)
    {
        sLog.outError("any charter on server...");
        return;
    }

    fields = result->Fetch();
    ownerguid = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);
    signs = fields[1].GetUInt8();

    delete result;

    uint32 plguidlo = _player->GetGUIDLow();
    if(GUID_LOPART(ownerguid)==plguidlo)
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && GetPlayer()->GetTeam() != objmgr.GetPlayerTeamByGUID(ownerguid) )
        return;

    signs += 1;
    if (signs > 9)                                          // client signs maximum
        return;

    //client doesn't allow to sign petition two times by one player, not check yet (required additional SQL query)

    sDatabase.PExecute("INSERT INTO `guild_charter_sign` (`ownerguid`,`charterguid`, `playerguid`) VALUES ('%u', '%u', '%u')", GUID_LOPART(ownerguid),GUID_LOPART(petitionguid), plguidlo);

    sLog.outDebug("PETITION SIGN: GUID %u by player: %s (GUID: %u)", GUID_LOPART(petitionguid), _player->GetName(),plguidlo);

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
    data << petitionguid;
    data << _player->GetGUID();
    data << (uint32)0;                                      // can be other values for error reporting(need check 2, 4)

    // close at signer side
    SendPacket( &data );

    // update for owner if online
    if(Player* owner = objmgr.GetPlayer(ownerguid))
        owner->GetSession()->SendPacket( &data );
}

void WorldSession::HandlePetitionDeclineOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("Received opcode MSG_PETITION_DECLINE");

    uint64 petitionguid;
    uint64 ownerguid;
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("Petition %u declined by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    QueryResult *result = sDatabase.PQuery("SELECT `ownerguid` FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    if(!result)
        return;

    Field* fields = result->Fetch();
    ownerguid = MAKE_GUID(fields[0].GetUInt32(),HIGHGUID_PLAYER);
    delete result;

    Player* owner = objmgr.GetPlayer(ownerguid);
    if(owner)                                               // petition owner online
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << _player->GetGUID();
        owner->GetSession()->SendPacket( &data );
    }
}

void WorldSession::HandleOfferPetitionOpcode( WorldPacket & recv_data )
{
    /*
    Client >>> [18 bytes] CMSG_OFFER_PETITION=0x1C3 -- dump/004811.c
    0000: 00 00 8A E7 37 54 65 00  00 00 45 F5 7D 00 00 00 | ....7Te. ..E.}...
    0010: 00 00                                           | ..
    Server >>> [21 bytes] SMSG_PETITION_SHOW_SIGNATURES=0x1BF -- dump/000647.s
    0000: 8A E7 37 54 65 00 00 00  AE 60 80 00 00 00 00 00 | ..7Te... .`......
    0010: 01 00 00 00 00                                  | .....
    */
    sLog.outDebug("Received opcode CMSG_OFFER_PETITION");

    uint8 signs = 0;
    uint64 petitionguid, plguid;
    Player *player;
    recv_data >> petitionguid;                              // petition guid
    recv_data >> plguid;                                    // player guid
    sLog.outDebug("OFFER PETITION: GUID1 %u, to player id: %u", GUID_LOPART(petitionguid), GUID_LOPART(plguid));

    player = ObjectAccessor::Instance().FindPlayer(plguid);
    if(!player || player->HasInIgnoreList(GetPlayer()->GetGUID()))
        return;

    // not let offer to enemies
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && GetPlayer()->GetTeam() != player->GetTeam() )
        return;

    QueryResult *result = sDatabase.PQuery("SELECT `charterguid` FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    if(!result)
    {
        sLog.outError("any charter on server...");
        return;
    }

    delete result;

    result = sDatabase.PQuery("SELECT `playerguid` FROM `guild_charter_sign` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    // result==NULL also correct charter without signs
    if(result)
        signs = result->GetRowCount();

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+signs+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << GUID_LOPART(petitionguid);                      // guild guid (in mangos always same as GUID_LOPART(petition guid)
    data << signs;                                          // sign's count

    for(uint8 i = 1; i <= signs; i++)
    {
        Field *fields = result->Fetch();
        uint64 plguid = fields[0].GetUInt64();

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }

    delete result;
    player->GetSession()->SendPacket( &data );
}

void WorldSession::HandleTurnInPetitionOpcode( WorldPacket & recv_data )
{
    /*
    Client >>> [10 bytes] CMSG_TURN_IN_PETITION=0x1C4 -- dump/001956.c
    0000: 00 00 8A E7 37 54 65 00  00 00                   | ....7Te. ..
    Server >>> [60 bytes] SMSG_MESSAGECHAT=0x96 -- dump/001957.s
    0000: 0A 00 00 00 00 AE 60 80  00 00 00 00 00 2A 00 00 | ......`. .....*..
    0010: 00 47 75 69 6C 64 20 63  68 61 72 74 65 72 20 68 | .Guild c harter h
    0020: 61 76 65 20 6E 6F 74 20  65 6E 6F 75 67 68 20 73 | ave not  enough s
    0030: 69 67 6E 61 74 75 72 65  73 2E 00 00             | ignature s...

    don't know how to cause this opcode, it is possible that charter-party has wrong properties \flags (are not present even names)...
    Here it is necessary to receive number of signatures, to compare with 9 and if it is equal - if guild, to add the players who have signed charter in guild, to remove charter-party and to send the answer...
    Still is any interesting opcode UMSG_DELETE_GUILD_CHARTER:)*/
    sLog.outDebug("Received opcode CMSG_TURN_IN_PETITION");
    WorldPacket data;
    uint64 petitionguid;

    uint32 ownerguidlo;
    std::string guildname;

    recv_data >> petitionguid;

    if(_player->GetGuildId())
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << (uint32)2;                                  // already in guild
        _player->GetSession()->SendPacket(&data);
    }

    sLog.outDebug("Petition %u turned in by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    // Guild data
    QueryResult *result = sDatabase.PQuery("SELECT `ownerguid`, `guildname` FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    if(result)
    {
        Field *fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt32();
        guildname = fields[1].GetCppString();
        delete result;
    }
    else
    {
        sLog.outError("guild_charters table has broken data!");
        return;
    }

    if(_player->GetGUIDLow() != ownerguidlo)
        return;

    // Guild signs
    uint8 signs;
    result = sDatabase.PQuery("SELECT `playerguid` FROM `guild_charter_sign` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    if(result)
        signs = result->GetRowCount();
    else
        signs = 0;

    if(signs < sWorld.getConfig(CONFIG_MIN_PETITION_SIGNS))
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << (uint32)4;                                  // need more signatures...
        SendPacket(&data);
        delete result;
        return;
    }

    if(objmgr.GetGuildByName(guildname))
    {
        SendCommandResult(GUILD_CREATE_S,guildname,GUILD_NAME_EXISTS);
        delete result;
        return;
    }

    // and at last charter item check
    uint16 pos = _player->GetPosByGuid(petitionguid);
    Item *item = _player->GetItemByPos( pos );
    if(!item)
    {
        delete result;
        return;
    }

    // OK!

    // delete charter item
    _player->DestroyItem( (pos >> 8),(pos & 255), true);

    // create guild
    Guild* guild = new Guild;
    guild->create(_player->GetGUID(),guildname);

    // register guild and add guildmaster
    objmgr.AddGuild(guild);

    // add members
    for(uint8 i = 0; i < signs; ++i)
    {
        Field* fields = result->Fetch();
        guild->AddMember(fields[0].GetUInt32(), (uint32)GR_INITIATE);
        result->NextRow();
    }

    delete result;

    sDatabase.BeginTransaction();
    sDatabase.PExecute("DELETE FROM `guild_charter` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    sDatabase.PExecute("DELETE FROM `guild_charter_sign` WHERE `charterguid` = '%u'", GUID_LOPART(petitionguid));
    sDatabase.CommitTransaction();

    // Guild created
    sLog.outDebug("TURN IN PETITION GUID %u", GUID_LOPART(petitionguid));

    data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << (uint32)0;
    SendPacket( &data );
}

void WorldSession::HandlePetitionShowListOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    unsigned char tdata[21] =
    {
        0x01, 0x01, 0x00, 0x00, 0x00, 0xe7, 0x16, 0x00, 0x00, 0xef, 0x23, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };
    recv_data >> guid;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: HandlePetitionShowListOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if( !unit->isGuildMaster())                             // it's not guild master
        return;

    data.Initialize( SMSG_PETITION_SHOWLIST, (8+sizeof(tdata)) );
    data << guid;
    data.append( tdata, sizeof(tdata) );
    SendPacket( &data );
}

//old guild code
void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
    uint32 guildId;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_QUERY"  );

    recvPacket >> guildId;

    guild = objmgr.GetGuildById(guildId);
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->Query(this);
}

void WorldSession::HandleGuildCreateOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    std::string gname;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_CREATE"  );

    recvPacket >> gname;
    if(!GetPlayer()->GetGuildId())
    {
        guild = new Guild;
        guild->create(GetPlayer()->GetGUID(),gname);
        objmgr.AddGuild(guild);
    }
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    std::string Invitedname,plname;
    Player * player;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_INVITE"  );

    recvPacket >> Invitedname;

    player = ObjectAccessor::Instance().FindPlayerByName(Invitedname.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if( !player )
    {
        SendCommandResult(GUILD_INVITE_S,Invitedname,GUILD_PLAYER_NOT_FOUND);
        return;
    }

    // OK result but not send invite
    if(player->HasInIgnoreList(GetPlayer()->GetGUID()))
        return;

    // not let enemies sign guild charter
    if ( !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && player->GetTeam() != GetPlayer()->GetTeam() )
    {
        SendCommandResult(GUILD_INVITE_S,Invitedname,GUILD_NOT_ALLIED);
        return;
    }

    if( player->GetGuildId() )
    {
        plname = player->GetName();
        SendCommandResult(GUILD_INVITE_S,plname,ALREADY_IN_GUILD);
        return;
    }

    if( player->GetGuildIdInvited() )
    {
        plname = player->GetName();
        SendCommandResult(GUILD_INVITE_S,plname,ALREADY_INVITED_TO_GUILD);
        return;
    }

    if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_INVITE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    sLog.outDebug( "Player %s Invited %s to Join his Guild",GetPlayer()->GetName(),Invitedname.c_str());

    player->SetGuildIdInvited(GetPlayer()->GetGuildId());

    data.Initialize(SMSG_GUILD_INVITE, (8+10));             // guess size
    data << GetPlayer()->GetName();
    data << guild->GetName();
    player->GetSession()->SendPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_INVITE)" );
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    std::string plName;
    uint64 plGuid;
    Guild *guild;
    Player *player;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_REMOVE"  );

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
        plGuid = player->GetGUID();
    else
        plGuid = objmgr.GetPlayerGUIDByName(plName.c_str());

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_REMOVE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }
    if(plGuid == guild->GetLeader())
    {
        SendCommandResult(GUILD_QUIT_S,"",GUILD_LEADER_LEAVE);
        return;
    }

    guild->DelMember(plGuid);

    data.Initialize(SMSG_GUILD_EVENT, (2+20));              // guess size
    data << (uint8)GE_REMOVED;
    data << (uint8)2;
    data << plName;
    data << GetPlayer()->GetName();
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    Player *player = GetPlayer();

    sLog.outDebug( "WORLD: Received CMSG_GUILD_ACCEPT"  );

    guild = objmgr.GetGuildById(player->GetGuildIdInvited());
    if(!guild || player->GetGuildId())
        return;

    // not let enemies sign guild charter
    if ( !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION) && player->GetTeam() != objmgr.GetPlayerTeamByGUID(guild->GetLeader()) )
        return;

    guild->AddMember(GetPlayer()->GetGUID());

    WorldPacket data(SMSG_GUILD_EVENT, (2+10));             // guess size
    data << (uint8)GE_JOINED;
    data << (uint8)1;
    data << player->GetName();
    guild->BroadcastPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    sLog.outDebug( "WORLD: Received CMSG_GUILD_DECLINE"  );

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);

}

void WorldSession::HandleGuildInfoOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    sLog.outDebug( "WORLD: Received CMSG_GUILD_INFO"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    WorldPacket data( SMSG_GUILD_INFO, (5*4 + guild->GetName().size() + 1) );
    data << guild->GetName();
    data << guild->GetCreatedDay();
    data << guild->GetCreatedMonth();
    data << guild->GetCreatedYear();
    data << guild->GetMemberSize();
    data << guild->GetMemberSize();

    SendPacket(&data);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_ROSTER"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild) return;

    guild->Roster(this);

}

void WorldSession::HandleGuildPromoteOpcode(WorldPacket& recvPacket)
{
    std::string plName;
    uint64 plGuid;
    uint32 plGuildId;
    uint32 plRankId;
    Player * player;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_PROMOTE"  );

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
        plRankId = player->GetRank();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName.c_str());
        plGuildId = Player::GetGuildIdFromDB(plGuid);
        plRankId = Player::GetRankFromDB(plGuid);
    }

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(plGuid == GetPlayer()->GetGUID())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_NAME_INVALID);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_PROMOTE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }
    else if((plRankId-1) == 0 || (plRankId-1) < this->GetPlayer()->GetRank()) return;

    guild->ChangeRank(plGuid, (plRankId-1));

    WorldPacket data(SMSG_GUILD_EVENT, (2+30));             // guess size
    data << (uint8)GE_PROMOTION;
    data << (uint8)3;
    data << GetPlayer()->GetName();
    data << plName;
    data << guild->GetRankName(plRankId-1);
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildDemoteOpcode(WorldPacket& recvPacket)
{
    std::string plName;
    uint64 plGuid;
    uint32 plGuildId;
    uint32 plRankId;
    Player *player;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_DEMOTE"  );

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
        plRankId = player->GetRank();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName.c_str());
        plGuildId = Player::GetGuildIdFromDB(plGuid);
        plRankId = Player::GetRankFromDB(plGuid);
    }

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(plGuid == GetPlayer()->GetGUID())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_NAME_INVALID);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    else if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_DEMOTE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }
    else if((plRankId+1) >= guild->GetNrRanks() || plRankId <= this->GetPlayer()->GetRank()) return;

    guild->ChangeRank(plGuid, (plRankId+1));

    WorldPacket data(SMSG_GUILD_EVENT, (2+30));             // guess size
    data << (uint8)GE_DEMOTION;
    data << (uint8)3;
    data << GetPlayer()->GetName();
    data << plName;
    data << guild->GetRankName(plRankId+1);
    guild->BroadcastPacket(&data);
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& recvPacket)
{
    std::string plName;
    Guild *guild;
    Player *player = GetPlayer();

    sLog.outDebug( "WORLD: Received CMSG_GUILD_LEAVE"  );

    guild = objmgr.GetGuildById(player->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(player->GetGUID() == guild->GetLeader() && guild->GetMemberSize() > 1)
    {
        SendCommandResult(GUILD_QUIT_S,"",GUILD_LEADER_LEAVE);
        return;
    }

    if(player->GetGUID() == guild->GetLeader())
    {
        guild->Disband();
        return;
    }

    plName = player->GetName();

    guild->DelMember(player->GetGUID());

    WorldPacket data(SMSG_GUILD_EVENT, (2+10));             // guess size
    data << (uint8)GE_LEFT;
    data << (uint8)1;
    data << plName;
    guild->BroadcastPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );

    SendCommandResult(GUILD_QUIT_S,plName,GUILD_PLAYER_NO_MORE_IN_GUILD);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& recvPacket)
{
    WorldPacket data;
    std::string name;
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_DISBAND"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    guild->Disband();

    sLog.outDebug( "WORLD: Guild Sucefully Disbanded" );
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
    std::string name;
    Player * newLeader;
    uint64 newLeaderGUID;
    uint32 newLeaderGuild;
    Player * oldLeader = GetPlayer();
    Guild *guild;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_LEADER"  );

    recvPacket >> name;

    if(name.size() == 0)
        return;

    normalizePlayerName(name);
    sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    newLeader = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
    if(newLeader)
    {
        newLeaderGUID = newLeader->GetGUID();
        newLeaderGuild = newLeader->GetGuildId();
    }
    else
    {
        newLeaderGUID = objmgr.GetPlayerGUIDByName(name.c_str());
        newLeaderGuild = Player::GetGuildIdFromDB(newLeaderGUID);
    }
    guild = objmgr.GetGuildById(oldLeader->GetGuildId());

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !newLeaderGUID )
    {
        SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    if(oldLeader->GetGuildId() != newLeaderGuild)
    {
        SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(oldLeader->GetGUID() != guild->GetLeader())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    guild->SetLeader(newLeaderGUID);
    guild->ChangeRank(oldLeader->GetGUID(), GR_OFFICER);

    WorldPacket data(SMSG_GUILD_EVENT, (2+20));             // guess size
    data << (uint8)GE_LEADER_CHANGED;
    data << (uint8)2;
    data << oldLeader->GetName();
    data << name.c_str();
    guild->BroadcastPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string MOTD;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_MOTD"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_SETMOTD))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    if(recvPacket.size() != 0)
        recvPacket >> MOTD;
    else
        MOTD = "";

    guild->SetMOTD(MOTD);

    WorldPacket data(SMSG_GUILD_EVENT, (2+MOTD.size()+1));
    data << (uint8)GE_MOTD;
    data << (uint8)1;
    data << MOTD;
    guild->BroadcastPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_EVENT)" );
}

void WorldSession::HandleGuildSetPublicNoteOpcode(WorldPacket& recvPacket)
{

    Guild *guild;
    Player * player;
    uint64 plGuid;
    uint32 plGuildId;
    std::string name,PNOTE;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_SET_PUBLIC_NOTE"  );

    recvPacket >> name;

    if(name.size() == 0)
        return;

    normalizePlayerName(name);
    sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(name.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(name.c_str());
        plGuildId = Player::GetGuildIdFromDB(plGuid);
    }

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendCommandResult(GUILD_INVITE_S,name,GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_EPNOTE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> PNOTE;
    guild->SetPNOTE(plGuid,PNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildSetOfficerNoteOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    Player *player;
    uint64 plGuid;
    uint32 plGuildId;
    std::string plName, OFFNOTE;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_SET_OFFICER_NOTE"  );

    recvPacket >> plName;

    if(plName.size() == 0)
        return;

    normalizePlayerName(plName);
    sDatabase.escape_string(plName);                        // prevent SQL injection - normal name don't must changed by this call

    player = ObjectAccessor::Instance().FindPlayerByName(plName.c_str());
    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(player)
    {
        plGuid = player->GetGUID();
        plGuildId = player->GetGuildId();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName(plName.c_str());
        plGuildId = Player::GetGuildIdFromDB(plGuid);
    }

    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    else if( !plGuid )
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_FOUND);
        return;
    }
    else if(GetPlayer()->GetGuildId() != plGuildId)
    {
        SendCommandResult(GUILD_INVITE_S,plName,GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }
    if(!guild->HasRankRight(GetPlayer()->GetRank(),GR_RIGHT_EOFFNOTE))
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> OFFNOTE;
    guild->SetOFFNOTE(plGuid,OFFNOTE);

    guild->Roster(this);
}

void WorldSession::HandleGuildRankOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string rankname;
    uint32 rankId;
    uint32 rights;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_RANK"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> rankId;
    recvPacket >> rights;
    recvPacket >> rankname;

    sLog.outDebug( "WORLD: Changed RankName to %s , Rights to 0x%.4X",rankname.c_str() ,rights );

    guild->SetRankName(rankId,rankname);
    guild->SetRankRights(rankId,rights);

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string rankname;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_ADD_RANK"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    recvPacket >> rankname;

    guild->CreateRank(rankname,GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recvPacket)
{
    Guild *guild;
    std::string rankname;

    sLog.outDebug( "WORLD: Received CMSG_GUILD_DEL_RANK"  );

    guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    else if(GetPlayer()->GetGUID() != guild->GetLeader())
    {
        SendCommandResult(GUILD_INVITE_S,"",GUILD_PERMISSIONS);
        return;
    }

    guild->DelRank();

    guild->Query(this);
    guild->Roster(this);
}

void WorldSession::SendCommandResult(uint32 typecmd,std::string str,uint32 cmdresult)
{
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, (8+str.size()+1));
    data << typecmd;
    data << str;
    data << cmdresult;
    SendPacket(&data);

    sLog.outDebug( "WORLD: Sent (SMSG_GUILD_COMMAND_RESULT)" );
}

void WorldSession::HandleGuildChangeInfoOpcode(WorldPacket& recvPacket)
{

    sLog.outDebug( "WORLD: Received CMSG_GUILD_CHANGEINFO"  );

    std::string GINFO;

    recvPacket >> GINFO;

    Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    guild->SetGINFO(GINFO);

}

void WorldSession::HandleGuildSaveEmblemOpcode(WorldPacket& recvPacket)
{
    sLog.outDebug( "WORLD: Received MSG_SAVE_GUILD_EMBLEM"  );

    uint32 stuff0;
    uint32 stuff1;

    uint32 EmblemStyle;
    uint32 EmblemColor;
    uint32 BorderStyle;
    uint32 BorderColor;
    uint32 BackgroundColor;

    recvPacket >> stuff0;
    recvPacket >> stuff1;

    recvPacket >> EmblemStyle;
    recvPacket >> EmblemColor;
    recvPacket >> BorderStyle;
    recvPacket >> BorderColor;
    recvPacket >> BackgroundColor;

    Guild *guild = objmgr.GetGuildById(GetPlayer()->GetGuildId());
    if(!guild)
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (guild->GetLeader() != GetPlayer()->GetGUID())
    {
        SendCommandResult(GUILD_CREATE_S,"",GUILD_PERMISSIONS);
        return;
    }

    guild->SetEmblem(EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor);

    guild->Query(this);

    WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
    data << (uint32)0;
    SendPacket( &data );
}
