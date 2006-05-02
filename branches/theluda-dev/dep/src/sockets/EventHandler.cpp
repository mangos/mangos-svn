/** \file EventHandler.cpp
 **	\date  2005-12-07
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2005  Anders Hedstrom

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
#include "EventHandler.h"
#include "IEventOwner.h"
#include "Event.h"
#include "Socket.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


EventHandler::EventHandler(StdLog *p) : SocketHandler(p), m_quit(false)
{
}


EventHandler::EventHandler(Mutex& m,StdLog *p) : SocketHandler(m, p), m_quit(false)
{
}


EventHandler::~EventHandler()
{
}


bool EventHandler::GetTimeUntilNextEvent(struct timeval *tv)
{
	if (!m_events.size())
		return false;
	std::list<Event *>::iterator it = m_events.begin();
	if (it != m_events.end())
	{
		EventTime now;
		mytime_t diff = (*it) -> GetTime() - now;
		tv -> tv_sec = static_cast<long>(diff / 1000000);
		tv -> tv_usec = static_cast<long>(diff % 1000000);
		return true;
	}
	return false;
}


void EventHandler::CheckEvents()
{
	EventTime now;
	std::list<Event *>::iterator it = m_events.begin();
	while (it != m_events.end() && (*it) -> GetTime() < now)
	{
		Event *e = *it;
		Socket *s = dynamic_cast<Socket *>(e -> GetFrom());
		if (!s || (s && Valid(s)))
		{
			e -> GetFrom() -> OnEvent(e -> GetID());
		}
		delete e;
		m_events.erase(it);
		it = m_events.begin();
	}
}


int EventHandler::AddEvent(IEventOwner *from,long sec,long usec)
{
	Event *e = new Event(from, sec, usec);
	std::list<Event *>::iterator it = m_events.begin();
	while (it != m_events.end() && *(*it) < *e)
	{
		it++;
	}
	m_events.insert(it, e);
	return e -> GetID();
}


void EventHandler::ClearEvents(IEventOwner *from)
{
	bool repeat;
	do
	{
		repeat = false;
		for (std::list<Event *>::iterator it = m_events.begin(); it != m_events.end(); it++)
		{
			Event *e = *it;
			if (e -> GetFrom() == from)
			{
				delete e;
				m_events.erase(it);
				repeat = true;
				break;
			}
		}
	} while (repeat);
}


void EventHandler::EventLoop()
{
	while (!m_quit)
	{
		struct timeval tv;
		if (GetTimeUntilNextEvent(&tv))
		{
			Select(&tv);
			CheckEvents();
		}
		else
		{
			Select();
		}
	}
}


void EventHandler::SetQuit(bool x)
{
	m_quit = x;
}


#ifdef SOCKETS_NAMESPACE
}
#endif
