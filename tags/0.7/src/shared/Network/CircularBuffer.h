/** \file CircularBuffer.h
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
#ifndef _SOCKETS_CircularBuffer_H
#define _SOCKETS_CircularBuffer_H

#include "sockets-config.h"
#include <string>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class Socket;

/** \defgroup internal Internal utility */

/** Buffer class containing one read/write circular buffer. 
	\ingroup internal */
class CircularBuffer
{
public:
	CircularBuffer(Socket& owner,size_t size);
	~CircularBuffer();

	/** append l bytes from p to buffer */
	bool Write(const char *p,size_t l);
	/** copy l bytes from buffer to dest */
	bool Read(char *dest,size_t l);
	/** copy l bytes from buffer to dest, dont touch buffer pointers */
    bool SoftRead(char *dest, size_t l);
	/** skip l bytes from buffer */
	bool Remove(size_t l);
	/** read l bytes from buffer, returns as string. */
	std::string ReadString(size_t l);

	/** total buffer length */
	size_t GetLength();
	/** pointer to circular buffer beginning */
	const char *GetStart();
	/** return number of bytes from circular buffer beginning to buffer physical end */
	size_t GetL();
	/** return free space in buffer, number of bytes until buffer overrun */
	size_t Space();

	/** return total number of bytes written to this buffer, ever */
	unsigned long ByteCounter(bool clear = false);

private:
	Socket& GetOwner() const;
	CircularBuffer(const CircularBuffer& s) : m_owner( s.GetOwner() ) {}
	CircularBuffer& operator=(const CircularBuffer& ) { return *this; }
	Socket& m_owner;
	char *buf;
	size_t m_max;
	size_t m_q;
	size_t m_b;
	size_t m_t;
	unsigned long m_count;
};


#ifdef SOCKETS_NAMESPACE
}
#endif



#endif // _SOCKETS_CircularBuffer_H
