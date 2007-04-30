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
#include "WorldPacket.h"
#include "WorldSocket.h" 
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Guild.h"
#include "UpdateMask.h"
#include "Chat.h"
#include "Auth/md5.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Transports.h"
#include "Group.h"

void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
    // keys can be non cleared if player open realm list and close it by 'cancel'
    loginDatabase.PQuery("UPDATE `account` SET `v` = '0', `s` = '0' WHERE `id` = '%u'", GetAccountId());

    WorldPacket data(SMSG_CHAR_ENUM, 100);                  // we guess size

    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%u' ORDER BY `guid`", GetAccountId());

    uint8 num = 0;

    data << num;

    if( result )
    {
        Player *plr = new Player(this);
        do
        {
            sLog.outDetail("Loading char guid %u from account %u.",(*result)[0].GetUInt32(),GetAccountId());

            if(plr->MinimalLoadFromDB( (*result)[0].GetUInt32() ))
            {
                plr->BuildEnumData( &data );
                num++;
            }
        }
        while( result->NextRow() );

        delete plr;
        delete result;
    }

    data.put<uint8>(0, num);

    SendPacket( &data );
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
    // in Player::Create:
    //uint8 race,class_,gender,skin,face,hairStyle,hairColor,facialHair,outfitId;
    //data >> name
    //data >> race >> class_ >> gender >> skin >> face;
    //data >> hairStyle >> hairColor >> facialHair >> outfitId;

    CHECK_PACKET_SIZE(recv_data,1+1+1+1+1+1+1+1+1+1);

    std::string name;
    WorldPacket data;
    uint8 race_;

    recv_data >> name;

    // recheck with known string size
    CHECK_PACKET_SIZE(recv_data,(name.size()+1)+1+1+1+1+1+1+1+1+1);

    recv_data >> race_;
    recv_data.rpos(0);

    // prevent character creating with invalid name
    if(name.size() == 0)
    {
        data.Initialize( SMSG_CHAR_CREATE, 1 );
        data << (uint8)CHAR_NAME_INVALID_CHARACTER;
        SendPacket( &data );
        return;
    }

    normalizePlayerName(name);

    // check used symbols
    static std::string const notAllowedChars = "\t\v\b\f\a\n\r\\\0\"\'\? <>[](){}_=+-|/!@#$%^&*~`.,0123456789";

    if(name.find_first_of(notAllowedChars)!=name.npos)
    {
        data.Initialize( SMSG_CHAR_CREATE, 1 );
        data << (uint8)CHAR_NAME_INVALID_CHARACTER;;
        SendPacket( &data );
        return;
    }

    sDatabase.escape_string(name);
    QueryResult *result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `name` = '%s'", name.c_str());

    if ( result )
    {
        delete result;

        data.Initialize(SMSG_CHAR_CREATE, 1);
        data << (uint8)CHAR_LOGIN_DUPLICATE_CHARACTER;
        SendPacket( &data );

        return;
    }

    result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account` = '%u'", GetAccountId());

    if ( result )
    {
        if (result->GetRowCount() >= 10)
        {
            data.Initialize(SMSG_CHAR_CREATE, 1);
            data << (uint8)CHAR_CREATE_ACCOUNT_LIMIT;
            SendPacket( &data );
            delete result;
            return;
        }
        delete result;
    }

    bool AllowTwoSideAccounts = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS);
    if(sWorld.IsPvPRealm())
    {
        QueryResult *result2 = sDatabase.PQuery("SELECT `race` FROM `character` WHERE `account` = '%u' LIMIT 1", GetAccountId());
        if(result2)
        {
            Field * field = result2->Fetch();
            uint8 race = field[0].GetUInt32();
            delete result2;
            uint32 team=0;
            if(race > 0)
                team = Player::TeamForRace(race);

            uint32 team_=0;
            if(race_ > 0)
                team_ = Player::TeamForRace(race_);

            if(team != team_ && GetSecurity() < 2 && !AllowTwoSideAccounts)
            {
                data.Initialize( SMSG_CHAR_CREATE, 1 );
                data << (uint8)CHAR_CREATE_PVP_TEAMS_VIOLATION;
                SendPacket( &data );
                return;
            }
        }
    }

    Player * pNewChar = new Player(this);

    if(pNewChar->Create( objmgr.GenerateLowGuid(HIGHGUID_PLAYER), recv_data ))
    {
        // Player create
        pNewChar->SaveToDB();

        QueryResult *resultCount = sDatabase.PQuery("SELECT COUNT(guid) FROM `character` WHERE `account` = '%d'", GetAccountId());
        uint32 charCount = 0;
        if (resultCount)
        {
            Field *fields = resultCount->Fetch();
            charCount = fields[0].GetUInt32();
            delete resultCount;
            loginDatabase.PExecute("INSERT INTO `realmcharacters` (`numchars`, `acctid`, `realmid`) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE `numchars` = '%d'", charCount, GetAccountId(), realmID, charCount);
        }

        delete pNewChar;
    }
    else
    {
        // Player not create (race/class problem?)
        delete pNewChar;

        data.Initialize(SMSG_CHAR_CREATE, 1);
        data << (uint8)CHAR_CREATE_ERROR;
        SendPacket( &data );

        return;
    }

    data.Initialize( SMSG_CHAR_CREATE, 1 );
    data << (uint8)CHAR_CREATE_SUCCESS;
    SendPacket( &data );

    sLog.outBasic("Account: %d Create New Character:[%s]",GetAccountId(),name.c_str());
}

void WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    // can't delete loaded character
    if(objmgr.GetPlayer(guid))
        return;

    {
        Player plr(this);

        // "GetAccountId()==db stored account id" checked in LoadFromDB (prevent deleting not own character using cheating tools)
        if(!plr.LoadFromDB( GUID_LOPART(guid) ))
            return;
        sLog.outBasic("Account: %d Delete Character:[%s] (guid:%u)",GetAccountId(),plr.GetName(),guid);
        plr.DeleteFromDB();
    }

    WorldPacket data(SMSG_CHAR_DELETE, 1);
    data << (uint8)CHAR_DELETE_SUCCESS;
    SendPacket( &data );
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    m_playerLoading = true;
    uint64 playerGuid = 0;

    DEBUG_LOG( "WORLD: Recvd Player Logon Message" );

    recv_data >> playerGuid;

    Player* plr = new Player(this);
    ASSERT(plr);

    plr->SetSession(this);
    // "GetAccountId()==db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if(!plr->LoadFromDB(GUID_LOPART(playerGuid)))
    {
        m_playerLoading = false;
        return;
    }
    //plr->_RemoveAllItemMods();

    //set a count of unread mails:
    QueryResult *resultMails = sDatabase.PQuery("SELECT COUNT(id) FROM `mail` WHERE `receiver` = '%u' AND `checked` = 0", GUID_LOPART(playerGuid));
    if (resultMails)
    {
        Field *fieldMail = resultMails->Fetch();
        plr->unReadMails = fieldMail[0].GetUInt8();
        delete resultMails;
    }
    else
        plr->unReadMails = 0;

    SetPlayer(plr);

    Player *pCurrChar = GetPlayer();

    pCurrChar->SendDungeonDifficulty();

    WorldPacket data( SMSG_LOGIN_VERIFY_WORLD, 20 );
    data << plr->GetMapId();
    data << plr->GetPositionX();
    data << plr->GetPositionY();
    data << plr->GetPositionZ();
    data << plr->GetOrientation();
    SendPacket(&data);

    data.Initialize( SMSG_ACCOUNT_DATA_MD5, 128 );
    for(int i = 0; i < 128; i++)
        data << uint8(0);
    SendPacket(&data);

    pCurrChar->LoadIgnoreList();
    pCurrChar->SendFriendlist();
    pCurrChar->SendIgnorelist();


    // Send MOTD
    {
        data.Initialize(SMSG_MOTD, 50); // new in 2.0.1
        data << (uint32)0;

        uint32 linecount=0;
        string str_motd = sWorld.GetMotd();
        string::size_type pos, nextpos;

        pos = 0;
        while ( (nextpos= str_motd.find('@',pos)) != string::npos )
        {
            if (nextpos != pos)
            {
                data << str_motd.substr(pos,nextpos-pos);
                linecount++;
            }
            pos = nextpos+1;
        }

        if (pos<str_motd.length())
        {
            data << str_motd.substr(pos);
            linecount++;
        }

        data.put(0, linecount);

        SendPacket( &data );
        DEBUG_LOG( "WORLD: Sent motd (SMSG_MOTD)" );
    }

    if(pCurrChar->GetGuildId() != 0)
    {
        Guild* guild = objmgr.GetGuildById(pCurrChar->GetGuildId());
        if(guild)
        {
            data.Initialize(SMSG_GUILD_EVENT, (2+guild->GetMOTD().size()+1));
            data << (uint8)GE_MOTD;
            data << (uint8)1;
            data << guild->GetMOTD();
            SendPacket(&data);
            DEBUG_LOG( "WORLD: Sent guild-motd (SMSG_GUILD_EVENT)" );

            data.Initialize(SMSG_GUILD_EVENT, (5+10));      // we guess size
            data<<(uint8)GE_SIGNED_ON;
            data<<(uint8)1;
            data<<pCurrChar->GetName();
            data<<pCurrChar->GetGUID();
            guild->BroadcastPacket(&data);
            DEBUG_LOG( "WORLD: Sent guild-signed-on (SMSG_GUILD_EVENT)" );
        }
        else
        {
            // remove wrong guild data
            sLog.outError("Player %s (GUID: %u) marked as member not existed guild (id: %u), removing guild membership for player.",pCurrChar->GetName(),pCurrChar->GetGUIDLow(),pCurrChar->GetGuildId());
            pCurrChar->SetUInt32Value(PLAYER_GUILDID,0);
            pCurrChar->SetUInt32ValueInDB(PLAYER_GUILDID,0,pCurrChar->GetGUID());
        }
    }

    // rest_start

    // home bind stuff
    {
        QueryResult *result4 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `character_homebind` WHERE `guid` = '%u'", GUID_LOPART(playerGuid));
        if (result4)
        {
            Field *fields = result4->Fetch();
            _player->m_homebindMapId = fields[0].GetUInt32();
            _player->m_homebindZoneId = fields[1].GetUInt16();
            _player->m_homebindX = fields[2].GetFloat();
            _player->m_homebindY = fields[3].GetFloat();
            _player->m_homebindZ = fields[4].GetFloat();
            delete result4;
        }
        else
        {
            int plrace = GetPlayer()->getRace();
            int plclass = GetPlayer()->getClass();
            QueryResult *result5 = sDatabase.PQuery("SELECT `map`,`zone`,`position_x`,`position_y`,`position_z` FROM `playercreateinfo` WHERE `race` = '%u' AND `class` = '%u'", plrace, plclass);

            if(!result5)
            {
                sLog.outErrorDb("Table `playercreateinfo` not have data for race %u class %u , character can't be loaded.",plrace, plclass);
                LogoutPlayer(false);                            // without save
                return;
            }

            Field *fields = result5->Fetch();
            // store and send homebind for player
            _player->m_homebindMapId = fields[0].GetUInt32();
            _player->m_homebindZoneId = fields[1].GetUInt16();
            _player->m_homebindX = fields[2].GetFloat();
            _player->m_homebindY = fields[3].GetFloat();
            _player->m_homebindZ = fields[4].GetFloat();
            sDatabase.PExecute("INSERT INTO `character_homebind` (`guid`,`map`,`zone`,`position_x`,`position_y`,`position_z`) VALUES ('%u', '%u', '%u', '%f', '%f', '%f')", GUID_LOPART(playerGuid), _player->m_homebindMapId, (uint32)_player->m_homebindZoneId, _player->m_homebindX, _player->m_homebindY, _player->m_homebindZ);
            delete result5;
        }

        data.Initialize (SMSG_BINDPOINTUPDATE, 5*4);
        data << _player->m_homebindX << _player->m_homebindY << _player->m_homebindZ;
        data << (uint32) _player->m_homebindMapId;
        data << (uint32) _player->m_homebindZoneId;
        SendPacket (&data);

        DEBUG_LOG("Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f\n",
            _player->m_homebindMapId,_player->m_homebindZoneId,_player->m_homebindX,_player->m_homebindY, _player->m_homebindZ);
    }

    data.Initialize( SMSG_TUTORIAL_FLAGS, 8*32 );
    for (int i = 0; i < 8; i++)
        data << uint32( GetPlayer()->GetTutorialInt(i) );
    SendPacket(&data);
    //sLog.outDebug( "WORLD: Sent tutorial flags." );

    pCurrChar->_LoadSpellCooldowns();
    GetPlayer()->SendInitialSpells();
    GetPlayer()->SendInitialActionButtons();
    GetPlayer()->SendInitialReputations();

    /*if(GetPlayer()->getClass() == CLASS_HUNTER || GetPlayer()->getClass() == CLASS_ROGUE)
    {
        uint32 shiftdata=0x01;
        for(uint8 i=0;i<32;i++)
        {
            if ( 522753 & shiftdata )
            {
                data.Initialize(SMSG_SET_FLAT_SPELL_MODIFIER);
                data << uint8(i);
                data << uint8(5);
                data << uint16(1);
                data << uint16(0);
                SendPacket(&data);
            }
            shiftdata=shiftdata<<1;
        }
    }*/

    //Show cinematic at the first time that player login
    if( !GetPlayer()->getCinematic() )
    {
        GetPlayer()->setCinematic(1);

        ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(GetPlayer()->getRace());
        if(rEntry)
        {
            data.Initialize( SMSG_TRIGGER_CINEMATIC,4 );
            data << uint32(rEntry->startmovie);
            SendPacket( &data );
        }
    }

    pCurrChar->SendInitWorldStates();

    pCurrChar->CastSpell(pCurrChar, 836, true); // LOGINEFFECT

    data.Initialize(SMSG_LOGIN_SETTIMESPEED, 8);
    time_t gameTime = sWorld.GetGameTime();
    struct tm *lt = localtime(&gameTime);
    uint32 xmitTime = (lt->tm_year - 100) << 24 | lt->tm_mon  << 20 |
        (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 |
        lt->tm_hour << 6 | lt->tm_min;
    data << xmitTime;
    data << (uint32)0x3C888889;                             //(float)0.017f;
    SendPacket( &data );

    GetPlayer()->UpdateHonorFields();

    QueryResult *result = sDatabase.PQuery("SELECT `guildid`,`rank` FROM `guild_member` WHERE `guid` = '%u'",pCurrChar->GetGUIDLow());

    if(result)
    {
        Field *fields = result->Fetch();
        pCurrChar->SetInGuild(fields[0].GetUInt32());
        pCurrChar->SetRank(fields[1].GetUInt32());
        delete result;
    }
    else if(pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(0);
        pCurrChar->SetRank(0);
    }

    // this remained from raid group load (raid group load was moved to player code)
    if(pCurrChar->groupInfo.group)
    {
        //pCurrChar->groupInfo.group->SendInit(this); // useless
        pCurrChar->groupInfo.group->SendUpdate();
    }

    if (!MapManager::Instance().GetMap(pCurrChar->GetMapId(), pCurrChar)->AddInstanced(pCurrChar))
    {
        // TODO : Teleport to zone-in area
    }

    MapManager::Instance().GetMap(pCurrChar->GetMapId(), pCurrChar)->Add(pCurrChar);
    ObjectAccessor::Instance().InsertPlayer(pCurrChar);
    //sLog.outDebug("Player %s added to Map.",pCurrChar->GetName());

    if (pCurrChar->m_transport)
    {
        Transport* curTrans = pCurrChar->m_transport;
        pCurrChar->TeleportTo(curTrans->GetMapId(), curTrans->GetPositionX(), curTrans->GetPositionY(), curTrans->GetPositionZ(), curTrans->GetOrientation(), true, false);
    }

    sDatabase.PExecute("UPDATE `character` SET `online` = 1 WHERE `guid` = '%u'", pCurrChar->GetGUIDLow());
    loginDatabase.PExecute("UPDATE `account` SET `online` = 1 WHERE `id` = '%u'", GetAccountId());
    plr->SetInGameTime( getMSTime() );

    // set some aura effects after add player to map
    if(pCurrChar->HasAuraType(SPELL_AURA_MOD_STUN))
        pCurrChar->SetMovement(MOVE_ROOT);

    if(pCurrChar->HasAuraType(SPELL_AURA_MOD_ROOT))
    {
        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
        data.append(pCurrChar->GetPackGUID());
        data << (uint32)2;
        pCurrChar->SendMessageToSet(&data,true);
    }

    // friend status
    data.Initialize(SMSG_FRIEND_STATUS, 19);
    data<<uint8(FRIEND_ONLINE);
    data<<pCurrChar->GetGUID();
    data<<uint8(1);
    data<<pCurrChar->GetAreaId();
    data<<pCurrChar->getLevel();
    data<<pCurrChar->getClass();
    pCurrChar->BroadcastPacketToFriendListers(&data);

    pCurrChar->LoadEnchant();

    // Place character in world (and load zone) before some object loading
    pCurrChar->LoadCorpse();

    // setting Ghost+speed if dead
    //if ( pCurrChar->m_deathState == DEAD )
    if ( pCurrChar->m_deathState != ALIVE )
    {
        // not blizz like, we must correctly save and load player instead...
        pCurrChar->CastSpell(pCurrChar, 20584, true, 0); // auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
        pCurrChar->CastSpell(pCurrChar, 8326, true, 0); // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)
        //pCurrChar->SetUInt32Value(UNIT_FIELD_AURA+41, 8326);
        //pCurrChar->SetUInt32Value(UNIT_FIELD_AURA+42, 20584);
        //pCurrChar->SetUInt32Value(UNIT_FIELD_AURAFLAGS+6, 238);
        //pCurrChar->SetUInt32Value(UNIT_FIELD_AURALEVELS+11, 514);
        //pCurrChar->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+11, 65535);
        //pCurrChar->SetUInt32Value(UNIT_FIELD_DISPLAYID, 1825);
        //if (pCurrChar->getRace() == RACE_NIGHTELF)
        //{
        //    pCurrChar->SetSpeed(MOVE_RUN,  1.5f*1.2f, true);
        //    pCurrChar->SetSpeed(MOVE_SWIM, 1.5f*1.2f, true);
        //}
        //else
        //{
        //    pCurrChar->SetSpeed(MOVE_RUN,  1.5f, true);
        //    pCurrChar->SetSpeed(MOVE_SWIM, 1.5f, true);
        //}
        pCurrChar->SetMovement(MOVE_WATER_WALK);
    }

    // Load pet if any and player is alive
    if(pCurrChar->isAlive())
        pCurrChar->LoadPet();

    // show time before shutdown if shutdown planned.
    if(sWorld.IsShutdowning())
        sWorld.ShutdownMsg(true,pCurrChar);

    if(pCurrChar->isGameMaster())
        SendNotification("GM mode is ON");
    m_playerLoading = false;
    pCurrChar->SendAllowMove(); // may be add it to CMSG_SET_ACTIVE_MOVER handler?
}

void WorldSession::HandleSetFactionAtWar( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4+1);

    //sLog.outDebug("WORLD SESSION: HandleSetFactionAtWar");

    uint32 repListID;
    uint8  flag;

    recv_data >> repListID;
    recv_data >> flag;

    GetPlayer()->SetFactionAtWar(repListID,flag);
}

//I think this function is never used :/ I dunno, but i guess this opcode not exists
void WorldSession::HandleSetFactionCheat( WorldPacket & recv_data )
{
    //CHECK_PACKET_SIZE(recv_data,4+4);

    //sLog.outDebug("WORLD SESSION: HandleSetFactionCheat");
    /*
        uint32 FactionID;
        uint32 Standing;

        recv_data >> FactionID;
        recv_data >> Standing;

        std::list<struct Factions>::iterator itr;

        for(itr = GetPlayer()->factions.begin(); itr != GetPlayer()->factions.end(); ++itr)
        {
            if(itr->ReputationListID == FactionID)
            {
                itr->Standing += Standing;
                itr->Flags = (itr->Flags | 1);
                break;
            }
        }
    */
    GetPlayer()->UpdateReputation();
}

void WorldSession::HandleMeetingStoneInfo( WorldPacket & recv_data )
{
    DEBUG_LOG( "WORLD: Received CMSG_MEETING_STONE_INFO" );
}

void WorldSession::HandleTutorialFlag( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 iFlag;
    recv_data >> iFlag;

    uint32 wInt = (iFlag / 32);
    uint32 rInt = (iFlag % 32);

    uint32 tutflag = GetPlayer()->GetTutorialInt( wInt );
    tutflag |= (1 << rInt);
    GetPlayer()->SetTutorialInt( wInt, tutflag );

    //sLog.outDebug("Received Tutorial Flag Set {%u}.", iFlag);
}

void WorldSession::HandleTutorialClear( WorldPacket & recv_data )
{
    for ( uint32 iI = 0; iI < 8; iI++)
        GetPlayer()->SetTutorialInt( iI, 0xFFFFFFFF );
}

void WorldSession::HandleTutorialReset( WorldPacket & recv_data )
{
    for ( uint32 iI = 0; iI < 8; iI++)
        GetPlayer()->SetTutorialInt( iI, 0x00000000 );
}

void WorldSession::HandleSetWatchedFactionIndexOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    DEBUG_LOG("WORLD: Received CMSG_SET_WATCHED_FACTION_INDEX");
    uint32 fact;
    recv_data >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

void WorldSession::HandleToggleHelmOpcode( WorldPacket & recv_data )
{
    DEBUG_LOG("CMSG_TOGGLE_HELM for %s", _player->GetName());
    if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
        _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
    else
        _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleToggleCloakOpcode( WorldPacket & recv_data )
{
    DEBUG_LOG("CMSG_TOGGLE_CLOAK for %s", _player->GetName());
    if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
        _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
    else
        _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}
