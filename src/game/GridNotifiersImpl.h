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

#ifndef MANGOS_GRIDNOTIFIERSIMPL_H
#define MANGOS_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "SpellAuras.h"

template<>
inline void
MaNGOS::NotVisibleNotifier::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( ( i_player.isAlive() && iter->second->isAlive()) ||
        (i_player.isDead() && iter->second->isDead()) )
            iter->second->BuildOutOfRangeUpdateBlock(&i_data);
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second->IsVisibleInGridForPlayer(&i_player) )
        {
            iter->second->BuildUpdate(i_updateDatas);
            iter->second->BuildCreateUpdateBlockForPlayer(&i_data, &i_player);
        }
        else
        {
            iter->second->DestroyForPlayer(&i_player);
        }
    }
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if( iter->second == &i_player )
            continue;

        if( (i_player.isAlive() && iter->second->isAlive()) ||
            (i_player.isDead() && iter->second->isDead()) )
        {
            if (iter->second->isVisibleFor(&i_player,false))
                iter->second->SendUpdateToPlayer(&i_player);
            if (i_player.isVisibleFor(iter->second,false))
                i_player.SendUpdateToPlayer(iter->second);
        }
        else
        {
            i_player.DestroyForPlayer(iter->second);
            iter->second->DestroyForPlayer(&i_player);
        }
    }
}

inline void
MaNGOS::ObjectUpdater::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if(!iter->second->isSpiritHealer())
            iter->second->Update(i_timeDiff);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(PlayerMapType &m)
{
    if(!i_player.isAlive() || i_player.isInFlight())
        return;

    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        // Remove selection
        if(i_player.GetSelection()==iter->second->GetGUID())
                                                            // visibility distance
            if(!i_player.IsWithinDistInMap(iter->second, 160))
                                                            // valor under 160 can generate a bug of visibility, as a player
                i_player.SendOutOfRange(iter->second);      // can reach 158 yards until it disapears, without been selected

        if(iter->second->GetSelection()==i_player.GetGUID())
                                                            // visibility distance
            if(!i_player.IsWithinDistInMap(iter->second, 160))
                iter->second->SendOutOfRange(&i_player);

        // Cancel Trade
        if(i_player.GetTrader()==iter->second)
            if(!i_player.IsWithinDistInMap(iter->second, 5))     // iteraction distance
                i_player.GetSession()->SendCancelTrade();   // will clode both side trade windows

    }
}

inline void PlayerCreatureRelocationWorker(Player* pl, Creature* c)
{
    // Remove selection
    if(pl->GetSelection()==c->GetGUID())
        if(!pl->IsWithinDistInMap(c, 160))                       // visibility distance
            pl->SendOutOfRange(c);

    // Creature AI reaction
    if( c->AI() && c->AI()->IsVisible(pl) )
        c->AI()->MoveInLineOfSight(pl);
}

inline void CreatureCreatureRelocationWorker(Creature* c1, Creature* c2)
{
    if( c1->AI() && c1->AI()->IsVisible(c2) )
        c1->AI()->MoveInLineOfSight(c2);

    if( c2->AI() && c2->AI()->IsVisible(c1) )
        c2->AI()->MoveInLineOfSight(c1);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(CorpseMapType &m)
{
    for(CorpseMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( !i_player.isAlive() && iter->second->GetType()==CORPSE_RESURRECTABLE )
            iter->second->UpdateForPlayer(&i_player,false);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(CreatureMapType &m)
{
    if(!i_player.isAlive() || i_player.isInFlight())
        return;

    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            PlayerCreatureRelocationWorker(&i_player,iter->second);
}

template<>
inline void
MaNGOS::CreatureRelocationNotifier::Visit(PlayerMapType &m)
{
    if(!i_creature.isAlive() || i_creature.isInFlight())
        return;

    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            PlayerCreatureRelocationWorker(iter->second, &i_creature);
}

template<>
inline void
MaNGOS::CreatureRelocationNotifier::Visit(CreatureMapType &m)
{
    if(!i_creature.isAlive() || i_creature.isInFlight())
        return;

    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            CreatureCreatureRelocationWorker(iter->second, &i_creature);
}

template<>
inline void
MaNGOS::DynamicObjectUpdater::Visit(CreatureMapType  &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->isAlive() && !itr->second->isInFlight() )
        {
            if (owner.GetCaster()->IsFriendlyTo(itr->second) || !owner.IsWithinDistInMap(itr->second, owner.GetRadius()))
                continue;

            if (!owner.IsAffecting(itr->second))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(owner.GetSpellId());
                PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, owner.GetEffIndex(), itr->second, owner.GetCaster());
                itr->second->AddAura(Aur);
                owner.AddAffected(itr->second);
            }
        }
    }
}

template<>
inline void
MaNGOS::DynamicObjectUpdater::Visit(PlayerMapType  &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->isAlive() && !itr->second->isInFlight() )
        {
            if (owner.GetCaster()->IsFriendlyTo(itr->second) || !owner.IsWithinDistInMap(itr->second,owner.GetRadius()))
                continue;

            if (!owner.IsAffecting(itr->second))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(owner.GetSpellId());
                PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, owner.GetEffIndex(), itr->second, owner.GetCaster());
                itr->second->AddAura(Aur);
                owner.AddAffected(itr->second);
            }
        }
    }
}

// SEARCHERS & LIST SEARCHERS & WORKERS

// WorldObject searchers & workers

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    // already found
    if(i_object) return;

    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if(i_object) return;

    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object) return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(CorpseMapType &m)
{
    // already found
    if(i_objectptr) return;

    for(CorpseMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_objectptr = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    // already found
    if(i_object) return;

    for(DynamicObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(CorpseMapType &m)
{
    for(CorpseMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(&*itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    for(DynamicObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Gameobject searchers

template<class Check>
void MaNGOS::GameObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    // already found
    if(i_object) return;

    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::GameObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Unit searchers

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object) return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if(i_object) return;

    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Creature searchers

template<class Check>
void MaNGOS::CreatureSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object) return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::CreatureListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}
#endif                                                      // MANGOS_GRIDNOTIFIERSIMPL_H
