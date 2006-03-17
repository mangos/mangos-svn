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
    TARGET_FLAG_SELF             = 0x0,           
    TARGET_FLAG_UNIT             = 0x0002,
    TARGET_FLAG_OBJECT           = 0x0800,
    TARGET_FLAG_ITEM             = 0x1010,
    TARGET_FLAG_SOURCE_LOCATION  = 0x20,
    TARGET_FLAG_DEST_LOCATION    = 0x40,
    TARGET_FLAG_STRING           = 0x2000
};













enum Targets
{
	TARGET_SELF         = 1,
	TARGET_PET          = 5,
	TARGET_S_E          = 6,
	TARGET_AE_E         = 15,
	TARGET_AE_E_INSTANT = 16,
	TARGET_AC_P         = 20,
	TARGET_S_F			= 21,
	TARGET_AC_E			= 22,
	TARGET_S_GO			= 23,
	TARGET_INFRONT      = 24,
	TARGET_DUELVSPLAYER = 25,
	TARGET_GOITEM		= 26,
	TARGET_AE_E_CHANNEL = 28,
	TARGET_MINION		= 32,
	TARGET_S_P			= 35,
	TARGET_CHAIN		= 45,
	TARGET_AE_SELECTED  = 53,
};

enum SpellCastFlags
{
    CAST_FLAG_UNKNOWN1           = 0x2,
    CAST_FLAG_UNKNOWN2           = 0x10,          
    CAST_FLAG_AMMO               = 0x20           
};

enum SpellNotifyPushType
{
    PUSH_IN_FRONT     = 0,
	PUSH_SELF_CENTER  = 1,
	PUSH_DEST_CENTER  = 2
};
struct TeleportCoords
{
    uint32 id;
    uint32 mapId;
    float x;
    float y;
    float z;
};


namespace MaNGOS
{
    struct SpellNotifierPlayer;
    struct SpellNotifierCreatureAndPlayer;
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

enum Effects
{
	EFF_SCHOOL_DMG          = 2,
	EFF_DUMMY		        = 3,
	EFF_TELEPORT_UNITS      = 5,
	EFF_APPLY_AURA	        = 6,
	EFF_POWER_DRAIN         = 8,
	EFF_HEAL		        = 10,
	EFF_WEAPON_DMG_NS       = 17,
	EFF_CREATE_ITEM         = 24,
	EFF_PRESISTENT_AA       = 27,
	EFF_ENERGIZE	        = 30,
	EFF_WEAPON_DMG_PERC     = 31,
	EFF_OPEN_LOCK		    = 33,
	EFF_OPEN_SECRECTSAFE    = 59,
	EFF_APPLY_AA		    = 35,
	EFF_LEARN_SPELL		    = 36,
	EFF_SUMMON_WILD		    = 41,
	EFF_ENCHANT_ITEM_PERM   = 53,
	EFF_ENCHANT_ITEM_TMP    = 54,
	EFF_SUMMON_PET		    = 56,
	EFF_WEAPON_DMG		    = 58,
	EFF_THREAT			    = 63,
	EFF_TRIGGER_SPELL	    = 64,
	EFF_HEAL_MAX_HEALTH     = 67,
	EFF_INTERRUPT_CAST      = 68,
	EFF_ADD_COMBO_POINTS    = 80,
	EFF_DUEL			    = 83,
	EFF_SUMMON_TOTEM_SLOT1  = 87,
	EFF_SUMMON_TOTEM_SLOT2  = 88,
	EFF_SUMMON_TOTEM_SLOT3  = 89,
	EFF_SUMMON_TOTEM_SLOT4  = 90,
	EFF_ENCHANT_HELD_ITEM   = 92,
	EFF_FEED_PET		    = 101,
	EFF_SUMMON_OBJECT_SLOT1 = 104,
	EFF_SUMMON_OBJECT_SLOT2 = 105,
	EFF_SUMMON_OBJECT_SLOT3 = 106,
	EFF_SUMMON_OBJECT_SLOT4 = 107,
	EFF_RESURRECT			= 113,
};


#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

class Spell
{
    friend struct MaNGOS::SpellNotifierPlayer;
    friend struct MaNGOS::SpellNotifierCreatureAndPlayer;
    public:
        Spell( Unit* Caster, SpellEntry *info, bool triggered, Affect* aff );

		
        
        void prepare(SpellCastTargets * targets);
        void cancel();
        void update(uint32 difftime);
        void cast();
        void finish();
        void TakePower();
        void TriggerSpell();
        uint8 CanCast();
        uint8 CheckItems();
        void RemoveItems();
        uint32 CalculateDamage(uint8 i);
        void HandleTeleport(uint32 id, Unit* Target);
        inline uint32 getState() { return m_spellState; }

		
		void writeSpellGoTargets( WorldPacket * data );
        void FillTargetMap();
        
        void SetTargetMap(uint32 i,uint32 cur,Player* p_caster,std::list<uint64> &TagMap);

		
        void SendCastResult(uint8 result);
        void SendSpellStart();
        void SendSpellGo();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendDuelRequest(Player* caster, Player* target,uint64 ArbiterID);

		
		void HandleEffects(uint64 guid,uint32 i);
        void HandleAddAffect(uint64 guid);
		

        SpellEntry * m_spellInfo;
        Item* m_CastItem;
        SpellCastTargets m_targets;

    protected:

		
		Unit* unitTarget;
		
		GameObject* gameObjTarget;
		Player* playerTarget, *playerCaster;
		
		uint32 damage;

		
		void EffectSchoolDMG(uint32 i);
		void EffectTepeportUnits(uint32 i);
		void EffectApplyAura(uint32 i);
		void EffectPowerDrain(uint32 i);
		void EffectHeal(uint32 i);
		void EffectWeaponDmgNS(uint32 i);
		void EffectCreateItem(uint32 i);
		void EffectPresistentAA(uint32 i);
		void EffectEnergize(uint32 i);
		void EffectWeaponDmgPerc(uint32 i);
		void EffectOpenLock(uint32 i);
		void EffectOpenSecretSafe(uint32 i);
		void EffectApplyAA(uint32 i);
		void EffectLearnSpell(uint32 i);
		void EffectSummonWild(uint32 i);
		void EffectEnchantItemPerm(uint32 i);
		void EffectEnchantItemTmp(uint32 i);
		void EffectSummonPet(uint32 i);
		void EffectWeaponDmg(uint32 i);
		void EffectHealMaxHealth(uint32 i);
		void EffectInterruptCast(uint32 i);
		void EffectAddComboPoints(uint32 i);
		void EffectDuel(uint32 i);
		void EffectSummonTotem(uint32 i);
		void EffectEnchantHeldItem(uint32 i);
		void EffectSummonObject(uint32 i);
		void EffectResurrect(uint32 i);

		

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
        int32 m_timer;
        uint32 m_intervalTimer;                   
        uint32 TriggerSpellId;                    

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        bool m_triggeredSpell;
        Affect* m_triggeredByAffect;
        bool m_AreaAura;
		
		uint64 m_currdynObjID;

};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH = 20,
    REPLENISH_MANA = 21,
    REPLENISH_RAGE = 22                           
};


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
	const uint32& i_push_type;
	SpellNotifierCreatureAndPlayer(Spell &spell, std::list<uint64> &data, const uint32 &i,const uint32 &type) : i_spell(spell), i_data(data), i_index(i), i_push_type(type){}

	template<class T> inline void Visit(std::map<OBJECT_HANDLE, T *>  &m)
	{
	    for(typename std::map<OBJECT_HANDLE, T*>::iterator itr=m.begin(); itr != m.end(); ++itr)
	    {
		
		    switch(i_push_type) 
			{ 
			case PUSH_IN_FRONT:  
		        if(i_spell.m_caster->isInFront((Unit*)(itr->second),GetRadius(sSpellRadius.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index]))) && (itr->second)->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != i_spell.m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE))
		            i_data.push_back(itr->second->GetGUID());
			break;
			case PUSH_SELF_CENTER:  
				if( i_spell._CalcDistance(i_spell.m_caster->GetPositionX(), i_spell.m_caster->GetPositionY(), i_spell.m_caster->GetPositionZ(),
				                                 itr->second->GetPositionX(),itr->second->GetPositionY(),itr->second->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index])) && itr->second->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != i_spell.m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) )
		            i_data.push_back(itr->second->GetGUID());
		    break;
			case PUSH_DEST_CENTER:
				if( i_spell._CalcDistance(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ,
				                                 itr->second->GetPositionX(),itr->second->GetPositionY(),itr->second->GetPositionZ()) < GetRadius(sSpellRadius.LookupEntry(i_spell.m_spellInfo->EffectRadiusIndex[i_index])) && itr->second->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) != i_spell.m_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE) )
		            i_data.push_back(itr->second->GetGUID());
			break;
			}
	    }
	}

	
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
