/*
 **	File ......... MinionSocket.h
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
#ifndef _MINIONSOCKET_H
#define _MINIONSOCKET_H
#ifdef HAVE_OPENSSL

//#include <string>
//#include <vector>
#include "CTcpSocket.h"
#include "Parse.h"
//#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


class ICrypt;



	typedef std::vector<unsigned long> ulong_v;

/** Encrypted peer2peer socket in the distributed network. 
	\ingroup minder */
class MinionSocket : public CTcpSocket
{
public:
	MinionSocket(SocketHandler& );
	MinionSocket(SocketHandler&,const std::string&,ipaddr_t,port_t);
	~MinionSocket();

//	ICrypt *AllocateCrypt();

	void SendConnectList();

	void SetMyIpPort(ipaddr_t l,port_t s) { my_ip = l; my_port = s; }

	void SendHello(const std::string&);
	ipaddr_t GetIP() { return m_ip; }
	port_t GetPort() { return m_port; }

	void OnAccept();

	const std::string& GetRemoteId() { return m_remote_id; }
	virtual bool OnVerifiedLine(const std::string& cmd,Parse& pa);
	void StopMessage(bool x = true) { m_bStopMessage = x; }

	long GetMessageCount() { return m_messagecount; }
	long GetSeenCount() { return m_seencount; }
	virtual void Notify(const std::string& ) {}

	void SetRemoteHostId(long x) { m_remote_host_id = x; }
	long GetRemoteHostId() { return m_remote_host_id; }

	virtual int GetMaxConnections() = 0;
	virtual unsigned char *GetKey_m2minion() = 0;

	ipaddr_t GetMyIP() { return my_ip; }
	port_t GetMyPort() { return my_port; }
	void SetRemoteId(const std::string& x) { m_remote_id = x; }
	void SetIP(ipaddr_t x) { m_ip = x; }
	void SetPort(port_t x) { m_port = x; }
	void SetIDVerified(bool x = true) { m_bIDVerified = x; }

protected:
	void OnDelete();
	void OnConnect();
	void OnLine(const std::string& line);

private:
	MinionSocket& operator=(const MinionSocket& ) { return *this; }
	std::string 	m_remote_id;
	ipaddr_t 	m_ip;
	port_t 		m_port;
	ipaddr_t 	my_ip;
	port_t 		my_port;
	bool 		m_bIDVerified;
	std::list<std::string> 	m_clist;
	bool 		m_bStopMessage;
	long		m_messagecount;
	long		m_seencount;
	long		m_remote_host_id;
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // HAVE_OPENSSL
#endif // _MINIONSOCKET_H
