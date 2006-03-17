/* NPCHandler.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#ifndef __NPCHANDLER_H
#define __NPCHANDLER_H

enum GOSSIP_SPECIAL
{
    GOSSIP_NO_SPECIAL           = 0x00,
    GOSSIP_POI                  = 0x01,
    GOSSIP_SPIRIT_HEALER_ACTIVE = 0x02,
    GOSSIP_VENDOR               = 0x03,
    GOSSIP_TRAINER              = 0x04,
    GOSSIP_TABARD_VENDOR        = 0x05,
    GOSSIP_INNKEEPER            = 0x06,
};

struct GossipText
{
    uint32 ID;
    std::string Text;
};

struct GossipOptions
{
    //uint32 ID;
    uint32 Guid;
    uint16 Icon;
    std::string OptionText;
    uint32 NextTextID;
    uint32 Special;
    //float PoiX;
    //float PoiY;
    //float PoiZ;
};

struct GossipNpc
{
    //GossipNpc(uint32 id=0, uint32 guid=0, uint32 tx_id=0, uint32 count=0) : ID(id), Guid(guid),TextID(tx_id),OptionCount(count),pOptions(NULL) {}
    GossipNpc(uint32 guid=0, uint32 tx_id=0, uint32 count=0) : Guid(guid),TextID(tx_id),OptionCount(count), pOptions(NULL) {}
    //uint32 ID;
    uint32 Guid;
    uint32 TextID;
    uint32 OptionCount;
    GossipOptions *pOptions;
};
#endif
