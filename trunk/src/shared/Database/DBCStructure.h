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
    uint32    ID;
    uint32    mapid;
    uint32    zone;                                         // if 0 then it's zone, else it's zone id of this area
    uint32    exploreFlag;
    uint32    flags;                                        // unknown value but 312 for all cities
    int32     area_level;
    uint32    team;
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
};

struct BattlemasterListEntry
{
    uint32      id;
    uint32      mapid1;
    uint32      mapid2; // for arenas...
    uint32      type; // 3 - BG, 4 - arena
    uint32      minlvl;
    uint32      maxlvl;
    uint32      maxplayersperteam;
    char*       name;
};

struct ChatChannelsEntry
{
    uint32      ChannelID;
    uint32      flags;
    char*       pattern;
    char*       name;
};

struct ChrClassesEntry
{
    uint32      ClassID;
    uint32      powerType;
    char*       name;
};

struct ChrRacesEntry
{
    uint32      RaceID;
    uint32      FactionID;                                  //facton template id
    uint32      model_m;
    uint32      model_f;
    uint32      TeamID;                                     // 7-Alliance 1-Horde
    //uint32      startingTaxiMask;                         // removed in 2.0.1
    uint32      startmovie;
    char*       name;
};

struct CreatureFamilyEntry
{
    uint32    ID;
    uint32    tamable;                                      //if this = 270 then "is tamable Creature" (or 0 is non-tamable)
    uint32    petFoodMask;
    char*     Name;
};

struct EmotesTextEntry
{
    uint32    Id;
    uint32    textid;
};

struct FactionEntry
{
    uint32      ID;
    int32       reputationListID;
    uint32      BaseRepMask[4];                             // Base reputation masks (see enum Races)
    int32       BaseRepValue[4];                            // Base reputation values
    uint32      team;
    char*       name;
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
    uint32    enchant_id_1;
    uint32    enchant_id_2;
    uint32    enchant_id_3;
};

struct ItemSetEntry
{
    char*     name;
    uint32    spells[8];
    uint32    items_to_triggerspell[8];
    uint32    required_skill_id;
    uint32    required_skill_value;
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
                                                            //17 not used
    uint32    CasterAuraState;                              //18
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
    uint32    SpellNameFlag;                                //131
    char*     Rank[8];                                      //132-139
    uint32    RankFlags;                                    //140
    //char*   Description[8];                               //141-148 not used
    //uint32  DescriptionFlags;                             //149     not used
    //char*   ToolTip[8];                                   //150-157 not used
    //uint32  ToolTipFlags;                                 //158     not used
    uint32    ManaCostPercentage;                           //159
    uint32    StartRecoveryCategory;                        //160
    uint32    StartRecoveryTime;                            //161
    uint32    AffectedTargetLevel;                          //162
    uint32    SpellFamilyName;                              //163
    uint32    SpellFamilyFlags;                             //164
    uint32    MaxAffectedTargets;                           //165
                                                            //166 not used
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

struct SpellFocusObjectEntry
{
    uint32    ID;
    char*     Name;
};

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
    char*       description;                                // 13
                                                            // 14-20 description2-description8
                                                            // 21 description flags
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
    uint32    ID;
    uint32    map_id;
    float     x;
    float     y;
    float     z;
    char*     name;
    uint32    horde_mount_type;
    uint32    alliance_mount_type;
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
