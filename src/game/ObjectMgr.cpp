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
#include "ArenaTeam.h"
#include "Transports.h"
#include "ProgressBar.h"
#include "Policies/SingletonImp.h"
#include "Language.h"
#include "GameEvent.h"
#include "Spell.h"
#include "Chat.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

ScriptMapMap sQuestEndScripts;
ScriptMapMap sQuestStartScripts;
ScriptMapMap sSpellScripts;
ScriptMapMap sButtonScripts;

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid        = 1;
    m_hiCreatureGuid    = 1;
    m_hiItemGuid        = 1;
    m_hiGoGuid          = 1;
    m_hiDoGuid          = 1;
    m_hiCorpseGuid      = 1;

    m_hiPetNumber       = 1;

    playerInfo          = NULL;
}

ObjectMgr::~ObjectMgr()
{

    for( QuestMap::iterator i = mQuestTemplates.begin( ); i != mQuestTemplates.end( ); ++ i )
    {
        delete i->second;
    }
    mQuestTemplates.clear( );

    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++ i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    mAreaTriggers.clear();

    for(PetLevelInfoMap::iterator i = petInfo.begin( ); i != petInfo.end( ); ++ i )
    {
        delete[] i->second;
    }
    petInfo.clear();

    // free only if loaded
    if(playerInfo)
    {
        for (int race = 0; race < MAX_RACES; ++race)
        {
            for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
            {
                delete[] playerInfo[race][class_].levelInfo;
            }
            delete[] playerInfo[race];
        }
        delete[] playerInfo;
    }

    // free group and guild objects
    for (GroupSet::iterator itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
        delete (*itr);
    for (GuildSet::iterator itr = mGuildSet.begin(); itr != mGuildSet.end(); ++itr)
        delete (*itr);

    for(ItemMap::iterator itr = mAitems.begin(); itr != mAitems.end(); ++itr)
        delete itr->second;
}

Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    GroupSet::const_iterator itr;
    for (itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
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

std::string ObjectMgr::GetGuildNameById(const uint32 GuildId) const
{
    GuildSet::const_iterator itr;
    for (itr = mGuildSet.begin(); itr != mGuildSet.end(); itr++)
    {
        if ((*itr)->GetId() == GuildId)
        {
            return (*itr)->GetName();
        }
    }

    return "";
}

ArenaTeam * ObjectMgr::GetArenaTeamById(const uint32 ArenaTeamId) const
{
    ArenaTeamSet::const_iterator itr;
    for (itr = mArenaTeamSet.begin(); itr != mArenaTeamSet.end(); itr++)
    {
        if ((*itr)->GetId() == ArenaTeamId)
        {
            return *itr;
        }
    }

    return NULL;
}

ArenaTeam * ObjectMgr::GetArenaTeamByName(std::string arenateamname) const
{
    ArenaTeamSet::const_iterator itr;
    for (itr = mArenaTeamSet.begin(); itr != mArenaTeamSet.end(); itr++)
    {
        if ((*itr)->GetName() == arenateamname)
        {
            return *itr;
        }
    }

    return NULL;
}

AuctionHouseObject * ObjectMgr::GetAuctionsMap( uint32 location )
{
    switch ( location )
    {
        case 6:                                             //horde
            return & mHordeAuctions;
            break;
        case 2:                                             //alliance
            return & mAllianceAuctions;
            break;
        default:                                            //neutral
            return & mNeutralAuctions;
    }
}

uint32 ObjectMgr::GetAuctionCut(uint32 location, uint32 highBid)
{
    if (location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        return (uint32) (0.15f * highBid * sWorld.getRate(RATE_AUCTION_CUT));
    else
        return (uint32) (0.05f * highBid * sWorld.getRate(RATE_AUCTION_CUT));
}

uint32 ObjectMgr::GetAuctionDeposit(uint32 location, uint32 time, Item *pItem)
{
    uint32 percentance;
    if ( location == 7 && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_TRADE))
        percentance = 25;
    else
        percentance = 5;
    return (uint32) ( ((percentance * sWorld.getRate(RATE_AUCTION_DEPOSIT) * pItem->GetProto()->SellPrice * pItem->GetCount() ) / 100 ) * (time / 120 ) );
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 ObjectMgr::GetAuctionOutBid(uint32 currentBid)
{
    uint32 outbid = (currentBid / 100) * 5;
    if (!outbid)
        outbid = 1;
    return outbid;
}

//does not clear ram
void ObjectMgr::SendAuctionWonMail( AuctionEntry *auction )
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    uint64 bidder_guid = MAKE_GUID(auction->bidder,HIGHGUID_PLAYER);
    Player *bidder = objmgr.GetPlayer(bidder_guid);

    uint32 bidder_accId = 0;

    // data for gm.log
    if( sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
    {
        uint32 bidder_security = 0;
        std::string bidder_name;
        if (bidder)
        {
            bidder_accId = bidder->GetSession()->GetAccountId();
            bidder_security = bidder->GetSession()->GetSecurity();
            bidder_name = bidder->GetName();
        }
        else
        {
            bidder_accId = GetPlayerAccountIdByGUID(bidder_guid);
            bidder_security = GetSecurityByAccount(bidder_accId);

            if(bidder_security > SEC_PLAYER )               // not do redundant DB requests
            {
                if(!GetPlayerNameByGUID(bidder_guid,bidder_name))
                    bidder_name = LANG_UNKNOWN;
            }
        }

        if( bidder_security > SEC_PLAYER )
        {
            std::string owner_name;
            if(!GetPlayerNameByGUID(auction->owner,owner_name))
                owner_name = LANG_UNKNOWN;

            uint32 owner_accid = GetPlayerAccountIdByGUID(auction->owner);

            sLog.outCommand("GM %s (Account: %u) won item in auction: %s (Entry: %u Count: %u) and pay money: %u. Original owner %s (Account: %u)",
                bidder_name.c_str(),bidder_accId,pItem->GetProto()->Name1,pItem->GetEntry(),pItem->GetCount(),auction->bid,owner_name.c_str(),owner_accid);
        }
    }
    else if(!bidder)
        bidder_accId = GetPlayerAccountIdByGUID(bidder_guid);

    // receiver exist
    if(bidder || bidder_accId)
    {
        std::ostringstream msgAuctionWonSubject;
        msgAuctionWonSubject << auction->item_template << ":0:" << AUCTION_WON;

        std::ostringstream msgAuctionWonBody;
        msgAuctionWonBody.width(16);
        msgAuctionWonBody << std::right << std::hex << auction->owner;
        msgAuctionWonBody << std::dec << ":" << auction->bid << ":" << auction->buyout;
        sLog.outDebug( "AuctionWon body string : %s", msgAuctionWonBody.str().c_str() );

        //prepare mail data... :
        uint32 mailId = this->GenerateMailID();
        uint32 itemTextId = this->CreateItemText( msgAuctionWonBody.str() );
        time_t dtime = time(NULL);                          //Will always be instant when from Auction House.
        time_t etime = dtime + (30 * DAY);

        // set owner to bidder (to prevent delete item with sender char deleting)
        // owner in `data` will set at mail receive and item extracting
        CharacterDatabase.PExecute("UPDATE `item_instance` SET `owner_guid` = '%u' WHERE `guid`='%u'",auction->bidder,pItem->GetGUIDLow());
        CharacterDatabase.CommitTransaction();

        if (bidder)
        {
            bidder->GetSession()->SendAuctionBidderNotification( auction->location, auction->Id, bidder_guid, 0, 0, auction->item_template);

            MailItemsInfo mi;
            mi.AddItem(auction->item_guid, auction->item_template, pItem);
            bidder->CreateMail(mailId, MAIL_AUCTION, auction->location, msgAuctionWonSubject.str(), itemTextId, &mi, etime, dtime, 0, 0, AUCTION_CHECKED);
        }
        else
        {
            objmgr.RemoveAItem(pItem->GetGUIDLow()); // we have to remove the item, before we delete it !!
            delete pItem;
        }

        CharacterDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`has_items`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
            "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '1', '" I64FMTD "','" I64FMTD "', '0', '0', '%d')",
            mailId, MAIL_AUCTION, auction->location, auction->bidder, msgAuctionWonSubject.str().c_str(), itemTextId, (uint64)etime, (uint64)dtime, AUCTION_CHECKED);
        CharacterDatabase.PExecute("INSERT INTO `mail_items` (`mail_id`,`item_guid`,`item_template`) VALUES ('%u', '%u', '%u')", mailId, auction->item_guid, auction->item_template);
    }
    // receiver not exist
    else
    {
        CharacterDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid`='%u'", pItem->GetGUIDLow());
        objmgr.RemoveAItem(pItem->GetGUIDLow()); // we have to remove the item, before we delete it !!
        delete pItem;
    }
}

//call this method to send mail to auction owner, when auction is successful, it does not clear ram
void ObjectMgr::SendAuctionSuccessfulMail( AuctionEntry * auction )
{
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
        return;

    uint64 owner_guid = MAKE_GUID(auction->owner,HIGHGUID_PLAYER);
    Player *owner = objmgr.GetPlayer(owner_guid);

    uint32 owner_accId = 0;
    if(!owner)
        owner_accId = GetPlayerAccountIdByGUID(owner_guid);

    // owner exist
    if(owner || owner_accId)
    {
        std::ostringstream msgAuctionSuccessfulSubject;
        msgAuctionSuccessfulSubject << auction->item_template << ":0:" << AUCTION_SUCCESSFUL;

        std::ostringstream auctionSuccessfulBody;
        uint32 auctionCut = this->GetAuctionCut(auction->location, auction->bid);

        auctionSuccessfulBody.width(16);
        auctionSuccessfulBody << std::right << std::hex << auction->bidder;
        auctionSuccessfulBody << std::dec << ":" << auction->bid << ":0:";
        auctionSuccessfulBody << auction->deposit << ":" << auctionCut;
        sLog.outDebug("AuctionSuccessful body string : %s", auctionSuccessfulBody.str().c_str());

        uint32 itemTextId = this->CreateItemText( auctionSuccessfulBody.str() );

        uint32 mailId = this->GenerateMailID();
        time_t dtime = time(NULL);                          //Instant because it's Auction House
        time_t etime = dtime + (30 * DAY);
        uint32 profit = auction->bid + auction->deposit - auctionCut;

        if (owner)
        {
            //send auction owner notification, bidder must be current!
            owner->GetSession()->SendAuctionOwnerNotification( auction );

            MailItemsInfo mi;
            owner->CreateMail(mailId, MAIL_AUCTION, auction->location, msgAuctionSuccessfulSubject.str(), itemTextId, &mi, etime, dtime, profit, 0, AUCTION_CHECKED);
        }

        CharacterDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`has_items`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
            "VALUES ('%u', '%d', '%u', '%u', '%s', '%u', '0', '" I64FMTD "', '" I64FMTD "', '%u', '0', '%d')",
            mailId, MAIL_AUCTION, auction->location, auction->owner, msgAuctionSuccessfulSubject.str().c_str(), itemTextId, (uint64)etime, (uint64)dtime, profit, AUCTION_CHECKED);
    }
}

//does not clear ram
void ObjectMgr::SendAuctionExpiredMail( AuctionEntry * auction )
{                                                           //return an item in auction to its owner by mail
    Item *pItem = objmgr.GetAItem(auction->item_guid);
    if(!pItem)
    {
        sLog.outError("Auction item (GUID: %u) not found, and lost.",auction->item_guid);
        return;
    }

    uint64 owner_guid = MAKE_GUID(auction->owner,HIGHGUID_PLAYER);
    Player *owner = objmgr.GetPlayer(owner_guid);

    uint32 owner_accId = 0;
    if(!owner)
        owner_accId = GetPlayerAccountIdByGUID(owner_guid);

    // owner exist
    if(owner || owner_accId)
    {
        uint32 messageId = objmgr.GenerateMailID();
        std::ostringstream subject;
        subject << auction->item_template << ":0:" << AUCTION_EXPIRED;
        time_t dtime = time(NULL);                          //Instant since it's Auction House
        time_t etime = dtime + 30 * DAY;

        CharacterDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`has_items`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
            "VALUES ('%u', '2', '%u', '%u', '%s', '0', '1', '" I64FMTD "','" I64FMTD "', '0', '0', '%d')",
            messageId, auction->location, auction->owner, subject.str().c_str(), (uint64)etime, (uint64)dtime, NOT_READ);
        CharacterDatabase.PExecute("INSERT INTO `mail_items` (`mail_id`,`item_guid`,`item_template`) VALUES ('%u', '%u', '%u')", messageId, auction->item_guid, auction->item_template);
        if ( owner )
        {
            owner->GetSession()->SendAuctionOwnerNotification( auction );

            MailItemsInfo mi;
            mi.AddItem(auction->item_guid, auction->item_template, pItem);
            owner->CreateMail(messageId, MAIL_AUCTION, auction->location, subject.str(), 0, &mi, etime, dtime, 0, 0, NOT_READ);
        }
        else
        {
            objmgr.RemoveAItem(pItem->GetGUIDLow()); // we have to remove the item, before we delete it !!
            delete pItem;
        }
    }
    // owner not found
    else
    {
        CharacterDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid`='%u'",pItem->GetGUIDLow());
        objmgr.RemoveAItem(pItem->GetGUIDLow()); // we have to remove the item, before we delete it !!
        delete pItem;
    }
}

CreatureInfo const* ObjectMgr::GetCreatureTemplate(uint32 id)
{
    return sCreatureStorage.LookupEntry<CreatureInfo>(id);
}

void ObjectMgr::LoadCreatureLocales()
{
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`name_loc1`,`subname_loc1`,`name_loc2`,`subname_loc2`,`name_loc3`,`subname_loc3`,`name_loc4`,`subname_loc4`,`name_loc5`,`subname_loc5`,`name_loc6`,`subname_loc6`,`name_loc7`,`subname_loc7` FROM `locales_creature`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 creature locale strings. DB table `locales_creature` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        CreatureLocale& data = mCreatureLocaleMap[entry];

        for(int i = 1; i < 8; ++i)
        {
            std::string str = fields[1+2*(i-1)].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Name.size() <= idx)
                        data.Name.resize(idx+1);

                    data.Name[idx] = str;
                }
            }
            str = fields[1+2*(i-1)+1].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.SubName.size() <= idx)
                        data.SubName.resize(idx+1);

                    data.SubName[idx] = str;
                }
            }
        }
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u creature locale strings", mCreatureLocaleMap.size() );
}

void ObjectMgr::LoadCreatureTemplates()
{
    sCreatureStorage.Load();

    sLog.outString( ">> Loaded %u creature definitions", sCreatureStorage.RecordCount );
    sLog.outString();

    // check data correctness
    for(uint32 i = 1; i < sCreatureStorage.MaxEntry; ++i)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i);
        if(!cInfo)
            continue;

        FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction_A);
        if(!factionTemplate)
            sLog.outErrorDb("Creature (Entry: %u) has non-existing faction_A template (%u)", cInfo->Entry, cInfo->faction_A);
        factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction_H);
        if(!factionTemplate)
            sLog.outErrorDb("Creature (Entry: %u) has non-existing faction_H template (%u)", cInfo->Entry, cInfo->faction_H);

        CreatureModelInfo const* minfo = sCreatureModelStorage.LookupEntry<CreatureModelInfo>(cInfo->DisplayID_A);
        if (!minfo)
            sLog.outErrorDb("Creature (Entry: %u) has non-existing modelId_A (%u)", cInfo->Entry, cInfo->DisplayID_A);
        minfo = sCreatureModelStorage.LookupEntry<CreatureModelInfo>(cInfo->DisplayID_H);
        if (!minfo)
            sLog.outErrorDb("Creature (Entry: %u) has non-existing modelId_H (%u)", cInfo->Entry, cInfo->DisplayID_H);

        if(cInfo->dmgschool >= MAX_SPELL_SCHOOL)
        {
            sLog.outErrorDb("Creature (Entry: %u) has invalid spell school value (%u) in `dmgschool`",cInfo->Entry,cInfo->dmgschool);
            const_cast<CreatureInfo*>(cInfo)->dmgschool = SPELL_SCHOOL_NORMAL;
        }

        if((cInfo->npcflag & UNIT_NPC_FLAG_TRAINER) && cInfo->trainer_type >= MAX_TRAINER_TYPE)
            sLog.outErrorDb("Creature (Entry: %u) has wrong trainer type %u",cInfo->Entry,cInfo->trainer_type);

        if(cInfo->InhabitType <= 0 || cInfo->InhabitType > INHAVIT_ANYWHERE)
        {
            sLog.outErrorDb("Creature (Entry: %u) has wrong value (%u) in `InhabitType`, creature will not correctly walk/swim/fly",cInfo->Entry,cInfo->InhabitType);
            const_cast<CreatureInfo*>(cInfo)->InhabitType = INHAVIT_ANYWHERE;
        }

        if(cInfo->MovementType >= MAX_DB_MOTION_TYPE)
        {
            sLog.outErrorDb("Creature (Entry: %u) has wrong movement generator type (%u), ignore and set to IDLE.",cInfo->Entry,cInfo->MovementType);
            const_cast<CreatureInfo*>(cInfo)->MovementType = IDLE_MOTION_TYPE;
        }
    }
}

void ObjectMgr::ConvertCreatureAddonAuras(CreatureDataAddon* addon, char const* table, char const* guidEntryStr)
{
    // Now add the auras, format "spellid effectindex spellid effectindex..."
    char *p,*s;
    std::vector<int> val;
    s=p=(char*)reinterpret_cast<char const*>(addon->auras);
    if(!p)
    {
        while (p[0]!=0)
        {
            p++;
            if (p[0]==' ')
            {
                val.push_back(atoi(s));
                s=++p;
            }
        }
        if (p!=s)
            val.push_back(atoi(s));

        // free char* loaded memory
        delete[] (char*)reinterpret_cast<char const*>(addon->auras);

        // wrong list
        if (val.size()%2)
        {
            addon->auras = NULL;
            sLog.outErrorDb("Creature (%s: %u) has wrong `auras` data in `%s`.",guidEntryStr,addon->guidOrEntry,table);
            return;
        }
    }

    // empty list
    if(val.empty())
    {
        addon->auras = NULL;
        return;
    }

    // replace by new strucutres array
    const_cast<CreatureDataAddonAura*&>(addon->auras) = new CreatureDataAddonAura[val.size()/2+1];

    int i=0;
    for(int j=0;j<val.size()/2;++j)
    {
        CreatureDataAddonAura& cAura = const_cast<CreatureDataAddonAura&>(addon->auras[i]);
        cAura.spell_id = (uint32)val[2*j+0];
        cAura.effect_idx  = (uint32)val[2*j+1];
        if ( cAura.effect_idx > 2 )
        {
            sLog.outErrorDb("Creature (%s: %u) has wrong effect %u for spell %u in `auras` field in`%s`.",guidEntryStr,addon->guidOrEntry,cAura.effect_idx,cAura.spell_id,table);
            continue;
        }
        SpellEntry const *AdditionalSpellInfo = sSpellStore.LookupEntry(cAura.spell_id);
        if (!AdditionalSpellInfo)
        {
            sLog.outErrorDb("Creature (%s: %u) has wrong spell %u defined in `auras` field in `%s`.",guidEntryStr,addon->guidOrEntry,cAura.spell_id,table);
            continue;
        }

        if (!AdditionalSpellInfo->Effect[cAura.effect_idx] || !AdditionalSpellInfo->EffectApplyAuraName[cAura.effect_idx])
        {
            sLog.outErrorDb("Creature (%s: %u) has not aura effect %u of spell %u defined in `auras` field in `%s`.",guidEntryStr,addon->guidOrEntry,cAura.effect_idx,cAura.spell_id,table);
            continue;
        }

        ++i;
    }

    // fill terminator element (after last added)
    CreatureDataAddonAura& endAura = const_cast<CreatureDataAddonAura&>(addon->auras[i]);
    endAura.spell_id   = 0;
    endAura.effect_idx = 0;
}

void ObjectMgr::LoadCreatureAddons()
{
    sCreatureInfoAddonStorage.Load();

    sLog.outString( ">> Loaded %u creature template addons", sCreatureInfoAddonStorage.RecordCount );
    sLog.outString();

    // check data correctness and convert 'auras'
    for(uint32 i = 1; i < sCreatureInfoAddonStorage.MaxEntry; ++i)
    {
        CreatureDataAddon const* addon = sCreatureInfoAddonStorage.LookupEntry<CreatureDataAddon>(i);
        if(!addon)
            continue;

        ConvertCreatureAddonAuras(const_cast<CreatureDataAddon*>(addon), "creature_template_addon", "Entry");

        if(!sCreatureStorage.LookupEntry<CreatureInfo>(addon->guidOrEntry))
            sLog.outErrorDb("Creature (Entry: %u) does not exist but has a record in `creature_template_addon`",addon->guidOrEntry);
    }

    sCreatureDataAddonStorage.Load();

    sLog.outString( ">> Loaded %u creature addons", sCreatureDataAddonStorage.RecordCount );
    sLog.outString();

    // check data correctness and convert 'auras'
    for(uint32 i = 1; i < sCreatureDataAddonStorage.MaxEntry; ++i)
    {
        CreatureDataAddon const* addon = sCreatureDataAddonStorage.LookupEntry<CreatureDataAddon>(i);
        if(!addon)
            continue;

        ConvertCreatureAddonAuras(const_cast<CreatureDataAddon*>(addon), "creature_addon", "GUIDLow");

        if(mCreatureDataMap.find(addon->guidOrEntry)==mCreatureDataMap.end())
            sLog.outErrorDb("Creature (GUID: %u) does not exist but has a record in `creature_addon`",addon->guidOrEntry);
    }
}

EquipmentInfo const* ObjectMgr::GetEquipmentInfo(uint32 entry)
{
    return sEquipmentStorage.LookupEntry<EquipmentInfo>(entry);
}

void ObjectMgr::LoadEquipmentTemplates()
{
    sEquipmentStorage.Load();

    sLog.outString( ">> Loaded %u equipment template", sEquipmentStorage.RecordCount );
    sLog.outString();
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelInfo(uint32 modelid)
{
    return sCreatureModelStorage.LookupEntry<CreatureModelInfo>(modelid);
}

uint32 ObjectMgr::ChooseDisplayId(uint32 team, const CreatureInfo *cinfo, const CreatureData *data)
{
    // Load creature model (display id)
    uint32 display_id;
    if (!data || data->displayid == 0)                      // use defaults from the template
    {
        // DisplayID_A is used if no team is given
        if (team == HORDE)
            display_id = (cinfo->DisplayID_H2 != 0 && urand(0,1) == 0) ? cinfo->DisplayID_H2 : cinfo->DisplayID_H;
        else
            display_id = (cinfo->DisplayID_A2 != 0 && urand(0,1) == 0) ? cinfo->DisplayID_A2 : cinfo->DisplayID_A;
    }
    else                                                    // overriden in creature data
        display_id = data->displayid;

    return display_id;
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelRandomGender(uint32 display_id)
{
    CreatureModelInfo const *minfo = GetCreatureModelInfo(display_id);
    if(!minfo)
        return NULL;

    // If a model for another gender exists, 50% chance to use it
    if(minfo->modelid_other_gender != 0 && urand(0,1) == 0)
    {
        CreatureModelInfo const *minfo_tmp = GetCreatureModelInfo(minfo->modelid_other_gender);
        if(!minfo_tmp)
        {
            sLog.outErrorDb("Model (Entry: %u) has modelid_other_gender %u not found in table `creature_model_info`. ", minfo->modelid, minfo->modelid_other_gender);
            return minfo;                                   // not fatal, just use the previous one
        }
        else
            return minfo_tmp;
    }
    else
        return minfo;
}

void ObjectMgr::LoadCreatureModelInfo()
{
    sCreatureModelStorage.Load();

    sLog.outString( ">> Loaded %u creature model based info", sCreatureModelStorage.RecordCount );
    sLog.outString();
}

void ObjectMgr::LoadCreatures()
{
    uint32 count = 0;
    //                                                       0      1    2     3
    QueryResult *result = WorldDatabase.Query("SELECT `creature`.`guid`,`id`,`map`,`modelid`,"
    //    4             5            6            7            8             9               10          11
        "`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawndist`,`currentwaypoint`,"
    //   12                 13                 14                 15                  16          17        18           19             20
        "`spawn_position_x`,`spawn_position_y`,`spawn_position_z`,`spawn_orientation`,`curhealth`,`curmana`,`DeathState`,`MovementType`,`event` "
        "FROM `creature` LEFT OUTER JOIN `game_event_creature` ON `creature`.`guid`=`game_event_creature`.`guid`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outErrorDb(">> Loaded 0 creature. DB table `creature` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 guid = fields[0].GetUInt32();

        CreatureData& data = mCreatureDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.displayid      = fields[ 3].GetUInt32();
        data.equipmentId    = fields[ 4].GetUInt32();
        data.posX           = fields[ 5].GetFloat();
        data.posY           = fields[ 6].GetFloat();
        data.posZ           = fields[ 7].GetFloat();
        data.orientation    = fields[ 8].GetFloat();
        data.spawntimesecs  = fields[ 9].GetUInt32();
        data.spawndist      = fields[10].GetFloat();
        data.currentwaypoint= fields[11].GetUInt32();
        data.spawn_posX     = fields[12].GetFloat();
        data.spawn_posY     = fields[13].GetFloat();
        data.spawn_posZ     = fields[14].GetFloat();
        data.spawn_orientation = fields[15].GetFloat();
        data.curhealth      = fields[16].GetUInt32();
        data.curmana        = fields[17].GetUInt32();
        data.deathState     = fields[18].GetUInt8();
        data.movementType   = fields[19].GetUInt8();
        int16 gameEvent     = fields[20].GetInt16();

        if (gameEvent==0)                                   // if not this is to be managed by GameEvent System
            AddCreatureToGrid(guid, &data);
        count++;

    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u creatures", mCreatureDataMap.size() );
}

void ObjectMgr::AddCreatureToGrid(uint32 guid, CreatureData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.creatures.insert(guid);
}

void ObjectMgr::RemoveCreatureFromGrid(uint32 guid, CreatureData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.creatures.erase(guid);
}

void ObjectMgr::LoadGameobjects()
{
    uint32 count = 0;

    //                                                         0      1    2     3            4            5            6
    QueryResult *result = WorldDatabase.Query("SELECT `gameobject`.`guid`,`id`,`map`,`position_x`,`position_y`,`position_z`,`orientation`,"
    //   7           8           9           10          11               12             13        14
        "`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`event` "
        "FROM `gameobject` LEFT OUTER JOIN `game_event_gameobject` ON `gameobject`.`guid`=`game_event_gameobject`.`guid`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 gameobjects. DB table `gameobject` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 guid = fields[0].GetUInt32();

        GameObjectData& data = mGameObjectDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.posX           = fields[ 3].GetFloat();
        data.posY           = fields[ 4].GetFloat();
        data.posZ           = fields[ 5].GetFloat();
        data.orientation    = fields[ 6].GetFloat();
        data.rotation0      = fields[ 7].GetFloat();
        data.rotation1      = fields[ 8].GetFloat();
        data.rotation2      = fields[ 9].GetFloat();
        data.rotation3      = fields[10].GetFloat();
        data.spawntimesecs  = fields[11].GetInt32();
        data.animprogress   = fields[12].GetUInt32();
        data.go_state       = fields[13].GetUInt32();
        int16 gameEvent     = fields[14].GetInt16();

        if (gameEvent==0)                                   // if not this is to be managed by GameEvent System
            AddGameobjectToGrid(guid, &data);
        count++;

    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u gameobjects", mGameObjectDataMap.size());
}

void ObjectMgr::AddGameobjectToGrid(uint32 guid, GameObjectData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.gameobjects.insert(guid);
}

void ObjectMgr::RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data)
{
    CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
    cell_guids.gameobjects.erase(guid);
}

void ObjectMgr::LoadCreatureRespawnTimes()
{
    // remove outdated data
    WorldDatabase.Execute("DELETE FROM `creature_respawn` WHERE `respawntime` <= UNIX_TIMESTAMP(NOW())");

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT `guid`,`respawntime`,`instance` FROM `creature_respawn`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 creature respawn time.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mCreatureRespawnTimes[MAKE_GUID(loguid,instance)] = time_t(respawn_time);

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString( ">> Loaded %u creature respawn times", mCreatureRespawnTimes.size() );
    sLog.outString();
}

void ObjectMgr::LoadGameobjectRespawnTimes()
{
    // remove outdated data
    WorldDatabase.Execute("DELETE FROM `gameobject_respawn` WHERE `respawntime` <= UNIX_TIMESTAMP(NOW())");

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT `guid`,`respawntime`,`instance` FROM `gameobject_respawn`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 gameobject respawn time.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mGORespawnTimes[MAKE_GUID(loguid,instance)] = time_t(respawn_time);

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString( ">> Loaded %u gameobject respawn times", mGORespawnTimes.size() );
    sLog.outString();
}

void ObjectMgr::LoadSpellThreats()
{
    sSpellThreatStore.Load();

    sLog.outString( ">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount );
    sLog.outString();
}

// name must be checked to correctness (if received) before call this function
uint64 ObjectMgr::GetPlayerGUIDByName(std::string name) const
{
    uint64 guid = 0;

    CharacterDatabase.escape_string(name);

    // Player name safe to sending to DB (checked at login) and this function using
    QueryResult *result = CharacterDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name.c_str());
    if(result)
    {
        guid = MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER);

        delete result;
    }

    return guid;
}

bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{
    // prevent DB access for online player
    if(Player* player = GetPlayer(guid))
    {
        name = player->GetName();
        return true;
    }

    QueryResult *result = CharacterDatabase.PQuery("SELECT `name` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

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
    QueryResult *result = CharacterDatabase.PQuery("SELECT `race` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));

    if(result)
    {
        uint8 race = (*result)[0].GetUInt8();
        delete result;
        return Player::TeamForRace(race);
    }

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByGUID(const uint64 &guid) const
{
    QueryResult *result = CharacterDatabase.PQuery("SELECT `account` FROM `character` WHERE `guid` = '%u'", GUID_LOPART(guid));
    if(result)
    {
        uint32 acc = (*result)[0].GetUInt32();
        delete result;
        return acc;
    }

    return 0;
}

uint32 ObjectMgr::GetSecurityByAccount(uint32 acc_id) const
{
    QueryResult *result = loginDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `id` = '%u'", acc_id);
    if(result)
    {
        uint32 sec = (*result)[0].GetUInt32();
        delete result;
        return sec;
    }

    return 0;
}

void ObjectMgr::LoadAuctions()
{
    QueryResult *result = CharacterDatabase.Query("SELECT COUNT(*) FROM `auctionhouse`");

    Field *fields = result->Fetch();
    uint32 AuctionCount=fields[0].GetUInt32();
    delete result;

    if(!AuctionCount)
        return;

    result = CharacterDatabase.Query( "SELECT `id`,`auctioneerguid`,`itemguid`,`item_template`,`itemowner`,`buyoutprice`,`time`,`buyguid`,`lastbid`,`startbid`,`deposit`,`location` FROM `auctionhouse`" );

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
        aItem->item_guid = fields[2].GetUInt32();
        aItem->item_template = fields[3].GetUInt32();
        aItem->owner = fields[4].GetUInt32();
        aItem->buyout = fields[5].GetUInt32();
        aItem->time = fields[6].GetUInt32();
        aItem->bidder = fields[7].GetUInt32();
        aItem->bid = fields[8].GetUInt32();
        aItem->startbid = fields[9].GetUInt32();
        aItem->deposit = fields[10].GetUInt32();
        aItem->location = fields[11].GetUInt8();
        //check if sold item exists
        if ( this->GetAItem( aItem->item_guid ) )
        {
            GetAuctionsMap( aItem->location )->AddAuction(aItem);
        }
        else
        {
            CharacterDatabase.PExecute("DELETE FROM `auctionhouse` WHERE `id` = '%u'",aItem->Id);
            sLog.outError("Auction %u has not a existing item : %u", aItem->Id, aItem->item_guid);
            delete aItem;
        }
    } while (result->NextRow());
    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u auctions", AuctionCount );
    sLog.outString();
}

void ObjectMgr::LoadItemLocales()
{
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`name_loc1`,`description_loc1`,`name_loc2`,`description_loc2`,`name_loc3`,`description_loc3`,`name_loc4`,`description_loc4`,`name_loc5`,`description_loc5`,`name_loc6`,`description_loc6`,`name_loc7`,`description_loc7` FROM `locales_item`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 Item locale strings. DB table `locales_item` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        ItemLocale& data = mItemLocaleMap[entry];

        for(int i = 1; i < 8; ++i)
        {
            std::string str = fields[1+2*(i-1)].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Name.size() <= idx)
                        data.Name.resize(idx+1);

                    data.Name[idx] = str;
                }
            }

            str = fields[1+2*(i-1)+1].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Description.size() <= idx)
                        data.Description.resize(idx+1);

                    data.Description[idx] = str;
                }
            }
        }
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u Item locale strings", mItemLocaleMap.size() );
}

void ObjectMgr::LoadItemPrototypes()
{
    sItemStorage.Load ();
    sLog.outString( ">> Loaded %u item prototypes", sItemStorage.RecordCount );
    sLog.outString();

    // check data correctness
    for(uint32 i = 1; i < sItemStorage.MaxEntry; ++i)
    {
        ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype >(i);
        if(!proto)
            continue;

        if(proto->Class >= MAX_ITEM_CLASS)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong Class value (%u)",i,proto->Class);
            const_cast<ItemPrototype*>(proto)->Class = ITEM_CLASS_JUNK;
        }

        if(proto->SubClass >= MaxItemSubclassValues[proto->Class])
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong Subclass value (%u) for class %u",i,proto->SubClass,proto->Class);
            const_cast<ItemPrototype*>(proto)->SubClass = 0;// exist for all item classes
        }

        if(proto->Quality >= MAX_ITEM_QUALITY)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong Quality value (%u)",i,proto->Quality);
            const_cast<ItemPrototype*>(proto)->Quality = ITEM_QUALITY_NORMAL;
        }

        if(proto->InventoryType >= MAX_INVTYPE)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong InventoryType value (%u)",i,proto->InventoryType);
            const_cast<ItemPrototype*>(proto)->InventoryType = INVTYPE_NON_EQUIP;
        }

        if(proto->RequiredSkill >= MAX_SKILL_TYPE)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong RequiredSkill value (%u)",i,proto->RequiredSkill);
            const_cast<ItemPrototype*>(proto)->RequiredSkill = 0;
        }

        if(proto->RequiredSpell && !sSpellStore.LookupEntry(proto->RequiredSpell))
        {
            sLog.outErrorDb("Item (Entry: %u) have wrong (non-existed) spell in RequiredSpell (%u)",i,proto->RequiredSpell);
            const_cast<ItemPrototype*>(proto)->RequiredSpell = 0;
        }

        if(proto->RequiredReputationRank >= MAX_REPUTATION_RANK)
            sLog.outErrorDb("Item (Entry: %u) has wrong reputation rank in RequiredReputationRank (%u), item can't be used.",i,proto->RequiredReputationRank);

        if(proto->RequiredReputationFaction)
        {
            if(!sFactionStore.LookupEntry(proto->RequiredReputationFaction))
            {
                sLog.outErrorDb("Item (Entry: %u) has wrong (not existing) faction in RequiredReputationFaction (%u)",i,proto->RequiredReputationFaction);
                const_cast<ItemPrototype*>(proto)->RequiredReputationFaction = 0;
            }

            if(proto->RequiredReputationRank == MIN_REPUTATION_RANK)
                sLog.outErrorDb("Item (Entry: %u) has min. reputation rank in RequiredReputationRank (0) but RequiredReputationFaction > 0, faction setting is useless.",i);
        }
        else if(proto->RequiredReputationRank > MIN_REPUTATION_RANK)
            sLog.outErrorDb("Item (Entry: %u) has RequiredReputationFaction ==0 but RequiredReputationRank > 0, rank setting is useless.",i);

        for (int j = 0; j < 10; j++)
        {
            // for ItemStatValue != 0
            if(proto->ItemStat[j].ItemStatValue && proto->ItemStat[j].ItemStatType >= MAX_ITEM_MOD)
            {
                sLog.outErrorDb("Item (Entry: %u) has wrong stat_type%d (%u)",i,j+1,proto->ItemStat[j].ItemStatType);
                const_cast<ItemPrototype*>(proto)->ItemStat[j].ItemStatType = 0;
            }
        }

        for (int j = 0; j < 5; j++)
        {
            if(proto->Damage[j].DamageType >= MAX_SPELL_SCHOOL)
            {
                sLog.outErrorDb("Item (Entry: %u) has wrong dmg_type%d (%u)",i,j+1,proto->Damage[j].DamageType);
                const_cast<ItemPrototype*>(proto)->Damage[j].DamageType = 0;
            }
        }

        for (int j = 0; j < 5; j++)
        {
            if(proto->Spells[j].SpellTrigger >= MAX_ITEM_SPELLTRIGGER)
            {
                sLog.outErrorDb("Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u)",i,j+1,proto->Spells[j].SpellTrigger);
                const_cast<ItemPrototype*>(proto)->Spells[j].SpellId = 0;
                const_cast<ItemPrototype*>(proto)->Spells[j].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }
            else if(proto->Spells[j].SpellId)
            {
                SpellEntry const* spellInfo = sSpellStore.LookupEntry(proto->Spells[j].SpellId);
                if(!spellInfo)
                {
                    sLog.outErrorDb("Item (Entry: %u) has wrong (not existing) spell in spellid_%d (%u)",i,j+1,proto->Spells[j].SpellId);
                    const_cast<ItemPrototype*>(proto)->Spells[j].SpellId = 0;
                }
                else
                {
                    if(!IsSpellValid(spellInfo))
                    {
                        sLog.outErrorDb("Item (Entry: %u) has broken spell in spellid_%d (%u)",i,j+1,proto->Spells[j].SpellId);
                        const_cast<ItemPrototype*>(proto)->Spells[j].SpellId = 0;
                    }
                }
            }
        }

        if(proto->Bonding >= MAX_BIND_TYPE)
            sLog.outErrorDb("Item (Entry: %u) has wrong Bonding value (%u)",i,proto->Bonding);

        if(proto->PageText && !sPageTextStore.LookupEntry<PageText>(proto->PageText))
            sLog.outErrorDb("Item (Entry: %u) has non existing first page (Id:%u)", i,proto->PageText);

        if(proto->LockID && !sLockStore.LookupEntry(proto->LockID))
            sLog.outErrorDb("Item (Entry: %u) has wrong LockID (%u)",i,proto->LockID);

        if(proto->Sheath >= MAX_SHEATHETYPE)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong Sheath (%u)",i,proto->Sheath);
            const_cast<ItemPrototype*>(proto)->Sheath = SHEATHETYPE_NONE;
        }

        if(proto->RandomProperty && !sItemRandomPropertiesStore.LookupEntry(GetItemEnchantMod(proto->RandomProperty)))
        {
            sLog.outErrorDb("Item (Entry: %u) has unknown (wrong or not listed in `item_enchantment_template`) RandomProperty (%u)",i,proto->RandomProperty);
            const_cast<ItemPrototype*>(proto)->RandomProperty = 0;
        }

        if(proto->RandomSuffix && !sItemRandomSuffixStore.LookupEntry(GetItemEnchantMod(proto->RandomSuffix)))
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong RandomSuffix (%u)",i,proto->RandomSuffix);
            const_cast<ItemPrototype*>(proto)->RandomSuffix = 0;
        }

        if(proto->ItemSet && !sItemSetStore.LookupEntry(proto->ItemSet))
        {
            sLog.outErrorDb("Item (Entry: %u) have wrong ItemSet (%u)",i,proto->ItemSet);
            const_cast<ItemPrototype*>(proto)->ItemSet = 0;
        }

        if(proto->Area && !GetAreaEntryByAreaID(proto->Area))
            sLog.outErrorDb("Item (Entry: %u) has wrong Area (%u)",i,proto->Area);

        if(proto->Map && !sMapStore.LookupEntry(proto->Map))
            sLog.outErrorDb("Item (Entry: %u) has wrong Map (%u)",i,proto->Map);

        if(proto->TotemCategory && !sTotemCategoryStore.LookupEntry(proto->TotemCategory))
            sLog.outErrorDb("Item (Entry: %u) has wrong TotemCategory (%u)",i,proto->TotemCategory);

        for (int j = 0; j < 3; j++)
        {
            if(proto->Socket[j].Color && (proto->Socket[j].Color & SOCKET_COLOR_ALL) != proto->Socket[j].Color)
            {
                sLog.outErrorDb("Item (Entry: %u) has wrong socketColor_%d (%u)",i,j+1,proto->Socket[j].Color);
                const_cast<ItemPrototype*>(proto)->Socket[j].Color = 0;
            }
        }

        if(proto->GemProperties && !sGemPropertiesStore.LookupEntry(proto->GemProperties))
            sLog.outErrorDb("Item (Entry: %u) has wrong GemProperties (%u)",i,proto->GemProperties);

        if(proto->ExtendedCost && !sItemExtendedCostStore.LookupEntry(proto->ExtendedCost))
            sLog.outErrorDb("Item (Entry: %u) has wrong ExtendedCost (%u)",i,proto->ExtendedCost);

        if(proto->FoodType >= MAX_PET_DIET)
        {
            sLog.outErrorDb("Item (Entry: %u) has wrong FoodType value (%u)",i,proto->FoodType);
            const_cast<ItemPrototype*>(proto)->FoodType = 0;
        }
    }
}

void ObjectMgr::LoadAuctionItems()
{
    QueryResult *result = CharacterDatabase.Query( "SELECT `itemguid`,`item_template` FROM `auctionhouse`" );

    if( !result )
        return;

    barGoLink bar( result->GetRowCount() );

    uint32 count = 0;

    Field *fields;
    do
    {
        bar.step();

        fields = result->Fetch();
        uint32 item_guid        = fields[0].GetUInt32();
        uint32 item_template    = fields[1].GetUInt32();

        ItemPrototype const *proto = objmgr.GetItemPrototype(item_template);

        if(!proto)
        {
            sLog.outError( "ObjectMgr::LoadAuctionItems: Unknown item (GUID: %u id: #%u) in auction, skipped.", item_guid,item_template);
            continue;
        }

        Item *item = NewItemOrBag(proto);

        if(!item->LoadFromDB(item_guid,0))
        {
            delete item;
            continue;
        }
        AddAItem(item);

        ++count;
    }
    while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u auction items", count );
}

void ObjectMgr::LoadPetLevelInfo()
{
    // Loading levels data
    {
        //                                              0               1       2    3      4     5     6     7     8     9
        QueryResult *result  = WorldDatabase.Query("SELECT `creature_entry`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi`,`armor` FROM `pet_levelstats`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u level pet stats definitions", count );
            sLog.outErrorDb( "Error loading pet_levelstats table or empty table.");
            return;
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 creature_id = fields[0].GetUInt32();
            if(!sCreatureStorage.LookupEntry<CreatureInfo>(creature_id))
            {
                sLog.outErrorDb("Wrong creature id %u in `pet_levelstats` table, ignoring.",creature_id);
                continue;
            }

            uint32 current_level = fields[1].GetUInt32();
            if(current_level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > 255)                     // hardcoded level maximum
                    sLog.outErrorDb("Wrong (> 255) level %u in `pet_levelstats` table, ignoring.",current_level);
                else
                    sLog.outDetail("Unused (> MaxPlayerLevel in mangosd.conf) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }
            else if(current_level < 1)
            {
                sLog.outErrorDb("Wrong (<1) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }

            PetLevelInfo*& pInfoMapEntry = petInfo[creature_id];

            if(pInfoMapEntry==NULL)
                pInfoMapEntry =  new PetLevelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            // data for level 1 stored in [0] array element, ...
            PetLevelInfo* pLevelInfo = &pInfoMapEntry[current_level-1];

            pLevelInfo->health = fields[2].GetUInt16();
            pLevelInfo->mana   = fields[3].GetUInt16();
            pLevelInfo->armor  = fields[9].GetUInt16();

            for (int i = 0; i < MAX_STATS; i++)
            {
                pLevelInfo->stats[i] = fields[i+4].GetUInt16();
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString();
        sLog.outString( ">> Loaded %u level pet stats definitions", count );
    }

    // Fill gaps and check integrity
    for (PetLevelInfoMap::iterator itr = petInfo.begin(); itr != petInfo.end(); ++itr)
    {
        PetLevelInfo* pInfo = itr->second;

        // fatal error if no level 1 data
        if(!pInfo || pInfo[0].health == 0 )
        {
            sLog.outErrorDb("Creature %u does not have pet stats data for Level 1!",itr->first);
            exit(1);
        }

        // fill level gaps
        for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
        {
            if(pInfo[level].health == 0)
            {
                sLog.outErrorDb("Creature %u has no data for Level %i pet stats data, using data of Level %i.",itr->first,level+1, level);
                pInfo[level] = pInfo[level-1];
            }
        }
    }
}

PetLevelInfo const* ObjectMgr::GetPetLevelInfo(uint32 creature_id, uint32 level) const
{
    if(level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        level = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL);

    PetLevelInfoMap::const_iterator itr = petInfo.find(creature_id);
    if(itr == petInfo.end())
        return NULL;

    return &itr->second[level-1];                           // data for level 1 stored in [0] array element, ...
}

void ObjectMgr::LoadPlayerInfo()
{
    // allocate dynamic array
    playerInfo = new PlayerInfo*[MAX_RACES];
    for (uint8 race = 0; race < MAX_RACES; ++race)
    {
        playerInfo[race] = new PlayerInfo[MAX_CLASSES];
    }

    // Load playercreate
    {
        //                                              0      1       2     3      4            5            6
        QueryResult *result = WorldDatabase.Query("SELECT `race`,`class`,`map`,`zone`,`position_x`,`position_y`,`position_z` FROM `playercreateinfo`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u player create definitions", count );
            sLog.outErrorDb( "Error loading `playercreateinfo` table or empty table.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outErrorDb("Wrong race %u in `playercreateinfo` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outErrorDb("Wrong class %u in `playercreateinfo` table, ignoring.",current_class);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            pInfo->mapId     = fields[2].GetUInt32();
            pInfo->zoneId    = fields[3].GetUInt32();
            pInfo->positionX = fields[4].GetFloat();
            pInfo->positionY = fields[5].GetFloat();
            pInfo->positionZ = fields[6].GetFloat();

            ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(fields[0].GetUInt32());
            if(rEntry)
            {
                pInfo->displayId_m = rEntry->model_m;
                pInfo->displayId_f = rEntry->model_f;
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString();
        sLog.outString( ">> Loaded %u player create definitions", count );
    }

    // Load playercreate items
    {
        //                                            0      1       2        3
        QueryResult *result = WorldDatabase.Query("SELECT `race`,`class`,`itemid`,`amount` FROM `playercreateinfo_item`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u player create items", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_item` table or empty table.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_item` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_item` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];

                pInfo->item.push_back(PlayerCreateInfoItem( fields[2].GetUInt32(), fields[3].GetUInt32()));

                bar.step();
                count++;
            }
            while(result->NextRow());

            delete result;

            sLog.outString();
            sLog.outString( ">> Loaded %u player create items", count );
        }
    }

    // Load playercreate spells
    {
        //                                            0      1       2       3
        QueryResult *result = WorldDatabase.Query("SELECT `race`,`class`,`Spell`,`Active` FROM `playercreateinfo_spell`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u player create spells", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_spell` table or empty table.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_spell` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_spell` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->spell.push_back(CreateSpellPair(fields[2].GetUInt16(), fields[3].GetBool()));

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString();
            sLog.outString( ">> Loaded %u player create spells", count );
        }
    }

    // Load playercreate skills
    {
        //                                            0      1       2
        QueryResult *result = WorldDatabase.Query("SELECT `race`,`class`,`Skill` FROM `playercreateinfo_skill`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u player create skills", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_skill` table or empty table.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_skill` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_skill` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];

                uint32 skill = fields[2].GetUInt16();

                pInfo->skill.push_back(skill);

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString();
            sLog.outString( ">> Loaded %u player create skills", count );
        }
    }

    // Load playercreate actions
    {
        //                                                    0      1       2        3        4      5
        QueryResult *result = WorldDatabase.Query("SELECT `race`,`class`,`button`,`action`,`type`,`misc` FROM `playercreateinfo_action`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u player create actions", count );
            sLog.outErrorDb( "Error loading `playercreateinfo_action` table or empty table.");
        }
        else
        {
            barGoLink bar( result->GetRowCount() );

            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    sLog.outErrorDb("Wrong race %u in `playercreateinfo_action` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    sLog.outErrorDb("Wrong class %u in `playercreateinfo_action` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->action[0].push_back(fields[2].GetUInt16());
                pInfo->action[1].push_back(fields[3].GetUInt16());
                pInfo->action[2].push_back(fields[4].GetUInt16());
                pInfo->action[3].push_back(fields[5].GetUInt16());

                bar.step();
                count++;
            }
            while( result->NextRow() );

            delete result;

            sLog.outString();
            sLog.outString( ">> Loaded %u player create actions", count );
        }
    }

    // Loading levels data
    {
        //                                              0      1       2       3    4      5     6     7     8     9
        QueryResult *result  = WorldDatabase.Query("SELECT `race`,`class`,`level`,`hp`,`mana`,`str`,`agi`,`sta`,`int`,`spi` FROM `player_levelstats`");

        uint32 count = 0;

        if (!result)
        {
            barGoLink bar( 1 );

            sLog.outString();
            sLog.outString( ">> Loaded %u level stats definitions", count );
            sLog.outErrorDb( "Error loading player_levelstats table or empty table.");
            exit(1);
        }

        barGoLink bar( result->GetRowCount() );

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                sLog.outErrorDb("Wrong race %u in `player_levelstats` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                sLog.outErrorDb("Wrong class %u in `player_levelstats` table, ignoring.",current_class);
                continue;
            }

            uint32 current_level = fields[2].GetUInt32();
            if(current_level > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > 255)                     // hardcoded level maximum
                    sLog.outErrorDb("Wrong (> 255) level %u in `player_levelstats` table, ignoring.",current_level);
                else
                    sLog.outDetail("Unused (> MaxPlayerLevel in mangosd.conf) level %u in `player_levelstats` table, ignoring.",current_level);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            if(!pInfo->levelInfo)
                pInfo->levelInfo = new PlayerLevelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            PlayerLevelInfo* pLevelInfo = &pInfo->levelInfo[current_level-1];

            pLevelInfo->health = fields[3].GetUInt16();
            pLevelInfo->mana   = fields[4].GetUInt16();

            for (int i = 0; i < MAX_STATS; i++)
            {
                pLevelInfo->stats[i] = fields[i+5].GetUInt8();
            }

            bar.step();
            count++;
        }
        while (result->NextRow());

        delete result;

        sLog.outString();
        sLog.outString( ">> Loaded %u level stats definitions", count );
    }

    // Fill gaps and check integrity
    for (int race = 0; race < MAX_RACES; ++race)
    {
        // skip non existed races
        if(!sChrRacesStore.LookupEntry(race))
            continue;

        for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
        {
            // skip non existed classes
            if(!sChrClassesStore.LookupEntry(class_))
                continue;

            PlayerInfo* pInfo = &playerInfo[race][class_];

            // skip non loaded combinations
            if(!pInfo->displayId_m || !pInfo->displayId_f)
                continue;

            // skip expansion races if not playing with expansion
            if (!sWorld.getConfig(CONFIG_EXPANSION) && (race == RACE_BLOODELF || race == RACE_DRAENEI))
                continue;

            // fatal error if no level 1 data
            if(!pInfo->levelInfo || pInfo->levelInfo[0].health == 0 )
            {
                sLog.outErrorDb("Race %i Class %i Level 1 does not have stats data!",race,class_);
                exit(1);
            }

            // fill level gaps
            for (uint32 level = 1; level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
            {
                if(pInfo->levelInfo[level].health == 0)
                {
                    sLog.outErrorDb("Race %i Class %i Level %i does not have stats data. Using stats data of level %i.",race,class_,level+1, level);
                    pInfo->levelInfo[level] = pInfo->levelInfo[level-1];
                }
            }
        }
    }
}

void ObjectMgr::GetPlayerLevelInfo(uint32 race, uint32 class_, uint32 level, PlayerLevelInfo* info) const
{
    if(level < 1) return;
    if(race   >= MAX_RACES)   return;
    if(class_ >= MAX_CLASSES) return;
    PlayerInfo const* pInfo = &playerInfo[race][class_];
    if(pInfo->displayId_m==0 || pInfo->displayId_f==0) return;

    if(level <= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        *info = pInfo->levelInfo[level-1];
    else
        BuildPlayerLevelInfo(race,class_,level,info);
}

void ObjectMgr::BuildPlayerLevelInfo(uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info) const
{
    // base data (last known level)
    *info = playerInfo[race][_class].levelInfo[sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)-1];

    for(int lvl = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)-1; lvl < level; ++lvl)
    {
        switch(_class)
        {
            case CLASS_WARRIOR:
                info->stats[STAT_STRENGTH]  += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 9 && !(lvl%2) ? 1: 0);
                break;
            case CLASS_PALADIN:
                info->stats[STAT_STRENGTH]  += (lvl > 3  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 6 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 7 ? 1: 0);
                break;
            case CLASS_HUNTER:
                info->stats[STAT_STRENGTH]  += (lvl > 4  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_ROGUE:
                info->stats[STAT_STRENGTH]  += (lvl > 5  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_PRIEST:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 3  ? 1: 0);
                break;
            case CLASS_SHAMAN:
                info->stats[STAT_STRENGTH]  += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 4 ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 7 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 5 ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 4 ? 1: 0);
                break;
            case CLASS_MAGE:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                break;
            case CLASS_WARLOCK:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                break;
            case CLASS_DRUID:
                info->stats[STAT_STRENGTH]  += (lvl > 38 ? 2: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 32 ? 2: (lvl > 4 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 2: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 38 ? 3: (lvl > 4 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 3: (lvl > 5 ? 1: 0));
        }

        info->mana   += (_class == CLASS_WARRIOR || _class == CLASS_ROGUE) ? 0 : info->stats[STAT_SPIRIT] / 2;
        info->health += info->stats[STAT_STAMINA] / 2;
    }
}

void ObjectMgr::LoadGuilds()
{
    Guild *newguild;
    uint32 count = 0;

    QueryResult *result = CharacterDatabase.Query( "SELECT `guildid` FROM `guild`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
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
        if(!newguild->LoadGuildFromDB(fields[0].GetUInt32()))
        {
            newguild->Disband();
            delete newguild;
            continue;
        }
        AddGuild(newguild);

    }while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u guild definitions", count );
}

void ObjectMgr::LoadArenaTeams()
{
    uint32 count = 0;

    QueryResult *result = CharacterDatabase.Query( "SELECT `arenateamid` FROM `arena_team`" );

    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u arenateam definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();
        count++;

        ArenaTeam *newarenateam = new ArenaTeam;
        if(!newarenateam->LoadArenaTeamFromDB(fields[0].GetUInt32()))
        {
            delete newarenateam;
            continue;
        }
        AddArenaTeam(newarenateam);
    }while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u arenateam definitions", count );
}

void ObjectMgr::LoadGroups()
{
    Group *group;
    uint32 count = 0;

    QueryResult *result = CharacterDatabase.Query( "SELECT `leaderGuid` FROM `group`" );

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u group definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();
        count++;

        group = new Group;
        if(!group->LoadGroupFromDB(MAKE_GUID((*result)[0].GetUInt32(),HIGHGUID_PLAYER)))
        {
            group->Disband();
            delete group;
            continue;
        }
        AddGroup(group);

    }while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u group definitions", count );
}

void ObjectMgr::LoadQuests()
{
    // For reload case
    for(QuestMap::const_iterator itr=mQuestTemplates.begin(); itr != mQuestTemplates.end(); ++itr)
        delete itr->second;
    mQuestTemplates.clear();

    mExclusiveQuestGroups.clear();

    //                                            0       1            2          3            4
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`ZoneOrSort`,`MinLevel`,`QuestLevel`,`Type`,"
    //   5               6                    7                       8                     9                       10                    11                 12
        "`RequiredRaces`,`RequiredSkillValue`,`RequiredMinRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepFaction`,`RequiredMaxRepValue`,`SuggestedPlayers`,`LimitTime`,"
    //   13           14             15            16               17                 18          19             20            21
        "`QuestFlags`,`SpecialFlags`,`PrevQuestId`,`NextQuestId`,`ExclusiveGroup`,`NextQuestInChain`,`SrcItemId`,`SrcItemCount`,`SrcSpell`,"
    //   22      23        24           25                26                 27        28               29               30               31
        "`Title`,`Details`,`Objectives`,`OfferRewardText`,`RequestItemsText`,`EndText`,`ObjectiveText1`,`ObjectiveText2`,`ObjectiveText3`,`ObjectiveText4`,"
    //   32           33           34           35           36              37              38              39
        "`ReqItemId1`,`ReqItemId2`,`ReqItemId3`,`ReqItemId4`,`ReqItemCount1`,`ReqItemCount2`,`ReqItemCount3`,`ReqItemCount4`,"
    //   40             41             42             43             44                45                46                47                48              49              50              51
        "`ReqSourceId1`,`ReqSourceId2`,`ReqSourceId3`,`ReqSourceId4`,`ReqSourceCount1`,`ReqSourceCount2`,`ReqSourceCount3`,`ReqSourceCount4`,`ReqSourceRef1`,`ReqSourceRef2`,`ReqSourceRef3`,`ReqSourceRef4`,"
    //   52                   53                   54                   55                   56                      57                      58                      59
        "`ReqCreatureOrGOId1`,`ReqCreatureOrGOId2`,`ReqCreatureOrGOId3`,`ReqCreatureOrGOId4`,`ReqCreatureOrGOCount1`,`ReqCreatureOrGOCount2`,`ReqCreatureOrGOCount3`,`ReqCreatureOrGOCount4`,"
    //   60              61              62              63
        "`ReqSpellCast1`,`ReqSpellCast2`,`ReqSpellCast3`,`ReqSpellCast4`,"
    //   64                 65                 66                 67                 68                 69
        "`RewChoiceItemId1`,`RewChoiceItemId2`,`RewChoiceItemId3`,`RewChoiceItemId4`,`RewChoiceItemId5`,`RewChoiceItemId6`,"
    //   70                    71                    72                    73                    74                    75
        "`RewChoiceItemCount1`,`RewChoiceItemCount2`,`RewChoiceItemCount3`,`RewChoiceItemCount4`,`RewChoiceItemCount5`,`RewChoiceItemCount6`,"
    //   76           77           78           79           80              81              82              83
        "`RewItemId1`,`RewItemId2`,`RewItemId3`,`RewItemId4`,`RewItemCount1`,`RewItemCount2`,`RewItemCount3`,`RewItemCount4`,"
    //   84               85               86               87               88               89             90             91             92             93
        "`RewRepFaction1`,`RewRepFaction2`,`RewRepFaction3`,`RewRepFaction4`,`RewRepFaction5`,`RewRepValue1`,`RewRepValue2`,`RewRepValue3`,`RewRepValue4`,`RewRepValue5`,"
    //   94              95                 96         97           98       99       100        101              102            103             104
        "`RewOrReqMoney`,`RewMoneyMaxLevel`,`RewSpell`,`PointMapId`,`PointX`,`PointY`,`PointOpt`,`DetailsEmote1`,`DetailsEmote2`,`DetailsEmote3`,`DetailsEmote4`,"
    //   105               106             107                 108                 109                 110                 111           112
        "`IncompleteEmote`,`CompleteEmote`,`OfferRewardEmote1`,`OfferRewardEmote2`,`OfferRewardEmote3`,`OfferRewardEmote4`,`StartScript`,`CompleteScript`"
        " FROM `quest_template`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 quests definitions" );
        sLog.outErrorDb("`quest_template` table is empty!");
        return;
    }

    // create multimap previous quest for each existed quest
    // some quests can have many previous maps set by NextQuestId in previous quest
    // for example set of race quests can lead to single not race specific quest
    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        Quest * newQuest = new Quest(fields);
        mQuestTemplates[newQuest->GetQuestId()] = newQuest;
    } while( result->NextRow() );

    delete result;

    // Post processing
    for (QuestMap::iterator iter = mQuestTemplates.begin(); iter != mQuestTemplates.end(); iter++)
    {
        Quest * qinfo = iter->second;

        // additional quest integrity checks (GO, creature_template and item_template must be loaded already)

        if (qinfo->QuestFlags & ~QUEST_MANGOS_FLAGS_DB_ALLOWED)
        {
            sLog.outErrorDb("Quest %u has `QuestFlags` = %u > max allowed value. Correct `SpecialFlags` to value <= %u",
                qinfo->GetQuestId(),qinfo->QuestFlags,QUEST_MANGOS_FLAGS_DB_ALLOWED >> 16);
            qinfo->QuestFlags &= QUEST_MANGOS_FLAGS_DB_ALLOWED;
        }

        if(qinfo->Type == QUEST_TYPE_DAILY && (qinfo->GetFlags() & QUEST_SPECIAL_FLAGS_DAILY) == 0)
        {
            sLog.outErrorDb("Quest %u has `Type` = %u (QUEST_TYPE_DAILY) but does not have `SpecialFlags` set with mask %u (QUEST_SPECIAL_FLAGS_DAILY).",
                qinfo->GetQuestId(),qinfo->Type,QUEST_SPECIAL_FLAGS_DAILY);
            qinfo->QuestFlags |= QUEST_SPECIAL_FLAGS_DAILY;
        }

        // zone case
        if( qinfo->ZoneOrSort > 0 )
        {
            if(!GetAreaEntryByAreaID(qinfo->ZoneOrSort))
            {
                sLog.outErrorDb("Quest %u has `ZoneOrSort` = %u (zone case) but zone with this id does not exist.",
                    qinfo->GetQuestId(),qinfo->ZoneOrSort);
                // no changes, quest not dependent from this value but can have problems at client
            }
        }

        if( qinfo->RequiredSkillValue )
        {
            if( qinfo->RequiredSkillValue > sWorld.GetConfigMaxSkillValue() )
            {
                sLog.outErrorDb("Quest %u has `RequiredSkillValue` = %u but max possible skill is %u, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->RequiredSkillValue,sWorld.GetConfigMaxSkillValue());
                // no changes, quest can't be done for this requirement
            }

            if( qinfo->ZoneOrSort >= 0 )
            {
                sLog.outErrorDb("Quest %u has `RequiredSkillValue` = %u but ZoneOrSort = %i (zone case), value ignored.",
                    qinfo->GetQuestId(),qinfo->RequiredSkillValue,qinfo->ZoneOrSort);
                // no changes, quest will ignore requirement
            }
            else
            {
                uint32 reqskill = SkillByQuestSort(-qinfo->ZoneOrSort);
                if(!reqskill)
                {
                    sLog.outErrorDb("Quest %u has `RequiredSkillValue` = %u but ZoneOrSort = %i (non skill case), value ignored.",
                        qinfo->GetQuestId(),qinfo->RequiredSkillValue,qinfo->ZoneOrSort);
                    // no changes, quest will ignore requirement
                }
            }
        }
        // else Skill quests can have 0 skill level, this is ok

        if(qinfo->RequiredMinRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMinRepFaction))
        {
            sLog.outErrorDb("Quest %u has `RequiredMinRepFaction` = %u but faction template %u does not exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMinRepFaction,qinfo->RequiredMinRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMaxRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMaxRepFaction))
        {
            sLog.outErrorDb("Quest %u has `RequiredMaxRepFaction` = %u but faction template %u does not exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMaxRepFaction,qinfo->RequiredMaxRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMinRepValue && qinfo->RequiredMinRepValue > Player::Reputation_Cap)
        {
            sLog.outErrorDb("Quest %u has `RequiredMinRepValue` = %d but max reputation is %u, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMinRepValue,Player::Reputation_Cap);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMinRepValue && qinfo->RequiredMaxRepValue && qinfo->RequiredMaxRepValue <= qinfo->RequiredMinRepValue)
        {
            sLog.outErrorDb("Quest %u has `RequiredMaxRepValue` = %d and `RequiredMinRepValue` = %d, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMaxRepValue,qinfo->RequiredMinRepValue);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->SrcItemId)
        {
            if(!sItemStorage.LookupEntry<ItemPrototype>(qinfo->SrcItemId))
            {
                sLog.outErrorDb("Quest %u has `SrcItemId` = %u but item with entry %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcItemId,qinfo->SrcItemId);
                qinfo->SrcItemId = 0;                       // quest can't be done for this requirement
            }
            else if(qinfo->SrcItemCount==0)
            {
                sLog.outErrorDb("Quest %u has `SrcItemId` = %u but `SrcItemCount` = 0, set to 1 but need fix in DB.",
                    qinfo->GetQuestId(),qinfo->SrcItemId);
                qinfo->SrcItemCount = 1;                    // update to 1 for allow quest work for backward comptibility with DB
            }
        }
        else if(qinfo->SrcItemCount>0)
        {
            sLog.outErrorDb("Quest %u has `SrcItemId` = 0 but `SrcItemCount` = %u, useless value.",
                qinfo->GetQuestId(),qinfo->SrcItemCount);
            qinfo->SrcItemCount=0;                          // no quest work changes in fact
        }

        if(qinfo->SrcSpell)
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(qinfo->SrcSpell);
            if(!spellInfo)
            {
                sLog.outErrorDb("Quest %u has `SrcSpell` = %u but spell %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcSpell,qinfo->SrcSpell);
                qinfo->SrcSpell = 0;                        // quest can't be done for this requirement
            }
            else if(!IsSpellValid(spellInfo))
            {
                sLog.outErrorDb("Quest %u has `SrcSpell` = %u but spell %u is broken, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcSpell,qinfo->SrcSpell);
                qinfo->SrcSpell = 0;                        // quest can't be done for this requirement
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqItemId[j];
            if(id)
            {
                if(qinfo->ReqItemCount[j]==0)
                {
                    sLog.outErrorDb("Quest %u has `ReqItemId%d` = %u but `ReqItemCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can't be done for this requirement
                }

                qinfo->SetFlag(QUEST_MANGOS_FLAGS_DELIVER);

                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `ReqItemId%d` = %u but item with entry %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->ReqItemCount[j] = 0;             // prevent incorrect work of quest
                }
            }
            else if(qinfo->ReqItemCount[j]>0)
            {
                sLog.outErrorDb("Quest %u has `ReqItemId%d` = 0 but `ReqItemCount%d` = %u, quest can't be done.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->ReqItemCount[j]);
                qinfo->ReqItemCount[j] = 0;                 // prevent incorrect work of quest
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSourceId[j];
            if(id)
            {
                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = %u but item with entry %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }

                if(!qinfo->ReqSourceCount[j])
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = %u but `ReqSourceCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }

                if(!qinfo->ReqSourceRef[j])
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = %u but `ReqSourceRef%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }
            }
            else
            {
                if(qinfo->ReqSourceCount[j]>0)
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = 0 but `ReqSourceCount%d` = %u.",
                        qinfo->GetQuestId(),j+1,j+1,qinfo->ReqSourceCount[j]);
                    // no changes, quest ignore this data
                }

                if(qinfo->ReqSourceRef[j]>0)
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceId%d` = 0 but `ReqSourceRef%d` = %u.",
                        qinfo->GetQuestId(),j+1,j+1,qinfo->ReqSourceRef[j]);
                    // no changes, quest ignore this data
                }
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 ref = qinfo->ReqSourceRef[j];
            if(ref)
            {
                if(ref > QUEST_OBJECTIVES_COUNT)
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceRef%d` = %u but max value in `ReqSourceRef%d` is %u, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,j+1,QUEST_OBJECTIVES_COUNT);
                    // no changes, quest can't be done for this requirement
                }
                else
                if(!qinfo->ReqItemId[ref-1] && !qinfo->ReqSpell[ref-1])
                {
                    sLog.outErrorDb("Quest %u has `ReqSourceRef%d` = %u but `ReqItemId%u` = 0 and `ReqSpellCast%u` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,ref,ref);
                    // no changes, quest can't be done for this requirement
                }
                else if(qinfo->ReqItemId[ref-1] && qinfo->ReqSpell[ref-1])
                {
                    sLog.outErrorDb("Quest %u has `ReqItemId%u` = %u and `ReqSpellCast%u` = %u, quest can't have both fields <> 0, then can't be done.",
                        qinfo->GetQuestId(),ref,qinfo->ReqItemId[ref-1],ref,qinfo->ReqSpell[ref-1]);
                    // no changes, quest can't be done for this requirement
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSpell[j];
            if(id)
            {
                SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
                if(!spellInfo)
                {
                    sLog.outErrorDb("Quest %u has `ReqSpellCast%d` = %u but spell %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }

                if(!qinfo->ReqCreatureOrGOId[j])
                {
                    bool found = false;
                    for(int k = 0; k < 3; ++k)
                    {
                        if( spellInfo->Effect[k]==SPELL_EFFECT_QUEST_COMPLETE && spellInfo->EffectMiscValue[k]==qinfo->QuestId ||
                            spellInfo->Effect[k]==SPELL_EFFECT_SEND_EVENT)
                        {
                            found = true;
                            break;
                        }
                    }

                    if(found)
                    {
                        if(!qinfo->HasFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT))
                        {
                            sLog.outErrorDb("Spell (id: %u) have SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT for quest %u and ReqCreatureOrGOId%d = 0, but quest not have flag QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT. Quest flags or ReqCreatureOrGOId%d must be fixed, quest modified to enable objective.",spellInfo->Id,qinfo->QuestId,j+1,j+1);

                            // this will prevent quest completing without objective
                            const_cast<Quest*>(qinfo)->SetFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT);
                        }
                    }
                    else
                    {
                        sLog.outErrorDb("Quest %u has `ReqSpellCast%d` = %u and ReqCreatureOrGOId%d = 0 but spell %u does not have SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT effect for this quest, quest can't be done.",
                            qinfo->GetQuestId(),j+1,id,j+1,id);
                        // no changes, quest can't be done for this requirement
                    }
                }
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            int32 id = qinfo->ReqCreatureOrGOId[j];
            if(id < 0 && !sGOStorage.LookupEntry<GameObjectInfo>(-id))
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %i but gameobject %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(-id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id > 0 && !sCreatureStorage.LookupEntry<CreatureInfo>(id))
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %i but creature with entry %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id)
            {
                // In fact SpeakTo and Kill are quite same: either you can speak to mob:SpeakTo or you can't:Kill/Cast

                qinfo->SetFlag(QUEST_MANGOS_FLAGS_KILL_OR_CAST | QUEST_MANGOS_FLAGS_SPEAKTO);

                if(!qinfo->ReqCreatureOrGOCount[j])
                {
                    sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = %u but `ReqCreatureOrGOCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can be incorrectly done, but we already report this
                }
            }
            else if(qinfo->ReqCreatureOrGOCount[j]>0)
            {
                sLog.outErrorDb("Quest %u has `ReqCreatureOrGOId%d` = 0 but `ReqCreatureOrGOCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->ReqCreatureOrGOCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j )
        {
            uint32 id = qinfo->RewChoiceItemId[j];
            if(id)
            {
                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `RewChoiceItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->RewChoiceItemId[j] = 0;          // no changes, quest will not reward this
                }

                if(!qinfo->RewChoiceItemCount[j])
                {
                    sLog.outErrorDb("Quest %u has `RewChoiceItemId%d` = %u but `RewChoiceItemCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can't be done
                }
            }
            else if(qinfo->RewChoiceItemCount[j]>0)
            {
                sLog.outErrorDb("Quest %u has `RewChoiceItemId%d` = 0 but `RewChoiceItemCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewChoiceItemCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REWARDS_COUNT; ++j )
        {
            uint32 id = qinfo->RewItemId[j];
            if(id)
            {
                if(!sItemStorage.LookupEntry<ItemPrototype>(id))
                {
                    sLog.outErrorDb("Quest %u has `RewItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->RewItemId[j] = 0;                // no changes, quest will not reward this item
                }

                if(!qinfo->RewItemCount[j])
                {
                    sLog.outErrorDb("Quest %u has `RewItemId%d` = %u but `RewItemCount%d` = 0, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes
                }
            }
            else if(qinfo->RewItemCount[j]>0)
            {
                sLog.outErrorDb("Quest %u has `RewItemId%d` = 0 but `RewItemCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewItemCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        {
            if(qinfo->RewRepFaction[j])
            {
                if(!qinfo->RewRepValue[j])
                {
                    sLog.outErrorDb("Quest %u has `RewRepFaction%d` = %u but `RewRepValue%d` = 0, quest will not reward this reputation.",
                        qinfo->GetQuestId(),j+1,qinfo->RewRepValue[j],j+1);
                    // no changes
                }

                if(!sFactionStore.LookupEntry(qinfo->RewRepFaction[j]))
                {
                    sLog.outErrorDb("Quest %u has `RewRepFaction%d` = %u but raw faction (faction.dbc) %u does not exist, quest will not reward reputation for this faction.",
                        qinfo->GetQuestId(),j+1,qinfo->RewRepFaction[j] ,qinfo->RewRepFaction[j] );
                    qinfo->RewRepFaction[j] = 0;            // quest will not reward this
                }
            }
            else if(qinfo->RewRepValue[j]!=0)
            {
                sLog.outErrorDb("Quest %u has `RewRepFaction%d` = 0 but `RewRepValue%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewRepValue[j]);
                // no changes, quest ignore this data
            }
        }

        if(qinfo->RewSpell)
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(qinfo->RewSpell);

            if(!spellInfo)
            {
                sLog.outErrorDb("Quest %u has `RewSpell` = %u but spell %u does not exist, quest will not have a spell reward.",
                    qinfo->GetQuestId(),qinfo->RewSpell,qinfo->RewSpell);
                qinfo->RewSpell = 0;                        // quest will not reward this
            }

            else if(!IsSpellValid(spellInfo))
            {
                sLog.outErrorDb("Quest %u has `RewSpell` = %u but spell %u is broken, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->RewSpell,qinfo->RewSpell);
                qinfo->RewSpell = 0;                        // quest will not reward this
            }

        }

        if(qinfo->NextQuestInChain)
        {
            if(mQuestTemplates.find(qinfo->NextQuestInChain) == mQuestTemplates.end())
            {
                sLog.outErrorDb("Quest %u has `NextQuestInChain` = %u but quest %u does not exist, quest chain will not work.",
                    qinfo->GetQuestId(),qinfo->NextQuestInChain ,qinfo->NextQuestInChain );
                qinfo->NextQuestInChain = 0;
            }
            else
                mQuestTemplates[qinfo->NextQuestInChain]->prevChainQuests.push_back(qinfo->GetQuestId());
        }

        // fill additional data stores
        if(qinfo->PrevQuestId)
            qinfo->prevQuests.push_back(qinfo->PrevQuestId);

        if(qinfo->NextQuestId)
        {
            if (mQuestTemplates.find(abs(qinfo->GetNextQuestId())) == mQuestTemplates.end())
            {
                sLog.outErrorDb("Quest %d has NextQuestId %i, but no such quest", qinfo->GetQuestId(), qinfo->GetNextQuestId());
            }
            else
            {
                int32 signedQuestId = qinfo->NextQuestId < 0 ? -int32(qinfo->GetQuestId()) : int32(qinfo->GetQuestId());
                mQuestTemplates[abs(qinfo->GetNextQuestId())]->prevQuests.push_back(signedQuestId);
            }
        }

        if(qinfo->ExclusiveGroup)
            mExclusiveQuestGroups.insert(std::pair<int32, uint32>(qinfo->ExclusiveGroup, qinfo->GetQuestId()));
        if(qinfo->LimitTime)
            qinfo->SetFlag(QUEST_MANGOS_FLAGS_TIMED);
    }

    // check QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT for spell with SPELL_EFFECT_QUEST_COMPLETE
    for (uint32 i = 0; i < sSpellStore.GetNumRows(); ++i)
    {
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(i);
        if(!spellInfo)
            continue;

        for(int j = 0; j < 3; ++j)
        {
            if(spellInfo->Effect[j] != SPELL_EFFECT_QUEST_COMPLETE)
                continue;

            uint32 quest_id = spellInfo->EffectMiscValue[j];

            Quest const* quest = GetQuestTemplate(quest_id);

            // some quest referenced in spells not exist (outdataed spells)
            if(!quest)
                continue;

            if(!quest->HasFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT))
            {
                sLog.outErrorDb("Spell (id: %u) have SPELL_EFFECT_QUEST_COMPLETE for quest %u , but quest not have flag QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT. Quest flags must be fixed, quest modified to enable objective.",spellInfo->Id,quest_id);

                // this will prevent quest completing without objective
                const_cast<Quest*>(quest)->SetFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT);
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u quests definitions", mQuestTemplates.size() );
}

void ObjectMgr::LoadQuestLocales()
{
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,"
        "`Title_loc1`,`Details_loc1`,`Objectives_loc1`,`OfferRewardText_loc1`,`RequestItemsText_loc1`,`EndText_loc1`,`ObjectiveText1_loc1`,`ObjectiveText2_loc1`,`ObjectiveText3_loc1`,`ObjectiveText4_loc1`,"
        "`Title_loc2`,`Details_loc2`,`Objectives_loc2`,`OfferRewardText_loc2`,`RequestItemsText_loc2`,`EndText_loc2`,`ObjectiveText1_loc2`,`ObjectiveText2_loc2`,`ObjectiveText3_loc2`,`ObjectiveText4_loc2`,"
        "`Title_loc3`,`Details_loc3`,`Objectives_loc3`,`OfferRewardText_loc3`,`RequestItemsText_loc3`,`EndText_loc3`,`ObjectiveText1_loc3`,`ObjectiveText2_loc3`,`ObjectiveText3_loc3`,`ObjectiveText4_loc3`,"
        "`Title_loc4`,`Details_loc4`,`Objectives_loc4`,`OfferRewardText_loc4`,`RequestItemsText_loc4`,`EndText_loc4`,`ObjectiveText1_loc4`,`ObjectiveText2_loc4`,`ObjectiveText3_loc4`,`ObjectiveText4_loc4`,"
        "`Title_loc5`,`Details_loc5`,`Objectives_loc5`,`OfferRewardText_loc5`,`RequestItemsText_loc5`,`EndText_loc5`,`ObjectiveText1_loc5`,`ObjectiveText2_loc5`,`ObjectiveText3_loc5`,`ObjectiveText4_loc5`,"
        "`Title_loc6`,`Details_loc6`,`Objectives_loc6`,`OfferRewardText_loc6`,`RequestItemsText_loc6`,`EndText_loc6`,`ObjectiveText1_loc6`,`ObjectiveText2_loc6`,`ObjectiveText3_loc6`,`ObjectiveText4_loc6`,"
        "`Title_loc7`,`Details_loc7`,`Objectives_loc7`,`OfferRewardText_loc7`,`RequestItemsText_loc7`,`EndText_loc7`,`ObjectiveText1_loc7`,`ObjectiveText2_loc7`,`ObjectiveText3_loc7`,`ObjectiveText4_loc7`"
        " FROM `locales_quest`"
        );

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 Quest locale strings. DB table `locales_quest` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        QuestLocale& data = mQuestLocaleMap[entry];

        for(int i = 1; i < 8; ++i)
        {
            std::string str = fields[1+10*(i-1)].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Title.size() <= idx)
                        data.Title.resize(idx+1);

                    data.Title[idx] = str;
                }
            }
            str = fields[1+10*(i-1)+1].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Details.size() <= idx)
                        data.Details.resize(idx+1);

                    data.Details[idx] = str;
                }
            }
            str = fields[1+10*(i-1)+2].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Objectives.size() <= idx)
                        data.Objectives.resize(idx+1);

                    data.Objectives[idx] = str;
                }
            }
            str = fields[1+10*(i-1)+3].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.OfferRewardText.size() <= idx)
                        data.OfferRewardText.resize(idx+1);

                    data.OfferRewardText[idx] = str;
                }
            }
            str = fields[1+10*(i-1)+4].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.RequestItemsText.size() <= idx)
                        data.RequestItemsText.resize(idx+1);

                    data.RequestItemsText[idx] = str;
                }
            }
            str = fields[1+10*(i-1)+5].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.EndText.size() <= idx)
                        data.EndText.resize(idx+1);

                    data.EndText[idx] = str;
                }
            }
            for(int k = 0; k < 4; ++k)
            {
                str = fields[1+10*(i-1)+6+k].GetCppString();
                if(!str.empty())
                {
                    int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                    if(idx >= 0)
                    {
                        if(data.ObjectiveText[k].size() <= idx)
                            data.ObjectiveText[k].resize(idx+1);

                        data.ObjectiveText[k][idx] = str;
                    }
                }
            }
        }
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u Quest locale strings", mQuestLocaleMap.size() );
}

void ObjectMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `spell_id`, `prev_spell`, `first_spell`, `rank` FROM `spell_chain`");
    if(result == NULL)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell chain records" );
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` does not exist",spell_id);
            continue;
        }

        if(node.prev!=0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not existing previous rank spell: %u",spell_id,node.prev);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not existing first rank spell: %u",spell_id,node.first);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) ||
            (spell_id == node.first) != (node.prev == 0) ||
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` has not compatible chain data (prev: %u, first: %u, rank: %d)",spell_id,node.prev,node.first,node.rank);
            continue;
        }

        mSpellChains[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    // additional integrity checks
    for(SpellChainMap::iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::iterator i_prev = mSpellChains.find(i->second.prev);
            if(i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has not found previous rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i_prev->second.prev,i_prev->second.first,i_prev->second.rank);
            }
        }
    }

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell chain records", count );
}

void ObjectMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `SkillID`, `Value`, `MaxValue` FROM `spell_learn_skill`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn skills" );
        sLog.outErrorDb("`spell_learn_skill` table is empty!");
        return;
    }

    uint32 count = 0;

    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        int32 skill_val = fields[2].GetInt32();
        int32 skill_max = fields[3].GetInt32();

        SpellLearnSkillNode node;
        node.skill    = fields[1].GetUInt32();
        node.value    = skill_val < 0 ? maxconfskill : skill_val;
        node.maxvalue = skill_max < 0 ? maxconfskill : skill_max;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_skill` does not exist",spell_id);
            continue;
        }

        if(!sSkillLineStore.LookupEntry(node.skill))
        {
            sLog.outErrorDb("Skill %u listed in `spell_learn_skill` does not exist",node.skill);
            continue;
        }

        mSpellLearnSkills[spell_id] = node;

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.value    = 1;
                dbc_node.maxvalue = (entry->EffectBasePoints[i]+1)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                if(db_node)
                {
                    if(db_node->skill != dbc_node.skill)
                        sLog.outErrorDb("Spell %u auto-learn skill %u in spell.dbc but learn skill %u in `spell_learn_skill`, please fix DB.",
                            spell,dbc_node.skill,db_node->skill);

                    continue;                               // skip already added spell-skill pair
                }

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn skills + %u found in DBC", count, dbc_count );
}

void ObjectMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `SpellID`,`IfNoSpell` FROM `spell_learn_spell`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn spells" );
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.ifNoSpell  = fields[2].GetUInt32();
        node.autoLearned= false;

        if(!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        if(node.ifNoSpell && !sSpellStore.LookupEntry(node.ifNoSpell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",node.ifNoSpell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.nCount; ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < 3; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.ifNoSpell   = 0;
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if(itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if(!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count );
}

void ObjectMgr::LoadPetCreateSpells()
{
    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`, `Spell1`, `Spell2`, `Spell3`, `Spell4`,`FamilyPassive` FROM `petcreateinfo_spell`");
    if(!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 pet create spells" );
        sLog.outErrorDb("`petcreateinfo_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( result->GetRowCount() );

    mPetCreateSpell.clear();

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 creature_id = fields[0].GetUInt32();

        if(!creature_id || !sCreatureStorage.LookupEntry<CreatureInfo>(creature_id))
            continue;

        PetCreateSpellEntry PetCreateSpell;
        for(int i = 0; i < 4; i++)
        {
            PetCreateSpell.spellid[i] = fields[i + 1].GetUInt32();

            if(PetCreateSpell.spellid[i] && !sSpellStore.LookupEntry(PetCreateSpell.spellid[i]))
                sLog.outErrorDb("Spell %u listed in `petcreateinfo_spell` does not exist",PetCreateSpell.spellid[i]);
        }

        PetCreateSpell.familypassive = fields[5].GetUInt32();

        if(PetCreateSpell.familypassive && !sSpellStore.LookupEntry(PetCreateSpell.familypassive))
            sLog.outErrorDb("Spell %u listed in `petcreateinfo_spell` does not exist",PetCreateSpell.familypassive);

        mPetCreateSpell[creature_id] = PetCreateSpell;

        count++;
    }
    while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u pet create spells", count );
}

void ObjectMgr::LoadScripts(ScriptMapMap& scripts, char const* tablename)
{
    if(sWorld.IsScriptScheduled())                          // function don't must be called in time scripts use.
        return;

    sLog.outString( "%s :", tablename);

    scripts.clear();                                        // need for reload support

    QueryResult *result = WorldDatabase.PQuery( "SELECT `id`,`delay`,`command`,`datalong`,`datalong2`,`datatext`, `x`, `y`, `z`, `o` FROM `%s`", tablename );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u script definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        ScriptInfo tmp;
        tmp.id = fields[0].GetUInt32();
        tmp.delay = fields[1].GetUInt32();
        tmp.command = fields[2].GetUInt32();
        tmp.datalong = fields[3].GetUInt32();
        tmp.datalong2 = fields[4].GetUInt32();
        tmp.datatext = fields[5].GetString();
        tmp.x = fields[6].GetFloat();
        tmp.y = fields[7].GetFloat();
        tmp.z = fields[8].GetFloat();
        tmp.o = fields[9].GetFloat();

        // generic command args check
        switch(tmp.command)
        {
            case SCRIPT_COMMAND_TELEPORT_TO:
            {
                if(!sMapStore.LookupEntry(tmp.datalong))
                {
                    sLog.outErrorDb("Table `%s` has invalid map (Id: %u) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                if(!MaNGOS::IsValidMapCoord(tmp.x,tmp.y))
                {
                    sLog.outErrorDb("Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",tablename,tmp.x,tmp.y,tmp.id);
                    continue;
                }
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if(!MaNGOS::IsValidMapCoord(tmp.x,tmp.y))
                {
                    sLog.outErrorDb("Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",tablename,tmp.x,tmp.y,tmp.id);
                    continue;
                }

                if(!GetCreatureTemplate(tmp.datalong))
                {
                    sLog.outErrorDb("Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }
                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                GameObjectData const* data = GetGOData(tmp.datalong);
                if(!data)
                {
                    sLog.outErrorDb("Table `%s` has invalid gameobject (GUID: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                GameObjectInfo const* info = GetGameObjectInfo(data->id);
                if(!info)
                {
                    sLog.outErrorDb("Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,tmp.datalong,data->id,tmp.id);
                    continue;
                }

                if( info->type==GAMEOBJECT_TYPE_FISHINGNODE ||
                    info->type==GAMEOBJECT_TYPE_FISHINGNODE ||
                    info->type==GAMEOBJECT_TYPE_DOOR        ||
                    info->type==GAMEOBJECT_TYPE_BUTTON      ||
                    info->type==GAMEOBJECT_TYPE_TRAP )
                {
                    sLog.outErrorDb("Table `%s` have gameobject type (%u) unsupported by command SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,info->id,tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            {
                GameObjectData const* data = GetGOData(tmp.datalong);
                if(!data)
                {
                    sLog.outErrorDb("Table `%s` has invalid gameobject (GUID: %u) in SCRIPT_COMMAND_OPEN_DOOR for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                GameObjectInfo const* info = GetGameObjectInfo(data->id);
                if(!info)
                {
                    sLog.outErrorDb("Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in SCRIPT_COMMAND_OPEN_DOOR for script id %u",tablename,tmp.datalong,data->id,tmp.id);
                    continue;
                }

                if( info->type!=GAMEOBJECT_TYPE_DOOR)
                {
                    sLog.outErrorDb("Table `%s` has gameobject type (%u) non supported by command SCRIPT_COMMAND_OPEN_DOOR for script id %u",tablename,info->id,tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_QUEST_EXPLORED:
            {
                Quest const* quest = objmgr.GetQuestTemplate(tmp.datalong);
                if(!quest)
                {
                    sLog.outErrorDb("Table `%s` has invalid quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                if(!quest->HasFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT))
                {
                    sLog.outErrorDb("Table `%s` has quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, but quest not have flag QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT in quest flags. Script command or quest flags wrong. Quest modified to require objective.",tablename,tmp.datalong,tmp.id);

                    // this will prevent quest completing without objective
                    const_cast<Quest*>(quest)->SetFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT);

                    // continue; - quest objective requiremet set and command can be allowed
                }

                if(float(tmp.datalong2) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",tablename,tmp.datalong2,tmp.id);
                    continue;
                }

                if(tmp.datalong2 && float(tmp.datalong2) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, max distance is %u or 0 for disable distance check",tablename,tmp.datalong2,tmp.id,uint32(DEFAULT_VISIBILITY_DISTANCE));
                    continue;
                }

                if(tmp.datalong2 && float(tmp.datalong2) < INTERACTION_DISTANCE)
                {
                    sLog.outErrorDb("Table `%s` has too small distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, min distance is %u or 0 for disable distance check",tablename,tmp.datalong2,tmp.id,uint32(INTERACTION_DISTANCE));
                    continue;
                }

                break;
            }
        }

        if (scripts.find(tmp.id) == scripts.end())
        {
            ScriptMap emptyMap;
            scripts[tmp.id] = emptyMap;
        }
        scripts[tmp.id].insert(std::pair<uint32, ScriptInfo>(tmp.delay, tmp));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u script definitions", count );
}

void ObjectMgr::LoadButtonScripts()
{
    LoadScripts(sButtonScripts,    "button_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sButtonScripts.begin(); itr != sButtonScripts.end(); ++itr)
    {
        if(!GetGOData(itr->first))
            sLog.outErrorDb("Table `button_scripts` has not existing gameobject (GUID: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadQuestEndScripts()
{
    objmgr.LoadScripts(sQuestEndScripts,  "quest_end_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sQuestEndScripts.begin(); itr != sQuestEndScripts.end(); ++itr)
    {
        if(!GetQuestTemplate(itr->first))
            sLog.outErrorDb("Table `quest_end_scripts` has not existing quest (Id: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadQuestStartScripts()
{
    objmgr.LoadScripts(sQuestStartScripts,"quest_start_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sQuestStartScripts.begin(); itr != sQuestStartScripts.end(); ++itr)
    {
        if(!GetQuestTemplate(itr->first))
            sLog.outErrorDb("Table `quest_start_scripts` has not existing quest (Id: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadSpellScripts()
{
    objmgr.LoadScripts(sSpellScripts,     "spell_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sSpellScripts.begin(); itr != sSpellScripts.end(); ++itr)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(itr->first);

        if(!spellInfo)
        {
            sLog.outErrorDb("Table `spell_scripts` has not existing spell (Id: %u) as script id",itr->first);
            continue;
        }

        //check for correct spellEffect
        bool found = false;
        for(int i=0; i<3; ++i)
        {
            // skip empty effects
            if( !spellInfo->Effect[i] )
                continue;
            
            if( spellInfo->Effect[i] == SPELL_EFFECT_SEND_EVENT || spellInfo->Effect[i] == SPELL_EFFECT_SCRIPT_EFFECT )
            {
                found =  true;
                break;
            }
        }

        if(!found)
            sLog.outErrorDb("Table `spell_scripts` has unsupported spell (Id: %u) without SPELL_EFFECT_SCRIPT_EFFECT (%u) or SPELL_EFFECT_SEND_EVENT (%u) spell effect",itr->first,SPELL_EFFECT_SCRIPT_EFFECT,SPELL_EFFECT_SEND_EVENT);
    }
}

void ObjectMgr::LoadItemTexts()
{
    QueryResult *result = CharacterDatabase.PQuery("SELECT `id`, `text` FROM `item_text`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u item pages", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    Field* fields;
    do
    {
        bar.step();

        fields = result->Fetch();

        mItemTexts[ fields[0].GetUInt32() ] = fields[1].GetCppString();

        ++count;

    } while ( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u item texts", count );
}

void ObjectMgr::LoadPageTexts()
{
    sPageTextStore.Load ();
    sLog.outString( ">> Loaded %u page texts", sPageTextStore.RecordCount );
    sLog.outString();

    // check data correctness
    for(uint32 i = 1; i < sPageTextStore.MaxEntry; ++i)
    {
        PageText const* page = sPageTextStore.LookupEntry<PageText>(i);
        if(!page)
            continue;

        if(page->Next_Page && !sPageTextStore.LookupEntry<PageText>(page->Next_Page))
            sLog.outErrorDb("Paget text (Id: %u) has not existing next page (Id:%u)", i,page->Next_Page);
    }
}

void ObjectMgr::LoadPageTextLocales()
{
    QueryResult *result = WorldDatabase.PQuery("SELECT `entry`,`text_loc1`,`text_loc2`,`text_loc3`,`text_loc4`,`text_loc5`,`text_loc6`,`text_loc7` FROM `locales_page_text`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 PageText locale strings. DB table `locales_page_text` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        PageTextLocale& data = mPageTextLocaleMap[entry];

        for(int i = 1; i < 8; ++i)
        {
            std::string str = fields[i].GetCppString();
            if(str.empty())
                continue;

            int idx = GetOrNewIndexForLocale(LocaleConstant(i));
            if(idx >= 0)
            {
                if(data.Text.size() <= idx)
                    data.Text.resize(idx+1);

                data.Text[idx] = str;
            }
        }

    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u PageText locale strings", mPageTextLocaleMap.size() );
}

void ObjectMgr::LoadInstanceTemplate()
{
    sInstanceTemplate.Load();

    sLog.outString( ">> Loaded %u Instance Template definitions", sInstanceTemplate.RecordCount );
    sLog.outString();
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
    QueryResult *result = WorldDatabase.Query( "SELECT * FROM `npc_text`" );

    int count = 0;
    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u npc texts", count );
        return;
    }

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

    sLog.outString();
    sLog.outString( ">> Loaded %u npc texts", count );
    delete result;
}

void ObjectMgr::LoadNpcTextLocales()
{
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,"
        "`Text0_0_loc1`,`Text0_1_loc1`,`Text1_0_loc1`,`Text1_1_loc1`,`Text2_0_loc1`,`Text2_1_loc1`,`Text3_0_loc1`,`Text3_1_loc1`,`Text4_0_loc1`,`Text4_1_loc1`,`Text5_0_loc1`,`Text5_1_loc1`,`Text6_0_loc1`,`Text6_1_loc1`,`Text7_0_loc1`,`Text7_1_loc1`,"
        "`Text0_0_loc2`,`Text0_1_loc2`,`Text1_0_loc2`,`Text1_1_loc2`,`Text2_0_loc2`,`Text2_1_loc2`,`Text3_0_loc2`,`Text3_1_loc1`,`Text4_0_loc2`,`Text4_1_loc2`,`Text5_0_loc2`,`Text5_1_loc2`,`Text6_0_loc2`,`Text6_1_loc2`,`Text7_0_loc2`,`Text7_1_loc2`,"
        "`Text0_0_loc3`,`Text0_1_loc3`,`Text1_0_loc3`,`Text1_1_loc3`,`Text2_0_loc3`,`Text2_1_loc3`,`Text3_0_loc3`,`Text3_1_loc1`,`Text4_0_loc3`,`Text4_1_loc3`,`Text5_0_loc3`,`Text5_1_loc3`,`Text6_0_loc3`,`Text6_1_loc3`,`Text7_0_loc3`,`Text7_1_loc3`,"
        "`Text0_0_loc4`,`Text0_1_loc4`,`Text1_0_loc4`,`Text1_1_loc4`,`Text2_0_loc4`,`Text2_1_loc4`,`Text3_0_loc4`,`Text3_1_loc1`,`Text4_0_loc4`,`Text4_1_loc4`,`Text5_0_loc4`,`Text5_1_loc4`,`Text6_0_loc4`,`Text6_1_loc4`,`Text7_0_loc4`,`Text7_1_loc4`,"
        "`Text0_0_loc5`,`Text0_1_loc5`,`Text1_0_loc5`,`Text1_1_loc5`,`Text2_0_loc5`,`Text2_1_loc5`,`Text3_0_loc5`,`Text3_1_loc1`,`Text4_0_loc5`,`Text4_1_loc5`,`Text5_0_loc5`,`Text5_1_loc5`,`Text6_0_loc5`,`Text6_1_loc5`,`Text7_0_loc5`,`Text7_1_loc5`,"
        "`Text0_0_loc6`,`Text0_1_loc6`,`Text1_0_loc6`,`Text1_1_loc6`,`Text2_0_loc6`,`Text2_1_loc6`,`Text3_0_loc6`,`Text3_1_loc1`,`Text4_0_loc6`,`Text4_1_loc6`,`Text5_0_loc6`,`Text5_1_loc6`,`Text6_0_loc6`,`Text6_1_loc6`,`Text7_0_loc6`,`Text7_1_loc6`,"
        "`Text0_0_loc7`,`Text0_1_loc7`,`Text1_0_loc7`,`Text1_1_loc7`,`Text2_0_loc7`,`Text2_1_loc7`,`Text3_0_loc7`,`Text3_1_loc1`,`Text4_0_loc7`,`Text4_1_loc7`,`Text5_0_loc7`,`Text5_1_loc7`,`Text6_0_loc7`,`Text6_1_loc7`,`Text7_0_loc7`,`Text7_1_loc7` "
        " FROM `locales_npc_text`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 Quest locale strings. DB table `locales_npc_text` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        NpcTextLocale& data = mNpcTextLocaleMap[entry];

        for(int i=1; i<8; ++i)
        {
            for(int j=0; j<8; ++j)
            {
                std::string str0 = fields[1+8*2*(i-1)+2*j].GetCppString();
                if(!str0.empty())
                {
                    int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                    if(idx >= 0)
                    {
                        if(data.Text_0[j].size() <= idx)
                            data.Text_0[j].resize(idx+1);

                        data.Text_0[j][idx] = str0;
                    }
                }
                std::string str1 = fields[1+8*2*(i-1)+2*j+1].GetCppString();
                if(!str1.empty())
                {
                    int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                    if(idx >= 0)
                    {
                        if(data.Text_1[j].size() <= idx)
                            data.Text_1[j].resize(idx+1);

                        data.Text_1[j][idx] = str1;
                    }
                }
            }
        }
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u NpcText locale strings", mNpcTextLocaleMap.size() );
}

//not very fast function but it is called only once a day, or on starting-up
void ObjectMgr::ReturnOrDeleteOldMails(bool serverUp)
{
    time_t basetime = time(NULL);
    sLog.outDebug("Returning mails current time: hour: %d, minute: %d, second: %d ", localtime(&basetime)->tm_hour, localtime(&basetime)->tm_min, localtime(&basetime)->tm_sec);
    //delete all old mails without item and without body immediately, if starting server
    if (!serverUp)
        CharacterDatabase.PExecute("DELETE FROM `mail` WHERE `expire_time` < '" I64FMTD "' AND `has_items` = '0' AND `itemTextId` = 0", (uint64)basetime);
    QueryResult* result = CharacterDatabase.PQuery("SELECT `id`,`messageType`,`sender`,`receiver`,`itemTextId`,`has_items`,`expire_time`,`cod`,`checked` FROM `mail` WHERE `expire_time` < '" I64FMTD "'", (uint64)basetime);
    if ( !result )
        return;                                             // any mails need to be returned or deleted
    Field *fields;
    //std::ostringstream delitems, delmails; //will be here for optimization
    //bool deletemail = false, deleteitem = false;
    //delitems << "DELETE FROM `item_instance` WHERE `guid` IN ( ";
    //delmails << "DELETE FROM `mail` WHERE `id` IN ( "
    do
    {
        fields = result->Fetch();
        Mail *m = new Mail;
        m->messageID = fields[0].GetUInt32();
        m->messageType = fields[1].GetUInt8();
        m->sender = fields[2].GetUInt32();
        m->receiver = fields[3].GetUInt32();
        m->itemTextId = fields[4].GetUInt32();
        bool has_items = fields[5].GetBool();
        m->expire_time = (time_t)fields[6].GetUInt64();
        m->deliver_time = 0;
        m->COD = fields[7].GetUInt32();
        m->checked = fields[8].GetUInt32();
        Player *pl = 0;
        if (serverUp)
            pl = objmgr.GetPlayer((uint64)m->receiver);
        if (pl && pl->m_mailsLoaded)
        {                                                   //this code will run very improbably (the time is between 4 and 5 am, in game is online a player, who has old mail
            //his in mailbox and he has already listed his mails )
            delete m;
            continue;
        }
        //delete or return mail:
        if (has_items)
        {
            QueryResult *resultItems = CharacterDatabase.PQuery("SELECT `item_guid`,`item_template` FROM `mail_items` WHERE `mail_id`='%u'", m->messageID);
            if(resultItems)
            {
                do
                {
                    Field *fields2 = resultItems->Fetch();

                    uint32 item_guid = fields2[0].GetUInt32();
                    uint32 item_template = fields2[1].GetUInt32();

                    m->AddItem(item_guid, item_template);
                }
                while (resultItems->NextRow());

                delete resultItems;
            }

            if (m->checked < 4)
            {
                //mail will be returned:
                CharacterDatabase.PExecute("UPDATE `mail` SET `sender` = '%u', `receiver` = '%u', `expire_time` = '" I64FMTD "', `deliver_time` = '" I64FMTD "',`cod` = '0', `checked` = '16' WHERE `id` = '%u'", m->receiver, m->sender, (uint64)(basetime + 30*DAY), (uint64)basetime, m->messageID);
                delete m;
                continue;
            }
            else
            {
                //deleteitem = true;
                //delitems << m->item_guid << ", ";
                for(std::vector<MailItemInfo>::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                    CharacterDatabase.PExecute("DELETE FROM `item_instance` WHERE `guid` = '%u'", itr2->item_guid);
            }
        }
        if (m->itemTextId)
        {
            CharacterDatabase.PExecute("DELETE FROM `item_text` WHERE `id` = '%u'", m->itemTextId);
        }
        //deletemail = true;
        //delmails << m->messageID << ", ";
        CharacterDatabase.PExecute("DELETE FROM `mail` WHERE `id` = '%u'", m->messageID);
        delete m;
    } while (result->NextRow());
    delete result;
}

void ObjectMgr::LoadQuestAreaTriggers()
{
    mQuestAreaTriggerMap.clear();                           // need for reload case

    QueryResult *result = WorldDatabase.Query( "SELECT `id`,`quest` FROM `areatrigger_involvedrelation`" );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u quest trigger points", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 trigger_ID = fields[0].GetUInt32();
        uint32 quest_ID   = fields[1].GetUInt32();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(trigger_ID);
        if(!atEntry)
        {
            sLog.outErrorDb("Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",trigger_ID);
            continue;
        }

        Quest const* quest = GetQuestTemplate(quest_ID);

        if(!quest)
        {
            sLog.outErrorDb("Table `areatrigger_involvedrelation` has record (id: %u) for not existing quest %u",trigger_ID,quest_ID);
            continue;
        }

        if(!quest->HasFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT))
        {
            sLog.outErrorDb("Table `areatrigger_involvedrelation` has record (id: %u) for not quest %u, but quest not have flag QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT. Trigger or quest flags must be fixed, quest modified to require objective.",trigger_ID,quest_ID);

            // this will prevent quest completing without objective
            const_cast<Quest*>(quest)->SetFlag(QUEST_MANGOS_FLAGS_EXPLORATION_OR_EVENT);

            // continue; - quest modified to required obkective and trigger can be allowed.
        }

        mQuestAreaTriggerMap[trigger_ID] = quest_ID;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u quest trigger points", count );
}

void ObjectMgr::LoadTavernAreaTriggers()
{
    mTavernAreaTriggerSet.clear();                          // need for reload case

    QueryResult *result = WorldDatabase.Query("SELECT `id` FROM `areatrigger_tavern`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u tavern triggers", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 Trigger_ID      = fields[0].GetUInt32();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if(!atEntry)
        {
            sLog.outErrorDb("Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",Trigger_ID);
            continue;
        }

        mTavernAreaTriggerSet.insert(Trigger_ID);
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u tavern triggers", count );
}

uint32 ObjectMgr::GetNearestTaxiNode( float x, float y, float z, uint32 mapid )
{
    bool found = false;
    float dist;
    uint32 id = 0;

    for(uint32 i = 1; i < sTaxiNodesStore.nCount; ++i)
    {
        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
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

uint16 ObjectMgr::GetTaxiMount( uint32 id, uint32 team )
{
    uint16 mount_id = 0;

    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
    if(node)
    {
        if (team == ALLIANCE)
        {
            CreatureInfo const *ci = objmgr.GetCreatureTemplate(node->alliance_mount_type);
            if(ci)
                mount_id = ci->DisplayID_A;
        }
        if (team == HORDE)
        {
            CreatureInfo const *ci = objmgr.GetCreatureTemplate(node->horde_mount_type);
            if(ci)
                mount_id = ci->DisplayID_H;
        }
    }
    CreatureModelInfo const *minfo = objmgr.GetCreatureModelInfo(mount_id);
    if(!minfo)
    {
        sLog.outErrorDb("Creature (Entry: %u) has model %u not found in table `creature_model_info`, can't load. ",id,mount_id);
        return false;
    }
    if(minfo->modelid_other_gender!=0)
        mount_id = urand(0,1) ? mount_id : minfo->modelid_other_gender;

    return mount_id;
}

void ObjectMgr::GetTaxiPathNodes( uint32 path, Path &pathnodes )
{
    if(path >= sTaxiPathNodesByPath.size())
        return;

    TaxiPathNodeList& nodeList = sTaxiPathNodesByPath[path];

    pathnodes.Resize(nodeList.size());

    for(size_t i = 0; i < nodeList.size(); ++i)
    {
        pathnodes[ i ].x = nodeList[i].x;
        pathnodes[ i ].y = nodeList[i].y;
        pathnodes[ i ].z = nodeList[i].z;
    }
}

void ObjectMgr::GetTransportPathNodes( uint32 path, TransportPath &pathnodes )
{
    if(path >= sTaxiPathNodesByPath.size())
        return;

    TaxiPathNodeList& nodeList = sTaxiPathNodesByPath[path];

    pathnodes.Resize(nodeList.size());

    for(size_t i = 0; i < nodeList.size(); ++i)
    {
        pathnodes[ i ].mapid = nodeList[i].mapid;
        pathnodes[ i ].x = nodeList[i].x;
        pathnodes[ i ].y = nodeList[i].y;
        pathnodes[ i ].z = nodeList[i].z;
        pathnodes[ i ].actionFlag = nodeList[i].actionFlag;
        pathnodes[ i ].delay = nodeList[i].delay;
    }
}

void ObjectMgr::LoadGraveyardZones()
{
    mGraveYardMap.clear();                                  // need for reload case

    QueryResult *result = WorldDatabase.Query("SELECT `id`,`ghost_zone`,`faction` FROM `game_graveyard_zone`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u graveyard-zone links", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 safeLocId = fields[0].GetUInt32();
        uint32 zoneId = fields[1].GetUInt32();
        uint32 team   = fields[2].GetUInt32();

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(safeLocId);
        if(!entry)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.",safeLocId);
            continue;
        }

        AreaTableEntry const *areaEntry = GetAreaEntryByAreaID(zoneId);
        if(!areaEntry)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record for not existing zone id (%u), skipped.",zoneId);
            continue;
        }

        if(areaEntry->zone != 0)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record subzone id (%u) instead of zone, skipped.",zoneId);
            continue;
        }

        if(team!=0 && team!=HORDE && team!=ALLIANCE)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record for non player faction (%u), skipped.",team);
            continue;
        }

        if(entry->map_id != areaEntry->mapid && team != 0)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record for ghost zone (%u) at map %u and graveyard (%u) at map %u for team %u, but in case maps are different, player faction setting is ignored. Use faction 0 instead.",zoneId,areaEntry->mapid, safeLocId, entry->map_id, team);
            team = 0;
        }

        if(!AddGraveYardLink(safeLocId,zoneId,team,false))
            sLog.outErrorDb("Table `game_graveyard_zone` has a duplicate record for Garveyard (ID: %u) and Zone (ID: %u), skipped.",safeLocId,zoneId);
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u graveyard-zone links", count );
}

WorldSafeLocsEntry const *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team)
{
    // search for zone associated closest graveyard
    uint32 zoneId = MapManager::Instance().GetZoneId(MapId,x,y);

    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone,ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check `faction`
    //   if mapId != graveyard.mapId (ghost in instance) and search ANY graveyard associated
    //     then skip check `faction`
    GraveYardMap::const_iterator graveLow  = mGraveYardMap.lower_bound(zoneId);
    GraveYardMap::const_iterator graveUp   = mGraveYardMap.upper_bound(zoneId);
    if(graveLow==graveUp)
    {
        sLog.outErrorDb("Table `game_graveyard_zone` incomplete: Zone %u Team %u does not have a linked graveyard.",zoneId,team);
        return NULL;
    }

    bool foundNear = false;
    float distNear;
    WorldSafeLocsEntry const* entryNear = NULL;
    WorldSafeLocsEntry const* entryFar = NULL;

    for(GraveYardMap::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        GraveYardData const& data = itr->second;

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(data.safeLocId);
        if(!entry)
        {
            sLog.outErrorDb("Table `game_graveyard_zone` has record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.",data.safeLocId);
            continue;
        }

        // remember first graveyard at another map and ignore other
        if(MapId != entry->map_id)
        {
            if(!entryFar)
                entryFar = entry;
            continue;
        }

        // skip enemy faction graveyard at same map (normal area, city, or battleground)
        // team == 0 case can be at call from .neargrave
        if(data.team != 0 && team != 0 && data.team != team)
            continue;

        // find now nearest graveyard at same map
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
    }

    if(entryNear)
        return entryNear;

    return entryFar;
}

GraveYardData const* ObjectMgr::FindGraveYardData(uint32 id, uint32 zoneId)
{
    GraveYardMap::const_iterator graveLow  = mGraveYardMap.lower_bound(zoneId);
    GraveYardMap::const_iterator graveUp   = mGraveYardMap.upper_bound(zoneId);

    for(GraveYardMap::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        if(itr->second.safeLocId==id)
            return &itr->second;
    }

    return NULL;
}

bool ObjectMgr::AddGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool inDB)
{

    if(FindGraveYardData(id,zoneId))
        return false;

    // add link to loaded data
    GraveYardData data;
    data.safeLocId = id;
    data.team = team;

    mGraveYardMap.insert(GraveYardMap::value_type(zoneId,data));

    // add link to DB
    if(inDB)
    {
        WorldDatabase.PExecuteLog("INSERT INTO `game_graveyard_zone` ( `id`,`ghost_zone`,`faction`) "
            "VALUES ('%u', '%u','%u')",id,zoneId,team);
    }

    return true;
}

void ObjectMgr::LoadAreaTriggerTeleports()
{
    mAreaTriggers.clear();                                  // need for reload case

    uint32 count = 0;

    //                                            0    1                2               3            4                   5                   6                    7
    QueryResult *result = WorldDatabase.Query("SELECT `id`,`required_level`,`required_item`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `areatrigger_teleport`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u area trigger teleport definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        uint32 Trigger_ID = fields[0].GetUInt32();

        AreaTrigger at;

        at.requiredLevel      = fields[1].GetUInt8();
        at.requiredItem       = fields[2].GetUInt32();
        at.target_mapId       = fields[3].GetUInt32();
        at.target_X           = fields[4].GetFloat();
        at.target_Y           = fields[5].GetFloat();
        at.target_Z           = fields[6].GetFloat();
        at.target_Orientation = fields[7].GetFloat();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if(!atEntry)
        {
            sLog.outErrorDb("Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",Trigger_ID);
            continue;
        }

        if(at.requiredItem)
        {
            ItemPrototype const *pProto = objmgr.GetItemPrototype(at.requiredItem);
            if(!pProto)
            {
                sLog.outError("Key item %u does not exist for trigger %u, removing key requirement.", at.requiredItem, Trigger_ID);
                at.requiredItem = 0;
            }
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(at.target_mapId);
        if(!mapEntry)
        {
            sLog.outErrorDb("Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Trigger_ID,at.target_mapId);
            continue;
        }

        if(at.target_X==0 && at.target_Y==0 && at.target_Z==0)
        {
            sLog.outErrorDb("Area trigger (ID:%u) target coordinates not provided.",Trigger_ID);
            continue;
        }

        mAreaTriggers[Trigger_ID] = at;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u area trigger teleport definitions", count );
}

void ObjectMgr::LoadSpellTeleports()
{
    mSpellTeleports.clear();                                // need for reload case

    uint32 count = 0;

    //                                            0    1            2                   3                   4                   5
    QueryResult *result = WorldDatabase.Query("SELECT `id`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation` FROM `spell_teleport`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell teleport coordinates", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        count++;

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTeleport st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(Spell_ID);
        if(!spellInfo)
        {
            sLog.outErrorDb("Spell (ID:%u) listed in `spell_teleport` does not exist`.",Spell_ID);
            continue;
        }

        bool found = false;
        for(int i = 0; i < 3; ++i)
        {
            if( spellInfo->Effect[i]==SPELL_EFFECT_TELEPORT_UNITS )
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            sLog.outErrorDb("Spell (Id: %u) listed in `spell_teleport` does not have effect SPELL_EFFECT_TELEPORT_UNITS (5).",Spell_ID);
            continue;
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if(!mapEntry)
        {
            sLog.outErrorDb("Spell (ID:%u) teleport target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if(st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outErrorDb("Spell (ID:%u) teleport target coordinates not provided.",Spell_ID);
            continue;
        }

        mSpellTeleports[Spell_ID] = st;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell teleport coordinates", count );
}

void ObjectMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                            0        1          2         3            4          5         6             7                 8
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`effectId`,`SpellId`,`SchoolMask`,`Category`,`SkillID`,`SpellFamily`,`SpellFamilyMask`,`Charges` FROM `spell_affect`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell affect definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if( spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER )
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        SpellAffection sa;

        sa.SpellId = fields[2].GetUInt16();
        sa.SchoolMask = fields[3].GetUInt8();
        sa.Category = fields[4].GetUInt16();
        sa.SkillId = fields[5].GetUInt16();
        sa.SpellFamily = fields[6].GetUInt8();
        sa.SpellFamilyMask = fields[7].GetUInt64();
        sa.Charges = fields[8].GetUInt16();

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,sa));

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell affect definitions", count );
}

bool ObjectMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 const& familyFlags)
{
    if (!spellInfo)
        return false;

    SpellAffection const *spellAffect = GetSpellAffection(spellId,effectId);

    if (spellAffect )
    {
        if (spellAffect->SpellId && (spellAffect->SpellId == spellInfo->Id))
            return true;
        if (spellAffect->SchoolMask && (spellAffect->SchoolMask & (1 << spellInfo->School)))
            return true;
        if (spellAffect->Category && (spellAffect->Category == spellInfo->Category))
            return true;
        if (spellAffect->SkillId)
        {
            SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(spellInfo->Id);
            if(skillLineEntry && skillLineEntry->skillId == spellAffect->SkillId)
                return true;
        }
        if (spellAffect->SpellFamily && spellAffect->SpellFamily == spellInfo->SpellFamilyName)
            return true;

        if (spellAffect->SpellFamilyMask && (spellAffect->SpellFamilyMask & spellInfo->SpellFamilyFlags))
            return true;
    }
    else if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    return false;
}

void ObjectMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                            0       1            2          3         4          5      6                 7           8
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`SchoolMask`,`Category`,`SkillID`,`SpellFamilyName`,`SpellFamilyMask`,`procFlags`,`ppmRate` FROM `spell_proc_event`");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell proc event conditions", count  );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();

        if (!sSpellStore.LookupEntry(entry))
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.category        = fields[2].GetUInt32();
        spe.skillId         = fields[3].GetUInt32();
        spe.spellFamilyName = fields[4].GetUInt32();
        spe.spellFamilyMask = fields[5].GetUInt64();
        spe.procFlags       = fields[6].GetUInt32();
        spe.ppmRate         = fields[7].GetFloat();

        mSpellProcEventMap[entry] = spe;

        count++;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell proc event conditions", count  );
}

bool ObjectMgr::IsSpellProcEventCanTriggeredBy( SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags )
{
    if((procFlags & spellProcEvent->procFlags) == 0)
        return false;

    // Additional checks in case spell cast/hit/crit is the event
    // Check (if set) school, category, skill line, spell talent mask
    if(spellProcEvent->schoolMask && (!procSpell || !procSpell->School || ((1<<procSpell->School) & spellProcEvent->schoolMask) == 0))
        return false;
    if(spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
        return false;
    if(spellProcEvent->skillId)
    {
        if (!procSpell) return false;
        SkillLineAbilityEntry const *skillLineEntry = sSkillLineAbilityStore.LookupEntry(procSpell->Id);
        if(!skillLineEntry || skillLineEntry->skillId != spellProcEvent->skillId)
            return false;
    }
    if(spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
        return false;
    if(spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
        return false;

    return true;
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult *result = CharacterDatabase.Query( "SELECT MAX(`guid`) FROM `character`" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = WorldDatabase.Query( "SELECT MAX(`guid`) FROM `creature`" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = CharacterDatabase.Query( "SELECT MAX(`guid`) FROM `item_instance`" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(`guid`) FROM `gameobject`" );
    if( result )
    {
        m_hiGoGuid = (*result)[0].GetUInt32()+1;

        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(`id`) FROM `auctionhouse`" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_auctionid = 0;
    }
    result = CharacterDatabase.Query( "SELECT MAX(`id`) FROM `mail`" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;

        delete result;
    }
    else
    {
        m_mailid = 0;
    }
    result = CharacterDatabase.Query( "SELECT MAX(`id`) FROM `item_text`" );
    if( result )
    {
        m_ItemTextId = (*result)[0].GetUInt32();

        delete result;
    }
    else
        m_ItemTextId = 0;

    result = CharacterDatabase.Query( "SELECT MAX(`guid`) FROM `corpse`" );
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

uint32 ObjectMgr::GenerateItemTextID()
{
    return ++m_ItemTextId;
}

uint32 ObjectMgr::CreateItemText(std::string text)
{
    uint32 newItemTextId = GenerateItemTextID();
    //insert new itempage to container
    mItemTexts[ newItemTextId ] = text;
    //save new itempage
    CharacterDatabase.escape_string(text);
    //any Delete query needed, itemTextId is maximum of all ids
    std::ostringstream query;
    query << "INSERT INTO `item_text` (`id`,`text`) VALUES ( '" << newItemTextId << "', '" << text << "')";
    CharacterDatabase.Execute(query.str().c_str());         //needs to be run this way, because mail body may be more than 1024 characters
    return newItemTextId;
}

uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
    switch(guidhigh)
    {
        case HIGHGUID_ITEM:
            ++m_hiItemGuid;
            if(m_hiItemGuid==0xFFFFFFFF)
            {
                sLog.outError("Item guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiItemGuid;
        case HIGHGUID_UNIT:
            ++m_hiCreatureGuid;
            if(m_hiCreatureGuid==0xFFFFFFFF)
            {
                sLog.outError("Creature guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiCreatureGuid;
        case HIGHGUID_PLAYER:
            ++m_hiCharGuid;
            if(m_hiCharGuid==0xFFFFFFFF)
            {
                sLog.outError("Players guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiCharGuid;
        case HIGHGUID_GAMEOBJECT:
            ++m_hiGoGuid;
            if(m_hiGoGuid==0xFFFFFFFF)
            {
                sLog.outError("Gameobject guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiGoGuid;
        case HIGHGUID_CORPSE:
            ++m_hiCorpseGuid;
            if(m_hiCorpseGuid==0xFFFFFFFF)
            {
                sLog.outError("Corpse guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiCorpseGuid;
        case HIGHGUID_DYNAMICOBJECT:
            ++m_hiDoGuid;
            if(m_hiDoGuid==0xFFFFFFFF)
            {
                sLog.outError("DynamicObject guid overflow!! Can't continue, shuting down server. ");
                sWorld.m_stopEvent = true;
            }
            return m_hiDoGuid;
        default:
            ASSERT(0);
    }

    ASSERT(0);
    return 0;
}

void ObjectMgr::LoadGameObjectLocales()
{
    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`name_loc1`,`name_loc2`,`name_loc3`,`name_loc4`,`name_loc5`,`name_loc6`,`name_loc7` FROM `locales_gameobject`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString("");
        sLog.outString(">> Loaded 0 gameobject locale strings. DB table `locales_gameobject` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 entry = fields[0].GetUInt32();

        GameObjectLocale& data = mGameObjectLocaleMap[entry];

        for(int i = 1; i < 8; ++i)
        {
            std::string str = fields[i].GetCppString();
            if(!str.empty())
            {
                int idx = GetOrNewIndexForLocale(LocaleConstant(i));
                if(idx >= 0)
                {
                    if(data.Name.size() <= idx)
                        data.Name.resize(idx+1);

                    data.Name[idx] = str;
                }
            }
        }
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u gameobject locale strings", mGameObjectLocaleMap.size() );
}

void ObjectMgr::LoadGameobjectInfo()
{
    sGOStorage.Load();

    sLog.outString( ">> Loaded %u game object templates", sGOStorage.RecordCount );
    sLog.outString();
}

void ObjectMgr::LoadExplorationBaseXP()
{
    uint32 count = 0;
    QueryResult *result = WorldDatabase.Query("SELECT `level`,`basexp` FROM `exploration_basexp`");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u BaseXP definitions", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        uint32 level  = fields[0].GetUInt32();
        uint32 basexp = fields[1].GetUInt32();
        mBaseXPTable[level] = basexp;
        ++count;
    }
    while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u BaseXP definitions", count );
}

uint32 ObjectMgr::GetBaseXP(uint32 level)
{
    return mBaseXPTable[level] ? mBaseXPTable[level] : 0;
}

void ObjectMgr::LoadPetNames()
{
    uint32 count = 0;
    QueryResult *result = WorldDatabase.Query("SELECT `word`,`entry`,`half` FROM `pet_name_generation`");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u pet name parts", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        bar.step();

        Field *fields = result->Fetch();
        std::string word = fields[0].GetString();
        uint32 entry     = fields[1].GetUInt32();
        bool   half      = fields[2].GetBool();
        if(half)
            PetHalfName1[entry].push_back(word);
        else
            PetHalfName0[entry].push_back(word);
        count++;
    }
    while (result->NextRow());
    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u pet name parts", count );
}

void ObjectMgr::LoadPetNumber()
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(`id`) FROM `character_pet`");
    if(result)
    {
        Field *fields = result->Fetch();
        m_hiPetNumber = fields[0].GetUInt32()+1;
        delete result;
    }

    barGoLink bar( 1 );
    bar.step();

    sLog.outString();
    sLog.outString( ">> Loaded the max pet number: %d", m_hiPetNumber-1);
}

std::string ObjectMgr::GeneratePetName(uint32 entry)
{
    std::vector<std::string> & list0 = PetHalfName0[entry];
    std::vector<std::string> & list1 = PetHalfName1[entry];

    if(list0.empty() || list1.empty())
    {
        CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(entry);
        char* petname = GetPetName(cinfo->family, sWorld.GetDBClang());
        if(!petname)
            petname = cinfo->Name;
        return std::string(petname);
    }

    return *(list0.begin()+urand(0, list0.size()-1)) + *(list1.begin()+urand(0, list1.size()-1));
}

uint32 ObjectMgr::GeneratePetNumber()
{
    return ++m_hiPetNumber;
}

bool ObjectMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2)
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool ObjectMgr::canStackSpellRanks(SpellEntry const *spellInfo,SpellEntry const *spellInfo2)
{
    // Riding not listed in spellbook and not stacked but have ranks and don't must replace ranks at learning
    if(spellInfo->Attributes == 0x10000D0)
        return true;

    if(spellInfo->powerType == 0)
    {
        if(spellInfo->manaCost > 0 && spellInfo->manaCost != spellInfo2->manaCost)
            return true;
        if(spellInfo->ManaCostPercentage > 0 && spellInfo->ManaCostPercentage != spellInfo2->ManaCostPercentage)
            return true;
        if(spellInfo->manaCostPerlevel > 0 && spellInfo->manaCostPerlevel != spellInfo2->manaCostPerlevel)
            return true;
        if(spellInfo->manaPerSecond > 0 && spellInfo->manaPerSecond != spellInfo2->manaPerSecond)
            return true;
        if(spellInfo->manaPerSecondPerLevel > 0 && spellInfo->manaPerSecondPerLevel != spellInfo2->manaPerSecondPerLevel)
            return true;
        if(spellInfo->Reagent[0] > 0 && spellInfo->Reagent[0] != spellInfo2->Reagent[0])
            return true;
    }
    return false;
}

void ObjectMgr::LoadCorpses()
{
    uint32 count = 0;
    //                                             0            1            2            3             4     5      6            7          8        9
    QueryResult *result = CharacterDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map`,`data`,`bones_flag`,`instance`,`player`,`guid` FROM `corpse` WHERE `bones_flag` = 0");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u corpses", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    Corpse *corpse;
    do
    {
        bar.step();

        Field *fields = result->Fetch();

        uint32 guid = fields[result->GetFieldCount()-1].GetUInt32();

        corpse = new Corpse(NULL);
        if(!corpse->LoadFromDB(guid,fields))
        {
            delete corpse;
            continue;
        }

        ObjectAccessor::Instance().AddCorpse(corpse);

        count++;
    }
    while (result->NextRow());
    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u corpses", count );
}

bool ObjectMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2)
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);

    if(!spellInfo_1 || !spellInfo_2)
        return false;

    if(spellInfo_1->Id == spellId_2)
        return false;

    //I think we don't check this correctly because i need a exception for spell:
    //72,11327,18461...(called from 1856,1857...) Call Aura 16,31, after trigger another spell who call aura 77 and 77 remove 16 and 31, this should not happen.
    if(spellInfo_2->SpellFamilyFlags == 2048)
        return false;

    // One more hack (Resurrection sickness)
    if(spellInfo_2->Id==15007)
        return false;

    // Paladin Seals
    if( IsSealSpell(spellId_1) && IsSealSpell(spellId_2) )
        return true;

    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER )
                isModifier = true;
        }
        if (!isModifier)
            return true;
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
        return true;

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    for (int i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
        spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
        spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
        spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

    return true;
}

bool ObjectMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionSkill(skill);
}

bool ObjectMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsPrimaryProfessionSkill(skill);
}

bool ObjectMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId)
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

void ObjectMgr::LoadReputationOnKill()
{
    uint32 count = 0;

    //                                                0             1                      2
    QueryResult *result = WorldDatabase.Query("SELECT `creature_id`,`RewOnKillRepFaction1`,`RewOnKillRepFaction2`,"
    //   3              4              5                    6              7              8                    9
        "`IsTeamAward1`,`MaxStanding1`,`RewOnKillRepValue1`,`IsTeamAward2`,`MaxStanding2`,`RewOnKillRepValue2`,`TeamDependent` "
        "FROM `creature_onkill_reputation`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 creature award reputation definitions. DB table `creature_onkill_reputation` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 creature_id = fields[0].GetUInt32();

        ReputationOnKillEntry repOnKill;
        repOnKill.repfaction1          = fields[1].GetUInt32();
        repOnKill.repfaction2          = fields[2].GetUInt32();
        repOnKill.is_teamaward1        = fields[3].GetBool();
        repOnKill.reputration_max_cap1 = fields[4].GetUInt32();
        repOnKill.repvalue1            = fields[5].GetInt32();
        repOnKill.is_teamaward2        = fields[6].GetBool();
        repOnKill.reputration_max_cap2 = fields[7].GetUInt32();
        repOnKill.repvalue2            = fields[8].GetInt32();
        repOnKill.team_dependent       = fields[9].GetUInt8();

        if(repOnKill.repfaction1)
        {
            FactionEntry const *factionEntry1 = sFactionStore.LookupEntry(repOnKill.repfaction1);
            if(!factionEntry1)
            {
                sLog.outErrorDb("Faction (faction.dbc) %u does not exist but is used in `creature_onkill_reputation`",repOnKill.repfaction1);
                continue;
            }
        }

        if(repOnKill.repfaction2)
        {
            FactionEntry const *factionEntry2 = sFactionStore.LookupEntry(repOnKill.repfaction2);
            if(!factionEntry2)
            {
                sLog.outErrorDb("Faction (faction.dbc) %u does not exist but is used in `creature_onkill_reputation`",repOnKill.repfaction2);
                continue;
            }
        }

        mRepOnKill[creature_id] = repOnKill;

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString(">> Loaded %u creature award reputation definitions", count);
}

void ObjectMgr::CleanupInstances()
{
    // this routine cleans up old instances from all the tables before server start

    uint32 okcount = 0;
    uint32 delcount = 0;

    QueryResult *result;

    // first, obtain total instance set
    std::set< uint32 > InstanceSet;

    // creature_respawn
    result = WorldDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `creature_respawn` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // gameobject_respawn
    result = WorldDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `gameobject_respawn` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // character_instance
    result = CharacterDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `character_instance`");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // instance
    result = CharacterDatabase.PQuery("SELECT `id` FROM `instance`");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    // now remove all valid instances from instance list (it will become list of instances to delete)
    // instances considered valid:
    //   1) reset time > current time
    //   2) bound to at least one character (id is found in `character_instance` table)
    result = CharacterDatabase.PQuery("SELECT DISTINCT(`instance`.`id`) AS `id` FROM `instance` LEFT JOIN `character_instance` ON (`character_instance`.`instance` = `instance`.`id`) WHERE (`instance`.`id` = `character_instance`.`instance`) AND (`instance`.`resettime` > " I64FMTD ")", (uint64)time(NULL));
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            if (InstanceSet.find(fields[0].GetUInt32()) != InstanceSet.end())
            {
                InstanceSet.erase(fields[0].GetUInt32());
                okcount++;
            }
        }
        while (result->NextRow());
        delete result;
    }

    delcount = InstanceSet.size();

    barGoLink bar( delcount + 1 );
    bar.step();

    // remove all old instances from tables
    for (std::set< uint32 >::iterator i = InstanceSet.begin(); i != InstanceSet.end(); i++)
    {
        WorldDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `instance` = '%u'", *i);
        WorldDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `instance` = '%u'", *i);
        CharacterDatabase.PExecute("DELETE FROM `corpse` WHERE `instance` = '%u'", *i);
        CharacterDatabase.PExecute("DELETE FROM `instance` WHERE `id` = '%u'", *i);

        bar.step();
    }

    sLog.outString();
    sLog.outString( ">> Initialized %u instances, deleted %u old instances", okcount, delcount );
}

void ObjectMgr::PackInstances()
{
    // this routine renumbers player instance associations in such a way so they start from 1 and go up

    // obtain set of all associations
    std::set< uint32 > InstanceSet;

    // the check in query allows us to prevent table destruction in case of a bug we must never encounter
    QueryResult *result = CharacterDatabase.PQuery("SELECT DISTINCT(`instance`) FROM `character_instance` WHERE `instance` <> 0");
    if( result )
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
        delete result;
    }

    barGoLink bar( InstanceSet.size() + 1);
    bar.step();

    uint32 InstanceNumber = 1;

    // we do assume std::set is sorted properly on integer value
    for (std::set< uint32 >::iterator i = InstanceSet.begin(); i != InstanceSet.end(); i++)
    {
        if (*i != InstanceNumber)
        {
            // remap instance id
            WorldDatabase.PExecute("UPDATE `creature_respawn` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            WorldDatabase.PExecute("UPDATE `gameobject_respawn` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            CharacterDatabase.PExecute("UPDATE `corpse` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            CharacterDatabase.PExecute("UPDATE `character_instance` SET `instance` = '%u' WHERE `instance` = '%u'", InstanceNumber, *i);
            CharacterDatabase.PExecute("UPDATE `instance` SET `id` = '%u' WHERE `id` = '%u'", InstanceNumber, *i);
        }

        InstanceNumber++;
        bar.step();
    }

    sLog.outString();
    sLog.outString( ">> Instance numbers remapped, next instance id is %u", InstanceNumber );
}

void ObjectMgr::LoadWeatherZoneChances()
{
    uint32 count = 0;

    //                                             0      1                    2                    3                     4                    5                    6                     7                  8                  9                   10                   11                   12
    QueryResult *result = WorldDatabase.Query("SELECT `zone`,`spring_rain_chance`,`spring_snow_chance`,`spring_storm_chance`,`summer_rain_chance`,`summer_snow_chance`,`summer_storm_chance`,`fall_rain_chance`,`fall_snow_chance`,`fall_storm_chance`,`winter_rain_chance`,`winter_snow_chance`,`winter_storm_chance` FROM `game_weather`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 weather definitions. DB table `game_weather` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 zone_id = fields[0].GetUInt32();

        WeatherZoneChances& wzc = mWeatherZoneMap[zone_id];

        for(int season = 0; season < WEATHER_SEASONS; ++season)
        {
            wzc.data[season].rainChance  = fields[season * (MAX_WEATHER_TYPE-1) + 1].GetUInt32();
            wzc.data[season].snowChance  = fields[season * (MAX_WEATHER_TYPE-1) + 2].GetUInt32();
            wzc.data[season].stormChance = fields[season * (MAX_WEATHER_TYPE-1) + 3].GetUInt32();

            if(wzc.data[season].rainChance > 100)
            {
                wzc.data[season].rainChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u has wrong rain chance > 100%",zone_id,season);
            }

            if(wzc.data[season].snowChance > 100)
            {
                wzc.data[season].snowChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u has wrong snow chance > 100%",zone_id,season);
            }

            if(wzc.data[season].stormChance > 100)
            {
                wzc.data[season].stormChance = 25;
                sLog.outErrorDb("Weather for zone %u season %u has wrong storm chance > 100%",zone_id,season);
            }
        }

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString(">> Loaded %u weather definitions", count);
}

void ObjectMgr::SaveCreatureRespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    mCreatureRespawnTimes[MAKE_GUID(loguid,instance)] = t;
    WorldDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", loguid, instance);
    if(t)
        WorldDatabase.PExecute("INSERT INTO `creature_respawn` VALUES ( '%u', '" I64FMTD "', '%u' )", loguid, uint64(t), instance);
}

void ObjectMgr::DeleteCreatureData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    CreatureData const* data = GetCreatureData(guid);
    if(data)
    {
        CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
        uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

        CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
        cell_guids.creatures.erase(guid);
    }

    mCreatureDataMap.erase(guid);
}

void ObjectMgr::SaveGORespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    mGORespawnTimes[MAKE_GUID(loguid,instance)] = t;
    WorldDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `guid` = '%u' AND `instance` = '%u'", loguid, instance);
    if(t)
        WorldDatabase.PExecute("INSERT INTO `gameobject_respawn` VALUES ( '%u', '" I64FMTD "', '%u' )", loguid, uint64(t), instance);
}

void ObjectMgr::DeleteRespawnTimeForInstance(uint32 instance)
{
    RespawnTimes::iterator next;

    for(RespawnTimes::iterator itr = mGORespawnTimes.begin(); itr != mGORespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mGORespawnTimes.erase(itr);
    }

    for(RespawnTimes::iterator itr = mCreatureRespawnTimes.begin(); itr != mCreatureRespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mCreatureRespawnTimes.erase(itr);
    }

    WorldDatabase.PExecute("DELETE FROM `creature_respawn` WHERE `instance` = '%u'", instance);
    WorldDatabase.PExecute("DELETE FROM `gameobject_respawn` WHERE `instance` = '%u'", instance);
}

void ObjectMgr::DeleteGOData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    GameObjectData const* data = GetGOData(guid);
    if(data)
    {
        CellPair cell_pair = MaNGOS::ComputeCellPair(data->posX, data->posY);
        uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

        CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
        cell_guids.gameobjects.erase(guid);
    }

    mGameObjectDataMap.erase(guid);
}

void ObjectMgr::AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance)
{
    CellObjectGuids& cell_guids = mMapObjectGuids[mapid][cellid];
    cell_guids.corpses[player_guid] = instance;
}

void ObjectMgr::DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid)
{
    CellObjectGuids& cell_guids = mMapObjectGuids[mapid][cellid];
    cell_guids.corpses.erase(player_guid);
}

void ObjectMgr::LoadQuestRelationsHelper(QuestRelations& map,char const* table)
{
    map.clear();                                            // need for reload case

    uint32 count = 0;

    QueryResult *result = WorldDatabase.PQuery("SELECT `id`,`quest` FROM `%s`",table);

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 quest relations from %s. DB table `%s` is empty.",table,table);
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 id    = fields[0].GetUInt32();
        uint32 quest = fields[1].GetUInt32();

        if ( !mQuestTemplates[quest] )
        {
            sLog.outErrorDb("Table %s: Quest %u listed for entry %u does not exist.",table,quest,id);
            continue;
        }

        map.insert(QuestRelations::value_type(id,quest));

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString(">> Loaded %u quest relations from %s", count,table);
}

void ObjectMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT `entry`,`type`,`targetEntry` FROM `spell_script_target`");

    if(!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 SpellScriptTarget. DB table `spell_script_target` is empty.");
        return;
    }

    barGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);

        if(!spellProto)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        bool targetfound = false;
        for(int i = 0; i <3; ++i)
        {
            if(spellProto->EffectImplicitTargetA[i]==TARGET_SCRIPT||spellProto->EffectImplicitTargetB[i]==TARGET_SCRIPT)
            {
                targetfound = true;
                break;
            }
        }
        if(!targetfound)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implecit target TARGET_SCRIPT(38).",spellId,targetEntry);
            continue;
        }

        if( type >= MAX_SPELL_TARGET_TYPE )
        {
            sLog.outErrorDb("Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch(type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if( targetEntry==0 )
                    break;

                if(!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
            {
                if( targetEntry==0 )
                {
                    sLog.outErrorDb("Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }
                if(!sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellTargetType(type),targetEntry)));

        count++;
    } while (result->NextRow());

    delete result;

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

// Character Dumps (Write/Load)

#define DUMP_TABLE_COUNT 13

struct DumpTable
{
    char const* name;
    uint8 type;
};

static DumpTable dumpTables[DUMP_TABLE_COUNT] =
{
    { "character",                1 },
    { "character_action",         0 },
    { "character_aura",           0 },
    { "character_homebind",       0 },
    { "character_inventory",      2 },
    { "character_kill",           0 },
    { "character_queststatus",    0 },
    { "character_reputation",     0 },
    { "character_spell",          0 },
    { "character_spell_cooldown", 0 },
    { "item_instance",            3 },
    { "character_ticket",         0 },
    { "character_tutorial",       0 }
};

std::string CreateDumpString(char const* tableName, QueryResult *result)
{
    if(!tableName || !result) return "";
    std::ostringstream ss;
    ss << "INSERT INTO `" << tableName << "` VALUES (";
    Field *fields = result->Fetch();
    for(uint32 i = 0; i < result->GetFieldCount(); i++)
    {
        if (i == 0) ss << "'";
        else ss << ", '";

        std::string s = fields[i].GetCppString();
        CharacterDatabase.escape_string(s);
        ss << s;

        ss << "'";
    }
    ss << ");";
    return ss.str();
}

bool DumpPlayerTable(std::string& dump, uint32 guid, char const*tableFrom, char const*tableTo, uint8 type = 0)
{
    if (!tableFrom || !tableTo) return false;
    QueryResult *result = CharacterDatabase.PQuery("SELECT * FROM `%s` WHERE `%s` = '%d'", tableFrom, type != 3 ? "guid" : "owner_guid", guid);
    if (!result) return false;
    do
    {
        dump += CreateDumpString(tableTo, result);
        dump += "\n";
    }
    while (result->NextRow());
    delete result;
    return true;
}

std::string ObjectMgr::GetPlayerDump(uint32 guid)
{
    std::string dump;
    for(int i = 0; i < DUMP_TABLE_COUNT; i++)
        DumpPlayerTable(dump, guid, dumpTables[i].name, dumpTables[i].name, dumpTables[i].type);

    // TODO: Add pets/instance/group/gifts..
    // TODO: Add a dump level option to skip some non-important tables

    return dump;
}

bool ObjectMgr::WritePlayerDump(std::string file, uint32 guid)
{
    FILE *fout = fopen(file.c_str(), "w");
    if (!fout) { sLog.outError("Failed to open file!\r\n"); return false; }

    std::string dump = GetPlayerDump(guid);

    fprintf(fout,"%s\n",dump.c_str());
    fclose(fout);
    return true;
}

bool findnth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    s = str.find("VALUES ('")+9;
    if (s == std::string::npos) return false;

    do
    {
        e = str.find("'",s);
        if (e == std::string::npos) return false;
    } while(str[e-1] == '\\');

    for(int i = 1; i < n; i++)
    {
        do
        {
            s = e+4;
            e = str.find("'",s);
            if (e == std::string::npos) return false;
        } while (str[e-1] == '\\');
    }
    return true;
}

std::string gettablename(std::string &str)
{
    std::string::size_type s = 13;
    std::string::size_type e = str.find('`', s);
    if (e == std::string::npos)
        return "";

    return str.substr(s, e-s);
}

bool changenth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s, e;
    if(!findnth(str,n,s,e))
        return false;

    if(nonzero && str.substr(s,e-s) == "0")
        return true;                                        // not an error
    if(!insert)
        str.replace(s,e-s, with);
    else
        str.insert(s, with);

    return true;
}

std::string getnth(std::string &str, int n)
{
    std::string::size_type s, e;
    if(!findnth(str,n,s,e))
        return "";

    return str.substr(s, e-s);
}

bool findtoknth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    int i; s = e = 0;
    std::string::size_type size = str.size();
    for(i = 1; s < size && i < n; s++) if(str[s] == ' ') i++;
    if (i < n)
        return false;

    e = str.find(' ', s);

    return e != std::string::npos;
}

std::string gettoknth(std::string &str, int n)
{
    std::string::size_type s = 0, e = 0;
    if(!findtoknth(str, n, s, e))
        return "";

    return str.substr(s, e-s);
}

bool changetoknth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s = 0, e = 0;
    if(!findtoknth(str, n, s, e))
        return false;
    if(nonzero && str.substr(s,e-s) == "0")
        return true;                                        // not an error
    if(!insert)
        str.replace(s, e-s, with);
    else
        str.insert(s, with);

    return true;
}

uint32 registerItem(uint32 oldGuid, std::map<uint32, uint32> &items, uint32 hiGuid)
{
    if(items[oldGuid] == 0)
        items[oldGuid] = hiGuid + items.size();

    return items[oldGuid];
}

bool changeItem(std::string &str, int n, std::map<uint32, uint32> &items, uint32 hiGuid, bool nonzero = false)
{
    char chritem[20];
    uint32 oldGuid = atoi(getnth(str, n).c_str());
    if (nonzero && oldGuid == 0)
        return true;                                        // not an error

    uint32 newGuid = registerItem(oldGuid, items, hiGuid);
    snprintf(chritem, 20, "%d", newGuid);

    return changenth(str, n, chritem, false, nonzero);
}

bool changetokItem(std::string &str, int n, std::map<uint32, uint32> &items, uint32 hiGuid, bool nonzero = false)
{
    char chritem[20];
    uint32 oldGuid = atoi(gettoknth(str, n).c_str());
    if (nonzero && oldGuid == 0)
        return true;                                        // not an error

    uint32 newGuid = registerItem(oldGuid, items, hiGuid);
    snprintf(chritem, 20, "%d", newGuid);

    return changetoknth(str, n, chritem, false, nonzero);
}

#define ROLLBACK {CharacterDatabase.RollbackTransaction(); fclose(fin); return false;}

extern std::string notAllowedChars;

bool ObjectMgr::LoadPlayerDump(std::string file, uint32 account, std::string name, uint32 guid)
{
    FILE *fin = fopen(file.c_str(), "r");
    if(!fin) return false;

    QueryResult * result = NULL;
    char newguid[20], chraccount[20];

    // make sure the same guid doesn't already exist and is safe to use
    bool incHighest = true;
    if(guid != 0 && guid < m_hiCharGuid)
    {
        result = CharacterDatabase.PQuery("SELECT * FROM `character` WHERE `guid` = '%d'", guid);
        if (result)
        {
            guid = m_hiCharGuid;                            // use first free if exists
            delete result;
        }
        else incHighest = false;
    }
    else guid = m_hiCharGuid;

    // normalize the name if specified and check if it exists
    if(name != "" && name.find_first_of(notAllowedChars) == name.npos)
    {
        CharacterDatabase.escape_string(name);
        normalizePlayerName(name);
        result = CharacterDatabase.PQuery("SELECT * FROM `character` WHERE `name` = '%s'", name.c_str());
        if (result)
        {
            name = "";                                      // use the one from the dump
            delete result;
        }
    }
    else name = "";

    snprintf(newguid, 20, "%d", guid);
    snprintf(chraccount, 20, "%d", account);

    std::map<uint32, uint32> items;
    char buf[32000] = "";

    CharacterDatabase.BeginTransaction();
    while(!feof(fin))
    {
        if(!fgets(buf, 32000, fin))
        {
            if(feof(fin)) break;
            sLog.outError("LoadPlayerDump: File read error!");
            ROLLBACK;
        }

        std::string line; line.assign(buf);

        // skip empty strings
        if(line.find_first_not_of(" \t\n\r\7")==std::string::npos)
            continue;

        // determine table name and load type
        std::string tn = gettablename(line);
        if(tn.empty())
        {
            sLog.outError("LoadPlayerDump: Can't extract table name from line: '%s'!", line.c_str());
            ROLLBACK;
        }

        uint8 type, i;
        for(i = 0; i < DUMP_TABLE_COUNT; i++)
        {
            if (tn == dumpTables[i].name)
            {
                type = dumpTables[i].type;
                break;
            }
        }

        if (i == DUMP_TABLE_COUNT)
        {
            sLog.outError("LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            ROLLBACK;
        }

        // change the data to server values
        if(type <= 2)                                       // most tables
            if(!changenth(line, 1, newguid)) ROLLBACK;
        if(type == 1)                                       // character t.
        {
            // guid, data field:guid, items
            if(!changenth(line, 2, chraccount)) ROLLBACK;
            std::string vals = getnth(line, 3);
            if(!changetoknth(vals, OBJECT_FIELD_GUID+1, newguid)) ROLLBACK;
            for(uint16 field = PLAYER_FIELD_INV_SLOT_HEAD; field < PLAYER_FARSIGHT; field++)
                if(!changetokItem(vals, field+1, items, m_hiItemGuid, true)) ROLLBACK;
            if(!changenth(line, 3, vals.c_str())) ROLLBACK;
            if (name == "")
            {
                // check if the original name already exists
                name = getnth(line, 4);
                result = CharacterDatabase.PQuery("SELECT * FROM `character` WHERE `name` = '%s'", name.c_str());
                if (result)
                {
                    delete result;
                    if(!changenth(line, 29, "1")) ROLLBACK; // rename on login
                }
            }
            else if(!changenth(line, 4, name.c_str())) ROLLBACK;
        }
        else if(type == 2)                                  // character_inventory t.
        {
            // bag, item
            if(!changeItem(line, 2, items, m_hiItemGuid, true)) ROLLBACK;
            if(!changeItem(line, 4, items, m_hiItemGuid)) ROLLBACK;
        }
        else if(type == 3)                                  // item_instance t.
        {
            // item, owner, data field:item, owner guid
            if(!changeItem(line, 1, items, m_hiItemGuid)) ROLLBACK;
            if(!changenth(line, 2, newguid)) ROLLBACK;
            std::string vals = getnth(line,3);
            if(!changetokItem(vals, OBJECT_FIELD_GUID+1, items, m_hiItemGuid)) ROLLBACK;
            if(!changetoknth(vals, ITEM_FIELD_OWNER+1, newguid)) ROLLBACK;
            if(!changenth(line, 3, vals.c_str())) ROLLBACK;
        }

        if(!CharacterDatabase.Execute(line.c_str())) ROLLBACK;
    }
    CharacterDatabase.CommitTransaction();
    m_hiItemGuid += items.size();
    if(incHighest) m_hiCharGuid++;
    fclose(fin);
    return true;
}

void ObjectMgr::LoadReservedPlayersNames()
{
    m_ReservedNames.clear();                                // need for reload case

    QueryResult *result = WorldDatabase.PQuery("SELECT `name` FROM `reserved_name`");

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u reserved player names", count );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    Field* fields;
    do
    {
        bar.step();
        fields = result->Fetch();
        std::string name= fields[0].GetCppString();
        if(!name.empty())
        {
            normalizePlayerName(name);
            m_ReservedNames.insert(name);
            ++count;
        }
    } while ( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u reserved player names", count );
}

int ObjectMgr::GetIndexForLocale( LocaleConstant loc )
{
    if(loc==LOCALE_ENG)
        return -1;

    for(size_t i=0;i < m_LocalToIndex.size(); ++i)
        if(m_LocalToIndex[i]==loc)
            return i;

    return -1;
}

int ObjectMgr::GetOrNewIndexForLocale( LocaleConstant loc )
{
    if(loc==LOCALE_ENG)
        return -1;

    for(size_t i=0;i < m_LocalToIndex.size(); ++i)
        if(m_LocalToIndex[i]==loc)
            return i;

    m_LocalToIndex.push_back(loc);
    return m_LocalToIndex.size()-1;
}

void ObjectMgr::LoadBattleMastersEntry()
{
    mBattleMastersMap.clear();                              // need for reload case

    QueryResult *result = WorldDatabase.Query( "SELECT `entry`,`bg_template` FROM `battlemaster_entry`" );

    uint32 count = 0;

    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 battlemaster entries - table is empty!" );
        return;
    }

    barGoLink bar( result->GetRowCount() );

    do
    {
        ++count;
        bar.step();

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();

        mBattleMastersMap[entry] = bgTypeId;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u battlemaster entries", count );
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool ObjectMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(int i=0; i<3; ++i)
    {
        switch(spellInfo->Effect[i])
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!GetItemPrototype( spellInfo->EffectItemType[i] ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outErrorDb("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(spellInfo->EffectTriggerSpell[0]);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                        else
                            sLog.outErrorDb("Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[0]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j = 0; j < 8; ++j)
        {
            if(spellInfo->Reagent[j] > 0 && !GetItemPrototype( spellInfo->Reagent[j] ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outErrorDb("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

void ObjectMgr::LoadGameObjectForQuests()
{
    mGameObjectForQuestSet.clear();                         // need for reload case

    uint32 count = 0;

    // scan GO templates with loot including quest items
    for(uint32 go_entry = 1; go_entry < sGOStorage.MaxEntry; ++go_entry)
    {
        GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(go_entry);
        if(!goInfo)
            continue;

        uint32 loot_id = GameObject::GetLootId(goInfo);

        // find loot for GO
        LootStore::const_iterator go_loot = LootTemplates_Gameobject.find(loot_id);
        if(go_loot == LootTemplates_Gameobject.end())
            continue;

        // scan loot for quest items
        for(LootStoreItemList::const_iterator item_i = go_loot->second.begin(); item_i != go_loot->second.end(); ++item_i )
        {
            if(item_i->questChanceOrGroup > 0)
            {
                mGameObjectForQuestSet.insert(go_entry);
                count++;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u GameObject for quests", count );
}
