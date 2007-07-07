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

Channel::Channel(std::string _name, uint32 _channal_id)
    : name(_name), announce(true), channel_id(_channal_id), moderate(false), m_ownerGUID(0), password("")
{
    // set special flags if built-in channel
    ChatChannelsEntry const* ch = GetChannelEntryFor(_channal_id);
    if(ch)
    {
        channel_id = ch->ChannelID;                         // built-in channel    
        announce = false;                                   // no join/leave announces
    }
}

void Channel::Join(uint64 p, const char *pass)
{
    WorldPacket data;
    if(IsOn(p))
    {
        if(!IsConstant())                                   // non send error message for built-in channels
        {
            MakeAlreadyOn(&data,p);
            SendToOne(&data,p);
        }
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

        MakeYouJoined(&data,p);
        SendToOne(&data,p);

        // if no owner first logged will become
        if(!IsConstant() && !m_ownerGUID)
        {
            SetOwner(p, (players.size()>1?true:false));
            players[p].moderator = true;
        }
    }
}

void Channel::Leave(uint64 p, bool send)
{
    if(!IsOn(p))
    {
        if(send)
        {
            WorldPacket data;
            MakeNotOn(&data);
            SendToOne(&data,p);
        }
    }
    else
    {
        if(send)
        {
            WorldPacket data;
            MakeYouLeft(&data);
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
            WorldPacket data;
            MakeLeft(&data,p);
            SendToAll(&data);
        }

        if(changeowner)
        {
            uint64 newowner = players.size() > 0 ? players.begin()->second.player : 0;
            SetOwner(newowner);
        }
    }
}

void Channel::KickOrBan(uint64 good, const char *badname, bool ban)
{
    uint32 sec = 0;
    Player *gplr = objmgr.GetPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();

    if(!IsOn(good))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,good);
    }
    else
    {
        Player *bad = objmgr.GetPlayer(badname);
        if(bad == NULL || !IsOn(bad->GetGUID()))
        {
            WorldPacket data;
            MakeNotOn(&data,badname);
            SendToOne(&data,good);
        }
        else if(sec < SEC_GAMEMASTER && bad->GetGUID() == m_ownerGUID && good != m_ownerGUID)
        {
            WorldPacket data;
            MakeNotOwner(&data);
            SendToOne(&data,good);
        }
        else
        {
            bool changeowner = (m_ownerGUID == bad->GetGUID());

            WorldPacket data;

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
    uint32 sec = 0;
    Player *gplr = objmgr.GetPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();

    if(!IsOn(good))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,good);
    }
    else
    {
        Player *bad = objmgr.GetPlayer(badname);
        if(bad == NULL || !IsBanned(bad->GetGUID()))
        {
            WorldPacket data;
            MakeNotOn(&data,badname);
            SendToOne(&data,good);
        }
        else
        {
            banned.remove(bad->GetGUID());

            WorldPacket data;
            MakeUnbanned(&data,good,bad->GetGUID());
            SendToAll(&data);
        }
    }
}

void Channel::Password(uint64 p, const char *pass)
{
    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        password = pass;

        WorldPacket data;
        MakeSetPassword(&data,p);
        SendToAll(&data);
    }
}

void Channel::SetMode(uint64 p, const char *p2n, bool mod, bool set)
{
    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(p2n);
        if(!newp)
            return;

        PlayerInfo inf = players[newp->GetGUID()];
        if(p == m_ownerGUID && newp->GetGUID() == m_ownerGUID && mod)
            return;
        if(newp == NULL || !IsOn(newp->GetGUID()))
        {
            WorldPacket data;
            MakeNotOn(&data,p2n);
            SendToOne(&data,p);
        }
        else if(m_ownerGUID == newp->GetGUID() && m_ownerGUID != p)
        {
            WorldPacket data;
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
    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(sec < SEC_GAMEMASTER && p != m_ownerGUID)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(newname);
        if(newp == NULL || !IsOn(newp->GetGUID()))
        {
            WorldPacket data;
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
    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        WorldPacket data;
        MakeWhoOwner(&data);
        SendToOne(&data,p);
    }
}

void Channel::List(uint64 p)
{
    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        WorldPacket data(SMSG_CHANNEL_LIST,1+4+players.size()*(8+1));
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
    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        announce = !announce;

        WorldPacket data;
        MakeAnnounce(&data,p,announce);
        SendToAll(&data);
    }
}

void Channel::Moderate(uint64 p)
{
    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        moderate = !moderate;

        WorldPacket data;
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

    uint32 sec = 0;
    Player *plr = objmgr.GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(players[p].muted)
    {
        WorldPacket data;
        MakeYouCantSpeak(&data);
        SendToOne(&data,p);
    }
    else if(moderate && !players[p].moderator && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        uint32 messageLength = strlen(what) + 1;

        WorldPacket data(SMSG_MESSAGECHAT,1+4+name.size()+1+8+4+messageLength+1);
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
    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(newname);
        if(!newp)
        {
            WorldPacket data;
            MakeNotOn(&data,newname);
            SendToOne(&data,p);
        }
        else if(IsOn(newp->GetGUID()))
        {
            WorldPacket data;
            MakeAlreadyOn(&data,newp->GetGUID());
            SendToOne(&data,p);
        }
        else
        {
            WorldPacket data;
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
    Player *plr = objmgr.GetPlayer(m_ownerGUID);
    if(plr)
        name = plr->GetName();
    else
        name = "PLAYER_NOT_FOUND";

    *MakeNotifyPacket(data,WHOOWNER);
    *data << ((IsConstant() || !m_ownerGUID) ? "Nobody" : name);
}
