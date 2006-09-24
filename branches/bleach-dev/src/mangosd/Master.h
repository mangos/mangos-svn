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

#ifndef _MASTER_H
#define _MASTER_H

#include "Common.h"
#include "Config/ConfigEnv.h"
#include "Database/DatabaseEnv.h"
#include "SystemConfig.h"

#include <ace/Singleton.h>

enum eThreadGroup
{
	THR_GRP_REACTOR = 0,
	THR_GRP_MAPMANAGER,
	THR_GRP_SESSION,
	THR_GRP_SIZE
};

class Master
{
    public:
        Master();
        ~Master();
        bool Run();

        static volatile bool m_stopEvent;
    private:
        int _StartDB();
        void _StopDB();

        void _HookSignals();
        //void _UnhookSignals();

        static void _OnSignal(int s);

        void clearOnlineAccounts();
};

typedef ACE_Singleton<Master, ACE_Recursive_Thread_Mutex> MasterSingleton;
#define sMaster MasterSingleton::instance()

#endif
