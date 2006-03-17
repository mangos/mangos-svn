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

#ifndef MANGOSSERVER_CREATURE_H
#define MANGOSSERVER_CREATURE_H

#include "Unit.h"
#include "UpdateMask.h"
#include "MotionMaster.h"
#include "ItemPrototype.h"
#include "LootMgr.h"
struct SpellEntry;

class CreatureAI;
class Quest;
class Player;
class WorldSession;

#define UNIT_MOVEMENT_INTERPOLATE_INTERVAL 300    
#define MAX_CREATURE_WAYPOINTS 16
#define MAX_CREATURE_ITEMS 128

enum CreatureState
{
    STOPPED = 0,
    STUNDED = 1L, 
    ROAMING = (1L << 1),
    CHASE = (1L << 2),
    SEARCHING = (1L << 3),
    FLEEING = (1L << 4),
    MOVING = (ROAMING | CHASE | SEARCHING | FLEEING),
    ATTACKING = (1L << 5),
    ALL_STATE = (STOPPED | MOVING | SEARCHING | ATTACKING)
};


struct CreatureItem
{
    uint32 ItemId;
	uint32 amount;
};

struct TrainerSpell
{
	SpellEntry* spell;
	uint32 spellcost;
	uint32 reqspell;
};


struct CreatureInfo
{
    uint32	Entry;
    uint32	DisplayID;
    char*	Name;
    char*	SubName;
    uint32	maxhealth;
    uint32	maxmana;
    uint32	level;
    uint32	armor;
	uint32	faction;
    uint32	npcflag;
    float	speed;
    uint32	rank;
    float	mindmg;
    float	maxdmg;
	uint32	attackpower;
    uint32	baseattacktime;
    uint32	rangeattacktime;
	uint32	Flags;
    uint32	mount;
    uint32	level_max;
	uint32	dynamicflags;
	float	size;
    uint32	family;
    float	bounding_radius;
    uint32	trainer_type;
    uint32	classNum;
    float	minrangedmg;
	float	maxrangedmg;
	uint32	rangedattackpower;
	float	combat_reach;
    uint32	type;
	uint32	civilian;
	uint32	flag1;
	uint32	equipmodel[3];
	uint32	equipinfo[3];
	uint32	equipslot[3];
	uint32	lootid;
	uint32	SkinLootId;
	uint32	resistance1;
	uint32	resistance2;
	uint32	resistance3;
	uint32	resistance4;
	uint32	resistance5;
	uint32	resistance6;
	char*	AIName;
    char*	MovementGen;
	char*	ScriptName;
};

enum UNIT_TYPE
{
    NOUNITTYPE = 0,
    BEAST      = 1,
    DRAGONSKIN = 2,
    DEMON      = 3,
    ELEMENTAL  = 4,
    GIANT      = 5,
    UNDEAD     = 6,
    HUMANOID   = 7,
    CRITTER    = 8,
    MECHANICAL = 9,
};




class Creature : public Unit
{
	CreatureAI *i_AI;
	MotionMaster i_motionMaster;

	public:

        Creature();
        virtual ~Creature();

		typedef std::list<TrainerSpell*> SpellsList;

        bool Create (uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 Entry);
		bool CreateFromProto(uint32 guidlow,uint32 Entry);
		




    
				virtual void Update( uint32 time );
				inline void GetRespawnCoord(float &x, float &y, float &z) const { x = respawn_cord[0]; y = respawn_cord[1]; z = respawn_cord[2]; }
		
		    
				inline bool TestState(unsigned mask) const { return (i_creatureState & mask); }
				inline void SetState(unsigned mask) { i_creatureState |= mask; }
				inline void ClearState(unsigned mask) { i_creatureState &= ~mask; }
				bool isPet() { return m_isPet; }
				void SetisPet(bool val) { m_isPet=val; }
		
		
		    
				void AIM_Update(const uint32 &);
				void AIM_Initialize(void);
				MotionMaster* operator->(void) { return &i_motionMaster; }
		
		    
				void AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run);
				inline CreatureAI &AI(void) { return *i_AI; }
		
		    
				inline void setMoveRandomFlag(bool f) { m_moveRandom = f; }
				inline void setMoveRunFlag(bool f) { m_moveRun = f; }
				inline bool getMoveRandomFlag() { return m_moveRandom; }
				inline bool getMoveRunFlag() { return m_moveRun; }
				inline bool IsStopped(void) const { return !(TestState(MOVING)); }
				inline void StopMoving(void) { ClearState(MOVING); }
				inline const float& GetMobSpeed(void) const { return m_moveSpeed; }
		
		    
				void setItemId(int slot, uint32 tempitemid);
				void setItemAmount(int slot, int tempamount);
				void setItemAmountById(uint32 tempitemid, int tempamount);
		
				void increaseItemCount() { itemcount++; }
				void addItem(uint32 itemid, uint32 amount);
				int getItemCount() { return itemcount; }
				int getItemSlotById(uint32 itemid);
				
				int getItemAmount(int slot) { return item_list[slot].amount; }
				uint32 getItemId(int slot) { return item_list[slot].ItemId; }
				ItemPrototype *getProtoByslot(uint32 slot);
				
				CreatureInfo *GetCreatureInfo();
				
				void CreateTrainerSpells();
				uint32 GetTrainerSpellsSize(){ return m_tspells.size(); }
				std::list<TrainerSpell*>::iterator GetTspellsBegin(){ return m_tspells.begin(); }
				std::list<TrainerSpell*>::iterator GetTspellsEnd(){ return m_tspells.end(); }
			
				uint32 getDialogStatus(Player *pPlayer, uint32 defstatus);
				Quest *getNextAvailableQuest(Player *pPlayer, Quest *prevQuest);
		
				bool hasQuest(uint32 quest_id);
				bool hasInvolvedQuest(uint32 quest_id);
		
				void addQuest(Quest *quest) { mQuests.push_back(quest); }
				void addInvolvedQuest(Quest *quest) { mInvolvedQuests.push_back(quest); }
		
				void prepareQuestMenu( Player *pPlayer );

     
        void generateLoot();
       

        
        
        
        inline void setEmoteState(uint8 emote) { m_emoteState = emote; };

        virtual void setDeathState(DeathState s)
        {
            m_deathState = s;
            if(s == JUST_DIED)
            {
                m_deathTimer = m_corpseDelay;
            }
        };

        
        void SaveToDB();
        void LoadFromDB(uint32 guid);
        void DeleteFromDB();

        
        uint32 getFaction()
        {
            return m_faction;
        };
		Loot loot;
	
    protected:
        void _LoadGoods();
        void _LoadQuests();
        void _LoadMovement();
        void _RealtimeSetCreatureInfo();

        
        uint32 m_lootMoney;

        /// Timers
        uint32 m_deathTimer;                      // timer for death or corpse disappearance
        uint32 m_respawnTimer;                    // timer for respawn to happen
        uint32 m_respawnDelay;                    // delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                     // delay between death and corpse disappearance
		float m_respawnradius;
        CreatureItem item_list[MAX_CREATURE_ITEMS];
        int itemcount;
        
      
        SpellsList m_tspells;
        
        uint32 mTaxiNode;

        
        std::list<Quest*> mQuests;
        std::list<Quest*> mInvolvedQuests;

        
        float respawn_cord[3];
        bool m_moveBackward;
        bool m_moveRandom;
        bool m_moveRun;
        float m_moveSpeed;
        unsigned i_creatureState;   

        
        uint32 m_faction;
        uint8 m_emoteState;
        bool m_isPet; //add by vendy        
        float GetAttackDistance(Unit *pl);

};
#endif
