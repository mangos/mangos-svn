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

#ifndef _REALMLIST_H
#define _REALMLIST_H



#include "Network/socket_include.h"
#include "Policies/Singleton.h"

struct Realm
{
    
    const char *name;
    
    std::string address;
    
    
    
    uint32 icon;
    
    uint8 color;
    
    uint8 timezone;
    
    

    Realm (const char *Name, std::string Address, uint32 Icon, uint8 Color, uint8 Timezone)
    {
        name = Name;
        address = Address;
        
        icon = Icon;
        color = Color;
        timezone = Timezone;
        
    }

    ~Realm ()
    {
    }
};

class RealmList 
{
    public:
        typedef std::map<std::string, Realm*> RealmMap;

        RealmList();
        ~RealmList();

        void AddRealm( const char * name, const char * address, uint8 icon, uint8 color, uint8 timezone );
        int GetAndAddRealms();
        void SetRealm( const char * name, uint8 icon, uint8 color, uint8 timezone );
        void RemoveRealm (const char * name );
        inline void setServerPort(port_t p) { i_serverPort = p; }

        RealmMap::const_iterator begin() const { return _realms.begin(); }
        RealmMap::const_iterator end() const { return _realms.end(); }
        uint32 size() const { return _realms.size(); }

    private:
        RealmMap _realms;
/*
        struct Patch
        {
            uint8 Hash[16];
            char Platform[4];
        };

        //typedef std::map <uint32, Patch*> PatchMap;
   //     PatchMap _patches;


   */
        port_t i_serverPort;
};

#define sRealmList MaNGOS::Singleton<RealmList>::Instance()
#endif
