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

#include "ChannelMgr.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( ChannelMgr );

void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    string channelname,pass;
    recvPacket >> channelname;
    recvPacket >> pass;
    channelmgr.GetJoinChannel(channelname.c_str(),GetPlayer())->Join(GetPlayer(),pass.c_str());
}

void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;

    if(!channelname.length())
        return;

    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Leave(GetPlayer());
    channelmgr.LeftChannel(channelname.c_str());
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->List(GetPlayer());
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    string channelname,pass;
    recvPacket >> channelname;
    recvPacket >> pass;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Password(GetPlayer(),pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    string channelname,newp;
    recvPacket >> channelname;
    recvPacket >> newp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->SetOwner(GetPlayer(),newp.c_str());
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->GetOwner(GetPlayer());
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->SetModerator(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->UnsetModerator(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->SetMute(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->UnsetMute(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Invite(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Kick(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Ban(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->UnBan(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Announce(GetPlayer());
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    Channel *chn = channelmgr.GetChannel(channelname.c_str(),GetPlayer()); if(chn) chn->Moderate(GetPlayer());
}
