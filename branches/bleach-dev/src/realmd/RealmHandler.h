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


#ifndef REALM_HANDLER_H
#define REALM_HANDLER_H

#include "Common.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "Config/ConfigEnv.h" 
#include "SystemConfig.h"
#include "CircularBuffer.h"
#include "Patcher.h"

#include "ace/Svc_Handler.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/SOCK_Stream.h"

#define ChunkSize 2048
#define TCP_BUFSIZE_READ 16400

class RealmHandler;

enum eAuthCmd
{
	//AUTH_NO_CMD                 = 0xFF,
	AUTH_LOGON_CHALLENGE        = 0x00,
	AUTH_LOGON_PROOF            = 0x01,
	AUTH_RECONNECT_CHALLENGE    = 0x02,
	AUTH_RECONNECT_PROOF        = 0x03,
//update srv =4	
	REALM_LIST                  = 0x10,
	XFER_INITIATE               = 0x30,   
	XFER_DATA                   = 0x31,   
	XFER_ACCEPT                 = 0x32,   
	XFER_RESUME                 = 0x33,   
	XFER_CANCEL                 = 0x34    
};
enum eStatus
{
	STATUS_NOTAUTHED = 0,
	STATUS_AUTHED = 1
};

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if __GNUC__ && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

typedef struct
{
    uint8   cmd;
    uint8   error;                                
    uint16  size;                                 
    uint8   gamename[4];                          
    uint8   version1;                             
    uint8   version2;                             
    uint8   version3;                             
    uint16  build;                                
    uint8   platform[4];                          
    uint8   os[4];                                
    uint8   country[4];                           
    uint32  timezone_bias;                        
    uint32  ip;                                   
    uint8   I_len;                                
    uint8   I[1];                                 
} sAuthLogonChallenge_C;

typedef sAuthLogonChallenge_C sAuthReconnectChallenge_C;

typedef struct
{
    uint8   cmd;                                  
    uint8   error;                                
    uint8   unk2;                                 
    uint8   B[32];
    uint8   g_len;                                
    uint8   g[1];
    uint8   N_len;                                
    uint8   N[32];
    uint8   s[32];
    uint8   unk3[16];
} sAuthLogonChallenge_S;

typedef struct
{
    uint8   cmd;                                  
    uint8   A[32];
    uint8   M1[20];
    uint8   crc_hash[20];
    uint8   number_of_keys;
} sAuthLogonProof_C;

typedef struct
{
    uint16  unk1;
    uint32  unk2;
    uint8   unk3[4];
    uint16  unk4[20];                             
}  sAuthLogonProofKey_C;

typedef struct
{
    uint8   cmd;                                  
    uint8   error;
    uint8   M2[20];
    uint32  unk2;
} sAuthLogonProof_S;

typedef struct {
	uint8 cmd;//XFER_INITIATE
	uint8 size;//strlen("Patch");
	uint8 name[5];
	uint64 file_size;
	uint8 md5[MD5_DIGEST_LENGTH];
}XFER_INIT;

typedef struct {
	uint8 opcode;
	uint16 data_size;
	uint8 data[ChunkSize];
}XFER_DATA_STRUCT;


class RealmHandler : public ACE_Svc_Handler <ACE_SOCK_STREAM, ACE_MT_SYNCH>
{

public:
	RealmHandler();
	~RealmHandler ();

	int open (void *acceptor);
	int handle_close (ACE_HANDLE handle,
                    ACE_Reactor_Mask mask);
	
	const static int N_BYTE_SIZE = 32;
	const static int s_BYTE_SIZE = 32;

	int _HandleLogonChallenge();
	int _HandleLogonProof();
	int _HandleRealmList();

	int _HandleXferResume();
	int _HandleXferCancel();
	void _HandleXferAccept();
	FILE *pPatch;

protected:
	int handle_input (ACE_HANDLE handle);

private:
	
	BigNumber N, s, g, v;
	BigNumber b, B;
	BigNumber rs;

	BigNumber K;
	bool _authed;

	std::string _login;

	ACE_Message_Block *mblk;
	
	int read_input (void);
	int terminate_io (ACE_Reactor_Mask mask);
	int initiate_io (ACE_Reactor_Mask mask);
	int check_destroy (void);

	int  flg_mask_;

	ACE_Recursive_Thread_Mutex mutex_;
	CircularBuffer ibuf;
	CircularBuffer obuf;
	struct MES
    {
        MES( const char *buf_in,size_t len_in)
            :buf(new  char[len_in])
            ,len(len_in)
            ,ptr(0)
        {
            memcpy(buf,buf_in,len);
        }
        ~MES() { delete[] buf; }
        size_t left() { return len - ptr; }
        char *curbuf() { return buf + ptr; }
        char *buf;
        size_t len;
        size_t ptr;
    };
    typedef std::list<MES *> ucharp_v;
	ucharp_v m_mes;

};

#endif /* REALM_HANDLER_H */
