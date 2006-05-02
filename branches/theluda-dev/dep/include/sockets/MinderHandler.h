/*
 **	File ......... MinderHandler.h
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
#ifndef _MINION_MINDERHANDLER_H
#define _MINION_MINDERHANDLER_H
#ifdef HAVE_OPENSSL

//#include <string>
//#include <time.h>
#include "SocketHandler.h"
#include "MinionSocket.h" // for SendMessage ( to other minions )
//#include "Utility.h"
#include "Base64.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class MinderSocket;

/** \defgroup minder Distributed network framework */
/** Specialized sockethandler for a distributed network. 
	\ingroup minder */
class MinderHandler : public SocketHandler
{
	/** Structure describing a message that has been received by a host. 
		\ingroup minder */
	struct SEEN {
		SEEN(unsigned long l1,unsigned long l2) : m_hid(l1),m_mid(l2),m_t(time(NULL)) {}
		bool Eq(unsigned long l1,unsigned long l2) {
			return l1 == m_hid && l2 == m_mid;
		}
		unsigned long m_hid; // host id
		unsigned long m_mid; // message id
		bool m_temp;
		time_t m_t;
	};
	typedef std::vector<SEEN *> seen_v;
	/** \ingroup minder */
	struct STORE {
		STORE(long x,long y,const std::string& msg) : m_hid(x),m_mid(y),m_message(msg),m_t(time(NULL)) {}
		long m_hid;
		long m_mid;
		std::string m_message;
		time_t m_t;
	};
	typedef std::vector<STORE *> store_v;
	/** Minion hosts list.
		\ingroup minder */
	struct HOSTS {
		HOSTS(ipaddr_t a,port_t p,const std::string& k,long r) : ip(a),port(p),key(k),remote_host_id(r) {}
		ipaddr_t ip;
		port_t port;
		std::string key;
		long remote_host_id;
	};
	typedef std::vector<HOSTS *> hosts_v;
public:
	MinderHandler();
	~MinderHandler();

	void GenerateID();
	const std::string& GetID();

	void SetExternalAddress(const std::string& str) { m_external_ip = str; }
	void SetLocalPort(port_t s) { m_local_port = s; }
	port_t GetLocalPort() { return m_local_port; }

	void SetHostId(unsigned long id) { m_host_id = id; }
	unsigned long GetHostId() { return m_host_id; }

	void SendMessage(const std::string&,short ttl = 30);
	void SendMessage(const std::string&,const std::string&,short,const std::string&,std::list<std::string>&,ulong_v&);

	bool Seen(unsigned long,unsigned long,bool = false);

	void SendConnectList();

	void KeepAlive();

	// socket presence
	bool MinderSockets();
	bool FindMinion(const std::string& );
	int Count();

	void Store(long,long,const std::string& );
	bool StoreGet(long,long,std::string& );

	void SetDebug(bool x = true) { m_bDebug = x; }
	bool Debug() { return m_bDebug; }

	void AddHost(ipaddr_t,port_t,const std::string&,long);
	bool GetHost(ipaddr_t&,port_t&,std::string&,long&);
	void SetMyIpPort(ipaddr_t a,port_t p) { my_ip = a; my_port = p; }
	void GetMyIpPort(ipaddr_t& a,port_t& p) { a = my_ip; p = my_port; }

	void SendTop(const std::string& );
	void Tops(FILE *);

	time_t GetMinderTime() { return m_tMinder; }
	void SetMinderTime(time_t x) { m_tMinder = x; }

	virtual std::string GetVersion() = 0; //{ return ""; }
	virtual unsigned char *GetKey_m2minion() = 0;

protected:
	Base64 m_b;
	
private:
	MinderHandler(const MinderHandler& ) {}
	MinderHandler& operator=(const MinderHandler& ) { return *this; }
	std::string m_id;
	std::string m_external_ip;
	unsigned long m_message_id;
	port_t m_local_port;
	unsigned long m_host_id;
	seen_v m_seen;
	store_v m_store;
	bool m_bDebug;
	hosts_v m_hosts;
	ipaddr_t my_ip;
	port_t my_port;
	time_t m_tMinder;
};



#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
#endif // _MINION_MINDERHANDLER_H
