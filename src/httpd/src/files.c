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

int sendfile(int sid, unsigned char *file)
{
	struct stat sb;
	FILE *fp;
	char fileblock[2048];
	int blocksize;
	int ich;

	decodeurl(file);
	fixslashes(file);
	if (strstr(file, "..")!=NULL) return -1;
	if (stat(file, &sb)!=0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
/*	SET THIS TO PARSE THE DATE AND ACTUALLY CHECK FOR AN UPDATE
	if (strlen(conn[sid].dat->in_IfModifiedSince)) {
		send_fileheader(sid, 1, 304, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
		conn[sid].dat->out_headdone=1;
		conn[sid].dat->out_bodydone=1;
		conn[sid].dat->out_flushed=1;
		conn[sid].dat->out_ReplyData[0]='\0';
		flushbuffer(sid);
		return 0;
	}
*/
	conn[sid].dat->out_ContentLength=sb.st_size;
	send_fileheader(sid, 1, 200, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
	fp=fopen(file, "rb");
	if (fp==NULL) return -1;
	blocksize=0;
	while ((ich=getc(fp))!=EOF) {
		if (blocksize>sizeof(fileblock)-1) {
			send(conn[sid].socket, fileblock, blocksize, 0);
			blocksize=0;
		}
		fileblock[blocksize]=ich;
		blocksize++;
	}
	if (blocksize) {
		send(conn[sid].socket, fileblock, blocksize, 0);
		blocksize=0;
	}
	fclose(fp);
	conn[sid].dat->out_headdone=1;
	conn[sid].dat->out_bodydone=1;
	conn[sid].dat->out_flushed=1;
	conn[sid].dat->out_ReplyData[0]='\0';
	flushbuffer(sid);
	return 0;
}

int dirlist(int sid)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR	*handle;
	char	file[1024];
	char	index[1024];
	char	showfile[1024];
	struct	stat sb;
	char	*directory;
	char timebuf[100];
	time_t t;

	if (strncmp(conn[sid].dat->in_RequestURI, "/", 1)!=0) {
		return -1;
	}
	directory=conn[sid].dat->in_RequestURI+1;
	snprintf(file, sizeof(file)-1, "%s/%s", config.server_htdocs_dir, directory);
	decodeurl(file);
	fixslashes(file);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	if (strstr(file, "..")!=NULL) return -1;
	snprintf(index, sizeof(index)-1, "%s/%s/index.html", config.server_htdocs_dir, directory);
	decodeurl(index);
	fixslashes(index);
	if (stat(index, &sb)==0) {
		sendfile(sid, index);
		return 0;
	}
	if (stat(file, &sb)!=0) return -1;
	if (!(sb.st_mode & S_IFDIR)) return sendfile(sid, file);
	t=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime(&t));
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints("<CENTER>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
	prints("<TR BGCOLOR=#00A5D0><TH COLSPAN=4>Index of %s</TH></TR>\n", conn[sid].dat->in_RequestURI);
	prints("<TR BGCOLOR=#E0E0E0>");
	prints("<TH width=20%%>Filename</TH><TH width=10%%>Size</TH>");
	prints("<TH width=10%%>Date</TH><TH width=60%%>Description</TH></TR>\n");
	handle=opendir(file);
	while ((dentry=readdir(handle))!=NULL) {
		snprintf(file, sizeof(file)-1, "%s/%s%s", config.server_htdocs_dir, directory, dentry->d_name);
		fixslashes(file);
		stat(file, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if ((strcmp("..", dentry->d_name)==0)&&(strcmp("/files/", conn[sid].dat->in_RequestURI)==0)) continue;
		if (strcmp("..", dentry->d_name)==0) {
			prints("<TR BGCOLOR=#F0F0F0><TD COLSPAN=4><IMG SRC=/images/foldero.gif>");
			prints("<A HREF=%s/> Parent Directory</A></TD>\n", dentry->d_name);
			continue;
		}
		strftime(timebuf, sizeof(timebuf), "%b %d %Y %H:%M", localtime(&sb.st_mtime));
		memset(showfile, 0, sizeof(showfile));
		snprintf(showfile, sizeof(showfile)-1, "%s", dentry->d_name);
		prints("<TR BGCOLOR=#F0F0F0><TD ALIGN=left NOWRAP>");
		if (sb.st_mode & S_IFDIR) {
			prints("<IMG SRC=/images/folder.gif>&nbsp;<A HREF=");
			printhex("%s", showfile);
			prints("/>%s/</A></TD>", dentry->d_name);
		} else {
			prints("<IMG SRC=/images/default.gif>&nbsp;<A HREF=");
			printhex("%s", showfile);
			prints(">%s</A></TD>", dentry->d_name);
		}
		if (sb.st_size>1048576) {
			prints("<TD ALIGN=right NOWRAP>%10.1f M</TD>\n", (float)sb.st_size/1048576.0);
		} else {
			prints("<TD ALIGN=right NOWRAP>%10.1f K</TD>\n", (float)sb.st_size/1024.0);
		}
		prints("<TD ALIGN=right NOWRAP>%s</TD>\n", timebuf);
		prints("<TD ALIGN=left NOWRAP>&nbsp;</TD></TR>\n");
	}
	closedir(handle);
	prints("</TABLE>\n");
	prints("</CENTER>\n");
	prints("</BODY></HTML>\n");
	return 0;
}
