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

#ifndef MANGOS_FACTIONTEMPLATEHOLER_H
#define MANGOS_FACTIONTEMPLATEHOLER_H

#include "Platform/Define.h"
#include "Database/DBCStores.h"

struct MANGOS_DLL_DECL FactionTemplateResolver
{
    FactionTemplateResolver(FactionTemplateEntry const *entry)
    {
        data.All = 0;
        if( entry != NULL )
        {
            data.Faction.friendly_mask.ALL = entry->friendly;
            data.Faction.hostile_mask.ALL = entry->hostile;
        }
    }

    union
    {
        struct
        {
            union
            {
                struct
                {
                    unsigned All : 1;
                    unsigned Allia : 1;
                    unsigned Hrd : 1;
                    unsigned Monster : 1;
                } friendly;
                unsigned ALL : 4;
            } friendly_mask;

            union
            {
                struct
                {
                    unsigned All : 1;
                    unsigned Allia : 1;
                    unsigned Hrd : 1;
                    unsigned Monster : 1;
                } hostile;
                unsigned ALL : 4;
            } hostile_mask;

        } Faction;
        unsigned char All;
    } data;

    bool IsHostileToAll(void) const
    {
        return ( data.Faction.hostile_mask.hostile.All );
    }

    bool IsNeutralToAll(void) const
    {

        return ( data.Faction.hostile_mask.ALL == 0 && data.Faction.friendly_mask.ALL == 0);
    }

    bool IsHostileTo(const FactionTemplateResolver &holder) const
    {
        return( IsHostileToAll() || (data.Faction.hostile_mask.ALL & holder.data.Faction.friendly_mask.ALL) );
    }

    bool IsFriendlyTo(const FactionTemplateResolver &holder) const
    {
        return( !IsHostileToAll() && (data.Faction.friendly_mask.ALL & holder.data.Faction.friendly_mask.ALL) );
    }
};
#endif
