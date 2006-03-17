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

void decodeurl(unsigned char *pEncoded)
{
	char *pDecoded;

	pDecoded=pEncoded;
	while (*pDecoded) {
		if (*pDecoded=='+') *pDecoded=' ';
		pDecoded++;
	};
	pDecoded=pEncoded;
	while (*pEncoded) {
		if (*pEncoded=='%') {
			pEncoded++;
			if (isxdigit(pEncoded[0])&&isxdigit(pEncoded[1])) {
				*pDecoded++=(char)hex2int(pEncoded);
				pEncoded+=2;
			}
		} else {
			*pDecoded++=*pEncoded++;
		}
	}
	*pDecoded='\0';
}

void fixslashes(char *pOriginal)
{
#ifdef WIN32
	swapchar(pOriginal, '/', '\\');
#else
	swapchar(pOriginal, '\\', '/');
#endif
}

int hex2int(char *pChars)
{
	int Hi;
	int Lo;
	int Result;

	Hi=pChars[0];
	if ('0'<=Hi&&Hi<='9') {
		Hi-='0';
	} else if ('a'<=Hi&&Hi<='f') {
		Hi-=('a'-10);
	} else if ('A'<=Hi&&Hi<='F') {
		Hi-=('A'-10);
	}
	Lo = pChars[1];
	if ('0'<=Lo&&Lo<='9') {
		Lo-='0';
	} else if ('a'<=Lo&&Lo<='f') {
		Lo-=('a'-10);
	} else if ('A'<=Lo&&Lo<='F') {
		Lo-=('A'-10);
	}
	Result=Lo+(16*Hi);
	return (Result);
}

void striprn(char *string)
{
	while ((string[strlen(string)-1]=='\r')||(string[strlen(string)-1]=='\n')) {
		string[strlen(string)-1]='\0';
	}
}

void swapchar(char *string, char oldchar, char newchar)
{
 	while (*string) {
 		if (*string==oldchar) *string=newchar;
		string++;
	}
}

char *strcasestr(char *src, char *query)
{
	char *pToken;
	char Buffer[8192];
	char Query[64];
	int loop;

	if (strlen(src)==0) return NULL;
	memset(Buffer, 0, sizeof(Buffer));
	strncpy(Buffer, src, sizeof(Buffer)-1);
	strncpy(Query, query, sizeof(Query)-1);
	loop=0;
	while (Buffer[loop]) {
		Buffer[loop]=toupper(Buffer[loop]);
		loop++;
	}
	loop=0;
	while (Query[loop]) {
		Query[loop]=toupper(Query[loop]);
		loop++;
	}
	pToken=strstr(Buffer, Query);
	if (pToken!=NULL) {
		return src+(pToken-(char *)&Buffer);
	}
	return NULL;
}

char *strcatf(char *dest, const char *format, ...)
{
	char catbuffer[1024];
	va_list ap;

	memset(catbuffer, 0, sizeof(catbuffer));
	va_start(ap, format);
	vsnprintf(catbuffer, sizeof(catbuffer)-1, format, ap);
	va_end(ap);
	strcat(dest, catbuffer);
	return dest;
}

int printhex(const char *format, ...)
{
	char *hex="0123456789ABCDEF";
	unsigned char buffer[1024];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if ((buffer[offset]>32)&&(buffer[offset]<128)&&(buffer[offset]!='<')&&(buffer[offset]!='>')) {
			prints("%c", buffer[offset]);
		} else {
			prints("%%%c%c", hex[(unsigned int)buffer[offset]/16], hex[(unsigned int)buffer[offset]&15]);
		}
		offset++;
	}
	return 0;
}

int printht(const char *format, ...)
{
	unsigned char buffer[1024];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if (buffer[offset]=='<') {
			prints("&lt;");
		} else if (buffer[offset]=='>') {
			prints("&gt;");
		} else if (buffer[offset]=='&') {
			prints("&amp;");
		} else if (buffer[offset]=='"') {
			prints("&quot;");
		} else {
			prints("%c", buffer[offset]);
		}
		offset++;
	}
	return 0;
}
