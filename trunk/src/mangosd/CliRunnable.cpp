/* CliRunnable.cpp
 *
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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
#include "Log.h"
#include "World.h"
#include "Master.h"
#include "Timer.h"
#include "Singleton.h"

#ifdef ENABLE_CLI
#include "CliRunnable.h"

/// command buffer
char commandbuf[20] = { 0 };

void Help()
{
 sLog.outString( "These are the commands." );
 sLog.outString( "type 'help' for help" );
 sLog.outString( "type 'info' for server information" );
 sLog.outString( "type 'exit' for exiting daemon" );
}

void CliRunnable::run()
{
    uint32 realCurrTime = 0, realPrevTime = 0;

    while (!Master::m_stopEvent)
    {
        // uint32 exceeded
        if (realPrevTime > realCurrTime)
            realPrevTime = 0;

        realCurrTime = getMSTime();
        //sCli.Update( realCurrTime - realPrevTime );
        realPrevTime = realCurrTime;

        ZThread::Thread::sleep(50);

	printf("command>");
	char *command = fgets(commandbuf,20,stdin);
	if( strncmp(command,"help",4)== 0)
        {
            Help();
        }
	        else if( strncmp(command,"exit",4)== 0)
        {
            DEBUG_LOG( "Exiting daemon..." );
            exit(1);
        }
        else if( strncmp(command,"info",4)== 0)
        {
            const char* online;
            std::stringstream query;
            query << "SELECT COUNT(*) FROM characters WHERE online>0";
            QueryResult *result = sDatabase.Query( query.str().c_str() );
            online = (*result)[0].GetString();
            printf( "Users online: " );
            printf(online);
            printf( "\n" );
            delete result;
        }
        else {
        sLog.outString( "Unknown command." );
        }
	


    }
}

#endif
