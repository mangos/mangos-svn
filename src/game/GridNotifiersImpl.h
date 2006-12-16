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

#ifndef MANGOS_GRIDNOTIFIERSIMPL_H
#define MANGOS_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "Utilities.h"
#include "SpellAuras.h"

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( ( i_player.isAlive() && iter->second->isAlive()) ||
        (i_player.isDead() && iter->second->isDead()) )
            iter->second->BuildOutOfRangeUpdateBlock(&i_data);
}

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;
        if( (i_player.isAlive() && iter->second->isAlive()) ||
            (i_player.isDead() && iter->second->isDead()) )
        {
            iter->second->BuildOutOfRangeUpdateBlock(&i_data);

            UpdateData his_data;
            WorldPacket his_pk;
            i_player.BuildOutOfRangeUpdateBlock(&his_data);
            his_data.BuildPacket(&his_pk);
            iter->second->GetSession()->SendPacket(&his_pk);
        }
    }
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second->IsVisibleInGridForPlayer(&i_player) )
        {
            iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
        }
        else
        {
            ObjectAccessor::Instance().RemoveCreatureFromPlayerView(&i_player, iter->second);
        }
    }
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;

        if( (i_player.isAlive() && iter->second->isAlive()) ||
            (i_player.isDead() && iter->second->isDead()) )
        {
            if (iter->second->isVisibleFor(&i_player))
                iter->second->SendUpdateToPlayer(&i_player);
            if (i_player.isVisibleFor(iter->second))
                i_player.SendUpdateToPlayer(iter->second);
        }
        else
        {
            ObjectAccessor::Instance().RemovePlayerFromPlayerView(&i_player, iter->second);
        }
    }
}

template<>
inline void
MaNGOS::VisibleChangesNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        // Invisibility for gms
        if(iter->second != &i_player)
        {
            switch(i_player.GetUpdateVisibility())
            {
                case VISIBLE_SET_INVISIBLE:
                {
                    ObjectAccessor::Instance().RemoveInvisiblePlayerFromPlayerView(&i_player, iter->second);
                    iter->second->m_DetectInvTimer = 1;
                }
                break;
                case VISIBLE_SET_INVISIBLE_FOR_GROUP:
                    if (!iter->second->IsInGroupWith(&i_player))
                    {
                        ObjectAccessor::Instance().RemoveInvisiblePlayerFromPlayerView(&i_player, iter->second);
                        iter->second->m_DetectInvTimer = 1;
                    }
                    break;
                case VISIBLE_SET_VISIBLE:
                    i_player.SendUpdateToPlayer(iter->second);
                    break;
            }

            // Detect invisible pjs
            if (i_player.m_enableDetect && iter->second->GetVisibility() == VISIBILITY_GROUP && !iter->second->IsInGroupWith(&i_player))
            {
                if(i_player.IsWithinDist(iter->second, 20))
                    i_player.InvisiblePjsNear.push_back(iter->second);
            }

            // Reveal Invisible Pjs
            if (iter->second == i_player.m_DiscoveredPj)
                iter->second->SendUpdateToPlayer(&i_player);
        }
    }
}

template<>
inline void
MaNGOS::ObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature*>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if(!MaNGOS::Utilities::IsSpiritHealer(iter->second))
            iter->second->Update(i_timeDiff);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    if(!i_player.isAlive() || i_player.isInFlight())
        return;

    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        // Remove selection
        if(i_player.GetSelection()==iter->second->GetGUID())
                                                            // visibility distance
            if(!i_player.IsWithinDist(iter->second, 160))   // valor under 160 can generate a bug of visibility, as a player
                i_player.SendOutOfRange(iter->second);      // can reach 158 yards until it disapears, without been selected

        if(iter->second->GetSelection()==i_player.GetGUID())
                                                            // visibility distance
            if(!i_player.IsWithinDist(iter->second, 160))
                iter->second->SendOutOfRange(&i_player);

        // Cancel Trade
        if(i_player.GetTrader()==iter->second)
            if(!i_player.IsWithinDist(iter->second, 5))     // iteraction distance
                i_player.GetSession()->SendCancelTrade();   // will clode both side trade windows

    }
}

inline void PlayerCreatureRelocationWorker(Player* pl, Creature* c)
{
    // Remove selection
    if(pl->GetSelection()==c->GetGUID())
        if(!pl->IsWithinDist(c, 100))                       // visibility distance
            pl->SendOutOfRange(c);

    // Creature AI reaction
    if( c->AI().IsVisible(pl) )
        c->AI().MoveInLineOfSight(pl);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    for(std::map<OBJECT_HANDLE, Corpse *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( !i_player.isAlive() )
            iter->second->UpdateForPlayer(&i_player,false);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    if(!i_player.isAlive() || i_player.isInFlight())
        return;

    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            PlayerCreatureRelocationWorker(&i_player,iter->second);
}

template<>
inline void
MaNGOS::CreatureRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    if(!i_creature.isAlive() || i_creature.isInFlight())
        return;

    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            PlayerCreatureRelocationWorker(iter->second, &i_creature);
}

template<>
inline void
MaNGOS::DynamicObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *>  &m)
{
    for(std::map<OBJECT_HANDLE, Creature*>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->isAlive() && !itr->second->isInFlight() )
        {
            if (owner.GetCaster()->IsFriendlyTo(itr->second) || !owner.IsWithinDist(itr->second, owner.GetRadius()))
                continue;

            if (!owner.IsAffecting(itr->second))
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry(owner.GetSpellId());
                PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, owner.GetEffIndex(), itr->second, owner.GetCaster());
                itr->second->AddAura(Aur);
                owner.AddAffected(itr->second);
            }
        }
    }
}

template<>
inline void
MaNGOS::DynamicObjectUpdater::Visit(std::map<OBJECT_HANDLE, Player *>  &m)
{
    for(std::map<OBJECT_HANDLE, Player*>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->isAlive() && !itr->second->isInFlight() )
        {
            if (owner.GetCaster()->IsFriendlyTo(itr->second) || !owner.IsWithinDist(itr->second,owner.GetRadius()))
                continue;

            if (!owner.IsAffecting(itr->second))
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry(owner.GetSpellId());
                PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, owner.GetEffIndex(), itr->second, owner.GetCaster());
                itr->second->AddAura(Aur);
                owner.AddAffected(itr->second);
            }
        }
    }
}

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
    i_object = i_check.GetResult();
}

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, Player *>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
    i_object = i_check.GetResult();
}
#endif                                                      // MANGOS_GRIDNOTIFIERSIMPL_H
