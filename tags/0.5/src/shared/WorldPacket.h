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

#ifndef MANGOSSERVER_WORLDPACKET_H
#define MANGOSSERVER_WORLDPACKET_H

#include "Common.h"
#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
    public:
        WorldPacket() : ByteBuffer(), m_opcode(0) { }
        WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
        WorldPacket(const WorldPacket &packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

        void Initialize(uint16 opcode )
        {
            clear();
            m_opcode = opcode;
        }

        uint16 GetOpcode() const { return m_opcode; }
        void SetOpcode(uint16 opcode) { m_opcode = opcode; }

    protected:
        uint16 m_opcode;
};
#endif
