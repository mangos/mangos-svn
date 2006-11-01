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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "SpellAuras.h"

void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
    WorldPacket data;
    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    uint32 CurTalentPoints =  GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS1);
    if(CurTalentPoints == 0)
        return;

    if (requested_rank > 4)
        return;

    TalentEntry *talentInfo = sTalentStore.LookupEntry( talent_id );

    if(!talentInfo)
        return;

    Player * player = GetPlayer();

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        TalentEntry *depTalentInfo = sTalentStore.LookupEntry(talentInfo->DependsOn);
        bool hasEnoughRank = false;
        for (int i = talentInfo->DependsOnRank; i <= 4; i++)
        {
            if (depTalentInfo->RankID[i] != 0)
                if (player->HasSpell(depTalentInfo->RankID[i]))
                    hasEnoughRank = true;
        }
        if (!hasEnoughRank)
            return;
    }

    // Find out how many points we have in this field
    uint32 spentPoints = 0;

    uint32 tTree = talentInfo->TalentTree;
    if (talentInfo->Row > 0)
    {
        unsigned int numRows = sTalentStore.GetNumRows();
        for (unsigned int i = 0; i < numRows; i++)          // Loop through all talents.
        {
            // Someday, someone needs to revamp
            TalentEntry *tmpTalent = sTalentStore.data[i];
            if (tmpTalent)                                  // the way talents are tracked
            {
                if (tmpTalent->TalentTree == tTree)
                {
                    for (int j = 0; j <= 4; j++)
                    {
                        if (tmpTalent->RankID[j] != 0)
                        {
                            if (player->HasSpell(tmpTalent->RankID[j]))
                            {
                                spentPoints += j + 1;
                            }
                        }
                    }
                }
            }
        }
    }

    uint32 spellid = talentInfo->RankID[requested_rank];
    if( spellid == 0 )
    {
        sLog.outDetail("Talent: %u Rank: %u = 0", talent_id, requested_rank);
    }
    else
    {
        if(spentPoints < (talentInfo->Row * 5))             // Min points spent
        {
            return;
        }

        if(!(GetPlayer( )->HasSpell(spellid)))
        {
            if(!GetPlayer( )->addSpell((uint16)spellid,1))
                return;

            data.Initialize(SMSG_LEARNED_SPELL);
            sLog.outDetail("TalentID: %u Rank: %u Spell: %u\n", talent_id, requested_rank, spellid);
            data << spellid;
            GetPlayer( )->GetSession()->SendPacket(&data);

            SpellEntry *spellInfo = sSpellStore.LookupEntry( spellid );
            assert(spellInfo);                              // checked in addSpell

            for(uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effect[i];
                if (eff>=TOTAL_SPELL_EFFECTS)
                    continue;

                // Duration 21 = permanent
                if ((eff == 6) && (spellInfo->DurationIndex == 21) && (spellInfo->rangeIndex == 1))
                {
                    Aura *Aur = new Aura(spellInfo, i, GetPlayer());
                    GetPlayer()->AddAura(Aur);
                }
            }

            if(requested_rank > 0 )
            {
                uint32 respellid = talentInfo->RankID[requested_rank-1];
                GetPlayer( )->removeSpell((uint16)respellid);
                GetPlayer()->RemoveAurasDueToSpell(respellid);
            }
            GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS1, CurTalentPoints - 1);
        }
    }
}

void WorldSession::HandleTalentWipeOpcode( WorldPacket & recv_data )
{
    sLog.outString("MSG_TALENT_WIPE_CONFIRM");
    recv_data.hexlike();
    uint64 GUID;
    recv_data >> GUID;
    Player * player = GetPlayer();
    if(player->GetGUID()==GUID)
    {
        if(!(player->removeTalent()))
        {
            WorldPacket data;
            data.Initialize( MSG_TALENT_WIPE_CONFIRM );     //you have not any talent
            SendPacket( &data );
            return;
        }
        // send spell 14867
        WorldPacket data;
        data.Initialize(SMSG_SPELL_START );
        data << uint8(0xFF) << GUID << uint8(0xFF) << GUID << uint16(14867);
        data << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
        SendPacket( &data );

        data.Initialize(SMSG_SPELL_GO);
        data << uint8(0xFF) << GUID << uint8(0xFF) << GUID << uint16(14867);
        data << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << GUID;
        data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
        SendPacket( &data );
    }
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket & recv_data)
{
    uint32 skill_id;
    recv_data >> skill_id;
    GetPlayer()->SetSkill(skill_id, 0, 0);
}
