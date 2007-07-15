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
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "RedZoneDistrict.h"
#include "Transports.h"
#ifdef _DEBUG_VMAPS
#include "VMapFactory.h"
#endif
bool ChatHandler::HandleSayCommand(const char* args)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();    
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->Say(args, LANG_UNIVERSAL, 0);

    return true;
}

bool ChatHandler::HandleYellCommand(const char* args)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();    
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->Yell(args, LANG_UNIVERSAL, 0);

    return true;
}

bool ChatHandler::HandleEmote2Command(const char* args)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();        
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->TextEmote(args, 0);

    return true;
}

bool ChatHandler::HandleWhisperCommand(const char* args)        //WTF? what should it do?
{
    char* receiver = strtok((char*)args, " ");
    char* text = strtok(NULL, "");

    uint64 guid = m_session->GetPlayer()->GetSelection();  
    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature || !receiver || !text)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    pCreature->Whisper(atol(receiver), text);
    
    return true;
}

bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    if(!*args)
        return false;

    std::string str = LANG_SYSTEMMESSAGE;
    str += args;
    sWorld.SendWorldText(str.c_str(), NULL);

#ifdef _DEBUG_VMAPS
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    float x,y,z;
    m_session->GetPlayer()->GetPosition(x,y,z);
    char buffer[100];
    sprintf(buffer, "pos %f,%f,%f",x,y,z);
    vMapManager->processCommand(buffer);
#endif
    return true;
}

bool ChatHandler::HandleNotifyCommand(const char* args)
{
    if(!*args)
        return false;

    std::string str = LANG_GLOBAL_NOTIFY;
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
    data << str;
    sWorld.SendGlobalMessage(&data);

    return true;
}

bool ChatHandler::HandleGMOnCommand(const char* args)
{
    m_session->GetPlayer()->SetGameMaster(true);
    m_session->SendNotification("GM mode is ON");
#ifdef _DEBUG_VMAPS
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    vMapManager->processCommand("stoplog");
#endif
    return true;
}

bool ChatHandler::HandleGMOffCommand(const char* args)
{
    m_session->GetPlayer()->SetGameMaster(false);
    m_session->SendNotification("GM mode is OFF");
#ifdef _DEBUG_VMAPS
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    vMapManager->processCommand("startlog");
#endif
    return true;
}

bool ChatHandler::HandleVisibleCommand(const char* args)
{
    int option = atoi((char*)args);

    if (option != 0 && option != 1 || !*args)
    {
        SendSysMessage(LANG_USE_BOL);
        PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->isGMVisible() ?  LANG_VISIBLE : LANG_INVISIBLE);
        return true;
    }

    if ( option )
    {
        m_session->GetPlayer()->SetGMVisible(true);
        m_session->SendNotification( LANG_INVISIBLE_VISIBLE );
    }
    else
    {
        m_session->SendNotification( LANG_INVISIBLE_INVISIBLE );
        m_session->GetPlayer()->SetGMVisible(false);
    }

    return true;
}

bool ChatHandler::HandleGPSCommand(const char* args)
{
    WorldObject *obj = getSelectedUnit();

    if(!obj)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    CellPair cell_val = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell = RedZone::GetZone(cell_val);

    PSendSysMultilineMessage(LANG_MAP_POSITION,
        obj->GetMapId(), obj->GetZoneId(), obj->GetAreaId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(),
        obj->GetOrientation(),cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(),obj->GetInstanceId());

    sLog.outDebug("Player %s GPS call %s %u " LANG_MAP_POSITION, m_session->GetPlayer()->GetName(),
        (obj->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), obj->GetGUIDLow(),
        obj->GetMapId(), obj->GetZoneId(), obj->GetAreaId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(),
        obj->GetOrientation(), cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(),obj->GetInstanceId());

    return true;
}

bool ChatHandler::HandleNamegoCommand(const char* args)
{
    if(m_session->GetPlayer()->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    if(!*args)
        return false;

    std::string name = args;
    normalizePlayerName(name);
    //sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {

        if(chr->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
            return true;
        }

        if(chr->isInFlight())
        {
            PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
            return true;
        }

        if (MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId(), m_session->GetPlayer())->Instanceable())
        {
            if ( (MapManager::Instance().GetMap(chr->GetMapId(), chr)->Instanceable()) &&
                (MapManager::Instance().GetMap(chr->GetMapId(), chr)->GetInstanceId() != 
                  MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId(), m_session->GetPlayer())->GetInstanceId()) )
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_INST_INST);
                return true;
            }

            // we are in instance, and can summon only player in our group with us as lead
            if ( !m_session->GetPlayer()->GetGroup() || !chr->GetGroup() || 
                (chr->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
                (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) )
                // the last check is a bit excessive, but let it be, just in case
            {
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,chr->GetName());
                return true;
            }
        }

        PSendSysMessage(LANG_SUMMONING, chr->GetName(),"");

        if (m_session->GetPlayer()->isVisibleFor(chr))
        {
            char buf0[256];
            snprintf((char*)buf0,256,LANG_SUMMONED_BY, m_session->GetPlayer()->GetName());

            WorldPacket data;
            FillSystemMessageData(&data, m_session, buf0);
            chr->GetSession()->SendPacket( &data );
        }

        chr->SetRecallPosition(chr->GetMapId(),chr->GetPositionX(),chr->GetPositionY(),chr->GetPositionZ(),chr->GetOrientation());

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetClosePoint(NULL,x,y,z,chr->GetObjectSize());
        chr->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,chr->GetOrientation());
    }
    else if (uint64 guid = objmgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_SUMMONING, name.c_str()," (offline)");

        // in point where GM stay
        Player::SavePositionInDB(m_session->GetPlayer()->GetMapId(),
            m_session->GetPlayer()->GetPositionX(),
            m_session->GetPlayer()->GetPositionY(),
            m_session->GetPlayer()->GetPositionZ(),m_session->GetPlayer()->GetOrientation(),guid);
    }
    else
        PSendSysMessage(LANG_NO_PLAYER, args);

    return true;
}

bool ChatHandler::HandleGonameCommand(const char* args)
{
    Player* _player = m_session->GetPlayer();

    if(_player->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    if(!*args)
        return false;

    std::string name = args;
    normalizePlayerName(name);
    //sDatabase.escape_string(name);                          // prevent SQL injection - normal name don't must changed by this call

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {
        if (MapManager::Instance().GetMap(chr->GetMapId(), chr)->Instanceable())
        {
            if ( (MapManager::Instance().GetMap(_player->GetMapId(), _player)->Instanceable()) &&
                (MapManager::Instance().GetMap(chr->GetMapId(), chr)->GetInstanceId() != 
                  MapManager::Instance().GetMap(_player->GetMapId(), _player)->GetInstanceId()) )
            {
                // cannot go from instance to instance
                PSendSysMessage(LANG_CANNOT_INST_INST);
                return true;
            }

            // we have to go to instance, and can go to player only if:
            //   1) we are in his group (either as leader or as member)
            //   2) we are not bound to any group and have GM mode on
            if (_player->GetGroup())
            {
                // we are in group, we can go only if we are in the player group
                if (_player->GetGroup() != chr->GetGroup())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY,chr->GetName());
                    return true;
                }
            }
            else
            {
                // we are not in group, let's verify our GM mode
                if (!_player->isGameMaster())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM,chr->GetName());
                    return true;
                }
            }

            // bind us to the players instance
            BoundInstancesMap::iterator i = chr->m_BoundInstances.find(chr->GetMapId());
            if (i == chr->m_BoundInstances.end()) return true; // error, the player has no instance bound!!!
            _player->m_BoundInstances[chr->GetMapId()] = std::pair < uint32, uint32 >(i->second.first, i->second.second);
            _player->SetInstanceId(chr->GetInstanceId());
        }

        PSendSysMessage(LANG_APPEARING_AT, chr->GetName());

        if (_player->isVisibleFor(chr))
        {
            char buf0[256];
            sprintf((char*)buf0,LANG_APPEARING_TO, _player->GetName());

            WorldPacket data;
            FillSystemMessageData(&data, m_session, buf0);
            chr->GetSession()->SendPacket(&data);
        }

        _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

        // to point to see at target with same orientation
        float x,y,z;
        chr->GetContactPoint(m_session->GetPlayer(),x,y,z);

        _player->TeleportTo(chr->GetMapId(), x, y, z, _player->GetAngle( chr ), true, true, true);
        return true;
    }

    if (uint64 guid = objmgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_APPEARING_AT, name.c_str());

        // to point where player stay (if loaded)
        float x,y,z,o;
        uint32 map;
        if(Player::LoadPositionFromDB(map,x,y,z,o,guid))
        {
            _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());
            _player->TeleportTo(map, x, y, z,_player->GetOrientation());
            return true;
        }
    }

    PSendSysMessage(LANG_NO_PLAYER, args);

    return true;
}

bool ChatHandler::HandleRecallCommand(const char* args)
{
    Player* chr = NULL;

    if(!*args)
    {
        chr = getSelectedPlayer();

        if(!chr)
        {
            chr = m_session->GetPlayer();

            if(chr->isInFlight())
            {
                SendSysMessage(LANG_TARGET_IN_FLIGHT);
                return true;
            }
        }
    }
    else
    {
        std::string name = args;
        normalizePlayerName(name);
        //sDatabase.escape_string(name);                      // prevent SQL injection - normal name don't must changed by this call

        chr = objmgr.GetPlayer(name.c_str());

        if(!chr)
        {
            PSendSysMessage(LANG_NO_PLAYER, args);
            return true;
        }
    }

    if(chr->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
        return true;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT, chr->GetName());
        return true;
    }

    chr->TeleportTo(chr->m_recallMap, chr->m_recallX, chr->m_recallY, chr->m_recallZ, chr->m_recallO);
    return true;
}

bool ChatHandler::HandleModifyKnownTitlesCommand(const char* args)
{
    uint32 titles = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    chr->SetUInt32Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    SendSysMessage(LANG_DONE);

    return true;
}

bool ChatHandler::HandleModifyHPCommand(const char* args)
{
    //    char* pHp = strtok((char*)args, " ");
    //    if (!pHp)
    //        return false;

    //    char* pHpMax = strtok(NULL, " ");
    //    if (!pHpMax)
    //        return false;

    //    int32 hpm = atoi(pHpMax);
    //    int32 hp = atoi(pHp);

    int32 hp = atoi((char*)args);
    int32 hpm = atoi((char*)args);

    if (hp <= 0 || hpm <= 0 || hpm < hp)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_HP, hp, hpm, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_HP_CHANGED, m_session->GetPlayer()->GetName(), hp, hpm);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetMaxHealth( hpm );
    chr->SetHealth( hp );

    return true;
}

bool ChatHandler::HandleModifyManaCommand(const char* args)
{
    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);
    int32 mana = atoi((char*)args);
    int32 manam = atoi((char*)args);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_MANA, mana, manam, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_MANA_CHANGED, m_session->GetPlayer()->GetName(), mana, manam);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetMaxPower(POWER_MANA,manam );
    chr->SetPower(POWER_MANA, mana );

    return true;
}

bool ChatHandler::HandleModifyEnergyCommand(const char* args)
{
    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);
    int32 mana = atoi((char*)args)*10;
    int32 manam = atoi((char*)args)*10;

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ENERGY, mana/10, manam/10, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_ENERGY_CHANGED, m_session->GetPlayer()->GetName(), mana/10, manam/10);
    FillSystemMessageData(&data, m_session, buf);
    chr->GetSession()->SendPacket(&data);

    chr->SetMaxPower(POWER_ENERGY,manam );
    chr->SetPower(POWER_ENERGY, mana );

    sLog.outDetail(LANG_CURRENT_ENERGY,chr->GetMaxPower(POWER_ENERGY));

    return true;
}

bool ChatHandler::HandleModifyRageCommand(const char* args)
{
    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);
    int32 mana = atoi((char*)args)*10;
    int32 manam = atoi((char*)args)*10;

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_RAGE, mana/10, manam/10, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_RAGE_CHANGED, m_session->GetPlayer()->GetName(), mana/10, manam/10);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetMaxPower(POWER_RAGE,manam );
    chr->SetPower(POWER_RAGE, mana );

    return true;
}

bool ChatHandler::HandleModifyFactionCommand(const char* args)
{
    uint32 factionid;
    uint32 flag;
    uint32  npcflag;
    uint32 dyflag;

    char* pfactionid = strtok((char*)args, " ");

    Creature* chr = getSelectedCreature();
    if(!chr)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(!pfactionid)
    {
        if(chr)
        {
            factionid = chr->getFaction();
            flag      = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
            npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
            dyflag    = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            PSendSysMessage(LANG_CURRENT_FACTION,chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);
        }
        return true;
    }

    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    factionid = atoi(pfactionid);

    char *pflag = strtok(NULL, " ");
    if (!pflag)
        flag = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
    else
        flag = atoi(pflag);

    char* pnpcflag = strtok(NULL, " ");
    if(!pnpcflag)
        npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
    else
        npcflag = atoi(pnpcflag);

    char* pdyflag = strtok(NULL, " ");
    if(!pdyflag)
        dyflag   = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
    else
        dyflag = atoi(pdyflag);

    if(!sFactionTemplateStore.LookupEntry(factionid))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionid);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FACTION, chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);

    //sprintf((char*)buf,"%s changed your Faction to %i.", m_session->GetPlayer()->GetName(), factionid);
    //FillSystemMessageData(&data, m_session, buf);

    //chr->GetSession()->SendPacket(&data);

    chr->setFaction(factionid);
    chr->SetUInt32Value(UNIT_FIELD_FLAGS,flag);
    chr->SetUInt32Value(UNIT_NPC_FLAGS,npcflag);
    chr->SetUInt32Value(UNIT_DYNAMIC_FLAGS,dyflag);

    return true;
}

bool ChatHandler::HandleModifySpellCommand(const char* args)
{
    char* pspellflatid = strtok((char*)args, " ");
    if (!pspellflatid)
        return false;

    char* pop = strtok(NULL, " ");
    if (!pop)
        return false;

    char* pval = strtok(NULL, " ");
    if (!pval)
        return false;

    uint16 mark;

    char* pmark = strtok(NULL, " ");

    uint8 spellflatid = atoi(pspellflatid);
    uint8 op   = atoi(pop);
    uint16 val = atoi(pval);
    if(!pmark)
        mark = 65535;
    else
        mark = atoi(pmark);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPELLFLATID, spellflatid, val, mark, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_SPELLFLATID_CHANGED, m_session->GetPlayer()->GetName(), spellflatid, val, mark);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_SET_FLAT_SPELL_MODIFIER, (1+1+2+2));
    data << uint8(spellflatid);
    data << uint8(op);
    data << uint16(val);
    data << uint16(mark);
    chr->GetSession()->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleModifyTalentCommand (const char* args)
{
    if (!*args)
        return false;

    int tp = atoi((char*)args);
    if (tp>0)
    {
        Player* player = getSelectedPlayer();
        if(!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            return true;
        }
        player->SetFreeTalentPoints(tp);
        return true;
    }
    return false;
}

bool ChatHandler::HandleTaxiCheatCommand(const char* args)
{
    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if (flag != 0)
    {
        chr->SetTaxiCheater(true);
        PSendSysMessage(LANG_YOU_GIVE_TAXIS, chr->GetName());

        if(chr != m_session->GetPlayer())
        {
            WorldPacket data;
            char buf[256];
            sprintf((char*)buf,LANG_YOURS_TAXIS_ADDED, m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        chr->SetTaxiCheater(false);
        PSendSysMessage(LANG_YOU_REMOVE_TAXIS, chr->GetName());

        if(chr != m_session->GetPlayer())
        {
            WorldPacket data;
            char buf[256];
            sprintf((char*)buf,LANG_YOURS_TAXIS_REMOVED, m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);
        }
    }

    return true;
}

bool ChatHandler::HandleModifyASpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float ASpeed = (float)atof((char*)args);

    if (ASpeed > 10 || ASpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ASPEED, ASpeed, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_ASPEED_CHANGED, m_session->GetPlayer()->GetName(), ASpeed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetSpeed(MOVE_WALK,    ASpeed,true);
    chr->SetSpeed(MOVE_RUN,     ASpeed,true);
    chr->SetSpeed(MOVE_WALKBACK,ASpeed,true);
    chr->SetSpeed(MOVE_SWIM,    ASpeed,true);
    chr->SetSpeed(MOVE_SWIMBACK,ASpeed,true);
    //chr->SetSpeed(MOVE_TURN,    ASpeed,true);
    chr->SetSpeed(MOVE_FLY,     ASpeed,true);
    return true;
}

bool ChatHandler::HandleModifySpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float Speed = (float)atof((char*)args);

    if (Speed > 10 || Speed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPEED, Speed, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_SPEED_CHANGED, m_session->GetPlayer()->GetName(), Speed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetSpeed(MOVE_RUN,Speed,true);

    return true;
}

bool ChatHandler::HandleModifySwimCommand(const char* args)
{
    if (!*args)
        return false;

    float Swim = (float)atof((char*)args);

    if (Swim > 10 || Swim < 0.01)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SWIM_SPEED, Swim, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_SWIM_SPEED_CHANGED, m_session->GetPlayer()->GetName(), Swim);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetSpeed(MOVE_SWIM,Swim,true);

    return true;
}

bool ChatHandler::HandleModifyBWalkCommand(const char* args)
{
    if (!*args)
        return false;

    float BSpeed = (float)atof((char*)args);

    if (BSpeed > 10 || BSpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_BACK_SPEED, BSpeed, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_BACK_SPEED_CHANGED, m_session->GetPlayer()->GetName(), BSpeed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetSpeed(MOVE_WALKBACK,BSpeed,true);

    return true;
}

bool ChatHandler::HandleModifyFlyCommand(const char* args)
{
    if (!*args)
        return false;

    float FSpeed = (float)atof((char*)args);

    if (FSpeed > 10 || FSpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FLY_SPEED, FSpeed, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_FLY_SPEED_CHANGED, m_session->GetPlayer()->GetName(), FSpeed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetSpeed(MOVE_FLY,FSpeed,true);

    return true;
}

bool ChatHandler::HandleModifyScaleCommand(const char* args)
{
    if (!*args)
        return false;

    float Scale = (float)atof((char*)args);
    if (Scale > 3 || Scale <= 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SIZE, Scale, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_YOURS_SIZE_CHANGED, m_session->GetPlayer()->GetName(), Scale);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetFloatValue(OBJECT_FIELD_SCALE_X, Scale);

    return true;
}

bool ChatHandler::HandleModifyMountCommand(const char* args)
{
    if(!*args)
        return false;

    uint16 mId = 1147;
    float speed = (float)15;
    uint32 num = 0;

    num = atoi((char*)args);
    switch(num)
    {
        case 1:
            mId=14340;
            break;
        case 2:
            mId=4806;
            break;
        case 3:
            mId=6471;
            break;
        case 4:
            mId=12345;
            break;
        case 5:
            mId=6472;
            break;
        case 6:
            mId=6473;
            break;
        case 7:
            mId=10670;
            break;
        case 8:
            mId=10719;
            break;
        case 9:
            mId=10671;
            break;
        case 10:
            mId=10672;
            break;
        case 11:
            mId=10720;
            break;
        case 12:
            mId=14349;
            break;
        case 13:
            mId=11641;
            break;
        case 14:
            mId=12244;
            break;
        case 15:
            mId=12242;
            break;
        case 16:
            mId=14578;
            break;
        case 17:
            mId=14579;
            break;
        case 18:
            mId=14349;
            break;
        case 19:
            mId=12245;
            break;
        case 20:
            mId=14335;
            break;
        case 21:
            mId=207;
            break;
        case 22:
            mId=2328;
            break;
        case 23:
            mId=2327;
            break;
        case 24:
            mId=2326;
            break;
        case 25:
            mId=14573;
            break;
        case 26:
            mId=14574;
            break;
        case 27:
            mId=14575;
            break;
        case 28:
            mId=604;
            break;
        case 29:
            mId=1166;
            break;
        case 30:
            mId=2402;
            break;
        case 31:
            mId=2410;
            break;
        case 32:
            mId=2409;
            break;
        case 33:
            mId=2408;
            break;
        case 34:
            mId=2405;
            break;
        case 35:
            mId=14337;
            break;
        case 36:
            mId=6569;
            break;
        case 37:
            mId=10661;
            break;
        case 38:
            mId=10666;
            break;
        case 39:
            mId=9473;
            break;
        case 40:
            mId=9476;
            break;
        case 41:
            mId=9474;
            break;
        case 42:
            mId=14374;
            break;
        case 43:
            mId=14376;
            break;
        case 44:
            mId=14377;
            break;
        case 45:
            mId=2404;
            break;
        case 46:
            mId=2784;
            break;
        case 47:
            mId=2787;
            break;
        case 48:
            mId=2785;
            break;
        case 49:
            mId=2736;
            break;
        case 50:
            mId=2786;
            break;
        case 51:
            mId=14347;
            break;
        case 52:
            mId=14346;
            break;
        case 53:
            mId=14576;
            break;
        case 54:
            mId=9695;
            break;
        case 55:
            mId=9991;
            break;
        case 56:
            mId=6448;
            break;
        case 57:
            mId=6444;
            break;
        case 58:
            mId=6080;
            break;
        case 59:
            mId=6447;
            break;
        case 60:
            mId=4805;
            break;
        case 61:
            mId=9714;
            break;
        case 62:
            mId=6448;
            break;
        case 63:
            mId=6442;
            break;
        case 64:
            mId=14632;
            break;
        case 65:
            mId=14332;
            break;
        case 66:
            mId=14331;
            break;
        case 67:
            mId=8469;
            break;
        case 68:
            mId=2830;
            break;
        case 69:
            mId=2346;
            break;
        default:
            SendSysMessage(LANG_NO_MOUNT);
            return true;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    PSendSysMessage(LANG_YOU_GIVE_MOUNT, chr->GetName());

    WorldPacket data;
    char buf[256];
    sprintf((char*)buf,LANG_MOUNT_GIVED, m_session->GetPlayer()->GetName());
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_FLAGS , 0x001000 );
    chr->Mount(mId);

    data.Initialize( SMSG_FORCE_RUN_SPEED_CHANGE, (8+4+1+4) );
    data.append(chr->GetPackGUID());
    data << (uint32)0;
    data << (uint8)0; //new 2.1.0
    data << float(speed);
    chr->SendMessageToSet( &data, true );

    data.Initialize( SMSG_FORCE_SWIM_SPEED_CHANGE, (8+4+4) );
    data.append(chr->GetPackGUID());
    data << (uint32)0;
    data << float(speed);
    chr->SendMessageToSet( &data, true );

    return true;
}

bool ChatHandler::HandleModifyMoneyCommand(const char* args)
{
    if (!*args)
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    int32 addmoney = atoi((char*)args);

    uint32 moneyuser = m_session->GetPlayer()->GetMoney();

    if(addmoney < 0)
    {
        int32 newmoney = moneyuser + addmoney;

        sLog.outDetail(LANG_CURRENT_MONEY, moneyuser, addmoney, newmoney);
        if(newmoney <= 0 )
        {
            PSendSysMessage(LANG_YOU_TAKE_ALL_MONEY, chr->GetName());

            WorldPacket data;
            char buf[256];
            sprintf((char*)buf,LANG_YOURS_ALL_MONEY_GONE, m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);

            chr->SetMoney(0);
        }
        else
        {
            PSendSysMessage(LANG_YOU_TAKE_MONEY, abs(addmoney), chr->GetName());

            WorldPacket data;
            char buf[256];
            sprintf((char*)buf,LANG_YOURS_MONEY_TAKEN, m_session->GetPlayer()->GetName(), abs(addmoney));
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);

            chr->SetMoney( newmoney );
        }
    }
    else
    {
        PSendSysMessage(LANG_YOU_GIVE_MONEY, addmoney, chr->GetName());

        WorldPacket data;
        char buf[256];
        sprintf((char*)buf,LANG_YOURS_MONEY_GIVEN, m_session->GetPlayer()->GetName(), addmoney);
        FillSystemMessageData(&data, m_session, buf);
        chr->GetSession()->SendPacket(&data);

        chr->ModifyMoney( addmoney );
    }

    sLog.outDetail(LANG_NEW_MONEY, moneyuser, addmoney, chr->GetMoney() );

    return true;
}

bool ChatHandler::HandleModifyBitCommand(const char* args)
{
    if( !*args )
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = atoi(pField);
    uint32 bit   = atoi(pBit);

    if (field < 1 || field >= PLAYER_END)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    if ( chr->HasFlag( field, (1<<(bit-1)) ) )
    {
        chr->RemoveFlag( field, (1<<(bit-1)) );
        PSendSysMessage(LANG_REMOVE_BIT, bit, field);
    }
    else
    {
        chr->SetFlag( field, (1<<(bit-1)) );
        PSendSysMessage(LANG_SET_BIT, bit, field);
    }

    return true;
}

bool ChatHandler::HandleTeleCommand(const char * args)
{
    Player* _player = m_session->GetPlayer();

    if(_player->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        return true;
    }

    char* cId = extractKeyFromLink((char*)args,"Htele");           // string or [name] Shift-click form |color|Htele:name|h[name]|h|r
    if(!cId)
        return false;

    std::string name = cId;
    sDatabase.escape_string(name);

    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `game_tele` WHERE `name` = '%s'",name.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        return true;
    }
    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        return true;
    }

    _player->SetRecallPosition(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleLookupTeleCommand(const char * args)
{
    QueryResult *result;
    if(!*args)
    {
        SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
        return true;
    }
    char const* str = strtok((char*)args, " ");
    if(!str)
        return false;

    std::string namepart = str;
    sDatabase.escape_string(namepart);
    result = sDatabase.PQuery("SELECT `name` FROM `game_tele` WHERE `name` LIKE '%%%s%%'",namepart.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOREQUEST);
        return true;
    }
    std::string reply;
    for (uint64 i=0; i < result->GetRowCount(); i++)
    {
        Field *fields = result->Fetch();
        reply += "  |cffffffff|Htele:";
        reply += fields[0].GetCppString();
        reply += "|h[";
        reply += fields[0].GetCppString();
        reply += "]|h|r\n";
        result->NextRow();
    }
    delete result;

    if(reply.empty())
        SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
    else
    {
        reply = LANG_COMMAND_TELE_LOCATION + reply;
        SendSysMultilineMessage(reply.c_str());
    }
    return true;
}

bool ChatHandler::HandleWhispersCommand(const char* args)
{
    char* px = strtok((char*)args, " ");

    // ticket<end>
    if (!px)
    {
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, m_session->GetPlayer()->isAcceptWhispers() ?  LANG_ON : LANG_OFF);
        return true;
    }

    // ticket on
    if(strncmp(px,"on",3) == 0)
    {
        m_session->GetPlayer()->SetAcceptWhispers(true);
        SendSysMessage(LANG_COMMAND_WHISPERON);
        return true;
    }

    // ticket off
    if(strncmp(px,"off",4) == 0)
    {
        m_session->GetPlayer()->SetAcceptWhispers(false);
        SendSysMessage(LANG_COMMAND_WHISPEROFF);
        return true;
    }

    return false;
}

bool ChatHandler::HandlePlaySoundCommand(const char* args)
{
    // USAGE: .playsound #soundid
    // #soundid - ID decimal number from SoundEntries.dbc (1 column)
    // this file have about 5000 sounds.
    // In this realisation only caller can hear this sound.
    if( *args )
    {
        int dwSoundId = atoi((char*)args);
        if( dwSoundId >= 0 )
        {
            WorldPacket data(SMSG_PLAY_OBJECT_SOUND,4+8);
            data << uint32(dwSoundId) << m_session->GetPlayer()->GetGUID();
            m_session->SendPacket(&data);

            sLog.outDebug("Player %s use command .playsound with #soundid=%u", m_session->GetPlayer()->GetName(), dwSoundId);
            PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
            return true;
        }
    }

    SendSysMessage(LANG_BAD_VALUE);
    return false;
}

bool ChatHandler::HandleSaveAllCommand(const char* args)
{
    ObjectAccessor::Instance().SaveAllPlayers();
    SendSysMessage(LANG_PLAYERS_SAVED);
    return true;
}

bool ChatHandler::HandleSendMailCommand(const char* args)
{
    if(!*args)
        return false;

    char* pName = strtok((char*)args, " ");
    char* msgSubject = strtok(NULL, " ");
    char* msgText = strtok(NULL, "");

    if (!msgText)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check

    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;
    
    normalizePlayerName(name);

    uint32 receiver_guid = objmgr.GetPlayerGUIDByName(name);
    uint32 mailId = objmgr.GenerateMailID();
    uint32 sender_guid = m_session->GetPlayer()->GetGUID();
    time_t dtime = time(NULL);
    time_t etime = dtime + (30 * DAY);
    uint32 messagetype = MAIL_GM;
    uint32 itemTextId = 0;
    if (text.size() > 0)
    {
        itemTextId = objmgr.CreateItemText( text );
    }

    Player *receiver = objmgr.GetPlayer((uint32) receiver_guid);
    if(receiver)
        receiver->CreateMail(mailId,messagetype,sender_guid,subject.c_str(),itemTextId,0,0,(uint64)etime,(uint64)dtime,0,0,0,0);

    sDatabase.escape_string(subject);
    sDatabase.PExecute("INSERT INTO `mail` (`id`,`messageType`,`sender`,`receiver`,`subject`,`itemTextId`,`item_guid`,`item_template`,`expire_time`,`deliver_time`,`money`,`cod`,`checked`) "
        "VALUES ('%u', '%u', '%u', '%u', '%s', '%u', '0', '0', '" I64FMTD "','" I64FMTD "', '0', '0', '%d')",
        mailId, messagetype, sender_guid, receiver_guid, subject.c_str(), itemTextId, (uint64)etime, (uint64)dtime, NOT_READ);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

bool ChatHandler::HandleNameTeleCommand(const char * args)
{
    char* pName = strtok((char*)args, " ");
    char* loc = strtok(NULL, "");

    if(!pName || !loc)
        return false;

    std::string name = pName;
    std::string location = loc;

    normalizePlayerName(name);
    sDatabase.escape_string(name);

    QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`orientation`,`map` FROM `game_tele` WHERE `name` = '%s'",location.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        return true;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        return true;
    }

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {

        if(chr->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
            return true;
        }

        if(chr->isInFlight())
        {
            PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
            return true;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, chr->GetName(),"", location.c_str());

        if (m_session->GetPlayer()->isVisibleFor(chr))
        {
            WorldPacket data;
            char buf0[256];
            snprintf((char*)buf0,256,LANG_TELEPORTED_TO_BY, m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf0);
            chr->GetSession()->SendPacket( &data );
        }

        chr->SetRecallPosition(chr->GetMapId(),chr->GetPositionX(),chr->GetPositionY(),chr->GetPositionZ(),chr->GetOrientation());

        chr->TeleportTo(mapid,x,y,z,chr->GetOrientation());
    }
    else if (uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str()))
    {
        PSendSysMessage(LANG_TELEPORTING_TO, name.c_str(), LANG_OFFLINE, location.c_str());
        Player::SavePositionInDB(mapid,x,y,z,ort,guid);
    }
    else
        PSendSysMessage(LANG_NO_PLAYER, name.c_str());

    return true;
}
