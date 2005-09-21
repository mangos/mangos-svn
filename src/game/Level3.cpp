/* Level3.cpp
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

/////////////////////////////////////////////////
//  Admin Chat Commands
//

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "GameObject.h"
#include "Chat.h"
#include "Log.h"

bool ChatHandler::HandleSecurityCommand(const char* args)
{
    WorldPacket data;

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* pgm = strtok(NULL, " ");
    if (!pgm)
        return false;

    int8 gm = (uint8) atoi(pgm);
    if ( gm < 0 || gm > 5)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    Player *chr = objmgr.GetPlayer(args);
    if (chr)
    {
        // send message to user
        sprintf((char*)buf,"You change security level of %s to %i.", chr->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        // send message to player
        sprintf((char*)buf,"%s changed your security level to %i.", m_session->GetPlayer()->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);

        chr->GetSession()->SendPacket(&data);
        chr->GetSession()->SetSecurity(gm);

        char sql[512];
        sprintf(sql, "UPDATE accounts SET gm = '%i' WHERE acct = '%u'", gm, chr->GetSession()->GetAccountId());
        sDatabase.Execute( sql );
    }
    else
    {
        sprintf((char*)buf,"Player (%s) does not exist or is not logged in.", pName);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
    }

    return true;
}


bool ChatHandler::HandleWorldPortCommand(const char* args)
{
    char* pContinent = strtok((char*)args, " ");
    if (!pContinent)
        return false;

    char* px = strtok(NULL, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    // LINA
    smsg_NewWorld(m_session, atoi(pContinent), (float)atof(px), (float)atof(py), (float)atof(pz));

    return true;
}


bool ChatHandler::HandleAllowMovementCommand(const char* args)
{
    WorldPacket data;
    if(sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        FillSystemMessageData(&data, m_session, "Creature Movement Disabled.");
    }
    else
    {

        sWorld.SetAllowMovement(true);
        FillSystemMessageData(&data, m_session, "Creature Movement Enabled.");
    }

    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleAddSpiritCommand(const char* args)
{
    Log::getSingleton( ).outDetail("Spawning Spirit Healers\n");

    std::stringstream query;
    Creature* pCreature;
    UpdateMask unitMask;
    WorldPacket data;

    query << "select X,Y,Z,F,name_id,mapId,zoneId,faction_id from spirithealers";
    QueryResult *result = sDatabase.Query( query.str( ).c_str( ) );

    if(!result)
    {
        FillSystemMessageData(&data, m_session, "No spirit healers in db, exiting.");
        m_session->SendPacket( &data );

        return true;
    }

    uint32 name;
    do
    {
        Field* fields = result->Fetch();

        name = fields[4].GetUInt32();
        Log::getSingleton( ).outDetail("%s name is %d\n", fields[4].GetString(), name);

        pCreature = new Creature();

        pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), objmgr.GetCreatureName(name)->Name.c_str(), fields[5].GetUInt16(),
            fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat());

        pCreature->SetZoneId( fields[6].GetUInt16() );
        pCreature->SetUInt32Value( OBJECT_FIELD_ENTRY, name );
        pCreature->SetFloatValue( OBJECT_FIELD_SCALE_X, 1.0f );
        pCreature->SetUInt32Value( UNIT_FIELD_DISPLAYID, 5233 );
        pCreature->SetUInt32Value( UNIT_NPC_FLAGS , 1 );
        pCreature->SetUInt32Value( UNIT_FIELD_FACTIONTEMPLATE , fields[7].GetUInt32() );
        pCreature->SetUInt32Value( UNIT_FIELD_HEALTH, 100 + 30*(60) );
        pCreature->SetUInt32Value( UNIT_FIELD_MAXHEALTH, 100 + 30*(60) );
        pCreature->SetUInt32Value( UNIT_FIELD_LEVEL , 60 );
        pCreature->SetFloatValue( UNIT_FIELD_COMBATREACH , 1.5f );
        pCreature->SetFloatValue( UNIT_FIELD_MAXDAMAGE ,  5.0f );
        pCreature->SetFloatValue( UNIT_FIELD_MINDAMAGE , 8.0f );
        pCreature->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME, 1900 );
        pCreature->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME+1, 2000 );
        pCreature->SetFloatValue( UNIT_FIELD_BOUNDINGRADIUS, 2.0f );
        Log::getSingleton( ).outError("AddObject at Level3.cpp line 172");
        objmgr.AddObject(pCreature);
        pCreature->PlaceOnMap();

        pCreature->SaveToDB();
    }
    while( result->NextRow() );

    delete result;

    return true;
}


bool ChatHandler::HandleMoveCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);

    MovePlayer(m_session, x, y, z);

    return true;
}


bool ChatHandler::HandleLearnCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 spell = atol((char*)args);

    if (m_session->GetPlayer()->HasSpell(spell))  // check to see if char already learned spell
    {
        FillSystemMessageData(&data, m_session, "You already know that spell.");
        m_session->SendPacket(&data);
        return true;
    }

    data.Initialize( SMSG_LEARNED_SPELL );
    data << (uint32)spell;
    m_session->SendPacket( &data );
    m_session->GetPlayer()->addSpell((uint16)spell);

    return true;
}


bool ChatHandler::HandleObjectCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 display_id = atoi((char*)args);
    //char* name = strtok((char*)args, " ");
    char* safe = strtok((char*)args, " ");

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    GameObject* pGameObj = new GameObject();
    pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id, chr->GetMapId(), x, y, z, o);	
    pGameObj->SetZoneId(chr->GetZoneId());
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 19);
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 252");
    objmgr.AddObject(pGameObj);
    pGameObj->PlaceOnMap();
    
    if(strcmp(safe,"true") == 0)
	pGameObj->SaveToDB();
    
    return true;
}


bool ChatHandler::HandleAddWeaponCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

    Creature * pCreature = objmgr.GetCreature(guid);
    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
        m_session->SendPacket( &data );
        return true;
    }

    char* pSlotID = strtok((char*)args, " ");
    if (!pSlotID)
        return false;

    char* pItemID = strtok(NULL, " ");
    if (!pItemID)
        return false;

    uint32 ItemID = atoi(pItemID);
    uint32 SlotID = atoi(pSlotID);

    ItemPrototype* tmpItem = objmgr.GetItemPrototype(ItemID);

    bool added = false;
    std::stringstream sstext;
    if(tmpItem)
    {
        switch(SlotID)
        {
            case 1:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
                added = true;
                break;
            case 2:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
                added = true;
                break;
            case 3:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
                added = true;
                break;
            default:
                sstext << "Item Slot '" << SlotID << "' doesn't exist." << '\0';
                added = false;
                break;
        }
        if(added)
        {
            sstext << "Item '" << ItemID << "' '" << tmpItem->Name1 << "' Added to Slot " << SlotID << '\0';
        }
    }
    else
    {
        sstext << "Item '" << ItemID << "' Not Found in Database." << '\0';
        return true;
    }
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleGameObjectCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;

    uint32 display_id = atoi((char*)args);
    if(!display_id) return false;

    uint16 typesid = atoi((char*)args);
    if(!typesid) return false;

    uint16 factionid = atoi((char*)args);
    if(!factionid) return false;

    uint32 fieldentry = atoi((char*)args);
    if(!fieldentry) return false;

    // char* name = strtok((char*)args, " ");
    // char* safe = strtok((char*)args, " ");

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();
    uint32 guidlow = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);
    Log::getSingleton( ).outError("GameObjectGUIDlow %u",guidlow);

    GameObject* pGameObj = new GameObject();

    // uint32 guidlow, uint16 display_id, uint8 state, uint32 obj_field_entry, uint8 scale, uint16 type, uint16 faction,  float x, float y, float z, float ang
    pGameObj->Create(guidlow, display_id, chr->GetMapId(), x, y, z, o);
    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, fieldentry);
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, typesid);
    pGameObj->SetZoneId(chr->GetZoneId());
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 252");
    objmgr.AddObject(pGameObj);
    pGameObj->PlaceOnMap();

    //if(strcmp(safe,"true") == 0)
    pGameObj->SaveToDB();

    return true;
}


bool ChatHandler::HandleAnimCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 anim_id = atoi((char*)args);

    data.Initialize( SMSG_EMOTE );
    data << anim_id << m_session->GetPlayer( )->GetGUID();
    WPAssert(data.size() == 12);
    m_session->GetPlayer()->SendMessageToSet(&data, true);

    return true;
}


bool ChatHandler::HandleStandStateCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 anim_id = atoi((char*)args);
    m_session->GetPlayer( )->SetUInt32Value( UNIT_NPC_EMOTESTATE , anim_id );

    return true;
}


bool ChatHandler::HandleDieCommand(const char* args)
{
    Player* SelectedPlayer;
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if(guid == 0)
    {
        SelectedPlayer = m_session->GetPlayer();
    }
    else
    {
        SelectedPlayer = objmgr.GetPlayer(guid);
    }
    if(!SelectedPlayer)
    {
        SelectedPlayer = m_session->GetPlayer();
    }

    SelectedPlayer->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
    SelectedPlayer->setDeathState(JUST_DIED);
    return true;
}


bool ChatHandler::HandleReviveCommand(const char* args)
{
    Player* SelectedPlayer;
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if(guid == 0)
    {
        SelectedPlayer = m_session->GetPlayer();
    }
    else
    {
        SelectedPlayer = objmgr.GetPlayer(guid);
    }
    if(!SelectedPlayer)
    {
        SelectedPlayer = m_session->GetPlayer();
    }

    SelectedPlayer->SetMovement(MOVE_LAND_WALK);
    SelectedPlayer->SetMovement(MOVE_UNROOT);
    SelectedPlayer->SetPlayerSpeed(RUN, (float)7.5, true);
    SelectedPlayer->SetPlayerSpeed(SWIM, (float)4.9, true);

    SelectedPlayer->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    SelectedPlayer->ResurrectPlayer();
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(SelectedPlayer->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
    SelectedPlayer->SpawnCorpseBones();
    return true;
}


bool ChatHandler::HandleMorphCommand(const char* args)
{
    if (!*args)
        return false;

    uint16 display_id = (uint16)atoi((char*)args);

    m_session->GetPlayer()->SetUInt32Value(UNIT_FIELD_DISPLAYID, display_id);
    // m_session->GetPlayer()->UpdateObject( );
    // m_session->GetPlayer()->SendMessageToSet(&data, true);

    return true;
}


bool ChatHandler::HandleAuraCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 aura_id = atoi((char*)args);

    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURA, aura_id );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAFLAGS, 0x0000000d );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURA+32, aura_id );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURALEVELS+8, 0xeeeeee00 );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeee00 );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAFLAGS+4, 0x0000000d );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURASTATE, 0x00000002 );
    // m_session->GetPlayer()->UpdateObject( );

    return true;
}


bool ChatHandler::HandleAddGraveCommand(const char* args)
{
    //QueryResult *result;
    std::stringstream ss;
  //  GraveyardTeleport *pGrave;

   // ss.rdbuf()->str("");
  //  pGrave = new GraveyardTeleport;

   // result = sDatabase.Query( "SELECT MAX(ID) FROM graveyards" );

  /*  if( result )
    {
        pGrave->ID = (*result)[0].GetUInt32()+1;

        delete result;
    }*/
   // pGrave->X = m_session->GetPlayer()->GetPositionX();
  //  pGrave->Y = m_session->GetPlayer()->GetPositionY();
  //  pGrave->Z = m_session->GetPlayer()->GetPositionZ();
    //pGrave->O = m_session->GetPlayer()->GetOrientation();
  //  pGrave->ZoneId = m_session->GetPlayer()->GetZoneId();
 //   pGrave->MapId = m_session->GetPlayer()->GetMapId();
  //  pGrave->FactionID = 0;

/*    ss << "INSERT INTO graveyards ( X, Y, Z, O, zoneId, mapId) VALUES ("
        << pGrave->X << ", "
        << pGrave->Y << ", "
        << pGrave->Z << ", "
        << pGrave->O<< ", "
        << pGrave->ZoneId << ", "
        << pGrave->MapId << ")";*/
	
	 ss << "INSERT INTO graveyards ( X, Y, Z, mapId) VALUES ("
        << m_session->GetPlayer()->GetPositionX() << ", "
        << m_session->GetPlayer()->GetPositionY() << ", "
        << m_session->GetPlayer()->GetPositionZ() << ", "
        << m_session->GetPlayer()->GetMapId() << ")";

    sDatabase.Execute( ss.str( ).c_str( ) );

    //objmgr.AddGraveyard(pGrave);
    return true;
}


bool ChatHandler::HandleAddSHCommand(const char *args)
{
    WorldPacket data;

    // Create the requested monster
    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    Creature* pCreature = new Creature();

    pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), "Spirit Healer", chr->GetMapId(), x, y, z, o);
    pCreature->SetZoneId(chr->GetZoneId());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureName(pCreature->GetName(), 5233));
    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 5233);
    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, 33);
    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , 35);
    pCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 100);
    pCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 100);
    pCreature->SetUInt32Value(UNIT_FIELD_LEVEL, 60);
    pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, 768);
    pCreature->SetUInt32Value(UNIT_FIELD_AURA+0, 10848);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS+0, 0xEEEEEE3C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+0, 0xEEEEEE00);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS+0, 0x00000009);
    pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH , 1.5f);
    pCreature->SetFloatValue(UNIT_FIELD_MAXDAMAGE ,  5.0f);
    pCreature->SetFloatValue(UNIT_FIELD_MINDAMAGE , 8.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 1900);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, 2000);
    pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2.0f);
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 455");
    objmgr.AddObject(pCreature);
    pCreature->PlaceOnMap();
    pCreature->SaveToDB();

    std::stringstream ss,ss2,ss3;
    QueryResult *result;

    result = sDatabase.Query( "SELECT MAX(ID) FROM npc_gossip" );
    if( result )
    {
        ss2 << "INSERT INTO npc_gossip ( ID , NPC_GUID, GOSSIP_TYPE, TEXTID, OPTION_COUNT) VALUES ("
            << (*result)[0].GetUInt32()+1 << ", "
            << pCreature->GetGUIDLow() << ", "
            << 1 << ", "
            << 1 << ", "
            << 1 << ")";

        sDatabase.Execute( ss2.str( ).c_str( ) );
        delete result;
        result = NULL;

        result = sDatabase.Query( "SELECT MAX(ID) FROM npc_options" );
        if( result )
        {
            ss << "INSERT INTO npc_options ( `ID` , `GOSSIP_ID`, `TYPE`, `OPTION`, `NPC_TEXT_NEXTID`, `SPECIAL`) VALUES ("
                << (*result)[0].GetUInt32()+1 << ", "
                << (*result)[0].GetUInt32()+2 << ", "
                << 0 << ", '"
                << "Return me to life." << "', "
                << 0 << ", "
                << 2 << ")";

            sDatabase.Execute( ss.str( ).c_str( ) );
            delete result;
            result = NULL;
        }
        result = sDatabase.Query( "SELECT MAX(ID) FROM npc_text" );
        if( result )
        {
            ss3 << "INSERT INTO npc_text ( ID , TYPE_UNUSED, TEXT) VALUES ("
                << (*result)[0].GetUInt32()+1 << ", "
                << 0 << ", '"
                << "It is not yet your time. I shall aid your journey back to the realm of the living... For a price." << "')";
            sDatabase.Execute( ss3.str( ).c_str( ) );
            delete result;
            result = NULL;
        }
    }

    return true;
}


bool ChatHandler::HandleSpawnTransportCommand(const char* args)
{
    // don't need this anymore cuz its in the world.save or sql table
    return true;
}


bool ChatHandler::HandleEmoteCommand(const char* args)
{
    uint32 emote = atoi((char*)args);
    Player* chr = m_session->GetPlayer();
    if(chr->GetSelection() == 0)
        return false;

    Unit* target = objmgr.GetObject<Creature>(chr->GetSelection());
    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);

    return true;
}


bool ChatHandler::HandleNpcInfoCommand(const char* args)
{
    WorldPacket data;
    char buf[256];
    uint32 guid = m_session->GetPlayer()->GetSelection();
    uint32 factionid = 0, npcflags = 0, skinid = 0;
    Unit* target = objmgr.GetObject<Creature>(m_session->GetPlayer()->GetSelection());

    if(!target)
    {
        sChatHandler.FillSystemMessageData(&data, m_session, "Select something first.");
        m_session->SendPacket(&data);
        return true;
    }

    factionid = target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE);
    npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    skinid = target->GetUInt32Value(UNIT_FIELD_DISPLAYID);

    std::stringstream ss;
    ss << "SELECT * FROM creatures WHERE id = " << target->GetGUIDLow() << '\0';

    QueryResult *result = sDatabase.Query( ss.str().c_str() );

    Field *fields = result->Fetch();

    sprintf(buf,"Player selected guid: %d. faction %d. flag %d. name_id %d. skin_id %d.",guid,factionid,npcflags,fields[8].GetUInt32(),skinid);
    sChatHandler.FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket(&data);

    return true;
}


bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    if (flag != 0)
    {
        sprintf((char*)buf,"%s has explored all zones now.", chr->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has no more explored zones.", chr->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    if (flag != 0)
    {
        sprintf((char*)buf,"%s has explored all zones for you.",
            m_session->GetPlayer()->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has hidden all zones from you.",
            m_session->GetPlayer()->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    chr->GetSession()->SendPacket(&data);

    for (uint8 i=0; i<64; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0);
        }
    }

    return true;
}


bool ChatHandler::HandleHoverCommand(const char* args)
{
    WorldPacket data;

    data.Initialize(SMSG_MOVE_SET_HOVER);
    data << m_session->GetPlayer()->GetGUID();
    m_session->SendPacket( &data );

    WorldPacket data1;
    std::stringstream sstext;
    sstext << "Hover Enabled" << '\0';
    FillSystemMessageData(&data1, m_session, sstext.str().c_str());
    m_session->SendPacket( &data1 );

/*
SMSG_MOVE_SET_HOVER = 244,
SMSG_MOVE_UNSET_HOVER = 245,
*/

    return true;
}


bool ChatHandler::HandleLevelUpCommand(const char* args)
{
    uint32 curXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 givexp = nextLvlXP - curXP;

    uint32 points2 = m_session->GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS2);
    m_session->GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS2,points2+2);

    m_session->GetPlayer()->GiveXP(givexp,m_session->GetPlayer()->GetGUID());

    WorldPacket data;
    std::stringstream sstext;
    sstext << "You have been leveled Up" << '\0';
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    FillSystemMessageData(&data, m_session, "The area has been set as explored.");
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleHideAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    FillSystemMessageData(&data, m_session, "The area has been set as not explored.");
    m_session->SendPacket( &data );
    return true;
}
