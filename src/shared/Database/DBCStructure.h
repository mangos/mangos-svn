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

#ifndef DBCSTRUCTURE_H
#define DBCSTRUCTURE_H

#include <map>
#include <set>
#include <vector>

// Structures using to access raw DBC data and required packing to portability

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

enum AreaTeams
{
    AREATEAM_NONE  = 0,
    AREATEAM_ALLY  = 2,
    AREATEAM_HORDE = 4
};

struct AreaTableEntry
{
    uint32    ID;                                           // 0
    uint32    mapid;                                        // 1
    uint32    zone;                                         // 2 if 0 then it's zone, else it's zone id of this area
    uint32    exploreFlag;                                  // 3, main index
    uint32    flags;                                        // 4, unknown value but 312 for all cities
                                                            // 5-9 unused
    int32     area_level;                                   //10
    char*     area_name[16];                                //11-26
                                                            //27, string flags, unused
    uint32    team;                                         //28
};

struct AreaTriggerEntry
{
    uint32    id;                                           // 0
    uint32    mapid;                                        // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    float     radius;                                       // 5
    float     box_x;                                        // 6 extent x edge
    float     box_y;                                        // 7 extent y edge
    float     box_z;                                        // 8 extent z edge
    float     box_orientation;                              // 9 extent rotation by about z axis
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
};

struct BattlemasterListEntry
{
    uint32      id;                                         // 0
    uint32      mapid[3];                                   // 1-3 mapid
                                                            // 4-8 unused
    uint32      type;                                       // 9 (3 - BG, 4 - arena)
    uint32      minlvl;                                     //10
    uint32      maxlvl;                                     //11
    uint32      maxplayersperteam;                          //12
                                                            //13-14 unused
    char*       name[16];                                   //15-30
                                                            //31 string flag, unused
};

struct ChatChannelsEntry
{
    uint32      ChannelID;                                  // 0
    uint32      flags;                                      // 1
    char*       pattern[16];                                // 3-18
                                                            //19 string flags, unused
    //char*       name[16];                                  //20-35 unused
                                                            //36 string flag, unused
};

struct ChrClassesEntry
{
    uint32      ClassID;                                    // 0
                                                            // 1-2, unused
    uint32      powerType;                                  // 3
                                                            // 4, unused
    //char*       name[16];                                 // 5-20 unused
                                                            //21 string flag, unused
};

struct ChrRacesEntry
{
    uint32      RaceID;                                     // 0
                                                            // 1 unused
    uint32      FactionID;                                  // 2 facton template id
                                                            // 3 unused
    uint32      model_m;                                    // 4
    uint32      model_f;                                    // 5
                                                            // 6-7 unused
    uint32      TeamID;                                     // 8 (7-Alliance 1-Horde)
                                                            // 9-12 unused
    uint32      startmovie;                                 // 13
    char*       name[16];                                   // 14-29 used for DBC language detection/selection
                                                            // 30 string flags, unused
};

struct CreatureDisplayInfoEntry
{
    uint32      Displayid;                                  //0
                                                            //1-3,unused
    float       scale;                                      //4
                                                            //5-20,unused
};

struct CreatureFamilyEntry
{
    uint32    ID;                                           // 0
                                                            // 1-5 unused
    uint32    tamable;                                      // 6 if this = 270 then "is tamable Creature" (or 0 is non-tamable)
    uint32    petFoodMask;                                  // 7
    char*     Name[16];                                     // 8-23
                                                            // 24 string flags, unused
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                      // 0
    uint32    multiplier[29];                               // 1-29
};

struct DurabilityQualityEntry
{
    uint32    Id;                                           // 0
    float     quality_mod;                                  // 1
};

struct EmotesTextEntry
{
    uint32    Id;
    uint32    textid;
};

struct FactionEntry
{
    uint32      ID;                                         // 0
    int32       reputationListID;                           // 1
    uint32      BaseRepMask[4];                             // 2-5 Base reputation masks (see enum Races)
                                                            // 6-9 unused
    int32       BaseRepValue[4];                            //10-13 Base reputation values
                                                            //14-17 unused
    uint32      team;                                       //18
    char*       name[16];                                   //19-34
                                                            //35 string flags, unused
    //char*     description[16];                            //36-51 unused
                                                            //52 string flags, unused
};

enum FactionMasks
{
    FACTION_MASK_PLAYER   = 1,                              // any player
    FACTION_MASK_ALLIANCE = 2,                              // player or creature from alliance team
    FACTION_MASK_HORDE    = 4,                              // player or creature from horde team
    FACTION_MASK_MONSTER  = 8                               // aggressive creature from monster team
                                                            // if none flags set then non-aggressive creature
};

struct FactionTemplateEntry
{
    uint32      ID;                                         // 0
    uint32      faction;                                    // 1
                                                            // 2 not used
    uint32      ourMask;                                    // 3 if mask set (see FactionMasks) then faction included in masked team
    uint32      friendlyMask;                               // 4 if mask set (see FactionMasks) then faction friendly to masked team
    uint32      hostileMask;                                // 5 if mask set (see FactionMasks) then faction hostile to masked team
    uint32      enemyFaction1;                              // 6
    uint32      enemyFaction2;                              // 7
    uint32      enemyFaction3;                              // 8
    uint32      enemyFaction4;                              // 9
    uint32      friendFaction1;                             // 10
    uint32      friendFaction2;                             // 11
    uint32      friendFaction3;                             // 12
    uint32      friendFaction4;                             // 13
    //-------------------------------------------------------  end structure

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction || enemyFaction3 == entry.faction || enemyFaction4 == entry.faction )
            return false;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return true;
        return (friendlyMask & entry.ourMask) != 0;
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction || enemyFaction3 == entry.faction || enemyFaction4 == entry.faction )
            return true;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return false;
        return (hostileMask & entry.ourMask) != 0;
    }
    bool IsHostileToPlayers() const { return hostileMask & FACTION_MASK_PLAYER; }
    bool IsNeutralToAll() const { return hostileMask == 0 && friendlyMask == 0 && enemyFaction1==0 && enemyFaction2==0 && enemyFaction3==0 && enemyFaction4==0; }
};

struct GemPropertiesEntry
{
    uint32      ID;
    uint32      spellitemenchantement;
    uint32      color;
};

#define GT_MAX_LEVEL    100
struct GtCombatRatingsEntry
{
    float    ratio;
};

struct GtChanceToMeleeCritBaseEntry
{
    float    base;
};

struct GtChanceToMeleeCritEntry
{
    float    ratio;
};

struct GtChanceToSpellCritBaseEntry
{
    float    base;
};

struct GtChanceToSpellCritEntry
{
    float    ratio;
};

struct GtOCTRegenHPEntry
{
    float    ratio;
};

struct GtOCTRegenMPEntry
{
    float    ratio;
};

struct GtRegenHPPerSptEntry
{
    float    ratio;
};

struct GtRegenMPPerSptEntry
{
    float    ratio;
};

struct ItemDisplayInfoEntry
{
    uint32      ID;
    uint32      randomPropertyChance;
};

struct ItemExtendedCostEntry
{
    uint32      ID;                                         // extended-cost entry id
    uint32      reqhonorpoints;                             // required honor points
    uint32      reqarenapoints;                             // required arena points
    uint32      reqitem1;                                   // 1st required item id
    uint32      reqitem2;                                   // 2nd required item id
    uint32      reqitem3;                                   // 3rd required item id
    uint32      reqitemcount1;                              // required count of 1st item
    uint32      reqitemcount2;                              // required count of 2st item
    uint32      reqitemcount3;                              // required count of 3st item
};

struct ItemRandomPropertiesEntry
{
    uint32    ID;                                           //0
    //char*     internalName                                //1   unused
    uint32    enchant_id[3];                                //2-4
                                                            //5-6 unused, 0 only values, reserved for additional enchantments?
    //char*     nameSuffix[16]                              //7-22, unused
                                                            //23 nameSufix flags, unused
};

struct ItemRandomSuffixEntry
{
    uint32    ID;                                           //0
    //char*     name[16]                                    //1-16 unused
                                                            //17, name flags, unused
                                                            //18  unused
    uint32    enchant_id[3];                                //19-21
    uint32    prefix[3];                                    //22-24
};

struct ItemSetEntry
{
                                                            // 0 unused
    char*     name[16];                                     // 1-16
                                                            //17 string flags, unused
                                                            //18-43 unused
    uint32    spells[8];                                    //44-52
    uint32    items_to_triggerspell[8];                     //53-61
    uint32    required_skill_id;                            //62
    uint32    required_skill_value;                         //63
};

struct LockEntry
{
    uint32      ID;                                         //0
    uint32      type[5];                                    //1-5
                                                            //6-8, not used
    uint32      key[5];                                     //9-13
                                                            //14-16, not used
    uint32      requiredminingskill;                        //17
    uint32      requiredlockskill;                          //18
                                                            //19-32, not used
};

struct MapEntry
{
    uint32      MapID;                                      //0
    //char*       internalname;                             //1 unused
    uint32      map_type;                                   //2
                                                            //3 unused
    char*       name[16];                                   //4-19
                                                            //20 name flags, unused
                                                            //21-23 unused (something PvPZone related - levels?)
                                                            //24-27
    //char*     hordeIntro                                  //28-43 text for PvP Zones
                                                            //44 intro text flags
    //char*     allianceIntro                               //45-60 text for PvP Zones
                                                            //46 intro text flags
                                                            //47-65 not used
    //chat*     unknownText1                                //66-81 unknown empty text fields, possible normal Intro text.
                                                            //82 text flags
    //chat*     heroicIntroText                             //83-98 heroic mode requirement text
                                                            //99 text flags
    //chat*     unknownText2                                //100-115 unknown empty text fields
                                                            //116 text flags
    int32       parent_map;                                 //117 map_id of parent map
    //float start_x                                         //118 enter x coordinate (if exist single entry)
    //float start_y                                         //119 enter y coordinate (if exist single entry)
                                                            //120-122
};

inline bool IsExpansionMap(MapEntry const* map)
{
    return map && (map->MapID == 530 || map->parent_map == 530);
}

struct SkillLineEntry
{
    uint32    id;                                           // 0 
    uint32    categoryId;                                   // 1
                                                            // 2 unknow, not used
    char*     name[16];                                     // 3-18
                                                            // 19 string flags, not used
    //char*     description[16];                            // 20-35, not used
                                                            // 36 string flags, not used
    uint32    spellIcon;                                    // 37
};

struct SkillLineAbilityEntry
{
    uint32    id;                                           // 0
    uint32    skillId;                                      // 1
    uint32    spellId;                                      // 2 INDEX
    uint32    racemask;                                     // 3 
    uint32    classmask;                                    // 4
                                                            // 5-6, unknown, always 0
    uint32    req_skill_value;                              // 7 for trade skill.not for training.
    uint32    forward_spellid;                              // 8
                                                            // 9
    uint32    max_value;                                    //10
    uint32    min_value;                                    //11
                                                            //12-13, unknown, always 0
    uint32    reqtrainpoints;                               //14
};

struct SpellEntry
{
    uint32    Id;                                           //0 normally counted from 0 field (but some tools start counting from 1, check this before tool use for data view!)
    uint32    School;                                       //1
    uint32    Category;                                     //2
                                                            //3 not used
    uint32    Dispel;                                       //4
    uint32    Mechanic;                                     //5
    uint32    Attributes;                                   //6
    uint32    AttributesEx;                                 //7
    uint32    AttributesEx2;                                //8
    uint32    AttributesEx3;                                //9
    uint32    AttributesExEx;                               //10
                                                            //11 not used
    uint32    Stances;                                      //12
                                                            //13 not used
    uint32    Targets;                                      //14
    uint32    TargetCreatureType;                           //15
    uint32    RequiresSpellFocus;                           //16
    uint32    CasterAuraState;                              //17
    uint32    TargetAuraState;                              //18
                                                            //19 not used
                                                            //20 not used
    uint32    CastingTimeIndex;                             //21
    uint32    RecoveryTime;                                 //22
    uint32    CategoryRecoveryTime;                         //23
    uint32    InterruptFlags;                               //24
    uint32    AuraInterruptFlags;                           //25
    uint32    ChannelInterruptFlags;                        //26
    uint32    procFlags;                                    //27
    uint32    procChance;                                   //28
    uint32    procCharges;                                  //29
    uint32    maxLevel;                                     //30
    uint32    baseLevel;                                    //31
    uint32    spellLevel;                                   //32
    uint32    DurationIndex;                                //33
    int32     powerType;                                    //34
    uint32    manaCost;                                     //35
    uint32    manaCostPerlevel;                             //36
    uint32    manaPerSecond;                                //37
    uint32    manaPerSecondPerLevel;                        //38
    uint32    rangeIndex;                                   //39
    float     speed;                                        //40
    uint32    modalNextSpell;                               //41
    uint32    StackAmount;                                  //42
    uint32    Totem[2];                                     //43-44
    uint32    Reagent[8];                                   //45-52
    uint32    ReagentCount[8];                              //53-60
    int32     EquippedItemClass;                            //61 (value)
    int32     EquippedItemSubClassMask;                     //62 (mask)
    int32     EquippedItemInventoryTypeMask;                //63 (mask)
    uint32    Effect[3];                                    //64-66
    int32     EffectDieSides[3];                            //67-69
    uint32    EffectBaseDice[3];                            //70-72
    float     EffectDicePerLevel[3];                        //73-75
    float     EffectRealPointsPerLevel[3];                  //76-78
    int32     EffectBasePoints[3];                          //79-81 (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
                                                            //82-84 not used
    uint32    EffectImplicitTargetA[3];                     //85-87
    uint32    EffectImplicitTargetB[3];                     //88-90
    uint32    EffectRadiusIndex[3];                         //91-93 - spellradius.dbc
    uint32    EffectApplyAuraName[3];                       //94-96
    uint32    EffectAmplitude[3];                           //97-99
    float     EffectMultipleValue[3];                       //100-102
    uint32    EffectChainTarget[3];                         //103-105
    uint32    EffectItemType[3];                            //106-108
    uint32    EffectMiscValue[3];                           //109-111
    uint32    EffectTriggerSpell[3];                        //112-114
    float     EffectPointsPerComboPoint[3];                 //115-117
    uint32    SpellVisual;                                  //118
                                                            //119 not used
    uint32    SpellIconID;                                  //120
    uint32    activeIconID;                                 //121
    uint32    spellPriority;                                //122
    char*     SpellName[16];                                //123-138
    //uint32    SpellNameFlag;                              //139
    char*     Rank[16];                                     //140-155
    //uint32    RankFlags;                                  //156
    //char*     Description[16];                            //157-172 not used
    //uint32    DescriptionFlags;                           //173     not used
    //char*     ToolTip[16];                                //174-189 not used
    //uint32    ToolTipFlags;                               //190     not used
    uint32    ManaCostPercentage;                           //191
    uint32    StartRecoveryCategory;                        //192
    uint32    StartRecoveryTime;                            //193
    uint32    AffectedTargetLevel;                          //194
    uint32    SpellFamilyName;                              //195
    uint64    SpellFamilyFlags;                             //196+197
    uint32    MaxAffectedTargets;                           //198
    uint32    DmgClass;                                     //199
    //uint32    unk2[2];                                    //200-201 not used
    float     DmgMultiplier[3];                             //202-204
    //uint32    unk3[3];                                    //205-207 not used null
    uint32    TotemCategory[2];                             //208-209
    uint32    AreaId;                                       //210

    private:
        // prevent creating custom entries (copy data from original infact)
        SpellEntry(SpellEntry const&);                      // DON'T must have implementation
};

typedef std::set<uint32> SpellCategorySet;
typedef std::map<uint32,SpellCategorySet > SpellCategoryStore;

struct SpellCastTimesEntry
{
    uint32    ID;
    uint32    CastTime;
};

/* unused
struct SpellFocusObjectEntry
{
    uint32    ID;                                           // 0
    char*     Name[16];                                     // 1-15 unused
                                                            // 16 string flags, unused
};
*/

// stored in SQL table
struct SpellProcEventEntry
{
    uint32      spellId;                                    // obviously, id of spell capable to proc on event
    uint32      schoolMask;                                 // if nonzero - bit mask for matching proc condition based on spell candidate's school: Fire=2, Mask=1<<(2-1)=2
    uint32      category;                                   // if nonzero - match proc condition based on candidate spell's category
    uint32      skillId;                                    // if nonzero - for matching proc condition based on candidate spell's skillId from SkillLineAbility.dbc (Shadow Bolt = Destruction)
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyNamer value
    uint64      spellFamilyMask;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags (like auras 107 and 108 do)
    uint32      procFlags;                                  // bitmask for matching proc event
    float       ppmRate;                                    // for melee (ranged?) damage spells - proc rate per minute. if zero, falls back to flat chance from Spell.dbc
};

struct SpellThreatEntry
{
    uint32      spellId;
    int32       threat;
};

struct SpellRadiusEntry
{
    uint32    ID;
    float     Radius;
    float     Radius2;
};

struct SpellRangeEntry
{
    uint32    ID;
    float     minRange;
    float     maxRange;
};

struct SpellDurationEntry
{
    uint32    ID;
    int32     Duration[3];
};

enum ItemEnchantmentType
{
    ITEM_ENCHANTMENT_TYPE_NONE         = 0,
    ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL = 1,
    ITEM_ENCHANTMENT_TYPE_DAMAGE       = 2,
    ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL  = 3,
    ITEM_ENCHANTMENT_TYPE_RESISTANCE   = 4,
    ITEM_ENCHANTMENT_TYPE_STAT         = 5,
    ITEM_ENCHANTMENT_TYPE_TOTEM        = 6
};

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0
    uint32      type[3];                                    // 1-3
    uint32      amount[3];                                  // 4-6
    //uint32    amount2[3]                                  // 7-9 always same as similar `amount` value
    uint32      spellid[3];                                 // 10-12
    char*       description[16];                            // 13-29
                                                            // 30 description flags
    uint32      aura_id;                                    // 31
    uint32      slot;                                       // 32
    uint32      GemID;                                      // 33
    uint32      EnchantmentCondition;                       // 34
};

struct SpellItemEnchantmentConditionEntry
{
    uint32  ID;
    uint8   Color[5];
    uint8   Comparator[5];
    uint8   CompareColor[5];
    uint32  Value[5];
};

struct StableSlotPricesEntry
{
    uint32 Slot;
    uint32 Price;
};

struct TalentEntry
{
    uint32    TalentID;
    uint32    TalentTab;
    uint32    Row;
    uint32    Col;
    uint32    RankID[5];
    uint32    DependsOn;
    uint32    DependsOnRank;
};

struct TalentTabEntry
{
    uint32    TalentTabID;                                  //0
    //char*   name[16];                                     //1-16, unused
    //uint32  nameFlags;                                    //17, unused
    //unit32  spellicon;                                    //18
                                                            //19 not used
    uint32    ClassMask;                                    //20
    //uint32  tabpage                                       //21
    //char*   internalname;                                 //22
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*     name[16];                                   // 5-21
                                                            //22 string flags, unused
    uint32    horde_mount_type;                             //23
    uint32    alliance_mount_type;                          //24
};

enum TotemCategoryType
{
    TOTEM_CATEGORY_TYPE_KNIFE   = 1,
    TOTEM_CATEGORY_TYPE_TOTEM   = 2,
    TOTEM_CATEGORY_TYPE_ROD     = 3,
    TOTEM_CATEGORY_TYPE_PICK    = 21,
    TOTEM_CATEGORY_TYPE_STONE   = 22,
    TOTEM_CATEGORY_TYPE_HAMMER  = 23,
    TOTEM_CATEGORY_TYPE_SPANNER = 24
};

struct TotemCategoryEntry
{
    uint32    ID;                                           // 0
    //char*   name[16];                                     // 1-16
                                                            //17 string flags, unused
    uint32    categoryType;                                 //18 (one for specialization)
    uint32    categoryMask;                                 //19 (compatibility mask for same type: different for totems, compatible from high to low for rods)
};

struct WorldSafeLocsEntry
{
    uint32    ID;
    uint32    map_id;
    float     x;
    float     y;
    float     z;
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

// Structures not used for casting to loaded DBC data and not required then packing
typedef std::map<uint32,uint32> TalentSpellCosts;

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0),price(0) {}
    TaxiPathBySourceAndDestination(uint32 _id,uint32 _price) : ID(_id),price(_price) {}

    uint32    ID;
    uint32    price;
};
typedef std::map<uint32,TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32,TaxiPathSetForSource> TaxiPathSetBySource;

struct TaxiPathNode
{
    TaxiPathNode() : mapid(0), x(0),y(0),z(0),actionFlag(0),delay(0) {}
    TaxiPathNode(uint32 _mapid, float _x, float _y, float _z, uint32 _actionFlag, uint32 _delay) : mapid(_mapid), x(_x),y(_y),z(_z),actionFlag(_actionFlag),delay(_delay) {}

    uint32    mapid;
    float     x;
    float     y;
    float     z;
    uint32    actionFlag;
    uint32    delay;
};
typedef std::vector<TaxiPathNode> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#define TaxiMaskSize 8
typedef uint32 TaxiMask[TaxiMaskSize];
#endif
