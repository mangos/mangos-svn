/*
 **	File ......... MinderSocket.cpp 
 **	Published ....  2004-04-17
 **	Author ....... grymse@alhem.net
**/
/*
Copyright (C) 2004  Anders Hedstrom

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
#ifdef HAVE_OPENSSL
#include "MinderHandler.h"
#include "Utility.h"
#include "MinderSocket.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)  
#endif



#ifdef _WIN32
char MinderSocket::g_UpdateHost[256];
port_t MinderSocket::g_UpdatePort = 0;
char MinderSocket::g_UpdateUrl[256];
#endif


MinderSocket::MinderSocket(SocketHandler& h,const std::string& app)
:TcpSocket(h)
,m_app(app)
,my_ip(0)
,my_port(0)
,m_function("")
,m_extra_info(0)
{
	SetLineProtocol();
}


MinderSocket::~MinderSocket()
{
}


void MinderSocket::OnLine(const std::string& line)
{
	std::string cmd;
	std::string id;
	std::string ipstr;
	port_t port;
	ipaddr_t ip;
	int max = GetMaxConnections(); //atoi(config["max_connections"].c_str());
	Parse pa(Utility::base64d(line),"_:");

	pa.getword(cmd);
	static_cast<MinderHandler&>(Handler()).SetMinderTime(time(NULL));

	if (cmd == "You")
	{
		pa.getword(id);
		pa.getword(ipstr);
		port = (port_t)pa.getvalue();
		unsigned long hostid = pa.getvalue();

		Utility::u2ip(ipstr,ip);

DEB(		printf(" received my id '%s' %s:%d - %lu\n",id.c_str(),ipstr.c_str(),port,hostid);)
		// this is ourselves
		my_ip = ip;
		my_port = port;
		static_cast<MinderHandler&>(Handler()).SetMyIpPort(my_ip, my_port);
DEB(		printf("ignoring %s:%d\n",ipstr.c_str(),port);)
		static_cast<MinderHandler&>(Handler()).SetExternalAddress(ipstr);
		if (static_cast<MinderHandler&>(Handler()).GetHostId() == 0)
		{
			static_cast<MinderHandler&>(Handler()).SetHostId(hostid);
		}
	}
	else
#ifdef _WIN32
	if (cmd == "Update")
	{
		std::string host = pa.getword();
		port_t port = (port_t)pa.getvalue();
		std::string url = pa.getword();
		strncpy(g_UpdateHost, host.c_str(), 255);
		g_UpdatePort = port;
		strncpy(g_UpdateUrl, url.c_str(), 255);
	}
	else
#endif
	if (cmd == "Minion")
	{
		pa.getword(id);
		pa.getword(ipstr);
		port = (port_t)pa.getvalue();
		long remote_host_id = pa.getvalue();

		Utility::u2ip(ipstr,ip);
		
		if (ip == my_ip && port == my_port)
		{
			return;
		}
		max = (max == 0) ? 4 : max;

		if (!static_cast<MinderHandler&>(Handler()).FindMinion(id) )
		{
			if (0 && static_cast<MinderHandler&>(Handler()).Count() < max)
			{
DEB(				printf(" connect to %s:%d\n",ipstr.c_str(),port);)
//printf("Minder List: %s:%d id %s\n",ipstr.c_str(),port,id.c_str());
				MinionSocket *tmp = CreateMinionSocket(id,ip,port); //new MinionSocket(Handler(),id,ip,port);
				tmp -> SetMyIpPort(my_ip,my_port);
				if (tmp -> Open(ip,port))
				{
					tmp -> SetDeleteByHandler(true);
					Handler().Add(tmp);
					//
					tmp -> SendHello("Hello");
				}
				else
				if (tmp -> Connecting())
				{
					tmp -> SetDeleteByHandler(true);
					// check OnConnect
					Handler().Add(tmp);
				}
				else
				{
					delete tmp;
				}
			}
			else
			{
				static_cast<MinderHandler&>(Handler()).AddHost(ip,port,id,remote_host_id);
			}
		}
		else
		{
DEB(			printf(" id found\n");)
		}
	}
	else
	if (cmd == "End")
	{
		SetCloseAndDelete( true );
	}
}


void MinderSocket::SendHello()
{
	std::string msg = m_function + "_";
//	assert(m_app.size());
	if (!m_app.size())
	{
		SetCloseAndDelete();
		return;
	}
	msg += Utility::base64(m_app);
	msg += ":" + static_cast<MinderHandler&>(Handler()).GetID();
	msg += ":" + local_ip;
	msg += ":" + Utility::l2string(local_port);
	msg += ":" + Utility::l2string(static_cast<MinderHandler&>(Handler()).GetHostId());
	msg += ":" + Utility::l2string(m_extra_info);
	Send( Utility::base64(msg) + "\n" );
}


void MinderSocket::OnConnect()
{
	SendHello();
}


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
