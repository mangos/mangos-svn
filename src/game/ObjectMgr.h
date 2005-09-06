/* ObjectMgr.h
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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Log.h"
#include "Object.h"
#include "Item.h"
#include "Creature.h"
#include "Player.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "Quest.h"
#include "Path.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "MiscHandler.h"
#include "Database/DatabaseEnv.h"
#include "Mail.h"
#include "Spell.h"

enum HIGHGUID
{
    HIGHGUID_ITEM          = 0x40000000,
    HIGHGUID_CONTAINER     = 0x40000000,
    HIGHGUID_UNIT          = 0xF0001000,
    HIGHGUID_PLAYER        = 0x00000000,
    HIGHGUID_GAMEOBJECT    = 0xF0007000,
    HIGHGUID_DYNAMICOBJECT = 0xF000A000,
    HIGHGUID_CORPSE        = 0xF0007000
};

class Group;
class Path;

#define MAX_CONTINENTS 500

class ObjectMgr : public Singleton < ObjectMgr >
{
    public:
        ObjectMgr();
        ~ObjectMgr();

        // objects
        typedef HM_NAMESPACE::hash_map<uint32, Player*> PlayerMap;
        typedef HM_NAMESPACE::hash_map<uint32, Creature*> CreatureMap;
        typedef HM_NAMESPACE::hash_map<uint32, Item*> ItemMap;
        typedef HM_NAMESPACE::hash_map<uint32, DynamicObject*> DynamicObjectMap;
        typedef HM_NAMESPACE::hash_map<uint32, GameObject*> GameObjectMap;
        typedef HM_NAMESPACE::hash_map<uint32, Corpse*> CorpseMap;
        // other objects
        typedef std::set< Group * > GroupSet;
        typedef HM_NAMESPACE::hash_map<uint32, GossipText*> GossipTextMap;
        typedef HM_NAMESPACE::hash_map<uint32, GossipNpc*> GossipNpcMap;
        typedef HM_NAMESPACE::hash_map<uint32, GraveyardTeleport*> GraveyardMap;
        typedef HM_NAMESPACE::hash_map<uint32, CreatureInfo*> CreatureNameMap;
        typedef HM_NAMESPACE::hash_map<uint32, ItemPrototype*> ItemPrototypeMap;
        typedef HM_NAMESPACE::hash_map<uint32, AuctionEntry*> AuctionEntryMap;
        typedef HM_NAMESPACE::hash_map<uint32, Trainerspell*> TrainerspellMap;
        typedef HM_NAMESPACE::hash_map<uint32, PlayerCreateInfo*> PlayerCreateInfoMap;
        typedef HM_NAMESPACE::hash_map<uint32, TaxiNodes*> TaxiNodesMap;
        typedef HM_NAMESPACE::hash_map<uint32, TaxiPath*> TaxiPathMap;
        typedef std::vector<TaxiPathNodes*> TaxiPathNodesVec;
        typedef HM_NAMESPACE::hash_map<uint32, TeleportCoords*> TeleportMap;

        // objects
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

        Player* GetPlayer(const char* name)
        {
            PlayerMap::const_iterator itr;
            for (itr = mPlayers.begin(); itr != mPlayers.end(); itr++)
            {
                if(strcmp(itr->second->GetName(), name) == 0)
                    return itr->second;
            }

            return NULL;
        }

        Player* GetPlayer(uint64 guid)
        {
            PlayerMap::const_iterator itr = mPlayers.find(GUID_LOPART(guid));
            if (itr != mPlayers.end())
                return itr->second;
            return NULL;
        }

        Creature* GetCreature(uint64 guid)
        {
            CreatureMap::const_iterator itr = mCreatures.find(GUID_LOPART(guid));
            if (itr != mCreatures.end())
                return itr->second;
            return NULL;
        }

        // Groups
        Group * GetGroupByLeader(const uint64 &guid) const;
        void AddGroup(Group* group) { mGroupSet.insert( group ); }
        void RemoveGroup(Group* group) { mGroupSet.erase( group ); }

        // Quests
        void AddQuest(Quest* quest)
        {
            ASSERT( quest );
            ASSERT( mQuests.find(quest->m_questId) == mQuests.end() );
            mQuests[quest->m_questId] = quest;
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
            Log::getSingleton().outString("adding auction entry with id %u",ah->Id);
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
        // Creature names
        uint32 AddCreatureName(const char* name);
        uint32 AddCreatureName(const char* name, uint32 displayid);
        void AddCreatureName(uint32 id, const char* name);
        void AddCreatureName(uint32 id, const char* name, uint32 displayid);
        void AddCreatureName(CreatureInfo *cinfo);
        CreatureInfo *GetCreatureName( uint32 id );

        // Sub names
        uint32 AddCreatureSubName(const char* name, const char* subname, uint32 displayid);

        // Item prototypes
        ItemPrototype* GetItemPrototype(uint32 id) const
        {
            ItemPrototypeMap::const_iterator itr = mItemPrototypes.find( id );
            if( itr != mItemPrototypes.end( ) )
                return itr->second;
            return NULL;
        }
        void AddItemPrototype(ItemPrototype *itemProto)
        {
            ASSERT( itemProto );
            ASSERT( mItemPrototypes.find(itemProto->ItemId) == mItemPrototypes.end() );
            mItemPrototypes[itemProto->ItemId] = itemProto;
        }
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
        // Trainer spells
        Trainerspell* GetTrainerspell(uint32 id) const
        {
            TrainerspellMap::const_iterator itr = mTrainerspells.find( id );
            if( itr != mTrainerspells.end( ) )
                return itr->second;
            return NULL;
        }
        void AddTrainerspell(Trainerspell *trainspell)
        {
            ASSERT( trainspell );
            ASSERT( mTrainerspells.find(trainspell->Id) == mTrainerspells.end() );
            mTrainerspells[trainspell->Id] = trainspell;
        }
        // Create Player Info
        void AddPlayerCreateInfo(PlayerCreateInfo *playerCreate)
        {
            ASSERT( playerCreate );
            mPlayerCreateInfo[playerCreate->index] = playerCreate;
        }
        PlayerCreateInfo* GetPlayerCreateInfo(uint8 race, uint8 class_) const
        {
            PlayerCreateInfoMap::const_iterator itr;
            // Log::getSingleton().outError("race: %d, class: %d",race,class_);
            for (itr = mPlayerCreateInfo.begin(); itr != mPlayerCreateInfo.end(); itr++)
            {
                if( (itr->second->race == race) && (itr->second->class_ == class_) )
                    return itr->second;
            }
            return NULL;
        }

        // it's kind of db related, not sure where to put it
        uint64 GetPlayerGUIDByName(const char *name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;

        // taxi code
        void AddTaxiNodes(TaxiNodes *taxiNodes)
        {
            ASSERT( taxiNodes );
            mTaxiNodes[taxiNodes->id] = taxiNodes;
        }
        void AddTaxiPath(TaxiPath *taxiPath)
        {
            ASSERT( taxiPath );
            mTaxiPath[taxiPath->id] = taxiPath;
        }
        void AddTaxiPathNodes(TaxiPathNodes *taxiPathNodes)
        {
            ASSERT( taxiPathNodes );
            vTaxiPathNodes.push_back(taxiPathNodes);
        }
        bool GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask );
        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid );
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint16 GetTaxiMount( uint32 id );
        void GetTaxiPathNodes( uint32 path, Path *pathnodes );

        // Corpse Stuff
        Corpse *GetCorpseByOwner(Player *pOwner);
        void SaveCorpses();

        // Gossip Stuff
        void AddGossipText(GossipText *pGText);
        void AddGossip(GossipNpc *pGossip);
        GossipText *GetGossipText(uint32 ID);
        GossipNpc *GetGossipByGuid(uint32 guid, uint32 mapid);
        GossipNpcMap::iterator GetGossipListBegin(uint32 mapid) { return mGossipNpc[mapid].begin(); }
        GossipNpcMap::iterator GetGossipListEnd(uint32 mapid) { return mGossipNpc[mapid].end(); }

        // Death stuff
        void AddGraveyard(GraveyardTeleport *pgrave);
        GraveyardMap::iterator GetGraveyardListBegin() { return mGraveyards.begin(); }
        GraveyardMap::iterator GetGraveyardListEnd() { return mGraveyards.end(); }

        // Teleport Stuff
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

        // Serialization
        void LoadCreatures();
        void LoadGameObjects();
        void LoadQuests();
        void LoadCreatureNames();
        void SaveCreatureNames();
        void LoadItemPrototypes();
        void LoadTrainerSpells();
        void LoadPlayerCreateInfo();
        void LoadTaxiNodes();
        void LoadTaxiPath();
        void LoadTaxiPathNodes();
        void LoadCorpses();
        void LoadGossipText();
        void LoadGossips();
        void LoadGraveyards();
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
        // highest GUIDs, used for creating new objects
        uint32 m_hiCharGuid;
        uint32 m_hiCreatureGuid;
        uint32 m_hiItemGuid;
        uint32 m_hiGoGuid;
        uint32 m_hiDoGuid;
        uint32 m_hiNameGuid;

        template<class T> HM_NAMESPACE::hash_map<uint32,T*>& _GetContainer();
        template<class T> TYPEID _GetTypeId() const;

        ///// Object Tables ////
        // These tables are modified as creatures are created and destroyed in the world

        typedef HM_NAMESPACE::hash_map<uint32, Quest*> QuestMap;

        // Map of active characters in the game
        PlayerMap           mPlayers;

        // Map of active creatures in the game
        CreatureMap         mCreatures;

        // Map of dynamic objects
        GameObjectMap       mGameObjects;

        // Map of dynamic objects
        DynamicObjectMap    mDynamicObjects;

        // Map of corpse objects
        CorpseMap           mCorpses;

        // Group List
        GroupSet            mGroupSet;

        // Map of all item types in the game
        ItemMap             mItems;

        // Map of auction item intances
        ItemMap             mAitems;

        // Map of mailed itesm
        ItemMap             mMitems;

        // Map of all item types in the game
        ItemPrototypeMap    mItemPrototypes;

        // Map of the trainer spells
        TrainerspellMap     mTrainerspells;

        // Map of auctioned items
        AuctionEntryMap     mAuctions;

        // Map entry to a creature name
        CreatureNameMap     mCreatureNames;

        // Quest data
        QuestMap            mQuests;

        // Map of all starting infos needed for player creation
        PlayerCreateInfoMap mPlayerCreateInfo;

        // Maps containing the infos for taxi paths
        TaxiNodesMap        mTaxiNodes;
        TaxiPathMap         mTaxiPath;
        TaxiPathNodesVec    vTaxiPathNodes;

        // Maps for Gossip stuff
        GossipTextMap       mGossipText;
        GossipNpcMap        mGossipNpc[MAX_CONTINENTS];

        // Death Stuff
        GraveyardMap        mGraveyards;

        // Teleport Stuff
        TeleportMap         mTeleports;

};

// According to C++ standard explicit template declarations should be in scope where ObjectMgr is.

template<> inline HM_NAMESPACE::hash_map<uint32,DynamicObject*>& ObjectMgr::_GetContainer<DynamicObject>()
{ return mDynamicObjects; }
template<> inline HM_NAMESPACE::hash_map<uint32,Creature*>& ObjectMgr::_GetContainer<Creature>()
{ return mCreatures; }
template<> inline HM_NAMESPACE::hash_map<uint32,Player*>& ObjectMgr::_GetContainer<Player>()
{ return mPlayers; }
template<> inline HM_NAMESPACE::hash_map<uint32,GameObject*>& ObjectMgr::_GetContainer<GameObject>()
{ return mGameObjects; }
template<> inline HM_NAMESPACE::hash_map<uint32,Corpse*>& ObjectMgr::_GetContainer<Corpse>()
{ return mCorpses; }

template<> inline TYPEID ObjectMgr::_GetTypeId<DynamicObject>() const
{ return TYPEID_DYNAMICOBJECT; }
template<> inline TYPEID ObjectMgr::_GetTypeId<GameObject>() const
{ return TYPEID_GAMEOBJECT; }
template<> inline TYPEID ObjectMgr::_GetTypeId<Creature>() const
{ return TYPEID_UNIT; }
template<> inline TYPEID ObjectMgr::_GetTypeId<Player>() const
{ return TYPEID_PLAYER; }
template<> inline TYPEID ObjectMgr::_GetTypeId<Corpse>() const
{ return TYPEID_CORPSE; }

#define objmgr ObjectMgr::getSingleton()
#endif
