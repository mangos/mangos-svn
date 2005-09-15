/* Creature.h
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

#ifndef MANGOSSERVER_CREATURE_H
#define MANGOSSERVER_CREATURE_H

#include "Unit.h"
#include "UpdateMask.h"

class Quest;
class Player;
class WorldSession;

#define UNIT_MOVEMENT_INTERPOLATE_INTERVAL 300    // ms
#define MAX_CREATURE_WAYPOINTS 16
#define MAX_CREATURE_ITEMS 128

enum CreatureState
{
    STOPPED,
    MOVING,
    ATTACKING
};

// we do not need complicated inventory here (probably)
struct CreatureItem
{
    uint32 itemid;
    int amount;
};

struct CreatureInfo
{
    // ctor in case users forget to intialized
    CreatureInfo() : Id(0), Type(0), DisplayID(0), unknown1(0), unknown2(0), unknown3(0), unknown4(0) {}
    uint32 Id;
    uint32 Type;
    uint32 DisplayID;
    uint32 unknown1;
    uint32 unknown2;
    uint32 unknown3;
    uint32 unknown4;
    std::string Name;
    std::string SubName;
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

///////////////////
/// Creature object

class Creature : public Unit
{
    public:

        Creature();
        virtual ~Creature();

        /// Creation
        void Create (uint32 guidlow, const char* creature_name, uint32 mapid,
            float x, float y, float z, float ang);

        /// Updates
        void UpdateMobMovement( uint32 p_time );
        virtual void Update( uint32 time );

        /// AI
        void AI_ToggleAI(bool useAI) { m_useAI = useAI; }
        void AI_Update();
        void AI_AttackReaction(Unit *pAttacker, uint32 damage_dealt);
        void AI_SendMoveToPacket(float x, float y, float z, uint32 time, bool run);
        void AI_ChangeState(CreatureState state) { m_creatureState = state; }
        void AI_MoveTo(float x, float y, float z, bool run);

        /// Movement
        bool addWaypoint(float x, float y, float z);
        inline bool hasWaypoints() { return m_nWaypoints > 0; }
        inline void setMoveRandomFlag(bool f) { m_moveRandom = f; }
        inline void setMoveRunFlag(bool f) { m_moveRun = f; }
        inline bool getMoveRandomFlag() { return m_moveRandom; }
        inline bool getMoveRunFlag() { return m_moveRun; }

        /// Creature inventory
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

        /// Quests
        uint32 getQuestStatus(Player *pPlayer);
        uint32 getCurrentQuest(Player *pPlayer);

        char* getQuestTitle(uint32 quest_id);
        char* getQuestDetails(uint32 quest_id);
        char* getQuestObjectives(uint32 quest_id);
        char* getQuestCompleteText(uint32 quest_id);
        char* getQuestIncompleteText(uint32 quest_id);

        bool hasQuest(uint32 quest_id);
        void addQuest(uint32 quest_id) { mQuestIds.push_back(quest_id); };
        void removeQuest(uint32 quest_id);

        /// Looting
        void generateLoot();
        uint32 getLootMoney() { return m_lootMoney; }
        void setLootMoney(uint32 amount) { m_lootMoney = amount; }

        /// Name
        const char* GetName() const { return m_name.c_str(); };
        void SetName(const char* name) { m_name = name; }

        /// Misc
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

        // Serialization
        void SaveToDB();
        void LoadFromDB(uint32 guid);
        void DeleteFromDB();

    protected:
        void _LoadGoods();
        void _LoadQuests();
        void _LoadMovement();

	// use internally to generate a sequence of number
    // given a start index (defaulted to be 0 - that's
    // the first number generated.. SequenceGen will
    // generate a sequence of number every time the
    // operator() is call.
    struct SequenceGen
    {
		SequenceGen(short start_idx = -1) : i_current(start_idx) {}
		short operator()(void) { return ++i_current; }
		short i_current;
    };

        /// Looting
        uint32 m_lootMoney;

        /// Spells/Skills
        uint8 m_movementState;

        /// Timers
        uint32 m_deathTimer;                      // timer for death or corpse disappearance
        uint32 m_respawnTimer;                    // timer for respawn to happen
        uint32 m_moveTimer;                       // timer creature moves
        uint32 m_respawnDelay;                    // delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                     // delay between death and corpse disappearance

        /// Vendor data
        CreatureItem item_list[MAX_CREATURE_ITEMS];
        int itemcount;

        /// Taxi data
        uint32 mTaxiNode;

        /// Quest data
        std::list<uint32> mQuestIds;

        /// Respawn Coordenates
        float respawn_cord[3];

        /// AI data
        bool m_useAI;

        /// Movement
        uint32 m_currentWaypoint;
        bool m_moveBackward;
        bool m_moveRandom;
        bool m_moveRun;
        CreatureState m_creatureState;
        uint32 m_nWaypoints;
        // will be changed to list
        float m_waypoints[MAX_CREATURE_WAYPOINTS][3];
        float m_moveSpeed;

        float m_destinationX;
        float m_destinationY;
        float m_destinationZ;

        uint32 m_timeToMove;
        uint32 m_timeMoved;

        /// Misc
        uint8 m_emoteState;
        std::string m_name;
};
#endif
