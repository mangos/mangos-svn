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
/****************************************************************************
 *	dorequest()
 *
 *	Purpose	: Authenticate and direct the request to the correct function
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void dorequest(int sid)
{
	unsigned char file[255];

	if (read_header(sid)<0) {
		closeconnect(sid, 1);
		return;
	}
	logaccess(2, "%s - HTTP Request: %s %s", conn[sid].dat->in_RemoteAddr, conn[sid].dat->in_RequestMethod, conn[sid].dat->in_RequestURI);
	snprintf(file, sizeof(file)-1, "%s%s", config.server_htdocs_dir, conn[sid].dat->in_RequestURI);
	snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "text/html");
	if (strncmp(conn[sid].dat->in_RequestURI, "/cgi-bin/",    9)==0) {
		cgi_main();
	} else if (sendfile(sid, file)==0) {
		return;
	} else if (dirlist(sid)==0) {
		return;
	} else {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		prints("<BR><CENTER>The file or function '");
		printht("%s", conn[sid].dat->in_RequestURI);
		prints("' could not be found.</CENTER>\n");
		logerror("%s - Incorrect function call '%s'", conn[sid].dat->in_RemoteAddr, conn[sid].dat->in_RequestURI);
	}
	return;
}

#ifdef WIN32
/****************************************************************************
 *	WinMain()
 *
 *	Purpose	: Program entry point (Win32), thread handling and window management
 *	Args	: Command line parameters (if any)
 *	Returns	: Exit status of program
 *	Notes	: None
 ***************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	snprintf(program_name, sizeof(program_name)-1, "%s", GetCommandLine());
	init();
	if (_beginthread(WSAReaper, 0, NULL)==-1) {
		logerror("Winsock reaper thread failed to start");
		exit(0);
	}
	accept_loop(NULL);
	return 0;
}
#else
/****************************************************************************
 *	main()
 *
 *	Purpose	: Program entry point (UNIX) and call accept loop
 *	Args	: Command line parameters (if any)
 *	Returns	: Exit status of program
 *	Notes	: None
 ***************************************************************************/
int main(int argc, char *argv[])
{
	snprintf(program_name, sizeof(program_name)-1, "%s", argv[0]);
	init();
	accept_loop(NULL);
	return 0;
}

#endif

