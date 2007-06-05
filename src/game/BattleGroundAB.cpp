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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundAB.h"
#include "Creature.h"
#include "Chat.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"

BattleGroundAB::BattleGroundAB()
{
    m_TeamScores[0] = 0;
    m_TeamScores[1] = 0;
    m_Points[0] = 0;
    m_Points[1] = 0;
    m_Points[2] = 0;
    m_Points[3] = 0;
    m_Points[4] = 0;
}

BattleGroundAB::~BattleGroundAB()
{

}

void BattleGroundAB::Update(time_t diff)
{
    BattleGround::Update(diff);
    //If BG-Status = WAIT_JOIN, we must start BG
    if(GetStatus() == STATUS_WAIT_JOIN)
    {

    }
    if(GetStatus() == STATUS_IN_PROGRESS)
    {
        for(int i = 0;i < 5; i++)
            if(m_Points[i])                                 //If point is controled
                AddPoint(m_Points[i], diff);
        if(GetTeamScore(ALLIANCE) >= (2000*1000))           //1 score/per second
            EndBattleGround(ALLIANCE);
        if(GetTeamScore(HORDE) >= (2000*1000))              //1 score/per second
            EndBattleGround(HORDE);
    }
}

void BattleGroundAB::RemovePlayer(Player *plr, uint64 guid)
{

}

void BattleGroundAB::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if(GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 SpellId = 0;
    switch(Trigger)
    {
        case 3866:                                          // Stables
        case 3869:                                          // Gold Mine
        case 3867:                                          // Farm
        case 3868:                                          // Lumber Mill
        case 3870:                                          // Black Smith
        case 4020:                                          // Unk1
        case 4021:                                          // Unk2
            break;
        case 3948:                                          // Arathi Basin Alliance Exit.
            if(Source->GetTeam() != ALLIANCE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                ((BattleGround*)this)->RemovePlayer(Source->GetGUID(), true, true);
            break;
        case 3949:                                          // Arathi Basin Horde Exit.
            if(Source->GetTeam() != HORDE)
                Source->GetSession()->SendAreaTriggerMessage("Only The Horde can use that portal");
            else
                ((BattleGround*)this)->RemovePlayer(Source->GetGUID(), true, true);
            break;
        default:
            sLog.outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    if(SpellId)
    {
        SpellEntry const *Entry = sSpellStore.LookupEntry(SpellId);

        if(!Entry)
        {
            sLog.outError("ERROR: Tried to cast unknown spell id %u to player.", SpellId);
            return;
        }

        Source->CastSpell(Source, Entry, true, 0);
    }
}

void BattleGroundAB::SetupBattleGround()
{

}
