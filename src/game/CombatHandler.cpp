/* CombatHandler.cpp
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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#endif

#if defined( _VERSION_1_7_0_ ) || defined( _VERSION_1_8_0_ )

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    //WorldPacket data;
    uint64 guid;
    recv_data >> guid;

    // AttackSwing
    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid) );

#ifndef ENABLE_GRID_SYSTEM
	Creature *pEnemy = objmgr.GetObject<Creature>(guid);
	//Unit *pEnemy = objmgr.GetObject<Creature>(guid);
    Player *pPVPEnemy = objmgr.GetObject<Player>(guid);
#else
	Creature *pEnemy = ObjectAccessor::Instance().GetCreature(*_player, _player->GetSelection()/*guid*/);
	//Unit *pEnemy = ObjectAccessor::Instance().GetCreature(*_player, _player->GetSelection()/*guid*/);
	//Unit *pEnemy = ObjectAccessor::Instance().GetUnit(*_player, guid);
    Player *pPVPEnemy = ObjectAccessor::Instance().GetPlayer(*_player, _player->GetSelection()/*guid*/);
#endif
    
    if(pEnemy)
    {
        Player *pThis = GetPlayer();

        pThis->addStateFlag(UF_ATTACKING);
        pThis->smsg_AttackStart(pEnemy, pThis);
        pThis->inCombat = true;
        pThis->logoutDelay = LOGOUTDELAY;
    }
    else if(pPVPEnemy)
    {
        Player *pThis = GetPlayer();

        pThis->addStateFlag(UF_ATTACKING);
        pThis->smsg_AttackStart(pPVPEnemy, pThis);
        pThis->inCombat = true;
        pThis->logoutDelay = LOGOUTDELAY;
    }
    else
    {
        Log::getSingleton( ).outError( "WORLD: Enemy %u %.8X is not a player or a creature",
            GUID_LOPART(guid), GUID_HIPART(guid));

        return;
    }
}

#else //!defined( _VERSION_1_7_0_ ) && !defined( _VERSION_1_8_0_ )

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    recv_data >> guid;

    // AttackSwing
    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid) );
#ifndef ENABLE_GRID_SYSTEM
    Creature *pEnemy = objmgr.GetObject<Creature>(guid);
#else
    Creature *pEnemy = ObjectAccessor::Instance().GetCreature(*_player, guid);
#endif
    if(!pEnemy)
    {
        Log::getSingleton( ).outError( "WORLD: %u %.8X is not a creature",
            GUID_LOPART(guid), GUID_HIPART(guid));
        return;                                     // we do not attack PCs for now
    }

    Player *pThis = GetPlayer();
    pThis->addStateFlag(UF_ATTACKING);
    pThis->smsg_AttackStart(pEnemy, pThis);

    pThis->inCombat = true;
    pThis->logoutDelay = LOGOUTDELAY;
}

#endif //!defined( _VERSION_1_7_0_ ) && !defined( _VERSION_1_8_0_ )


void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid = GetPlayer()->GetSelection();

    GetPlayer()->smsg_AttackStop(guid);

    GetPlayer()->clearStateFlag(UF_ATTACKING);
    // GetPlayer()->removeCreatureFlag(0x00080000);
}

