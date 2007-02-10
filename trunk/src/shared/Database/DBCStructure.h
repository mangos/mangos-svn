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
    uint32    zone;
    uint32    exploreFlag;
    uint32    zone_type;                                    // unknown value but 312 for all cities
    int32     area_level;
    uint32    team;
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
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
    uint32      TeamID;                                     // 7-Alliance 1-Horde
    uint32      startingTaxiMask;
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

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1 == entry.faction || enemyFaction2 == entry.faction)
            return false;
        return friendlyMask & entry.ourMask;
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if(enemyFaction1 == entry.faction || enemyFaction2 == entry.faction)
            return true;
        return hostileMask & entry.ourMask;
    }
    bool IsHostileToPlayer() const { return hostileMask & FACTION_MASK_PLAYER; }
    bool IsNeutralToAll() const { return hostileMask == 0 && friendlyMask == 0 && enemyFaction1==0 && enemyFaction2==0; } 
};

struct ItemDisplayInfoEntry
{
    uint32      ID;
    uint32      randomPropertyChance;
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
    uint32    Id;
    uint32    School;
    uint32    Category;
    uint32    Dispel;                                       //4
    uint32    Mechanic;                                     //5
    uint32    Attributes;                                   //6
    uint32    AttributesEx;                                 //7
    uint32    AttributesEx2;                                //8
    uint32    AttributesExEx;                               //9
    uint32    Stances;
    uint32    Targets;
    uint32    TargetCreatureType;
    uint32    RequiresSpellFocus;
    uint32    CasterAuraState;
    uint32    CastingTimeIndex;
    uint32    RecoveryTime;
    uint32    CategoryRecoveryTime;
    uint32    InterruptFlags;
    uint32    AuraInterruptFlags;
    uint32    ChannelInterruptFlags;
    uint32    procFlags;
    uint32    procChance;
    uint32    procCharges;
    uint32    maxLevel;
    uint32    baseLevel;
    uint32    spellLevel;
    uint32    DurationIndex;
    int32     powerType;
    uint32    manaCost;
    uint32    manaCostPerlevel;
    uint32    manaPerSecond;
    uint32    manaPerSecondPerLevel;
    uint32    rangeIndex;
    float     speed;
    uint32    modalNextSpell;                               //38
    uint32    StackAmount;
    uint32    Totem[2];                                     //40-41
    uint32    Reagent[8];                                   //42-49
    uint32    ReagentCount[8];                              //50-57
    int32     EquippedItemClass;                            //58 (value)
    int32     EquippedItemSubClassMask;                     //59 (mask)
    int32     EquippedItemInventoryTypeMask;                //60 (mask)
    uint32    Effect[3];                                    //61-63
    int32     EffectDieSides[3];                            //64-66
    uint32    EffectBaseDice[3];                            //67-69
    float     EffectDicePerLevel[3];                        //70-72
    float     EffectRealPointsPerLevel[3];                  //73-75
    int32     EffectBasePoints[3];                          //76-78
    uint32    EffectImplicitTargetA[3];                     //82-84
    uint32    EffectImplicitTargetB[3];                     //85-87
    uint32    EffectRadiusIndex[3];                         //88-90 - spellradius.dbc
    uint32    EffectApplyAuraName[3];                       //91-93
    uint32    EffectAmplitude[3];                           //94-96
    float     EffectMultipleValue[3];                       //97-99
    uint32    EffectChainTarget[3];                         //100-102
    uint32    EffectItemType[3];                            //103-105
    uint32    EffectMiscValue[3];                           //106-108
    uint32    EffectTriggerSpell[3];
    float     EffectPointsPerComboPoint[3];
    uint32    SpellVisual;
    uint32    SpellIconID;
    uint32    activeIconID;
    uint32    spellPriority;
    char*     SpellName[8];
    uint32    SpellNameFlag;
    char*     Rank[8];
    uint32    RankFlags;
    uint32    ManaCostPercentage;
    uint32    StartRecoveryCategory;
    uint32    StartRecoveryTime;
    uint32    AffectedTargetLevel;
    uint32    SpellFamilyName;
    uint32    SpellFamilyFlags;
    uint32    MaxAffectedTargets;
    uint32    DmgClass;
    float     DmgMultiplier[3];
};

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
    uint32      ID;
    uint32      display_type;
    uint32      value1;
    uint32      value2;
    uint32      spellid;
    char*       description;
    uint32      aura_id;
    uint32      slot;
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

// Structures not used for casting to loaded DBC data and not requred then packing

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
