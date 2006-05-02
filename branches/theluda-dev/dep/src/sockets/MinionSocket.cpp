/*
 **	File ......... MinionSocket.cpp
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

#include <vector>

#include "MinderHandler.h"
#include "Uid.h"
#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x) 
#endif



// statics
//MinionSocket::connect_v MinionSocket::m_connect;


MinionSocket::MinionSocket(SocketHandler& h) : CTcpSocket(h)
,m_remote_id("")
,m_ip(0)
,m_port(0)
,m_bIDVerified(false)
,m_bStopMessage(false)
,m_messagecount(0)
,m_seencount(0)
,m_remote_host_id(0)
{
	SetLineProtocol();
}


MinionSocket::MinionSocket(SocketHandler& h,const std::string& id,ipaddr_t l,port_t s)
:CTcpSocket(h)
,m_remote_id(id)
,m_ip(l)
,m_port(s)
,m_bIDVerified(true)
,m_bStopMessage(false)
,m_messagecount(0)
,m_seencount(0)
,m_remote_host_id(0)
{
	SetLineProtocol();

	std::string str;
	Utility::l2ip(l,str);
DEB(	printf(" new connect: %s:%d\n",str.c_str(),s);)
}


MinionSocket::~MinionSocket()
{
}


/*
ICrypt *MinionSocket::AllocateCrypt()
{
	return new HCrypt;
}
*/


void MinionSocket::OnDelete()
{
	// %! avregistrera i connection-lista
//	Remove(m_remote_id,m_ip,m_port);
	SendConnectList();
}


void MinionSocket::OnConnect()
{
//	printf("Connected to: %s\n",GetRemoteAddress().c_str());
	SendHello("Hello");
}


void MinionSocket::SendHello(const std::string& cmd)
{
	std::string str;
	Utility::l2ip(my_ip,str);
	char msg[200];
	sprintf(msg,"%s_%s:%s:%d:%s:%ld\n",cmd.c_str(),m_remote_id.c_str(),str.c_str(),my_port,
		static_cast<MinderHandler&>(Handler()).GetID().c_str(),
		static_cast<MinderHandler&>(Handler()).GetHostId() );
	{
		Uid ruid(m_remote_id);
//printf("encrypt with remote id '%s'\n",m_remote_id.c_str());
		memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
//		Send( Utility::base64(msg) + "\n" );
		Send( encrypt(GetKey_m2minion(), msg) + "\n" );
	}
}


void MinionSocket::SendConnectList()
{
	static_cast<MinderHandler&>(Handler()).SendConnectList();
}


void MinionSocket::OnLine(const std::string& line)
{
	std::string line_decrypt;
	std::string cmd;
	Uid myid(static_cast<MinderHandler&>(Handler()).GetID());

	memcpy(GetKey_m2minion() + 8,myid.GetBuf(),16);
	if (!decrypt(GetKey_m2minion(),line,line_decrypt))
	{
		SetCloseAndDelete();
		return;
	}
//	Parse pa(decrypt(GetKey_m2minion(),line),"_:");
	Parse pa(line_decrypt, "_:");

	m_messagecount++;
	pa.getword(cmd);
	if (!m_bIDVerified)
	{
		if (cmd == "Hello")
		{
			std::string id; // supposed to be my id
			std::string ipstr; // remote ip
			std::string remote_id;
			pa.getword(id);
			pa.getword(ipstr);
			port_t port = (port_t)pa.getvalue(); // remote listen port
			pa.getword(remote_id);
			long remote_host_id = pa.getvalue();
			ipaddr_t ip;
			int max = GetMaxConnections(); //atoi(config["max_connections"].c_str());
			max = (max == 0) ? 4 : max;

			Utility::u2ip(ipstr, ip);

			if (id == static_cast<MinderHandler&>(Handler()).GetID() &&
				!static_cast<MinderHandler&>(Handler()).FindMinion(remote_id) &&
				static_cast<MinderHandler&>(Handler()).Count() < max * 2 + 1)
			{
DEB(				printf("My ID verified!\n");)
				m_remote_id = remote_id;
				m_ip = ip; // remote ip
				m_port = port; // remote listen port
				m_bIDVerified = true;
				m_remote_host_id = remote_host_id;
				SendHello("Hi");
			}
			else
			{
				{
					Uid ruid(remote_id);
					memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
					Send( encrypt(GetKey_m2minion(), "Bye") + "\n" );
				}
				SetCloseAndDelete(true);
			}
		}
		else
		{
DEB(			fprintf(stderr,"Ignoring message\n");)
		}
	}
	else
	{
		if (!OnVerifiedLine(cmd, pa))
		{
			// %! notify?
			SetCloseAndDelete();
		}
	}
}

bool MinionSocket::OnVerifiedLine(const std::string& cmd,Parse& pa)
{
	std::string str;

	if (static_cast<MinderHandler&>(Handler()).Debug() )
	{
		str = "&l&fCommand: " + cmd + "&n\n";
		Notify( str );
	}

DEB(		printf("Incoming command: '%s'\n",cmd.c_str());)
	if (cmd == "Tip")
	{
		// Tip _ host id
		std::string hid = pa.getword();
//printf("Tip from: %s\n",hid.c_str());
		static_cast<MinderHandler&>(Handler()).SendTop( hid );
	}
	else
	if (cmd == "Top")
	{
		unsigned long hid = pa.getvalue();
//printf("Top to: %ld\n",hid);
		if (hid == static_cast<MinderHandler&>(Handler()).GetHostId())
		{
			std::string src = pa.getword();
			std::string vstr = Utility::base64d(pa.getword());
			std::string os = pa.getword();
//printf(" (Top from: %s)\n",src.c_str());
			std::string dst;
			if (isdigit(os[0]))
			{
				dst = os;
				os = "";
			}
			else
			{
				dst = pa.getword();
			}
			FILE *fil = fopen("top_map.dot","at");
			if (!fil)
				fil = fopen("top_map.dot","wt");
			if (fil)
			{
				fprintf(fil,"\t\"%s\" [label=\"%s\\n%s\\n%s\"]\n",
					src.c_str(),
					src.c_str(),
					vstr.c_str(),
					os.c_str());
				while (dst.size())
				{
					fprintf(fil,"\t\"%s\" -> \"%s\"\n",
						src.c_str(),
						dst.c_str());
					//
					pa.getword(dst);
				}
				fclose(fil);
			}
		}
		else
		{
//printf("Top ignored: %ld != %ld\n",hid,static_cast<MinderHandler&>(Handler()).GetHostId());
		}
	}
	else
	if (cmd == "Accept")
	{
		long hid = pa.getvalue();
		long mid = pa.getvalue();
		std::string msg;
		if (static_cast<MinderHandler&>(Handler()).StoreGet(hid,mid,msg))
		{
			Uid ruid(m_remote_id);
			memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
			Send( encrypt(GetKey_m2minion(), msg) + "\n" );
		}
	}
	else
	if (cmd == "Try")
	{
		long hid = pa.getvalue();
		long mid = pa.getvalue();
		if (!static_cast<MinderHandler&>(Handler()).Seen(hid,mid,true))
		{
			std::string msg;
			msg = "Accept_" + Utility::l2string(hid);
			msg += ":" + Utility::l2string(mid);
			Uid ruid(m_remote_id);
			memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
			Send( encrypt(GetKey_m2minion(), msg) + "\n" );
		}
		else
		{
			m_seencount++;
		}
	}
	else
	if (cmd == "KeepAlive")
	{
		// good for you
		m_bStopMessage = true;
	}
	else
	if (cmd == "ConnectList")
	{
		std::string id;
		while (m_clist.size())
		{
			std::list<std::string>::iterator it = m_clist.begin();
			m_clist.erase(it);
		}
		pa.getword(id);
		while (id.size())
		{
			m_clist.push_back(id);
			//
			pa.getword(id);
		}
	}
	else
	if (cmd == "Message")
	{
		std::string hid = pa.getword();
		std::string mid = pa.getword();
		int ttl = pa.getvalue();
		std::string msg_in = pa.getword();
		unsigned long host_id = atol(hid.c_str());
		unsigned long message_id = atol(mid.c_str());
		ulong_v hosts;

		if (!static_cast<MinderHandler&>(Handler()).Seen(host_id,message_id))
		{
DEB(				printf("Message\n hostid: %ld\n messageid: %ld\n ttl: %d\n message:\n%s\n--end of message\n", host_id, message_id, ttl, msg_in.c_str());)
DEB(				printf("Message:\n%s\n--End of Message\n",Utility::base64d(msg_in).c_str());)
			if (static_cast<MinderHandler&>(Handler()).Debug() )
			{
				str = "&l&fMessage: Not Seen&n\n";
				Notify( str );
			}
			std::string msg = Utility::base64d(msg_in);
			m_bStopMessage = false;
			{
				Uid myid(static_cast<MinderHandler&>(Handler()).GetID());
				memcpy(GetKey_m2minion() + 8,myid.GetBuf(),16);
				OnLine( encrypt(GetKey_m2minion(), msg ) );
			}
			if (!m_bStopMessage && ttl--)
			{
				while (host_id > 0)
				{
					hosts.push_back(host_id);
					//
					host_id = pa.getvalue();
				}
//				Parse pa(msg,"_");
//				std::string cmd = pa.getword();
//				if (cmd == "Pong")
				// always add host id to message before forwarding
				{
					msg += ":" + Utility::l2string(static_cast<MinderHandler&>(Handler()).GetHostId());
					msg_in = Utility::base64(msg);
				}
				static_cast<MinderHandler&>(Handler()).SendMessage(hid, mid, ttl, msg_in, m_clist, hosts);
			}
		}
		else
		{
			m_seencount++;
			if (static_cast<MinderHandler&>(Handler()).Debug() )
			{
				str = "&l&fMessage: Seen&n\n";
				Notify( str );
			}
		}
	}
	else
	if (cmd == "Hi")
	{
		SendConnectList();
	}
	else
	if (cmd == "Bye")
	{
		SetCloseAndDelete(true);
	}
	else
	{
		return false;
	}
	return true;
}


void MinionSocket::OnAccept()
{
DEB(	printf("Incoming connection from: %s\n",GetRemoteAddress().c_str());)
}


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
