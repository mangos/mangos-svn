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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectDefines.h"

#if defined( _VERSION_1_7_0_ ) || defined( _VERSION_1_8_0_ )

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    
    uint64 guid;
    recv_data >> guid;

    
    DEBUG_LOG( "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid) );
    
    if( IS_CREATURE_GUID(guid) )
    {
	Creature *pEnemy = ObjectAccessor::Instance().GetCreature(*_player, guid);
	if( pEnemy != NULL )
	{
	    assert( pEnemy != NULL );
	    _player->addStateFlag(UF_ATTACKING);
	    _player->smsg_AttackStart(pEnemy);
	    pEnemy->AI().AttackStart(_player);
	    _player->inCombat = true;
	    _player->logoutDelay = LOGOUTDELAY;
	    return;
	}
    }
    else if( IS_PLAYER_GUID(guid) )
    {
	Player *pPVPEnemy = ObjectAccessor::Instance().GetPlayer(*_player, guid);
	if( pPVPEnemy != NULL )
	{
	    _player->addStateFlag(UF_ATTACKING);
	    _player->smsg_AttackStart(pPVPEnemy);
	    _player->inCombat = true;
	    _player->logoutDelay = LOGOUTDELAY;
	    return;
	}
    }

    Log::getSingleton( ).outError( "WORLD: Enemy %u %.8X is not a player or a creature",
				   GUID_LOPART(guid), GUID_HIPART(guid));	
}

#else 

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid;
    recv_data >> guid;

    
    Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid) );
    Creature *pEnemy = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if(!pEnemy)
    {
        Log::getSingleton( ).outError( "WORLD: %u %.8X is not a creature",
            GUID_LOPART(guid), GUID_HIPART(guid));
        return;                                     
    }

    Player *pThis = GetPlayer();
    pThis->addStateFlag(UF_ATTACKING);
    pThis->smsg_AttackStart(pEnemy, pThis);

    pThis->inCombat = true;
    pThis->logoutDelay = LOGOUTDELAY;
}

#endif 

void WorldSession::HandleAttackStopOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint64 guid = GetPlayer()->GetSelection();

    GetPlayer()->smsg_AttackStop(guid);
    GetPlayer()->clearStateFlag(UF_ATTACKING);
}

void WorldSession::HandleSetSheathedOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
	uint64 guid = GetPlayer()->GetGUID();
	uint32 sheathed;
    recv_data >> sheathed;

	
	Log::getSingleton( ).outDebug( "WORLD: Recvd CMSG_SETSHEATHED Message guidlow:%u guidhigh:%u value1:%u", GUID_LOPART(guid), GUID_HIPART(guid), sheathed );

	GetPlayer()->SetSheath(sheathed);
}

