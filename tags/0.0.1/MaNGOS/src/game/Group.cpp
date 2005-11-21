/* Group.cpp
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

#include "Common.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Chat.h"

void Group::ChangeLeader(const uint64 &guid)
{
    uint32 i;
    WorldPacket data;

    Player *player;

    for( i = 0; i < m_count; i++ )
    {
        if( m_members[i].guid == guid )
            break;
    }

    ASSERT( i < MAXGROUPSIZE );

    m_leaderGuid=guid;
    data.Initialize( SMSG_GROUP_SET_LEADER );
    data << m_members[i].name;

    for( i = 0; i < m_count; i++ )
    {
        player = objmgr.GetObject<Player>( m_members[i].guid );
        ASSERT( player );

        player->SetLeader(guid );
        player->GetSession()->SendPacket( &data );
    }

    SendUpdate();
}


void Group::Disband()
{
    uint32 i;
    WorldPacket data;
    Player *player;

    data.Initialize( SMSG_GROUP_DESTROYED );

    for( i = 0; i < m_count; i++ )
    {
        player = objmgr.GetObject<Player>( m_members[i].guid );
        ASSERT( player );

        player->UnSetInGroup();
        player->GetSession()->SendPacket( &data );
    }

}


void Group::SendUpdate()
{
    uint32 i ,j;
    Player *player;
    WorldPacket data;

    for( i = 0; i < m_count; i ++ )
    {
        player = objmgr.GetObject<Player>( m_members[i].guid );
        ASSERT( player );

        data.Initialize(SMSG_GROUP_LIST);
        data << (uint16)m_grouptype;
        data << uint32(m_count - 1);

        for( j = 0; j < m_count; j++ )
        {
            if (m_members[j].guid != m_members[i].guid)
            {
                data << m_members[j].name;
                data << (uint32)m_members[j].guid;
                data << uint32(0) << uint16(1);
            }
        }
        data << (uint64)m_leaderGuid;
        data << (uint8)m_lootMethod;
        data << (uint32)m_looterGuid;
        data << uint32(0);
        data << uint8(2);

        player->GetSession()->SendPacket( &data );
    }
}


uint32 Group::RemoveMember(const uint64 &guid)
{
    uint32 i, j;
    bool leaderFlag;

    for( i = 0; i < m_count; i++ )
    {
        if (m_members[i].guid == guid)
        {
            leaderFlag = m_members[i].guid == m_leaderGuid;

            for( j = i + 1; j < m_count; j++ )
            {
                m_members[j-1].guid = m_members[j].guid;
                m_members[j-1].name = m_members[j].name;
            }

            break;
        }
    }

    m_count--;

    if (m_count > 1 && leaderFlag)
        ChangeLeader(m_members[0].guid);

    return m_count;
}


void Group::BroadcastToGroup(WorldSession *session, std::string msg)
{
    if (session && session->GetPlayer())
    {
        for (int i = 0; i < m_count; i++)
        {
            WorldPacket data;
            sChatHandler.FillMessageData(&data, session, CHAT_MSG_PARTY, LANG_UNIVERSAL, NULL, msg.c_str());
            Player *pl = objmgr.GetObject<Player>(m_members[i].guid);
            if (pl && pl->GetSession())
                pl->GetSession()->SendPacket(&data);
        }
    }
}
