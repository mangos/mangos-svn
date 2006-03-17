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

#ifndef __SPELLSTORE_H
#define __SPELLSTORE_H

#include "Common.h"
#include "DataStore.h"
#include "Timer.h"


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
    uint32 skilline4;
    uint32 skilline5;
    uint32 skilline6;
    uint32 skilline7;
    uint32 skilline8;
    uint32 skilline9;
    uint32 skilline10;
    uint32 skilline11;
    uint32 skilline12;
    uint32 skilline13;
    uint32 skilline14;
    uint32 skilline15;
    uint32 skilline16;
    uint32 skilline17;
    uint32 skilline18;
    uint32 skilline19;
    uint32 skilline20;
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
    uint32 areaW;                                 
    uint32 areaH;                                 
    uint32 unk6;                                  
    uint32 unk7;                                  
    uint32 unk8;                                  
    uint32 unk9;
    uint32 drawX;                                 
    uint32 drawY;                                 
};

struct FactionEntry
{
	uint32 ID;
	int	reputationListID;
	uint32 unk1;
	uint32 unk2;
	uint32 unk3;
	uint32 unk4;
	uint32 unk5;
	uint32 unk6;
	uint32 unk7;
	uint32 something1;
	uint32 something2; 
	uint32 something3;
	uint32 something4;
	uint32 something5;
	uint32 something6;
	uint32 something7;
	uint32 something8;
	uint32 something9;
	uint32 faction;
	uint32 name; 
	uint32 unk8; 
	uint32 unk9;
	uint32 unk10;
	uint32 unk11;
	uint32 unk12;
	uint32 unk13;
	uint32 unk14;
	uint32 unk15;
	uint32 unk16;
	uint32 unk17; 
	uint32 unk18; 
	uint32 unk19;
	uint32 unk20;
	uint32 unk21;
	uint32 unk22;
	uint32 unk23;
	uint32 unk24;
};

struct FactionTemplateEntry
{
	uint32 ID;
	uint32 faction;
	uint32 unk1;
	uint32 fightsupport;
	uint32 friendly;
	uint32 hostile;
	uint32 unk2;
	uint32 unk3;
	uint32 unk4;
	uint32 unk5;
	uint32 unk6;
	uint32 unk7;
	uint32 unk8;
	uint32 unk9;
};

struct ItemDisplayTemplateEntry
{
	uint32 ID;
	char modelFile;
	uint32 unk1;
	char imageFile;
	uint32 unk2;
	char invImageFile;
	uint32 unk3;
	uint32 unk4;
	uint32 unk5;
	uint32 unk6;
	uint32 unk7;
	uint32 seed;
	uint32 unk9;
	uint32 unk10;
	uint32 unk11;
	uint32 unk12;
	uint32 unk13;
	uint32 unk14;
	uint32 unk15;
	uint32 unk16;
	uint32 unk17;
	uint32 unk18;
	uint32 randomPropertyID; 
};

float GetRadius(SpellRadius *radius);
uint32 GetCastTime(SpellCastTime *time);
float GetMinRange(SpellRange *range);
float GetMaxRange(SpellRange *range);
uint32 GetDuration(SpellDuration *dur);

defineIndexedDBCStore(SpellStore,SpellEntry);
defineIndexedDBCStore(DurationStore,SpellDuration);
defineIndexedDBCStore(RangeStore,SpellRange);
defineIndexedDBCStore(EmoteStore,emoteentry);
defineIndexedDBCStore(RadiusStore,SpellRadius);
defineIndexedDBCStore(CastTimeStore,SpellCastTime);
defineIndexedDBCStore(TalentStore,TalentEntry);

defineIndexedDBCStore(AreaTableStore,AreaTableEntry);
defineIndexedDBCStore(WorldMapAreaStore,WorldMapAreaEntry);

defineDBCStore(WorldMapOverlayStore,WorldMapOverlayEntry);

defineIndexedDBCStore(FactionStore,FactionEntry);
defineIndexedDBCStore(FactionTemplateStore,FactionTemplateEntry);

defineDBCStore(ItemDisplayTemplateStore,ItemDisplayTemplateEntry);

defineDBCStore(SkillStore,skilllinespell);

#define sSpellStore SpellStore::getSingleton()
#define sSkillStore SkillStore::getSingleton()
#define sEmoteStore EmoteStore::getSingleton()
#define sSpellDuration DurationStore::getSingleton()
#define sSpellRange RangeStore::getSingleton()
#define sSpellRadius RadiusStore::getSingleton()
#define sCastTime CastTimeStore::getSingleton()
#define sTalentStore TalentStore::getSingleton()

#define sAreaTableStore AreaTableStore::getSingleton()
#define sWorldMapAreaStore WorldMapAreaStore::getSingleton()
#define sWorldMapOverlayStore WorldMapOverlayStore::getSingleton()
#define sFactionStore FactionStore::getSingleton()
#define sFactionTemplateStore FactionTemplateStore::getSingleton()

#define sItemDisplayTemplateStore ItemDisplayTemplateStore::getSingleton()
#endif
