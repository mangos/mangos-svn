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

#include "Common.h"
#include "Mthread.h"
#include "EventSystem.h"

#ifndef WIN32
#include <sys/timeb.h>
#endif

PeriodicEvent *msPEvents=NULL;
PeriodicEvent *sPEvents=NULL;
PeriodicEvent *mPEvents=NULL;

Event *msEvents=NULL;
Event *sEvents=NULL;
Event *mEvents=NULL;

static uint32 eventid=0;
uint32 avalabelid=0;

volatile uint32 rmse=0;
volatile uint32 wmse=0;
volatile uint32 rse=0;
volatile uint32 wse=0;
volatile uint32 rme=0;
volatile uint32 wme=0;

volatile uint32 rmspe=0;
volatile uint32 wmspe=0;
volatile uint32 rspe=0;
volatile uint32 wspe=0;
volatile uint32 rmpe=0;
volatile uint32 wmpe=0;

#define write_mse while(rmse||wmse);wmse=1;
#define end_write_mse wmse=0;

#define read_mse while(wmse);rmse++;
#define end_read_mse rmse--;
#define expand_mse rmse--;write_mse

#define write_se while(rse||wse);wse=1;
#define end_write_se wse=0;

#define read_se while(wse);rse++;
#define end_read_se rse--;
#define expand_se rse--;write_se

#define write_me while(rme||wme);wme=1;
#define end_write_me wme=0;

#define read_me while(wme);rme++;
#define end_read_me rme--;
#define expand_me rme--;write_me

//----------------------------
#define write_mspe while(rmspe||wmspe);wmspe=1;
#define end_write_mspe wmspe=0;

#define read_mspe while(wmspe);rmspe++;
#define end_read_mspe rmspe--;
#define expand_mspe rmspe--;write_mspe

#define write_spe while(rspe||wspe);wspe=1;
#define end_write_spe wspe=0;

#define read_spe while(wspe);rspe++;
#define end_read_spe rspe--;
#define expand_spe rspe--;write_spe

#define write_mpe while(rmpe||wmpe);wmpe=1;
#define end_write_mpe wmpe=0;

#define read_mpe while(wmpe);rmpe++;
#define end_read_mpe rmpe--;
#define expand_mpe rmpe--;write_mpe

#if PLATFORM == PLATFORM_WIN32
#define MSleep Sleep
#else
#define MSleep sleep
#endif

bool stopEvent = false;

inline uint32 now()
{
    #if PLATFORM == PLATFORM_WIN32
    return timeGetTime();
    #else
    struct timeb tp;
    ftime(&tp);

    return  tp.time * 1000 + tp.millitm;
    #endif
}

void PushmsEvent(Event * event)
{
    Event *pos;
    read_mse
        if(msEvents)
    {
        pos=msEvents;
        while(pos->pNext && ((Event *)(pos->pNext))->time <=event->time)
            pos=(Event*)pos->pNext;

        event->pNext =pos->pNext ;

        expand_mse
            pos->pNext =event;

    }
    else
    {
        event->pNext =NULL;
        expand_mse
            msEvents =event;

    }
    end_write_mse
}

void PushsEvent(Event * event)
{
    Event *pos;
    read_se
        if(sEvents)
    {
        pos=sEvents;
        while(pos->pNext && ((Event *)(pos->pNext))->time <=event->time)
            pos=(Event*)pos->pNext;

        event->pNext =pos->pNext ;

        expand_se
            pos->pNext =event;

    }
    else
    {
        event->pNext =NULL;
        expand_se
            sEvents =event;
    }
    end_write_se
}

void PushmEvent(Event * event)
{
    Event *pos;
    read_me
        if(mEvents)
    {
        pos=mEvents;
        while(pos->pNext && ((Event *)(pos->pNext))->time <=event->time)
            pos=(Event*)pos->pNext;

        event->pNext =pos->pNext;
        expand_me
            pos->pNext =event;
    }
    else
    {
        event->pNext =NULL;
        expand_me
            mEvents =event;
    }
    end_write_me
}

uint32 AddEvent(EventHandler func,void* param,uint32 timer,bool separate_thread,bool bPeriodic)
{
    if(bPeriodic)
    {
        if(avalabelid==0)
        {
            eventid++;
            avalabelid=eventid;
        }
        PeriodicEvent *event=new PeriodicEvent;

        event->handler=func;
        event->param =param;
        event->period =timer;
        event->st=separate_thread;
        event->id=avalabelid;
        event->time =now()+timer;

        if(timer<=60000)                                    //less then a minute
        {
            write_mspe
                event->pNext =msPEvents;
            msPEvents=event;
            end_write_mspe
        }
        else if(timer<=3600000)                             //hour
        {
            write_spe
                event->pNext =sPEvents;
            sPEvents=event;
            end_write_spe
        }
        else
        {
            write_mpe
                event->pNext =mPEvents;
            mPEvents=event;
            end_write_mpe
        }
        uint32 useid=avalabelid;
        avalabelid=0;
        return useid;

    }else
    {
        Event * event=new Event;

        event->handler=func;
        event->param =param;
        event->st =separate_thread;
        event->time =now()+timer;

        if(timer<=60000)                                    //less then a minute
        {
            if(rmse||wmse)
            {
                MThread::Start(( void (*)(void*))&PushmsEvent,event);
            }
            else PushmsEvent(event);
        }
        else if(timer<=3600000)                             //hour
        {
            if(rse||wse)
            {
                MThread::Start(( void (*)(void*))&PushsEvent,event);
            }
            else PushsEvent(event);
        }
        else
        {
            if(rme||wme)
            {
                MThread::Start(( void (*)(void*))&PushmEvent,event);
            }
            else PushmEvent(event);
        }
        return 0;
    }
    return 0;
}

void RemoveEvent(uint32 eventid)
{
    /*MThread* tr=new MThread;
    tr->Start(( void (*)(void*))&RemovePeriodicEvent,(void*)&eventid);*/
}

void RemovePeriodicEvent(uint32 eventid)
{
    //uint32 eventid = *(uint32*)etid;
    PeriodicEvent * prev=NULL;
    read_mspe
        PeriodicEvent * pos=msPEvents;

    while(pos)
    {
        if(pos->id==eventid)
        {
            expand_mspe
                if(prev)
                prev->pNext=pos->pNext ;
            else
                msPEvents=(PeriodicEvent*)pos->pNext ;

            delete pos;
            if(avalabelid==0)
                avalabelid=eventid;
            end_write_mspe
                return;

        }else
        {
            prev=pos;
            pos=(PeriodicEvent*)pos->pNext ;
        }

    }
    end_read_mspe

        prev=NULL;

    read_spe
        pos=sPEvents;

    while(pos)
    {
        if(pos->id==eventid)
        {
            expand_spe
                if(prev)
                prev->pNext=pos->pNext ;
            else
                sPEvents=(PeriodicEvent*)pos->pNext ;

            delete pos;
            if(avalabelid==0)
                avalabelid=eventid;
            end_write_spe
                return;

        }else
        {
            prev=pos;
            pos=(PeriodicEvent*)pos->pNext ;
        }

    }
    end_read_spe

        prev=NULL;

    read_mpe
        pos=mPEvents;

    while(pos)
    {
        if(pos->id==eventid)
        {
            expand_mpe
                if(prev)
                prev->pNext=pos->pNext ;
            else
                mPEvents=(PeriodicEvent*)pos->pNext ;

            delete pos;
            if(avalabelid==0)
                avalabelid=eventid;
            end_write_mpe
                return;

        }else
        {
            prev=pos;
            pos=(PeriodicEvent*)pos->pNext;
        }

    }
    end_read_mpe
        return;
}

void msThread()
{

    uint32 cur=now();

    while(!stopEvent)
    {
        MSleep(ES_RESOLUTION-(now()-cur));                  //execution time compensation
        if(stopEvent) break;

        write_mse
            Event * pos=msEvents;
        cur=now();
        while(pos && cur >= pos->time)
        {
            if(pos->st)
            {
                MThread::Start(pos->handler,pos->param );
            }
            else
                pos->handler (pos->param );

            msEvents=(Event*)pos->pNext ;
            delete pos;
            pos=msEvents;
        }
        end_write_mse
    }

}

void mspThread()
{
    uint32 cur=now();

    while(!stopEvent)
    {
        MSleep(ES_RESOLUTION-(now()-cur));                  //execution time compensation
        if(stopEvent) break;

        read_mspe
            PeriodicEvent * ppos=msPEvents;
        cur=now();

        while(ppos)
        {
            if(cur >= ppos->time)
            {
                if(ppos->st)
                {
                    MThread::Start(ppos->handler,ppos->param );
                }
                else
                    ppos->handler (ppos->param );

                ppos->time=cur+ppos->period ;
            }

            ppos=(PeriodicEvent*)ppos->pNext;
        }
        end_read_mspe
    }
}

void sThread()
{

    uint32 cur=now();

    while(!stopEvent)
    {
        MSleep(ES_RESOLUTION*30-(now()-cur));
        if(stopEvent) break;

        write_se
            Event * pos=sEvents;
        cur=now();

        while(pos && cur >= pos->time)
        {
            if(pos->st)
            {
                MThread::Start(pos->handler,pos->param );
            }
            else
                pos->handler (pos->param );

            sEvents=(Event*)pos->pNext ;
            delete pos;
            pos=sEvents;
        }

        end_write_se

    }

}

void spThread()
{
    uint32 cur=now();

    while(!stopEvent)
    {
        MSleep(ES_RESOLUTION*30-(now()-cur));
        if(stopEvent) break;

        read_spe
            PeriodicEvent * ppos=sPEvents;
        cur=now();
        while(ppos)
        {
            if(cur >= ppos->time)
            {
                if(ppos->st)
                {
                    MThread::Start(ppos->handler,ppos->param );
                }
                else
                    ppos->handler (ppos->param );

                ppos->time=cur+ppos->period;
            }

            ppos=(PeriodicEvent*)ppos->pNext;
        }
        end_read_spe
    }
}

void mThread()
{

    uint32 cur=now();

    while(!stopEvent)
    {
        // respond fast to shutdown events
        MSleep(ES_RESOLUTION*30-(now()-cur));
        for(int cnt = 0; cnt < 59 && !stopEvent; cnt++)
            MSleep(ES_RESOLUTION*30);

        if(stopEvent) break;

        write_me
            Event * pos=mEvents;
        cur=now();

        while(pos && cur >= pos->time)
        {
            if(pos->st)
            {
                MThread::Start(pos->handler,pos->param );
            }
            else
                pos->handler (pos->param );

            mEvents=(Event*)pos->pNext ;
            delete pos;
            pos=mEvents;
        }

        end_write_me
    }

}

void mpThread()
{
    uint32 cur=now();

    while(!stopEvent)
    {
        // respond fast to shutdown events
        MSleep(ES_RESOLUTION*30-(now()-cur));
        for(int cnt = 0; cnt < 59 && !stopEvent; cnt++)
            MSleep(ES_RESOLUTION*30);

        if(stopEvent) break;

        read_mpe
            PeriodicEvent * ppos=mPEvents;
        cur=now();
        while(ppos)
        {
            if(cur >= ppos->time)
            {
                if(ppos->st)
                {
                    MThread::Start(ppos->handler,ppos->param );
                }
                else
                    ppos->handler (ppos->param );

                ppos->time=cur+ppos->period;
            }

            ppos=(PeriodicEvent*)ppos->pNext;
        }

        end_read_mpe
    }

}

MThread *tm = NULL, *ts = NULL, *tms = NULL, *tmp = NULL, *tsp = NULL, *tmsp = NULL;

void StartEventSystem()
{
    return;
    tm = MThread::Start(( void (*)(void*))&mThread,NULL);
    ts = MThread::Start(( void (*)(void*))&sThread,NULL);
    tms = MThread::Start(( void (*)(void*))&msThread,NULL);
    tmp = MThread::Start(( void (*)(void*))&mpThread,NULL);
    tsp = MThread::Start(( void (*)(void*))&spThread,NULL);
    tmsp = MThread::Start(( void (*)(void*))&mspThread,NULL);
}

void StopEventSystem()
{
    return;
    stopEvent = true;

#ifdef WIN32
    HANDLE h[6]= {tm->th, ts->th, tms->th, tmp->th, tsp->th, tmsp->th};
    WaitForMultipleObjects(6, h, TRUE, INFINITE);
#endif

    while(msEvents) { Event *p = msEvents; msEvents = (Event*)p->pNext; delete p;  }
    while(sEvents) { Event *p = sEvents; sEvents = (Event*)p->pNext; delete p; }
    while(mEvents) { Event *p = mEvents; mEvents = (Event*)p->pNext; delete p; }

    while(msPEvents) { PeriodicEvent *p = msPEvents; msPEvents = (PeriodicEvent*)p->pNext; delete p; }
    while(sPEvents) { PeriodicEvent *p = sPEvents; sPEvents = (PeriodicEvent*)p->pNext; delete p; }
    while(mPEvents) { PeriodicEvent *p = mPEvents; mPEvents = (PeriodicEvent*)p->pNext; delete p; }

    delete tm;
    delete ts;
    delete tms;
    delete tmp;
    delete tsp;
    delete tmsp;
}
