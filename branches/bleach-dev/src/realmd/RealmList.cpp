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

#include "Common.h"
#include "RealmList.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( RealmList );

RealmList::RealmList( )
{
}

RealmList::~RealmList( )
{
    for( RealmMap::iterator i = _realms.begin(); i != _realms.end(); i++ )
        delete i->second;

    _realms.clear( );
    /*  for( PatchMap::iterator i = _patches.begin(); i != _patches.end(); i++ )
          delete i->second;

      _patches.clear( );
    */
}

int RealmList::GetAndAddRealms(std::string dbstring)
{
    int count = 0;
    //QueryResult *result = dbRealmServer.PQuery( "SELECT `name`,`address`,`icon`,`color`,`timezone`, `dbstring` FROM `realmlist` ORDER BY `name`;" );
    QueryResult *result = dbRealmServer.Query( "SELECT `id`, `name`,`address`,`icon`,`color`,`timezone` FROM `realmlist` ORDER BY `name`;" );
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
                                                            //, fields[5].GetString());
            AddRealm(fields[0].GetUInt32(), fields[1].GetString(),fields[2].GetString(),fields[3].GetUInt8(), fields[4].GetUInt8(), fields[5].GetUInt8());
            count++;
        } while( result->NextRow() );
        delete result;
    }
    //if (_realms.size() == 0)
    //{
    //    sLog.outString( "Realm:***There are no valid realms specified in the database!  Working in localhost mode!***" );
    //    //AddRealm("localhost","127.0.0.1",1,0,1,dbstring.c_str());
    //    AddRealm("localhost","127.0.0.1",1,0,1);
    //}
    return count;
}

                                                            //, const char *dbstring )
void RealmList::AddRealm( uint32 ID, const char *name, const char *address, uint8 icon, uint8 color, uint8 timezone)
{
    if( _realms.find( name ) == _realms.end() )
    {
                                                            //, dbstring);
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
        std::string addr(address);

        if( addr.find(':', 0) == std::string::npos )
        {
            std::stringstream ss;
            ss << addr << ":" << DEFAULT_WORLDSERVER_PORT;
            addr = ss.str();
        }
        newRealm->address = addr;
        _realms[name] = newRealm;
        sLog.outString("Added realm \"%s\".", newRealm->name.c_str());
        //        }
    }
}

void RealmList::SetRealm( const char *name, uint8 icon, uint8 color, uint8 timezone )
{
    if( _realms.find( name ) != _realms.end( ) )
    {
        _realms[ name ]->icon = icon;
        _realms[ name ]->color = color;
        _realms[ name ]->timezone = timezone;
    }
}
