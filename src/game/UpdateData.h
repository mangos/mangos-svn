/* UpdateData.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef __UPDATEDATA_H
#define __UPDATEDATA_H

class WorldPacket;

enum OBJECT_UPDATE_TYPE
{
    UPDATETYPE_VALUES = 0,
//  8 bytes - GUID
//  Goto Update Block
    UPDATETYPE_MOVEMENT = 1,
//  8 bytes - GUID
//  Goto Position Update
    UPDATETYPE_CREATE_OBJECT = 2,
//  8 bytes - GUID
//  1 byte - Object Type (*)
//  Goto Position Update
//  Goto Update Block
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 3,
//  4 bytes - Count
//  Loop Count Times:
//  8 bytes - GUID
    UPDATETYPE_NEAR_OBJECTS = 4                   // looks like 3 & 4 do the same thing
//  4 bytes - Count
//  Loop Count Times:
//  8 bytes - GUID
};

class UpdateData
{
    public:
        UpdateData();

        void AddOutOfRangeGUID(const uint64 &guid);
        void AddUpdateBlock(const ByteBuffer &block);
        bool BuildPacket(WorldPacket *packet);
        bool HasData() { return m_blockCount > 0 || m_outOfRangeGUIDs.size() > 0; }
        void Clear();

    protected:
        uint32 m_blockCount;
        std::set<uint64> m_outOfRangeGUIDs;
        ByteBuffer m_data;
};
#endif
