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
    uint32 itemid;
    int amount;
};

struct CreatureInfo
{
    
    CreatureInfo() : Id(0), Type(0), DisplayID(0), unknown1(0), unknown2(0), unknown3(0), unknown4(0) {}
    uint32 Id;
    uint32 Type;
    uint32 DisplayID;
    uint32 unknown1;  
    uint32 unknown2;  
    uint32 unknown3;  
    uint32 unknown4;
    uint32 maxhealth;
    uint32 maxmana;
    uint32 level;
    uint32 faction;
    uint32 flag;
    float scale;
    float speed;
    uint32 rank;
    float mindmg;
    float maxdmg;
    uint32 baseattacktime;
    uint32 rangeattacktime;
    uint32 mount;
    uint32 level_max;
    uint32 flags1;
    float size;
    uint32 family;
    float bounding_radius;
    uint32 trainer_type;
    uint32 classNum;
    uint32 slot1model;
    uint32 slot1pos;
    uint32 slot2model;
    uint32 slot2pos;
    uint32 slot3model;
    uint32 slot3pos;
    std::string Name;
    std::string SubName;
    std::string AIName;
    std::string MovementGen;
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

        
        void Create (uint32 guidlow, const char* creature_name, uint32 mapid,
		     float x, float y, float z, float ang, uint32 nameId);
    
    
    virtual void Update( uint32 time );
    inline void GetRespawnCoord(float &x, float &y, float &z) const { x = respawn_cord[0]; y = respawn_cord[1]; z = respawn_cord[2]; }

    
    inline bool TestState(unsigned mask) const { return (i_creatureState & mask); }
    inline void SetState(unsigned mask) { i_creatureState |= mask; }
    inline void ClearState(unsigned mask) { i_creatureState &= ~mask; }


    
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

    
    void setItemId(int slot, uint32 tempitemid) { item_list[slot].itemid = tempitemid; }
    void setItemAmount(int slot, int tempamount) { item_list[slot].amount = tempamount; }
    void setItemAmountById(uint32 tempitemid, int tempamount)
    {
	int i;
	for(i=0;i<itemcount;i++)
	{
	    if(item_list[i].itemid == tempitemid)
		item_list[i].amount = tempamount;
	}
    }

    void increaseItemCount() { itemcount++; }
    void addItem(uint32 itemid, uint32 amount)
    {
	item_list[itemcount].amount = amount;
	item_list[itemcount].itemid = itemid;
	itemcount++;
    }
    int getItemCount() { return itemcount; }
    int getItemSlotById(uint32 itemid)
    {
	int i;
	for(i=0;i<itemcount;i++)
	{
	    if(item_list[i].itemid == itemid)
		return i;
	}
	return -1;
    }
    int getItemAmount(int slot) { return item_list[slot].amount; }
    uint32 getItemId(int slot) { return item_list[slot].itemid; }


	
    
	
    uint32 getDialogStatus(Player *pPlayer, uint32 defstatus);
	Quest *getNextAvailableQuest(Player *pPlayer, Quest *prevQuest);

    bool hasQuest(uint32 quest_id);
    bool hasInvolvedQuest(uint32 quest_id);

    void addQuest(uint32 quest_id) { mQuestIds.push_back(quest_id); };
    void addInvolvedQuest(uint32 quest_id) { mInvolvedQuestIds.push_back(quest_id); };

	void prepareQuestMenu( Player *pPlayer );


        
        void generateLoot();
        uint32 getLootMoney() { return m_lootMoney; }
        void setLootMoney(uint32 amount) { m_lootMoney = amount; }

        
        const char* GetName() const { return m_name.c_str(); };
        void SetName(const char* name) { m_name = name; }

        
        inline void setEmoteState(uint8 emote) { m_emoteState = emote; };
        uint8 getMovementState() { return m_movementState; };
        uint8 setMovementState(uint8 movement) { m_movementState = movement; };

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

    protected:
        void _LoadGoods();
        void _LoadQuests();
        void _LoadMovement();
        void _RealtimeSetCreatureInfo(void);

        
        uint32 m_lootMoney;

        
        uint8 m_movementState;

        
        uint32 m_deathTimer;                      
        uint32 m_respawnTimer;                    
        uint32 m_moveTimer;                       
        uint32 m_respawnDelay;                    
        uint32 m_corpseDelay;                     

        
        CreatureItem item_list[MAX_CREATURE_ITEMS];
        int itemcount;

        
        uint32 mTaxiNode;

        
        std::list<uint32> mQuestIds;
        std::list<uint32> mInvolvedQuestIds;

        
        float respawn_cord[3];

        
       bool m_enabled;

        
        bool m_moveBackward;
        bool m_moveRandom;
        bool m_moveRun;
        float m_moveSpeed;
        unsigned i_creatureState;   

        
        uint32 m_faction;

        
        uint8 m_emoteState;
        std::string m_name;
        
        
        float GetAttackDistance(Unit *pl);

};
#endif
