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
#define MAXRAIDSIZE 40
#define TARGETICONCOUNT 8

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    NOT_EMITED_YET    = 3,
    NOT_VALID         = 4
};
enum GroupType
{
    GROUPTYPE_NORMAL = 0,
    GROUPTYPE_RAID   = 1
};


/** request member stats checken **/
/** todo: uninvite people that not accepted invite **/
class Group
{
    protected:
        struct MemberSlot
        {
            uint64      guid;
            std::string name;
            uint8       group;
            bool        assistant;
        };
        struct Roll
        {
            Roll(): itemGUID(0), itemid(0), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), loot(NULL), itemSlot(0) {}

            uint64 itemGUID;
            uint32 itemid;
            map<uint64, RollVote> playerVote;              //vote position correspond with player position (in group)
            uint8 totalPlayersRolling;
            uint8 totalNeed;
            uint8 totalGreed;
            uint8 totalPass;
            Loot *loot;
            uint8 itemSlot;            
        };



    public:        
        Group()
        {
            m_leaderGuid = 0;
            m_groupType  = (GroupType)0;
            m_lootMethod = (LootMethod)0;
            m_looterGuid = 0;
            for(int i=0; i<TARGETICONCOUNT; i++)
                m_targetIcons[i] = 0;
        }
        ~Group() {}

        // group manipulation methods
        void   Create(const uint64 &guid, const char * name);
        void   LoadRaidGroupFromDB(const uint64 &leaderGuid);
        bool   AddInvite(Player *player);
        void   RemoveInvite(const uint64 &guid);
        bool   AddMember(const uint64 &guid, const char* name);
        uint32 RemoveMember(const uint64 &guid, const uint8 &method);   // method: 0=just remove, 1=kick
        void   ChangeLeader(const uint64 &guid);
        void   SetLootMethod(LootMethod method) { m_lootMethod = method; }
        void   SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }
        void   Disband(bool hideDestroy=false);

        // properties accessories
        bool IsFull() const { return (m_groupType==GROUPTYPE_NORMAL) ? (m_members.size()>=MAXGROUPSIZE) : (m_members.size()>=MAXRAIDSIZE); }
        bool isRaidGroup() { return (m_groupType==GROUPTYPE_RAID); }
        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        LootMethod    GetLootMethod() const { return m_lootMethod; }
        const uint64& GetLooterGuid() const { return m_looterGuid; }

        // member manipulation methods
        bool IsMember(uint64 guid);
        bool IsLeader(uint64 guid) { return (GetLeaderGUID() == guid); }
        bool IsAssistant(uint64 guid)
        {
            uint8 id = _getMemberIndex(guid);
            if(id<=0)
                return false;

            return m_members[id].assistant;
        }
        bool SameSubGroup(uint64 guid1, uint64 guid2)
        {
            uint8 id1 = _getMemberIndex(guid1);
            uint8 id2 = _getMemberIndex(guid2);
            if(id1<=0 || id2<=0)
                return false;

            return (m_members[id1].group==m_members[id2].group);
        }

        uint32 GetMembersCount() const { return m_members.size(); }
        uint64 GetMemberGUID(uint8 id) { if(id>=m_members.size()) return 0; else return m_members[id].guid; }
        uint8  GetMemberGroup(uint64 guid) 
        { 
            uint8 id = _getMemberIndex(guid);
            if(id<=0) 
                return (MAXRAIDSIZE/MAXGROUPSIZE+1); 

            return m_members[id].group; 
        }        

        // some additional raid methods
        void ConvertToRaid();
        void ChangeMembersGroup(const uint64 &guid, const uint8 &group)
        { 
            if(!isRaidGroup())
                return;
            if(_setMembersGroup(guid, group))
                SendUpdate();
        }
        void ChangeAssistantFlag(const uint64 &guid, const bool &state)
        { 
            if(!isRaidGroup())
                return;
            if(_setAssistantFlag(guid, state))
                SendUpdate();
        }

        void SetTargetIcon(uint8 id, uint64 guid);     

        // -no description-
        void SendInit(WorldSession *session);
        void SendTargetIconList(WorldSession *session);
        void SendUpdate(); 
        void BroadcastPacket(WorldPacket *packet, int group=-1, uint64 ignore=0); // ignore: GUID of player that will be ignored

        // roll system
        void SendLootStartRoll(uint64 Guid, uint32 NumberinGroup, uint32 ItemEntry, uint32 ItemInfo, uint32 CountDown, const Roll &r);
        void SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint32 ItemEntry, uint32 ItemInfo, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, uint32 ItemEntry, uint32 ItemInfo, const Roll &r);
        void GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature);
        void NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature);
        void CountTheRoll(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise);

        
    protected:
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant=false);
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group);
        bool _removeMember(const uint64 &guid); // returns true if leader has changed
        void _setLeader(const uint64 &guid);

        void _removeRolls(const uint64 &guid);

        void _convertToRaid();
        bool _setMembersGroup(const uint64 &guid, const uint8 &group);
        bool _setAssistantFlag(const uint64 &guid, const bool &state);

        int8 _getMemberIndex(uint64 Guid);


        vector<MemberSlot> m_members;
        vector<uint64> m_invitees;
        uint64       m_leaderGuid;
        std::string  m_leaderName;
        GroupType    m_groupType;          
        uint64       m_targetIcons[TARGETICONCOUNT];        
        LootMethod   m_lootMethod;
        uint64       m_looterGuid;
        vector<Roll> RollId;
};
#endif
