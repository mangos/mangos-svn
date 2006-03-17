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


bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    char pAnnounce[256];
    
    sprintf((char*)pAnnounce, "BROADCAST: %s", args);
    sWorld.SendWorldText(pAnnounce);              

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
        if(!(obj = (Object*)ObjectAccessor::Instance().FindPlayer(guid)) && !(obj = (Object*)ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(),guid)))
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




bool ChatHandler::HandleSummonCommand(const char* args)
{
    WorldPacket data;

    if(!*args)
        return false;

    Player *chr = objmgr.GetPlayer(args);
    if (chr)
    {
        
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the HP to %i/%i of %s.", hp, hpm, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the MANA to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the ENERGY to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the RAGE to %i/%i of %s.", mana, manam, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the LVL to %i of %s.", lvl, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
    sprintf((char*)buf,"%s changed your LVL to %i.", m_session->GetPlayer()->GetName(), lvl);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value( UNIT_FIELD_LEVEL, lvl );

    return true;
}

bool ChatHandler::HandleModifyFactionCommand(const char* args)
{

    WorldPacket data;

    if(!*args)
        return false;

    uint32 factionid = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the Faction to %i of %s.", factionid, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
    sprintf((char*)buf,"%s changed your Faction to %i.", m_session->GetPlayer()->GetName(), factionid);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    chr->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,factionid);

	
	
	
  
    return true;
}

bool ChatHandler::HandleModifySpellCommand(const char* args)
{

    WorldPacket data;

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

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You change the spellflatid=%i,val= %i ,mark =%i to %s.", spellflatid, val, mark, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
    sprintf((char*)buf,"%s changed your spellflatid=%i,val= %i ,mark =%i.", m_session->GetPlayer()->GetName(), spellflatid, val, mark);
    FillSystemMessageData(&data, m_session, buf);

    chr->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_SET_FLAT_SPELL_MODIFIER);
    data << uint8(spellflatid);
    data << uint8(op);
    data << uint16(val);
    data << uint16(mark);
    chr->GetSession()->SendPacket(&data);       
    
    return true;
}

bool ChatHandler::HandleTaxiCheatCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You set all speeds to %2.2f of %s.", ASpeed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You set the speed to %2.2f of %s.", Speed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You set the swim speed to %2.2f of %s.", Swim, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You set the backwards run speed to %2.2f of %s.", BSpeed, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You set the size %2.2f of %s.", Scale, chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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

            FillSystemMessageData(&data, m_session, "There is no such mount.");
            m_session->SendPacket( &data );

            return true;
    }

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    
    sprintf((char*)buf,"You give a mount to %s.", chr->GetName());
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    
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
    if (chr == NULL)                              
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
        
        Log::getSingleton( ).outDetail("USER1: %i, ADD: %i, DIF: %i\n", moneyuser, gold, newmoney);
        if(newmoney < 0 )
        {
            
            sprintf((char*)buf,"You take all copper of %s.", chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );

            
            sprintf((char*)buf,"%s took you all of your copper.", m_session->GetPlayer()->GetName());
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);

            
            chr->SetUInt32Value( PLAYER_FIELD_COINAGE, 0);
        }
        else
        {
            
            sprintf((char*)buf,"You take %i copper to %s.", abs(gold), chr->GetName());
            FillSystemMessageData(&data, m_session, buf);
            m_session->SendPacket( &data );

            
            sprintf((char*)buf,"%s took %i copper from you.", m_session->GetPlayer()->GetName(), abs(gold));
            FillSystemMessageData(&data, m_session, buf);
            chr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        
        sprintf((char*)buf,"You give %i copper to %s.", gold, chr->GetName());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        
        sprintf((char*)buf,"%s gave you %i copper.", m_session->GetPlayer()->GetName(), gold);
        FillSystemMessageData(&data, m_session, buf);
        chr->GetSession()->SendPacket(&data);
    }

    
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
