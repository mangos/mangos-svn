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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "UpdateData.h"
#include "Chat.h"
#include "MapManager.h"
#include "FactionTemplateResolver.h"
#include "EventSystem.h"

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{

    uint64 guid;

    recvPacket >> guid;

    Player *pl;
    Player *plTarget;

    pl     = GetPlayer();
    plTarget = pl->m_pDuel;

    if(pl->GetGUID() != plTarget->GetGUID())
    {

        sLog.outDebug( "WORLD: received CMSG_DUEL_ACCEPTED" );
        DEBUG_LOG("Player 1 is: %lu", (unsigned long)pl->GetGUID());
        DEBUG_LOG("Player 2 is: %lu", (unsigned long)plTarget->GetGUID());

        //Set players team
        pl->SetUInt32Value(PLAYER_DUEL_TEAM, 1);
        plTarget->SetUInt32Value(PLAYER_DUEL_TEAM, 2);

        #if 1
        //******************************* TEMPORARY *********************************
        //TODO: Set PvP ON and OFF to players is a little magic to Duel System works
        //      It is not the right way to do... We need to fix it! :D
        //
        //Set players factions. These factios are into factionstemplate.dbc
        pl->setFaction(-1, BLUE_TEAM);                      //Blue faction
        plTarget->setFaction(-1, RED_TEAM);                 //Red faction

        pl->StorePvpState();
        plTarget->StorePvpState();

        pl->SetPvP(false);
        plTarget->SetPvP(true);
        //******************************* TEMPORARY *********************************
        #endif

        pl->SetInDuel(true);
        plTarget->SetInDuel(true);

        WorldPacket data;

        data.Initialize(SMSG_DUEL_COUNTDOWN);
        data << (uint32)3000;                               // 3 seconds
        pl->GetSession()->SendPacket(&data);
        plTarget->GetSession()->SendPacket(&data);

    }

}

void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{

    sLog.outDebug( "WORLD: received CMSG_DUEL_CANCELLED" );

    uint64 guid;
    Player *pl;
    Player *plTarget;
    WorldPacket data;

    recvPacket >> guid;

    pl       = GetPlayer();
    plTarget = pl->m_pDuel;

    data.Initialize(SMSG_GAMEOBJECT_DESPAWN_ANIM);
    data << (uint64)guid;
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DESTROY_OBJECT);
    data << (uint64)guid;
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_DUEL_COMPLETE);
    data << (uint8)0;
    pl->GetSession()->SendPacket(&data);
    plTarget->GetSession()->SendPacket(&data);

    pl->m_isInDuel = false;
    plTarget->m_isInDuel = false;

    GameObject* obj = NULL;
    if( pl )
        obj = ObjectAccessor::Instance().GetGameObject(*pl, guid);

    if(obj)
        MapManager::Instance().GetMap(obj->GetMapId())->Remove(obj,true);

    pl->SetUInt64Value(PLAYER_DUEL_ARBITER,0);
    plTarget->SetUInt64Value(PLAYER_DUEL_ARBITER,0);
    pl->SetUInt32Value(PLAYER_DUEL_TEAM,0);
    plTarget->SetUInt32Value(PLAYER_DUEL_TEAM,0);

}
