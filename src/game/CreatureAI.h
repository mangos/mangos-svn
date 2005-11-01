/* CreatureAI.h
 *
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

#ifndef MANGOS_CREATUREAI_H
#define MANGOS_CREATUREAI_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"

/** This is an API for Creatures AI
 */

class Creature;
class Player;

class MANGOS_DLL_DECL CreatureAI 
{
public:
    
    // dtor
    virtual ~CreatureAI();

    /** Call on a creature/Player move in line of sight of creature.
     * The line of sight of the creature depends on the distance
     * of the target plus the angle the creature is looking.
     */
    virtual void MoveInLineOfSight(Creature *) = 0;
    virtual void MoveInLineOfSight(Player *) = 0;

    /// Call when a player/Creature starts to attack the creature
    virtual void AttackStart(Creature *) = 0;
    virtual void AttackStart(Player *) = 0;

    /// Call when the creature/Player stop attacking
    virtual void AttackStop(Creature *) = 0;
    virtual void AttackStop(Player *) = 0;

    /// Call when the victim of the creature heals by another creature/player
    virtual void HealBy(Creature *healer, uint32 amount_healed) = 0;
    virtual void HealBy(Player *healer, uint32 amount_healed) = 0;

    /// Call when the attacker inflict damage on the binded victim
    virtual void DamageInflict(Player *done_by, uint32 amount_damage) = 0;
    virtual void DamageInflict(Creature *done_by, uint32 amount_damage) = 0;

    /// Call when a unit is near... the AI figure if visible
    virtual bool IsVisible(Creature *) const = 0; 
    virtual bool IsVisible(Player *) const = 0; 

    /// Updates the player attack
    virtual void UpdateAI(const uint32 diff) = 0;

};


struct SelectableAI : public FactoryHolder<CreatureAI>, public Permissible<Creature>
{
    /*
     * SelectableAI is a place holder that allows AI specific to implement
     * a Permit method that indicates how good this AI handles the given
     * creature.  One that score the highest has the best result.
     */
    SelectableAI(const char *id) : FactoryHolder<CreatureAI>(id) {}
};

/** CreatureAI Factor is a factory holder for creating real objects
 */
template<class REAL_AI>
struct CreatureAIFactory : public SelectableAI
{
    CreatureAIFactory(const char *name) : SelectableAI(name) {}

    // implement API for FactorHolder<T>
    CreatureAI* Create(void *) const;

    // implement API for Permissible<T>
    int Permit(const Creature *c) const { return REAL_AI::Permissible(c); }
};

/* Permit defines
 */
#define NO_PERMIT  -1
#define IDLE_PERMIT_BASE 1
#define REACTIVE_PERMIT_BASE 100
#define PROACTIVE_PERMIT_BASE 200
#define FACTION_SPECIFIC_PERMIT_BASE 400
#define SPEICAL_PERMIT_BASE 800

typedef FactoryHolder<CreatureAI> CreatureAICreator;
typedef FactoryHolder<CreatureAI>::FactoryHolderRegistry CreatureAIRegistry;
typedef FactoryHolder<CreatureAI>::FactoryHolderRepository CreatureAIRepository;

#endif
