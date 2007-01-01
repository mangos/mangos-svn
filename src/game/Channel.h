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
#include "Opcodes.h"
#include "Player.h"

using namespace std;

class Channel
{
    struct PlayerInfo
    {
        Player *player;
        bool owner, moderator, muted;
    };
    typedef map<Player*,PlayerInfo> PlayerList;
    PlayerList players;
    list<uint64> banned;
    string name;
    bool announce, constant, moderate;
    Player *owner;
    string password;
    private:

        WorldPacket *MakeNotifyPacket(WorldPacket *data, uint8 code)
        {
            data->Initialize(SMSG_CHANNEL_NOTIFY, (1+10)); // guess size
            *data << code << name.c_str();
            return data;
        }
        void MakeJoined(WorldPacket *data, Player *joined) { *MakeNotifyPacket(data,0x00) << joined->GetGUID(); }
        void MakeLeft(WorldPacket *data, Player *left) { *MakeNotifyPacket(data,0x01) << left->GetGUID(); }
        void MakeYouJoined(WorldPacket *data, Player *p ) { *MakeNotifyPacket(data,0x02) << p->GetGUID(); }
        void MakeYouLeft(WorldPacket *data) { MakeNotifyPacket(data,0x03); }
        void MakeWrongPass(WorldPacket *data) { MakeNotifyPacket(data,0x04); }
        void MakeNotOn(WorldPacket *data) { MakeNotifyPacket(data,0x05); }
        void MakeNotModerator(WorldPacket *data) { MakeNotifyPacket(data,0x06); }
        void MakeSetPassword(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x07) << who->GetGUID(); }
        void MakeChangeOwner(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x08) << who->GetGUID(); }
        void MakeNotOn(WorldPacket *data, const char *who) { *MakeNotifyPacket(data,0x09) << who; }
        void MakeNotOwner(WorldPacket *data) { MakeNotifyPacket(data,0x0A); }
        void MakeWhoOwner(WorldPacket *data) { *MakeNotifyPacket(data,0x0B) << ((constant || owner == NULL) ? "Nobody" : owner->GetName()); }

        void MakeModeChange(WorldPacket *data, Player *who, char moderator, char voice)
        {
            MakeNotifyPacket(data,0x0C);
            *data << who->GetGUID();
            uint8 byte1 = 0x00, byte2 = 0x00;
            if(moderator == 1) byte1 |= 0x02;
            if(voice == 1) byte1 |= 0x04;

            if(moderator == 2) byte2 |= 0x02;
            if(voice == 2) byte2 |= 0x04;
            *data << byte1 << byte2;
        }
        void MakeEnabledAnnounce(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x0D) << who->GetGUID(); }
        void MakeDisabledAnnounce(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x0E) << who->GetGUID(); }
        void MakeAnnounce(WorldPacket *data, Player *who, bool set) { set ? MakeEnabledAnnounce(data,who) : MakeDisabledAnnounce(data,who); }
        void MakeModerated(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x0F) << who->GetGUID(); }
        void MakeUnmoderated(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x10) << who->GetGUID(); }
        void MakeModerate(WorldPacket *data, Player *who, bool set) { set ? MakeModerated(data,who) : MakeUnmoderated(data,who); }
        void MakeYouCantSpeak(WorldPacket *data) { MakeNotifyPacket(data,0x11); }
        void MakeKicked(WorldPacket *data, Player *good, Player *bad) { *MakeNotifyPacket(data,0x12) << bad->GetGUID() << good->GetGUID(); }
        void MakeYouAreBanned(WorldPacket *data) { MakeNotifyPacket(data,0x13); }
        void MakeBanned(WorldPacket *data, Player *good, Player *bad) { *MakeNotifyPacket(data,0x14) << bad->GetGUID() << good->GetGUID(); }
        void MakeUnbanned(WorldPacket *data, Player *good, Player *bad) { *MakeNotifyPacket(data,0x15) << bad->GetGUID() << good->GetGUID(); }

        void MakeAlreadyOn(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x17) << who->GetGUID(); }
        void MakeInvited(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x18) << who->GetGUID(); }
        void MakeWrongAlliance(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x19) << who->GetGUID(); }

        void MakeYouInvited(WorldPacket *data, Player *who) { *MakeNotifyPacket(data,0x1D) << who->GetName(); }

        void SendToAll(WorldPacket *data, Player *p = NULL)
        {
            PlayerList::iterator i;
            for(i = players.begin(); i!=players.end(); i++)
                if(!p || !i->first->HasInIgnoreList(p->GetGUID()))
                    i->first->GetSession()->SendPacket(data);
        }

        void SendToAllButOne(WorldPacket *data, Player *who)
        {
            PlayerList::iterator i;
            for(i = players.begin(); i!=players.end(); i++)
                if(i->first != who)
                    i->first->GetSession()->SendPacket(data);
        }

        void SendToOne(WorldPacket *data, Player *who)
        {
            who->GetSession()->SendPacket(data);
        }

        bool IsOn(Player *who)
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

        void SetOwner(Player *p)
        {
            owner = p;
            if(owner != NULL)
            {
                WorldPacket data;
                MakeChangeOwner(&data,p);
                SendToAll(&data);
            }
        }

        void SetModerator(Player *p, bool set)
        {
            if(players[p].moderator != set)
            {
                players[p].moderator = set;
                WorldPacket data;
                MakeModeChange(&data,p,set ? 2 : 1,0);
                SendToAll(&data);
            }
        }

        void SetMute(Player *p, bool set)
        {
            if(players[p].muted != set)
            {
                players[p].muted = set;
                set = !set;
                WorldPacket data;
                MakeModeChange(&data,p,0,set ? 2 : 1);
                SendToAll(&data);
            }
        }

    public:
        Channel() : name(""), announce(true), constant(false), moderate(false), owner(NULL), password("")
        {
        }
        void SetName(string newname) { name = newname; }
        string GetName() { return name; }
        bool IsConstant() { return constant; }
        bool IsAnnounce() { return announce; }
        string GetPassword() { return password; }
        void SetConstant(bool nconstant) { constant = nconstant; }
        void SetPassword(string npassword) { password = npassword; }
        void SetAnnounce(bool nannounce) { announce = nannounce; }
        uint32 GetNumPlayers() { return players.size(); }

        void Join(Player *p, const char *pass);
        void Leave(Player *p, bool send = true);
        void KickOrBan(Player *good, const char *badname, bool ban);
        void Kick(Player *good, const char *badname) { KickOrBan(good,badname,false); }
        void Ban(Player *good, const char *badname) { KickOrBan(good,badname,true); }
        void UnBan(Player *good, const char *badname);
        void Password(Player *p, const char *pass);
        void SetMode(Player *p, const char *p2n, bool mod, bool set);
        void SetOwner(Player *p, const char *newname);
        void GetOwner(Player *p);
        void SetModerator(Player *p, const char *newname) { SetMode(p,newname,true,true); }
        void UnsetModerator(Player *p, const char *newname) { SetMode(p,newname,true,false); }
        void SetMute(Player *p, const char *newname) { SetMode(p,newname,false,true); }
        void UnsetMute(Player *p, const char *newname) { SetMode(p,newname,false,false); }
        void List(Player *p);
        void Announce(Player *p);
        void Moderate(Player *p);
        void Say(Player *p, const char *what, uint32 lang);
        void Invite(Player *p, const char *newp);
};
