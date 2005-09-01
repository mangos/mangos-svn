/* RealmList.cpp
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

#include "Common.h"
#include "RealmList.h"
//Deadknight Addon
#include "Database/DatabaseEnv.h"
//Finish

createFileSingleton( RealmList );

RealmList::RealmList( )
{
}


RealmList::~RealmList( )
{
    for( RealmMap::iterator i = _realms.begin(); i != _realms.end(); i++ )
        delete i->second;

    _realms.clear( );
    for( PatchMap::iterator i = _patches.begin(); i != _patches.end(); i++ )
        delete i->second;

    _patches.clear( );
}


//Deadknight Addon
int RealmList::GetAndAddRealms()
{
    std::stringstream query;

// Format: Realm_Name, Realm_IP, Icon (0 = Normal, 1 = PVP), Color (0 = Yellow, 1 = Red), TimeZone (1 - 4)
    AddRealm( "** MaNGOS Local Dev **", "127.0.0.1", 0, 0, 1 );

//query << "SELECT name,address,population,type,color,language,online FROM realms";
    query << "SELECT name,address,icon,color,timezone FROM realms";
    QueryResult *result = sDatabase.Query( query.str().c_str() );
    if(result)
    {
        Field *fields = result->Fetch();
//Openw0w style realm
//FIXME:Online-Offline
/*if(!(NumChars = fields[6].GetUInt8()))
  {
  r->Color = 2;
  }*/

        AddRealm(fields[0].GetString(),fields[1].GetString(),fields[2].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt8());

        while( result->NextRow() )
        {
            Field *fields = result->Fetch();
//Openw0w style realm
//FIXME:Online-Offline
/*if(!(NumChars = fields[6].GetUInt8()))
  {
  r->Color = 2;
  }*/
            AddRealm(fields[0].GetString(),fields[1].GetString(),fields[2].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt8());
        }
        delete result;
        return 1;
    }
    else
    {
        sLog.outString( "Realm:***There is no realm defined in database!Working at localhost mode!***" );
        return 0;
    }
}


//Finish

void RealmList::AddRealm( const char * name, const char * address, uint8 icon, uint8 color, uint8 timezone )
{
    RemoveRealm( name );

//_realms[ name ] = new Realm( );
    _realms[ name ] = new Realm( name, address, icon, color , timezone);

    std::string addr(address);
    if( addr.find(':', 0) == std::string::npos )
    {
        std::stringstream ss;
        ss << addr << ":" << i_serverPort;
        addr = ss.str();
    }

    _realms[ name ]->address = addr;
    _realms[ name ]->icon = icon;
    _realms[ name ]->color = color;
    _realms[ name ]->timezone = timezone;
}


void RealmList::SetRealm( const char * name, uint8 icon, uint8 color, uint8 timezone )
{
    if( _realms.find( name ) != _realms.end( ) )
    {
        _realms[ name ]->icon = icon;
        _realms[ name ]->color = color;
        _realms[ name ]->timezone = timezone;
    }
}


void RealmList::RemoveRealm( const char * name )
{
    if( _realms.find( name ) != _realms.end( ) )
    {
        delete _realms[ name ];
        _realms.erase( name );
    }
}
