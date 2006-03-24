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

#ifndef _AUTHSOCKET_H
#define _AUTHSOCKET_H

#include "Common.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "Network/TcpSocket.h"
#include "../../dep/include/openssl/md5.h"
#include "Config/ConfigEnv.h" 
#include "SystemConfig.h"
class SocketHandler;
class AuthSocket;
#define ChunkSize 2048

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
        STATUS_CONNECTED = 0,
        STATUS_AUTHED
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

typedef struct {
    eAuthCmd cmd;
    uint32 status;
    void (AuthSocket::*handler)(void);
}AuthHandler;

// Only GCC 4.1.0 and later support #pragma pack(pop) syntax
#if __GNUC__ && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
#pragma pack()
#else
#pragma pack(pop)
#endif

class AuthSocket: public TcpSocket
{
    public:
        const static int N_BYTE_SIZE = 32;
        const static int s_BYTE_SIZE = 32;

        AuthSocket(SocketHandler& h);
        ~AuthSocket();

        void OnAccept();
        void OnRead();

		void _HandleLogonChallenge();
        void _HandleLogonProof();
        void _HandleRealmList();
	//data transfer handle for patch

		void _HandleXferResume();
		void _HandleXferCancel();
		void _HandleXferAccept();
		FILE *pPatch;  
		bool IsLag();
	
    private:
		   
	
        BigNumber N, s, g, v;
        BigNumber b, B;
        BigNumber rs;
        
        BigNumber K;
        bool _authed;

        std::string _login;
};



class PatcherRunnable: public ZThread::Runnable
{
    public:
	PatcherRunnable(class AuthSocket *);
	void run();

	private:
	AuthSocket * mySocket;
};

typedef struct{
//	uint8 id[10];
	uint8 md5[MD5_DIGEST_LENGTH];
}PATCH_INFO;




class Patcher 
{
	public:
    typedef std::map<std::string, PATCH_INFO*> Patches;
    ~Patcher();
    Patcher();
    Patches::const_iterator begin() const { return _patches.begin(); }
    Patches::const_iterator end() const { return _patches.end(); }
	void LoadPatchMD5(char*);
	bool GetHash(char * pat,uint8 mymd5[16]);
	void LoadPatchesInfo();

	private:
    	Patches _patches;
};


#endif
