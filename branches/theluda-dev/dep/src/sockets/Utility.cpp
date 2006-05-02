/** \file Utility.cpp
 **	\date  2004-02-13
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

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
#include "Utility.h"
#include "Parse.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


// statics
std::string Utility::m_host;
bool Utility::m_local_resolved = false;
ipaddr_t Utility::m_ip = 0;
std::string Utility::m_addr;
#ifdef IPPROTO_IPV6
struct in6_addr Utility::m_local_ip6;
#endif
std::string Utility::m_local_addr6;


std::string Utility::base64(const std::string& str_in)
{
	std::string str;
	Base64 m_b;
	m_b.encode(str_in, str, false); // , false == do not add cr/lf
	return str;
}


std::string Utility::base64d(const std::string& str_in)
{
	std::string str;
	Base64 m_b;
	m_b.decode(str_in, str);
	return str;
}


std::string Utility::l2string(long l)
{
	std::string str;
	char tmp[100];
	sprintf(tmp,"%ld",l);
	str = tmp;
	return str;
}


std::string Utility::bigint2string(uint64_t l)
{
	std::string str;
	uint64_t tmp = l;
	while (tmp)
	{
		uint64_t a = tmp % 10;
		str = (char)(a + 48) + str;
		tmp /= 10;
	}
	if (!str.size())
	{
		str = "0";
	}
	return str;
}


uint64_t Utility::atoi64(const std::string& str) 
{
	uint64_t l = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		l = l * 10 + str[i] - 48;
	}
	return l;
}


unsigned int Utility::hex2unsigned(const std::string& str)
{
	unsigned int r = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		r = r * 16 + str[i] - 48 - ((str[i] >= 'A') ? 7 : 0) - ((str[i] >= 'a') ? 32 : 0);
	}
	return r;
}


/*
* Encode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_encode(const std::string& src)
{
static	char hex[] = "0123456789ABCDEF";
	std::string dst;
	for (size_t i = 0; i < src.size(); i++)
	{
		if (isalnum(src[i]))
		{
			dst += src[i];
		}
		else
		if (src[i] == ' ')
		{
			dst += '+';
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
} // rfc1738_encode


/*
* Decode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_decode(const std::string& src)
{
	std::string dst;
	for (size_t i = 0; i < src.size(); i++)
	{
		if (src[i] == '%' && isxdigit(src[i + 1]) && isxdigit(src[i + 2]))
		{
			char c1 = src[++i];
			char c2 = src[++i];
			c1 = c1 - 48 - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
			c2 = c2 - 48 - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);
			dst += (char)(c1 * 16 + c2);
		}
		else
		if (src[i] == '+')
		{
			dst += ' ';
		}
		else
		{
			dst += src[i];
		}
	}
	return dst;
} // rfc1738_decode


bool Utility::isipv4(const std::string& str)
{
	for (size_t i = 0; i < str.size(); i++)
		if (!isdigit(str[i]) && str[i] != '.')
			return false;
	return true;
}


bool Utility::isipv6(const std::string& str)
{
	size_t qc = 0;
	size_t qd = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		qc += (str[i] == ':') ? 1 : 0;
		qd += (str[i] == '.') ? 1 : 0;
	}
	if (qc > 7)
	{
		return false;
	}
	if (qd && qd != 3)
	{
		return false;
	}
	Parse pa(str,":.");
	std::string tmp = pa.getword();
	while (tmp.size())
	{
		if (tmp.size() > 4)
		{
			return false;
		}
		for (size_t i = 0; i < tmp.size(); i++)
		{
			if (tmp[i] < '0' || (tmp[i] > '9' && tmp[i] < 'A') ||
				(tmp[i] > 'F' && tmp[i] < 'a') || tmp[i] > 'f')
			{
				return false;
			}
		}
		//
		tmp = pa.getword();
	}
	return true;
}


bool Utility::u2ip(const std::string& str, ipaddr_t& l)
{
	if (isipv4(str))
	{
		Parse pa((char *)str.c_str(), ".");
		union {
			struct {
				unsigned char b1;
				unsigned char b2;
				unsigned char b3;
				unsigned char b4;
			} a;
			ipaddr_t l;
		} u;
		u.a.b1 = static_cast<unsigned char>(pa.getvalue());
		u.a.b2 = static_cast<unsigned char>(pa.getvalue());
		u.a.b3 = static_cast<unsigned char>(pa.getvalue());
		u.a.b4 = static_cast<unsigned char>(pa.getvalue());
		l = u.l;
		return true;
	}
	else
	{
		struct hostent *he = gethostbyname( str.c_str() );
		if (!he)
		{
			return false;
		}
		memcpy(&l, he -> h_addr, 4);
		return true;
	}
	return false;
}


#ifdef IPPROTO_IPV6
bool Utility::u2ip(const std::string& str, struct in6_addr& l)
{
	if (isipv6(str))
	{
		std::list<std::string> vec;
		size_t x = 0;
		for (size_t i = 0; i <= str.size(); i++)
		{
			if (i == str.size() || str[i] == ':')
			{
				std::string s = str.substr(x, i - x);
				//
				if (strstr(s.c_str(),".")) // x.x.x.x
				{
					Parse pa(s,".");
					char slask[100]; // u2ip temporary hex2string conversion
					unsigned long b0 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b1 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b2 = static_cast<unsigned long>(pa.getvalue());
					unsigned long b3 = static_cast<unsigned long>(pa.getvalue());
					sprintf(slask,"%lx",b0 * 256 + b1);
					vec.push_back(slask);
					sprintf(slask,"%lx",b2 * 256 + b3);
					vec.push_back(slask);
				}
				else
				{
					vec.push_back(s);
				}
				//
				x = i + 1;
			}
		}
		size_t sz = vec.size(); // number of byte pairs
		size_t i = 0; // index in in6_addr.in6_u.u6_addr16[] ( 0 .. 7 )
		for (std::list<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
		{
			std::string bytepair = *it;
			if (bytepair.size())
			{
				l.s6_addr16[i++] = htons(Utility::hex2unsigned(bytepair));
			}
			else
			{
				l.s6_addr16[i++] = 0;
				while (sz++ < 8)
				{
					l.s6_addr16[i++] = 0;
				}
			}
		}
		return true;
	}
	else
	{
#ifdef SOLARIS
		int errnum = 0;
		struct hostent *he = getipnodebyname( str.c_str(), AF_INET6, 0, &errnum );
#else
		struct hostent *he = gethostbyname2( str.c_str(), AF_INET6 );
#endif
		if (!he)
		{
			return false;
		}
		memcpy(&l,he -> h_addr_list[0],he -> h_length);
#ifdef SOLARIS
		free(he);
#endif
		return true;
	}
	return false;
}
#endif


void Utility::l2ip(const ipaddr_t ip, std::string& str)
{
	union {
		struct {
			unsigned char b1;
			unsigned char b2;
			unsigned char b3;
			unsigned char b4;
		} a;
		ipaddr_t l;
	} u;
	u.l = ip;
	char tmp[100];
	sprintf(tmp, "%u.%u.%u.%u", u.a.b1, u.a.b2, u.a.b3, u.a.b4);
	str = tmp;
}


#ifdef IPPROTO_IPV6
void Utility::l2ip(const struct in6_addr& ip, std::string& str,bool mixed)
{
	char slask[100]; // l2ip temporary
	*slask = 0;
	unsigned int prev = 0;
	bool skipped = false;
	bool ok_to_skip = true;
	if (mixed)
	{
		unsigned int x;
		for (size_t i = 0; i < 6; i++)
		{
			x = ntohs(ip.s6_addr16[i]);
			if (*slask && (x || !ok_to_skip || prev))
				strcat(slask,":");
			if (x || !ok_to_skip)
			{
				sprintf(slask + strlen(slask),"%X", x);
				if (x && skipped)
					ok_to_skip = false;
			}
			else
			{
				skipped = true;
			}
			prev = x;
		}
		x = ntohs(ip.s6_addr16[6]);
		sprintf(slask + strlen(slask),":%u.%u",x / 256,x & 255);
		x = ntohs(ip.s6_addr16[7]);
		sprintf(slask + strlen(slask),".%u.%u",x / 256,x & 255);
	}
	else
	{
		for (size_t i = 0; i < 8; i++)
		{
			unsigned int x = ntohs(ip.s6_addr16[i]);
			if (*slask && (x || !ok_to_skip || prev))
				strcat(slask,":");
			if (x || !ok_to_skip)
			{
				sprintf(slask + strlen(slask),"%X", x);
				if (x && skipped)
					ok_to_skip = false;
			}
			else
			{
				skipped = true;
			}
			prev = x;
		}
	}
	str = slask;
}
#endif


#ifdef IPPROTO_IPV6
int Utility::in6_addr_compare(in6_addr a,in6_addr b)
{
	for (size_t i = 0; i < 16; i++)
	{
		if (a.s6_addr[i] < b.s6_addr[i])
			return -1;
		if (a.s6_addr[i] > b.s6_addr[i])
			return 1;
	}
	return 0;
}
#endif


void Utility::ResolveLocal()
{
	char h[256];

	// get local hostname and translate into ip-address
	*h = 0;
	gethostname(h,255);
	{
		if (Utility::u2ip(h, m_ip))
		{
			Utility::l2ip(m_ip, m_addr);
		}
	}
#ifdef IPPROTO_IPV6
	memset(&m_local_ip6, 0, sizeof(m_local_ip6));
	{
		if (Utility::u2ip(h, m_local_ip6))
		{
			Utility::l2ip(m_local_ip6, m_local_addr6);
		}
	}
#endif
	m_host = h;
	m_local_resolved = true;
}


const std::string& Utility::GetLocalHostname()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_host;
}


ipaddr_t Utility::GetLocalIP()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_ip;
}


const std::string& Utility::GetLocalAddress()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_addr;
}


#ifdef IPPROTO_IPV6
const struct in6_addr& Utility::GetLocalIP6()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_local_ip6;
}


const std::string& Utility::GetLocalAddress6()
{
	if (!m_local_resolved)
	{
		ResolveLocal();
	}
	return m_local_addr6;
}
#endif


void Utility::SetEnv(const std::string& var,const std::string& value)
{
#if (defined(SOLARIS8) || defined(SOLARIS))
	{
		std::string slask = var + "=" + value;
		putenv( (char *)slask.c_str());
	}
#elif defined _WIN32
	{
		std::string slask = var + "=" + value;
		_putenv( (char *)slask.c_str());
	}
#else
	setenv(var.c_str(), value.c_str(), 1);
#endif
}


#ifdef SOCKETS_NAMESPACE
}
#endif

