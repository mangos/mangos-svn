/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

/// \addtogroup mangosd
/// @{
/// \file

#ifndef __CLIRUNNABLE_H
#define __CLIRUNNABLE_H

typedef int(* pPrintf)(const char*,...);
typedef void(* pCliFunc)(char *,pPrintf);

/// Command Template class
struct CliCommand
{
    char const * cmd;
    pCliFunc Func;
    char const * description;
};

/// Storage class for commands issued for delayed execution
class CliCommandHolder
{
    private:
        const CliCommand *cmd;
        char *args;
        pPrintf zprintf;
    public:
        CliCommandHolder(const CliCommand *command, const char *arguments, pPrintf zprintf) 
            : cmd(command), zprintf(zprintf)
        {
            args = new char[strlen(arguments)+1];
            strcpy(args, arguments);
        }
        ~CliCommandHolder() { delete[] args; }
        void Execute() const { cmd->Func(args, zprintf); }
};

/// Command Line Interface handling thread
class CliRunnable : public ZThread::Runnable
{
    public:
        void run();
};
#endif
/// @}
