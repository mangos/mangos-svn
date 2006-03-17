/* 
 * Copyright (C) 2005 MaNGOS <http://www.magosproject.org/>
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

#include "DBCStores.h"
#include "DataStore.h"




implementIndexedDBCStore(SpellStore,SpellEntry)
implementIndexedDBCStore(DurationStore,SpellDuration)
implementIndexedDBCStore(RangeStore,SpellRange)
implementIndexedDBCStore(EmoteStore,emoteentry)
implementIndexedDBCStore(RadiusStore,SpellRadius)
implementIndexedDBCStore(CastTimeStore,SpellCastTime)
implementIndexedDBCStore(TalentStore,TalentEntry)


implementIndexedDBCStore(AreaTableStore,AreaTableEntry)
implementIndexedDBCStore(WorldMapAreaStore,WorldMapAreaEntry)
implementDBCStore(WorldMapOverlayStore,WorldMapOverlayEntry)

implementIndexedDBCStore(FactionStore,FactionEntry)
implementIndexedDBCStore(FactionTemplateStore,FactionTemplateEntry)


implementDBCStore(ItemDisplayTemplateStore,ItemDisplayTemplateEntry)

implementDBCStore(SkillStore,skilllinespell)


float GetRadius(SpellRadius *radius)
{
    return radius->Radius;
}


uint32 GetCastTime(SpellCastTime *time)
{
    return time->CastTime;
}


float GetMaxRange(SpellRange *range)
{
    return range->maxRange;
}


float GetMinRange(SpellRange *range)
{
    return range->minRange;
}


uint32 GetDuration(SpellDuration *dur)
{
    return dur->Duration1;
}
