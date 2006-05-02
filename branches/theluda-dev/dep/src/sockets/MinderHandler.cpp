/*
 **	File ......... MinderHandler.cpp
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

#ifdef _WIN32
#define random rand
#define srandom srand
#endif
#include "Uid.h"
#include "MinderSocket.h"
#include "Utility.h"
#include "MinderHandler.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


#ifdef _DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif




MinderHandler::MinderHandler()
:SocketHandler()
,m_id("")
,m_external_ip("")
,m_message_id(0)
,m_host_id(0)
,m_bDebug(false)
,m_tMinder(0)
{
	GenerateID();
}


MinderHandler::~MinderHandler()
{
	for (seen_v::iterator it = m_seen.begin(); it != m_seen.end(); it++)
	{
		SEEN *p = *it;
		delete p;
	}
	{
		for (store_v::iterator it = m_store.begin(); it != m_store.end(); it++)
		{
			STORE *p = *it;
			delete p;
		}
	}
	{
		for (hosts_v::iterator it = m_hosts.begin(); it != m_hosts.end(); it++)
		{
			HOSTS *p = *it;
			delete p;
		}
	}
}


void MinderHandler::GenerateID()
{
	Uid t;
	m_id = t.GetUid();
}


const std::string& MinderHandler::GetID()
{
	return m_id;
}


void MinderHandler::SendMessage(const std::string& msg_in,short ttl)
{
	std::string msg;
	long message_id = m_message_id++;

	msg = "Message_" + Utility::l2string(m_host_id);
	msg += ":" + Utility::l2string(message_id);
	msg += ":" + Utility::l2string(ttl);
	msg += ":" + msg_in;

DEB(	printf("Message:\n%s\n",msg.c_str());)

	if (msg.size() > 255) // try is good even here, because of possible bandwidth differences between nodes
	{
		Store(m_host_id, message_id, msg);
		msg = "Try_" + Utility::l2string(m_host_id) + ":" + Utility::l2string(message_id);
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			MinionSocket *p = dynamic_cast<MinionSocket *>((*it).second);
			if (p && p -> Ready() )
			{
				Uid ruid(p -> GetRemoteId());
				memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
				p -> Send( p -> encrypt(GetKey_m2minion(),msg) + "\n" );
			}
		}
	}
	else
	{
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			MinionSocket *p = dynamic_cast<MinionSocket *>((*it).second);
			if (p && p -> Ready() )
			{
				Uid ruid(p -> GetRemoteId());
				memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
//				p -> Send( p -> Utility::base64(msg) + "\n" );
				p -> Send( p -> encrypt(GetKey_m2minion(),msg) + "\n" );
			}
		}
	}
}


void MinderHandler::SendMessage(const std::string& hid,const std::string& mid,short ttl,const std::string& msg_in,std::list<std::string>& clist,ulong_v& hosts)
{
	std::string msg;
	long host_id = atol(hid.c_str());
	long message_id = atol(mid.c_str());

	msg = "Message_" + hid;
	msg += ":" + mid;
	msg += ":" + Utility::l2string(ttl); // + ":" + ttlstr;
	msg += ":" + msg_in;

	if (msg.size() > 255)
	{
		Store(host_id, message_id, msg);
		msg = "Try_" + Utility::l2string(host_id) + ":" + Utility::l2string(message_id);
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			MinionSocket *p = dynamic_cast<MinionSocket *>((*it).second);
			if (p && p -> Ready() )
			{
				bool ok = true;
				for (std::list<std::string>::iterator it = clist.begin(); it != clist.end() && ok; it++)
				{
					std::string id = *it;
					if (!strcmp(p -> GetRemoteId().c_str(),id.c_str()))
					{
						ok = false;
					}
				}
				for (ulong_v::iterator i2 = hosts.begin(); i2 != hosts.end() && ok; i2++)
				{
					if (*i2 == static_cast<unsigned long>(p -> GetRemoteHostId()))
					{
						ok = false;
					}
				}
				if (ok)
				{
					Uid ruid(p -> GetRemoteId());
					memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
					p -> Send( p -> encrypt(GetKey_m2minion(), msg) + "\n" );
				}
			}
		}
	}
	else
	{
		for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			MinionSocket *p = dynamic_cast<MinionSocket *>((*it).second);
			if (p && p -> Ready() )
			{
				bool ok = true;
				for (std::list<std::string>::iterator it = clist.begin(); it != clist.end(); it++)
				{
					std::string id = *it;
					if (!strcmp(p -> GetRemoteId().c_str(),id.c_str()))
					{
						ok = false;
						break;
					}
				}
				if (ok)
				{
					Uid ruid(p -> GetRemoteId());
					memcpy(GetKey_m2minion() + 8,ruid.GetBuf(),16);
//					p -> Send( p -> Utility::base64(msg) + "\n" );
					p -> Send( p -> encrypt(GetKey_m2minion(), msg) + "\n" );
				}
			}
		}
	}
}


void MinderHandler::KeepAlive()
{
	std::string msg;
	std::string tmp;
	msg = "KeepAlive";
	m_b.encode(msg, tmp, false);
	SendMessage(tmp);
}


void MinderHandler::SendConnectList()
{
	std::string msg;
	std::string tmp = "_";
	msg = "ConnectList";
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*it).second);
		if (p && p -> Ready() )
		{
			msg += tmp + p -> GetRemoteId();
			tmp = ":";
		}
	}
	m_b.encode(msg, tmp, false);
//printf("ConnectList:\n%s\n",msg.c_str());
	SendMessage(tmp, 0);
}


bool MinderHandler::MinderSockets()
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinderSocket *p = dynamic_cast<MinderSocket *>((*it).second);
		if (p)
			return true;
	}
	return false;
}


bool MinderHandler::FindMinion(const std::string& id)
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*it).second);
		if (p && p -> GetRemoteId() == id) //!strcasecmp(p -> GetRemoteId().c_str(),id.c_str()))
		{
			return true;
		}
	}
	for (socket_m::iterator i2 = m_add.begin(); i2 != m_add.end(); i2++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*i2).second);
		if (p && p -> GetRemoteId() == id) //!strcasecmp(p -> GetRemoteId().c_str(),id.c_str()))
		{
			return true;
		}
	}
	return false;
}


int MinderHandler::Count()
{
	int q = 0;
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*it).second);
		if (p)
		{
			q++;
		}
	}
	for (socket_m::iterator i2 = m_add.begin(); i2 != m_add.end(); i2++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*i2).second);
		if (p)
		{
			q++;
		}
	}
	return q;
}


bool MinderHandler::Seen(unsigned long host_id,unsigned long message_id,bool temp)
{
	if (host_id == m_host_id)
		return true;
	for (seen_v::iterator it = m_seen.begin(); it != m_seen.end(); it++)
	{
		SEEN *p = *it;
		if (p -> Eq(host_id,message_id))
		{
			if (p -> m_temp && !temp)
			{
				p -> m_temp = temp;
				return false;
			}
			return true;
		}
	}
	// %! while time - erase
	time_t t = time(NULL);
	while (m_seen.size())
	{
		SEEN *p = m_seen[0];
		if (t - p -> m_t > 120)
		{
			delete p;
			m_seen.erase(m_seen.begin());
		}
		else
		{
			break;
		}
	}
	SEEN *p = new SEEN(host_id,message_id);
	p -> m_temp = temp;
	m_seen.push_back(p);
	return false;
}


void MinderHandler::Store(long hid,long mid,const std::string& msg)
{
	// %! while time - erase
	time_t t = time(NULL);
	while (m_store.size())
	{
		STORE *p = m_store[0];
		if (t - p -> m_t > 10)
		{
			delete p;
			m_store.erase(m_store.begin());
		}
		else
		{
			break;
		}
	}
	STORE *p = new STORE(hid,mid,msg);
	m_store.push_back(p);
}


bool MinderHandler::StoreGet(long hid,long mid,std::string& msg)
{
	for (store_v::iterator it = m_store.begin(); it != m_store.end(); it++)
	{
		STORE *p = *it;
		if (p -> m_hid == hid && p -> m_mid == mid)
		{
			msg = p -> m_message;
			return true;
		}
	}
	return false;
}


void MinderHandler::AddHost(ipaddr_t a,port_t p,const std::string& k,long r)
{
DEB(	printf("AddHost: size %d\n",m_hosts.size());)
	if (m_hosts.size() >= 20)
		return;
	for (hosts_v::iterator it = m_hosts.begin(); it != m_hosts.end(); it++)
	{
		HOSTS *p2 = *it;
		if (p2 -> ip == a && p2 -> port == p)
		{
			p2 -> key == k;
			return;
		}
	}
	HOSTS *p2 = new HOSTS(a,p,k,r);
	m_hosts.push_back(p2);
}


bool MinderHandler::GetHost(ipaddr_t& a,port_t& p,std::string& k,long& r)
{
	if (m_hosts.size())
	{
		hosts_v::iterator it = m_hosts.begin();
		HOSTS *p2 = *it;
		a = p2 -> ip;
		p = p2 -> port;
		k = p2 -> key;
		r = p2 -> remote_host_id;
		delete p2;
		m_hosts.erase(it);
		return true;
	}
	return false;
}


void MinderHandler::SendTop(const std::string& hid)
{
	std::string msg;
	std::string tmp;
	msg = "Top_" + hid;
	msg += ":" + Utility::l2string(GetHostId());
	m_b.encode(GetVersion(), tmp, false);
	msg += ":" + tmp;
#ifdef _WIN32
	msg += ":Win32";
#else
	msg += ":Linux";
#endif
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*it).second);
//		TelnetSocket *p2 = dynamic_cast<TelnetSocket *>((*it).second);
		if (p && p -> Ready() )
		{
			msg += ":" + Utility::l2string(p -> GetRemoteHostId() );
		}
	}
//printf(" Top reply: %s\n",msg.c_str());
	m_b.encode(msg, tmp, false);
	SendMessage(tmp);
}


void MinderHandler::Tops(FILE *fil)
{
	for (socket_m::iterator it = m_sockets.begin(); it != m_sockets.end(); it++)
	{
		MinionSocket *p = dynamic_cast<MinionSocket*>((*it).second);
		if (p && p -> Ready() )
		{
			fprintf(fil,"\t\"%ld\" -> \"%ld\"\n",
				GetHostId(),
				p -> GetRemoteHostId());
		}
	}
}




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
