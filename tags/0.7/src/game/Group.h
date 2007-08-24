/* 
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
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

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE      = 0,
    MEMBER_STATUS_ONLINE       = 1,
    MEMBER_STATUS_OFFLINE_PVP  = 2,
    MEMBER_STATUS_ONLINE_PVP   = 3
};

enum GroupType
{
    GROUPTYPE_NORMAL = 0,
    GROUPTYPE_RAID   = 1
};

/** request member stats checken **/
/** todo: uninvite people that not accepted invite **/
class MANGOS_DLL_SPEC Group
{
    public:
        struct MemberSlot
        {
            uint64      guid;
            std::string name;
            uint8       group;
            bool        assistant;
        };
        typedef std::list<MemberSlot> MemberList;
        typedef MemberList::const_iterator member_citerator;
    protected:
        typedef MemberList::iterator member_witerator;
        typedef std::set<uint64> InvitesList;

        struct Roll
        {
            Roll(): itemGUID(0), itemid(0), itemRandomPropId(0), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), loot(NULL), itemSlot(0) {}

            uint64 itemGUID;
            uint32 itemid;
            uint32 itemRandomSuffix;
            int32  itemRandomPropId;
            map<uint64, RollVote> playerVote;               //vote position correspond with player position (in group)
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
            m_mainTank   = 0;
            m_mainAssistant =0;
            m_groupType  = (GroupType)0;
            m_bgGroup    = false;
            m_lootMethod = (LootMethod)0;
            m_looterGuid = 0;
            m_lootThreshold = (LootThreshold)2;
            for(int i=0; i<TARGETICONCOUNT; i++)
                m_targetIcons[i] = 0;
        }
        ~Group() {}

        // group manipulation methods
        bool   Create(const uint64 &guid, const char * name);
        bool   LoadGroupFromDB(const uint64 &leaderGuid);
        bool   AddInvite(Player *player);
        uint32 RemoveInvite(Player *player);
        bool   AddMember(const uint64 &guid, const char* name);
                                                            // method: 0=just remove, 1=kick
        uint32 RemoveMember(const uint64 &guid, const uint8 &method);
        void   ChangeLeader(const uint64 &guid);
        void   SetLootMethod(LootMethod method) { m_lootMethod = method; }
        void   SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }
        void   SetLootThreshold(LootThreshold threshold) { m_lootThreshold = threshold; }
        void   Disband(bool hideDestroy=false);

        // properties accessories
        bool IsFull() const { return (m_groupType==GROUPTYPE_NORMAL) ? (m_members.size()>=MAXGROUPSIZE) : (m_members.size()>=MAXRAIDSIZE); }
        bool isRaidGroup() { return (m_groupType==GROUPTYPE_RAID); }
        bool isBGGroup() { return m_bgGroup; }
        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        LootMethod    GetLootMethod() const { return m_lootMethod; }
        const uint64& GetLooterGuid() const { return m_looterGuid; }
        LootThreshold GetLootThreshold() const { return m_lootThreshold; }

        // member manipulation methods
        bool IsMember(uint64 guid) const { return _getMemberCSlot(guid) != m_members.end(); }
        bool IsLeader(uint64 guid) const { return (GetLeaderGUID() == guid); }
        bool IsAssistant(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_members.end())
                return false;

            return mslot->assistant;
        }
        uint64 GetNextGuidAfter(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_members.end())
                return 0;
            ++mslot;
            if(mslot==m_members.end())
                return 0;
            return mslot->guid;
        }
        bool SameSubGroup(uint64 guid1, uint64 guid2) const
        {
            member_citerator mslot2 = _getMemberCSlot(guid2);
            if(mslot2==m_members.end())
                return false;

            return SameSubGroup(guid1,&*mslot2);
        }

        bool SameSubGroup(uint64 guid1, MemberSlot const* slot2) const
        {
            member_citerator mslot1 = _getMemberCSlot(guid1);
            if(mslot1==m_members.end() || !slot2)
                return false;

            return (mslot1->group==slot2->group);
        }

        MemberList const& GetMembers() const { return m_members; }
        uint32 GetMembersCount() const { return m_members.size(); }
        uint32 GetMemberCountForXPAtKill(Unit const* victim);
        Player* GetMemberForXPAtKill(uint64 guid, Unit const* victim);
        uint8  GetMemberGroup(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_members.end())
                return (MAXRAIDSIZE/MAXGROUPSIZE+1);

            return mslot->group;
        }

        // some additional raid methods
        void ConvertToRaid()
        {
            _convertToRaid();
            SendUpdate();
        }
        void SetBattlegroundGroup(const bool &state) { m_bgGroup = state; }
        void ChangeMembersGroup(const uint64 &guid, const uint8 &group)
        {
            if(!isRaidGroup())
                return;
            if(_setMembersGroup(guid, group))
                SendUpdate();
        }
        void SetAssistant(const uint64 &guid, const bool &state)
        {
            if(!isRaidGroup())
                return;
            if(_setAssistantFlag(guid, state))
                SendUpdate();
        }
        void SetMainTank(const uint64 &guid)
        {
            if(!isRaidGroup())
                return;

            if(_setMainTank(guid))
                SendUpdate();
        }
        void SetMainAssistant(const uint64 &guid)
        {
            if(!isRaidGroup())
                return;

            if(_setMainAssistant(guid))
                SendUpdate();
        }

        void SetTargetIcon(uint8 id, uint64 guid);

        // -no description-
        //void SendInit(WorldSession *session);
        void SendTargetIconList(WorldSession *session);
        void SendUpdate();
                                                            // ignore: GUID of player that will be ignored
        void BroadcastPacket(WorldPacket *packet, int group=-1, uint64 ignore=0);

        /*********************************************************/
        /***                   LOOT SYSTEM                     ***/
        /*********************************************************/

        void SendLootStartRoll(uint64 Guid, uint32 CountDown, const Roll &r);
        void SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(uint64 Guid, uint32 NumberOfPlayers, const Roll &r);
        void GroupLoot(uint64 playerGUID, Loot *loot, Creature *creature);
        void NeedBeforeGreed(uint64 playerGUID, Loot *loot, Creature *creature);
        vector<Roll>::iterator GetRoll(uint64 Guid)
        {
            vector<Roll>::iterator iter;
            for (iter=RollId.begin(); iter != RollId.end(); ++iter)
            {
                if (iter->itemGUID == Guid)
                {
                    return iter;
                }
            }
            return RollId.end();
        }
        void CountTheRoll(vector<Roll>::iterator roll, uint32 NumberOfPlayers);
        void CountRollVote(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise);
        void EndRoll();

    protected:
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant=false);
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group);
        bool _removeMember(const uint64 &guid);             // returns true if leader has changed
        void _setLeader(const uint64 &guid);

        void _removeRolls(const uint64 &guid);

        void _convertToRaid();
        bool _setMembersGroup(const uint64 &guid, const uint8 &group);
        bool _setAssistantFlag(const uint64 &guid, const bool &state);
        bool _setMainTank(const uint64 &guid);
        bool _setMainAssistant(const uint64 &guid);

        member_citerator _getMemberCSlot(uint64 Guid) const
        {
            for(member_citerator itr = m_members.begin(); itr != m_members.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_members.end();
        }

        member_witerator _getMemberWSlot(uint64 Guid)
        {
            for(member_witerator itr = m_members.begin(); itr != m_members.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_members.end();
        }

        MemberList   m_members;
        InvitesList  m_invitees;
        uint64       m_leaderGuid;
        std::string  m_leaderName;
        uint64       m_mainTank;
        uint64       m_mainAssistant;
        GroupType    m_groupType;
        bool         m_bgGroup;
        uint64       m_targetIcons[TARGETICONCOUNT];
        LootMethod   m_lootMethod;
        LootThreshold m_lootThreshold;
        uint64       m_looterGuid;
        vector<Roll> RollId;
};
#endif
