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

#define BUFF_SIZE 8192

void cgi_makeargs(int sid, char *args[])
{
	char *ptemp;
	char progname[255];

	if (strncmp(conn[sid].dat->in_RequestURI, "/cgi-bin/", 9)!=0) return;
	args[0]=calloc(255, sizeof(char));
	snprintf(progname, sizeof(progname)-1, "%s", conn[sid].dat->in_RequestURI+9);
	if ((ptemp=strchr(progname, '?'))!=NULL) {
		args[1]=calloc(255, sizeof(char));
		snprintf(args[1], 254, "%s", ptemp+1);
		*ptemp='\0';
	}
	if ((ptemp=strchr(progname, '/'))!=NULL) {
		args[2]=calloc(255, sizeof(char));
		snprintf(args[2], 254, "%s", ptemp);
		*ptemp='\0';
	}
	snprintf(args[0], 254, "%s/%s", config.server_cgi_dir, progname);
	fixslashes(args[0]);
}

void cgi_makeenv(int sid, char *env[], char *args[])
{
	char *ptemp;
	int n=0;

	if (strncmp(conn[sid].dat->in_RequestURI, "/cgi-bin/", 9)!=0) return;
#ifdef WIN32
	if ((ptemp=getenv("COMSPEC"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], MAX_PATH-1, "COMSPEC=%s", ptemp);
	}
#endif
	if (strcasecmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "CONTENT_LENGTH=%d", conn[sid].dat->in_ContentLength);
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "CONTENT_TYPE=%s", conn[sid].dat->in_ContentType);
	}
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "DOCUMENT_ROOT=%s", config.server_htdocs_dir);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "GATEWAY_INTERFACE=CGI/1.1");
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_CONNECTION=%s", conn[sid].dat->in_Connection);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_COOKIE=%s", conn[sid].dat->in_Cookie);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_HOST=%s", conn[sid].dat->in_Host);
	if ((ptemp=strchr(env[n-1], ':'))!=NULL) *ptemp='\0';
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "HTTP_USER_AGENT=%s", conn[sid].dat->in_UserAgent);
	if ((ptemp=getenv("PATH"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "PATH=%s", ptemp);
	}
	if (args[2]!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "PATH_INFO=%s", args[2]);
	}
	env[n]=calloc(1024, sizeof(char));
	if (args[1]!=NULL) {
		snprintf(env[n++], 1023, "QUERY_STRING=%s", args[1]);
	} else {
		snprintf(env[n++], 1023, "QUERY_STRING=");
	}
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REMOTE_ADDR=%s", conn[sid].dat->in_RemoteAddr);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REMOTE_PORT=%d", ntohs(conn[sid].ClientAddr.sin_port));
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REQUEST_METHOD=%s", conn[sid].dat->in_RequestMethod);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "REQUEST_URI=%s", conn[sid].dat->in_RequestURI);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SCRIPT_FILENAME=%s", args[0]);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SCRIPT_NAME=%s", conn[sid].dat->in_RequestURI);
	if ((ptemp=strchr(env[n-1], '?'))!=NULL) *ptemp='\0';
	if ((ptemp=strchr(env[n-1]+21, '/'))!=NULL) *ptemp='\0';
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_NAME=%s", config.server_hostname);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_PORT=%d", config.server_port);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_PROTOCOL=HTTP/1.1");
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_SIGNATURE=<ADDRESS>%s</ADDRESS>", SERVER_NAME);
	env[n]=calloc(1024, sizeof(char));
	snprintf(env[n++], 1023, "SERVER_SOFTWARE=%s", SERVER_NAME);
#ifdef WIN32
	if ((ptemp=getenv("WINDIR"))!=NULL) {
		env[n]=calloc(1024, sizeof(char));
		snprintf(env[n++], 1023, "WINDIR=%s", ptemp);
	}
#endif
	free(args[1]);
	args[1]=NULL;
	free(args[2]);
	args[2]=NULL;
}

int cgi_main()
{
#ifdef WIN32
	char *cgi_types[3][2]={
		{ ".php", "PHP.EXE" },
		{ ".pl",  "PERL.EXE" },
		{ NULL,   NULL }
	};
	DWORD exitcode=0;
	HANDLE hMyProcess=GetCurrentProcess();
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char Command[512];
	char Environ[8192];
	char Path[255];
#else
	char *cgi_types[3][2]={
		{ ".php", "/usr/bin/php" },
		{ ".pl",  "/usr/bin/perl" },
		{ NULL,   NULL }
	};
	int pset1[2];
	int pset2[2];
#endif
	char *args[10];
	char *env[50];
	char cgifilename[255];
	char *extension;
	char szBuffer[BUFF_SIZE];
	pipe_fd local;
	pipe_fd remote;
	int sid=getsid();
	int nOutRead;
	int pid;
	unsigned int i;
	unsigned int n;

	memset(args, 0, sizeof(args));
	cgi_makeargs(sid, args);
	memset(env, 0, sizeof(env));
	cgi_makeenv(sid, env, args);
	snprintf(cgifilename, sizeof(cgifilename)-1, "%s", args[0]);
	for (i=0;i<10;i++) free(args[i]);
	n=0;
	if ((extension=strrchr(cgifilename, '.'))!=NULL) {
		for (i=0;cgi_types[i][0]!=NULL;i++) {
			if (strcmp(extension, cgi_types[i][0])==0) {
				args[n]=calloc(255, sizeof(char));
				snprintf(args[n], 254, "%s", cgi_types[i][1]);
				n++;
				break;
			}
		}
	}
	args[n]=calloc(255, sizeof(char));
	snprintf(args[n], 254, "%s", cgifilename);
#ifdef WIN32
	memset(Command, 0, sizeof(Command));
	memset(Environ, 0, sizeof(Environ));
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	snprintf(Path, sizeof(Path)-1, "%s", cgifilename);
	if ((extension=strrchr(Path, '\\'))!=NULL) *extension='\0';
	if (args[1]==NULL) {
		snprintf(Command, sizeof(Command)-1, "%s", cgifilename);
	} else {
		snprintf(Command, sizeof(Command)-1, "%s \"%s\"", cgi_types[i][1], cgifilename);
	}
	for (i=0, n=0;env[i]!=NULL;i++) {
		if (n+strlen(env[i])>sizeof(Environ)) break;
		n+=sprintf(&Environ[n], "%s", env[i]);
		n++;
	}
	if (!CreatePipe((HANDLE)&remote.in, (HANDLE)&local.out, NULL, BUFF_SIZE)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		printerror(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return -1;
	}
	if (!CreatePipe((HANDLE)&local.in, (HANDLE)&remote.out, NULL, BUFF_SIZE)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)local.out);
		printerror(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return -1;
	}
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;
	si.hStdInput=(HANDLE)remote.in;
	si.hStdOutput=(HANDLE)remote.out;
	si.hStdError=(HANDLE)remote.out;
	if (!CreateProcess(NULL, Command, NULL, NULL, TRUE, CREATE_NO_WINDOW, Environ, Path, &si, &pi)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		CloseHandle((HANDLE)local.in);
		CloseHandle((HANDLE)local.out);
		CloseHandle((HANDLE)remote.in);
		CloseHandle((HANDLE)remote.out);
		logerror("CGI failed. [%s]", Command);
		printerror(sid, 500, "Internal Server Error", "There was a problem running the requested CGI.");
		return -1;
	}
	pid=pi.dwProcessId;
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdOutput);
#else
	if ((pipe(pset1)==-1) || (pipe(pset2)==-1)) {
		for (i=0;i<10;i++) free(args[i]);
		for (i=0;i<50;i++) free(env[i]);
		close(pset1[0]);
		close(pset1[1]);
		logerror("pipe() error");
		printerror(sid, 500, "Internal Server Error", "Unable to create pipe.");
		return -1;
	}
	local.in=pset1[0]; remote.out=pset1[1];
	remote.in=pset2[0]; local.out=pset2[1];
	logaccess(1, "Executing CGI [%s %s]", args[0], args[1]);
	pid=fork();
	if (pid<0) {
		logerror("fork() error");
		return -1;
	} else if (pid==0) {
		close(local.in);
		close(local.out);
		dup2(remote.in, fileno(stdin));
		dup2(remote.out, fileno(stdout));
//		if ((dup2(remote.in, fileno(stdin))!=0)||(dup2(remote.out, fileno(stdout))!=0)) {
//			logerror("dup2() error");
//			exit(0);
//		}
		execve(args[0], &args[0], &env[0]);
		logerror("execve() error [%s][%s]", args[0], args[1]);
		exit(0);
	} else {
		close(remote.in);
		close(remote.out);
	}
#endif
	if (conn[sid].dat->in_ContentLength>0) {
#ifdef WIN32
		WriteFile((HANDLE)local.out, conn[sid].PostData, conn[sid].dat->in_ContentLength, &nOutRead, NULL);
#else
		write(local.out, conn[sid].PostData, conn[sid].dat->in_ContentLength);
#endif
	}
	conn[sid].dat->out_headdone=1;
	conn[sid].dat->out_status=200;
	if (strcasestr(conn[sid].dat->in_Protocol, "HTTP/1.1")!=NULL) {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.1");
	} else {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.0");
	}
	snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	prints("%s %d OK\r\n", conn[sid].dat->out_Protocol, conn[sid].dat->out_status);
	prints("Connection: %s\r\n", conn[sid].dat->out_Connection);
	flushbuffer(sid);
	do {
		memset(szBuffer, 0, sizeof(szBuffer));
#ifdef WIN32
		ReadFile((HANDLE)local.in, szBuffer, sizeof(szBuffer)-1, &nOutRead, NULL);
#else
		nOutRead=read(local.in, szBuffer, BUFF_SIZE-1);
#endif
		if (nOutRead>0) {
			send(conn[sid].socket, szBuffer, nOutRead, 0);
		};
	} while (nOutRead>0);
	flushbuffer(sid);
	/* cleanup */
	for (i=0;i<10;i++) free(args[i]);
	for (i=0;i<50;i++) free(env[i]);
#ifdef WIN32
	GetExitCodeProcess(pi.hProcess, &exitcode);
	if (exitcode==STILL_ACTIVE) TerminateProcess(pi.hProcess, 1);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle((HANDLE)local.in);
	CloseHandle((HANDLE)local.out);
#else
	close(local.in);
	close(local.out);
#endif
	conn[sid].dat->out_bodydone=1;
	flushbuffer(sid);
	closeconnect(sid, 1);
	return 0;
}
