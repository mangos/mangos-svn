/* RealmList.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef _REALMLIST_H
#define _REALMLIST_H

/*struct Realm
{
    std::string address;
    uint8 icon;
    uint8 color;
    uint8 timezone;
};*/

#include "Network/socket_include.h"

struct Realm
{
/// Realm name
    const char *name;
/// Server address (x.x.x.x:port)
    std::string address;
/// Low/Medium/High, absolute values do not matter, everything is relative
//float population;
/// 0 - Normal, 1 - PvP, 2 - Offline
    uint32 icon;
/// 0 - Orange, 1 - Red, 2 - Disabled
    uint8 color;
/// 0,1 - English, 2 - German, 3 - French, 4 - Other
    uint8 timezone;
/// Number of characters owned by the client on this server
//uint8 numChars;

/*Realm (const char *Name, const char *Address, float population, uint32 icon,
       uint8 color, uint8 language)*/
    Realm (const char *Name, std::string Address, uint32 Icon, uint8 Color, uint8 Timezone)
    {
        name = Name;
        address = Address;
//population = Population;
        icon = Icon;
        color = Color;
        timezone = Timezone;
//numChars = 0;
    }

    ~Realm ()
    {
    }
};

class RealmList : public Singleton<RealmList>
{
    public:
        typedef std::map<std::string, Realm*> RealmMap;

        RealmList();
        ~RealmList();

        void AddRealm( const char * name, const char * address, uint8 icon, uint8 color, uint8 timezone );
//Deadknight Addon
        int GetAndAddRealms();
//Finish
        void SetRealm( const char * name, uint8 icon, uint8 color, uint8 timezone );
        void RemoveRealm (const char * name );
        inline void setServerPort(port_t p) { i_serverPort = p; }

        RealmMap::const_iterator begin() const { return _realms.begin(); }
        RealmMap::const_iterator end() const { return _realms.end(); }
        uint32 size() const { return _realms.size(); }

    private:
        RealmMap _realms;

        struct Patch
        {
            uint8 Hash[16];
            char Platform[4];
        };

        typedef std::map <uint32, Patch*> PatchMap;
        PatchMap _patches;
        port_t i_serverPort;
};

#define sRealmList RealmList::getSingleton()
#endif
