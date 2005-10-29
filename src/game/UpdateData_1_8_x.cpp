/* UpdateData.cpp
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

#if defined( _VERSION_1_7_0_ ) || defined( _VERSION_1_8_0_ )

#include "Common.h"
#include "ByteBuffer.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "Log.h"
#include "Opcodes.h"
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

// uses zlib with Z_BEST_SPEED method
// out: dst/dst_size
// *dst_size=0 if error
void UpdateData::Compress(void* dst, uint32 *dst_size, void* src, int src_size)
{
	z_stream c_stream;

	c_stream.zalloc = (alloc_func)0;
	c_stream.zfree = (free_func)0;
	c_stream.opaque = (voidpf)0;

	if (Z_OK != deflateInit(&c_stream, Z_BEST_SPEED))
	{
		Log::getSingleton().outError("Can't compress update packet (zlib: deflateInit).");
		*dst_size = 0;
		return;
	}

	c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = *dst_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;
    
	if (Z_OK != deflate(&c_stream, Z_NO_FLUSH))
	{
		Log::getSingleton().outError("Can't compress update packet (zlib: deflate)");
		*dst_size = 0;
		return;
	}

    if (c_stream.avail_in != 0)
	{
        Log::getSingleton().outError("Can't compress update packet (zlib: deflate not greedy)");
		*dst_size = 0;
		return;
    }

    if (Z_STREAM_END != deflate(&c_stream, Z_FINISH))
	{
        Log::getSingleton().outError("Can't compress update packet (zlib: deflate should report Z_STREAM_END)");
		*dst_size = 0;
		return;
    }

    if (Z_OK != deflateEnd(&c_stream))
	{
        Log::getSingleton().outError("Can't compress update packet (zlib: deflateEnd)");
		*dst_size = 0;
		return;
    }

	*dst_size = c_stream.total_out;
}


bool UpdateData::BuildPacket(WorldPacket *packet)
{
    ByteBuffer buf(m_data.size() + 10 + m_outOfRangeGUIDs.size()*8);

    buf << (uint32) (m_outOfRangeGUIDs.size() > 0 ? m_blockCount + 1 : m_blockCount);
    buf << (uint8) 0; // unknown

    if(m_outOfRangeGUIDs.size())
    {
        buf << (uint8) UPDATETYPE_OUT_OF_RANGE_OBJECTS;
        buf << (uint32) m_outOfRangeGUIDs.size();

        for(std::set<uint64>::const_iterator i = m_outOfRangeGUIDs.begin();
            i != m_outOfRangeGUIDs.end(); i++)
        {
            buf << (uint64) *i;
        }
    }

    buf.append(m_data);

    packet->clear();

    // do not compress small packets
    if (m_data.size() > 50)
    {
        // not sure about that, saw in qz code
        unsigned long destsize = buf.size() + buf.size()/10 + 16;
        packet->resize( destsize );

        packet->put(0, (uint32)buf.size());

        // i know, it's evil
        Compress(const_cast<uint8*>(packet->contents()) + sizeof(uint32),
					&destsize,
					(void*)buf.contents(),
					buf.size());
		if (destsize == 0)
            return false; // Loged by Compress()

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

#endif //defined( _VERSION_1_7_0_ ) || defined( _VERSION_1_8_0_ )

