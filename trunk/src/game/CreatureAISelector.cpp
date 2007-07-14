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

#include "CreatureAIImpl.h"
#include "CreatureAISelector.h"
#include "NullCreatureAI.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"
#include "MovementGenerator.h"
#include "ScriptCalls.h"

INSTANTIATE_SINGLETON_1(CreatureAIRegistry);
INSTANTIATE_SINGLETON_1(MovementGeneratorRegistry);

namespace FactorySelector
{
    CreatureAI* selectAI(Creature *creature)
    {
        /*if(CreatureAI* scriptedAI = Script->GetAI(creature))
            return scriptedAI;*/
        //NOTE: in case there are valid scripts for pets remove check for !creature->isPet()
        //if there are scripts for charms remove check for !creature->isCharmed()
        //the commented condition is too db dependant to get into the core, but is an example to create the possibilty to have scripts for untamed pets and scripts for their tamed/owned equivalents
        //e.g. if there are scripts for warlock pets but not for hunter's the activation of the check will allow warlock pets to be scripted while hunter pets would retain standart pet AI
        CreatureAI* scriptedAI = Script->GetAI(creature);
		if(scriptedAI && !((creature->isPet() || creature->isCharmed())/* && creature->GetCreatureInfo()->ScriptName == "generic_creature"*/))
			return scriptedAI;

        CreatureAIRegistry &ai_registry(CreatureAIRepository::Instance());
        assert( creature->GetCreatureInfo() != NULL );
        CreatureInfo const *cinfo=creature->GetCreatureInfo();

        const CreatureAICreator *ai_factory = NULL;

        std::string ainame=cinfo->AIName;

        // select by script name
        if( ainame!="")
            ai_factory = ai_registry.GetRegistryItem( ainame.c_str() );

        // select by NPC flags
        if(!ai_factory)
        {
            if( creature->isGuard() )
                ainame="GuardAI";
            else if(creature->isPet() || creature->isCharmed())
                ainame="PetAI";
            else if(creature->isTotem())
                ainame="TotemAI";

            ai_factory = ai_registry.GetRegistryItem( ainame.c_str() );
        }

        // select by permit check
        if(!ai_factory)
        {
            int best_val = -1;
            std::vector<std::string> l;
            ai_registry.GetRegisteredItems(l);
            for( std::vector<std::string>::iterator iter = l.begin(); iter != l.end(); ++iter)
            {
                const CreatureAICreator *factory = ai_registry.GetRegistryItem((*iter).c_str());
                const SelectableAI *p = dynamic_cast<const SelectableAI *>(factory);
                assert( p != NULL );
                int val = p->Permit(creature);
                if( val > best_val )
                {
                    best_val = val;
                    ai_factory = p;
                }
            }
        }

        // select NullCreatureAI if not another cases
        ainame = (ai_factory == NULL) ? "NullCreatureAI" : ai_factory->key();

        DEBUG_LOG("Creature %u used AI is %s.", creature->GetGUIDLow(), ainame.c_str() );
        return ( ai_factory == NULL ? new NullCreatureAI : ai_factory->Create(creature) );
    }

    MovementGenerator* selectMovementGenerator(Creature *creature)
    {
        MovementGeneratorRegistry &mv_registry(MovementGeneratorRepository::Instance());
        assert( creature->GetCreatureInfo() != NULL );
        const MovementGeneratorCreator *mv_factory = mv_registry.GetRegistryItem( creature->GetDefaultMovementType());

        /* if( mv_factory == NULL  )
        {
            int best_val = -1;
            std::vector<std::string> l;
            mv_registry.GetRegisteredItems(l);
            for( std::vector<std::string>::iterator iter = l.begin(); iter != l.end(); ++iter)
            {
            const MovementGeneratorCreator *factory = mv_registry.GetRegistryItem((*iter).c_str());
            const SelectableMovement *p = dynamic_cast<const SelectableMovement *>(factory);
            assert( p != NULL );
            int val = p->Permit(creature);
            if( val > best_val )
            {
                best_val = val;
                mv_factory = p;
            }
            }
        }*/

        return ( mv_factory == NULL ? NULL : mv_factory->Create(creature) );

    }
}
