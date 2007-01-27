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

#ifndef WIN32
#include <sys/timeb.h>
#endif

typedef void (*EventHandler) (void *arg);

#define ES_RESOLUTION 33

typedef struct
{

    EventHandler handler;
    void *param;
    uint32 time;
    bool st;

    void * pNext;

}Event;

typedef struct
{

    EventHandler handler;
    void *param;
    uint32 time;
    bool st;
    uint32 id;
    uint32 period;
    void * pNext;

}PeriodicEvent;

uint32 AddEvent(EventHandler  func,void* param,uint32 timer,bool separate_thread=false,bool bPeriodic = false);
void RemovePeriodicEvent(uint32 eventid);
void StartEventSystem();
void RemoveEvent(uint32 eventid);
