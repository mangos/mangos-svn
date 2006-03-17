/* DBC.h
 *
 * Copyright (C) 2004 Wow Daemon
 * Copyright (C) 2005 MaNGOS <https://opensvn.csie.org/traccgi/MaNGOS/trac.cgi/>
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

#ifndef __DBC_H
#define __DBC_H

#include <stdio.h>

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include <windows.h>
#else
#include <string.h>
#define MAX_PATH 1024
#define __fastcall __attribute__((__fastcall__))
#endif

enum DBCFmat
{
    F_STRING = 0,
    F_INT = 1,
    F_FLOAT = 2,
    F_NADA = 3
};

class DBC
{
    int rows, cols, dblength,weird2;              // Weird2 = most probably line length
    unsigned int* tbl;
    char* db,name[MAX_PATH];
    bool loaded;
    DBCFmat *format;
    public:
        DBC();
        void Load(const char *filename);
        void CSV(char *filename, bool info = false);
        void GuessFormat();
        DBCFmat GuessFormat(int row, int col);
        void FormatCSV(const char *filename, bool info = false);
        void Lookup(char* out, int row, int col,char isstr=0,bool onlystr=false);
        void LookupFormat(char* out, int row, int col);
        void RowToStruct(void* out, int row);
        bool IsLoaded() { return loaded; }
        void* __fastcall GetRow(unsigned const int index) { return (void *)&tbl[index*cols]; }
        char* __fastcall LookupString(unsigned const int offset) { return db+offset; }
        int GetRows() { return rows; }
        int GetCols() { return cols; }
        int GetDBSize() { return dblength; }
        ~DBC();
};
#endif
