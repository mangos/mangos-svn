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
MaNGOS::NotVisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( ( i_player.isAlive() && iter->second->isAlive()) ||
        (i_player.isDead() && iter->second->isDead()) )
            iter->second->BuildOutOfRangeUpdateBlock(&i_data);
}

template<>
inline void
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
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
MaNGOS::VisibleNotifier::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator iter=m.begin(); iter != m.end(); ++iter)
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
MaNGOS::ObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature*>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if(!iter->second->isSpiritHealer())
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
    if( c->AI().IsVisible(pl) )
        c->AI().MoveInLineOfSight(pl);
}

inline void CreatureCreatureRelocationWorker(Creature* c1, Creature* c2)
{
    if( c1->AI().IsVisible(c2) )
        c1->AI().MoveInLineOfSight(c2);

    if( c2->AI().IsVisible(c1) )
        c2->AI().MoveInLineOfSight(c1);
}

template<>
inline void
MaNGOS::PlayerRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    for(std::map<OBJECT_HANDLE, Corpse *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( !i_player.isAlive() && iter->second->GetType()==CORPSE_RESURRECTABLE )
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
MaNGOS::CreatureRelocationNotifier::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    if(!i_creature.isAlive() || i_creature.isInFlight())
        return;

    for(std::map<OBJECT_HANDLE, Creature *>::iterator iter=m.begin(); iter != m.end(); ++iter)
        if( iter->second->isAlive() && !iter->second->isInFlight())
            CreatureCreatureRelocationWorker(iter->second, &i_creature);
}

template<>
inline void
MaNGOS::DynamicObjectUpdater::Visit(std::map<OBJECT_HANDLE, Creature *>  &m)
{
    for(std::map<OBJECT_HANDLE, Creature*>::iterator itr=m.begin(); itr != m.end(); ++itr)
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
MaNGOS::DynamicObjectUpdater::Visit(std::map<OBJECT_HANDLE, Player *>  &m)
{
    for(std::map<OBJECT_HANDLE, Player*>::iterator itr=m.begin(); itr != m.end(); ++itr)
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
void MaNGOS::WorldObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Player*> &m)
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
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature*> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, Creature*>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Corpse*> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, Corpse *>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, DynamicObject*> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, DynamicObject*>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Corpse *> &m)
{
    for(std::map<OBJECT_HANDLE, Corpse *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m)
{
    for(std::map<OBJECT_HANDLE, DynamicObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Gameobject searchers

template<class Check>
void MaNGOS::GameObjectSearcher<Check>::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    // already found
    if(i_object) return;

    for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->second))
        {
            i_object = itr->second;
            return;
        }
    }
}

template<class Check>
void MaNGOS::GameObjectListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, GameObject *> &m)
{
    for(std::map<OBJECT_HANDLE, GameObject *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Unit searchers

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
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Player *> &m)
{
    for(std::map<OBJECT_HANDLE, Player *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}

// Creature searchers

template<class Check>
void MaNGOS::CreatureSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
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
}

template<class Check>
void MaNGOS::CreatureListSearcher<Check>::Visit(std::map<OBJECT_HANDLE, Creature *> &m)
{
    for(std::map<OBJECT_HANDLE, Creature *>::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->second))
            i_objects.push_back(itr->second);
}
#endif                                                      // MANGOS_GRIDNOTIFIERSIMPL_H
