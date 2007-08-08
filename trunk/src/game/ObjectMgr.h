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
#include "Database/DatabaseEnv.h"
#include "AuctionHouseObject.h"
#include "Mail.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Policies/Singleton.h"
#include "Database/SQLStorage.h"
#include "GameEvent.h"

#include <string>
#include <map>

extern SQLStorage sCreatureStorage;
extern SQLStorage sCreatureDataAddonStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sPageTextStore;
extern SQLStorage sItemStorage;
extern SQLStorage sSpellThreatStore;

class Group;
class Guild;
class ArenaTeam;
class Path;
class TransportPath;

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;
    uint32 datalong;
    uint32 datalong2;
    std::string datatext;
    float x;
    float y;
    float z;
    float o;
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;
extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sSpellScripts;

struct AreaTrigger
{
    uint8  requiredLevel;
    uint32 requiredItem;
    uint32 trigger_mapId;
    float  trigger_X;
    float  trigger_Y;
    float  trigger_Z;
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;

    bool IsTeleport() const { return target_X != 0 || target_Y !=0 || target_Z !=0; }
};

typedef std::set<uint32> CellGuidSet;
typedef std::map<uint32/*player guid*/,uint32/*instance*/> CellCorpseSet;
struct CellObjectGuids
{
    CellGuidSet creatures;
    CellGuidSet gameobjects;
    CellCorpseSet corpses;
};
typedef HM_NAMESPACE::hash_map<uint32/*cell_id*/,CellObjectGuids> CellObjectGuidsMap;
typedef HM_NAMESPACE::hash_map<uint32/*mapid*/,CellObjectGuidsMap> MapObjectGuids;

typedef HM_NAMESPACE::hash_map<uint64/*(instance,guid) pair*/,time_t> RespawnTimes;

typedef HM_NAMESPACE::hash_map<uint32,CreatureData> CreatureDataMap;
typedef HM_NAMESPACE::hash_map<uint32,GameObjectData> GameObjectDataMap;

typedef std::multimap<uint32,uint32> QuestRelations;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0) { for(int i=0; i < MAX_STATS; ++i ) stats[i] = 0; }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
    uint16 armor;
};

struct ReputationOnKillEntry 
{
    uint32 repfaction1;
    uint32 repfaction2;
    bool is_teamaward1;
    uint32 reputration_max_cap1;
    int32 repvalue1;
    bool is_teamaward2;
    uint32 reputration_max_cap2;
    int32 repvalue2;
    bool team_dependent;
};

struct PetCreateSpellEntry
{
    uint32 spellid[4];
    uint32 familypassive;
};

#define WEATHER_SEASONS 4
struct WeatherSeasonChances {
    uint32 rainChance;
    uint32 snowChance;
    uint32 stormChance;
};

struct WeatherZoneChances
{
    WeatherSeasonChances data[WEATHER_SEASONS];
};

struct SpellAffection
{
    uint16 SpellId;
    uint8 SchoolMask;
    uint16 Category;
    uint16 SkillId;
    uint8 SpellFamily;
    uint64 SpellFamilyMask;
    uint16 Charges;
};

/// Player state
enum SessionStatus
{
    STATUS_AUTHED = 0,                                      ///< Player authenticated
    STATUS_LOGGEDIN                                         ///< Player in game
};

struct OpcodeHandler
{
    OpcodeHandler() : status(STATUS_AUTHED), handler(NULL) {};
    OpcodeHandler( SessionStatus _status, void (WorldSession::*_handler)(WorldPacket& recvPacket) ) : status(_status), handler(_handler) {};

    SessionStatus status;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

typedef HM_NAMESPACE::hash_map< uint16 , OpcodeHandler > OpcodeTableMap;

class ObjectMgr
{
    public:
        ObjectMgr();
        ~ObjectMgr();

        typedef HM_NAMESPACE::hash_map<uint32, Item*> ItemMap;

        typedef std::set< Group * > GroupSet;
        typedef std::set< Guild * > GuildSet;
        typedef std::set< ArenaTeam * > ArenaTeamSet;

        typedef HM_NAMESPACE::hash_map<uint32, AreaTrigger> AreaTriggerMap;

        typedef HM_NAMESPACE::hash_map<uint32, ReputationOnKillEntry> RepOnKillMap;

        typedef HM_NAMESPACE::hash_map<uint32, WeatherZoneChances> WeatherZoneMap;
        
        typedef HM_NAMESPACE::hash_map<uint32, PetCreateSpellEntry> PetCreateSpellMap;

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

        Player* GetPlayer(const char* name) const { return ObjectAccessor::Instance().FindPlayerByName(name);}
        Player* GetPlayer(uint64 guid) const { return ObjectAccessor::Instance().FindPlayer(guid); }

        static GameObjectInfo const *GetGameObjectInfo(uint32 id) { return sGOStorage.LookupEntry<GameObjectInfo>(id); }

        void LoadGameobjectInfo();
        void AddGameobjectInfo(GameObjectInfo *goinfo);

        Group * GetGroupByLeader(const uint64 &guid) const;
        void AddGroup(Group* group) { mGroupSet.insert( group ); }
        void RemoveGroup(Group* group) { mGroupSet.erase( group ); }

        Guild* GetGuildById(const uint32 GuildId) const;
        Guild* GetGuildByName(std::string guildname) const;
        std::string GetGuildNameById(const uint32 GuildId) const;
        void AddGuild(Guild* guild) { mGuildSet.insert( guild ); }
        void RemoveGuild(Guild* guild) { mGuildSet.erase( guild ); }

        ArenaTeam* GetArenaTeamById(const uint32 ArenaTeamId) const;
        ArenaTeam* GetArenaTeamByName(std::string ArenaTeamName) const;
        void AddArenaTeam(ArenaTeam* arenateam) { mArenaTeamSet.insert( arenateam ); }
        void RemoveArenaTeam(ArenaTeam* arenateam) { mArenaTeamSet.erase( arenateam ); }

        CreatureInfo const *GetCreatureTemplate( uint32 id );
        CreatureDataAddon const *GetCreatureAddon( uint32 lowguid )
        {
            return sCreatureDataAddonStorage.LookupEntry<CreatureDataAddon>(lowguid);
        }

        static ItemPrototype const* GetItemPrototype(uint32 id) { return sItemStorage.LookupEntry<ItemPrototype>(id); }

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
        AuctionHouseObject * GetAuctionsMap( uint32 location );

        //auction messages
        void SendAuctionWonMail( AuctionEntry * auction );
        void SendAuctionSuccessfulMail( AuctionEntry * auction );
        void SendAuctionExpiredMail( AuctionEntry * auction );
        uint32 GetAuctionCut( uint32 location, uint32 highBid );
        uint32 GetAuctionDeposit(uint32 location, uint32 time, Item *pItem);
        uint32 GetAuctionOutBid(uint32 currentBid);

        PetLevelInfo const* GetPetLevelInfo(uint32 creature_id, uint32 level) const;

        PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const
        {
            if(race   >= MAX_RACES)   return NULL;
            if(class_ >= MAX_CLASSES) return NULL;
            PlayerInfo const* info = &playerInfo[race][class_];
            if(info->displayId_m==0 || info->displayId_f==0) return NULL;
            return info;
        }
        void GetPlayerLevelInfo(uint32 race, uint32 class_,uint32 level, PlayerLevelInfo* info) const;

        uint64 GetPlayerGUIDByName(std::string name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;
        uint32 GetPlayerTeamByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByGUID(const uint64 &guid) const;
        uint32 GetSecurityByAccount(uint32 acc_id) const;

        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid );
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint16 GetTaxiMount( uint32 id, uint32 team );
        void GetTaxiPathNodes( uint32 path, Path &pathnodes );
        void GetTransportPathNodes( uint32 path, TransportPath &pathnodes );

        uint32 GetQuestForAreaTrigger(uint32 Trigger_ID) const
        {
            QuestAreaTriggerMap::const_iterator itr = mQuestAreaTriggerMap.find(Trigger_ID);
            if(itr != mQuestAreaTriggerMap.end())
                return itr->second;
            return 0;
        }
        bool IsTavernAreaTrigger(uint32 Trigger_ID) const { return mTavernAreaTriggerSet.count(Trigger_ID) != 0; }

        void AddGossipText(GossipText *pGText);
        GossipText *GetGossipText(uint32 Text_ID);

        WorldSafeLocsEntry const *GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team);

        AreaTrigger const* GetAreaTrigger(uint32 trigger) const
        {
            AreaTriggerMap::const_iterator itr = mAreaTriggers.find( trigger );
            if( itr != mAreaTriggers.end( ) )
                return &itr->second;
            return NULL;
        }

        SpellAffection const* GetSpellAffection(uint16 spellId, uint8 effectId) const
        {
            SpellAffectMap::const_iterator itr = mSpellAffectMap.find((spellId<<8) + effectId);
            if( itr != mSpellAffectMap.end( ) )
                return &itr->second;
            return NULL;
        }
        bool IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 const& familyFlags);

        SpellProcEventEntry const* GetSpellProcEvent(uint32 spellId) const
        {
            SpellProcEventMap::const_iterator itr = mSpellProcEventMap.find(spellId);
            if( itr != mSpellProcEventMap.end( ) )
                return &itr->second;
            return NULL;
        }

        ReputationOnKillEntry const* GetReputationOnKilEntry(uint32 id) const
        {
            RepOnKillMap::const_iterator itr = mRepOnKill.find(id);
            if(itr != mRepOnKill.end())
                return &itr->second;
            return NULL;
        }
        
        PetCreateSpellEntry const* GetPetCreateSpellEntry(uint32 id) const
        {
            PetCreateSpellMap::const_iterator itr = mPetCreateSpell.find(id);
            if(itr != mPetCreateSpell.end())
                return &itr->second;
            return NULL;
        }
        void LoadGuilds();
        void LoadArenaTeams();
        void LoadGroups();
        void LoadQuests();
        void LoadQuestRelations()
        {
            LoadQuestRelationsHelper(mGOQuestRelations,"gameobject_questrelation");
            LoadQuestRelationsHelper(mGOQuestInvolvedRelations,"gameobject_involvedrelation");
            LoadQuestRelationsHelper(mCreatureQuestRelations,"creature_questrelation");
            LoadQuestRelationsHelper(mCreatureQuestInvolvedRelations,"creature_involvedrelation");
        }
        void LoadQuestRelationsHelper(QuestRelations& map,char const* table);
        QuestRelations mGOQuestRelations;
        QuestRelations mGOQuestInvolvedRelations;
        QuestRelations mCreatureQuestRelations;
        QuestRelations mCreatureQuestInvolvedRelations;

        void LoadSpellChains();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadPetCreateSpells();
        void LoadScripts(ScriptMapMap& scripts, char const* tablename);
        void LoadCreatureTemplates();
        void LoadCreatures();
        void LoadCreatureRespawnTimes();
        void LoadCreatureAddons();
        void LoadGameobjects();
        void LoadGameobjectRespawnTimes();
        void LoadSpellProcEvents();
        void LoadSpellThreats();
        void LoadItemPrototypes();

        void LoadGossipText();

        void LoadAreaTriggers();
        void LoadSpellAffects();
        void LoadQuestAreaTriggers();
        void LoadTavernAreaTriggers();

        void LoadItemTexts();
        void LoadPageTexts();

        // instance system
        void CleanupInstances();
        void PackInstances();

        //load first auction items, because of check if item exists, when loading
        void LoadAuctionItems();
        void LoadAuctions();
        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadCorpses();

        void LoadReputationOnKill();

        void LoadWeatherZoneChances();

        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint32 level);

        void ReturnOrDeleteOldMails(bool serverUp);

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateAuctionID();
        uint32 GenerateMailID();
        uint32 GenerateItemTextID();
        uint32 GeneratePetNumber();

        uint32 CreateItemText(std::string text);
        std::string GetItemText( uint32 id )
        {
            ItemTextMap::const_iterator itr = mItemTexts.find( id );
            if ( itr != mItemTexts.end() )
                return itr->second;
            else
                return "There is no info for this item";
        }

        typedef HM_NAMESPACE::hash_map<uint32, Quest*> QuestMap;
        QuestMap mQuestTemplates;
        typedef std::multimap<uint32, uint32> ExclusiveQuestGroups;
        ExclusiveQuestGroups mExclusiveQuestGroups;

        struct SpellChainNode
        {
            uint32 prev;
            uint32 first;
            uint8  rank;
        };

        typedef HM_NAMESPACE::hash_map<uint32, SpellChainNode> SpellChainMap;
        SpellChainMap mSpellChains;

        uint32 GetFirstSpellInChain(uint32 spell_id)
        {
            SpellChainMap::iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return spell_id;

            return itr->second.first;
        }

        uint32 GetPrevSpellInChain(uint32 spell_id)
        {
            SpellChainMap::iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return 0;

            return itr->second.prev;
        }

        uint8 GetSpellRank(uint32 spell_id)
        {
            SpellChainMap::iterator itr = mSpellChains.find(spell_id);
            if(itr == mSpellChains.end())
                return 0;

            return itr->second.rank;
        }

        uint32 GetLastSpellInChain(uint32 spell_id)
        {
            // fast check non ranked spell
            SpellChainMap::iterator spell_itr = mSpellChains.find(spell_id);
            if(spell_itr == mSpellChains.end())
                return 0;

            for(SpellChainMap::iterator itr = mSpellChains.begin(); itr != mSpellChains.end(); ++itr)
            {
                if(itr->second.first==spell_itr->second.first && itr->second.rank > spell_itr->second.rank)
                    spell_itr = itr;
            }

            return spell_itr->first;
        }

        bool IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2);
        bool canStackSpellRanks(SpellEntry const *spellInfo,SpellEntry const *spellInfo2);
        bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2);
        static bool IsProfessionSpell(uint32 spellId);
        static bool IsPrimaryProfessionSpell(uint32 spellId);
        bool IsPrimaryProfessionFirstRankSpell(uint32 spellId);

        struct SpellLearnSkillNode
        {
            uint32 skill;
            uint32 value;                                   // 0  - max skill value for player level
            uint32 maxvalue;                                // 0  - max skill value for player level
        };

        typedef std::map<uint32, SpellLearnSkillNode> SpellLearnSkillMap;
        SpellLearnSkillMap SpellLearnSkills;

        SpellLearnSkillNode const* GetSpellLearnSkill(uint32 spell_id)
        {
            SpellLearnSkillMap::const_iterator itr = SpellLearnSkills.find(spell_id);
            if(itr != SpellLearnSkills.end())
                return &itr->second;
            else
                return NULL;
        }

        struct SpellLearnSpellNode
        {
            uint32 spell;
            uint32 ifNoSpell;
            uint32 autoLearned;
        };

        typedef std::multimap<uint32, SpellLearnSpellNode> SpellLearnSpellMap;
        SpellLearnSpellMap SpellLearnSpells;

        bool IsSpellLearnSpell(uint32 spell_id) const
        {
            return SpellLearnSpells.count(spell_id)!=0;
        }
        SpellLearnSpellMap::const_iterator GetBeginSpellLearnSpell(uint32 spell_id) const
        {
            return SpellLearnSpells.lower_bound(spell_id);
        }
        SpellLearnSpellMap::const_iterator GetEndSpellLearnSpell(uint32 spell_id) const
        {
            return SpellLearnSpells.upper_bound(spell_id);
        }
        
        bool IsSpellLearnToSpell(uint32 spell_id1,uint32 spell_id2) const
        {
            SpellLearnSpellMap::const_iterator b = GetBeginSpellLearnSpell(spell_id1);
            SpellLearnSpellMap::const_iterator e = GetEndSpellLearnSpell(spell_id1);
            for(SpellLearnSpellMap::const_iterator i = b; i != e; ++i)
                if(i->second.spell==spell_id2)
                    return true;
            return false;
        }

        WeatherZoneChances const* GetWeatherChances(uint32 zone_id) const
        {
            WeatherZoneMap::const_iterator itr = mWeatherZoneMap.find(zone_id);
            if(itr != mWeatherZoneMap.end())
                return &itr->second;
            else
                return NULL;
        }

        OpcodeTableMap opcodeTable;

        CellObjectGuids const& GetCellObjectGuids(uint32 mapid, uint32 cell_id)
        {
            return mMapObjectGuids[mapid][cell_id];
        }

        CreatureData const* GetCreatureData(uint32 guid) const
        {
            CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
            if(itr==mCreatureDataMap.end()) return NULL;
            return &itr->second;
        }
        CreatureData& NewCreatureData(uint32 guid) { return mCreatureDataMap[guid]; }
        void DeleteCreatureData(uint32 guid);

        GameObjectData const* GetGOData(uint32 guid) const
        {
            GameObjectDataMap::const_iterator itr = mGameObjectDataMap.find(guid);
            if(itr==mGameObjectDataMap.end()) return NULL;
            return &itr->second;
        }
        GameObjectData& NewGOData(uint32 guid) { return mGameObjectDataMap[guid]; }
        void DeleteGOData(uint32 guid);

        void AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid);

        time_t GetCreatureRespawnTime(uint32 loguid, uint32 instance) { return mCreatureRespawnTimes[MAKE_GUID(loguid,instance)]; }
        void SaveCreatureRespawnTime(uint32 loguid, uint32 instance, time_t t);
        time_t GetGORespawnTime(uint32 loguid, uint32 instance) { return mGORespawnTimes[MAKE_GUID(loguid,instance)]; }
        void SaveGORespawnTime(uint32 loguid, uint32 instance, time_t t);
        void DeleteRespawnTimeForInstance(uint32 instance);

        // player Dumps
        bool WritePlayerDump(std::string file, uint32 guid);
        bool LoadPlayerDump(std::string file, uint32 account, std::string name = "", uint32 newGuid = 0);

        void AddCreatureToGrid(uint32 guid, CreatureData const* data);
        void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);
        void AddGameobjectToGrid(uint32 guid, GameObjectData const* data);
        void RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data);
    protected:
        uint32 m_auctionid;
        uint32 m_mailid;
        uint32 m_ItemTextId;

        uint32 m_hiCharGuid;
        uint32 m_hiCreatureGuid;
        uint32 m_hiItemGuid;
        uint32 m_hiGoGuid;
        uint32 m_hiDoGuid;
        uint32 m_hiCorpseGuid;

        uint32 m_hiPetNumber;

        template<class T> HM_NAMESPACE::hash_map<uint32,T*>& _GetContainer();
        template<class T> TYPEID _GetTypeId() const;

        typedef HM_NAMESPACE::hash_map<uint32, GossipText*> GossipTextMap;
        typedef HM_NAMESPACE::hash_map<uint32, uint32> QuestAreaTriggerMap;
        typedef HM_NAMESPACE::hash_map<uint32, std::string> ItemTextMap;
        typedef std::set<uint32> TavernAreaTriggerSet;

        GroupSet            mGroupSet;
        GuildSet            mGuildSet;
        ArenaTeamSet        mArenaTeamSet;

        ItemMap             mItems;
        ItemMap             mAitems;

        ItemTextMap         mItemTexts;

        AuctionHouseObject  mHordeAuctions;
        AuctionHouseObject  mAllianceAuctions;
        AuctionHouseObject  mNeutralAuctions;

        QuestAreaTriggerMap mQuestAreaTriggerMap;
        TavernAreaTriggerSet mTavernAreaTriggerSet;
        GossipTextMap       mGossipText;
        AreaTriggerMap      mAreaTriggers;

        RepOnKillMap        mRepOnKill;

        WeatherZoneMap      mWeatherZoneMap;
        
        PetCreateSpellMap   mPetCreateSpell;

    private:
        typedef std::map<uint32,PetLevelInfo*> PetLevelInfoMap;
        // PetLevelInfoMap[creature_id][level]
        PetLevelInfoMap petInfo;                            // [creature_id][level]

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;
        PlayerInfo **playerInfo;                            // [race][class]

        typedef std::map<uint32,uint32> BaseXPMap;          // [area level][base xp]
        BaseXPMap mBaseXPTable;

        typedef std::map<uint32,std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;

        MapObjectGuids mMapObjectGuids;
        CreatureDataMap mCreatureDataMap;
        GameObjectDataMap mGameObjectDataMap;
        RespawnTimes mCreatureRespawnTimes;
        RespawnTimes mGORespawnTimes;

        typedef HM_NAMESPACE::hash_map<uint32, SpellAffection> SpellAffectMap;
        SpellAffectMap mSpellAffectMap;

        typedef HM_NAMESPACE::hash_map<uint32, SpellProcEventEntry> SpellProcEventMap;
        SpellProcEventMap mSpellProcEventMap;
};

#define objmgr MaNGOS::Singleton<ObjectMgr>::Instance()
#endif
