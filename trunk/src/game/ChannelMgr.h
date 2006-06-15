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
#include "Policies/Singleton.h"

class ChannelMgr
{
    typedef map<string,Channel *> ChannelMap;
    ChannelMap channels;
    void MakeNotOnPacket(WorldPacket *data, std::string name)
    {
        data->Initialize(SMSG_CHANNEL_NOTIFY);
        (*data) << (uint8)0x05 << name;
    }
    public:
        Channel *GetJoinChannel(std::string name, Player *p)
        {
            if(channels.count(name) == 0)
            {
                Channel *nchan = new Channel;
                nchan->SetName(name);
                channels[name] = nchan;
            }
            return channels[name];
        }
        Channel *GetChannel(std::string name, Player *p)
        {
            ChannelMap::const_iterator i = channels.find(name);

            if(i == channels.end())
            {
                WorldPacket data;
                MakeNotOnPacket(&data,name);
                p->GetSession()->SendPacket(&data);
                return NULL;
            }
            else
                return i->second;
        }
        void LeftChannel(std::string name)
        {
            ChannelMap::const_iterator i = channels.find(name);

            if(i == channels.end()) return;

            Channel* channel = i->second;

            if(channel->GetNumPlayers() == 0 && !channel->IsConstant())
            {
                channels.erase(name);
                delete channel;
            }
        }
};

#define channelmgr MaNGOS::Singleton<ChannelMgr>::Instance()
