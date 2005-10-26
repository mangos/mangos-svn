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
#include "main.h"

#ifdef WIN32
static WSADATA wsaData;
#endif
static int ListenSocket;

void logaccess(int loglevel, const char *format, ...)
{
	char logbuffer[1024];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	if (loglevel>config.server_loglevel) return;
	snprintf(file, sizeof(file)-1, "%s/access.log", config.server_etc_dir);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime(&ttime.tv_sec));
		fprintf(fp, "%s - [%d] %s\n", timebuffer, loglevel, logbuffer);
		fclose(fp);
	}
}

void logerror(const char *format, ...)
{
	char logbuffer[1024];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	struct timeval ttime;
	struct timezone tzone;

	snprintf(file, sizeof(file)-1, "%s/error.log", config.server_etc_dir);
	fixslashes(file);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		gettimeofday(&ttime, &tzone);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", localtime(&ttime.tv_sec));
		fprintf(fp, "%s - %s\n", timebuffer, logbuffer);
		fclose(fp);
	}
}

int getsid()
{
	int sid;

	for (sid=0;sid<config.server_maxconn;sid++) {
		if (conn[sid].id==pthread_self()) break;
	}
	if ((sid<0)||(sid>=config.server_maxconn)) {
		return -1;
	}
	return sid;
}

void flushheader(int sid)
{
	char line[256];

	if (conn[sid].dat->out_headdone) return;
	if (!conn[sid].dat->out_status) {
		conn[sid].dat->out_headdone=1;
		return;
	}
#ifdef DEBUG
logdata("\n[[[ FLUSHING HEADER ]]]\n");
#endif
	if ((conn[sid].dat->out_bodydone)&&(!conn[sid].dat->out_flushed)) {
		conn[sid].dat->out_ContentLength=strlen(conn[sid].dat->out_ReplyData);
	}
	if ((strcasecmp(conn[sid].dat->in_Connection, "Keep-Alive")==0)&&(conn[sid].dat->out_bodydone)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Keep-Alive");
	} else {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	// Nutscrape and Mozilla don't know what a fucking keepalive is
	if ((strcasestr(conn[sid].dat->in_UserAgent, "MSIE")==NULL)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	if (strcasestr(conn[sid].dat->in_Protocol, "HTTP/1.1")!=NULL) {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.1");
	} else {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.0");
	}
	snprintf(line, sizeof(line)-1, "%s %d OK\r\n", conn[sid].dat->out_Protocol, conn[sid].dat->out_status);
	send(conn[sid].socket, line, strlen(line), 0);
	if (strlen(conn[sid].dat->out_CacheControl)) {
		snprintf(line, sizeof(line)-1, "Cache-Control: %s\r\n", conn[sid].dat->out_CacheControl);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (strlen(conn[sid].dat->out_Connection)) {
		snprintf(line, sizeof(line)-1, "Connection: %s\r\n", conn[sid].dat->out_Connection);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (conn[sid].dat->out_bodydone) {
		snprintf(line, sizeof(line)-1, "Content-Length: %d\r\n", conn[sid].dat->out_ContentLength);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (strlen(conn[sid].dat->out_Date)) {
		snprintf(line, sizeof(line)-1, "Date: %s\r\n", conn[sid].dat->out_Date);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (strlen(conn[sid].dat->out_Expires)) {
		snprintf(line, sizeof(line)-1, "Expires: %s\r\n", conn[sid].dat->out_Expires);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (strlen(conn[sid].dat->out_LastModified)) {
		snprintf(line, sizeof(line)-1, "Last-Modified: %s\r\n", conn[sid].dat->out_LastModified);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	if (strlen(conn[sid].dat->out_Pragma)) {
		snprintf(line, sizeof(line)-1, "Pragma: %s\r\n", conn[sid].dat->out_Pragma);
		send(conn[sid].socket, line, strlen(line), 0);
	}
	snprintf(line, sizeof(line)-1, "Server: %s\r\n", SERVER_NAME);
	send(conn[sid].socket, line, strlen(line), 0);
	if (strlen(conn[sid].dat->out_ContentType)) {
		snprintf(line, sizeof(line)-1, "Content-Type: %s\r\n\r\n", conn[sid].dat->out_ContentType);
		send(conn[sid].socket, line, strlen(line), 0);
	} else {
		snprintf(line, sizeof(line)-1, "Content-Type: text/plain\r\n\r\n");
		send(conn[sid].socket, line, strlen(line), 0);
	}
	conn[sid].dat->out_headdone=1;
#ifdef DEBUG
logdata("\n[[[ DONE FLUSHING HEADER ]]]\n");
#endif
	return;
}

void flushbuffer(int sid)
{
	char *pTemp=conn[sid].dat->out_ReplyData;
	unsigned int dcount;

	flushheader(sid);
	if (strlen(pTemp)==0) return;
	conn[sid].dat->out_flushed=1;
	while (strlen(pTemp)) {
		dcount=512;
		if (strlen(pTemp)<dcount) dcount=strlen(pTemp);
		send(conn[sid].socket, pTemp, dcount, 0);
		pTemp+=dcount;
	}
	memset(conn[sid].dat->out_ReplyData, 0, sizeof(conn[sid].dat->out_ReplyData));
	return;
}

int prints(const char *format, ...)
{
	unsigned char buffer[2048];
	va_list ap;
	int sid=getsid();

	if (sid==-1) return -1;
	conn[sid].atime=time((time_t*)0);
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	if (strlen(conn[sid].dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	strcat(conn[sid].dat->out_ReplyData, buffer);
	if (strlen(conn[sid].dat->out_ReplyData)+sizeof(buffer)>MAX_REPLYSIZE-2) {
		flushbuffer(sid);
	}
	return 0;
}

int sgets(char *buffer, int max, int fd)
{
	int n=0;
	int rc;
	int sid=getsid();

	if (sid==-1) return -1;
	conn[sid].atime=time((time_t*)0);
	while (n<max) {
		if ((rc=recv(conn[sid].socket, buffer, 1, 0))<0) {
			conn[sid].dat->out_headdone=1;
			conn[sid].dat->out_bodydone=1;
			conn[sid].dat->out_flushed=1;
			conn[sid].dat->out_ReplyData[0]='\0';
			closeconnect(sid, 1);
		} else if (rc!=1) {
			n= -n;
			break;
		}
		n++;
		if (*buffer=='\n') {
			buffer++;
			break;
		}
		buffer++;
	}
	*buffer=0;
	return n;
}

int closeconnect(int sid, int exitflag)
{
#ifdef WIN32
	char junk[16];
	int rc;
#endif

	flushbuffer(sid);
#ifdef WIN32
	/* shutdown(x,0=recv, 1=send, 2=both) */
	shutdown(conn[sid].socket, 1);
	while ((rc=recv(conn[sid].socket, junk-1, sizeof(junk)-1, 0))>0);
	shutdown(conn[sid].socket, 2);
	closesocket(conn[sid].socket);
#else
	close(conn[sid].socket);
#endif
	if (exitflag) {
	logaccess(4, "Closing [%u][%u]", conn[sid].id, conn[sid].socket);
#ifdef WIN32
		CloseHandle(conn[sid].handle);
#endif
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		memset((char *)&conn[sid], 0, sizeof(conn[sid]));
		pthread_exit(0);
	}
	return 0;
}

#ifndef WIN32
int daemon(int nochdir, int noclose)
{
	int fd;

	switch (fork()) {
		case -1: return -1;
		case 0:  break;
		default: _exit(0);
	}
	if (setsid()==-1) return -1;
	if (noclose) return 0;
	fd=open(_PATH_DEVNULL, O_RDWR, 0);
	if (fd!=-1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd>2) close(fd);
	}
	return 0;
}
#endif

void server_shutdown()
{
	logaccess(0, "Stopping %s", SERVER_NAME);
	close(ListenSocket);
	fflush(stdout);
	exit(0);
}

#ifdef WIN32
void logsegv(int sig)
{
	switch (sig) {
		case 2:
			logerror("SIGINT [%d] Interrupt", sig);
			break;
		case 4:
			logerror("SIGILL [%d] Illegal Instruction", sig);
			break;
		case 8:
			logerror("SIGFPE [%d] Floating Point Exception", sig);
			break;
		case 11:
			logerror("SIGSEGV [%d] Segmentation Violation", sig);
			break;
		case 15:
			logerror("SIGTERM [%d] Software Termination signal from kill", sig);
			break;
		case 22:
			logerror("SIGABRT [%d] Abnormal Termination", sig);
			break;
		default:
			logerror("Unknown signal [%d] received", sig);
	}
//	closeconnect(sid, 1);
	exit(-1);
}
#endif

void setsigs()
{
#ifdef WIN32
#ifndef DEBUG
//	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
#endif
	signal(SIGINT, logsegv);
	signal(SIGILL, logsegv);
	signal(SIGFPE, logsegv);
	signal(SIGSEGV, logsegv);
	signal(SIGTERM, logsegv);
	signal(SIGABRT, logsegv);
#else
	sigset_t blockmask;
	sigset_t emptymask;
	struct sigaction sa;

	sigemptyset(&emptymask);
	sigemptyset(&blockmask);
	sigaddset(&blockmask, SIGCHLD);
	sigaddset(&blockmask, SIGHUP);
	sigaddset(&blockmask, SIGALRM);
	memset(&sa, 0, sizeof(sa));
	sa.sa_mask = blockmask;
	sa.sa_handler = server_shutdown;
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = server_shutdown;
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
#endif
}

int sockinit()
{
	struct hostent *hp;
	struct sockaddr_in sin;
	int i;

#ifdef WIN32
	if (WSAStartup(0x202, &wsaData)) {
		MessageBox(0, "Winsock 2 initialization failed.", APPTITLE, MB_ICONERROR);
		WSACleanup();
		exit(0);
	}
#else
	printf("Binding to 'http://%s:%d/'...", config.server_hostname, config.server_port);
	fflush(stdout);
#endif
	ListenSocket=socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family=AF_INET;
	if (strcasecmp("ANY", config.server_hostname)==0) {
		sin.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
		if ((hp=gethostbyname(config.server_hostname))==NULL) {
#ifdef WIN32
			MessageBox(0, "DNS Lookup failure.", APPTITLE, MB_ICONERROR);
#else
			printf("DNS Lookup failure.\r\n");
#endif
			exit(0);
		}
		memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
	}
	sin.sin_port=htons((unsigned short)config.server_port);
	i=0;
	while (bind(ListenSocket, (struct sockaddr *)&sin, sizeof(sin))<0) {
		sleep(5);
		i++;
		if (i>6) {
			logerror("bind() error [%s:%d]", config.server_hostname, config.server_port);
#ifdef WIN32
			MessageBox(0, "Bind error: Null httpd could not bind itself to the specified port.", APPTITLE, MB_ICONERROR);
#else
			perror("\nBind error");
#endif
			exit(0);
		}
	}
	if (listen(ListenSocket, 50)<0) {
		logerror("listen() error");
#ifdef WIN32
		closesocket(ListenSocket);
#else
		close(ListenSocket);
#endif
		exit(0);
	}
#ifndef WIN32
	printf("OK.\r\n");
	daemon(0, 0);
#endif
	setsigs();
	return 0;
}

/****************************************************************************
 *	CGIkilltimer()
 *
 *	Purpose	: Function to kill retarded child processes
 *	Args	: None
 *	Returns	: void
 *	Notes	: Created as a thread in Win32
 ***************************************************************************/
#ifdef WIN32
void CGIkilltimer(void *x)
{
	int idleseconds;

	for (;;) {
		sleep(5);
		idleseconds=time((time_t*)0)-conn[0].atime;
		if (idleseconds>config.server_maxidle) break;
	}
	logaccess(4, "CGI is idle for more than %d seconds.  Terminating.", config.server_maxidle);
	exit(0);
	return;
}
#endif

/****************************************************************************
 *	init()
 *
 *	Purpose	: Test external dependencies and initialize socket interface
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void init()
{
	printf("Initializing %s.\r\n", SERVER_NAME);
	pthread_mutex_init(&Lock.Crypt, NULL);
	pthread_mutex_init(&Lock.Global, NULL);
	pthread_mutex_init(&Lock.SQL, NULL);
	if (config_read()!=0) {
#ifdef WIN32
		MessageBox(0, "Error reading configuration file", APPTITLE, MB_ICONERROR);
#else
		printf("\r\nError reading configuration file\r\n");
#endif
		exit(0);
	}
	logaccess(0, "Starting %s", SERVER_NAME);
	conn=calloc(config.server_maxconn, sizeof(CONNECTION));
	sockinit();
	printf("Initialized %s.\r\n", SERVER_NAME);
}

#ifdef WIN32
void WSAReaper(void *x)
{
	short int connections;
	short int i;
	char junk[10];
	int rc;
	time_t ctime;

	for (;;) {
		connections=0;
		ctime=time((time_t)0);
		for (i=0;i<config.server_maxconn;i++) {
 			if (conn[i].id==0) continue;
			GetExitCodeThread((HANDLE)conn[i].handle, &rc);
			if (rc!=STILL_ACTIVE) continue;
			connections++;
			if ((ctime-conn[i].atime<config.server_maxidle)||(conn[i].atime==0)) continue;
			logaccess(4, "Reaping socket %u from pid %u (runtime ~= %d seconds)", conn[i].socket, conn[i].id, ctime-conn[i].atime);
			shutdown(conn[i].socket, 2);
			while (recv(conn[i].socket, junk, sizeof(junk), 0)>0) { };
			closesocket(conn[i].socket);
			TerminateThread(conn[i].handle, (DWORD)&rc);
			CloseHandle(conn[i].handle);
			if (conn[i].PostData!=NULL) free(conn[i].PostData);
			if (conn[i].dat!=NULL) free(conn[i].dat);
			memset((char *)&conn[i], 0, sizeof(conn[i]));
		}
		Sleep(100);
	}
	return;
}
#endif

#ifdef WIN32
unsigned _stdcall htloop(void *x)
#else
unsigned htloop(void *x)
#endif
{
	int sid=(int)x;

	conn[sid].id=pthread_self();
#ifndef WIN32
	pthread_detach(conn[sid].id);
#endif
	logaccess(4, "New client [%u][%u]", conn[sid].id, conn[sid].socket);
	for (;;) {
		if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
		if (conn[sid].dat!=NULL) free(conn[sid].dat);
		conn[sid].dat=calloc(1, sizeof(CONNDATA));
		conn[sid].dat->out_ContentLength=-1;
		conn[sid].atime=time((time_t)0);
		conn[sid].ctime=time((time_t)0);
		conn[sid].PostData=NULL;
		dorequest(sid);
//		if (conn[sid].dat->out_status!=304) {
			prints("\r\n\r\n");
//		}
		conn[sid].dat->out_bodydone=1;
		flushbuffer(sid);
		if (strcasestr(conn[sid].dat->out_Connection, "close")!=NULL) break;
	}
	closeconnect(sid, 0);
	logaccess(4, "Closing [%u][%u]", conn[sid].id, conn[sid].socket);
#ifdef WIN32
	CloseHandle((HANDLE)conn[sid].handle);
#endif
	if (conn[sid].PostData!=NULL) free(conn[sid].PostData);
	if (conn[sid].dat!=NULL) free(conn[sid].dat);
	memset((char *)&conn[sid], 0, sizeof(conn[sid]));
	pthread_exit(0);
	return 0;
}

/****************************************************************************
 *	accept_loop()
 *
 *	Purpose	: Function to handle incoming socket connections
 *	Args	: None
 *	Returns	: void
 *	Notes	: Created as a thread in Win32
 ***************************************************************************/
void accept_loop(void *x)
{
	pthread_attr_t thr_attr;
	int fromlen;
	int i;

	if (pthread_attr_init(&thr_attr)) {
		logerror("pthread_attr_init()");
		exit(1);
	}
	if (pthread_attr_setstacksize(&thr_attr, 65536L)) {
		logerror("pthread_attr_setstacksize()");
		exit(1);
	}
	for (;;) {
		for (i=0;;i++) {
			if (i>=config.server_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].socket==0) break;
		}
		if (conn[i].PostData!=NULL) free(conn[i].PostData);
		if (conn[i].dat!=NULL) free(conn[i].dat);
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		fromlen=sizeof(conn[i].ClientAddr);
		conn[i].socket=accept(ListenSocket, (struct sockaddr *)&conn[i].ClientAddr, &fromlen);
#ifdef WIN32
		if (conn[i].socket==INVALID_SOCKET) {
			logerror("accept() died...  restarting...");
			closesocket(ListenSocket);
			WSACleanup();
			exit(0);
#else
		if (conn[i].socket<0) {
			continue;
#endif
		} else {
			conn[i].id=1;
			if (pthread_create(&conn[i].handle, &thr_attr, htloop, (void *)i)==-1) {
				logerror("htloop() failed...");
				exit(0);
			}
		}
	}
	return;
}

/****************************************************************************
 *	internal_accept_loop()
 *
 *	Purpose	: Function to handle incoming socket connections
 *	Args	: None
 *	Returns	: void
 *	Notes	: Created as a thread in Win32
 ***************************************************************************/
void internal_accept_loop(void *x)
{
	pthread_attr_t thr_attr;
	int i, fromlen;

	//for (;;) {
		for (i=0;;i++) {
			if (i>=config.server_maxconn) {
				sleep(1);
				i=0;
				continue;
			}
			if (conn[i].socket==0) break;
		}
		if (conn[i].PostData!=NULL) free(conn[i].PostData);
		if (conn[i].dat!=NULL) free(conn[i].dat);
		memset((char *)&conn[i], 0, sizeof(conn[i]));
		fromlen=sizeof(conn[i].ClientAddr);
		conn[i].socket=accept(ListenSocket, (struct sockaddr *)&conn[i].ClientAddr, &fromlen);
#ifdef WIN32
		if (conn[i].socket==INVALID_SOCKET) {
			logerror("accept() died...  restarting...");
			closesocket(ListenSocket);
			WSACleanup();
			exit(0);
#else
		if (conn[i].socket<0) {
			continue;
#endif
		} else {
			conn[i].id=1;
			if (pthread_create(&conn[i].handle, &thr_attr, htloop, (void *)i)==-1) {
				logerror("htloop() failed...");
				exit(0);
			}
		}
	//}
	//return;
}

