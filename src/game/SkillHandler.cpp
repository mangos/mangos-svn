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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"








void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    TalentEntry *talentInfo = sTalentStore.LookupEntry( talent_id );

    uint32 CurTalentPoints =  GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS1);
    if(CurTalentPoints == 0)
    {
        
    }
    else
    {
        uint32 spellid = talentInfo->RankID[requested_rank];
        if( spellid == 0 || requested_rank > 4)
        {
            sLog.outDetail("Talent: %d Rank: %d = 0", talent_id, requested_rank);
        }
        else
        {
            if(!(GetPlayer( )->HasSpell(spellid)))
            {
                
                data.Initialize(SMSG_LEARNED_SPELL);
                sLog.outDetail("TalentID: %d Rank: %d Spell: %d\n", talent_id, requested_rank, spellid);
                data << spellid;
                GetPlayer( )->GetSession()->SendPacket(&data);
                GetPlayer( )->addSpell((uint16)spellid);

                if(requested_rank > 0 )
                {
                    uint32 respellid = talentInfo->RankID[requested_rank-1];
                    data.Initialize(SMSG_REMOVED_SPELL);
                    data << respellid;
                    GetPlayer( )->GetSession()->SendPacket(&data);
                    GetPlayer( )->removeSpell((uint16)respellid);
                }
            }

            
            GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS1, CurTalentPoints - 1);
        }
    }
}

    
