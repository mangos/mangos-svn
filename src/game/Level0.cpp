/* Level0.cpp
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

/////////////////////////////////////////////////
//  Normal User Chat Commands
//

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

bool ChatHandler::ShowHelpForCommand(ChatCommand *table, const char* cmd)
{
    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(!hasStringAbbr(table[i].Name, cmd))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        if(table[i].ChildCommands != NULL)
        {
            cmd = strtok(NULL, " ");
            if(cmd && ShowHelpForCommand(table[i].ChildCommands, cmd))
                return true;
        }

        if(table[i].Help == "")
        {
            WorldPacket data;
            FillSystemMessageData(&data, m_session, "There is no help for that command");
            m_session->SendPacket(&data);
            return true;
        }

        SendMultilineMessage(table[i].Help.c_str());

        return true;
    }

    return false;
}


bool ChatHandler::HandleHelpCommand(const char* args)
{
    ChatCommand *table = getCommandTable();
    WorldPacket data;

    if(!*args)
        return false;

    char* cmd = strtok((char*)args, " ");
    if(!cmd)
        return false;

    if(!ShowHelpForCommand(getCommandTable(), cmd))
    {
        FillSystemMessageData(&data, m_session, "There is no such command");
        m_session->SendPacket( &data );
    }

    return true;
}


bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ChatCommand *table = getCommandTable();
    WorldPacket data;

    FillSystemMessageData(&data, m_session, "Commands aviable to you:");
    m_session->SendPacket(&data);

    for(uint32 i = 0; table[i].Name != NULL; i++)
    {
        if(*args && !hasStringAbbr(table[i].Name, (char*)args))
            continue;

        if(m_session->GetSecurity() < table[i].SecurityLevel)
            continue;

        FillSystemMessageData(&data, m_session, table[i].Name);
        m_session->SendPacket(&data);
    }

    return true;
}


bool ChatHandler::HandleAcctCommand(const char* args)
{
    WorldPacket data;

    uint32 gmlevel = m_session->GetSecurity();    // get account level
    char buf[256];
    sprintf(buf, "Your account level is: %i", gmlevel);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );               // send message

    return true;
}


bool ChatHandler::HandleStartCommand(const char* args)
{
    Player *chr = m_session->GetPlayer();
    chr->SetUInt32Value(PLAYER_FARSIGHT, 0x01);   // what does that do?

    PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo(
        m_session->GetPlayer()->getRace(), m_session->GetPlayer()->getClass());
    ASSERT(info);

    smsg_NewWorld(m_session, info->mapId, info->positionX, info->positionY,
        info->positionZ);

    return true;
}


bool ChatHandler::HandleInfoCommand(const char* args)
{
    WorldPacket data;

    uint32 clientsNum = sWorld.GetSessionCount();
    char buf[256];

    // more info come.. right now only display users connected
    sprintf((char*)buf,"Number of users connected: %i", (int) clientsNum);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}


bool ChatHandler::HandleNYICommand(const char* args)
{
    WorldPacket data;
    char buf[256];

    sprintf((char*)buf,"Not yet Implamented");
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}


bool ChatHandler::HandleMountCommand(const char* args)
{
    WorldPacket data;

    // get level
    uint32 theLevel = m_session->GetPlayer( )->GetUInt32Value( UNIT_FIELD_LEVEL );
    uint16 mId=1147;
    float speed = (float)8;
    uint8 theRace = m_session->GetPlayer()->getRace();
    uint32 num=0;

    if (theLevel < 10 )
    {
    // If not level 10, then this text will be displayed
        FillSystemMessageData(&data, m_session, "You have to be atleast level 10 to use this command.");
        m_session->SendPacket( &data );
        return true;
    }
    else
    {
        char* pMount = strtok((char*)args, " ");
        if( pMount )
        {
            num = atoi(pMount);
            switch(num)
            {
                case 1:                           //nothing to do, min lvl mount lvl 10, lol
                    break;
                case 2:
                    if(theLevel<15) num=1;
                    break;
                case 3:
                    if(theLevel<20)
                        if(theLevel<15) num=1;
                    else
                        num=2;
                    break;
                default:
                    return true;
            }
        }
        else
        {
            if(theLevel>19)
                num=3;
            else
            if(theLevel>14)
                num=2;
            else
                num=1;
        }
        if (num > 2 )
        {
            switch(theRace)
            {
                case HUMAN:                       //HUMAN
                    mId=1147;
                    break;
                case ORC:                         //ORC
                    mId=295;
                    break;
                case DWARF:                       //DWARF
                    mId=1147;
                    break;
                case NIGHTELF:                    //NIGHT ELF
                    mId=479;
                    break;
                case UNDEAD_PLAYER:               //UNDEAD
                    mId=1147;                     //need to change
                    break;
                case TAUREN:                      //TAUREN
                    mId=295;
                    break;
                case GNOME:                       //GNOME
                    mId=1147;
                    break;
                case TROLL:                       //TROLL
                    mId=479;
                    break;
            }
        }
        else if (num > 1 )
        {
            switch(theRace)
            {
                case HUMAN:                       //HUMAN
                    mId=1531;
                    break;
                case ORC:                         //ORC
                    mId=207;                      //need to change
                    break;
                case DWARF:                       //DWARF
                    mId=2786;
                    break;
                case NIGHTELF:                    //NIGHT ELF
                    mId=720;
                    break;
                case UNDEAD_PLAYER:               //UNDEAD
                    mId=2346;
                    break;
                case TAUREN:                      //TAUREN
                    mId=180;
                    break;
                case GNOME:                       //GNOME
                    mId=1147;                     //need to change
                    break;
                case TROLL:                       //TROLL
                    mId=1340;
                    break;
            }
        }
        else
        {
            switch(theRace)
            {
                case HUMAN:                           //HUMAN
                    mId=236;
                    break;
                case ORC:                           //ORC
                    mId=207;
                    break;
                case DWARF:                       //DWARF
                    mId=2186;
                    break;
                case NIGHTELF:                    //NIGHT ELF
                    mId=632;
                    break;
                case UNDEAD_PLAYER:               //UNDEAD
                    mId=5050;
                    break;
                case TAUREN:                      //TAUREN
                    mId=1220;
                    break;
                case GNOME:                       //GNOME
                    mId=748;                      //need to change
                    break;
                case TROLL:                       //TROLL
                    mId=2320;
                    break;
            }
        }
    }

    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , mId);
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_FLAGS , 0x002000 );

    if(theLevel>25)
        speed = (float)theLevel;
    else
        speed = (float)theLevel-1;

    if(speed > 35)
        speed = 35;

    data.Initialize( SMSG_FORCE_RUN_SPEED_CHANGE );
    data << m_session->GetPlayer( )->GetUInt32Value( OBJECT_FIELD_GUID );
    data << m_session->GetPlayer( )->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << speed;
    WPAssert(data.size() == 12);
    m_session->GetPlayer( )->SendMessageToSet(&data, true);

    char cmount[256];
    sprintf(cmount, "You have a level %i mount at %i speed.", num, (int)speed);
    FillSystemMessageData(&data, m_session, cmount);
    m_session->SendPacket( &data );               // send message

    return true;
}


bool ChatHandler::HandleDismountCommand(const char* args)
{
    WorldPacket data;

    m_session->GetPlayer( )->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID , 0);
    m_session->GetPlayer( )->RemoveFlag( UNIT_FIELD_FLAGS, 0x002000 );

    // Remove the "player locked" flag, to allow movement
    if (m_session->GetPlayer( )->GetUInt32Value(UNIT_FIELD_FLAGS) & 0x000004 )
        m_session->GetPlayer( )->RemoveFlag( UNIT_FIELD_FLAGS, 0x000004 );

    m_session->GetPlayer( )->SetPlayerSpeed(RUN, 7.5, true);
    return true;
}


bool ChatHandler::HandleSaveCommand(const char* args)
{
    WorldPacket data;

    m_session->GetPlayer()->SaveToDB();
    FillSystemMessageData(&data, m_session, "Player saved.");
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleGMListCommand(const char* args)
{
    WorldPacket data;
    bool first = true;

#ifndef ENABLE_GRID_SYSTEM
    ObjectMgr::PlayerMap::const_iterator itr;
    for (itr = objmgr.Begin<Player>(); itr != objmgr.End<Player>(); itr++)
#else
    ObjectAccessor::PlayerMapType &m(ObjectAccessor::Instance().GetPlayers());
    for(ObjectAccessor::PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
#endif
    {
        if(itr->second->GetSession()->GetSecurity())
        {
            if(first)
            {
                FillSystemMessageData(&data, m_session, "There are following active GMs on this server:");
                m_session->SendPacket( &data );
            }

            FillSystemMessageData(&data, m_session, itr->second->GetName());
            m_session->SendPacket( &data );

            first = false;
        }
    }

    if(first)
    {
        FillSystemMessageData(&data, m_session, "There are no GMs currently logged in on this server.");
        m_session->SendPacket( &data );
    }

    return true;
}
