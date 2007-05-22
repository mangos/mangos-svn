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

#include "WorldSession.h"
#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "Player.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"

void WorldSession::HandleLfgAutoJoinOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_LFG_AUTO_JOIN");
}

void WorldSession::HandleLfgCancelAutoJoinOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_UNSET_LFG_AUTO_JOIN");
}

void WorldSession::HandleLfmAutoAddMembersOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_LFM_AUTOADD");
}

void WorldSession::HandleLfmCancelAutoAddmembersOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_UNSET_LFM_AUTOADD");
}

void WorldSession::HandleLfgClearOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_LOOKING_FOR_GROUP_CLEAR");
    sDatabase.Execute("DELETE FROM `looking_for_group` WHERE `guid` = '%u' AND `lfg_type` = '0'", _player->GetGUIDLow());
}

void WorldSession::HandleLfmSetNoneOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_LOOKING_FOR_NONE");
    sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '0', `type` = '0' WHERE `guid` = '%u' AND `lfg_type` = '1'", _player->GetGUIDLow()); 
}

void WorldSession::HandleLfmSetOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_LOOKING_FOR_MORE");
    recv_data.hexlike();
    uint32 temp, entry, type;
    uint8 lfg_type = 1;

    recv_data >> temp;

    entry = ( temp & 0xFFFF);
    type = ( (temp >> 24) & 0xFFFF);
    //sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '0', `type` = '0' WHERE `guid` = '%u'", _player->GetGUIDLow());
    sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '%u', `type` = '%u', `lfg_type` = '1' WHERE `guid` = '%u' AND `slot` = '1'", entry, type, _player->GetGUIDLow()); 
    sLog.outDebug("LFM set: temp %u, zone %u, type %u", temp, entry, type);
    SendLfgResult(type, entry);
}

void WorldSession::HandleLfgSetCommentOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_COMMENTARY");
    std::string comment;
    uint32 number;
    recv_data.hexlike();

    QueryResult *result = sDatabase.Query("SELECT COUNT(*) FROM `looking_for_group` WHERE `guid` = '%u'", _player->GetGUIDLow());

    if(result)
    {
        Field *fields = result->Fetch();
        number = fields[0].GetUInt32();
        if(number == 0)
            for(uint8 i = 0; i < 3; i++)
                sDatabase.Execute("INSERT INTO `looking_for_group` (`guid`, `slot`, `comment`) VALUES ('%u', '%u', '')", _player->GetGUIDLow(), i);
        recv_data >> comment;
        sLog.outDebug("LFG comment %s", comment.c_str());
        sDatabase.escape_string(comment);
        sDatabase.Execute("UPDATE `looking_for_group` SET `comment` = '%s' WHERE `guid` = '%u'", comment.c_str(), _player->GetGUIDLow());     
    }
    else
        sLog.outDebug("strange, DB problem?");
}

void WorldSession::HandleLookingForGroup(WorldPacket& recv_data)
{
    sLog.outDebug("MSG_LOOKING_FOR_GROUP");
    recv_data.hexlike();
    WorldPacket data;
    uint32 type, entry, unk;

    recv_data >> type >> entry >> unk;
    sLog.outDebug("MSG_LOOKING_FOR_GROUP: type %u, entry %u, unk %u", type, entry, unk);
    //sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '0', `type` = '0' WHERE `guid` = '%u'", _player->GetGUIDLow());
    sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '%u', `type` = '%u', `lfg_type` = '1' WHERE `guid` = '%u' AND `slot` = '1'", entry, type, _player->GetGUIDLow()); 
    SendLfgResult(type, entry);
}

void WorldSession::SendLfgResult(uint32 type, uint32 entry)
{
    WorldPacket data;
    std::string comment;
    uint32 number;
    QueryResult *result = sDatabase.Query("SELECT COUNT(*) FROM `looking_for_group` WHERE `type` = '%u' AND `entry` = '%u'", type, entry);

    if(result)
    {
        Field *fields = result->Fetch();
        number = fields[0].GetUInt32();
    }
    else
    {
        return;
    }

    data.Initialize(MSG_LOOKING_FOR_GROUP);
    data << type;   // type
    data << entry;  // entry from LFGDungeons.dbc
    data << number; // count
    data << number; // count again, strange

    result = sDatabase.Query("SELECT `guid`, `lfg_type`, `comment` FROM `looking_for_group` WHERE `type` = '%u' AND `entry` = '%u'", type, entry);
    if(result)
    {
        Field *fields = result->Fetch();
        do
        {
            uint64 guid = fields[0].GetUInt64();
            comment = fields[2].GetString();
            Player *plr = objmgr.GetPlayer(guid);
            if(plr)
            {
                data.append(plr->GetPackGUID()); // packed guid
                data << plr->getLevel();         // level
                data << plr->GetZoneId();        // current zone
                data << fields[1].GetUInt8();    // 0x00 - LFG, 0x01 - LFM
                for(uint8 i = 0; i < 3; i++)     // we have three slots
                {
                    QueryResult *result2 = sDatabase.Query("SELECT `entry`, `type` FROM `looking_for_group` WHERE `slot` = '%u' AND `guid` = '%u'", i, plr->GetGUID());
                    if(result2)
                    {
                        Field *fields = result2->Fetch();
                        data << uint32( fields[0].GetUInt32() | (fields[1].GetUInt32() << 24) );
                    }
                    else
                    {
                        data << uint32(0x00);
                    }
                    delete result2;
                }
                data << comment;                // commentary
                Group *group = plr->groupInfo.group;
                if(group)
                {
                    data << group->GetMembersCount()-1; // count of group members without group leader
                    Group::MemberList const& members = group->GetMembers();
                    for(Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
                    {
                        Player *member = objmgr.GetPlayer(itr->guid);
                        if(member && member->GetGUID() != plr->GetGUID())
                        {
                            data.append(member->GetPackGUID()); // packed guid
                            data << member->getLevel(); // player level
                        }
                    }
                }
                else
                {
                    data << uint32(0x00);
                }
            }
        }
        while( result->NextRow() );
        delete result;
    }
    else
        return;
    data.hexlike();
    SendPacket(&data);
}

void WorldSession::HandleSetLfgOpcode( WorldPacket & recv_data )
{
    sLog.outDebug("CMSG_SET_LOOKING_FOR_GROUP");
    recv_data.hexlike();
    uint32 slot, temp, entry, type;

    recv_data >> slot >> temp;

    entry = ( temp & 0xFFFF);
    type = ( (temp >> 24) & 0xFFFF);

    sDatabase.Execute("UPDATE `looking_for_group` SET `entry` = '%u', `type` = '%u' WHERE `guid` = '%u' and `slot` = '%u'", entry, type, _player->GetGUIDLow(), slot);

    sLog.outDebug("LFG set: looknumber %u, temp %X, type %u, entry %u", slot, temp, type, entry);
    SendLfgResult(type, entry);
}
