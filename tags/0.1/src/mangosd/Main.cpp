/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Config/ConfigEnv.h"
#include "Log.h"
#include "Master.h"

#include <iostream>

uint8 loglevel = DEFAULT_LOG_LEVEL;

int usage(const char *prog)
{
    std::cerr << "Usage: " << prog << std::endl;
    std::cerr << "\t" << "-c: config_file [" << _MANGOSD_CONFIG << "]" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{

    std::string cfg_file = _MANGOSD_CONFIG;
    int c = 1;
    while( c < argc )
    {
    const char *tmp = argv[c];
    if( *tmp == '-' && std::string(tmp +1) == "c" )
    {
        if( ++c >= argc )
        {
        std::cerr << "Runtime-Error: -c option requires an input argument" << std::endl;
        usage(argv[0]);
        }
        else
        cfg_file = argv[c];
    }
    else
    {
        std::cerr << "Runtime-Error: unsupported option " << tmp << std::endl;
        usage(argv[0]);
    }
    ++c;
    }

     
    if (!Config::getSingleton().SetSource(cfg_file.c_str()) )
    {
        sLog.outError("\nCould not find configuration file %s.", cfg_file.c_str());
    }

    Master::getSingleton().Run();
    return 0;
}
