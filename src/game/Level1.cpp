/* Level1.cpp
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
//  GM Chat Commands
//

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

#ifdef ENABLE_GRID_SYSTEM
#include "MapManager.h"
#include "ObjectAccessor.h"
#endif

bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    char pAnnounce[256];
    // Adds BROADCAST:
    sprintf((char*)pAnnounce, "BROADCAST: %s", args);
    sWorld.SendWorldText(pAnnounce);              // send message

    return true;
}


bool ChatHandler::HandleGMOnCommand(const char* args)
{
    uint32 newbytes = m_session->GetPlayer( )->GetUInt32Value(PLAYER_BYTES_2) | 0x8;
    m_session->GetPlayer( )->SetUInt32Value( PLAYER_BYTES_2, newbytes);

    return true;
}


bool ChatHandler::HandleGMOffCommand(const char* args)
{
    uint32 newbytes = m_session->GetPlayer( )->GetUInt32Value(PLAYER_BYTES_2) & ~(0x8);
    m_session->GetPlayer( )->SetUInt32Value( PLAYER_BYTES_2, newbytes);

    return true;
}


bool ChatHandler::HandleGPSCommand(const char* args)
{
    WorldPacket data;
    Object *obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
#ifndef ENABLE_GRID_SYSTEM
        if(!(obj = (Object*)objmgr.GetObject<Player>(guid)) && !(obj = (Object*)objmgr.GetObject<Creature>(guid)))
#else
        if(!(obj = (Object*)ObjectAccessor::Instance().FindPlayer(guid)) && !(obj = (Object*)ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),guid)))
#endif
        {
            FillSystemMessageData(&data, m_session, "You should select a character or a creature.");
            m_session->SendPacket( &data );
            return true;
        }
    }
    else
        obj = (Object*)m_session->GetPlayer();

    char buf[256];
    sprintf((char*)buf, "X: %f Y: %f Z %f Orientation: %f",
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(),
        obj->GetOrientation());

    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}


/*
bool ChatHandler::HandleKickCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    Player *chr = objmgr.getPlayer((char*)args);
    if (chr)
    {
        if(m_session->getAccountLvl() < chr->GetSession()->getAccountLvl())
        {
            // send message to user
            char buf[256];
            sprintf((char*)buf,"You try to kick %s.", chr->getName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );

            // send message to player
            char buf0[256];
            sprintf((char*)buf0,"%s try kick you.", m_session->GetPlayer()->getName());
            FillSystemMessageData(&data, m_session, buf0);
            sWorld.SendMessageToPlayer(&data, chr->GetGUIDLow());

            return true;
        }
        // send message to user
        char buf[256];
        sprintf((char*)buf,"You kick %s.", chr->getName());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        // send message to player
        char buf0[256];
        sprintf((char*)buf0,"%s kick you.", m_session->GetPlayer()->getName());
        FillSystemMessageData(&data, m_session, buf0);
        sWorld.SendMessageToPlayer(&data, chr->GetGUIDLow());

        sWorld.disconnect_client(reinterpret_cast < Server::nlink_client *> (chr->GetSession()->GetNLink()));
    }
    else
    {
        char buf[256];
        sprintf((char*)buf,"Player (%s) does not exist or is not logged in.", args);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
    }

    return true;
}
*/

bool ChatHandler::HandleSummonCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    Player *chr = objmgr.GetPlayer(args);
    if (chr)
    {
        // send message to user
        char buf[256];
        char buf0[256];
        if(chr->IsBeingTeleported()==true)
        {
            sprintf((char*)buf,"%s is already being teleported.", chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );
            return true;
        }
        sprintf((char*)buf,"You are summoning %s.", chr->GetName());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        // send message to player
        sprintf((char*)buf0,"You are being summoned by %s.", m_session->GetPlayer()->GetName());
        FillSystemMessageData(&data, m_session, buf0);
        chr->GetSession()->SendPacket( &data );

        smsg_NewWorld(chr->GetSession(),
            m_session->GetPlayer()->GetMapId(),
            m_session->GetPlayer()->GetPositionX(),
            m_session->GetPlayer()->GetPositionY(),
            m_session->GetPlayer()->GetPositionZ());
    }
    else
    {
        char buf[256];
        sprintf((char*)buf,"Player (%s) does not exist or is not logged in.", args);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
    }
    return true;
}


bool ChatHandler::HandleAppearCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    Player *chr = objmgr.GetPlayer(args);
    if (chr)
    {
        char buf[256];
        if(chr->IsBeingTeleported()==true)
        {
            sprintf((char*)buf,"%s is already being teleported.", chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );
            return true;
        }
        // -- europa
        sprintf((char*)buf,"Appearing at %s's location.", chr->GetName());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        char buf0[256];
        sprintf((char*)buf0,"%s is appearing to your location.", m_session->GetPlayer()->GetName());
        FillSystemMessageData(&data, m_session, buf0);

        chr->GetSession()->SendPacket(&data);

        smsg_NewWorld(m_session, chr->GetMapId(), chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ());
    }
    else
    {
        char buf[256];
        sprintf((char*)buf,"Player (%s) does not exist or is not logged in.", args);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
    }

    return true;
}


bool ChatHandler::HandleRecallCommand(const char* args)
{
    if(!*args)
        return false;

    if (strncmp((char*)args,"sunr",5)==0)
        smsg_NewWorld(m_session, 1, -180.949f, -296.467f, 11.5384f);
    else if (strncmp((char*)args,"thun",5)==0)
        smsg_NewWorld(m_session, 1, -1196.22f, 29.0941f, 176.949f);
    else if (strncmp((char*)args,"cross",6)==0)
        smsg_NewWorld(m_session, 1, -443.128f, -2598.87f, 96.2114f);
    else if (strncmp((char*)args,"ogri",5)==0)
        smsg_NewWorld(m_session, 1, 1676.21f, -4315.29f, 61.5293f);
    else if (strncmp((char*)args,"neth",5)==0)
        smsg_NewWorld(m_session, 0, -10996.9f, -3427.67f, 61.996f);
    else if (strncmp((char*)args,"thel",5)==0)
        smsg_NewWorld(m_session, 0, -5395.57f, -3015.79f, 327.58f);
    else if (strncmp((char*)args,"storm",6)==0)
        smsg_NewWorld(m_session, 0, -8913.23f, 554.633f, 93.7944f);
    else if (strncmp((char*)args,"iron",5)==0)
        smsg_NewWorld(m_session, 0, -4981.25f, -881.542f, 501.66f);
    else if (strncmp((char*)args,"under",6)==0)
        smsg_NewWorld(m_session, 0, 1586.48f, 239.562f, -52.149f);
    else if (strncmp((char*)args,"darr",5)==0)
        smsg_NewWorld(m_session, 1, 10037.6f, 2496.8f, 1318.4f);
    else
        return false;

    return true;
}


bool ChatHandler::HandleModifyHPCommand(const char* args)
{
    WorldPacket data;

    // change level of char
    char* pHp = strtok((char*)args, " ");
    if (!pHp)
        return false;

    char* pHpMax = strtok(NULL, " ");
    if (!pHpMax)
        return false;

    int32 hpm = atoi(pHpMax);
    int32 hp = atoi(pHp);

    if (hp <= 0 || hpm <= 0 || hpm < hp)
    {
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You change the HP to %i/%i of %s.", hp, hpm, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s changed your HP to %i/%i.", m_session->GetPlayer()->GetName(), hp, hpm);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_MAXHEALTH, hpm );
    chr->SetUInt32Value( UNIT_FIELD_HEALTH, hp );

    return true;
}


bool ChatHandler::HandleModifyManaCommand(const char* args)
{
    WorldPacket data;

    char* pmana = strtok((char*)args, " ");
    if (!pmana)
        return false;

    char* pmanaMax = strtok(NULL, " ");
    if (!pmanaMax)
        return false;

    int32 manam = atoi(pmanaMax);
    int32 mana = atoi(pmana);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You change the MANA to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s changed your MANA to %i/%i.", m_session->GetPlayer()->GetName(), mana, manam);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_MAXPOWER1, manam );
    chr->SetUInt32Value( UNIT_FIELD_POWER1, mana );

    return true;
}


bool ChatHandler::HandleModifyEnergyCommand(const char* args)
{
    WorldPacket data;

    char* pmana = strtok((char*)args, " ");
    if (!pmana)
        return false;

    char* pmanaMax = strtok(NULL, " ");
    if (!pmanaMax)
        return false;

    int32 manam = atoi(pmanaMax);
    int32 mana = atoi(pmana);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You change the ENERGY to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s changed your ENERGY to %i/%i.", m_session->GetPlayer()->GetName(), mana, manam);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_MAXPOWER4, manam );
    chr->SetUInt32Value( UNIT_FIELD_POWER4, mana );

    printf("current energy: %u\n",chr->GetUInt32Value( UNIT_FIELD_MAXPOWER4));

    return true;
}


bool ChatHandler::HandleModifyRageCommand(const char* args)
{
    WorldPacket data;

    char* pmana = strtok((char*)args, " ");
    if (!pmana)
        return false;

    char* pmanaMax = strtok(NULL, " ");
    if (!pmanaMax)
        return false;

    int32 manam = atoi(pmanaMax);
    int32 mana = atoi(pmana);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You change the RAGE to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s changed your RAGE to %i/%i.", m_session->GetPlayer()->GetName(), mana, manam);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_MAXPOWER2, manam );
    chr->SetUInt32Value( UNIT_FIELD_POWER2, mana );

    return true;
}


bool ChatHandler::HandleModifyLevelCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    int32 lvl = atoi((char*)args);

    if(lvl > 99 || lvl < 1)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You change the LVL to %i of %s.", lvl, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s changed your LVL to %i.", m_session->GetPlayer()->GetName(), lvl);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_LEVEL, lvl );

    return true;
}


bool ChatHandler::HandleTaxiCheatCommand(const char* args)
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
        sprintf((char*)buf,"%s has all taxi nodes now.", chr->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has no more taxi nodes now.", chr->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    if (flag != 0)
    {
        sprintf((char*)buf,"%s has given you all taxi nodes.",
            m_session->GetPlayer()->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has deleted all your taxi nodes.",
            m_session->GetPlayer()->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    chr->GetSession()->SendPacket(&data);

    for (uint8 i=0; i<8; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetTaximask(i, 0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetTaximask(i, 0);
        }
    }

    return true;
}


bool ChatHandler::HandleModifyASpedCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    float ASpeed = (float)atof((char*)args);

    if (ASpeed > 50 || ASpeed <= 0)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You set all speeds to %2.2f of %s.", ASpeed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s set all your speeds to %2.2f.", m_session->GetPlayer()->GetName(), ASpeed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize( SMSG_FORCE_RUN_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)ASpeed;
    chr->SendMessageToSet( &data, true );

    data.Initialize( SMSG_FORCE_SWIM_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)ASpeed;
    chr->SendMessageToSet( &data, true );
    data.Initialize( SMSG_FORCE_RUN_BACK_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)ASpeed;
    chr->SendMessageToSet( &data, true );
    return true;
}


bool ChatHandler::HandleModifySpeedCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    float Speed = (float)atof((char*)args);

    if (Speed > 50 || Speed <= 0)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You set the speed to %2.2f of %s.", Speed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s set your speed to %2.2f.", m_session->GetPlayer()->GetName(), Speed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize( SMSG_FORCE_RUN_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)Speed;
    chr->SendMessageToSet( &data, true );

    return true;
}


bool ChatHandler::HandleModifySwimCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    float Swim = (float)atof((char*)args);

    if (Swim > 50 || Swim <= 0)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You set the swim speed to %2.2f of %s.", Swim, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s set your swim speed to %2.2f.", m_session->GetPlayer()->GetName(), Swim);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize( SMSG_FORCE_SWIM_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)Swim;
    chr->SendMessageToSet( &data, true );

    return true;
}


bool ChatHandler::HandleModifyBWalkCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    float BSpeed = (float)atof((char*)args);

    if (BSpeed > 50 || BSpeed <= 0)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You set the backwards run speed to %2.2f of %s.", BSpeed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s set your backwards run speed to %2.2f.", m_session->GetPlayer()->GetName(), BSpeed);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize( SMSG_FORCE_RUN_BACK_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << (float)BSpeed;
    chr->SendMessageToSet( &data, true );

    return true;
}


bool ChatHandler::HandleModifyScaleCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    float Scale = (float)atof((char*)args);
    if (Scale > 3 || Scale <= 0)
    {
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You set the size %2.2f of %s.", Scale, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s set your size to %2.2f.", m_session->GetPlayer()->GetName(), Scale);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetFloatValue(OBJECT_FIELD_SCALE_X, Scale);

    return true;
}


bool ChatHandler::HandleModifyMountCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    uint16 mId = 1147;
    float speed = (float)15;
    uint32 num = 0;

    num = atoi((char*)args);
    switch(num)
    {
        case 1:                                   //Troll
            mId=14340;                            //Black War Raptor
            break;
        case 2:
            mId=4806;                             //Emerald Raptor
            break;
        case 3:
            mId=6471;                             //Ivory Raptor
            break;
        case 4:
            mId=12345;                            //Mottled Red Raptor
            break;
        case 5:
            mId=6472;                             //Turquoise Raptor
            break;
        case 6:
            mId=6473;                             //Violet Raptor
            break;
        case 7:                                   //Undead
            mId=10670;                            //Red Skeletal Horse
            break;
        case 8:
            mId=10719;                            //Red Skeletal Warhorse
            break;
        case 9:
            mId=10671;                            //Blue Skeletal Horse
            break;
        case 10:
            mId=10672;                            //Brown Skeletal Horse
            break;
        case 11:
            mId=10720;                            //Green Skeletal Warhorse
            break;
        case 12:                                  //Tauren
            mId=14349;                            //Black War Kodo
            break;
        case 13:
            mId=11641;                            //Brown Kodo
            break;
        case 14:
            mId=12244;                            //Gray Kodo
            break;
        case 15:
            mId=12242;                            //Teal Kodo
            break;
        case 16:
            mId=14578;                            //Great Brown Kodo
            break;
        case 17:
            mId=14579;                            //Great Gray Kodo
            break;
        case 18:
            mId=14349;                            //Great White Kodo
            break;
        case 19:
            mId=12245;                            //Green Kodo
            break;
        case 20:
            mId=14335;                            //Black War Wolf
            break;
        case 21:
            mId=207;                              //Black Wolf
            break;
        case 22:
            mId=2328;                             //Brown Wolf
            break;
        case 23:
            mId=2327;                             //Dire Wolf
            break;
        case 24:
            mId=2326;                             //Red Wolf
            break;
        case 25:
            mId=14573;                            //Swift Brown Wolf
            break;
        case 26:
            mId=14574;                            //Swift Gray Wolf
            break;
        case 27:
            mId=14575;                            //Swift Timber Wolf
            break;
        case 28:
            mId=604;                              //Timber Wolf
            break;
        case 29:
            mId=1166;                             //Winter Wolf
            break;
        case 30:                                  //Human
            mId=2402;                             //Black Stallion
            break;
        case 31:
            mId=2410;                             //White Stallion
            break;
        case 32:
            mId=2409;                             //Pinto
            break;
        case 33:
            mId=2408;                             //Palomino
            break;
        case 34:
            mId=2405;                             //Chestnut Mare
            break;
        case 35:
            mId=14337;                            //Black War Steed
            break;
        case 36:                                  //Gnome
            mId=6569;                             //Blue Mechanostrider
            break;
        case 37:
            mId=10661;                            //Green Mechanostrider
            break;
        case 38:
            mId=10666;                            //Icy Blue Mechanostrider Mod A
            break;
        case 39:
            mId=9473;                             //Red Mechanostrider
            break;
        case 40:
            mId=9476;                             //Unpainted Mechanostrider
            break;
        case 41:
            mId=9474;                             //White Mechanostrider Mod A
            break;
        case 42:
            mId=14374;                            //Swift Green Mechanostrider
            break;
        case 43:
            mId=14376;                            //Swift White Mechanostrider
            break;
        case 44:
            mId=14377;                            //Swift Yellow Mechanostrider
            break;
        case 45:                                  //Human
            mId=2404;                             //Brown Horse
            break;
        case 46:                                  //Dawrf
            mId=2784;                             //Black Ram
            break;
        case 47:
            mId=2787;                             //Frost Ram
            break;
        case 48:
            mId=2785;                             //Brown Ram
            break;
        case 49:
            mId=2736;                             //Gray Ram
            break;
        case 50:
            mId=2786;                             //White Ram
            break;
        case 51:
            mId=14347;                            //Swift Brown Ram
            break;
        case 52:
            mId=14346;                            //Swift White Ram
            break;
        case 53:
            mId=14576;                            //Swift Gray Ram
            break;
        case 54:                                  //Nightelf
            mId=9695;                             //Frostsaber
            break;
        case 55:
            mId=9991;                             //Nightsaber
            break;
        case 56:
            mId=6448;                             //Striped Nightsaber
            break;
        case 57:
            mId=6444;                             //Spotted Frostsaber
            break;
        case 58:
            mId=6080;                             //Striped Frostsaber
            break;
        case 59:
            mId=6447;                             //Leopard
            break;
        case 60:
            mId=4805;                             //Primal Leopard
            break;
        case 61:
            mId=9714;                             //Tiger Mount
            break;
        case 62:
            mId=6448;                             //Spotted Panther
            break;
        case 63:
            mId=6442;                             //Spotted Nightsaber
            break;
        case 64:
            mId=14632;                            //Swift Stormsaber
            break;
        case 65:
            mId=14332;                            //Swift Mistsaber
            break;
        case 66:
            mId=14331;                            //Swift Frostsaber
            break;
        case 67:                                  //Paladin Mount
            mId=8469;                             //Warhorse
            break;
        case 68:                                  //Warlock Mount
            mId=2830;                             //Felsteed
            break;
        case 69:
            mId=2346;                             //Jezelles Felsteed
            break;
        default:                                  //NO ERROR OF MOUNT ;)

            FillSystemMessageData(&data, m_session, "There is no such mount.");
            m_session->SendPacket( &data );

            return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    sprintf((char*)buf,"You give a mount to %s.", chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    sprintf((char*)buf,"%s gave you a mount.", m_session->GetPlayer()->GetName());
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_FLAGS , 0x001000 );
    chr->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , mId);
    chr->SetUInt32Value( UNIT_FIELD_FLAGS , 0x003000 );

    data.Initialize( SMSG_FORCE_RUN_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << speed;
    WPAssert(data.size() == 12);
    chr->SendMessageToSet( &data, true );

    data.Initialize( SMSG_FORCE_SWIM_SPEED_CHANGE );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID );
    data << chr->GetUInt32Value( OBJECT_FIELD_GUID + 1 );
    data << speed;
    WPAssert(data.size() == 12);
    chr->SendMessageToSet( &data, true );

    return true;
}


bool ChatHandler::HandleModifyGoldCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int32 gold = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    uint32 moneyuser =(m_session->GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE));

    char buf[256];

    if(gold < 0)
    {
        int32 newmoney = moneyuser + gold;
        // INFO
        Log::getSingleton( ).outDetail("USER1: %i, ADD: %i, DIF: %i\n", moneyuser, gold, newmoney);
        if(newmoney < 0 )
        {
            // send message to user
            sprintf((char*)buf,"You take all copper of %s.", chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );

            // send message to player
            sprintf((char*)buf,"%s took you all of your copper.", m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);

            // update value
            chr->SetUInt32Value( PLAYER_FIELD_COINAGE, 0);
        }
        else
        {
            // send message to user
            sprintf((char*)buf,"You take %i copper to %s.", abs(gold), chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );

            // send message to player
            sprintf((char*)buf,"%s took %i copper from you.", m_session->GetPlayer()->GetName(), abs(gold));
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        // send message to user
        sprintf((char*)buf,"You give %i copper to %s.", gold, chr->GetName());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        // send message to player
        sprintf((char*)buf,"%s gave you %i copper.", m_session->GetPlayer()->GetName(), gold);
        FillSystemMessageData(&data, m_session, buf);
        chr->GetSession()->SendPacket(&data);
    }

    // update value
    Log::getSingleton( ).outDetail("USER2: %i, ADD: %i, RESULT: %i\n", moneyuser, gold, moneyuser+gold);
    chr->SetUInt32Value( PLAYER_FIELD_COINAGE, moneyuser+gold );

    return true;
}


bool ChatHandler::HandleModifyBitCommand(const char* args)
{
    WorldPacket data;

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
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
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        FillSystemMessageData(&data, m_session, "Incorrect values.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    if ( chr->HasFlag( field, (1<<(bit-1)) ) )
    {
        chr->RemoveFlag( field, (1<<(bit-1)) );
        sprintf((char*)buf,"Removed bit %i in field %i.", bit, field);
    }
    else
    {
        chr->SetFlag( field, (1<<(bit-1)) );
        sprintf((char*)buf,"Set bit %i in field %i.", bit, field);
    }

    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}
