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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "TargetedMovementGenerator.h"
#include "CreatureAI.h"
#include "Util.h"
#include "Pet.h"

void WorldSession::HandlePetAction( WorldPacket & recv_data )
{
    sLog.outString( "HandlePetAction.\n" );

    uint64 guid1;
    uint16 spellid;
    uint16 flag;
    uint64 guid2;
    WorldPacket data;
    recv_data >> guid1;                                     //pet guid
    recv_data >> flag;
    recv_data >> spellid;                                   //delete = 0x0700 CastSpell = C100
    recv_data >> guid2;                                     //tag guid
    Player *pl=GetPlayer();
    Creature* pet=ObjectAccessor::Instance().GetCreature(*pl,guid1);
    uint16 newFlag=flag;

    switch(flag)
    {
        case 0x0000:                                        //spellid=1792  //STAY
        {
            pet->StopMoving();
            (*pet)->Idle();
        }break;
        case 0x0001:                                        //spellid=1792  //FOLLOW
        {
            (*pet)->Mutate(new TargetedMovementGenerator(*pl));
            //Pet->AI_Follow(Owner);
        }break;
        case 0x0002:                                        //spellid=1792  //ATTACK
        {
            uint64 selguid = pl->GetSelection();
                                                            //guid2
            Unit *TargetUnit = ObjectAccessor::Instance().GetCreature(*pl, selguid);
            if(!TargetUnit)
                TargetUnit=ObjectAccessor::Instance().FindPlayer(selguid);
            if(TargetUnit == NULL) return;
            pet->AI().AttackStart(TargetUnit);

            data.Initialize(SMSG_AI_REACTION);
            data << guid1 << uint32(00000002);
            SendPacket(&data);

        }break;
        case 0x0003:                                        //dismiss
        {
            if(pet)
            {
                if( pl->getClass() == WARLOCK )
                    ((Pet*)pet)->SavePetToDB();
                pl->SetUInt64Value(UNIT_FIELD_SUMMON, 0);

                data.Initialize(SMSG_DESTROY_OBJECT);
                data << pet->GetGUID();
                pl->SendMessageToSet (&data, true);
                MapManager::Instance().GetMap(pet->GetMapId())->Remove(pet,true);

                data.Initialize(SMSG_PET_SPELLS);
                data << uint64(0);
                pl->GetSession()->SendPacket(&data);
            }
        }break;
        case 0xC100:                                        //pet cast spell
        case 0x100:
        case 0x8100:
        {
            SpellEntry *spellInfo = sSpellStore.LookupEntry(spellid );

            if(!spellInfo)
            {
                sLog.outError("WORLD: unknown PET spell id %i\n", spellid);
                return;
            }

            Spell *spell = new Spell(pet, spellInfo, false, 0);
            WPAssert(spell);

            Unit* unit_target=ObjectAccessor::Instance().GetCreature(*pl,guid2);
            if(!unit_target)
                unit_target=ObjectAccessor::Instance().FindPlayer(guid2);
            //guid2 = pl->GetGUID();
            SpellCastTargets targets;
            targets.m_unitTarget = unit_target;             //(Unit*)pl;
            spell->prepare(&targets);
        }break;
        case 0x0700:                                        //delete pet
        {
            /*
                        if(pet)
                        {
                            pl->SetUInt64Value(UNIT_FIELD_SUMMON, 0);

                            data.Initialize(SMSG_DESTROY_OBJECT);
                            data << pet->GetGUID();
                            pl->SendMessageToSet (&data, true);
                            MapManager::Instance().GetMap(pet->GetMapId())->Remove(pet,true);

                            data.Initialize(SMSG_PET_SPELLS);
                            data << uint64(0);
                            pl->GetSession()->SendPacket(&data);
                        }*/
        }break;
        default:
            sLog.outError("WORLD: unknown PET flag Action %i\n", flag);
    }
}

void WorldSession::HandlePetNameQuery( WorldPacket & recv_data )
{
    sLog.outString( "HandlePetNameQuery.\n" );
    uint32 state2;
    uint64 guid;
    uint32 state3;

    std::string name = "ERROR_NO_NAME_FOR_PET_GUID";

    recv_data >> state2;
    recv_data >> guid;

    Player *pl=GetPlayer();
    Creature* pet=ObjectAccessor::Instance().GetCreature(*pl,guid);
    if(pet)
    {
        name = pl->GetName();                               //pet->GetCreatureInfo()->Name;
        name.append("\'s Pet");
        state3=pet->GetUInt32Value(UNIT_FIELD_STAT3);
    }
    WorldPacket data;
    data.Initialize(SMSG_PET_NAME_QUERY_RESPONSE);
    data << uint32(0x18088);
    data << name;
    data << uint32(0x426D3DC6);
    pl->GetSession()->SendPacket(&data);
}

void WorldSession::HandlePetSetAction( WorldPacket & recv_data )
{
    sLog.outString( "HandlePetSetAction.\n" );
}
