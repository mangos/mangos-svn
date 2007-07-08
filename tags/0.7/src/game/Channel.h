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

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Player.h"

class Channel
{
    enum NotifyTypes
    {
        JOINED          = 0x00,
        LEFT            = 0x01,
        YOUJOINED       = 0x02,
        YOULEFT         = 0x03,
        WRONGPASS       = 0x04,
        NOTON1          = 0x05, // Not on channel channel name.
        NOTMOD          = 0x06,
        SETPASS         = 0x07,
        CHANGEOWNER     = 0x08,
        NOTON2          = 0x09, // Player %s is not on channel.
        NOTOWNER        = 0x0A,
        WHOOWNER        = 0x0B,
        MODECHANGE      = 0x0C,
        ANNOUNCEON      = 0x0D,
        ANNOUNCEOFF     = 0x0E,
        MODERATED       = 0x0F,
        UNMODERATED     = 0x10,
        YOUCANTSPEAK    = 0x11,
        KICKED          = 0x12,
        YOUAREBANNED    = 0x13,
        BANNED          = 0x14,
        UNBANNED        = 0x15,
        UNKNOWN1        = 0x16, // is not banned
        ALREADYON       = 0x17,
        INVITED         = 0x18,
        WRONGALLIANCE   = 0x19, // target is in the wrong alliance for channel name
        UNKNOWN2        = 0x1A, // wrong alliance for channel name
        UNKNOWN3        = 0x1B, // invalid channel name
        ISNOTMODERATED  = 0x1C,
        YOUINVITED      = 0x1D,
        UNKNOWN4        = 0x1E, // %s has been banned.
        UNKNOWN5        = 0x1F, // The number of messages that can be sent to this channel is limited, please wait to send another message.
        UNKNOWN6        = 0x20  // You are in not the correct area for this channel.
    };

    struct PlayerInfo
    {
        uint64 player;
        bool owner, moderator, muted;
    };
    typedef map<uint64,PlayerInfo> PlayerList;
    PlayerList players;
    list<uint64> banned;
    std::string name;
    bool announce, moderate;
    uint32 channel_id;
    uint64 m_ownerGUID;
    std::string password;

    private:

        WorldPacket *MakeNotifyPacket(WorldPacket *data, uint8 code)
        {
            data->Initialize(SMSG_CHANNEL_NOTIFY, (1+10));  // guess size
            *data << code << name.c_str();
            return data;
        }
        void MakeJoined(WorldPacket *data, uint64 joined) { *MakeNotifyPacket(data,JOINED) << joined; }
        void MakeLeft(WorldPacket *data, uint64 left) { *MakeNotifyPacket(data,LEFT) << left; }
        void MakeYouJoined(WorldPacket *data, uint64 p) { *MakeNotifyPacket(data,YOUJOINED) << (uint32)channel_id << (uint32)0; }
        void MakeYouLeft(WorldPacket *data) { *MakeNotifyPacket(data,YOULEFT) << (uint32)channel_id << (uint8)1; }
        void MakeWrongPass(WorldPacket *data) { MakeNotifyPacket(data,WRONGPASS); }
        void MakeNotOn(WorldPacket *data) { MakeNotifyPacket(data,NOTON1); }
        void MakeNotModerator(WorldPacket *data) { MakeNotifyPacket(data,NOTMOD); }
        void MakeSetPassword(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,SETPASS) << who; }
        void MakeChangeOwner(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,CHANGEOWNER) << who; }
        void MakeNotOn(WorldPacket *data, const char *who) { *MakeNotifyPacket(data,NOTON2) << who; }
        void MakeNotOwner(WorldPacket *data) { MakeNotifyPacket(data,NOTOWNER); }
        void MakeWhoOwner(WorldPacket *data);
        void MakeModeChange(WorldPacket *data, uint64 who, uint8 oldFlag) { *MakeNotifyPacket(data,MODECHANGE) << who << oldFlag << GetFlag(who); }
        void MakeEnabledAnnounce(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,ANNOUNCEON) << who; }
        void MakeDisabledAnnounce(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,ANNOUNCEOFF) << who; }
        void MakeAnnounce(WorldPacket *data, uint64 who, bool set) { set ? MakeEnabledAnnounce(data,who) : MakeDisabledAnnounce(data,who); }
        void MakeModerated(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,MODERATED) << who; }
        void MakeUnmoderated(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,UNMODERATED) << who; }
        void MakeIsNotModerated(WorldPacket *data) { *MakeNotifyPacket(data,ISNOTMODERATED); }
        void MakeModerate(WorldPacket *data, uint64 who, bool set) { set ? MakeModerated(data,who) : MakeUnmoderated(data,who); }
        void MakeYouCantSpeak(WorldPacket *data) { MakeNotifyPacket(data,YOUCANTSPEAK); }
        void MakeKicked(WorldPacket *data, uint64 good, uint64 bad) { *MakeNotifyPacket(data,KICKED) << bad << good; }
        void MakeYouAreBanned(WorldPacket *data) { MakeNotifyPacket(data,YOUAREBANNED); }
        void MakeBanned(WorldPacket *data, uint64 good, uint64 bad) { *MakeNotifyPacket(data,BANNED) << bad << good; }
        void MakeUnbanned(WorldPacket *data, uint64 good, uint64 bad) { *MakeNotifyPacket(data,UNBANNED) << bad << good; }
        void MakeAlreadyOn(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,ALREADYON) << who; }
        void MakeInvited(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,INVITED) << who; }
        void MakeWrongAlliance(WorldPacket *data, uint64 who) { *MakeNotifyPacket(data,WRONGALLIANCE) << who; }
        void MakeYouInvited(WorldPacket *data, uint64 who);

        void SendToAll(WorldPacket *data, uint64 p = 0);
        void SendToAllButOne(WorldPacket *data, uint64 who);
        void SendToOne(WorldPacket *data, uint64 who);

        bool IsOn(uint64 who)
        {
            return players.count(who) > 0;
        }

        bool IsBanned(const uint64 guid)
        {
            list<uint64>::iterator i;
            for(i = banned.begin(); i!=banned.end(); i++)
                if(*i == guid)
                    return true;
            return false;
        }

        bool IsFirst()
        {
            return !(players.size() > 1);
        }

        uint8 GetFlag(uint64 p) 
        {
            uint8 flag=0;

            if(players[p].owner)
                flag |= 1;
            if(players[p].moderator)
                flag |= 2;
            if(!players[p].muted)
                flag |= 4;

            return flag;
        }

        void _SetOwner(uint64 guid, bool exclaim = true)
        {
            if(m_ownerGUID)
                players[m_ownerGUID].owner = false;

            m_ownerGUID = guid;
            if(m_ownerGUID)
            {
                uint8 oldFlag = GetFlag(m_ownerGUID);
                players[m_ownerGUID].owner = true;

                WorldPacket data;
                MakeModeChange(&data,m_ownerGUID,oldFlag);
                SendToAll(&data);

                if(exclaim)
                {
                    MakeChangeOwner(&data,m_ownerGUID);
                    SendToAll(&data);
                }
            }
        }

        void SetModerator(uint64 p, bool set)
        {
            if(players[p].moderator != set)
            {
                uint8 oldFlag = GetFlag(p);
                players[p].moderator = set;

                WorldPacket data;
                MakeModeChange(&data,p,oldFlag);
                SendToAll(&data);
            }
        }

        void SetMute(uint64 p, bool set)
        {
            if(players[p].muted != set)
            {
                uint8 oldFlag = GetFlag(p);
                players[p].muted = set;
                set = !set;

                WorldPacket data;
                MakeModeChange(&data,p,oldFlag);
                SendToAll(&data);
            }
        }

    public:
        explicit Channel(std::string _name, uint32 _channal_id);
        std::string GetName() { return name; }
        uint32 GetChannelId() const { return channel_id; }
        bool IsConstant() { return channel_id!=0; }
        bool IsAnnounce() { return announce; }
        std::string GetPassword() { return password; }
        void SetPassword(std::string npassword) { password = npassword; }
        void SetAnnounce(bool nannounce) { announce = nannounce; }
        uint32 GetNumPlayers() { return players.size(); }

        void Join(uint64 p, const char *pass);
        void Leave(uint64 p, bool send = true);
        void KickOrBan(uint64 good, const char *badname, bool ban);
        void Kick(uint64 good, const char *badname) { KickOrBan(good,badname,false); }
        void Ban(uint64 good, const char *badname) { KickOrBan(good,badname,true); }
        void UnBan(uint64 good, const char *badname);
        void Password(uint64 p, const char *pass);
        void SetMode(uint64 p, const char *p2n, bool mod, bool set);
        void SetOwner(uint64 p, bool exclaim = true) { _SetOwner(p, exclaim); };
        void SetOwner(uint64 p, const char *newname);
        void GetOwner(uint64 p);
        void SetModerator(uint64 p, const char *newname) { SetMode(p,newname,true,true); }
        void UnsetModerator(uint64 p, const char *newname) { SetMode(p,newname,true,false); }
        void SetMute(uint64 p, const char *newname) { SetMode(p,newname,false,true); }
        void UnsetMute(uint64 p, const char *newname) { SetMode(p,newname,false,false); }
        void List(uint64 p);
        void Announce(uint64 p);
        void Moderate(uint64 p);
        void Say(uint64 p, const char *what, uint32 lang);
        void Invite(uint64 p, const char *newp);
};
#endif
