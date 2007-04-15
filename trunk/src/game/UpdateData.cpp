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

#include "Common.h"
#include "ByteBuffer.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "Log.h"
#include "Opcodes.h"
#include "World.h"
#include <zlib/zlib.h>

UpdateData::UpdateData() : m_blockCount(0)
{
}

void UpdateData::AddOutOfRangeGUID(const uint64 &guid)
{
    m_outOfRangeGUIDs.insert(guid);
}

void UpdateData::AddUpdateBlock(const ByteBuffer &block)
{
    m_data.append(block);
    m_blockCount++;
}

void UpdateData::Compress(void* dst, uint32 *dst_size, void* src, int src_size)
{
    z_stream c_stream;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    // default Z_BEST_SPEED (1)
    if (Z_OK != deflateInit(&c_stream, sWorld.getConfig(CONFIG_COMPRESSION)))
    {
        sLog.outError("Can't compress update packet (zlib: deflateInit).");
        *dst_size = 0;
        return;
    }

    c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = *dst_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;

    if (Z_OK != deflate(&c_stream, Z_NO_FLUSH))
    {
        sLog.outError("Can't compress update packet (zlib: deflate)");
        *dst_size = 0;
        return;
    }

    if (c_stream.avail_in != 0)
    {
        sLog.outError("Can't compress update packet (zlib: deflate not greedy)");
        *dst_size = 0;
        return;
    }

    if (Z_STREAM_END != deflate(&c_stream, Z_FINISH))
    {
        sLog.outError("Can't compress update packet (zlib: deflate should report Z_STREAM_END)");
        *dst_size = 0;
        return;
    }

    if (Z_OK != deflateEnd(&c_stream))
    {
        sLog.outError("Can't compress update packet (zlib: deflateEnd)");
        *dst_size = 0;
        return;
    }

    *dst_size = c_stream.total_out;
}

bool UpdateData::BuildPacket(WorldPacket *packet)
{
    ByteBuffer buf(m_data.size() + 10 + m_outOfRangeGUIDs.size()*8);

    buf << (uint32) (m_outOfRangeGUIDs.size() > 0 ? m_blockCount + 1 : m_blockCount);
    buf << (uint8) 0;

    if(m_outOfRangeGUIDs.size())
    {
        buf << (uint8) UPDATETYPE_OUT_OF_RANGE_OBJECTS;
        buf << (uint32) m_outOfRangeGUIDs.size();

        for(std::set<uint64>::const_iterator i = m_outOfRangeGUIDs.begin();
            i != m_outOfRangeGUIDs.end(); i++)
        {
            buf << (uint8)0xFF;
            buf << (uint64) *i;
        }
    }

    buf.append(m_data);

    packet->clear();

    if (m_data.size() > 50 )
    {
        uint32 destsize = buf.size() + buf.size()/10 + 16;
        packet->resize( destsize );

        packet->put(0, (uint32)buf.size());

        Compress(const_cast<uint8*>(packet->contents()) + sizeof(uint32),
            &destsize,
            (void*)buf.contents(),
            buf.size());
        if (destsize == 0)
            return false;

        packet->resize( destsize + sizeof(uint32) );
        packet->SetOpcode( SMSG_COMPRESSED_UPDATE_OBJECT );
    }
    else
    {
        packet->append( buf );
        packet->SetOpcode( SMSG_UPDATE_OBJECT );
    }

    return true;
}

void UpdateData::Clear()
{
    m_data.clear();
    m_outOfRangeGUIDs.clear();
    m_blockCount = 0;
}
