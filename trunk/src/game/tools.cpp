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

#include "Tools.h"
#include <math.h>

uint64 readGUID(WorldPacket *data)
{
	uint8 guidmark=0;
	uint8 bit;
	uint8 shiftdata=0x1;
	uint64 Temp=0,guid=0;
	*data >> guidmark;
	for(int i=0;i<8;i++)
	{
		if(guidmark & shiftdata)
		{
			Temp = 0;
			*data >> bit;
			Temp = bit;
			Temp <<= i*8;
			guid |= Temp;
			//guid=guid+bit*pow(16,i*2);
		}
	   shiftdata=shiftdata<<1;
	}

	return guid;
}

void  writeGUID(WorldPacket * data)
{
}
