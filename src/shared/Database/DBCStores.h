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

#ifndef DBCSTORES_H
#define DBCSTORES_H

#include "Common.h"
//#include "DataStore.h"
#include "dbcfile.h"
#include "DBCStructure.h"

struct AuctionEntry
{
    uint32 auctioneer;
    uint32 item;
    uint32 owner;
    uint32 bid;
    uint32 buyout;
    time_t time;
    uint32 bidder;
    uint32 Id;
};

enum SpellSpecific
{
    SPELL_NORMAL = 0,
    SPELL_SEAL = 1,
    SPELL_BLESSING = 2,
    SPELL_AURA = 3,
    SPELL_STING = 4,
    SPELL_CURSE = 5,
    SPELL_ASPECT = 6
};

enum SpellFamilyNames
{
    SPELLFAMILY_GENERIC = 0,
    SPELLFAMILY_MAGE = 3,
    SPELLFAMILY_WARRIOR = 4,
    SPELLFAMILY_WARLOCK = 5,
    SPELLFAMILY_PRIEST = 6,
    SPELLFAMILY_DRUID = 7,
    SPELLFAMILY_ROGUE = 8,
    SPELLFAMILY_HUNTER = 9,
    SPELLFAMILY_PALADIN = 10,
    SPELLFAMILY_SHAMAN = 11,
    SPELLFAMILY_POTION = 13
};

float GetRadius(SpellRadius *radius);
uint32 GetCastTime(SpellCastTime *time);
float GetMinRange(SpellRange *range);
float GetMaxRange(SpellRange *range);
int32 GetDuration(SpellEntry *spellInfo);
uint32 FindSpellRank(uint32 spellId);
bool canStackSpellRank(SpellEntry *spellInfo);
bool IsRankSpellDueToSpell(SpellEntry *spellInfo_1,uint32 spellId_2);
bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2);
bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2);
SpellSpecific GetSpellSpecific(uint32 spellId);
bool IsSpellSingleEffectPerCaster(uint32 spellId);
bool IsPassiveSpell(uint32 spellId);
bool IsPositiveSpell(uint32 spellId);
bool IsPositiveEffect(uint32 spellId, uint32 effIndex);

template<class T>
class DBCStorage
{
    public:
        DBCStorage(const char *f){data = NULL;fmt=f;}
        ~DBCStorage(){if(data) delete [] data;};

        inline
            T* LookupEntry(uint32 id)
        {
            return (id>nCount)?NULL:data[id];

        }
        inline
            unsigned int GetNumRows()
        {
            return nCount;
        }

        bool Load(char const* fn)
        {

            dbc = new DBCFile;
            // Check if load was sucessful, only then continue
            bool res = dbc->Load(fn);
            if (res)
            {
                data=(T **) dbc->AutoProduceData(fmt,&nCount);
            }
            delete dbc;

            return res;
        }

        T** data;
        uint32 nCount;

    private:
        DBCFile * dbc;
        const char * fmt;
};

extern DBCStorage <AreaTableEntry>           sAreaStore;
extern DBCStorage <SpellCastTime>            sCastTime;
extern DBCStorage <emoteentry>               sEmoteStore;
extern DBCStorage <FactionEntry>             sFactionStore;
extern DBCStorage <FactionTemplateEntry>     sFactionTemplateStore;
extern DBCStorage <ItemDisplayTemplateEntry> sItemDisplayTemplateStore;
extern DBCStorage <ItemSetEntry>             sItemSetStore;
extern DBCStorage <SkillLineAbility>         sSkillLineAbilityStore;
extern DBCStorage <SpellDuration>            sSpellDuration;
extern DBCStorage <SpellFocusObject>         sSpellFocusObject;
extern DBCStorage <SpellItemEnchantment>     sSpellItemEnchantmentStore;
extern DBCStorage <SpellRadius>              sSpellRadius;
extern DBCStorage <SpellRange>               sSpellRange;
extern DBCStorage <SpellEntry>               sSpellStore;
extern DBCStorage <TalentEntry>              sTalentStore;

void LoadDBCStores(std::string dataPath);
#endif
