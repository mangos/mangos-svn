/* SkillHandler.cpp
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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"

//////////////////////////////////////////////////////////////
/// This function handles CMSG_SKILL_LEVELUP
//////////////////////////////////////////////////////////////

/*void WorldSession::HandleSkillLevelUpOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 slot, skill_id, amount, current_points, current_skill, points;
    recv_data >> slot >> skill_id >> amount;
    current_points = GetPlayer( )->GetUInt32Value( PLAYER_SKILL_INFO_1_1+slot+1 );
    current_skill = GetPlayer( )->GetUInt32Value( PLAYER_SKILL_INFO_1_1+slot );
    points = GetPlayer( )->GetUInt32Value( PLAYER_CHARACTER_POINTS2 );
    GetPlayer( )->SetUInt32Value( PLAYER_SKILL_INFO_1_1+slot , ( 0x000001a1 ));
    GetPlayer( )->SetUInt32Value( PLAYER_SKILL_INFO_1_1+slot+1 , ( (current_points & 0xffff) + (amount << 16) ) );
    GetPlayer( )->SetUInt32Value( PLAYER_CHARACTER_POINTS2, points-amount );
GetPlayer( )->UpdateObject( );
}*/

void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    TalentEntry *talentInfo = sTalentStore.LookupEntry( talent_id );

    uint32 CurTalentPoints =  GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS1);
    if(CurTalentPoints == 0)
    {
//NO TALENT POINTS
    }
    else
    {
        uint32 spellid = talentInfo->RankID[requested_rank];
        if( spellid == 0 || requested_rank > 4)
        {
            Log::getSingleton( ).outDetail("Talent: %d Rank: %d = 0", talent_id, requested_rank);
        }
        else
        {
            if(!(GetPlayer( )->HasSpell(spellid)))
            {
//Send data if all OK
                data.Initialize(SMSG_LEARNED_SPELL);
                Log::getSingleton( ).outDetail("TalentID: %d Rank: %d Spell: %d\n", talent_id, requested_rank, spellid);
                data << spellid;
                GetPlayer( )->GetSession()->SendPacket(&data);
                GetPlayer( )->addSpell((uint16)spellid);
            }

//Update Talent Points
            GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS1, CurTalentPoints - 1);
        }
    }
}
