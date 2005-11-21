/*
    Null httpd -- simple http server
    Copyright (C) 2001-2002 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* #includes */
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
	#pragma comment(lib, "libcmt.lib")
	#pragma comment(lib, "wsock32.lib")
	#define _MT 1
	#include <winsock.h>
	#include <windows.h>
	#include <process.h>
	#include <shellapi.h>
	#include <signal.h>
	#include <windowsx.h>
	#include <io.h>
	#include <direct.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <dirent.h>
	#include <netdb.h>
	#include <paths.h>
	#include <pthread.h>
	#include <signal.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
#endif

/* #defines and typedefs */
#ifdef WIN32
#define APPTITLE	"Mangos Internal HTTPD 0.0.1"
#define DEFAULT_BASE_DIR ".\\"
#else
#define DEFAULT_BASE_DIR "./"
#endif
#define SERVER_NAME	"Mangos Internal HTTPD 0.0.1"

#define MAX_POSTSIZE	33554432 /* arbitrary 32 MB limit for POST request sizes */
#define MAX_REPLYSIZE	65536 /* arbitrary 64 KB limit for reply buffering */

#ifdef WIN32
#define S_IFMT		_S_IFMT   
#define S_IFDIR		_S_IFDIR  
#define S_IFCHR		_S_IFCHR  
#define S_IFIFO		_S_IFIFO  
#define S_IFREG		_S_IFREG  
#define S_IREAD		_S_IREAD  
#define S_IWRITE	_S_IWRITE 
#define S_IEXEC		_S_IEXEC 
#define S_IFLNK		0000000
#define S_IFSOCK	0000000
#define S_IFBLK		0000000
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001
#define S_ISDIR(x)	(x & _S_IFDIR)
#define S_ISLNK(x)	0
#define ATTRIBUTES	(_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_SUBDIR)
#define	MAXNAMLEN	255

struct direct {
	ino_t d_ino;
	int d_reclen;
	int d_namlen;
	char d_name[MAXNAMLEN+1];
};
struct _dircontents {
	char *_d_entry;
	struct _dircontents *_d_next;
};
typedef struct _dirdesc {
	int dd_id;
	long dd_loc;
	struct _dircontents *dd_contents;
	struct _dircontents *dd_cp;
} DIR;
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
/* pthread_ typedefs */
typedef HANDLE pthread_t;
typedef struct thread_attr {
	DWORD dwStackSize;
	DWORD dwCreatingFlag;
	int priority;
} pthread_attr_t;
typedef struct {
	int dummy;
} pthread_condattr_t;
typedef unsigned int uint;
typedef struct {
	uint waiting;
	HANDLE semaphore;
} pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

typedef struct {
	char config_filename[255];
	char server_base_dir[255];
	char server_bin_dir[255];
	char server_cgi_dir[255];
	char server_cgi_bin_dir[255];
	char server_etc_dir[255];
	char server_htdocs_dir[255];
	char server_hostname[64];
	short int server_port;
	short int server_loglevel;
	short int server_maxconn;
	short int server_maxidle;
} CONFIG;
typedef struct {
	// incoming data
	char in_Connection[16];
	int  in_ContentLength;
	char in_ContentType[128];
	char in_Cookie[1024];
	char in_Host[64];
	char in_IfModifiedSince[64];
	char in_PathInfo[128];
	char in_Protocol[16];
	char in_QueryString[1024];
	char in_Referer[128];
	char in_RemoteAddr[16];
	int  in_RemotePort;
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
	// outgoing data
	short int out_status;
	char out_CacheControl[16];
	char out_Connection[16];
	int  out_ContentLength;
	char out_Date[64];
	char out_Expires[64];
	char out_LastModified[64];
	char out_Pragma[16];
	char out_Protocol[16];
	char out_Server[128];
	char out_ContentType[128];
	char out_ReplyData[MAX_REPLYSIZE];
	short int out_headdone;
	short int out_bodydone;
	short int out_flushed;
	// user data
	char envbuf[8192];
} CONNDATA;
typedef struct {
	pthread_t handle;
	unsigned long int id;
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	char *PostData;
	CONNDATA *dat;
} CONNECTION;
typedef struct {
	int in;
	int out;
} pipe_fd;

/* global vars */
#ifdef WIN32
HINSTANCE hInst;
#endif
struct {
	pthread_mutex_t Crypt;
	pthread_mutex_t Global;
	pthread_mutex_t SQL;
} Lock;
unsigned char program_name[255];
CONFIG config;
CONNECTION *conn;

/* function forwards */
/* win32.c functions */
#ifdef WIN32
unsigned sleep(unsigned seconds);
DIR	*opendir(const char *);
void	closedir(DIR *);
#define	rewinddir(dirp)	seekdir(dirp, 0L)
void	seekdir(DIR *, long);
long	telldir(DIR *);
struct	direct *readdir(DIR *);
int	gettimeofday(struct timeval *tv, struct timezone *tz); 
/* registry and service control functions */
int get_reg_entries(void);
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
void pthread_exit(unsigned A);
#endif
/* main.c functions */
void dorequest(int sid);
/* cgi.c */
int cgi_main(void);
/* config.c functions */
int config_read(void);
/* files.c functions */
int dirlist(int sid);
int sendfile(int sid, unsigned char *file);
/* format.c */
void decodeurl(unsigned char *pEncoded);
void fixslashes(char *pOriginal);
int hex2int(char *pChars);
void striprn(char *string);
void swapchar(char *string, char oldchar, char newchar);
char *strcasestr(char *src, char *query);
char *strcatf(char *dest, const char *format, ...);
int printhex(const char *format, ...);
int printht(const char *format, ...);
/* http.c functions */
void printerror(int sid, int status, char* title, char* text);
char *get_mime_type(char *name);
void ReadPOSTData(int sid);
int read_header(int sid);
void send_header(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
void send_fileheader(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
/* server.c functions */
void logaccess(int loglevel, const char *format, ...);
void logerror(const char *format, ...);
int getsid(void);
void flushbuffer(int sid);
int prints(const char *format, ...);
int sgets(char *buffer, int max, int fd);
int closeconnect(int sid, int exitflag);
void server_shutdown();
int sockinit(void);
void cgiinit(void);
void init(void);
#ifdef WIN32
void WSAReaper(void *x);
#endif
void accept_loop(void *x);
