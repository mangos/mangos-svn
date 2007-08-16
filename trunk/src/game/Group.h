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

#include "GroupReference.h"
#include "GroupRefManager.h"

#include <map>
#include <vector>

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
    MEMBER_STATUS_OFFLINE   = 0x00,
    MEMBER_STATUS_ONLINE    = 0x01,
    MEMBER_STATUS_PVP       = 0x02,
    MEMBER_STATUS_UNK0      = 0x04, // dead? (health=0)
    MEMBER_STATUS_UNK1      = 0x08, // ghost? (health=1)
    MEMBER_STATUS_UNK2      = 0x10, // never seen
    MEMBER_STATUS_UNK3      = 0x20, // never seen
    MEMBER_STATUS_UNK4      = 0x40, // appears with dead and ghost flags
    MEMBER_STATUS_UNK5      = 0x80, // never seen
};

enum GroupType
{
    GROUPTYPE_NORMAL = 0,
    GROUPTYPE_RAID   = 1
};

class BattleGround;

enum GroupUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,
    GROUP_UPDATE_FLAG_ONLINE            = 0x00000001, // uint8, flags
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000002, // uint16
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000004, // uint16
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000008, // uint8
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000010, // uint16
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000020, // uint16
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000040, // uint16
    GROUP_UPDATE_FLAG_ZONE              = 0x00000080, // uint16
    GROUP_UPDATE_FLAG_POSITION          = 0x00000100, // uint16, uint16
    GROUP_UPDATE_FLAG_AURAS             = 0x00000200, // uint64 mask, for each bit set uint16 spellid?
    GROUP_UPDATE_FLAG_PET_GUID          = 0x00000400, // uint64 pet guid
    GROUP_UPDATE_FLAG_PET_NAME          = 0x00000800, // pet name, NULL terminated string
    GROUP_UPDATE_FLAG_PET_MODEL_ID      = 0x00001000, // uint16, model id
    GROUP_UPDATE_FLAG_PET_CUR_HP        = 0x00002000, // uint16 pet cur health
    GROUP_UPDATE_FLAG_PET_MAX_HP        = 0x00004000, // uint16 pet max health
    GROUP_UPDATE_FLAG_PET_POWER_TYPE    = 0x00008000, // uint8 pet power type
    GROUP_UPDATE_FLAG_PET_CUR_POWER     = 0x00010000, // uint16 pet cur power
    GROUP_UPDATE_FLAG_PET_MAX_POWER     = 0x00020000, // uint16 pet max power
    GROUP_UPDATE_FLAG_PET_AURAS         = 0x00040000, // uint64 mask, for each bit set uint16 spellid?, pet auras...
    //GROUP_UPDATE_FLAG_UNK1              = 0x00080000, // unused
    //GROUP_UPDATE_FLAG_UNK2              = 0x00100000, // unused
    //GROUP_UPDATE_FLAG_UNK3              = 0x00200000, // unused
    //GROUP_UPDATE_FLAG_UNK4              = 0x00400000, // unused
    //GROUP_UPDATE_FLAG_UNK5              = 0x00800000, // unused
    //GROUP_UPDATE_FLAG_UNK6              = 0x01000000, // unused
    //GROUP_UPDATE_FLAG_UNK7              = 0x02000000, // unused
    //GROUP_UPDATE_FLAG_UNK8              = 0x04000000, // unused
    //GROUP_UPDATE_FLAG_UNK9              = 0x08000000, // unused
    //GROUP_UPDATE_FLAG_UNK10             = 0x10000000, // unused
    //GROUP_UPDATE_FLAG_UNK11             = 0x20000000, // unused
    //GROUP_UPDATE_FLAG_UNK12             = 0x40000000, // unused
    GROUP_UPDATE_FULL                   = 0x000001FF,
    GROUP_UPDATE_FLAGS_COUNT            = 10
};

                                                                 //0, 1, 2, 3, 4, 5, 6, 7, 8, 9
static const uint8 GroupUpdateLength[GROUP_UPDATE_FLAGS_COUNT] = { 0, 1, 2, 2, 1, 2, 2, 2, 2, 4};

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
        typedef std::list<MemberSlot> MemberSlotList;
        typedef MemberSlotList::const_iterator member_citerator;
    protected:
        typedef MemberSlotList::iterator member_witerator;
        typedef std::set<uint64> InvitesList;

        struct Roll
        {
            Roll(): itemGUID(0), itemid(0), itemRandomPropId(0), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), loot(NULL), itemSlot(0) {}

            uint64 itemGUID;
            uint32 itemid;
            uint32 itemRandomSuffix;
            int32  itemRandomPropId;
            typedef std::map<uint64, RollVote> PlayerVote;
            PlayerVote playerVote;               //vote position correspond with player position (in group)
            uint8 totalPlayersRolling;
            uint8 totalNeed;
            uint8 totalGreed;
            uint8 totalPass;
            Loot *loot;
            uint8 itemSlot;
        };

        typedef std::vector<Roll> Rolls;

    public:
        Group();
        ~Group();

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
        void   SetLootThreshold(ItemQualities threshold) { m_lootThreshold = threshold; }
        void   Disband(bool hideDestroy=false);

        // properties accessories
        bool IsFull() const { return (m_groupType==GROUPTYPE_NORMAL) ? (m_memberSlots.size()>=MAXGROUPSIZE) : (m_memberSlots.size()>=MAXRAIDSIZE); }
        bool isRaidGroup() { return (m_groupType==GROUPTYPE_RAID); }
        bool isBGGroup() { return m_bgGroup != NULL; }
        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        LootMethod    GetLootMethod() const { return m_lootMethod; }
        const uint64& GetLooterGuid() const { return m_looterGuid; }
        ItemQualities GetLootThreshold() const { return m_lootThreshold; }

        // member manipulation methods
        bool IsMember(uint64 guid) const { return _getMemberCSlot(guid) != m_memberSlots.end(); }
        bool IsLeader(uint64 guid) const { return (GetLeaderGUID() == guid); }
        bool IsAssistant(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_memberSlots.end())
                return false;

            return mslot->assistant;
        }

        bool SameSubGroup(uint64 guid1, uint64 guid2) const
        {
            member_citerator mslot2 = _getMemberCSlot(guid2);
            if(mslot2==m_memberSlots.end())
                return false;

            return SameSubGroup(guid1,&*mslot2);
        }

        bool SameSubGroup(uint64 guid1, MemberSlot const* slot2) const
        {
            member_citerator mslot1 = _getMemberCSlot(guid1);
            if(mslot1==m_memberSlots.end() || !slot2)
                return false;

            return (mslot1->group==slot2->group);
        }

        bool SameSubGroup(Player *member1, Player *member2) const;

        MemberSlotList const& GetMemberSlots() const { return m_memberSlots; }
        GroupReference* GetFirstMember() { return m_memberMgr.getFirst(); }
        uint32 GetMembersCount() const { return m_memberSlots.size(); }
        uint32 GetMemberCountForXPAtKill(Unit const* victim);
        Player* GetMemberForXPAtKill(uint64 guid, Unit const* victim);
        Player* GetMemberForXPAtKill(Player *member, Unit const* victim);
        uint8  GetMemberGroup(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if(mslot==m_memberSlots.end())
                return (MAXRAIDSIZE/MAXGROUPSIZE+1);

            return mslot->group;
        }

        // some additional raid methods
        void ConvertToRaid()
        {
            _convertToRaid();
            SendUpdate();
        }
        void SetBattlegroundGroup(BattleGround *bg) { m_bgGroup = bg; }

        void ChangeMembersGroup(const uint64 &guid, const uint8 &group);
        void ChangeMembersGroup(Player *player, const uint8 &group);

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
        void UpdatePlayerOutOfRange(Player* pPlayer, uint32 mask);
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
        Rolls::iterator GetRoll(uint64 Guid)
        {
            Rolls::iterator iter;
            for (iter=RollId.begin(); iter != RollId.end(); ++iter)
            {
                if (iter->itemGUID == Guid)
                {
                    return iter;
                }
            }
            return RollId.end();
        }
        void CountTheRoll(Rolls::iterator roll, uint32 NumberOfPlayers);
        void CountRollVote(uint64 playerGUID, uint64 Guid, uint32 NumberOfPlayers, uint8 Choise);
        void EndRoll();

        void LinkMember(GroupReference *pRef) { m_memberMgr.insertFirst(pRef); }
        void DelinkMember(GroupReference *pRef ATTR_UNUSED ) { }

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
            for(member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        member_witerator _getMemberWSlot(uint64 Guid)
        {
            for(member_witerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        MemberSlotList      m_memberSlots;
        GroupRefManager     m_memberMgr;
        InvitesList         m_invitees;
        uint64              m_leaderGuid;
        std::string         m_leaderName;
        uint64              m_mainTank;
        uint64              m_mainAssistant;
        GroupType           m_groupType;
        BattleGround*       m_bgGroup;
        uint64              m_targetIcons[TARGETICONCOUNT];
        LootMethod          m_lootMethod;
        ItemQualities       m_lootThreshold;
        uint64              m_looterGuid;
        Rolls               RollId;
};
#endif
