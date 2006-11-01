/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"

#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "UpdateMask.h"
#include "World.h"
#include "WorldSession.h"
#include "Group.h"
#include "Guild.h"
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

extern SQLStorage sItemStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sCreatureStorage;
extern SQLStorage sQuestsStorage;

QuestRelations sPrevQuests;
QuestRelations sExclusiveQuestGroups;

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid = 1;
    m_hiCreatureGuid = 1;
    m_hiItemGuid = 1;
    m_hiGoGuid = 1;
    m_hiDoGuid = 1;
    m_hiCorpseGuid=1;
}

ObjectMgr::~ObjectMgr()
{

    for( QuestMap::iterator i = mQuests.begin( ); i != mQuests.end( ); ++ i )
    {
        delete i->second;
    }
    mQuests.clear( );

    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    for( AreaTriggerMap::iterator i = mAreaTriggerMap.begin( ); i != mAreaTriggerMap.end( ); ++ i )
    {
        delete i->second;
    }
    mAreaTriggerMap.clear( );

    for( TeleportMap::iterator i = mTeleports.begin( ); i != mTeleports.end( ); ++ i )
    {
        delete i->second;
    }
    mTeleports.clear();
}

Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    GroupSet::const_iterator itr;
    for (itr = mGroupSet.begin(); itr != mGroupSet.end(); itr++)
    {
        if ((*itr)->GetLeaderGUID() == guid)
        {
            return *itr;
        }
    }

    return NULL;
}

Guild * ObjectMgr::GetGuildById(const uint32 GuildId) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetId() == GuildId)
        {
            return *itr;
        }
    }

    return NULL;
}

Guild * ObjectMgr::GetGuildByName(std::string guildname) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetName() == guildname)
        {
            return *itr;
        }
    }

    return NULL;
}

CreatureInfo const* ObjectMgr::GetCreatureTemplate(uint32 id)
{
    return sCreatureStorage.LookupEntry<CreatureInfo>(id);
}

void ObjectMgr::LoadCreatureTemplates()
{

    sCreatureStorage.Load();

    sLog.outString( ">> Loaded %u creature definitions", sCreatureStorage.RecordCount );
    sLog.outString( "" );
}

PlayerCreateInfo* ObjectMgr::GetPlayerCreateInfo(uint32 race, uint32 class_)
{
    uint32 createId;

    Field *player_fields, *items_fields, *spells_fields, *skills_fields, *actions_fields;
    PlayerCreateInfo *pPlayerCreateInfo;

    QueryResult *player_result = sDatabase.PQuery("SELECT `createId`,`race`,`class`,`map`,`zone`,`position_x`,`position_y`,`position_z`,`displayID`,`BaseStrength`,`BaseAgility`,`BaseStamina`,`BaseIntellect`,`BaseSpirit`,`BaseArmor`,`BaseHealth`,`BaseMana`,`BaseRage`,`BaseFocus`,`BaseEnergy` FROM `playercreateinfo` WHERE `race` = '%u' AND `class` = '%u'", race, class_);

    if(!player_result)
    {
        sLog.outError("Warning: Can't get info for player creating with race %u and class %u (table 'playercreateinfo' is empty?)", race, class_);
        return NULL;
    }

    pPlayerCreateInfo = new PlayerCreateInfo;

    player_fields = player_result->Fetch();

    pPlayerCreateInfo->createId = player_fields[0].GetUInt8();
    createId = (uint32)pPlayerCreateInfo->createId;
    pPlayerCreateInfo->race = player_fields[1].GetUInt8();
    pPlayerCreateInfo->class_ = player_fields[2].GetUInt8();
    pPlayerCreateInfo->mapId = player_fields[3].GetUInt32();
    pPlayerCreateInfo->zoneId = player_fields[4].GetUInt32();
    pPlayerCreateInfo->positionX = player_fields[5].GetFloat();
    pPlayerCreateInfo->positionY = player_fields[6].GetFloat();
    pPlayerCreateInfo->positionZ = player_fields[7].GetFloat();
    pPlayerCreateInfo->displayId = player_fields[8].GetUInt16();
    pPlayerCreateInfo->strength = player_fields[9].GetUInt8();
    pPlayerCreateInfo->agility = player_fields[10].GetUInt8();
    pPlayerCreateInfo->stamina = player_fields[11].GetUInt8();
    pPlayerCreateInfo->intellect = player_fields[12].GetUInt8();
    pPlayerCreateInfo->spirit = player_fields[13].GetUInt8();
    pPlayerCreateInfo->basearmor = player_fields[14].GetUInt32();
    pPlayerCreateInfo->health = player_fields[15].GetUInt32();
    pPlayerCreateInfo->mana = player_fields[16].GetUInt32();
    pPlayerCreateInfo->rage = player_fields[17].GetUInt32();
    pPlayerCreateInfo->focus = player_fields[18].GetUInt32();
    pPlayerCreateInfo->energy = player_fields[19].GetUInt32();

    delete player_result;

    QueryResult *items_result = sDatabase.PQuery("SELECT `itemid`,`bagIndex`,`slot`,`amount` FROM `playercreateinfo_item` WHERE `createid` = '0' OR `createid` = '%u'", createId);

    do
    {
        if(!items_result) break;
        items_fields = items_result->Fetch();
        pPlayerCreateInfo->item_id.push_back(items_fields[0].GetUInt32());
        pPlayerCreateInfo->item_bagIndex.push_back(items_fields[1].GetUInt32());
        pPlayerCreateInfo->item_slot.push_back(items_fields[2].GetUInt8());
        pPlayerCreateInfo->item_amount.push_back(items_fields[3].GetUInt32());
    } while (items_result->NextRow());

    delete items_result;

    QueryResult *spells_result = sDatabase.PQuery("SELECT `Spell`,`Active` FROM `playercreateinfo_spell` WHERE `createid` = '0' OR `createid` = '%u'", createId);

    do
    {
        if(!spells_result) break;
        spells_fields = spells_result->Fetch();
        pPlayerCreateInfo->spell.push_back(CreateSpellPair(spells_fields[0].GetUInt16(), spells_fields[1].GetBool()));

    } while( spells_result->NextRow() );

    delete spells_result;

    QueryResult *skills_result = sDatabase.PQuery("SELECT `Skill`, `SkillMin`, `SkillMax` FROM `playercreateinfo_skill` WHERE `createid` = '0' OR `createid` = '%u'", createId);

    do
    {
        if(!skills_result) break;
        skills_fields = skills_result->Fetch();
        pPlayerCreateInfo->skill[0].push_back(skills_fields[0].GetUInt16());
        pPlayerCreateInfo->skill[1].push_back(skills_fields[1].GetUInt16());
        pPlayerCreateInfo->skill[2].push_back(skills_fields[2].GetUInt16());

    } while( skills_result->NextRow() );

    delete skills_result;

    QueryResult *actions_result = sDatabase.PQuery("SELECT `button`, `action`, `type`, `misc` FROM `playercreateinfo_action` WHERE `createid` = '0' OR `createid` = '%u'", createId);

    do
    {
        if(!actions_result) break;
        actions_fields = actions_result->Fetch();
        pPlayerCreateInfo->action[0].push_back(actions_fields[0].GetUInt16());
        pPlayerCreateInfo->action[1].push_back(actions_fields[1].GetUInt16());
        pPlayerCreateInfo->action[2].push_back(actions_fields[2].GetUInt16());
        pPlayerCreateInfo->action[3].push_back(actions_fields[3].GetUInt16());

    } while( actions_result->NextRow() );

    delete actions_result;

    return pPlayerCreateInfo;
}

uint64 ObjectMgr::GetPlayerGUIDByName(const char *name) const
{

    uint64 guid = 0;

    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name);

    if(result)
    {
        guid = (*result)[0].GetUInt32();

        delete result;
    }

    return guid;
}

bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{

    QueryResult *result = sDatabase.PQuery("SELECT `name` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

    if(result)
    {
        name = (*result)[0].GetCppString();
        delete result;
        return true;
    }

    return false;
}

uint32 ObjectMgr::GetPlayerTeamByGUID(const uint64 &guid) const
{
    QueryResult *result = sDatabase.PQuery("SELECT `race` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

    if(result)
    {
        uint8 race = (*result)[0].GetUInt8();
        delete result;
        return Player::TeamForRace(race);
    }

    return 0;
}

void ObjectMgr::LoadAuctions()
{

    QueryResult *result = sDatabase.Query("SELECT COUNT(*) FROM `auctionhouse`");

    Field *fields = result->Fetch();
    uint32 AuctionCount=fields[0].GetUInt32();
    delete result;

    if(!AuctionCount)
        return;

    result = sDatabase.Query( "SELECT `id`,`auctioneerguid`,`itemguid`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`location` FROM `auctionhouse`" );

    if( !result )
        return;

    barGoLink bar( AuctionCount );

    AuctionEntry *aItem;

    do
    {
        fields = result->Fetch();

        bar.step();

        aItem = new AuctionEntry;
        aItem->Id = fields[0].GetUInt32();
        aItem->auctioneer = fields[1].GetUInt32();
        aItem->item = fields[2].GetUInt32();
        aItem->owner = fields[3].GetUInt32();
        aItem->buyout = fields[4].GetUInt32();
        aItem->time = fields[5].GetUInt32();
        aItem->bidder = fields[6].GetUInt32();
        aItem->bid = fields[7].GetUInt32();
        aItem->location = fields[8].GetUInt8();

        AddAuction(aItem);
    } while (result->NextRow());
    delete result;

    sLog.outString("");
    sLog.outString( ">> Loaded %u auction items", AuctionCount);
    sLog.outString("");
}

void ObjectMgr::LoadItemPrototypes()
{
    sItemStorage.Load ();
    sLog.outString( ">> Loaded %u item prototypes", sItemStorage.RecordCount);
    sLog.outString("");
}

void ObjectMgr::LoadAuctionItems()
{
    QueryResult *result = sDatabase.Query( "SELECT `itemguid` FROM `auctionhouse`" );

    if( !result )
        return;
    Field *fields;
    do
    {
        fields = result->Fetch();
        Item* item = new Item;
        if(!item->LoadFromDB(fields[0].GetUInt32(),0))
        {
            delete item;
            continue;
        }
        AddAItem(item);
    }
    while( result->NextRow() );

    delete result;
}

void ObjectMgr::LoadLvlUpGains()
{
    Field *fields;
    uint32 count = 0;

    QueryResult *result  = sDatabase.PQuery("SELECT `race`,`class`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi` FROM `player_levelupgains`");
    if (!result)
    {
        barGoLink bar( 1 );

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u levelup definitions", count );
        sLog.outError( "Error loading player_levelupgains table or table empty.");
        exit(1);
    }

    barGoLink bar( result->GetRowCount() );

    // allocate dynamic array
    levelUpStatGains = new uint8***[MAX_CLASSES];
    for (uint8 i1 = 0; i1 < MAX_CLASSES; i1++)
    {
        levelUpStatGains[i1] = new uint8**[MAX_RACES];
        for (uint8 i2 = 0; i2 < MAX_RACES; i2++)
        {
            levelUpStatGains[i1][i2] = new uint8*[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];
            for (uint8 i3 = 0; i3 < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); i3++)
            {
                levelUpStatGains[i1][i2][i3] = new uint8[MAX_STATS+2];
                for (uint8 i4 = 0; i4 < MAX_STATS+2; i4++)
                    levelUpStatGains[i1][i2][i3][i4] = 0;
            }
        }
    }

    do
    {
        fields = result->Fetch();

        uint8 current_class = fields[1].GetUInt8();
        if(current_class >= MAX_CLASSES)
        {
            sLog.outError("Wrong class %u in `player_levelupgains` table, ignoring.",current_class);
            continue;
        }

        uint8 current_race = fields[0].GetUInt8();
        if(current_race >= MAX_RACES)
        {
            sLog.outError("Wrong race %u in `player_levelupgains` table, ignoring.",current_race);
            continue;
        }

        uint32 current_level = fields[2].GetUInt32();
        if(current_level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            sLog.outError("Wrong level %u in `player_levelupgains` table, ignoring.",current_level);
            continue;
        }

        for (int i = 0; i < MAX_STATS+2; i++)
        {
            levelUpStatGains[current_class][current_race][current_level][i] = fields[i+3].GetUInt8();
        }

        bar.step();
        count++;
    }
    while (result->NextRow());

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u levelup definitions", count );
}

void ObjectMgr::LoadGuilds()
{
    Guild *newguild;
    uint32 count = 0;

    QueryResult *result = sDatabase.Query( "SELECT `guildid` FROM `guild`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString( "" );
        sLog.outString( ">> Loaded %u guild definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();
        count++;

        newguild = new Guild;
        newguild->LoadGuildFromDB(fields[0].GetUInt32());
        AddGuild(newguild);

    }while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u guild definitions", count );
}

void ObjectMgr::LoadQuests()
{
    sQuestsStorage.Load ();

    // create multimap previous quest for each existed quest
    // some quests can have many previous maps setted by NextQuestId in previouse quest
    // for example set of race quests can lead to single not race specific quest
    for(uint32 i = 1; i < sQuestsStorage.MaxEntry; ++i )
    {
        QuestInfo const* qinfo = GetQuestInfo(i);

        if(!qinfo)
            continue;

        if(qinfo->PrevQuestId )
            sPrevQuests.insert(QuestRelations::value_type(qinfo->QuestId,qinfo->PrevQuestId));

        if(qinfo->NextQuestId )
            sPrevQuests.insert(QuestRelations::value_type(qinfo->NextQuestId,qinfo->QuestId));

        if(qinfo->ExclusiveGroup)
            sExclusiveQuestGroups.insert(QuestRelations::value_type(qinfo->ExclusiveGroup,qinfo->QuestId));
    };

    sLog.outString( ">> Loaded %u quests definitions", sQuestsStorage.RecordCount );
    sLog.outString( "" );
}

void ObjectMgr::AddGossipText(GossipText *pGText)
{
    ASSERT( pGText->Text_ID );
    ASSERT( mGossipText.find(pGText->Text_ID) == mGossipText.end() );
    mGossipText[pGText->Text_ID] = pGText;
}

GossipText *ObjectMgr::GetGossipText(uint32 Text_ID)
{
    GossipTextMap::const_iterator itr;
    for (itr = mGossipText.begin(); itr != mGossipText.end(); itr++)
    {
        if(itr->second->Text_ID == Text_ID)
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::LoadGossipText()
{
    GossipText *pGText;
    QueryResult *result = sDatabase.Query( "SELECT * FROM `npc_text`" );

    int count = 0;
    if( !result ) return;
    int cic;

    barGoLink bar( result->GetRowCount() );

    do
    {
        count++;
        cic = 0;

        Field *fields = result->Fetch();

        bar.step();

        pGText = new GossipText;
        pGText->Text_ID    = fields[cic++].GetUInt32();

        for (int i=0; i< 8; i++)
        {
            pGText->Options[i].Text_0           = fields[cic++].GetCppString();
            pGText->Options[i].Text_1           = fields[cic++].GetCppString();

            pGText->Options[i].Language         = fields[cic++].GetUInt32();
            pGText->Options[i].Probability      = fields[cic++].GetFloat();

            pGText->Options[i].Emotes[0]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[0]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[1]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[1]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[2]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[2]._Emote  = fields[cic++].GetUInt32();
        }

        if ( !pGText->Text_ID ) continue;
        AddGossipText( pGText );

    } while( result->NextRow() );

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u npc texts", count );
    delete result;
}

ItemPage *ObjectMgr::RetreiveItemPageText(uint32 Page_ID)
{
    ItemPage *pIText;
    QueryResult *result = sDatabase.PQuery("SELECT `id`,`text`,`next_page` FROM `item_page` WHERE `id` = '%u'", Page_ID);

    if( !result ) return NULL;
    int cic, count = 0;
    pIText = new ItemPage;

    do
    {
        count++;
        cic = 0;

        Field *fields = result->Fetch();

        pIText->Page_ID    = fields[cic++].GetUInt32();

        pIText->PageText   = fields[cic++].GetCppString();
        pIText->Next_Page  = fields[cic++].GetUInt32();

        if ( !pIText->Page_ID ) break;
    } while( result->NextRow() );

    delete result;
    return pIText;
}

void ObjectMgr::AddAreaTriggerPoint(AreaTriggerPoint *pArea)
{
    ASSERT( pArea->Trigger_ID );
    ASSERT( mAreaTriggerMap.find(pArea->Trigger_ID) == mAreaTriggerMap.end() );

    mAreaTriggerMap[pArea->Trigger_ID] = pArea;
}

AreaTriggerPoint *ObjectMgr::GetAreaTriggerQuestPoint(uint32 Trigger_ID)
{
    AreaTriggerMap::const_iterator itr;
    for (itr = mAreaTriggerMap.begin(); itr != mAreaTriggerMap.end(); itr++)
    {
        if(itr->second->Trigger_ID == Trigger_ID)
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::LoadAreaTriggerPoints()
{
    int count = 0;
    QueryResult *result = sDatabase.Query( "SELECT `id`,`quest` FROM `areatrigger_involvedrelation`" );
    AreaTriggerPoint *pArea;

    if( !result ) return;

    barGoLink bar( result->GetRowCount() );

    do
    {
        count++;

        bar.step();

        pArea = new AreaTriggerPoint;

        Field *fields = result->Fetch();

        pArea->Trigger_ID      = fields[0].GetUInt32();
        pArea->Quest_ID        = fields[1].GetUInt32();

        AddAreaTriggerPoint( pArea );

    } while( result->NextRow() );

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u quest trigger points", count );
    delete result;
}

bool ObjectMgr::GetGlobalTaxiNodeMask( uint32 curloc, uint32 *Mask )
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(curloc);

    if(src_i==sTaxiPathSetBySource.end())
        return false;

    TaxiPathSetForSource& pathSet = src_i->second;

    for(TaxiPathSetForSource::iterator  path_i = pathSet.begin(); path_i != pathSet.end(); ++path_i)
    {
        uint8 destination = path_i->first;
        uint8 field = (uint8)((destination - 1) / 32);
        Mask[field] |= 1 << ( (destination - 1 ) % 32 );
    }

    return true;
}

uint32 ObjectMgr::GetNearestTaxiNode( float x, float y, float z, uint32 mapid )
{

    bool found = false;
    float dist;
    uint32 id = 0;
    for(uint32 i = 1; i <= sTaxiNodesStore.nCount; ++i)
    {
        TaxiNodesEntry* node = sTaxiNodesStore.LookupEntry(i);
        if(node && node->map_id == mapid)
        {
            float dist2 = (node->x - x)*(node->x - x)+(node->y - y)*(node->y - y)+(node->z - z)*(node->z - z);
            if(found)
            {
                if(dist2 < dist)
                {
                    dist = dist2;
                    id = i;
                }
            }
            else
            {
                found = true;
                dist = dist2;
                id = i;
            }
        }
    }

    return id;
}

void ObjectMgr::GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{

    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(source);
    if(src_i==sTaxiPathSetBySource.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    TaxiPathSetForSource& pathSet = src_i->second;

    TaxiPathSetForSource::iterator dest_i = pathSet.find(destination);
    if(dest_i==pathSet.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint16 ObjectMgr::GetTaxiMount( uint32 id, uint32 team  )
{
    uint16 mount_id = 0;

    TaxiNodesEntry* node = sTaxiNodesStore.LookupEntry(id);
    if(node)
    {
        if (team == ALLIANCE)
            switch(node->alliance_mount_type)
            {
                case 541:
                    mount_id = 1147;                        // alliance
                    break;
                case 3837:
                    mount_id = 479;                         // nightelf
                    break;
                //case 17760:
                //  unknown outer bg mount?
            }
            else if (team == HORDE)
                switch(node->horde_mount_type)
                {
                    case 2224:
                        mount_id = 295;                     // horde
                        break;
                    case 3574:
                        mount_id = 1566;                    // undead
                        break;
                //case 17760:
                //  unknown outer bg mount?
                }

    }

    return mount_id;
}

void ObjectMgr::GetTaxiPathNodes( uint32 path, Path &pathnodes )
{
    if(path >= sTaxiPathNodesByPath.size())
        return;

    TaxiPathNodeList& nodeList = sTaxiPathNodesByPath[path];

    pathnodes.Resize(nodeList.size());

    unsigned int i = 0;
    for(int i = 0; i < nodeList.size(); ++i)
    {
        pathnodes[ i ].x = nodeList[i].x;
        pathnodes[ i ].y = nodeList[i].y;
        pathnodes[ i ].z = nodeList[i].z;
    }
}

WorldSafeLocsEntry *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team)
{

    // search for zone associated closest graveyard
    uint32 zoneId = MapManager::Instance().GetMap(MapId)->GetZoneId(x,y);

    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone,ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check `faction`
    //   if mapId != graveyard.mapId (ghost in instance) and search ANY graveyard associated
    //     then skip check `faction`
    QueryResult *result = sDatabase.PQuery("SELECT `id`,`faction` FROM `game_graveyard_zone` WHERE  `ghost_map` = %u AND `ghost_zone` = %u", MapId, zoneId);

    if(! result)
    {
        sLog.outError("DB incomplite: Zone %u Map %u Team %u not have linked graveyard.",zoneId,MapId,team);
        return NULL;
    }

    bool foundNear = false;
    float distNear;
    WorldSafeLocsEntry* entryNear = NULL;
    WorldSafeLocsEntry* entryFar = NULL;

    do
    {
        Field *fields = result->Fetch();
        uint32 g_id   = fields[0].GetUInt32();
        uint32 g_team = fields[1].GetUInt32();

        WorldSafeLocsEntry* entry = sWorldSafeLocsStore.LookupEntry(g_id);
        if(!entry)
        {
            sLog.outError("Table `game_graveyard_zone` have record for not existed graveyard (WorldSafeLocs.dbc id) %u, skipped.",g_id);
            continue;
        }

        // remember first graveyard at another map and ignore other
        if(MapId != entry->map_id)
        {
            if(!entryFar)
                entryFar = entry;
            continue;
        }

        // skip enimy faction graveyard at same map (normal area, city, or battleground)
        if(g_team != 0 && g_team != team)
            continue;

        // find now nearest graveyrd at same map
        float dist2 = (entry->x - x)*(entry->x - x)+(entry->y - y)*(entry->y - y)+(entry->z - z)*(entry->z - z);
        if(foundNear)
        {
            if(dist2 < distNear)
            {
                distNear = dist2;
                entryNear = entry;
            }
        }
        else
        {
            foundNear = true;
            distNear = dist2;
            entryNear = entry;
        }
    } while( result->NextRow() );

    delete result;

    if(entryNear)
        return entryNear;

    return entryFar;
}

AreaTrigger *ObjectMgr::GetAreaTrigger(uint32 Trigger_ID)
{
    if( !Trigger_ID )
        return NULL;
    QueryResult *result = sDatabase.PQuery("SELECT `target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `areatrigger_template` WHERE `id` = '%u'", Trigger_ID);
    if ( !result )
        return NULL;
    Field *fields = result->Fetch();
    AreaTrigger *at = new AreaTrigger;

    at->mapId = fields[0].GetUInt32();
    at->X = fields[1].GetFloat();
    at->Y = fields[2].GetFloat();
    at->Z = fields[3].GetFloat();
    at->Orientation = fields[4].GetFloat();
    if(at->X==0&&at->Y==0&&at->Z==0)return NULL;

    delete result;
    return at;
}

void ObjectMgr::LoadTeleportCoords()
{

    QueryResult *result = sDatabase.Query( "SELECT `id`,`target_position_x`,`target_position_y`,`target_position_z`,`target_map` FROM `areatrigger_template`" );

    if( !result )
        return;

    uint32 count = 0;

    TeleportCoords *pTC;

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        pTC = new TeleportCoords;
        pTC->id = fields[0].GetUInt32();
        //pTC->Name = fields[6].GetString();
        pTC->mapId = fields[4].GetUInt32();
        pTC->x = fields[1].GetFloat();
        pTC->y = fields[2].GetFloat();
        pTC->z = fields[3].GetFloat();

        AddTeleportCoords(pTC);

    } while( result->NextRow() );

    delete result;

    sLog.outString( "" );
    sLog.outString( ">> Loaded %u teleport definitions", count );
}

void ObjectMgr::SetHighestGuids()
{

    QueryResult *result = sDatabase.Query( "SELECT MAX(`guid`) FROM `character`" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `creature`" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `item_instance`" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query("SELECT MAX(`guid`) FROM `gameobject`" );
    if( result )
    {
        m_hiGoGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = sDatabase.Query("SELECT MAX(`id`) FROM `auctionhouse`" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_auctionid = 0;
    }
    result = sDatabase.Query( "SELECT MAX(`id`) FROM `mail`" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_mailid = 0;
    }

    result = sDatabase.Query( "SELECT MAX(`guid`) FROM `corpse`" );
    if( result )
    {
        m_hiCorpseGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

}

uint32 ObjectMgr::GenerateAuctionID()
{

    return ++m_auctionid;
}

uint32 ObjectMgr::GenerateMailID()
{
    return ++m_mailid;
}

uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{

    switch(guidhigh)
    {
        case HIGHGUID_ITEM          : return ++m_hiItemGuid;
        case HIGHGUID_UNIT          : return ++m_hiCreatureGuid;
        case HIGHGUID_PLAYER        : return ++m_hiCharGuid;
        case HIGHGUID_GAMEOBJECT    : return ++m_hiGoGuid;
        case HIGHGUID_CORPSE        : return ++m_hiCorpseGuid;
        case HIGHGUID_DYNAMICOBJECT : return ++m_hiDoGuid;
        default                     : ASSERT(0);
    }

    return 0;
}

void ObjectMgr::LoadGameobjectInfo()
{
    sGOStorage.Load();

    sLog.outString( ">> Loaded %u game object templates", sGOStorage.RecordCount );

}
