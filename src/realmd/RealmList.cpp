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

/** \file
    \ingroup realmd
*/

#include "Common.h"
#include "RealmList.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseEnv.h"
#include "SystemConfig.h"

INSTANTIATE_SINGLETON_1( RealmList );

extern DatabaseMysql dbRealmServer;

RealmList::RealmList( )
{
}

/// Destroys the internal realm map
RealmList::~RealmList( )
{
    for( RealmMap::iterator i = _realms.begin(); i != _realms.end(); i++ )
        delete i->second;

    _realms.clear( );
}

/// Load the realm list from the database
void RealmList::GetAndAddRealms(std::string dbstring)
{
    ///- Get the content of the realmlist table in the database
    QueryResult *result = dbRealmServer.Query( "SELECT `id`, `name`,`address`,`port`,`icon`,`color`,`timezone` FROM `realmlist` ORDER BY `name`" );

    ///- Circle through results and add them to the realm map
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            AddRealm(fields[0].GetUInt32(), fields[1].GetString(),fields[2].GetString(),fields[3].GetUInt32(),fields[4].GetUInt8(), fields[5].GetUInt8(), fields[6].GetUInt8());
        } while( result->NextRow() );
        delete result;
    }
    //if (_realms.size() == 0)
    //{
    //    sLog.outString( "Realm:***There are no valid realms specified in the database!  Working in localhost mode!***" );
    //    //AddRealm("localhost","127.0.0.1",1,0,1,dbstring.c_str());
    //    AddRealm("localhost","127.0.0.1",1,0,1);
    //}
    return;
}

                                                            //, const char *dbstring )
/// Add a Realm to the realm list
void RealmList::AddRealm( uint32 ID, const char *name, const char *address, uint32 port, uint8 icon, uint8 color, uint8 timezone)
{
    if( _realms.find( name ) == _realms.end() )
    {
                                                            //, dbstring);
        ///- If no Realm with same name exists, create it
        Realm *newRealm = new Realm(ID, name, address, icon, color, timezone);

        //        sLog.outString("Realm \"%s\", database \"%s\"", newRealm->name.c_str(), newRealm->m_dbstring.c_str() );
        //        if (dbstring[0] == 0) {
        //            sLog.outError("No dbstring specified, skipping realm \"%s\".", newRealm->name);
        //            return;
        //        }
        //
        //        if(!newRealm->dbinit())
        //        {
        //            sLog.outError("Cannot connect to database, skipping realm");
        //        }
        //        else
        //        {

        ///- Append port to IP address.
        std::string addr(address);

        std::ostringstream ss;
        ss << addr << ":" << port;
        addr = ss.str();

        newRealm->address = addr;

        ///- Add the new Realm to the list
        _realms[name] = newRealm;
        sLog.outString("Added realm \"%s\".", newRealm->name.c_str());
        //        }
    }
}
