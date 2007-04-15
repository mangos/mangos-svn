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

#include "Channel.h"
#include "ObjectMgr.h"
#include "Chat.h"

void Channel::Join(uint64 p, const char *pass, uint32 unk1)
{
    WorldPacket data;
    if(IsOn(p))
    {
        MakeAlreadyOn(&data,p);
        SendToOne(&data,p);
    }
    else if(IsBanned(p))
    {
        MakeYouAreBanned(&data);
        SendToOne(&data,p);
    }
    else if(password.length() > 0 && strcmp(pass,password.c_str()))
    {
        MakeWrongPass(&data);
        SendToOne(&data,p);
    }
    else
    {
        PlayerInfo pinfo;
        pinfo.player = p;
        pinfo.muted = false;
        pinfo.owner = false;
        pinfo.moderator = false;

        Player *plr = objmgr.GetPlayer(p);
        if(plr)
            plr->JoinedChannel(this);

        if(announce)
        {
            MakeJoined(&data,p);
            SendToAll(&data);
        }

        data.clear();
        players[p] = pinfo;

        MakeYouJoined(&data,p,unk1);
        SendToOne(&data,p);

        // if no owner first logged will become
        if(!constant && owner == false)
        {
            SetOwner(p, (players.size()>1?true:false));
            players[p].moderator = true;
        }
    }
}

void Channel::Leave(uint64 p, bool send, uint32 unk1)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        if(send)
        {
            MakeNotOn(&data);
            SendToOne(&data,p);
        }
    }
    else
    {
        if(send)
        {
            MakeYouLeft(&data, unk1);
            SendToOne(&data,p);
            Player *plr = objmgr.GetPlayer(p);
            if(plr)
                plr->LeftChannel(this);
            data.clear();
        }

        bool changeowner = players[p].owner;

        players.erase(p);
        if(announce)
        {
            MakeLeft(&data,p);
            SendToAll(&data);
        }

        if(changeowner)
        {
            uint64 newowner = players.size() > 0 ? players.begin()->second.player : false;
            SetOwner(newowner);
        }
    }
}

void Channel::KickOrBan(uint64 good, const char *badname, bool ban)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *gplr = objmgr.GetPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();

    if(!IsOn(good))
    {
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,good);
    }
    else
    {
        Player *bad = objmgr.GetPlayer(badname);
        if(bad == NULL || !IsOn(bad->GetGUID()))
        {
            MakeNotOn(&data,badname);
            SendToOne(&data,good);
        }
        else if(sec < 2 && bad->GetGUID() == owner && good != owner)
        {
            MakeNotOwner(&data);
            SendToOne(&data,good);
        }
        else
        {
            bool changeowner = (owner == bad->GetGUID());

            if(ban && !IsBanned(bad->GetGUID()))
            {
                banned.push_back(bad->GetGUID());
                MakeBanned(&data,good,bad->GetGUID());
            }
            else
                MakeKicked(&data,good,bad->GetGUID());

            SendToAll(&data);
            players.erase(bad->GetGUID());

            if(changeowner)
            {
                uint64 newowner = players.size() > 0 ? good : false;
                SetOwner(newowner);
            }
        }
    }
}

void Channel::UnBan(uint64 good, const char *badname)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *gplr = objmgr.GetPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();

    if(!IsOn(good))
    {
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,good);
    }
    else
    {
        Player *bad = objmgr.GetPlayer(badname);
        if(bad == NULL || !IsBanned(bad->GetGUID()))
        {
            MakeNotOn(&data,badname);
            SendToOne(&data,good);
        }
        else
        {
            banned.remove(bad->GetGUID());
            MakeUnbanned(&data,good,bad->GetGUID());
            SendToAll(&data);
        }
    }
}

void Channel::Password(uint64 p, const char *pass)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        password = pass;
        MakeSetPassword(&data,p);
        SendToAll(&data);
    }
}

void Channel::SetMode(uint64 p, const char *p2n, bool mod, bool set)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(p2n);
        if(!newp)
            return;

        PlayerInfo inf = players[newp->GetGUID()];
        if(p == owner && newp->GetGUID() == owner && mod)
            return;
        if(newp == NULL || !IsOn(newp->GetGUID()))
        {
            MakeNotOn(&data,p2n);
            SendToOne(&data,p);
        }
        else if(owner == newp->GetGUID() && owner != p)
        {
            MakeNotOwner(&data);
            SendToOne(&data,p);
        }
        else
        {
            if(mod)
                SetModerator(newp->GetGUID(),set);
            else
                SetMute(newp->GetGUID(),set);
        }
    }
}

void Channel::SetOwner(uint64 p, const char *newname)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(sec < 2 && p != owner)
    {
        MakeNotOwner(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(newname);
        if(newp == NULL || !IsOn(newp->GetGUID()))
        {
            MakeNotOn(&data,newname);
            SendToOne(&data,p);
        }
        else
        {
            players[newp->GetGUID()].moderator = true;
            SetOwner(newp->GetGUID());
        }
    }
}

void Channel::GetOwner(uint64 p)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        MakeWhoOwner(&data);
        SendToOne(&data,p);
    }
}

void Channel::List(uint64 p)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        data.Initialize(SMSG_CHANNEL_LIST);
        data << (uint8)3 << (uint32)players.size();

        PlayerList::iterator i;
        uint8 mode;
        for(i = players.begin(); i!=players.end(); i++)
        {
            data << i->first;
            mode = 0x00;
            if(i->second.muted)
                mode |= 0x04;
            if(i->second.moderator)
                mode |= 0x02;
            data << mode;
        }
        SendToOne(&data,p);
    }
}

void Channel::Announce(uint64 p)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        announce = !announce;
        MakeAnnounce(&data,p,announce);
        SendToAll(&data);
    }
}

void Channel::Moderate(uint64 p)
{
    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        moderate = !moderate;
        MakeModerate(&data,p,moderate);
        SendToAll(&data);
    }
}

void Channel::Say(uint64 p, const char *what, uint32 lang)
{
    if(!what)
        return;
    if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    WorldPacket data;

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(players[p].muted)
    {
        MakeYouCantSpeak(&data);
        SendToOne(&data,p);
    }
    else if(moderate && !players[p].moderator && sec < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        uint32 messageLength = strlen(what) + 1;

        data.Initialize(SMSG_MESSAGECHAT);
        data << (uint8)CHAT_MSG_CHANNEL;
        data << (uint32)lang;
        data << name;
        data << p;
        data << messageLength;
        data << what;
        data << uint8(plr ? plr->chatTag() : 0);

        SendToAll(&data,!players[p].moderator ? p : false);
    }
}

void Channel::Invite(uint64 p, const char *newname)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(newname);
        if(!newp)
        {
            MakeNotOn(&data,newname);
            SendToOne(&data,p);
        }
        else if(IsOn(newp->GetGUID()))
        {
            MakeAlreadyOn(&data,newp->GetGUID());
            SendToOne(&data,p);
        }
        else
        {
            if(!newp->HasInIgnoreList(p))
            {
                MakeInvited(&data,p);
                SendToOne(&data,newp->GetGUID());
                data.clear();
            }
            MakeYouInvited(&data,newp->GetGUID());
            SendToOne(&data,p);
        }
    }
}

void Channel::MakeYouInvited(WorldPacket *data, uint64 who)
{
    Player *plr = objmgr.GetPlayer(who);
    if(plr)
    {
        *MakeNotifyPacket(data,YOUINVITED);
        *data << plr->GetName();
    }
}

void Channel::SendToAll(WorldPacket *data, uint64 p)
{
    for(PlayerList::iterator i = players.begin(); i != players.end(); i++)
    {
        Player *plr = objmgr.GetPlayer(i->first);
        if(plr)
        {
            if(!p || !plr->HasInIgnoreList(p))
                plr->GetSession()->SendPacket(data);
        }
    }
}

void Channel::SendToAllButOne(WorldPacket *data, uint64 who)
{
    for(PlayerList::iterator i = players.begin(); i!=players.end(); i++)
    {
        if(i->first != who)
        {
            Player *plr = objmgr.GetPlayer(i->first);
            if(plr)
                plr->GetSession()->SendPacket(data);
        }
    }
}

void Channel::SendToOne(WorldPacket *data, uint64 who)
{
    Player *plr = objmgr.GetPlayer(who);
    if(plr)
        plr->GetSession()->SendPacket(data);
}

void Channel::MakeWhoOwner(WorldPacket *data)
{
    const char *name = "";
    Player *plr = objmgr.GetPlayer(owner);
    if(plr)
        name = plr->GetName();
    else
        name = "PLAYER_NOT_FOUND";

    *MakeNotifyPacket(data,WHOOWNER);
    *data << ((constant || owner == false) ? "Nobody" : name);
}
