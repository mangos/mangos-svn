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

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

void printerror(int sid, int status, char* title, char* text)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n", status, title);
	prints("<BODY BGCOLOR=#F0F0F0 TEXT=#000000 LINK=#0000FF ALINK=#0000FF VLINK=#0000FF>\n");
	prints("<H1>%d %s</H1>\n", status, title);
	prints("%s\n", text);
	prints("<HR>\n<ADDRESS>%s</ADDRESS>\n</BODY></HTML>\n", SERVER_NAME);
	conn[sid].dat->out_bodydone=1;
	flushbuffer(sid);
	closeconnect(sid, 1);
	return;
}

char *get_mime_type(char *name)
{
	char *mime_types[40][2]={
		{ ".html", "text/html" },
		{ ".htm",  "text/html" },
		{ ".shtml","text/html" },
		{ ".css",  "text/css" },
		{ ".txt",  "text/plain" },
		{ ".mdb",  "application/msaccess" },
		{ ".xls",  "application/msexcel" },
		{ ".doc",  "application/msword" },
		{ ".exe",  "application/octet-stream" },
		{ ".pdf",  "application/pdf" },
		{ ".rtf",  "application/rtf" },
		{ ".tgz",  "application/x-compressed" },
		{ ".gz",   "application/x-compressed" },
		{ ".z",    "application/x-compress" },
		{ ".swf",  "application/x-shockwave-flash" },
		{ ".tar",  "application/x-tar" },
		{ ".rar",  "application/x-rar-compressed" },
		{ ".zip",  "application/x-zip-compressed" },
		{ ".ra",   "audio/x-pn-realaudio" },
		{ ".ram",  "audio/x-pn-realaudio" },
		{ ".wav",  "audio/x-wav" },
		{ ".gif",  "image/gif" },
		{ ".jpeg", "image/jpeg" },
		{ ".jpe",  "image/jpeg" },
		{ ".jpg",  "image/jpeg" },
		{ ".png",  "image/png" },
		{ ".avi",  "video/avi" },
		{ ".mp3",  "video/mpeg" },
		{ ".mpeg", "video/mpeg" },
		{ ".mpg",  "video/mpeg" },
		{ ".qt",   "video/quicktime" },
		{ ".mov",  "video/quicktime" },
		{ "",      "" }
	};
	char *extension;
	int i;

	extension=strrchr(name, '.');
	if (extension==NULL) {
		return "text/plain";
	}
	i=0;
	while (strlen(mime_types[i][0])>0) {
		if (strcmp(extension, mime_types[i][0])==0) {
			return mime_types[i][1];
		}
		i++;
	}
	return "application/octet-stream";
}

void ReadPOSTData(int sid) {
	char *pPostData;
	int rc=0;
	int x=0;

	if (conn[sid].PostData!=NULL) {
		free(conn[sid].PostData);
		conn[sid].PostData=NULL;
	}
	conn[sid].PostData=calloc(conn[sid].dat->in_ContentLength+1024, sizeof(char));
	if (conn[sid].PostData==NULL) {
		logerror("Memory allocation error while reading POST data.");
		closeconnect(sid, 1);
	}
	pPostData=conn[sid].PostData;
	/* reading beyond PostContentLength is required for IE5.5 and NS6 (HTTP 1.1) */
	do {
		rc=recv(conn[sid].socket, pPostData, 1024, 0);
		if (rc==-1) {
			closeconnect(sid, 1);
			return;
		}
		pPostData+=rc;
		x+=rc;
	} while ((rc==1024)||(x<conn[sid].dat->in_ContentLength));
	conn[sid].PostData[conn[sid].dat->in_ContentLength]='\0';
}

int read_header(int sid)
{
	char line[2048];
	char *pTemp;
	time_t x;

	strncpy(conn[sid].dat->in_RemoteAddr, inet_ntoa(conn[sid].ClientAddr.sin_addr), sizeof(conn[sid].dat->in_RemoteAddr)-1);
	x=time((time_t*)0);
	do {
		memset(line, 0, sizeof(line));
		sgets(line, sizeof(line)-1, conn[sid].socket);
		striprn(line);
	} while ((strlen(line)==0)&&((time((time_t)0)-x)<30));
	if ((strlen(line)==0)&&((time((time_t)0)-x)>=30)) {
#ifdef DEBUG
logdata("\n[[[ KILLING IDLE KEEPALIVE ]]]\n");
#endif
		closeconnect(sid, 1);
	}
#ifdef DEBUG
logdata("\n[[[ STARTING REQUEST ]]]\n");
#endif
	if (strlen(line)==0)
		printerror(sid, 400, "Bad Request", "No Request Found.");
	if (sscanf(line, "%[^ ] %[^ ] %[^ ]", conn[sid].dat->in_RequestMethod, conn[sid].dat->in_RequestURI, conn[sid].dat->in_Protocol)!=3)
		printerror(sid, 400, "Bad Request", "Can't Parse Request.");
	pTemp=conn[sid].dat->in_RequestMethod;
	while (*pTemp) { *pTemp=toupper(*pTemp); pTemp++; };
	while (strlen(line)>0) {
		sgets(line, sizeof(line)-1, conn[sid].socket);
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) line[strlen(line)-1]='\0';
		if (strncasecmp(line, "Connection: ", 12)==0)
			strncpy(conn[sid].dat->in_Connection, (char *)&line+12, sizeof(conn[sid].dat->in_Connection)-1);
		if (strncasecmp(line, "Content-Length: ", 16)==0) {
			conn[sid].dat->in_ContentLength=atoi((char *)&line+16);
			if (conn[sid].dat->in_ContentLength<0) {
				// Negative Content-Length?  If so, the client is either broken or malicious.
				// Thanks to <ilja@idefense.be> for spotting this one.
				logerror("ERROR: negative Content-Length of %d provided by client.", conn[sid].dat->in_ContentLength);
				conn[sid].dat->in_ContentLength=0;
			}
		}
		if (strncasecmp(line, "Cookie: ", 8)==0)
			strncpy(conn[sid].dat->in_Cookie, (char *)&line+8, sizeof(conn[sid].dat->in_Cookie)-1);
		if (strncasecmp(line, "Host: ", 6)==0)
			strncpy(conn[sid].dat->in_Host, (char *)&line+6, sizeof(conn[sid].dat->in_Host)-1);
		if (strncasecmp(line, "If-Modified-Since: ", 19)==0)
			strncpy(conn[sid].dat->in_IfModifiedSince, (char *)&line+19, sizeof(conn[sid].dat->in_IfModifiedSince)-1);
		if (strncasecmp(line, "User-Agent: ", 12)==0)
			strncpy(conn[sid].dat->in_UserAgent, (char *)&line+12, sizeof(conn[sid].dat->in_UserAgent)-1);
	}
	if ((strcmp(conn[sid].dat->in_RequestMethod, "GET")!=0)&&(strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0)) {
		printerror(sid, 501, "Not Implemented", "That method is not implemented.");
		closeconnect(sid, 1);
		return -1;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if (conn[sid].dat->in_ContentLength<MAX_POSTSIZE) {
			ReadPOSTData(sid);
		} else {
			// try to print an error : note the inbuffer being full may block us
			// FIXME: this is causing the children to segfault in win32
			printerror(sid, 413, "Bad Request", "Request entity too large.");
			logerror("%s - Large POST (>%d bytes) disallowed", conn[sid].dat->in_RemoteAddr, MAX_POSTSIZE);
			closeconnect(sid, 1);
			return -1;
		}
	}
	if (conn[sid].dat->in_RequestURI[0]!='/') {
		printerror(sid, 400, "Bad Request", "Bad filename.");
	}
	if (strchr(conn[sid].dat->in_RequestURI, '?')!=NULL) {
		strncpy(conn[sid].dat->in_QueryString, strchr(conn[sid].dat->in_RequestURI, '?')+1, sizeof(conn[sid].dat->in_QueryString)-1);
	}
	return 0;
}

void send_header(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	char timebuf[100];
	time_t now;

	if (status) {
		conn[sid].dat->out_status=status;
	} else {
		conn[sid].dat->out_status=200;
	}
	if (length>=0) {
		conn[sid].dat->out_ContentLength=length;
	}
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		snprintf(conn[sid].dat->out_LastModified, sizeof(conn[sid].dat->out_LastModified)-1, "%s", timebuf);
	}
	now=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	snprintf(conn[sid].dat->out_Date, sizeof(conn[sid].dat->out_Date)-1, "%s", timebuf);
	if (cacheable) {
		snprintf(conn[sid].dat->out_CacheControl, sizeof(conn[sid].dat->out_CacheControl)-1, "public");
		snprintf(conn[sid].dat->out_Pragma, sizeof(conn[sid].dat->out_Pragma)-1, "public");
	} else {
		snprintf(conn[sid].dat->out_CacheControl, sizeof(conn[sid].dat->out_CacheControl)-1, "no-store");
		snprintf(conn[sid].dat->out_Expires, sizeof(conn[sid].dat->out_Expires)-1, "%s", timebuf);
		snprintf(conn[sid].dat->out_Pragma, sizeof(conn[sid].dat->out_Pragma)-1, "no-cache");
	}
	if (extra_header!=(char*)0) {
		snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "%s", mime_type);
	} else {
		snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "text/html");
	}
}

void send_fileheader(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	char timebuf[100];
	time_t now;

	if (status) {
		conn[sid].dat->out_status=status;
	} else {
		conn[sid].dat->out_status=200;
	}
	if (strcasestr(conn[sid].dat->in_Protocol, "HTTP/1.1")!=NULL) {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.1");
	} else {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.0");
	}
	if (strcasecmp(conn[sid].dat->in_Connection, "Keep-Alive")==0) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Keep-Alive");
	} else {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	// Nutscrape and Mozilla don't know what a fucking keepalive is
	if ((strcasestr(conn[sid].dat->in_UserAgent, "MSIE")==NULL)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	prints("%s %d OK\r\n", conn[sid].dat->out_Protocol, conn[sid].dat->out_status);
	prints("Connection: %s\r\n", conn[sid].dat->out_Connection);
	prints("Server: %s\r\n", SERVER_NAME);
	if ((length>=0)&&(status!=304)) {
		prints("Content-Length: %d\r\n", length);
	}
	now=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	prints("Date: %s\r\n", timebuf);
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		prints("Last-Modified: %s\r\n", timebuf);
	}
	if (cacheable) {
		now=time((time_t*)0)+604800;
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
		prints("Expires: %s\r\n", timebuf);
		prints("Cache-Control: public\r\n");
		prints("Pragma: public\r\n");
	} else {
		now=time((time_t*)0);
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
		prints("Expires: %s\r\n", timebuf);
		prints("Cache-Control: no-store\r\n");
		prints("Pragma: no-cache\r\n");
	}
	if (extra_header!=(char*)0) {
		prints("Content-Type: %s\r\n\r\n", mime_type);
	} else {
		prints("Content-Type: text/html\r\n\r\n");
	}
	conn[sid].dat->out_headdone=1;
	flushbuffer(sid);
}
