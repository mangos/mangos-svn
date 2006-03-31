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

#include "Common.h"
#include "Master.h"
#include "Database/DatabaseEnv.h"
#include "ByteBuffer.h"
#include "Log.h"
#include "RASocket.h"
#include "Timer.h"
#include "World.h"

#ifdef ENABLE_RA

#ifndef ENABLE_CLI
#error CLI is required
#endif

SOCKET r;

#define dropclient {Sendf("I'm busy right now, come back later."); \
		SetCloseAndDelete(); \
		return;} 
uint32 iSession=0;
unsigned int iUsers=0;
//bool bSingleUserMode;

typedef int(* pPrintf)(const char*,...);


void ParseCommand(pPrintf zprintf, char*command);

RASocket::RASocket(SocketHandler &h): TcpSocket(h)
{	

	iSess =iSession++ ;
	bLog = sConfig.GetBoolDefault( "RA.Log", 1 );
	bSecure = sConfig.GetBoolDefault( "RA.Secure", 1 );
	iMinLevel = sConfig.GetIntDefault( "RA.MinLevel", 3 );
	//bSingleUserMode = sConfig.GetBoolDefault( "RA.SingleUser", 1 );
	iInputLength=0;
	buff=new char[RA_BUFF_SIZE];
	stage=NONE;
}


RASocket::~RASocket()
{
  delete [] buff;
 
  if(bLog)
  {
	 // std::string ss=GetRemoteAddress();
	  Log("Connection was closed.\n");
  
  }
  if(stage==OK) 
  iUsers--;
  
}

void RASocket::OnAccept()
{
	if(bLog)
	{
		std::string ss=GetRemoteAddress();
		Log("Incoming connection from %s.\n",ss.c_str());
	}
	//if(bSingleUserMode)
	if(iUsers)
	dropclient

	Sendf("%s\x0d\x0a",(char*)sWorld.GetMotd());
}




void RASocket::OnRead()
{
    TcpSocket::OnRead();

	unsigned int sz=ibuf.GetLength ();		
	if(iInputLength+sz>=RA_BUFF_SIZE)
	{
	Log("Input buffer overflow, possible DOS attack\n");
	SetCloseAndDelete();
	return;
	}
	
	//if(bSingleUserMode)
	if(stage!=OK && iUsers)
	dropclient

	char * inp = new char [sz+1];
	ibuf.Read(inp,sz);
	
	if(stage==NONE)
	if(sz>4)//linux remote telnet 
	if(memcmp(inp ,"USER ",5))
	{delete [] inp;return;
	printf("lin bugfix");
	}//linux bugfix

			bool gotenter=false;
			unsigned int y=0;
				for(;y<sz;y++)
				
					if(inp[y]==0xd||inp[y]==0xa)
					{	
						gotenter=true;
						break;
					}

				memcpy(&buff[iInputLength],inp,y);
				iInputLength+=y;
				delete [] inp;	
				if(gotenter)
				{
					
				buff[iInputLength]=0;
				iInputLength=0;
				switch(stage)
				{
					case NONE:
						if(!memcmp(buff,"USER ",5))//got "USER" cmd
						{	
							//if(bLog)Log("User %s",
							strcpy(szLogin,&buff[5]);
							//access db now 
							QueryResult* result = sDatabase.PQuery(
							"SELECT password,gm FROM accounts WHERE login='%s'",szLogin); 
							if(!result)
							{
								Sendf("-No such user.\x0d\x0a");
								if(bLog)Log("User %s does not exist.\n",szLogin);
								if(bSecure)SetCloseAndDelete();
							}
							else 
							{
								Field *fields = result->Fetch(); 
						
								strcpy(szPass,fields[0].GetString());
			
								if(fields[1].GetUInt32()<iMinLevel)
								{	
									Sendf("-Not enough privileges.\x0d\x0a");
									if(bLog)Log("User %s has no privileges.\n",szLogin);
									if(bSecure)SetCloseAndDelete();
								}	else 
								{
									
									stage=LG;
								}
									delete result;
							}
						}
						break;
					case LG:

						if(!memcmp(buff,"PASS ",5))//got "PASS" cmd
						{	if(!strcmp(&buff[5],szPass))//login+pass ok
							{
								r=GetSocket();
								stage=OK;
								iUsers++;

								Sendf("+Logged in.\x0d\x0a");
								if(bLog)Log("User %s has logged in.\n",szLogin);
							}
							else 
							{
								Sendf("-Wrong pass.\x0d\x0a");
								if(bLog)Log("User %s has failed to log in.\n",szLogin);
								if(bSecure)SetCloseAndDelete();
							} 
						}

						break;
					
					
					case OK:
						if(strlen(buff))
						{
						if(bLog)Log("Got '%s' cmd.\n",buff);
						ParseCommand(  &RASocket::zprintf , buff);	
						}
						break;
				};

			
			}	
		
  
	
}


int RASocket::zprintf( const char * szText, ... )
{
	if( !szText ) return 0;
	va_list ap;
	va_start(ap, szText);
	char*megabuffer=new char[1024];
	unsigned int sz=vsprintf(megabuffer,szText,ap);
	#ifdef RA_CRYPT
	Encrypt(megabuffer,sz);
	#endif

	send(r,megabuffer,sz,0);
	delete [] megabuffer;
	va_end(ap);
	return 0;
}

		
void RASocket:: Log( const char * szText, ... )
{
	if( !szText ) return;
	va_list ap;
	va_start(ap, szText);
	time_t t = time(NULL);
	struct tm *tp = localtime(&t);
	FILE *pFile=fopen("RA.log","at");
	fprintf(pFile,"[%d-%02d-%02d %02d:%02d:%02d] [%d] ",
			tp -> tm_year + 1900,
			tp -> tm_mon + 1,
			tp -> tm_mday,
			tp -> tm_hour,tp -> tm_min,tp -> tm_sec,iSess);
	vfprintf( pFile, szText, ap );
	fclose(pFile);
	va_end(ap);
}

#endif
