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

#include "DBCStores.h"
//#include "DataStore.h"
#include "Policies/SingletonImp.h"
#include "Log.h"
#include "ProgressBar.h"

#include "DBCfmt.cpp"

#include <map>

typedef std::map<uint16,uint32> AreaFlagByAreaID;
typedef std::map<uint32,uint32> AreaFlagByMapID;

DBCStorage <AreaTableEntry> sAreaStore(AreaTableEntryfmt);
static AreaFlagByAreaID sAreaFlagByAreaID;
static AreaFlagByMapID  sAreaFlagByMapID;                   // for instances without generated *.map files

DBCStorage <AreaTriggerEntry> sAreaTriggerStore(AreaTriggerEntryfmt);
DBCStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DBCStorage <BattlemasterListEntry> sBattlemasterListStore(BattlemasterListEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);

DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

DBCStorage <EmotesTextEntry> sEmotesTextStore(EmoteEntryfmt);

typedef std::map<uint32,SimpleFactionsList> FactionTeamMap;
static FactionTeamMap sFactionTeamMap;
DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <GemPropertiesEntry> sGemPropertiesStore(GemPropertiesEntryfmt);

DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore(GtCombatRatingsfmt);
DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(GtChanceToMeleeCritBasefmt);
DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore(GtChanceToMeleeCritfmt);
DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(GtChanceToSpellCritBasefmt);
DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore(GtChanceToSpellCritfmt);
DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore(GtOCTRegenHPfmt);
DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore(GtOCTRegenMPfmt);
DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore(GtRegenHPPerSptfmt);
DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore(GtRegenMPPerSptfmt);

DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
//DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
DBCStorage <ItemExtendedCostEntry> sItemExtendedCostStore(ItemExtendedCostEntryfmt);
DBCStorage <ItemRandomPropertiesEntry> sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DBCStorage <ItemRandomSuffixEntry> sItemRandomSuffixStore(ItemRandomSuffixfmt);

DBCStorage <LockEntry> sLockStore(LockEntryfmt);

DBCStorage <MapEntry> sMapStore(MapEntryfmt);

DBCStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DBCStorage <SkillLineAbilityEntry> sSkillLineAbilityStore(SkillLineAbilityfmt);

DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore(SpellItemEnchantmentConditionfmt);
DBCStorage <SpellEntry> sSpellStore(SpellEntryfmt);
SpellCategoryStore sSpellCategoryStore;

DBCStorage <SpellCastTimesEntry> sCastTimesStore(SpellCastTimefmt);
DBCStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
//DBCStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DBCStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DBCStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DBCStorage <StableSlotPricesEntry> sStableSlotPricesStore(StableSlotPricesfmt);
DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellCosts sTalentSpellCosts;
DBCStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);

// store absolute bit position for first rank for talent inspect
typedef std::map<uint32,uint32> TalentInspectMap;
static TalentInspectMap sTalentPosInInspect;
static TalentInspectMap sTalentTabSizeInInspect;
static uint32 sTalentTabPages[12/*MAX_CLASSES*/][3];

DBCStorage <TaxiNodesEntry> sTaxiNodesStore(TaxiNodesEntryfmt);
TaxiMask sTaxiNodesMask;

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
struct TaxiPathEntry
{
    uint32    ID;
    uint32    from;
    uint32    to;
    uint32    price;
};
static DBCStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathNodesByPath sTaxiPathNodesByPath;
struct TaxiPathNodeEntry
{
    uint32    path;
    uint32    index;
    uint32    mapid;
    float     x;
    float     y;
    float     z;
    uint32    actionFlag;
    uint32    delay;
};
static DBCStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);

DBCStorage <TotemCategoryEntry> sTotemCategoryStore(TotemCategoryEntryfmt);

DBCStorage <WorldMapAreaEntry>  sWorldMapAreaStore(WorldMapAreaEntryfmt);
DBCStorage <WorldSafeLocsEntry> sWorldSafeLocsStore(WorldSafeLocsEntryfmt);

typedef std::list<std::string> StoreProblemList;

static bool LoadDBC_assert_print(uint32 fsize,uint32 rsize, std::string filename)
{
    sLog.outError("ERROR: Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).",filename.c_str(),fsize,rsize);

    // assert must fail after function call
    return false;
}

template<class T>
inline void LoadDBC(barGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, std::string filename)
{
    // compatibility format and C++ structure sizes
    assert(DBCFile::GetFormatRecordSize(storage.fmt) == sizeof(T) || LoadDBC_assert_print(DBCFile::GetFormatRecordSize(storage.fmt),sizeof(T),filename));

    if(storage.Load(filename.c_str()))
        bar.step();
    else
    {
        // sort problematic dbc to (1) non compatible and (2) non-existed
        FILE * f=fopen(filename.c_str(),"rb");
        if(f)
        {
            char buf[100];
            snprintf(buf,100," (exist, but have %d fields instead %d) Wrong client version DBC file?",storage.fieldCount,strlen(storage.fmt));
            errlist.push_back(filename + buf);
            fclose(f);
        }
        else
            errlist.push_back(filename);
    }
}

void LoadDBCStores(std::string dataPath)
{
    std::string tmpPath="";

    const uint32 DBCFilesCount = 48;

    barGoLink bar( DBCFilesCount );

    StoreProblemList bad_dbc_files;

    LoadDBC(bar,bad_dbc_files,sAreaStore,                dataPath+"dbc/AreaTable.dbc");

    // must be after sAreaStore loading
    for(uint32 i = 0; i < sAreaStore.nCount; ++i)           // areaflag numbered from 0
    {
        if(AreaTableEntry const* area = sAreaStore.LookupEntry(i))
        {
            // fill AreaId->DBC records
            sAreaFlagByAreaID.insert(AreaFlagByAreaID::value_type(area->ID,area->exploreFlag));

            // fill MapId->DBC records ( skip sub zones and continents )
            if(area->zone==0 && area->mapid != 0 && area->mapid != 1 && area->mapid != 530 )
                sAreaFlagByMapID.insert(AreaFlagByMapID::value_type(area->mapid,area->exploreFlag));
        }
    }

    LoadDBC(bar,bad_dbc_files,sAreaTriggerStore,         dataPath+"dbc/AreaTrigger.dbc");
    LoadDBC(bar,bad_dbc_files,sBankBagSlotPricesStore,   dataPath+"dbc/BankBagSlotPrices.dbc");
    LoadDBC(bar,bad_dbc_files,sBattlemasterListStore,    dataPath+"dbc/BattlemasterList.dbc");
    LoadDBC(bar,bad_dbc_files,sChatChannelsStore,        dataPath+"dbc/ChatChannels.dbc");
    LoadDBC(bar,bad_dbc_files,sChrClassesStore,          dataPath+"dbc/ChrClasses.dbc");
    LoadDBC(bar,bad_dbc_files,sChrRacesStore,            dataPath+"dbc/ChrRaces.dbc");
    LoadDBC(bar,bad_dbc_files,sCreatureDisplayInfoStore, dataPath+"dbc/CreatureDisplayInfo.dbc");
    LoadDBC(bar,bad_dbc_files,sCreatureFamilyStore,      dataPath+"dbc/CreatureFamily.dbc");
    LoadDBC(bar,bad_dbc_files,sDurabilityCostsStore,     dataPath+"dbc/DurabilityCosts.dbc");
    LoadDBC(bar,bad_dbc_files,sDurabilityQualityStore,   dataPath+"dbc/DurabilityQuality.dbc");
    LoadDBC(bar,bad_dbc_files,sEmotesTextStore,          dataPath+"dbc/EmotesText.dbc");
    LoadDBC(bar,bad_dbc_files,sFactionStore,             dataPath+"dbc/Faction.dbc");
    for (uint32 i=0;i<sFactionStore.nCount; ++i)
    {
        FactionEntry const * faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList &flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }
    LoadDBC(bar,bad_dbc_files,sFactionTemplateStore,     dataPath+"dbc/FactionTemplate.dbc");
    LoadDBC(bar,bad_dbc_files,sGemPropertiesStore,       dataPath+"dbc/GemProperties.dbc");

    LoadDBC(bar,bad_dbc_files,sGtCombatRatingsStore,     dataPath+"dbc/gtCombatRatings.dbc");

    LoadDBC(bar,bad_dbc_files,sGtChanceToMeleeCritBaseStore, dataPath+"dbc/gtChanceToMeleeCritBase.dbc");
    LoadDBC(bar,bad_dbc_files,sGtChanceToMeleeCritStore, dataPath+"dbc/gtChanceToMeleeCrit.dbc");

    LoadDBC(bar,bad_dbc_files,sGtChanceToSpellCritBaseStore, dataPath+"dbc/gtChanceToSpellCritBase.dbc");
    LoadDBC(bar,bad_dbc_files,sGtChanceToSpellCritStore, dataPath+"dbc/gtChanceToSpellCrit.dbc");

    LoadDBC(bar,bad_dbc_files,sGtOCTRegenHPStore,        dataPath+"dbc/gtOCTRegenHP.dbc");
    LoadDBC(bar,bad_dbc_files,sGtOCTRegenMPStore,        dataPath+"dbc/gtOCTRegenMP.dbc");
    LoadDBC(bar,bad_dbc_files,sGtRegenHPPerSptStore,     dataPath+"dbc/gtRegenHPPerSpt.dbc");
    LoadDBC(bar,bad_dbc_files,sGtRegenMPPerSptStore,     dataPath+"dbc/gtRegenMPPerSpt.dbc");

    //LoadDBC(bar,bad_dbc_files,sItemDisplayInfoStore,     dataPath+"dbc/ItemDisplayInfo.dbc");     -- not used currently
    LoadDBC(bar,bad_dbc_files,sItemExtendedCostStore,    dataPath+"dbc/ItemExtendedCost.dbc");
    LoadDBC(bar,bad_dbc_files,sItemRandomPropertiesStore,dataPath+"dbc/ItemRandomProperties.dbc");
    LoadDBC(bar,bad_dbc_files,sItemRandomSuffixStore,    dataPath+"dbc/ItemRandomSuffix.dbc");
    LoadDBC(bar,bad_dbc_files,sItemSetStore,             dataPath+"dbc/ItemSet.dbc");
    LoadDBC(bar,bad_dbc_files,sLockStore,                dataPath+"dbc/Lock.dbc");
    LoadDBC(bar,bad_dbc_files,sMapStore,                 dataPath+"dbc/Map.dbc");
    LoadDBC(bar,bad_dbc_files,sSkillLineStore,           dataPath+"dbc/SkillLine.dbc");
    LoadDBC(bar,bad_dbc_files,sSkillLineAbilityStore,    dataPath+"dbc/SkillLineAbility.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellStore,               dataPath+"dbc/Spell.dbc");
    for(uint32 i = 1; i < sSpellStore.nCount; ++i)
    {
        SpellEntry const * spell = sSpellStore.LookupEntry(i);
        if(spell && spell->Category)
            sSpellCategoryStore[spell->Category].insert(i);
    }

    LoadDBC(bar,bad_dbc_files,sCastTimesStore,           dataPath+"dbc/SpellCastTimes.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellDurationStore,       dataPath+"dbc/SpellDuration.dbc");
    //LoadDBC(bar,bad_dbc_files,sSpellFocusObjectStore,    dataPath+"dbc/SpellFocusObject.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellItemEnchantmentStore,dataPath+"dbc/SpellItemEnchantment.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellItemEnchantmentConditionStore,dataPath+"dbc/SpellItemEnchantmentCondition.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellRadiusStore,         dataPath+"dbc/SpellRadius.dbc");
    LoadDBC(bar,bad_dbc_files,sSpellRangeStore,          dataPath+"dbc/SpellRange.dbc");
    LoadDBC(bar,bad_dbc_files,sStableSlotPricesStore,    dataPath+"dbc/StableSlotPrices.dbc");
    LoadDBC(bar,bad_dbc_files,sTalentStore,              dataPath+"dbc/Talent.dbc");

    // create talent spells set
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; j++)
            if(talentInfo->RankID[j])
                sTalentSpellCosts[talentInfo->RankID[j]] = j+1;
    }

    LoadDBC(bar,bad_dbc_files,sTalentTabStore,           dataPath+"dbc/TalentTab.dbc");

    // preper fast data access to bit pos of talent ranks for use at inspecting
    {
        // fill table by amount of talent ranks and fill sTalentTabBitSizeInInspect
        // store in with (row,col,talent)->size key for correct sorting by (row,col)
        typedef std::map<uint32,uint32> TalentBitSize;
        TalentBitSize sTalentBitSize;
        for(uint32 i = 1; i < sTalentStore.nCount; ++i)
        {
            TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
            if (!talentInfo) continue;

            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
            if(!talentTabInfo)
                continue;

            // find talent rank
            uint32 curtalent_maxrank = 0;
            for(uint32 k = 5; k > 0; --k)
            {
                if(talentInfo->RankID[k-1])
                {
                    curtalent_maxrank = k;
                    break;
                }
            }

            sTalentBitSize[(talentInfo->Row<<24) + (talentInfo->Col<<16)+talentInfo->TalentID] = curtalent_maxrank;
            sTalentTabSizeInInspect[talentInfo->TalentTab] += curtalent_maxrank;
        }

        // now have all max ranks (and then bit amount used for store talent ranks in inspect)
        for(uint32 talentTabId = 1; talentTabId < sTalentTabStore.nCount; ++talentTabId)
        {
            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentTabId );
            if(!talentTabInfo)
                continue;

            // store class talent tab pages
            uint32 cls = 1;
            for(uint32 m=1;!(m & talentTabInfo->ClassMask) && cls < 12 /*MAX_CLASSES*/;m <<=1, ++cls);
            sTalentTabPages[cls][talentTabInfo->tabpage]=talentTabId;

            // add total amount bits for first rank starting from talent tab first talent rank pos.
            uint32 pos = 0;
            for(TalentBitSize::iterator itr = sTalentBitSize.begin(); itr != sTalentBitSize.end(); ++itr)
            {
                uint32 talentId = itr->first & 0xFFFF;
                TalentEntry const *talentInfo = sTalentStore.LookupEntry( talentId );
                if(!talentInfo)
                    continue;

                if(talentInfo->TalentTab != talentTabId)
                    continue;

                sTalentPosInInspect[talentId] = pos;
                pos+= itr->second;
            }
        }
    }

    LoadDBC(bar,bad_dbc_files,sTaxiNodesStore,           dataPath+"dbc/TaxiNodes.dbc");

    // Initialize global taxinodes mask
    memset(sTaxiNodesMask,0,sizeof(sTaxiNodesMask));
    for(uint32 i = 1; i < sTaxiNodesStore.nCount; ++i)
    {
        if(sTaxiNodesStore.LookupEntry(i))
        {
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1<<((i-1)%32);
            sTaxiNodesMask[field] |= submask;
        }
    }

    //## TaxiPath.dbc ## Loaded only for initialization different structures
    LoadDBC(bar,bad_dbc_files,sTaxiPathStore,            dataPath+"dbc/TaxiPath.dbc");
    for(uint32 i = 1; i < sTaxiPathStore.nCount; ++i)
        if(TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID,entry->price);
    uint32 pathCount = sTaxiPathStore.nCount;
    sTaxiPathStore.Clear();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadDBC(bar,bad_dbc_files,sTaxiPathNodeStore,        dataPath+"dbc/TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for(uint32 i = 1; i < sTaxiPathNodeStore.nCount; ++i)
        if(TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            ++pathLength[entry->path];
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for(uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    // fill data
    for(uint32 i = 1; i < sTaxiPathNodeStore.nCount; ++i)
        if(TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            sTaxiPathNodesByPath[entry->path][entry->index] = TaxiPathNode(entry->mapid,entry->x,entry->y,entry->z,entry->actionFlag,entry->delay);
    sTaxiPathNodeStore.Clear();

    LoadDBC(bar,bad_dbc_files,sTotemCategoryStore,       dataPath+"dbc/TotemCategory.dbc");
    LoadDBC(bar,bad_dbc_files,sWorldMapAreaStore,        dataPath+"dbc/WorldMapArea.dbc");
    LoadDBC(bar,bad_dbc_files,sWorldSafeLocsStore,       dataPath+"dbc/WorldSafeLocs.dbc");

    // error checks
    if(bad_dbc_files.size() >= DBCFilesCount )
    {
        sLog.outError("\nIncorrect DataDir value in mangosd.conf or ALL required *.dbc files (%d) not found by path: %sdbc",DBCFilesCount,dataPath.c_str());
        exit(1);
    }
    else if(!bad_dbc_files.empty() )
    {

        std::string str;
        for(std::list<std::string>::iterator i = bad_dbc_files.begin(); i != bad_dbc_files.end(); ++i)
            str += *i + "\n";

        sLog.outError("\nSome required *.dbc files (%u from %d) not found or not compatible:\n%s",bad_dbc_files.size(),DBCFilesCount,str.c_str());
        exit(1);
    }

    // check at up-to-date DBC files (45743 is last added spell in 2.3.2)
    if( !sSpellStore.LookupEntry(45743))
    {
        sLog.outError("\nYou have _outdated_ DBC files. Please extract correct versions from current using client.");
        exit(1);
    }

    sLog.outString();
    sLog.outString( ">> Loaded %d data stores", DBCFilesCount );
    sLog.outString();
}

SimpleFactionsList const* GetFactionTeamList(uint32 faction)
{
    FactionTeamMap::const_iterator itr = sFactionTeamMap.find(faction);
    if(itr==sFactionTeamMap.end())
        return NULL;
    return &itr->second;
}

char* GetPetName(uint32 petfamily, uint32 dbclang)
{
    if(!petfamily)
        return NULL;
    CreatureFamilyEntry const *pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if(!pet_family)
        return NULL;
    return pet_family->Name[dbclang]?pet_family->Name[dbclang]:NULL;
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    TalentSpellCosts::const_iterator itr = sTalentSpellCosts.find(spellId);
    if(itr==sTalentSpellCosts.end())
        return 0;

    return itr->second;
}

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id)
{
    AreaFlagByAreaID::iterator i = sAreaFlagByAreaID.find(area_id);
    if(i == sAreaFlagByAreaID.end())
        return NULL;

    return sAreaStore.LookupEntry(i->second);
}

AreaTableEntry const* GetAreaEntryByAreaFlag(uint32 area_flag)
{
    return sAreaStore.LookupEntry(area_flag);
}

uint32 GetAreaFlagByMapId(uint32 mapid)
{
    AreaFlagByMapID::iterator i = sAreaFlagByMapID.find(mapid);
    if(i == sAreaFlagByMapID.end())
        return 0;
    else
        return i->second;
}

ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id)
{
    // not sorted, numbering index from 0
    for(uint32 i = 0; i < sChatChannelsStore.nCount; ++i)
    {
        ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(i);
        if(ch && ch->ChannelID == channel_id)
            return ch;
    }
    return NULL;
}

bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId)
{
    if(requiredTotemCategoryId==0)
        return true;
    if(itemTotemCategoryId==0)
        return false;

    TotemCategoryEntry const* itemEntry = sTotemCategoryStore.LookupEntry(itemTotemCategoryId);
    if(!itemEntry)
        return false;
    TotemCategoryEntry const* reqEntry = sTotemCategoryStore.LookupEntry(requiredTotemCategoryId);
    if(!reqEntry)
        return false;

    if(itemEntry->categoryType!=reqEntry->categoryType)
        return false;

    return (itemEntry->categoryMask & reqEntry->categoryMask)==reqEntry->categoryMask;
}

void Zone2MapCoordinates(float& x,float& y,uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if(!maEntry)
        return;

    std::swap(x,y);                                         // at client map coords swapped
    x = x*((maEntry->x2-maEntry->x1)/100)+maEntry->x1;
    y = y*((maEntry->y2-maEntry->y1)/100)+maEntry->y1;      // client y coord from top to down
}

void Map2ZoneCoordinates(float& x,float& y,uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if(!maEntry)
        return;

    x = (x-maEntry->x1)/((maEntry->x2-maEntry->x1)/100);
    y = (y-maEntry->y1)/((maEntry->y2-maEntry->y1)/100);    // client y coord from top to down
    std::swap(x,y);                                         // client have map coords swapped
}

uint32 GetTalentInspectBitPosInTab(uint32 talentId)
{
    TalentInspectMap::const_iterator itr = sTalentPosInInspect.find(talentId);
    if(itr == sTalentPosInInspect.end())
        return 0;

    return itr->second;
}

uint32 GetTalentTabInspectBitSize(uint32 talentTabId)
{
    TalentInspectMap::const_iterator itr = sTalentTabSizeInInspect.find(talentTabId);
    if(itr == sTalentTabSizeInInspect.end())
        return 0;

    return itr->second;
}

uint32 const* GetTalentTabPages(uint32 cls)
{
    return sTalentTabPages[cls];
}

// script support functions
MANGOS_DLL_SPEC DBCStorage <SpellEntry>      const* GetSpellStore()      { return &sSpellStore;      }
MANGOS_DLL_SPEC DBCStorage <SpellRangeEntry> const* GetSpellRangeStore() { return &sSpellRangeStore; }
