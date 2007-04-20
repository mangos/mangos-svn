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
#include "MiscHandler.h"
#include "Database/DatabaseEnv.h"
#include "AuctionHouseObject.h"
#include "Mail.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Policies/Singleton.h"
#include "Database/SQLStorage.h"

extern SQLStorage sCreatureStorage;
extern SQLStorage sCreatureAddStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sPageTextStore;
extern SQLStorage sItemStorage;
extern SQLStorage sSpellProcEventStore;
extern SQLStorage sSpellThreatStore;

class Group;
class Guild;
class Path;
class TransportPath;

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;
    uint32 datalong;
    uint32 datalong2;
    string datatext;
    float x;
    float y;
    float z;
    float o;
};

typedef multimap<uint32, ScriptInfo> ScriptMap;
typedef map<uint32, ScriptMap > ScriptMapMap;
extern ScriptMapMap sScripts;
extern ScriptMapMap sSpellScripts;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0) { for(int i=0; i < MAX_STATS; ++i ) stats[i] = 0; }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
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

        typedef HM_NAMESPACE::hash_map<uint32, TeleportCoords*> TeleportMap;

        typedef HM_NAMESPACE::hash_map<uint32, ReputationOnKillEntry> RepOnKillMap;

        typedef HM_NAMESPACE::hash_map<uint32, WeatherZoneChances> WeatherZoneMap;

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

        CreatureInfo const *GetCreatureTemplate( uint32 id );
        CreatureAddInfo const *GetCreatureAddon( uint32 lowguid );

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

        uint64 GetPlayerGUIDByName(const char *name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;
        uint32 GetPlayerTeamByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByGUID(const uint64 &guid) const;
        uint32 GetSecurityByAccount(uint32 acc_id) const;

        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid );
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint16 GetTaxiMount( uint32 id, uint32 team );
        void GetTaxiPathNodes( uint32 path, Path &pathnodes );
        void GetTransportPathNodes( uint32 path, TransportPath &pathnodes );

        void AddAreaTriggerPoint(AreaTriggerPoint *pArea);
        AreaTriggerPoint *GetAreaTriggerQuestPoint(uint32 Trigger_ID);

        void AddGossipText(GossipText *pGText);
        GossipText *GetGossipText(uint32 Text_ID);

        WorldSafeLocsEntry const *GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team);

        void AddTeleportCoords(TeleportCoords* TC)
        {
            ASSERT( TC );
            mTeleports[TC->id] = TC;
        }
        TeleportCoords const* GetTeleportCoords(uint32 id) const
        {
            TeleportMap::const_iterator itr = mTeleports.find( id );
            if( itr != mTeleports.end( ) )
                return itr->second;
            return NULL;
        }

        AreaTrigger * GetAreaTrigger(uint32 trigger);

        ReputationOnKillEntry const* GetReputationOnKilEntry(uint32 id) const
        {
            RepOnKillMap::const_iterator itr = mRepOnKill.find(id);
            if(itr != mRepOnKill.end())
                return &itr->second;
            return NULL;
        }
        void LoadGuilds();
        void LoadGroups();
        void LoadQuests();
        void LoadSpellChains();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadScripts(ScriptMapMap& scripts, char const* tablename);
        void LoadCreatureTemplates();
        void LoadCreatureAddons();
        void LoadSpellProcEvents();
        void LoadSpellThreats();
        void LoadItemPrototypes();

        void LoadGossipText();
        void LoadAreaTriggerPoints();

        void LoadItemTexts();
        void LoadPageTexts();

        void LoadTeleportCoords();

        // instance system
        void CleanupInstances();
        void PackInstances();

        //load first auction items, because of check if item exists, when loading
        void LoadAuctionItems();
        void LoadAuctions();
        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadPetNames();
        void LoadCorpses();

        void LoadReputationOnKill();

        void LoadWeatherZoneChances();

        std::string GeneratePetName(uint32 entry);

        void ReturnOrDeleteOldMails(bool serverUp);

        void SetHighestGuids();
        uint32 GenerateLowGuid(uint32 guidhigh);
        uint32 GenerateAuctionID();
        uint32 GenerateMailID();
        uint32 GenerateItemTextID();

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
        QuestMap QuestTemplates;
        multimap<uint32, uint32> ExclusiveQuestGroups;

        struct SpellChainNode
        {
            uint32 prev;
            uint32 first;
            uint8  rank;
        };

        typedef HM_NAMESPACE::hash_map<uint32, SpellChainNode> SpellChainMap;
        SpellChainMap SpellChains;

        uint32 GetFirstSpellInChain(uint32 spell_id)
        {
            SpellChainMap::iterator itr = SpellChains.find(spell_id);
            if(itr == SpellChains.end())
                return spell_id;

            return itr->second.first;
        }

        uint32 GetPrevSpellInChain(uint32 spell_id)
        {
            SpellChainMap::iterator itr = SpellChains.find(spell_id);
            if(itr == SpellChains.end())
                return 0;

            return itr->second.prev;
        }

        uint8 GetSpellRank(uint32 spell_id)
        {
            SpellChainMap::iterator itr = SpellChains.find(spell_id);
            if(itr == SpellChains.end())
                return 0;

            return itr->second.rank;
        }
        bool IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2);
        bool canStackSpellRank(SpellEntry const *spellInfo);
        bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2);

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

        bool IsSpellLearnSpell(uint32 spell_id)
        {
            return SpellLearnSpells.count(spell_id)!=0;
        }
        SpellLearnSpellMap::const_iterator GetBeginSpellLearnSpell(uint32 spell_id)
        {
            return SpellLearnSpells.lower_bound(spell_id);
        }
        SpellLearnSpellMap::const_iterator GetEndSpellLearnSpell(uint32 spell_id)
        {
            return SpellLearnSpells.upper_bound(spell_id);
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

        template<class T> HM_NAMESPACE::hash_map<uint32,T*>& _GetContainer();
        template<class T> TYPEID _GetTypeId() const;

        typedef HM_NAMESPACE::hash_map<uint32, GossipText*> GossipTextMap;
        typedef HM_NAMESPACE::hash_map<uint32, AreaTriggerPoint*> AreaTriggerMap;
        typedef HM_NAMESPACE::hash_map<uint32, std::string> ItemTextMap;

        GroupSet            mGroupSet;
        GuildSet            mGuildSet;

        ItemMap             mItems;
        ItemMap             mAitems;

        ItemTextMap         mItemTexts;

        AuctionHouseObject  mHordeAuctions;
        AuctionHouseObject  mAllianceAuctions;
        AuctionHouseObject  mNeutralAuctions;

        AreaTriggerMap      mAreaTriggerMap;
        GossipTextMap       mGossipText;
        TeleportMap         mTeleports;

        RepOnKillMap        mRepOnKill;

        WeatherZoneMap      mWeatherZoneMap;

    private:
        typedef std::map<uint32,PetLevelInfo*> PetLevelInfoMap;
        // PetLevelInfoMap[creature_id][level]
        PetLevelInfoMap petInfo;                            // [creature_id][level]

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;
        PlayerInfo **playerInfo;                            // [race][class]

        typedef std::map<uint32,std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;
};

#define objmgr MaNGOS::Singleton<ObjectMgr>::Instance()
#endif
