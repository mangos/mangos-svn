/* DBCStores.h
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

#ifndef __SPELLSTORE_H
#define __SPELLSTORE_H

#include "Common.h"
#include "DataStore.h"
#include "Timer.h"

// Struct for the entry in Spell.dbc
struct bidentry
{
    uint32 AuctionID;
    uint32 amt;
};
struct AuctionEntry
{
    uint32 auctioneer;
    uint32 item;
    uint32 owner;
    uint32 bid;
    uint32 buyout;
    time_t time;
    uint32 bidder;
    uint32 Id;
};
struct TalentEntry
{
    uint32 TalentID;
    uint32 TalentTree;
    uint32 unk1;
    uint32 unk2;
    uint32 RankID[4];
    uint32 unk[11];
};
struct emoteentry
{
    uint32 Id;
    uint32 name;
    uint32 textid;
    uint32 textid2;
    uint32 textid3;
    uint32 textid4;
    uint32 unk1;
    uint32 textid5;
    uint32 unk2;
    uint32 textid6;
    uint32 unk3;
    uint32 unk4;
    uint32 unk5;
    uint32 unk6;
    uint32 unk7;
    uint32 unk8;
    uint32 unk9;
    uint32 unk10;
    uint32 unk11;
};

struct skilllinespell
{
    uint32 Id;
    uint32 skilline;
    uint32 spell;
    uint32 unk1;
    uint32 unk2;
    uint32 unk3;
    uint32 unk4;
    uint32 unk5;
    uint32 minrank;
    uint32 next;
    uint32 grey;
    uint32 green;
    uint32 unk10;
    uint32 unk11;
    uint32 unk12;
};
struct SpellEntry
{
    uint32 Id;
    uint32 School;
    uint32 Category;
    uint32 field4;
    uint32 field5;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 field8;
    uint32 field9;
    uint32 Targets;
    uint32 TargetCreatureType;
    uint32 RequiresSpellFocus;
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CastingTimeIndex;
    uint32 RecoveryTime;
    uint32 CategoryRecoveryTime;
    uint32 InterruptFlags;
    uint32 AuraInterruptFlags;
    uint32 ChannelInterruptFlags;
    uint32 procFlags;
    uint32 procChance;
    uint32 procCharges;
    uint32 maxLevel;
    uint32 baseLevel;
    uint32 spellLevel;
    uint32 DurationIndex;
    uint32 powerType;
    uint32 manaCost;
    uint32 manaCostPerlevel;
    uint32 manaPerSecond;
    uint32 manaPerSecondPerLevel;
    uint32 rangeIndex;
    float speed;
    uint32 modalNextSpell;
    uint32 field36;
    uint32 Totem[2];
    uint32 Reagent[8];
    uint32 ReagentCount[8];
    uint32 EquippedItemClass;
    uint32 EquippedItemSubClass;
    uint32 Effect[3];
    uint32 EffectDieSides[3];
    uint32 EffectBaseDice[3];
    float EffectDicePerLevel[3];
    float EffectRealPointsPerLevel[3];
    int32 EffectBasePoints[3];
    uint32 EffectImplicitTargetA[3];
    uint32 EffectImplicitTargetB[3];
    uint32 EffectRadiusIndex[3];
    uint32 EffectApplyAuraName[3];
    uint32 EffectAmplitude[3];
    uint32 Effectunknown[3];
    uint32 EffectChainTarget[3];
    uint32 EffectItemType[3];
    uint32 EffectMiscValue[3];
    uint32 EffectTriggerSpell[3];
    float EffectPointsPerComboPoint[3];
    uint32 SpellVisual;
    uint32 field110;
    uint32 SpellIconID;
    uint32 activeIconID;
    uint32 spellPriority;
    uint32 Name;
    uint32 NameAlt1;
    uint32 NameAlt2;
    uint32 NameAlt3;
    uint32 NameAlt4;
    uint32 NameAlt5;
    uint32 NameAlt6;
    uint32 NameAlt7;
    uint32 NameFlags;
    uint32 Rank;
    uint32 RankAlt1;
    uint32 RankAlt2;
    uint32 RankAlt3;
    uint32 RankAlt4;
    uint32 RankAlt5;
    uint32 RankAlt6;
    uint32 RankAlt7;
    uint32 RankFlags;
    uint32 Description;
    uint32 DescriptionAlt1;
    uint32 DescriptionAlt2;
    uint32 DescriptionAlt3;
    uint32 DescriptionAlt4;
    uint32 DescriptionAlt5;
    uint32 DescriptionAlt6;
    uint32 DescriptionAlt7;
    uint32 DescriptionFlags;
    uint32 BuffDescription;
    uint32 BuffDescriptionAlt1;
    uint32 BuffDescriptionAlt2;
    uint32 BuffDescriptionAlt3;
    uint32 BuffDescriptionAlt4;
    uint32 BuffDescriptionAlt5;
    uint32 BuffDescriptionAlt6;
    uint32 BuffDescriptionAlt7;
    uint32 ManaCostPercentage;
    uint32 StartRecoveryCategory;
    uint32 StartRecoveryTime;
    uint32 field151;
    uint32 field152;
    uint32 SpellFamilyName;
    int32 field154;
    uint32 field155;
    uint32 field156;
};
struct Trainerspell
{
    uint32 Id;
    uint32 skilline1;
    uint32 skilline2;
    uint32 skilline3;
    uint32 maxlvl;
    uint32 charclass;
};
struct SpellCastTime
{
    uint32 ID;
    uint32 CastTime;
    uint32 unk1;
    uint32 unk2;
};

struct SpellRadius
{
    uint32 ID;
    float Radius;
    float unk1;
    float Radius2;
};

struct SpellRange
{
    uint32 ID;
    float minRange;
    float maxRange;
    uint32 unks[18];
};

struct SpellDuration
{
    uint32 ID;
    uint32 Duration1;
    uint32 Duration2;
    uint32 Duration3;
};

//Made by Andre2k2
struct AreaTableEntry
{
	uint32 ID;
	uint32 map;
	uint32 zone;
	uint32 exploreFlag;
	uint32 ukn1;
	uint32 ukn2;
	uint32 ukn3;
	uint32 ukn4;
	uint32 ukn5;
	uint32 ukn6;
	uint32 ukn7;
	uint32 name;
	uint32 ukn8;
	uint32 ukn9;
	uint32 ukn10;
	uint32 ukn11;
	uint32 ukn12;
	uint32 ukn13;
	uint32 ukn14;
	uint32 ukn15;
	uint32 ukn16;
};

//Made by Andre2k2
struct WorldMapAreaEntry
{
    uint32 ID;
	uint32 map;
	uint32 areaTableID;
	uint32 name;
	float  areaVertexY1;
	float  areaVertexY2;
	float  areaVertexX1;
	float  areaVertexX2;
};

//Made by Andre2k2
struct WorldMapOverlayEntry
{
    uint32 ID;
	uint32 worldMapAreaID;
	uint32 areaTableID;
	uint32 unk1;
	uint32 unk2;
	uint32 unk3;
	uint32 unk4;
	uint32 unk5;
	uint32 name;
	uint32 areaW; //in pixels 2x
	uint32 areaH; //in pixels 2x
	uint32 unk6;  //I think columns #12, #13, #14 and #15
	uint32 unk7;  //are some kind of positions, but i did not
	uint32 unk8;  //discover what exactly.
	uint32 unk9;
	uint32 drawX; //in pixels
	uint32 drawY; //in pixels
};

float GetRadius(SpellRadius *radius);
uint32 GetCastTime(SpellCastTime *time);
float GetMinRange(SpellRange *range);
float GetMaxRange(SpellRange *range);
uint32 GetDuration(SpellDuration *dur);
// You need two lines like this for every new DBC store
defineIndexedDBCStore(SpellStore,SpellEntry);
defineIndexedDBCStore(DurationStore,SpellDuration);
defineIndexedDBCStore(RangeStore,SpellRange);
defineIndexedDBCStore(EmoteStore,emoteentry);
defineIndexedDBCStore(RadiusStore,SpellRadius);
defineIndexedDBCStore(CastTimeStore,SpellCastTime);
defineIndexedDBCStore(TalentStore,TalentEntry);
//Made by Andre2k2
defineIndexedDBCStore(AreaTableStore,AreaTableEntry);
defineIndexedDBCStore(WorldMapAreaStore,WorldMapAreaEntry);

defineDBCStore(WorldMapOverlayStore,WorldMapOverlayEntry);
//end Made
defineDBCStore(SkillStore,skilllinespell);

#define sSpellStore SpellStore::getSingleton()
#define sSkillStore SkillStore::getSingleton()
#define sEmoteStore EmoteStore::getSingleton()
#define sSpellDuration DurationStore::getSingleton()
#define sSpellRange RangeStore::getSingleton()
#define sSpellRadius RadiusStore::getSingleton()
#define sCastTime CastTimeStore::getSingleton()
#define sTalentStore TalentStore::getSingleton()
//Made by Andre2k2
#define sAreaTableStore AreaTableStore::getSingleton()
#define sWorldMapAreaStore WorldMapAreaStore::getSingleton()
#define sWorldMapOverlayStore WorldMapOverlayStore::getSingleton()
//end Made
#endif
