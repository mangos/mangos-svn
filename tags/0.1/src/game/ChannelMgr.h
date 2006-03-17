/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

class ChannelMgr :  public Singleton < ChannelMgr >
{
    map<string,Channel *> channels;
    void MakeNotOnPacket(WorldPacket *data, const char *name)
    {
        data->Initialize(SMSG_CHANNEL_NOTIFY);
        (*data) << (uint8)0x05 << string(name);
    }
    public:
        Channel *GetJoinChannel(const char *name, Player *p)
        {
            if(channels.count(string(name)) == 0)
            {
                Channel *nchan = new Channel;
                nchan->SetName(name);
                channels[name] = nchan;
            }
            return channels[name];
        }
        Channel *GetChannel(const char *name, Player *p)
        {
            if(channels.count(string(name)) == 0)
            {
                WorldPacket data;
                MakeNotOnPacket(&data,name);
                p->GetSession()->SendPacket(&data);
                return NULL;
            }
            else
                return channels[name];
        }
        void LeftChannel(const char *name)
        {
            if(channels[name]->GetNumPlayers() == 0 && !channels[name]->IsConstant())
            {
                delete channels[name];
                channels.erase(name);
            }
        }
};

#define channelmgr ChannelMgr::getSingleton()
