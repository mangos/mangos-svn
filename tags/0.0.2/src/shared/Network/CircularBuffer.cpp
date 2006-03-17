/**
 **	File ......... CircularBuffer.cpp
 **	Published ....  2004-02-13
 **	Author ....... grymse@alhem.net
 **/
/*
Copyright (C) 2004,2005  Anders Hedstrom

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
#include <stdio.h>
#include <string.h>

#include "Socket.h"
#include "SocketHandler.h"
#include "CircularBuffer.h"

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

CircularBuffer::CircularBuffer(Socket& owner,size_t size)
:m_owner(owner)
,buf(new char[size])
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
        return false;                             // overflow
    }
    m_count += (unsigned long)l;
    if (m_t + l > m_max)                          // block crosses circular border
    {
        size_t l1 = m_max - m_t;                  // size left until circular border crossing
        memcpy(buf + m_t, s, l1);
        memcpy(buf, s + l1, l - l1);
        m_t = l - l1;
        m_q += l;
    }
    else
    {
        memcpy(buf + m_t, s, l);
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
        return false;                             // not enough chars
    }
    if (m_b + l > m_max)                          // block crosses circular border
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
