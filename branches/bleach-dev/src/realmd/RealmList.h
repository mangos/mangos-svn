/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

#include <ace/Singleton.h>
#include "Database/DatabaseEnv.h"

struct Realm
{

    std::string name;

    std::string address;

    uint8 icon;

    uint8 color;

    uint8 timezone;
	uint32 player_limit;
    uint32 m_ID;

    // Leave these db functions commented out in case we need to maintain connections
    // to the realm db's at some point.
    //    std::string m_dbstring;

    //    DatabaseMysql dbRealm;

                                                            //, std::string dbstring)
    Realm (uint32 ID, const char *Name, std::string Address, uint8 Icon, uint8 Color, uint8 Timezone, uint32 pl)
    {
        m_ID = ID;
        name = Name;
        address = Address;
        //        m_dbstring = dbstring;

        icon = Icon;
        color = Color;
        timezone = Timezone;
		player_limit = pl;
    }

    //    int dbinit()
    //    {
    //        return dbRealm.Initialize(m_dbstring.c_str());
    //    }

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

        void AddRealm( uint32 ID, const char *name, const char *address, uint8 icon, uint8 color, uint8 timezone, uint32 pl);
        int GetAndAddRealms(std::string dbstring);
        void SetRealm( const char *name, uint8 icon, uint8 color, uint8 timezone, uint32 pl);

        inline RealmMap::const_iterator begin() const { return _realms.begin(); }
        inline RealmMap::const_iterator end() const { return _realms.end(); }
        inline uint32 size() const { return _realms.size(); }

    private:
        RealmMap _realms;
};

typedef ACE_Singleton<RealmList, ACE_Recursive_Thread_Mutex> RealmListSingleton;
#define sRealmList RealmListSingleton::instance()

extern DatabaseMysql stDatabaseMysql;

#endif
