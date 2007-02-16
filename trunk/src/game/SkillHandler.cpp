/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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
    CHECK_PACKET_SIZE(recv_data,4+4);

    uint32 talent_id, requested_rank;
    recv_data >> talent_id >> requested_rank;

    uint32 CurTalentPoints =  GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS1);
    if(CurTalentPoints == 0)
        return;

    if (requested_rank > 4)
        return;

    TalentEntry const *talentInfo = sTalentStore.LookupEntry( talent_id );

    if(!talentInfo)
        return;

    TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );

    if(!talentTabInfo)
        return;

    Player * player = GetPlayer();

    // prevent learn talent for different class (cheating)
    if( (player->getClassMask() & talentTabInfo->ClassMask) == 0 )
        return;

    // prevent skip talent ranks (cheating)
    if(requested_rank > 0 && !player->HasSpell(talentInfo->RankID[requested_rank-1]))
        return;

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        TalentEntry const *depTalentInfo = sTalentStore.LookupEntry(talentInfo->DependsOn);
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

    uint32 tTab = talentInfo->TalentTab;
    if (talentInfo->Row > 0)
    {
        unsigned int numRows = sTalentStore.GetNumRows();
        for (unsigned int i = 0; i < numRows; i++)          // Loop through all talents.
        {
            // Someday, someone needs to revamp
            TalentEntry *tmpTalent = sTalentStore.data[i];
            if (tmpTalent)                                  // the way talents are tracked
            {
                if (tmpTalent->TalentTab == tTab)
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

            WorldPacket data(SMSG_LEARNED_SPELL, 4);
            sLog.outDetail("TalentID: %u Rank: %u Spell: %u\n", talent_id, requested_rank, spellid);
            data << spellid;
            GetPlayer( )->GetSession()->SendPacket(&data);

            SpellEntry const *spellInfo = sSpellStore.LookupEntry( spellid );
            assert(spellInfo);                              // checked in addSpell

            // already apply in addSpell function
            //for(uint32 i = 0;i<3;i++)
            //{
            //    uint8 eff = spellInfo->Effect[i];
            //    if (eff>=TOTAL_SPELL_EFFECTS)
            //        continue;

            //    // Duration 21 = permanent
            //    if ((eff == 6) && (spellInfo->DurationIndex == 21) && (spellInfo->rangeIndex == 1))
            //    {
            //        Aura *Aur = new Aura(spellInfo, i, GetPlayer());
            //        GetPlayer()->AddAura(Aur);
            //    }
            //}

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
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outString("MSG_TALENT_WIPE_CONFIRM");
    uint64 guid;
    recv_data >> guid;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTalentWipeOpcode - NO SUCH UNIT! (GUID: %u)", uint32(GUID_LOPART(guid)) );
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if( !unit->isTrainer())                                 // it's not trainer
        return;

    if(!(_player->resetTalents()))
    {
        WorldPacket data;
        data.Initialize( MSG_TALENT_WIPE_CONFIRM );         //you have not any talent
        SendPacket( &data );
        return;
    }

    // send spell 14867
    WorldPacket data(SMSG_SPELL_START, (2+2+2+4+2+8+8));
    data.append(_player->GetPackGUID());
    data.append(unit->GetPackGUID());
    data << uint16(14867) << uint16(0x00) << uint16(0x0F) << uint32(0x00)<< uint16(0x00);
    SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO, (2+2+1+1+1+8+4+2+2+8+8));
    data.append(_player->GetPackGUID());
    data.append(unit->GetPackGUID());
    data << uint16(14867) << uint16(0x00) << uint8(0x0D) <<  uint8(0x01)<< uint8(0x01) << _player->GetGUID();
    data << uint32(0x00) << uint16(0x0200) << uint16(0x00);
    SendPacket( &data );
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 skill_id;
    recv_data >> skill_id;
    GetPlayer()->SetSkill(skill_id, 0, 0);
}
