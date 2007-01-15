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

#include "Channel.h"
#include "ObjectMgr.h"

void Channel::Join(Player *p, const char *pass)
{
    WorldPacket data;
    if(IsOn(p))
    {
        MakeAlreadyOn(&data,p);
        SendToOne(&data,p);
    }
    else if(IsBanned(p->GetGUID()))
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

        //MakeJoined(&data,p);
        p->JoinedChannel(this);
        /*if(announce)
            SendToAll(&data);*/

        data.clear();
        players[p] = pinfo;

        MakeYouJoined(&data,p);
        SendToOne(&data,p);

        // if no owner first logged will become
        //        if(!constant && owner == NULL)
        //        {
        //            SetOwner(p);
        //            players[p].moderator = true;
        //        }

    }
}

void Channel::Leave(Player *p, bool send)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        if(send) SendToOne(&data,p);
    }
    else
    {
        MakeYouLeft(&data);
        if(send)
        {
            SendToOne(&data,p);
            p->LeftChannel(this);
        }
        data.clear();

        players.erase(p);
        /*MakeLeft(&data,p);
        if(announce)
            SendToAll(&data);

        if(changeowner)
        {
            Player *newowner = players.size() > 0 ? players.begin()->second.player : NULL;
            SetOwner(newowner);
        }*/
    }
}

void Channel::KickOrBan(Player *good, const char *badname, bool ban)
{
    WorldPacket data;
    if(!IsOn(good))
    {
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && good->GetSession()->GetSecurity() < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,good);
    }
    else
    {
        Player *bad = objmgr.GetPlayer(badname);
        if(bad == NULL || !IsOn(bad))
        {
            MakeNotOn(&data,badname);
            SendToOne(&data,good);
        }
        else if(good->GetSession()->GetSecurity() < 2 && bad == owner && good != owner)
        {
            MakeNotOwner(&data);
            SendToOne(&data,good);
        }
        else
        {
            bool changeowner = (owner == bad);

            if(ban && !IsBanned(bad->GetGUID()))
            {
                banned.push_back(bad->GetGUID());
                MakeBanned(&data,good,bad);
            }
            else
                MakeKicked(&data,good,bad);

            SendToAll(&data);
            players.erase(bad);

            if(changeowner)
            {
                Player *newowner = players.size() > 0 ? good : NULL;
                SetOwner(newowner);
            }
        }
    }
}

void Channel::UnBan(Player *good, const char *badname)
{
    WorldPacket data;
    if(!IsOn(good))
    {
        MakeNotOn(&data);
        SendToOne(&data,good);
    }
    else if(!players[good].moderator && good->GetSession()->GetSecurity() < 2)
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
            MakeUnbanned(&data,good,bad);
            SendToAll(&data);
        }
    }
}

void Channel::Password(Player *p, const char *pass)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && p->GetSession()->GetSecurity() < 2)
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

void Channel::SetMode(Player *p, const char *p2n, bool mod, bool set)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && p->GetSession()->GetSecurity() < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(p2n);
        PlayerInfo inf = players[newp];
        if(p == owner && newp == owner && mod)
            return;
        if(newp == NULL || !IsOn(newp))
        {
            MakeNotOn(&data,p2n);
            SendToOne(&data,p);
        }
        else if(owner == newp && owner != p)
        {
            MakeNotOwner(&data);
            SendToOne(&data,p);
        }
        else
        {
            if(mod)
                SetModerator(newp,set);
            else
                SetMute(newp,set);
        }
    }
}

void Channel::SetOwner(Player *p, const char *newname)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(p->GetSession()->GetSecurity() < 2 && p != owner)
    {
        MakeNotOwner(&data);
        SendToOne(&data,p);
    }
    else
    {
        Player *newp = objmgr.GetPlayer(newname);
        if(newp == NULL || !IsOn(newp))
        {
            MakeNotOn(&data,newname);
            SendToOne(&data,p);
        }
        else
        {
            MakeChangeOwner(&data,newp);
            SendToAll(&data);

            SetModerator(newp,true);
            owner = newp;
        }
    }
}

void Channel::GetOwner(Player *p)
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

void Channel::List(Player *p)
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
            data << i->first->GetGUID();
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

void Channel::Announce(Player *p)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && p->GetSession()->GetSecurity() < 2)
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

void Channel::Moderate(Player *p)
{
    WorldPacket data;
    if(!IsOn(p))
    {
        MakeNotOn(&data);
        SendToOne(&data,p);
    }
    else if(!players[p].moderator && p->GetSession()->GetSecurity() < 2)
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

void Channel::Say(Player *p, const char *what, uint32 lang)
{
    if(!what)
        return;
    if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION))
        lang = LANG_UNIVERSAL;

    WorldPacket data;
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
    else if(moderate && !players[p].moderator && p->GetSession()->GetSecurity() < 2)
    {
        MakeNotModerator(&data);
        SendToOne(&data,p);
    }
    else
    {
        uint32 messageLength = strlen(what) + 1;

        data.Initialize(SMSG_MESSAGECHAT);
        data << (uint8)14;
        data << (uint32)lang;                               //language
        data << name.c_str();
        data << (uint32)0;
        data << p->GetGUID();
        data << messageLength;
        data << what;
        data << p->chatTag();

        SendToAll(&data,!players[p].moderator ? p : NULL);
    }
}

void Channel::Invite(Player *p, const char *newname)
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
        if(newp == NULL)
        {
            MakeNotOn(&data,newname);
            SendToOne(&data,p);
        }
        else if(IsOn(newp))
        {
            MakeAlreadyOn(&data,newp);
            SendToOne(&data,p);
        }
        else
        {
            if(!newp->HasInIgnoreList(p->GetGUID()))
            {
                MakeInvited(&data,p);
                SendToOne(&data,newp);
                data.clear();
            }
            MakeYouInvited(&data,newp);
            SendToOne(&data,p);
        }
    }
}
