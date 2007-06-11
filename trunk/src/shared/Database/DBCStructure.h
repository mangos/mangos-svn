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

// Structures using to access raw DBC data and required packing to portability

// Only GCC 4.1.0 and later support #pragma pack(push,1) syntax
#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
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
    char*     area_name[8];                                 //11-18
                                                            //19, string flags, unused
    uint32    team;                                         //20,
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
};

struct BattlemasterListEntry
{
    uint32      id;                                         // 0
    uint32      mapid1;                                     // 1
    uint32      mapid2;                                     // 2 for arenas...
                                                            // 3-8 unused
    uint32      type;                                       // 9 (3 - BG, 4 - arena)
    uint32      minlvl;                                     //10
    uint32      maxlvl;                                     //11
    uint32      maxplayersperteam;                          //12
                                                            //13-14 unused
    char*       name[8];                                    //15-22
                                                            //23 string flag, unused
};

struct ChatChannelsEntry
{
    uint32      ChannelID;                                  // 0
    uint32      flags;                                      // 1
                                                            // 2 unused
    char*       pattern[8];                                 // 3-10
                                                            // 11 string flags, unused
    //char*       name[8];                                  // 12-19, unused
                                                            // 13 string flags, unused
};

struct ChrClassesEntry
{
    uint32      ClassID;                                    // 0
                                                            // 1-2, unused
    uint32      powerType;                                  // 3
                                                            // 4, unused
    //char*       name[8];                                  // 5-12, unused
                                                            // 13 string flags, unused
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
    char*       name[8];                                    // 14-21 used for DBC language detection/selection
                                                            // 22 string flags, unused
};

struct CreatureFamilyEntry
{
    uint32    ID;                                           // 0
                                                            // 1-5 unused
    uint32    tamable;                                      // 6 if this = 270 then "is tamable Creature" (or 0 is non-tamable)
    uint32    petFoodMask;                                  // 7
    char*     Name[8];                                      // 8-15
                                                            // 16 string flags, unused
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                       // 0
    uint32    multiplier[29];                                // 1-29
};

struct DurabilityQualityEntry
{
    uint32    Id;                                            // 0
    float     quality_mod;                                   // 1
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
    char*       name[8];                                    //19-26
                                                            //27 string flags
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
    uint32      ID;
    uint32      faction;
    uint32      ourMask;                                    // if mask set (see FactionMasks) then faction included in masked team
    uint32      friendlyMask;                               // if mask set (see FactionMasks) then faction friendly to masked team
    uint32      hostileMask;                                // if mask set (see FactionMasks) then faction hostile to masked team
    uint32      enemyFaction1;
    uint32      enemyFaction2;
    //uint32      enemyFaction3;                            // empty in current DBC and can be ignored while
    //uint32      enemyFaction4;                            // empty in current DBC and can be ignored while
    uint32      friendFaction1;
    uint32      friendFaction2;
    uint32      friendFaction3;
    uint32      friendFaction4;

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction /*|| enemyFaction3 == entry.faction || enemyFaction4 == entry.faction*/ )
            return false;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return true;
        return friendlyMask & entry.ourMask;
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction /*|| enemyFaction3 == entry.faction || enemyFaction4 == entry.faction*/ )
            return true;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return false;
        return hostileMask & entry.ourMask;
    }
    bool IsHostileToPlayer() const { return hostileMask & FACTION_MASK_PLAYER; }
    bool IsNeutralToAll() const { return hostileMask == 0 && friendlyMask == 0 && enemyFaction1==0 && enemyFaction2==0; }
};

struct GemPropertiesEntry
{
    uint32      ID;
    uint32      spellitemenchantement;
    uint32      color;
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
    uint32    ID;
    uint32    enchant_id[3];
};

struct ItemSetEntry
{
                                                            // 0 unused
    char*     name[8];                                      // 1-8
                                                            // 9 string flags, unused
                                                            //10-26 unused
    uint32    spells[8];                                    //27-34
    uint32    items_to_triggerspell[8];                     //35-42
    uint32    required_skill_id;                            //43
    uint32    required_skill_value;                         //44
};

struct LockEntry
{
    uint32      ID;
    uint32      key;
    uint32      requiredskill;
    uint32      requiredlockskill;
};

struct MapEntry
{
    uint32      MapID;
    char*       name;
    uint32      map_type;
    uint32      map_flag;
};

struct SkillLineEntry
{
    uint32    id;
    uint32    categoryId;
    char*     name[8];
    char*     description[8];
    uint32    spellIcon;
};

struct SkillLineAbilityEntry
{
    uint32    id;
    uint32    skillId;
    uint32    spellId;
    uint32    req_skill_value;                              //for trade skill.not for training.
    uint32    forward_spellid;
    uint32    max_value;
    uint32    min_value;
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
    int32     EffectBasePoints[3];                          //79-81
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
    char*     SpellName[8];                                 //123-130
    //uint32    SpellNameFlag;                              //131     string flags, not used
    char*     Rank[8];                                      //132-139
    //uint32    RankFlags;                                  //140     string flags, not used
    //char*   Description[8];                               //141-148 not used
    //uint32  DescriptionFlags;                             //149     string flags, not used
    //char*   ToolTip[8];                                   //150-157 not used
    //uint32  ToolTipFlags;                                 //158     string flags, not used
    uint32    ManaCostPercentage;                           //159
    uint32    StartRecoveryCategory;                        //160
    uint32    StartRecoveryTime;                            //161
    uint32    AffectedTargetLevel;                          //162
    uint32    SpellFamilyName;                              //163
    uint64    SpellFamilyFlags;                             //164+165
    uint32    MaxAffectedTargets;                           //166
    uint32    DmgClass;                                     //167
                                                            //168-169 not used (DmgClass2, DmgClass3 ?)
    float     DmgMultiplier[3];                             //170-172
                                                            //173-175 not used
    uint32    TotemCategory[2];                             //176-177
    uint32    AreaId;                                       //178
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
    char*     Name[8];                                      // 1-8 unused
                                                            // 9 string flags, unused
};
*/

// stored in SQL table
struct SpellProcEventEntry
{
    uint32      spellId;                                    // obviously, id of spell capable to proc on event
    uint32      schoolMask;                                 // if nonzero - bit mask for matching proc condition based on spell candidate's school: Fire=2, Mask=1<<(2-1)=2
    uint32      category;                                   // if nonzero - match proc condition based on candidate spell's category
    uint32      skillId;                                    // if nonzero - for matching proc condition based on candidate spell's skillId from SkillLineAbility.dbc (Shadow Bolt = Destruction)
    uint32      spellMask;                                  // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags (like auras 107 and 108 do)
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

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0
    uint32      display_type[3];                            // 1-3
    uint32      amount[3];                                  // 4-6
    //uint32    amount2[3]                                  // 7-9 always same as similar `amount` value
    uint32      spellid[3];                                 // 10-12
    char*       description[8];                             // 13-20
                                                            // 21 description flags, unused
    uint32      aura_id;                                    // 22
    uint32      slot;                                       // 23
    uint32      GemID;                                      // 24
    uint32      EnchantmentCondition;                       // 25
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
    uint32    TalentTabID;
    uint32    ClassMask;
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*     name[8];                                    // 5-12, unused
                                                            //13 string flags, unused
    uint32    horde_mount_type;                             //14 
    uint32    alliance_mount_type;                          //15
};

struct WorldSafeLocsEntry
{
    uint32    ID;
    uint32    map_id;
    float     x;
    float     y;
    float     z;
};

#if defined( __GNUC__ ) && (GCC_MAJOR < 4 || GCC_MAJOR == 4 && GCC_MINOR < 1)
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
