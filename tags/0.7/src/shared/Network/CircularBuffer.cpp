/** \file CircularBuffer.cpp
 **	\date  2004-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004-2007  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "CircularBuffer.h"
#include <string.h>

#include "Socket.h"
#include "SocketHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


CircularBuffer::CircularBuffer(Socket& owner,size_t size)
:m_owner(owner)
,buf(new char[2 * size])
,m_max(size)
,m_q(0)
,m_b(0)
,m_t(0)
,m_count(0)
{
}


CircularBuffer::~CircularBuffer()
{
	delete[] buf;
}


bool CircularBuffer::Write(const char *s,size_t l)
{
	if (m_q + l > m_max)
	{
		m_owner.Handler().LogError(&m_owner, "CircularBuffer::Write", -1, "write buffer overflow");
		return false; // overflow
	}
	m_count += (unsigned long)l;
	if (m_t + l > m_max) // block crosses circular border
	{
		size_t l1 = m_max - m_t; // size left until circular border crossing
		// always copy full block to buffer(buf) + top pointer(m_t)
		// because we have doubled the buffer size for performance reasons
		memcpy(buf + m_t, s, l);
		memcpy(buf, s + l1, l - l1);
		m_t = l - l1;
		m_q += l;
	}
	else
	{
		memcpy(buf + m_t, s, l);
		memcpy(buf + m_max + m_t, s, l);
		m_t += l;
		if (m_t >= m_max)
			m_t -= m_max;
		m_q += l;
	}
	return true;
}


bool CircularBuffer::Read(char *s,size_t l)
{
	if (l > m_q)
	{
		m_owner.Handler().LogError(&m_owner, s ? "CircularBuffer::Read" : "CircularBuffer::Write", -1, "attempt to read beyond buffer");
		return false; // not enough chars
	}
	if (m_b + l > m_max) // block crosses circular border
	{
		size_t l1 = m_max - m_b;
		if (s)
		{
			memcpy(s, buf + m_b, l1);
			memcpy(s + l1, buf, l - l1);
		}
		m_b = l - l1;
		m_q -= l;
	}
	else
	{
		if (s)
		{
			memcpy(s, buf + m_b, l);
		}
		m_b += l;
		if (m_b >= m_max)
			m_b -= m_max;
		m_q -= l;
	}
	if (!m_q)
	{
		m_b = m_t = 0;
	}
	return true;
}

bool CircularBuffer::SoftRead(char *s, size_t l)
{
    if (l > m_q)
    {
        return false;
    }
    if (m_b + l > m_max)                          // block crosses circular border
    {
        size_t l1 = m_max - m_b;
        if (s)
        {
            memcpy(s, buf + m_b, l1);
            memcpy(s + l1, buf, l - l1);
        }
    }
    else
    {
        if (s)
        {
            memcpy(s, buf + m_b, l);
        }
    }
    return true;
}

bool CircularBuffer::Remove(size_t l)
{
	return Read(NULL, l);
}


size_t CircularBuffer::GetLength()
{
	return m_q;
}


const char *CircularBuffer::GetStart()
{
	return buf + m_b;
}


size_t CircularBuffer::GetL()
{
	return (m_b + m_q > m_max) ? m_max - m_b : m_q;
}


size_t CircularBuffer::Space()
{
	return m_max - m_q;
}


unsigned long CircularBuffer::ByteCounter(bool clear)
{
	if (clear)
	{
		unsigned long x = m_count;
		m_count = 0;
		return x;
	}
	return m_count;
}


Socket& CircularBuffer::GetOwner() const 
{ 
	return m_owner; 
}


std::string CircularBuffer::ReadString(size_t l)
{
	char *sz = new char[l + 1];
	if (!Read(sz, l)) // failed, debug printout in Read() method
	{
		delete[] sz;
		return "";
	}
	sz[l] = 0;
	std::string tmp = sz;
	delete[] sz;
	return tmp;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

