/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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


char commandbuf[40] = { 0 };
char charcr[10] = { 0 };
char pass[10] = { 0 };

void Help()
{
 sLog.outString( "These are the commands." );
 sLog.outString( "type 'help' for help" );
 sLog.outString( "type 'info' for server information" );
 sLog.outString( "type 'ban' for banning IP or Character" );
 sLog.outString( "type 'removeban' for removing bans" );
 sLog.outString( "type 'setgm' for setting gamemasters" );
 sLog.outString( "type 'create' for creatng character" );
 sLog.outString( "type 'exit' for exiting daemon" );
}

void CliRunnable::run()
{
    uint32 realCurrTime = 0, realPrevTime = 0;

    while (!Master::m_stopEvent)
    {
        
        if (realPrevTime > realCurrTime)
            realPrevTime = 0;

        realCurrTime = getMSTime();
        
        realPrevTime = realCurrTime;

        ZThread::Thread::sleep(50);

	printf("command>");
	char charcr[10] = { 0 };
	char pass[10] = { 0 };
	char *command = fgets(commandbuf,40,stdin);
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
            std::stringstream query1,query2;
	    Field *fields;
            query1 << "SELECT COUNT(*) FROM characters WHERE online>0";
            QueryResult *result = sDatabase.Query( query1.str().c_str() );
		
		if (result)
		 {
	            int cnt = 0;
        	    fields = result->Fetch();
	            cnt = fields[0].GetUInt32();

	            if ( cnt > 0 )
        	    {
			sLog.outString("Online users: %d",cnt);

              query2 << "SELECT c.name,c.last_ip,c.acct,a.gm,a.login FROM characters c,accounts a where c.online>0 and a.acct=c.acct";
              QueryResult *result = sDatabase.Query( query2.str().c_str() );

		
		int i = 0;
		while ( i != 56)
		{
		printf("=");
		i++;
		}
		printf("\n");
		printf("|    Account    ");
		printf("|   Character   ");
		printf("|      IP       ");
		printf("|  GM  |\n");
		i = 0;
		while ( i != 56)
		{
		printf("=");
		i++;
		}
		printf("\n");

		do {
			fields = result->Fetch();
			printf("|%15s|", fields[4].GetString());
			printf("%15s|",fields[0].GetString());
			printf("%15s|",fields[1].GetString());
			printf("%6d|\n", fields[3].GetUInt32());
		}while( result->NextRow() );

		i = 0;
		while ( i != 56)
		{
		printf("=");
		i++;
		}
		printf("\n");
		delete result;
		    }

		    else {
			sLog.outString("NO online users");
		    }

		}

        }

        else if( strncmp(command,"\n",1)== 0)
        {
	}
	else if( strncmp(command,"ban",3) == 0)
	{
		int i = 4;
		char banip[16] = { 0 };

		for ( i = 4; command[i] && command[i] != '\n'; i++ )
		banip[i-4] = command[i];

		if (strncmp(command+4,"",1) == 0)
		{

            std::stringstream query1,query2;
	    Field *fields;
            query1 << "SELECT COUNT(*) FROM accounts WHERE banned>0";
            QueryResult *result = sDatabase.Query( query1.str().c_str() );
		
		if (result)
		 {
	            int cnt = 0;
        	    fields = result->Fetch();
	            cnt = fields[0].GetUInt32();

	            if ( cnt > 0 )
        	    {
			sLog.outString("");
			sLog.outString("Banned users:");
		        query2 << "SELECT login FROM accounts WHERE banned>0";
		        QueryResult *result2 = sDatabase.Query( query2.str().c_str() );

		do {
			fields = result2->Fetch();
			printf("|%5s|\n", fields[0].GetString());
		}while( result2->NextRow() );
			delete result2;
			}

            std::stringstream query1,query2;
	    Field *fields;
            query1 << "SELECT COUNT(*) FROM ipbantable WHERE ip>0";
            QueryResult *result = sDatabase.Query( query1.str().c_str() );
		
		if (result)
		 {
	            int cnt = 0;
        	    fields = result->Fetch();
	            cnt = fields[0].GetUInt32();

	            if ( cnt > 0 )
        	    {
			sLog.outString("");
			sLog.outString("Banned IPs:");
		        query2 << "SELECT ip FROM ipbantable";
		        QueryResult *result3 = sDatabase.Query( query2.str().c_str() );

		do {
			fields = result3->Fetch();
			printf("|%5s|\n", fields[0].GetString());
		}while( result3->NextRow() );
			delete result3;
			}

		    }
		    else {
			sLog.outString("We don't have banned users");
			}
		 }

		delete result;
		sLog.outString("");
		sLog.outString("Syntax is: ban character or ip");
		}
		else if(isdigit(command[4]))
		{
		std::stringstream ipban;
		ipban << "INSERT into `ipbantable` values('" << banip << "')" ;
		sDatabase.Execute(ipban.str().c_str());
		sLog.outString("We banned IP: %s",banip);
		}
		else {
		std::stringstream ipban;
		ipban << "UPDATE `accounts` SET banned = '1' WHERE login = '" << banip << "'";
		sDatabase.Execute(ipban.str().c_str());
		sLog.outString("We banned Character: %s",banip);
		}
		}	

	
	else if( strncmp(command,"removeban",9) == 0)
	{
	}

	
	else if( strncmp(command,"setgm",5) == 0)
	{
		int i = 0;
		for ( i = 6; command[i] && command[i] != '\n'; i++ )
		charcr[i-6] = command[i];

		if (strncmp(command+6,"",1) == 0)
		{

		
		std::stringstream query,query2;
		Field *fields;
		query << "SELECT COUNT(*) FROM accounts WHERE gm > 0";
		QueryResult *result1 = sDatabase.Query( query.str().c_str() );

		if (result1)
		{
                    int cnt = 0;
                    fields = result1->Fetch();
                    cnt = fields[0].GetUInt32();

                    if ( cnt > 0 )
                    {
		sLog.outString("Current gamemasters: %d",cnt);

              query2 << "SELECT login,gm FROM accounts WHERE gm > 0";
              QueryResult *result = sDatabase.Query( query2.str().c_str() );

                
                int i = 0;
                while ( i != 24)
                {
                printf("=");
                i++;
                }
                printf("\n");
                printf("|    Account    ");
                printf("|  GM  |\n");
                i = 0;
                while ( i != 24)
                {
                printf("=");
                i++;
                }
                printf("\n");

                do {
                    fields = result->Fetch();
                    printf("|%15s|", fields[0].GetString());
                    printf("%6s|\n",fields[1].GetString());
                }while( result->NextRow() );

                i = 0;
                while ( i != 24)
                {
                printf("=");
                i++;
                }
                printf("\n");
		delete result;
		delete result1;
		    }
		}


		else {
			sLog.outString("NO gamemasters");
		     }

		sLog.outString("Syntax is: setgm character number (0 - normal, 3 - gamemaster)");

		}

		else {
		char charcr[10] = { 0 };
		char pass[10] = { 0 };
		int i = 6;
                for ( i = 6; command[i] && command[i] != ' '; i++ )
                charcr[i-6] = command[i];

		std::stringstream query3;
		Field *fields;
		query3 << "SELECT COUNT(*) FROM accounts WHERE login = '" << charcr << "';";
		QueryResult *result3 = sDatabase.Query( query3.str().c_str() );

		if (result3)
		{
                    int cnt = 0;
                    fields = result3->Fetch();
                    cnt = fields[0].GetUInt32();

                    if ( cnt > 0 )
                    {
			int i = 6;
	                for ( i = 6; command[i] && command[i] != ' '; i++ )
	                charcr[i-6] = command[i];
			pass[0] = command[i+1];
			std::stringstream ss;
			ss << "UPDATE `accounts` SET gm = '" << pass << "' WHERE login = '" << charcr << "'";
			sDatabase.Execute(ss.str().c_str());
			sLog.outString("We added %s gmlevel %s",charcr,pass);
			delete result3;
		    }

		    else {
			sLog.outString("No such account %s",charcr);
			}

		}

		}

	}



	
	else if( strncmp(command,"create",6) == 0)
	{

                if (strncmp(command+7,"",1) == 0)
                {
                sLog.outString("Syntax is: create username password");
                }
		else {
		int i = 7;
                for ( i = 7; command[i] && command[i] != ' '; i++ )
                charcr[i-7] = command[i];
		i++;
		int ii = i;
                for ( ii = i; command[ii] && command[ii] != '\n'; ii++ )
                pass[ii-i] = command[ii];

		
		std::stringstream query;
		Field *fields;
		query << "SELECT COUNT(*) FROM accounts WHERE login = '" << charcr << "';";
		QueryResult *result1 = sDatabase.Query( query.str().c_str() );

		if (result1)
		{
                    int cnt = 0;
                    fields = result1->Fetch();
                    cnt = fields[0].GetUInt32();

                    if ( cnt > 0 )
                    {
			sLog.outString("User %s already exists",charcr);
		    }

		    else {
			std::stringstream acccreate;
			acccreate << "INSERT into `accounts` values('','" << charcr << "','" << pass << "','','','0','','',NOW(),'0')" ;
			sDatabase.Execute(acccreate.str().c_str());
			sLog.outString("User %s with password %s created successfully",charcr,pass);
		    }

		}
	delete result1;
		}
	}


        else {
        sLog.outString( "Unknown command." );
        }

	}

}

#endif
