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

class Group
{
    public:
        Group()
        {
            m_count = 0;
            m_leaderGuid = 0;
            m_lootMethod = 0;
            m_looterGuid = 0;
            m_grouptype = 0;
        }

        ~Group()
        {
        }

        void Create(const uint64 &guid, const char * name)
        {
            AddMember(guid, name);

            m_leaderGuid = guid;
            m_leaderName = name;
        }

        void AddMember(uint64 guid, const char* name)
        {
            

            if (m_count < MAXGROUPSIZE-1)
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
        void ChangeLeader(const uint64 &guid);

        bool IsFull() const { return m_count == MAXGROUPSIZE; }

        void SendUpdate();
        void Disband();

        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        

        void SetLootMethod(uint32 method) { m_lootMethod = method; }
        void SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }

        uint32 GetLootMethod() const { return m_lootMethod; }
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

        
        void BroadcastToGroup(WorldSession *session, std::string msg);

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

        uint32 m_lootMethod;
        uint64 m_looterGuid;
};
#endif
