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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Log.h"
#include "Object.h"
#include "Item.h"
#include "Bag.h"
#include "Creature.h"
#include "Player.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "QuestDef.h"
#include "Path.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "MiscHandler.h"
#include "Database/DatabaseEnv.h"
#include "Mail.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Policies/Singleton.h"


class Group;
class Path;
class Guild;



class ObjectMgr 
{
    public:
        ObjectMgr();
        ~ObjectMgr();

        
        typedef HM_NAMESPACE::hash_map<uint32, Item*> ItemMap;
   
        typedef HM_NAMESPACE::hash_map<uint32, Player*> PlayerMap;

        
        typedef std::set< Group * > GroupSet;
        typedef std::set< Guild * > GuildSet;

        typedef HM_NAMESPACE::hash_map<uint32, AuctionEntry*> AuctionEntryMap;
        
        
        
        typedef HM_NAMESPACE::hash_map<uint32, TeleportCoords*> TeleportMap;

        
        template <class T>
            typename HM_NAMESPACE::hash_map<uint32, T*>::iterator
            Begin() { return _GetContainer<T>().begin(); }
        template <class T>
            typename HM_NAMESPACE::hash_map<uint32, T*>::iterator
            End() { return _GetContainer<T>().end(); }
        template <class T>
            T* GetObject(const uint64 &guid)
        {
            const HM_NAMESPACE::hash_map<uint32, T*> &container = _GetContainer<T>();
            typename HM_NAMESPACE::hash_map<uint32, T*>::const_iterator itr =
                container.find(GUID_LOPART(guid));
            if(itr == container.end() || itr->second->GetGUID() != guid)
                return NULL;
            else
                return itr->second;
        }
        template <class T>
            void AddObject(T *obj)
        {
            ASSERT(obj && obj->GetTypeId() == _GetTypeId<T>());
            ASSERT(_GetContainer<T>().find(obj->GetGUIDLow()) == _GetContainer<T>().end());
            _GetContainer<T>()[obj->GetGUIDLow()] = obj;
        }
        template <class T>
            bool RemoveObject(T *obj)
        {
            HM_NAMESPACE::hash_map<uint32, T*> &container = _GetContainer<T>();
            typename HM_NAMESPACE::hash_map<uint32, T*>::iterator i = container.find(obj->GetGUIDLow());
            if(i == container.end())
                return false;
            container.erase(i);
            return true;
        }

        Player* GetPlayer(const char* name){ return ObjectAccessor::Instance().FindPlayerByName(name);}
        Player* GetPlayer(uint64 guid){ return ObjectAccessor::Instance().FindPlayer(guid); }

		GameObjectInfo *GetGameObjectInfo(uint32 id);
		void LoadGameobjectInfo();
		void AddGameobjectInfo(GameObjectInfo *goinfo);

        
        Group * GetGroupByLeader(const uint64 &guid) const;
        void AddGroup(Group* group) { mGroupSet.insert( group ); }
        void RemoveGroup(Group* group) { mGroupSet.erase( group ); }

		
		Guild* GetGuildById(const uint32 GuildId) const;
		void AddGuild(Guild* guild) { mGuildSet.insert( guild ); }
        void RemoveGuild(Guild* guild) { mGuildSet.erase( guild ); }

        
        void AddQuest(Quest* quest)
        {
            ASSERT( quest );
            ASSERT( mQuests.find(quest->m_qId) == mQuests.end() );
            mQuests[quest->m_qId] = quest;
        }
        Quest* GetQuest(uint32 id) const
        {
            QuestMap::const_iterator itr = mQuests.find( id );
            if( itr != mQuests.end( ) )
                return itr->second;
            return NULL;
        }
        void AddAuction(AuctionEntry *ah)
        {
            ASSERT( ah );
            ASSERT( mAuctions.find(ah->Id) == mAuctions.end() );
            mAuctions[ah->Id] = ah;
            sLog.outString("adding auction entry with id %u",ah->Id);
        }
        AuctionEntry* GetAuction(uint32 id) const
        {
            AuctionEntryMap::const_iterator itr = mAuctions.find( id );
            if( itr != mAuctions.end( ) )
                return itr->second;
            return NULL;
        }
        bool RemoveAuction(uint32 id)
        {
            AuctionEntryMap::iterator i = mAuctions.find(id);
            if (i == mAuctions.end())
            {
                return false;
            }
            mAuctions.erase(i);
            return true;
        }
        
        CreatureInfo *GetCreatureTemplate( uint32 id );
              
        ItemPrototype* GetItemPrototype(uint32 id) ;
   
       
        Item* GetMItem(uint32 id)
        {
            ItemMap::const_iterator itr = mMitems.find(id);
            if (itr != mMitems.end())
            {
                return itr->second;
            }
            return NULL;
        }
        void AddMItem(Item* it)
        {
            ASSERT( it );
            ASSERT( mMitems.find(it->GetGUIDLow()) == mMitems.end());
            mMitems[it->GetGUIDLow()] = it;
        }
        bool RemoveMItem(uint32 id)
        {
            ItemMap::iterator i = mMitems.find(id);
            if (i == mMitems.end())
            {
                return false;
            }
            mMitems.erase(i);
            return true;
        }
        Item* GetAItem(uint32 id)
        {
            ItemMap::const_iterator itr = mAitems.find(id);
            if (itr != mAitems.end())
            {
                return itr->second;
            }
            return NULL;
        }
        void AddAItem(Item* it)
        {
            ASSERT( it );
            ASSERT( mAitems.find(it->GetGUIDLow()) == mAitems.end());
            mAitems[it->GetGUIDLow()] = it;
        }
        bool RemoveAItem(uint32 id)
        {
            ItemMap::iterator i = mAitems.find(id);
            if (i == mAitems.end())
            {
                return false;
            }
            mAitems.erase(i);
            return true;
        }
        AuctionEntryMap::iterator GetAuctionsBegin() {return mAuctions.begin();}
        AuctionEntryMap::iterator GetAuctionsEnd() {return mAuctions.end();}
               
        PlayerCreateInfo* GetPlayerCreateInfo(uint32 race, uint32 class_); 

        
        uint64 GetPlayerGUIDByName(const char *name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;

        bool GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask );
        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid );
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint16 GetTaxiMount( uint32 id );
        void GetTaxiPathNodes( uint32 path, Path &pathnodes );

        void AddAreaTriggerPoint(AreaTriggerPoint *pArea);
        AreaTriggerPoint *GetAreaTriggerQuestPoint(uint32 Trigger_ID);
        ItemPage *RetreiveItemPageText(uint32 Page_ID);

        void AddGossipText(GossipText *pGText);
        GossipText *GetGossipText(uint32 Text_ID);

        

        
       
        GraveyardTeleport *GetClosestGraveYard(float x, float y, float z, uint32 MapId);
       
       

        
        void AddTeleportCoords(TeleportCoords* TC)
        {
            ASSERT( TC );
            mTeleports[TC->id] = TC;
        }
        TeleportCoords* GetTeleportCoords(uint32 id) const
        {
            TeleportMap::const_iterator itr = mTeleports.find( id );
            if( itr != mTeleports.end( ) )
                return itr->second;
            return NULL;
        }

        
        AreaTrigger * GetAreaTrigger(uint32 trigger);

        
        void LoadGuilds();
        void LoadQuests();
        void LoadCreatureTemplates();
        void LoadItemPrototypes();

        void LoadGossipText();
        void LoadAreaTriggerPoints();

        void LoadTeleportCoords();
        void LoadAuctions();
        void LoadAuctionItems();
        void LoadMailedItems();

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateAuctionID();
        uint32 GenerateMailID();

    protected:
        uint32 m_auctionid;
        uint32 m_mailid;
        
        uint32 m_hiCharGuid;
        uint32 m_hiCreatureGuid;
        uint32 m_hiItemGuid;
        uint32 m_hiGoGuid;
        uint32 m_hiDoGuid;
		uint32 m_hiCorpseGuid;
       

        template<class T> HM_NAMESPACE::hash_map<uint32,T*>& _GetContainer();
        template<class T> TYPEID _GetTypeId() const;

        typedef HM_NAMESPACE::hash_map<uint32, Quest*> QuestMap;
        typedef HM_NAMESPACE::hash_map<uint32, GossipText*> GossipTextMap;
        typedef HM_NAMESPACE::hash_map<uint32, AreaTriggerPoint*> AreaTriggerMap;

        
        GroupSet            mGroupSet;
        GuildSet            mGuildSet;

        ItemMap             mItems;
        ItemMap             mAitems;
        ItemMap             mMitems;
 
        AuctionEntryMap     mAuctions;

        QuestMap            mQuests;
        AreaTriggerMap	    mAreaTriggerMap;
        GossipTextMap       mGossipText;
        TeleportMap         mTeleports;

private:
  
};


#define objmgr MaNGOS::Singleton<ObjectMgr>::Instance()

#endif
