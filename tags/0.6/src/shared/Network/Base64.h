/**
 **	File ......... Base64.h
 **	Published ....  2004-02-13
 **	Author ....... grymse@alhem.net
 **/
/*
Copyright (C) 2004,2005  Anders Hedstrom

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
#ifndef _BASE64_H
#define _BASE64_H

#include <stdio.h>
#include <string>

class Base64
{
    public:
        Base64();

        void encode(FILE *, std::string& , bool add_crlf = true);
        void encode(const std::string&, std::string& , bool add_crlf = true);
        void encode(const char *, size_t, std::string& , bool add_crlf = true);
        void encode(unsigned char *, size_t, std::string& , bool add_crlf = true);

        void decode(const std::string&, std::string& );
        void decode(const std::string&, unsigned char *, size_t&);

        size_t decode_length(const std::string& );

    private:
        Base64(const Base64& ) {}
        Base64& operator=(const Base64& ) { return *this; }
        static  char const *bstr;
        static  char const rstr[128];
};
#endif                                            // _BASE64_H
