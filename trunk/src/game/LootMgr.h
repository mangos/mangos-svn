/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#ifndef MANGOS_LOOTMGR_H
#define MANGOS_LOOTMGR_H

#include "ItemEnchantmentMgr.h"
#include "ByteBuffer.h"
#include "Utilities/LinkedReference/RefManager.h"

#include <map>
#include <vector>

enum RollType
{
    ROLL_PASS         = 0,
    ROLL_NEED         = 1,
    ROLL_GREED        = 2
};

#define MAX_NR_LOOT_ITEMS 16
// note: the client cannot show more than 16 items total
#define MAX_NR_QUEST_ITEMS 32
// unrelated to the number of quest items shown, just for reserve

enum LootMethod
{
    FREE_FOR_ALL      = 0,
    ROUND_ROBIN       = 1,
    MASTER_LOOT       = 2,
    GROUP_LOOT        = 3,
    NEED_BEFORE_GREED = 4
};

enum PermissionTypes
{
    ALL_PERMISSION    = 0,
    GROUP_PERMISSION  = 1,
    MASTER_PERMISSION = 2,
    NONE_PERMISSION   = 3
};

enum LootConditionType
{                                                           // value1       value2  for the Condition enumed
    CONDITION_NONE                  = 0,                    // 0            0
    CONDITION_AURA                  = 1,                    // spell_id     effindex
    CONDITION_ITEM                  = 2,                    // item_id      count
    CONDITION_ITEM_EQUIPPED         = 3,                    // item_id      0
    CONDITION_ZONEID                = 4,                    // zone_id      0
    CONDITION_REPUTATION_RANK       = 5,                    // faction_id   min_rank
    CONDITION_TEAM                  = 6,                    // player_team  0,      (469 - Alliance 67 - Horde)
    CONDITION_SKILL                 = 7,                    // skill_id     skill_value
    CONDITION_QUESTREWARDED         = 8,                    // quest_id     0
    CONDITION_QUESTTAKEN            = 9,                    // quest_id     0,      for quest item that can looted in any amount while quest active.
};

#define MAX_CONDITION                 10                    // maximun value in this enum 

class Player;

struct LootStoreItem
{
    uint32  itemid;                                         // id of the item
    float   chance;                                         // always positive, chance to drop for both quest and non-quest items, chance to be used for refs
    int32   mincountOrRef;                                  // mincount for drop items (positive) or minus referenced TemplateleId (negative)
    uint8   group       :8;           
    uint8   maxcount    :8;                                 // max drop count for the item (mincountOrRef positive) or Ref multiplicator (mincountOrRef negative)
    uint16  conditionId :16;                                // additional loot condition Id
    bool    freeforall  :1;                                 // free for all (clone for every looter)
    bool    needs_quest :1;                                 // quest drop (negative ChanceOrQuestChance in DB)

    // Constructor, converting ChanceOrQuestChance -> (chance, needs_quest)
    // displayid is filled in IsValid() which must be called after
    LootStoreItem(uint32 _itemid, float _chanceOrQuestChance, int8 _group, bool _freeforall, uint8 _conditionId, int32 _mincountOrRef, uint8 _maxcount)
        : itemid(_itemid), chance(abs(_chanceOrQuestChance)), group(_group),
          mincountOrRef(_mincountOrRef), maxcount(_maxcount), freeforall(_freeforall), 
          needs_quest(_chanceOrQuestChance < 0), conditionId(_conditionId) {}

    bool Roll() const;                                      // Checks if the entry takes it's chance (at loot generation)
    bool IsValid(uint32 entry) const;                       // Checks correctness of values
};

struct LootCondition
{
    LootConditionType condition;                            // additional loot condition type
    uint32  value1;                                         // data for the condition - see LootConditionType definition
    uint32  value2;                     

    LootCondition(uint8 _condition = 0, uint32 _value1 = 0, uint32 _value2 = 0)
        : condition(LootConditionType(_condition)), value1(_value1), value2(_value2) {}

    bool IsValid() const;                                   // Checks correctness of values
    bool Meets(Player const * APlayer) const;               // Checks if the player meets the condition 
    bool operator == (LootCondition const& lc) const
    {
        return (lc.condition == condition && lc.value1 == value1 && lc.value2 == value2);
    }
};

struct LootItem
{
    uint32  itemid;
    uint32  randomSuffix;
    int32   randomPropertyId;
    uint16  conditionId       :16;                          // allow compiler pack structure
    uint8   count             : 8;
    bool    is_looted         : 1;
    bool    is_blocked        : 1;
    bool    freeforall        : 1;                          // free for all
    bool    is_underthreshold : 1;
    bool    is_counted        : 1;
    bool    needs_quest       : 1;                          // quest drop

    // Constructor, copies most fields from LootStoreItem, generates random count and random suffixes/properties
    // Should be called for non-reference LootStoreItem entries only (mincountOrRef > 0)
    explicit LootItem(LootStoreItem const& li);      
                
    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const * player) const;
};

struct QuestItem
{
    uint8   index;                                          // position in quest_items;
    bool    is_looted;

    QuestItem()
        : index(0), is_looted(false) {}

    QuestItem(uint8 _index, bool _islooted = false)
        : index(_index), is_looted(_islooted) {}
};

struct Loot;
class LootTemplate;

typedef std::vector<QuestItem> QuestItemList;
typedef std::map<uint32, QuestItemList *> QuestItemMap;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef HM_NAMESPACE::hash_map<uint32, LootTemplate*> LootTemplateMap;

class LootStore
{
    public:
        explicit LootStore(char const* name) : m_name(name) {}
        virtual ~LootStore() { Clear(); }

        void LoadLootTable();
        void Verify() const;

        bool HaveLootFor(uint32 loot_id) const { return m_LootTemplates.find(loot_id) != m_LootTemplates.end(); }
        bool HaveQuestLootFor(uint32 loot_id) const;
        bool HaveQuestLootForPlayer(uint32 loot_id,Player* player) const;

        LootTemplate const* GetLootFor(uint32 loot_id) const;

        char const* GetName() const { return m_name; }
    protected:
        void Clear();
    private:
        LootTemplateMap m_LootTemplates;
        char const* m_name;
};

class LootTemplate
{
    class  LootGroup;       // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup> LootGroups;

    public:
        // Adds an entry to the group (at loading stage)
        void AddEntry(LootStoreItem& item);
        // Rolls for every item in the template and adds the rolled items the the loot
        void Process(Loot& loot, LootStore const& store, uint8 GroupId = 0) const;

        // True if template includes at least 1 quest drop entry
        bool HasQuestDrop(LootTemplateMap const& store, uint8 GroupId = 0) const;
        // True if template includes at least 1 quest drop for an active quest of the player
        bool HasQuestDropForPlayer(LootTemplateMap const& store, Player const * player, uint8 GroupId = 0) const;

        // Checks integrity of the template
        void Verify(LootTemplateMap const& store, uint32 Id) const;
    private:
        LootStoreItemList Entries;  // not grouped only 
        LootGroups        Groups;   // groups have own (optimised) processing, grouped entries go there
};

//=====================================================

class LootValidatorRef :  public Reference<Loot, LootValidatorRef>
{
    public:
        LootValidatorRef() {}
        void targetObjectDestroyLink() {}
        void sourceObjectDestroyLink() {}
};

//=====================================================

class LootValidatorRefManager : public RefManager<Loot, LootValidatorRef>
{
    public:
        typedef LinkedListHead::Iterator< LootValidatorRef > iterator;

        LootValidatorRef* getFirst() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getFirst(); }
        LootValidatorRef* getLast() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getLast(); }

        iterator begin() { return iterator(getFirst()); }
        iterator end() { return iterator(NULL); }
        iterator rbegin() { return iterator(getLast()); }
        iterator rend() { return iterator(NULL); }
};

//=====================================================

struct Loot
{
    std::set<uint64> PlayersLooting;
    QuestItemMap PlayerQuestItems;
    QuestItemMap PlayerFFAItems;
    QuestItemMap PlayerNonQuestNonFFAConditionalItems;
    std::vector<LootItem> items;
    std::vector<LootItem> quest_items;
    // All rolls are registered here. They need to know, when the loot is not valid anymore
    LootValidatorRefManager i_LootValidatorRefManager;
    uint32 gold;
    uint8 unlootedCount;

    Loot(uint32 _gold = 0) : gold(_gold), unlootedCount(0) {}
    ~Loot() { clear(); }

    // if loot becomes invalid this reference is used to inform the listener
    void addLootValidatorRef(LootValidatorRef* pLootValidatorRef)
    {
        i_LootValidatorRefManager.insertFirst(pLootValidatorRef);
    }

    // void clear();
    void clear()
    {
        items.clear(); gold = 0; PlayersLooting.clear();
        for (QuestItemMap::iterator itr = PlayerQuestItems.begin(); itr != PlayerQuestItems.end(); ++itr)
            delete itr->second;
        for (QuestItemMap::iterator itr = PlayerFFAItems.begin(); itr != PlayerFFAItems.end(); ++itr)
            delete itr->second;
        for (QuestItemMap::iterator itr = PlayerNonQuestNonFFAConditionalItems.begin(); itr != PlayerNonQuestNonFFAConditionalItems.end(); ++itr)
            delete itr->second;

        PlayerQuestItems.clear();
        PlayerFFAItems.clear();
        PlayerNonQuestNonFFAConditionalItems.clear();

        items.clear();
        quest_items.clear();
        gold = 0;
        unlootedCount = 0;
        i_LootValidatorRefManager.clearReferences();
    }

    bool empty() const { return items.empty() && gold == 0; }
    bool isLooted();
    void NotifyItemRemoved(uint8 lootIndex);
    void NotifyQuestItemRemoved(uint8 questIndex);
    void NotifyMoneyRemoved();
    void AddLooter(uint64 GUID) { PlayersLooting.insert(GUID); }
    void RemoveLooter(uint64 GUID) { PlayersLooting.erase(GUID); }

    void generateMoneyLoot(uint32 minAmount, uint32 maxAmount);
    void FillLoot(uint32 loot_id, LootStore const& store, Player* loot_owner);

    // Inserts the item into the loot (called by LootTemplate processors)
    void AddItem(LootStoreItem const & item);
};

struct LootView
{
    Loot &loot;
    QuestItemList *qlist;
    QuestItemList *ffalist;
    QuestItemList *conditionallist;
    Player *viewer;
    PermissionTypes permission;
    LootView(Loot &_loot, QuestItemList *_qlist, QuestItemList *_ffalist, QuestItemList *_conditionallist, Player *_viewer,PermissionTypes _permission = ALL_PERMISSION)
        : loot(_loot), qlist(_qlist), ffalist(_ffalist), conditionallist(_conditionallist), viewer(_viewer), permission(_permission) {}
};

extern LootStore LootTemplates_Creature;
extern LootStore LootTemplates_Fishing;
extern LootStore LootTemplates_Gameobject;
extern LootStore LootTemplates_Item;
extern LootStore LootTemplates_Pickpocketing;
extern LootStore LootTemplates_Skinning;
extern LootStore LootTemplates_Disenchant;
extern LootStore LootTemplates_Prospecting;

QuestItemList* FillFFALoot(Player* player, Loot *loot);
QuestItemList* FillQuestLoot(Player* player, Loot *loot);
QuestItemList* FillNonQuestNonFFAConditionalLoot(Player* player, Loot *loot);
void LoadLootTables();

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li);
ByteBuffer& operator<<(ByteBuffer& b, LootView const& lv);
#endif
