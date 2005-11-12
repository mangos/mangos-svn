/* Spell.h
 *
 * Copyright (C) 2004 Wow Daemon
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

#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "Database/DBCStores.h"

class WorldSession;
class Unit;
class DynamicObj;
class Player;
class Item;
class Group;
class Affect;

enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF             = 0x0,           // they are checked in following order
    TARGET_FLAG_UNIT             = 0x0002,
    TARGET_FLAG_OBJECT           = 0x0800,
    TARGET_FLAG_ITEM             = 0x1010,
    TARGET_FLAG_SOURCE_LOCATION  = 0x20,
    TARGET_FLAG_DEST_LOCATION    = 0x40,
    TARGET_FLAG_STRING           = 0x2000
};

enum SpellCastFlags
{
    CAST_FLAG_UNKNOWN1           = 0x2,
    CAST_FLAG_UNKNOWN2           = 0x10,          // no idea yet, i saw it in blizzard spell
    CAST_FLAG_AMMO               = 0x20           // load ammo display id (uint32) and ammo inventory type (uint32)
};

struct TeleportCoords
{
    uint32 id;
    uint32 mapId;
    float x;
    float y;
    float z;
};

// forward declaration
namespace MaNGOS
{
    class SpellNotifierPlayer;
    class SpellNotifierCreatureAndPlayer;
}

class SpellCastTargets
{

public:
        void read ( WorldPacket * data,uint64 caster );
        void write ( WorldPacket * data);

        SpellCastTargets& operator=(const SpellCastTargets &target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;

            m_srcX = target.m_srcX;
            m_srcY = target.m_srcY;
            m_srcZ = target.m_srcZ;

            m_destX = target.m_destX;
            m_destY = target.m_destY;
            m_destZ = target.m_destZ;

            m_strTarget = target.m_strTarget;

            m_targetMask = target.m_targetMask;

            return *this;
        }

        uint64 m_unitTarget;
        uint64 m_itemTarget;
        float m_srcX, m_srcY, m_srcZ;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint16 m_targetMask;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
};

enum ShapeshiftForm
{
    FORM_CAT              = 1,
    FORM_TREE             = 2,
    FORM_TRAVEL           = 3,
    FORM_AQUA             = 4,
    FORM_BEAR             = 5,
    FORM_AMBIENT          = 6,
    FORM_GHOUL            = 7,
    FORM_DIREBEAR         = 8,
    FORM_CREATUREBEAR     = 14,
    FORM_GHOSTWOLF        = 16,
    FORM_BATTLESTANCE     = 17,
    FORM_DEFENSIVESTANCE  = 18,
    FORM_BERSERKERSTANCE  = 19,
    FORM_SHADOW           = 28,
    FORM_STEALTH          = 30
};

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000
// Spell instance
class Spell
{
    friend class MaNGOS::SpellNotifierPlayer;
    friend class MaNGOS::SpellNotifierCreatureAndPlayer;
    public:
        Spell( Unit* Caster, SpellEntry *info, bool triggered, Affect* aff );

        void FillTargetMap();
        void prepare(SpellCastTargets * targets);
        void cancel();
        void update(uint32 difftime);
        void cast();
        void finish();
        void HandleEffects(uint64 guid,uint32 i);
        void TakePower();
        void TriggerSpell();
        uint8 CanCast();
        uint8 CheckItems();
        void RemoveItems();
        uint32 CalculateDamage(uint8 i);
        void HandleTeleport(uint32 id, Unit* Target);

        inline uint32 getState() { return m_spellState; }

// Send Packet functions
        void SendCastResult(uint8 result);
        void SendSpellStart();
        void SendSpellGo();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendDuelRequest(Player* caster, Player* target,uint64 ArbiterID);

        void HandleAddAffect(uint64 guid);
        void writeSpellGoTargets( WorldPacket * data );

        SpellEntry * m_spellInfo;
        Item* m_CastItem;
        SpellCastTargets m_targets;

    protected:

        float _CalcDistance(float sX, float sY, float sZ, float dX, float dY, float dZ)
        {
            return sqrt((dX-sX)*(dX-sX)+(dY-sY)*(dY-sY)+(dZ-sZ)*(dZ-sZ));
        }

        Unit* m_caster;

        std::list<uint64> m_targetUnits1;
        std::list<uint64> m_targetUnits2;
        std::list<uint64> m_targetUnits3;
        std::list<uint64> UniqueTargets;
        uint8 m_targetCount;

        uint32 m_spellState;
        uint32 m_timer;
        uint32 m_intervalTimer;                   // used to update channel bar
        uint32 TriggerSpellId;                    // used to set next spell to use

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        bool m_triggeredSpell;
        Affect* m_triggeredByAffect;
        bool m_AreaAura;

};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH = 20,
    REPLENISH_MANA = 21,
    REPLENISH_RAGE = 22                           //don't know if rage is 22 or what, but will do for now
};

// notifiers
namespace MaNGOS
{
    struct MANGOS_DLL_DECL SpellNotifierPlayer
    {
	std::list<uint64> &i_data;
	Spell &i_spell;
	const uint32& i_index;
	SpellNotifierPlayer(Spell &spell, std::list<uint64> &data, const uint32 &i) : i_spell(spell), i_data(data), i_index(i) {}
	inline void Visit(PlayerMapType &m)
	{
	    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
	    {
		if( !itr->second->isAlive() )
		    continue;
		if( i_spell._CalcDistance(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ,
					  itr->second->GetPositionX(),itr->second->GetPositionY(),itr->second->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index])) && itr->second->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != i_spell.m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) )
		    i_data.push_back(itr->second->GetGUID());
	    }
	}
    };

    struct MANGOS_DLL_DECL SpellNotifierCreatureAndPlayer
    {
	std::list<uint64> &i_data;
	Spell &i_spell;
	const uint32& i_index;
	SpellNotifierCreatureAndPlayer(Spell &spell, std::list<uint64> &data, const uint32 &i) : i_spell(spell), i_data(data), i_index(i) {}

	template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
	{
	    for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
	    {
		i_data.push_back(itr->second->GetGUID());
		if(i_spell.m_caster->isInFront((Unit*)(itr->second),GetRadius(sSpellRadius.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index]))) && (itr->second)->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != i_spell.m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
		    i_data.push_back(itr->second->GetGUID());
	    }
	}

	// specialization ..don't care about
#ifdef WIN32
	template<> inline void Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
	template<> inline void Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
	template<> inline void Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
#endif
    };

#ifndef WIN32
	template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, Corpse *> &m ) {}
	template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, GameObject *> &m ) {}
	template<> inline void SpellNotifierCreatureAndPlayer::Visit(std::map<OBJECT_HANDLE, DynamicObject *> &m ) {}
#endif

}


#endif
