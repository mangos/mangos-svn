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

#include "ChannelMgr.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( AllianceChannelMgr );
INSTANTIATE_SINGLETON_1( HordeChannelMgr );

void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    string channelname,pass;
    recvPacket >> channelname;
    recvPacket >> pass;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        cMgr->GetJoinChannel(channelname,GetPlayer())->Join(GetPlayer(),pass.c_str());
}

void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;

    if(!channelname.length())
        return;

    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
    {
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Leave(GetPlayer());
        cMgr->LeftChannel(channelname);
    }
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->List(GetPlayer());
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    string channelname,pass;
    recvPacket >> channelname;
    recvPacket >> pass;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Password(GetPlayer(),pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    string channelname,newp;
    recvPacket >> channelname;
    recvPacket >> newp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->SetOwner(GetPlayer(),newp.c_str());
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->GetOwner(GetPlayer());
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->SetModerator(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->UnsetModerator(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->SetMute(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->UnsetMute(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Invite(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Kick(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Ban(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    string channelname,otp;
    recvPacket >> channelname;
    recvPacket >> otp;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->UnBan(GetPlayer(),otp.c_str());
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Announce(GetPlayer());
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    string channelname;
    recvPacket >> channelname;
    if(ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
        if(Channel *chn = cMgr->GetChannel(channelname,GetPlayer())) chn->Moderate(GetPlayer());
}
