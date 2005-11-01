/* CreatureAISelector.cpp
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

#include "CreatureAIImpl.h"
#include "CreatureAISelector.h"
#include "NullCreatureAI.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"

namespace CreatureAISelector
{
    CreatureAI* select(Creature *creature)
    {
	CreatureAIRegistry &ai_registry(CreatureAIRepository::Instance());
	CreatureInfo *info = objmgr.GetCreatureName(creature->GetGUID());
	assert( info != NULL );
	const CreatureAICreator *ai_factory = ai_registry.GetRegistryItem( info->AIName.c_str() );

	if( ai_factory == NULL /* let's try the best effort mechanism */ )
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

	return ( ai_factory == NULL ? new NullCreatureAI : ai_factory->Create(creature) );
    }
}

