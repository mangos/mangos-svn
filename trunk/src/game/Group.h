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

#ifndef MANGOSSERVER_GROUP_H
#define MANGOSSERVER_GROUP_H

#define MAXGROUPSIZE 5

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    NOT_EMITED_YET    = 3,
    NOT_VALID         = 4
};

class Group
{
    public:
        Group()
        {
            m_count = 0;
            m_leaderGuid = 0;
            m_lootMethod = FREE_FOR_ALL;
            m_looterGuid = 0;
            m_grouptype = 0;
        }

        ~Group()
        {
        }

        bool Create(const uint64 &guid, const char * name)
        {
            AddMember(guid, name);

            m_leaderGuid = guid;
            m_leaderName = name;
            return true;
        }

        void AddMember(uint64 guid, const char* name)
        {

            if (m_count < MAXGROUPSIZE)
            {
                m_members[m_count].guid = guid;
                m_members[m_count].name = name;
                m_count++;
            }
            else
            {
                ;

            }
        }

        uint32 RemoveMember(const uint64 &guid);
        void RemoveRollsFromMember(const uint64 &guid);
        void ChangeLeader(const uint64 &guid);

        bool IsFull() const { return m_count == MAXGROUPSIZE; }

        void SendUpdate();
        void Disband();

        const uint64& GetLeaderGUID() const { return m_leaderGuid; }

        void SetLootMethod(LootMethod method) { m_lootMethod = method; }
        void SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }

        LootMethod GetLootMethod() const { return m_lootMethod; }
        const uint64 & GetLooterGuid() const { return m_looterGuid; }

        uint32 GetMembersCount() const { return m_count; }
        const uint64& GetMemberGUID(uint32 i) const { ASSERT(i < m_count); return m_members[i].guid; }

        bool GroupCheck(uint64 guid)
        {
            for(uint32 i = 0; i < m_count; i++ )
            {
                if (m_members[i].guid == guid)
                {
                    return true;
                }
            }
            return false;
        }

        int8 GetPlayerGroupSlot(uint64 Guid);

        struct Roll
        {
            uint64 itemGUID;
            uint32 itemid;
            RollVote playerVote[MAXGROUPSIZE];              //vote position correspond with player position (in group)
            uint8 totalPlayersRolling;
            uint8 totalNeed;
            uint8 totalGreed;
            uint8 totalPass;
            Loot *loot;
            uint8 itemSlot;

            Roll()
                : itemGUID(0), itemid(0), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), loot(NULL), itemSlot(0) {}
        };

        void BroadcastToGroup(WorldSession *session, std::string msg);
        void BroadcastPacket(WorldPacket *packet);
        void SendLootStartRoll(uint64 Guid, uint32 NumberinGroup, uint32 ItemEntry, uint32 ItemInfo, uint32 CountDown, const Roll &r);
        void SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, uint32 ItemEntry, uint32 ItemInfo, const Roll &r);
        void GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature);
        void NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature);
        void CountTheRoll(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise);

        vector<Roll> RollId;

    protected:

        typedef struct
        {
            std::string name;
            uint64 guid;
        } MemberSlot;

        MemberSlot m_members[MAXGROUPSIZE];

        uint64 m_leaderGuid;
        std::string m_leaderName;

        uint32 m_count;
        uint16 m_grouptype;

        LootMethod m_lootMethod;
        uint64 m_looterGuid;
};
#endif
