/**
 **	File ......... Thread.cpp
 **	Published ....  2004-10-30
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
#include <stdio.h>
#ifdef _WIN32
#include "socket_include.h"
#else
#include <unistd.h>
#endif

#include "Thread.h"

#ifndef __GNUC__

// UQ1: warning C4311: 'type cast' : pointer truncation
#pragma warning(disable:4311)

#endif  

Thread::Thread(bool release)
:m_thread(0)
,m_running(true)
,m_release(false)
{
#ifdef _WIN32
    m_thread = ::CreateThread(NULL, 0, StartThread, this, 0, &m_dwThreadId);
#else
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    if (pthread_create(&m_thread,&attr,StartThread,this) == -1)
    {
        perror("Thread: create failed");
        SetRunning(false);
    }
//	pthread_attr_destroy(&attr);
#endif
    m_release = release;
}


Thread::~Thread()
{
//	while (m_running || m_thread)
    if (m_running)
    {
        SetRunning(false);
        SetRelease(true);

#ifdef _WIN32
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        select(0,NULL,NULL,NULL,&tv);
        ::CloseHandle(m_thread);
#else
        sleep(1);
#endif
    }
}


threadfunc_t STDPREFIX Thread::StartThread(threadparam_t zz)
{
    Thread *pclThread = (Thread *)zz;

    while (pclThread -> m_running && !pclThread -> m_release)
    {
#ifdef _WIN32
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        select(0,NULL,NULL,NULL,&tv);
#else
        sleep(1);
#endif
    }
    if (pclThread -> m_running)
    {
        pclThread -> Run();
    }
    pclThread -> SetRunning(false);               // if return
    return (threadfunc_t)zz;
}


bool Thread::IsRunning()
{
    return m_running;
}


void Thread::SetRunning(bool x)
{
    m_running = x;
}


bool Thread::IsReleased()
{
    return m_release;
}


void Thread::SetRelease(bool x)
{
    m_release = x;
}
