/** \file Uid.cpp
 **	\date  2004-04-06
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

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
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <stdio.h>
#include <assert.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <objbase.h>
#elif defined MACOSX
#include <string.h>
#include <uuid/uuid.h>
#elif defined __FreeBSD__
# if __FreeBSD_version >= 500000
#  include <uuid.h>
# else
#  error FreeBSD versions prior to 500000 does not support uuid(3)
# endif
#else
#include <uuid/uuid.h>
#endif

#include "Uid.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


//e682119c-dea0-4e09-acf4-e66b8c522e99


Uid::Uid()
{
#if defined(_WIN32) || defined(__CYGWIN__)
	GUID randomGuid;
	// create random GUID
	randomGuid = GUID_NULL;
	::CoCreateGuid(&randomGuid);
	if (randomGuid == GUID_NULL)
	{
		fprintf(stderr,"Couldn't create a random GUID\n");
		return;
	}
	memcpy(m_bufuid, &randomGuid, 16);
#elif defined __FreeBSD__
	uuid_t uid; // uuid_t is a struct
	uuid_create(&uid, NULL);
	memcpy(m_bufuid, &uid, 16);
#else
	uuid_t uid; // uuid_t is defined as unsigned char[16]
	uuid_generate(uid);
	memcpy(m_bufuid, uid, 16);
#endif
}


Uid::Uid(const std::string& uidstr)
{
	unsigned uid[16];
	sscanf(uidstr.c_str(),"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		&uid[0],&uid[1],&uid[2],&uid[3],
		&uid[4],&uid[5],&uid[6],&uid[7],
		&uid[8],&uid[9],&uid[10],&uid[11],
		&uid[12],&uid[13],&uid[14],&uid[15]);
	for (int i = 0; i < 16; i++)
		m_bufuid[i] = (unsigned char)uid[i];
}


Uid::Uid(unsigned char *buf)
{
	memcpy(m_bufuid, buf, 16);
}


Uid::~Uid()
{
}


std::string Uid::GetUid()
{
	std::string tmp;
	char slask[100]; // Uid::GetUid temporary

	sprintf(slask,"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		m_bufuid[0],m_bufuid[1],m_bufuid[2],m_bufuid[3],
		m_bufuid[4],m_bufuid[5],m_bufuid[6],m_bufuid[7],
		m_bufuid[8],m_bufuid[9],m_bufuid[10],m_bufuid[11],
		m_bufuid[12],m_bufuid[13],m_bufuid[14],m_bufuid[15]);
	tmp = slask;
	return tmp;
}


const unsigned char *Uid::GetBuf() 
{ 
	return m_bufuid; 
}


#ifdef SOCKETS_NAMESPACE
}
#endif

