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

#ifndef MANGOS_CREATUREAI_H
#define MANGOS_CREATUREAI_H

#include "Common.h"
#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"

class Unit;
class Creature;

#define TIME_INTERVAL_LOOK   5000
#define VISIBILITY_RANGE    10000

class MANGOS_DLL_SPEC CreatureAI
{
    public:

        virtual ~CreatureAI();

        virtual void MoveInLineOfSight(Unit *) = 0;

        virtual void AttackStart(Unit *) = 0;

        virtual void EnterEvadeMode() = 0;

        virtual void HealBy(Unit *healer, uint32 amount_healed) = 0;

        virtual void DamageInflict(Unit *done_by, uint32 amount_damage) = 0;

        virtual bool IsVisible(Unit *) const = 0;

        virtual void UpdateAI(const uint32 diff) = 0;

        virtual void JustDied(Unit *) {};

        virtual void KilledUnit(Unit *) {};
};

struct SelectableAI : public FactoryHolder<CreatureAI>, public Permissible<Creature>
{

    SelectableAI(const char *id) : FactoryHolder<CreatureAI>(id) {}
};

template<class REAL_AI>
struct CreatureAIFactory : public SelectableAI
{
    CreatureAIFactory(const char *name) : SelectableAI(name) {}

    CreatureAI* Create(void *) const;

    int Permit(const Creature *c) const { return REAL_AI::Permissible(c); }
};

#define PERMIT_BASE_NO                  -1
#define PERMIT_BASE_IDLE                1
#define PERMIT_BASE_REACTIVE            100
#define PERMIT_BASE_PROACTIVE           200
#define PERMIT_BASE_FACTION_SPECIFIC    400
#define PERMIT_BASE_SPECIAL             800

typedef FactoryHolder<CreatureAI> CreatureAICreator;
typedef FactoryHolder<CreatureAI>::FactoryHolderRegistry CreatureAIRegistry;
typedef FactoryHolder<CreatureAI>::FactoryHolderRepository CreatureAIRepository;
#endif
