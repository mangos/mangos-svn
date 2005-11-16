/* SkillHandler.cpp
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
}
*/


void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    TalentEntry *talentInfo = sTalentStore.LookupEntry( talent_id );

    uint32 CurTalentPoints =  GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS1);
    if(CurTalentPoints == 0)
    {
        // NO TALENT POINTS
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
                // Send data if all OK
                data.Initialize(SMSG_LEARNED_SPELL);
                Log::getSingleton( ).outDetail("TalentID: %d Rank: %d Spell: %d\n", talent_id, requested_rank, spellid);
                data << spellid;
                GetPlayer( )->GetSession()->SendPacket(&data);
                GetPlayer( )->addSpell((uint16)spellid);
            }

            // Update Talent Points
            GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS1, CurTalentPoints - 1);
        }
    }
}

/*0:02:15 - CLIENT >>> OpCode=0x251 CMSG_LEARN_TALENT, size=14

      0- 1- 2- 3- 4- 5- 6- 7- | 8- 9- A- B- C- D- E- F- | 01234567 89ABCDEF
0000: 00#0C#51#02#00#00 E1 01 | 00 00 00 00 00 00       | ..Q..... ......  


0:02:15 - SERVER >>> OpCode=0x266 SMSG_SET_FLAT_SPELL_MODIFIER, size=10

      0- 1- 2- 3- 4- 5- 6- 7- | 8- 9- A- B- C- D- E- F- | 01234567 89ABCDEF
0000: 00#08#66#02 0D 0B 0C FE | FF FF                   | ..f..... ..      


0:02:15 - SERVER >>> OpCode=0x12B SMSG_LEARNED_SPELL, size=8

      0- 1- 2- 3- 4- 5- 6- 7- | 8- 9- A- B- C- D- E- F- | 01234567 89ABCDEF
0000: 00#06#2B#01 A9 3B 00 00 |                         | ..+..;..   */      
