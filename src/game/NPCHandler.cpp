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
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "UpdateMask.h"
#include "ScriptCalls.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "MapManager.h"
#include "Pet.h"
#include "WaypointMovementGenerator.h"

void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TABARDVENDOR);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate( uint64 guid )
{
    WorldPacket data( MSG_TABARDVENDOR_ACTIVATE, 8 );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    sLog.outDetail( "WORLD: Received CMSG_BANKER_ACTIVATE" );

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendShowBank(guid);
}

void WorldSession::SendShowBank( uint64 guid )
{
    WorldPacket data( SMSG_SHOW_BANK, 8 );
    data << guid;
    SendPacket( &data );
}

void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    WorldPacket data;
    uint64 guid;

    recv_data >> guid;
    SendTrainerList( guid );
}

void WorldSession::SendTrainerList( uint64 guid )
{
    std::string str = "Hello! Ready for some training?";
    SendTrainerList( guid, str );
}

void WorldSession::SendTrainerList( uint64 guid,std::string strTitle )
{
    sLog.outDebug( "WORLD: SendTrainerList" );

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if(!unit->isCanTrainingOf(_player,true))
        return;

    CreatureInfo const *ci = unit->GetCreatureInfo();

    if (!ci)
    {
        sLog.outDebug( "WORLD: SendTrainerList - (%u) NO CREATUREINFO! (GUID: %u)", uint32(GUID_LOPART(guid)), guid );
        return;
    }

    std::list<TrainerSpell*> Tspells;
    std::list<TrainerSpell*>::iterator itr;

    for (itr = unit->GetTspellsBegin(); itr != unit->GetTspellsEnd();itr++)
    {
        if(!(*itr)->spell  || _player->HasSpell((*itr)->spell->Id))
            continue;
        //if(!(*itr)->reqspell || _player->HasSpell((*itr)->reqspell))
        //    Tspells.push_back(*itr);
        if((*itr)->spell && sSpellStore.LookupEntry((*itr)->spell->EffectTriggerSpell[0]))
            Tspells.push_back(*itr);
    }

    WorldPacket data( SMSG_TRAINER_LIST, 200 );             // guess size
    data << guid;
    data << uint32(0) << uint32(Tspells.size());

    for (itr = Tspells.begin(); itr != Tspells.end();itr++)
    {
        uint8 canlearnflag = 1;
        bool ReqskillValueFlag = false;
        bool LevelFlag = false;
        bool ReqspellFlag = false;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry((*itr)->spell->EffectTriggerSpell[0]);
        assert(spellInfo);                                  // Tested already in prev. for loop

        if((*itr)->reqskill)
        {
            if(_player->GetPureSkillValue((*itr)->reqskill) >= (*itr)->reqskillvalue)
                ReqskillValueFlag = true;
        }
        else
            ReqskillValueFlag = true;

        uint32 spellLevel = ( (*itr)->reqlevel ? (*itr)->reqlevel : spellInfo->spellLevel);
        if(_player->getLevel() >= spellLevel)
            LevelFlag = true;

        uint32 prev_id =  objmgr.GetPrevSpellInChain(spellInfo->Id);
        if(!prev_id || _player->HasSpell(prev_id))
            ReqspellFlag = true;

        if(ReqskillValueFlag && LevelFlag && ReqspellFlag)
            canlearnflag = 0;                               //green, can learn
        else canlearnflag = 1;                              //red, can't learn

        if(_player->HasSpell(spellInfo->Id))
            canlearnflag = 2;                               //gray, can't learn
        else
        if((*itr)->spell->Effect[0] == SPELL_EFFECT_LEARN_SPELL &&
            _player->HasSpell((*itr)->spell->EffectTriggerSpell[0]))
            canlearnflag = 2;                               //gray, can't learn

        if((*itr)->spell->Effect[1] == SPELL_EFFECT_SKILL_STEP)
            if(!_player->CanLearnProSpell((*itr)->spell->Id))
                canlearnflag = 1;

        data << uint32((*itr)->spell->Id);
        data << uint8(canlearnflag);
        data << uint32((*itr)->spellcost);
        data << uint32(0);
        data << uint32(0);
        data << uint8(spellLevel);
        data << uint32((*itr)->reqskill);
        data << uint32((*itr)->reqskillvalue);
        data << uint32(prev_id);
        data << uint32(0);
        data << uint32(0);
    }

    data << strTitle;
    SendPacket( &data );

    Tspells.clear();
}

void WorldSession::HandleTrainerBuySpellOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 spellId = 0;
    TrainerSpell *proto=NULL;

    recv_data >> guid >> spellId;
    sLog.outDebug( "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u",uint32(GUID_LOPART(guid)), spellId );

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid,UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if(!unit->isCanTrainingOf(_player,true))
        return;

    std::list<TrainerSpell*>::iterator titr;

    for (titr = unit->GetTspellsBegin(); titr != unit->GetTspellsEnd();titr++)
    {
        if((*titr)->spell->Id == spellId)
        {
            proto = *titr;
            break;
        }
    }

    if (proto == NULL) return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(proto->spell->EffectTriggerSpell[0]);

    if(!spellInfo) return;
    if(_player->HasSpell(spellInfo->Id))
        return;
    if(_player->getLevel() < (proto->reqlevel ? proto->reqlevel : spellInfo->spellLevel))
        return;
    if(proto->reqskill && _player->GetSkillValue(proto->reqskill) < proto->reqskillvalue)
        return;

    uint32 prev_id =  objmgr.GetPrevSpellInChain(spellInfo->Id);
    if(prev_id && !_player->HasSpell(prev_id))
        return;

    if(proto->spell->Effect[1] == SPELL_EFFECT_SKILL_STEP)
        if(!_player->CanLearnProSpell(spellId))
            return;

    if(!proto)
    {
        sLog.outErrorDb("TrainerBuySpell: Trainer(%u) has not the spell(%u).", uint32(GUID_LOPART(guid)), spellId);
        return;
    }
    if( _player->GetMoney() >= proto->spellcost )
    {
        WorldPacket data( SMSG_TRAINER_BUY_SUCCEEDED, 12 );
        data << guid << spellId;
        SendPacket( &data );

        _player->ModifyMoney( -int32(proto->spellcost) );
        if(spellInfo->powerType == 2)
        {
            _player->addSpell(spellId,4);                   // active = 4 for spell book of hunter's pet
            return;
        }

        Spell *spell;

		if(spellId == 30546)
		{// UQ1: Learn direct. This spell can not be learnt any other way... (Maybe bad DB!)
			float u_oprientation = unit->GetOrientation();

			_player->learnSpell((uint16)spellId);

		    // trainer always see at customer in time of training (part of client functionality)
	        unit->SetInFront(_player);

			// Add some FX...
			SpellEntry const *spellInfo = sSpellStore.LookupEntry(20211);
			spell = new Spell(_player, spellInfo, false, NULL);

			// trainer always return to original orientation
			unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
			return;
		}
		else if(spellId == 19274)
		{// UQ1: Learn direct. This spell can not be learnt any other way... (Maybe bad DB!)
			float u_oprientation = unit->GetOrientation();

			_player->learnSpell((uint16)spellId);

		    // trainer always see at customer in time of training (part of client functionality)
	        unit->SetInFront(_player);

			// Add some FX...
			SpellEntry const *spellInfo = sSpellStore.LookupEntry(20211);
			spell = new Spell(_player, spellInfo, false, NULL);

			// trainer always return to original orientation
			unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
			return;
		}
		else if(spellId == 19275)
		{// UQ1: Learn direct. This spell can not be learnt any other way... (Maybe bad DB!)
			float u_oprientation = unit->GetOrientation();

			_player->learnSpell((uint16)spellId);

		    // trainer always see at customer in time of training (part of client functionality)
	        unit->SetInFront(_player);

			// Add some FX...
			SpellEntry const *spellInfo = sSpellStore.LookupEntry(20211);
			spell = new Spell(_player, spellInfo, false, NULL);

			// trainer always return to original orientation
			unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
			return;
		}
		else if(spellId == 29341)
			spell = new Spell(_player, proto->spell, false, NULL);
		else if(spellId == 3704)
			spell = new Spell(_player, proto->spell, false, NULL);
		else if(spellId == 1476)
			spell = new Spell(_player, proto->spell, false, NULL);
		else if(spellId == 33718) // UQ1: Fix for conjure food (rank 8) training!
			spell = new Spell(_player, proto->spell, false, NULL);
        else if(proto->spell->SpellVisual == 222)
            spell = new Spell(_player, proto->spell, false, NULL);
        else
            spell = new Spell(unit, proto->spell, false, NULL);

        SpellCastTargets targets;
        targets.setUnitTarget( _player );

        float u_oprientation = unit->GetOrientation();

        // trainer always see at customer in time of training (part of client functionality)
        unit->SetInFront(_player);

        spell->prepare(&targets);

        // trainer always return to original orientation
        unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
    }
}

#ifdef _MANGOS_ENHANCED
//
// Start: Unique1's Multi-Class Training
//

char const *formatString( char const *format, ... )
{
    va_list        argptr;
    #define    MAX_FMT_STRING    32000
    static char        temp_buffer[MAX_FMT_STRING];
    static char        string[MAX_FMT_STRING];
    static int        index = 0;
    char    *buf;
    int len;

    va_start(argptr, format);
    vsnprintf(temp_buffer,MAX_FMT_STRING, format, argptr);
    va_end(argptr);

    len = strlen(temp_buffer);

    if( len >= MAX_FMT_STRING )
        return "ERROR";

    if (len + index >= MAX_FMT_STRING-1)
    {
        index = 0;
    }

    buf = &string[index];
    memcpy( buf, temp_buffer, len+1 );

    index += len + 1;

    return buf;
}

void Strip_XX( const char *in, char *out ) 
{// Strip the " XX" from the end of the string! XX is II or IV or whatever...
	int length = strlen(in);
	int count = 0;

	while ( *in && count < length - 3 ) 
	{
		*out++ = *in++;
		count++;
	}
	*out = 0;
}

void prepareMulticlassGossipMenu( Creature *unit, Player *pPlayer, uint32 gossipid )
{
	GossipOptionList m_goptions;
	Player *_player = pPlayer;
	int	count = 0;

	unit = ObjectAccessor::Instance().GetCreature(*_player, _player->playerTalkNPCGUID);

	uint32 trainer_type = unit->GetCreatureInfo()->trainer_type;

    PlayerMenu* pm=pPlayer->PlayerTalkClass;
    pm->ClearMenus();

	if(!m_goptions.size())
        unit->LoadGossipOptions();

	std::list<TrainerSpell*> Tspells;
    std::list<TrainerSpell*>::iterator itr;

    for (itr = unit->GetTspellsBegin(); itr != unit->GetTspellsEnd();itr++)
    {
        if((*itr)->spell)
            Tspells.push_back(*itr);
    }

    for (itr = Tspells.begin(); itr != Tspells.end();itr++)
    {
		TrainerSpell *proto = *itr;
        uint8 canlearnflag = 1;
        bool ReqskillValueFlag = false;
        bool LevelFlag = false;
        bool ReqspellFlag = false;
		uint32 entry_id;

		if(trainer_type == TRAINER_TYPE_MULTICLASS)
			entry_id = (*itr)->spell->EffectTriggerSpell[0];
		else // Other types are direct casts!
			entry_id = (*itr)->spell->Id;

		SpellEntry const *spellInfo = sSpellStore.LookupEntry(entry_id);

        if(!spellInfo)
            continue;

        if(trainer_type == TRAINER_TYPE_MULTICLASS && (*itr)->reqskill)
        {
            if(_player->GetPureSkillValue((*itr)->reqskill) >= (*itr)->reqskillvalue)
                ReqskillValueFlag = true;
        }
        else ReqskillValueFlag = true;

        uint32 spellLevel = ( (*itr)->reqlevel ? (*itr)->reqlevel : spellInfo->spellLevel);

        if(trainer_type != TRAINER_TYPE_MULTICLASS || (_player->getLevel() >= spellLevel+10))
            LevelFlag = true;

        uint32 prev_id =  objmgr.GetPrevSpellInChain(spellInfo->Id);

        if(trainer_type != TRAINER_TYPE_MULTICLASS || !prev_id || _player->HasSpell(prev_id))
            ReqspellFlag = true;

        if(ReqskillValueFlag && LevelFlag && ReqspellFlag)
            canlearnflag = 0;                               //green, can learn

        else canlearnflag = 1;                              //red, can't learn

        if(trainer_type == TRAINER_TYPE_MULTICLASS && _player->HasSpell(spellInfo->Id))
            canlearnflag = 2;                               //gray, can't learn

        if((*itr)->spell->Effect[1] == 44)
            if(!_player->CanLearnProSpell((*itr)->spell->Id))
                canlearnflag = 1;

		//if (spellLevel > 30)
		//	continue;

		int lang_ID = 0;

		for (lang_ID = 0; lang_ID < 8; lang_ID++)
		{// UQ1: Find the DBC language we are using!
			if( strcmp((*itr)->spell->SpellName[lang_ID],"") != 0)
				break;
		}

		PlayerSpellMap m_spells = _player->GetSpellMap();

		//if (objmgr.GetSpellRank(spellInfo->Id) == 1)
		if(trainer_type == TRAINER_TYPE_MULTICLASS)
		{// Do we already have a higher level version of the spell?
			bool bad = false;

			for (PlayerSpellMap::const_iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
			{
				if(itr2->second->state == PLAYERSPELL_REMOVED) continue;
				SpellEntry const *i_spellInfo = sSpellStore.LookupEntry(itr2->first);
				if(!i_spellInfo) continue;

				//SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell);
				//PlayerSpell test_spell = itr2->second;
				if( strcmp(i_spellInfo->SpellName[lang_ID],(*itr)->spell->SpellName[lang_ID]) == 0)
				{// Same name, check ranks!
					if (objmgr.GetSpellRank(i_spellInfo->Id) >= objmgr.GetSpellRank(spellInfo->Id))
					{
						bad = true;
						break;
					}
				}
		    }

			if (bad) // Have a higher one!
				continue;
		}

		if (canlearnflag == 0)
		{
			if (count+1 >= GOSSIP_MAX_MENU_ITEMS)
				break;

			// Calculate gold, silver and copper cost seperately...
			uint32 total_cost = proto->spellcost*2;

			uint32 gold = total_cost /(100*100);
			uint32 silver = (total_cost % (100*100)) / 100;
			uint32 copper = (total_cost % (100*100)) % 100;

			char spell_name[255];

			if (strncmp((*itr)->spell->SpellName[lang_ID], "Healing Draenei Survivor", 24) == 0)
			{// Rename this one for healer NPC!
				strncpy( spell_name, "Heal", sizeof( spell_name ) );
			}
			else if (strncmp((*itr)->spell->SpellName[lang_ID], "zzOLD", 5) == 0)
			{// Remove the zzOLD from the name!
				strncpy( spell_name, (*itr)->spell->SpellName[lang_ID] + 5, sizeof( spell_name ) );
			}
			else if (strncmp( (*itr)->spell->SpellName[lang_ID] + (strlen((*itr)->spell->SpellName[lang_ID])-3), " II", 3 ) == 0)
			{// Remove the II from the name!
				//strncpy( spell_name, (*itr)->spell->SpellName[lang_ID], strlen((*itr)->spell->SpellName[lang_ID])-3 );
				//sLog.outDetail( "WORLD: Multiclass Trainer Request - %s found, length %i, stripped to %s.", (*itr)->spell->SpellName[lang_ID], strlen((*itr)->spell->SpellName[lang_ID]), spell_name );
				Strip_XX((*itr)->spell->SpellName[lang_ID], spell_name);
			}
			else
			{
				strcpy(spell_name, (*itr)->spell->SpellName[lang_ID]);
			}

			if (trainer_type == TRAINER_TYPE_MULTICLASS && strcmp((*itr)->spell->Rank[lang_ID],"") != 0) // UQ1: Only multiclass trainers show rank of the spells...
			{// "3" is a spellbook icon.
				pm->GetGossipMenu()->AddMenuItem(3,formatString("|c8f0000ff%s |c8f202020[|c8f2020ff%s|c8f202020]\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, (*itr)->spell->Rank[lang_ID], gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
			}
			else
			{
				if (trainer_type == TRAINER_TYPE_MULTICLASS)
				{// "3" is a spellbook icon.
					pm->GetGossipMenu()->AddMenuItem(3,formatString("|c8f0000ff%s\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
				}
				else
				{
					if (strncmp(spell_name + (strlen(spell_name)-5), "Armor", 5) == 0) 
					{// "8" is an armor icon.
						pm->GetGossipMenu()->AddMenuItem(8,formatString("|c8f0000ff%s\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
					}
					else if (strncmp(spell_name + (strlen(spell_name)-10), "Protection", 10) == 0) 
					{// "8" is an armor icon.
						pm->GetGossipMenu()->AddMenuItem(8,formatString("|c8f0000ff%s\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
					}
					// "9" is a weapon icon.
					else if (gold > 0) 
					{// "6" is a money sack (with gold coin) icon.
						pm->GetGossipMenu()->AddMenuItem(6,formatString("|c8f0000ff%s\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
					}
					else 
					{// "1" is a money sack icon.
						pm->GetGossipMenu()->AddMenuItem(1,formatString("|c8f0000ff%s\n|c8f202020%u |c8fffff00gold|c8f202020, %u |c8f888888silver|c8f202020, %u |c8fffaa00copper|c8f202020.", spell_name, gold, silver, copper),_player->playerTalkNPCGUID, (*itr)->spell->Id, false);
					}
				}
			}

			sLog.outDetail( "WORLD: Multiclass Trainer Request - Send option %i [%s].", count, spell_name );

			count++;
		}
    }

	sLog.outDetail( "WORLD: Multiclass Trainer Request - Send %i options.", count );

    Tspells.clear();
}

void HandleMulticlass( WorldPacket & recv_data, uint64 guid, Player *_player )
{
    sLog.outDetail( "WORLD: Received Multiclass Trainer Request" );

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, guid);

    if (!unit)
    {
        return;
    }

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!unit->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
        return;

    if(!_player->isAlive())
        return;

	_player->playerTalkNPCGUID = guid;

    if(!Script->GossipHello( _player, unit ))
    {
        prepareMulticlassGossipMenu(unit, _player,0);
        unit->sendPreparedGossip( _player );
    }
}

void HandleMulticlassOption( WorldPacket & recv_data, uint64 guid, Player *_player, uint32 option )
{
	PlayerMenu* pm=_player->PlayerTalkClass;
	guid = _player->playerTalkNPCGUID;

	sLog.outDetail( "WORLD: Received Multiclass Trainer Option %u (spell: %u) from %u to %u", option, pm->GetGossipMenu()->MenuItemAction( option ), _player->GetGUID(), uint32(GUID_LOPART(_player->playerTalkNPCGUID)) );

    Creature *unit = ObjectAccessor::Instance().GetCreature(*_player, _player->playerTalkNPCGUID);

    if (!unit)
    {
        return;
    }

	uint32 trainer_type = unit->GetCreatureInfo()->trainer_type;

    if( unit->IsHostileTo(_player))                         // do not talk with enemies
        return;

    if(!unit->IsWithinDistInMap(_player,OBJECT_ITERACTION_DISTANCE))
        return;

    if(!_player->isAlive())
        return;

    // Buy the spell!

	TrainerSpell *proto=NULL;

	//uint32 spellId = option;
	uint32 spellId = pm->GetGossipMenu()->MenuItemAction( option );

	std::list<TrainerSpell*>::iterator titr;

    for (titr = unit->GetTspellsBegin(); titr != unit->GetTspellsEnd();titr++)
    {
        if((*titr)->spell->Id == spellId)
        {
            proto = *titr;
            break;
        }
    }

	uint32 entry_id;

	//SpellEntry const *spellInfo = sSpellStore.LookupEntry(proto->spell->EffectTriggerSpell[0]);
	if(trainer_type == TRAINER_TYPE_MULTICLASS)
		entry_id = proto->spell->EffectTriggerSpell[0];
	else // Other types are direct casts!
		entry_id = spellId;

	SpellEntry const *spellInfo = sSpellStore.LookupEntry(entry_id);

    if(!spellInfo) return;
    if(trainer_type == TRAINER_TYPE_MULTICLASS && _player->HasSpell(spellInfo->Id))
        return;
    if(trainer_type == TRAINER_TYPE_MULTICLASS && _player->getLevel()-10 < (proto->reqlevel ? proto->reqlevel : spellInfo->spellLevel))
        return;
    if(trainer_type == TRAINER_TYPE_MULTICLASS && proto->reqskill && _player->GetSkillValue(proto->reqskill) < proto->reqskillvalue)
        return;

    uint32 prev_id =  objmgr.GetPrevSpellInChain(spellInfo->Id);
    if(trainer_type == TRAINER_TYPE_MULTICLASS && prev_id && !_player->HasSpell(prev_id))
        return;

    if(trainer_type == TRAINER_TYPE_MULTICLASS && proto->spell->Effect[1] == SPELL_EFFECT_SKILL_STEP)
        if(!_player->CanLearnProSpell(spellId))
            return;

    if(!proto)
    {
        sLog.outErrorDb("TrainerBuySpell: Trainer(%u) has not the spell(%u).", uint32(GUID_LOPART(guid)), spellId);
        return;
    }
    if( _player->GetMoney() >= proto->spellcost*2 )
    {
		// Clear menu and re-send an update!
		sLog.outDetail( "HandleMulticlassOption: Clear menu and re-send an update!" );
	
		//pm->ClearMenus();
		//_player->PlayerTalkClass->ClearMenus();
		//_player->PlayerTalkClass->SendTalking("New spell has been learnt.", "Make good use of your new spell %c.");
		//unit->LoadGossipOptions();

		GossipOptionList m_goptions;
		if(!m_goptions.size())
			unit->LoadGossipOptions();
	
		pm->ClearMenus();
		_player->PlayerTalkClass->ClearMenus();
		//unit->sendPreparedGossip( _player );
		//prepareMulticlassGossipMenu(unit, _player, 0);
		pm->CloseGossip();
		//unit->sendPreparedGossip( _player );
		//_player->PlayerTalkClass->SendGossipMenu(0, _player->playerTalkNPCGUID);
		//_player->PlayerTalkClass->SendGossipMenu(0, _player->playerTalkNPCGUID);

		// Learn the spell!
        WorldPacket data( SMSG_TRAINER_BUY_SUCCEEDED, 12 );
        data << guid << spellId;
        _player->GetSession()->SendPacket( &data );

        _player->ModifyMoney( -int32(proto->spellcost*2) );
        if(spellInfo->powerType == 2)
        {
            _player->addSpell(spellId,4);                   // ative = 4 for spell book of hunter's pet
            return;
        }

		if(trainer_type == TRAINER_TYPE_MULTICLASS)
		{
			Spell *spell;
			if(proto->spell->SpellVisual == 222)
				spell = new Spell(_player, proto->spell, false, NULL);
			else
			    spell = new Spell(unit, proto->spell, false, NULL);

			SpellCastTargets targets;
			targets.setUnitTarget( _player );

			float u_oprientation = unit->GetOrientation();

			// trainer always see at customer in time of training (part of client functionality)
			unit->SetInFront(_player);

			spell->prepare(&targets);

			// trainer always return to original orientation
			unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);

			// Remove the previous spell to save slots!
/*			while (prev_id)
			{
				if (prev_id)
				{
					_player->removeSpell(prev_id);
				}

				prev_id = objmgr.GetPrevSpellInChain(prev_id);
			}*/
		}
		else
		{
			// We need to check if this is actually a spell you can cast on someone else!
			// If it is not, make them cast it on themselves!
			Spell *spell;
			SpellCastTargets targets;
			float u_oprientation = unit->GetOrientation();

			switch (entry_id)
			{
			case 16166: // Elemental Mastery - Learn Directly!
			case 18562: // Innervate - Learn Directly!
				_player->learnSpell((uint16)entry_id);

				// trainer always see at customer in time of training (part of client functionality)
				unit->SetInFront(_player);

				// Add some FX...
				spell = new Spell(_player, sSpellStore.LookupEntry(20211), false, NULL);

				// trainer always return to original orientation
				unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);

				return;
				break;
			case 28624: // Healing Draenei Survivor (Healer NPC Heal) - NPC can cast it!
			case 526: // Cure Poison (Healer NPC) - NPC can cast it!
			case 2870: // Cure Disease (Healer NPC) - NPC can cast it!
			case 2782: // Remove Curse (Healer NPC) - NPC can cast it!
			case 25312: // Divine Spirit (rank 5) - NPC can cast it!
			case 25389: // Power Word: Fortitude (rank 7) - NPC can cast it!
			case 10156: // Arcane Intellect (rank 6) - NPC can cast it!
			case 1038: // Blessing of Salvation - NPC can cast it!
			    spell = new Spell(unit, proto->spell, false, NULL);

				targets.setUnitTarget( _player );

				// trainer always see at customer in time of training (part of client functionality)
				unit->SetInFront(_player);

				spell->prepare(&targets);

				// trainer always return to original orientation
				unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
				break;
			default: // Default method: Players cast on themselves!
				spell = new Spell(_player, proto->spell, false, NULL);

				SpellCastTargets targets;
				targets.setUnitTarget( _player );

				// trainer always see at customer in time of training (part of client functionality)
				unit->SetInFront(_player);

				spell->prepare(&targets);

				// trainer always return to original orientation
				unit->Relocate(unit->GetPositionX(),unit->GetPositionY(),unit->GetPositionZ(),u_oprientation);
				break;
			}
		}
    }
}

//
// End: Unique1's Multi-Class Training
//
#endif //_MANGOS_ENHANCED

void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail( "WORLD: Received CMSG_GOSSIP_HELLO" );

    uint64 guid;
    recv_data >> guid;

    // fix for spirit healers (temp?)
    Creature *temp = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!temp)
        return;

    uint32 npcflags = UNIT_NPC_FLAG_NONE;
    if(temp->isSpiritHealer())
        npcflags = UNIT_NPC_FLAG_SPIRITHEALER;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, npcflags);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

#ifdef _MANGOS_ENHANCED
	uint32 type = unit->GetCreatureInfo()->trainer_type;

	if (type == TRAINER_TYPE_MULTICLASS || type == TRAINER_TYPE_AURA_VENDOR || type == TRAINER_TYPE_HEAL_VENDOR)
	{
		HandleMulticlass( recv_data, guid, _player );
		return;
	}
#endif //_MANGOS_ENHANCED

    if((unit->isArmorer()) || (unit->isGuard()) || (unit->isCivilian()) || (unit->isQuestGiver()) || (unit->isServiceProvider()) || (unit->isVendor()))
    {
        unit->StopMoving();
        //npcIsStopped[unit->GetGUID()] = true;
    }

    if(!Script->GossipHello( _player, unit ))
    {
        unit->prepareGossipMenu(_player,0);
        unit->sendPreparedGossip( _player );
    }
}

void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 option;
    uint64 guid;

    recv_data >> guid >> option;

    // fix for spirit healers (temp?)
    Creature *temp = ObjectAccessor::Instance().GetCreature(*_player, guid);
    if (!temp)
        return;

    uint32 npcflags = UNIT_NPC_FLAG_NONE;
    if(temp->isSpiritHealer())
        npcflags = UNIT_NPC_FLAG_SPIRITHEALER;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, npcflags);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

#ifdef _MANGOS_ENHANCED
	uint32 type = unit->GetCreatureInfo()->trainer_type;

	if (type == TRAINER_TYPE_MULTICLASS || type == TRAINER_TYPE_AURA_VENDOR || type == TRAINER_TYPE_HEAL_VENDOR)
	{
		HandleMulticlassOption( recv_data, _player->GetSelection(), _player, option );
		return;
	}
#endif //_MANGOS_ENHANCED

    if(!Script->GossipSelect( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option )) )
        unit->OnGossipSelect( _player, option );
}

void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    uint64 guid;

    recv_data >> guid;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
        return;
    }

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    if (!_player)
        return;

    uint32 level = _player->getLevel();

    //Characters from level 1-10 are not affected by resurrection sickness.
    //Characters from level 11-19 will suffer from one minute of sickness
    //for each level they are above 10.
    //Characters level 20 and up suffer from ten minutes of sickness.
    if (level > 10)
    {
        // prepere resurrection sickness setup (will be set in ResurrectPlayer())
        uint32 spellLvl = level < 20 ? level : 20;
        _player->m_resurrectingSicknessExpire = time(NULL) + (spellLvl-10)*MINUTE;
    }

    _player->ResurrectPlayer();

    _player->ApplyStats(false);
    _player->SetHealth( _player->GetMaxHealth()/2 );
    _player->SetPower(POWER_MANA, _player->GetMaxPower(POWER_MANA)/2 );
    _player->SetPower(POWER_RAGE, 0 );
    _player->SetPower(POWER_ENERGY, _player->GetMaxPower(POWER_ENERGY));
    _player->ApplyStats(true);

    _player->DurabilityLossAll(0.25);

    // update world right away
    MapManager::Instance().GetMap(_player->GetMapId(), _player)->Add(GetPlayer());

    // get corpse nearest graveyard
    WorldSafeLocsEntry const *corpseGrave = NULL;
    CorpsePtr corpse = _player->GetCorpse();
    if((bool)corpse)
        corpseGrave = objmgr.GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam() );

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if(corpseGrave)
    {
        WorldSafeLocsEntry const *ghostGrave = objmgr.GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam() );

        if(corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
    }

    _player->SaveToDB();
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 npcGUID;
    recv_data >> npcGUID;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature *npc)
{
    WorldPacket data;
    uint32 bindspell = 3286, hearthstone_itemid = 6948;

    // update sql homebind
    sDatabase.PExecute("UPDATE `character_homebind` SET `map` = '%u', `zone` = '%u', `position_x` = '%f', `position_y` = '%f', `position_z` = '%f' WHERE `guid` = '%u'", _player->GetMapId(), _player->GetZoneId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetGUIDLow());
    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindZoneId = _player->GetZoneId();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // if a player lost/dropped hist hearthstone, he will get a new one
    if ( !_player->HasItemCount(hearthstone_itemid, 1) && _player->GetBankItemCount(hearthstone_itemid) <1)
    {
        uint16 dest;
        uint8 msg = _player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, hearthstone_itemid, 1, false );
        if( msg == EQUIP_ERR_OK )
        {
            Item* newitem = _player->StoreNewItem( dest, hearthstone_itemid, 1, true);
            _player->SendNewItem(newitem, 1, true, false);
        }
        else
        {
            _player->SendEquipError( msg, NULL, NULL );
        }
    }

    // send spell for bind 3286 bind magic
    data.Initialize(SMSG_SPELL_START, (8+8+4+2+4+2+8) );
    data.append(npc->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << bindspell;                                      // spell id
    data << uint16(0);                                      // cast flags
    data << uint32(0);                                      // time
    data << uint16(0x0002);                                 // target mask
    data.append(_player->GetPackGUID());                    // target's packed guid
    SendPacket( &data );

    data.Initialize(SMSG_SPELL_GO, (8+8+4+2+1+8+1+2+8));
    data.append(npc->GetPackGUID());
    data.append(npc->GetPackGUID());
    data << bindspell;                                      // spell id
    data << uint16(0x0100);                                 // cast flags
    data << uint8(0x01);                                    // targets count
    data << _player->GetGUID();                             // target's full guid
    data << uint8(0x00);                                    // ?
    data << uint16(0x0002);                                 // target mask
    data.append(_player->GetPackGUID());                    // target's packed guid
    SendPacket( &data );

    data.Initialize( SMSG_TRAINER_BUY_SUCCEEDED, (8+4));
    data << npc->GetGUID();
    data << bindspell;
    SendPacket( &data );

    // binding
    data.Initialize( SMSG_BINDPOINTUPDATE, (4+4+4+4+4) );
    data << float(_player->GetPositionX());
    data << float(_player->GetPositionY());
    data << float(_player->GetPositionZ());
    data << uint32(_player->GetMapId());
    data << uint32(_player->GetZoneId());
    SendPacket( &data );

    DEBUG_LOG("New Home Position X is %f",_player->GetPositionX());
    DEBUG_LOG("New Home Position Y is %f",_player->GetPositionY());
    DEBUG_LOG("New Home Position Z is %f",_player->GetPositionZ());
    DEBUG_LOG("New Home MapId is %u",_player->GetMapId());
    DEBUG_LOG("New Home ZoneId is %u",_player->GetZoneId());

    // zone update
    data.Initialize( SMSG_PLAYERBOUND, 8+4 );
    data << uint64(_player->GetGUID());
    data << uint32(_player->GetZoneId());
    SendPacket( &data );

    _player->PlayerTalkClass->CloseGossip();
}

//Need fix
void WorldSession::HandleListStabledPetsOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    WorldPacket data;
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleListStabledPetsOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid )
{
    sLog.outDetail("WORLD: Recv MSG_LIST_STABLED_PETS Send.");

    WorldPacket data(MSG_LIST_STABLED_PETS, 200);           // guess size
    data << uint64 ( guid );

    Pet *pet = _player->GetPet();

    data << uint8(0);                                       // place holder for slot show number
    data << uint8(GetPlayer()->m_stableSlots);

    uint8 num = 0;                                          // counter for place holder

    // not let move dead pet in slot
    if(pet && pet->isAlive() && pet->getPetType()==HUNTER_PET)
    {
        data << uint32(pet->GetPetNumber());
        data << uint32(pet->GetEntry());
        data << uint32(pet->getLevel());
        data << pet->GetName();                             // petname
        data << uint32(pet->getloyalty());                  // loyalty
        data << uint8(0x01);                                // client slot 1 == current pet (0)
        ++num;
    }

    //                                             0       1      2    3       4       5         6
    QueryResult* result = sDatabase.PQuery("SELECT `owner`,`slot`,`id`,`entry`,`level`,`loyalty`,`name` FROM `character_pet` WHERE `owner` = '%u' AND `slot` > 0 AND `slot` < 3",_player->GetGUIDLow());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            data << uint32(fields[2].GetUInt32());          // petnumber
            data << uint32(fields[3].GetUInt32());          // creature entry
            data << uint32(fields[4].GetUInt32());          // level
            data << fields[6].GetString();                  // name
            data << uint32(fields[5].GetUInt32());          // loyalty
            data << uint8(fields[1].GetUInt32()+1);         // slot

            ++num;
        }while( result->NextRow() );

        delete result;
    }

    data.put<uint8>(8, num);                                // set real data to placeholder
    SendPacket(&data);
}

void WorldSession::HandleStablePet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Recv CMSG_STABLE_PET not dispose.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    if(!GetPlayer()->isAlive())
        return;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    Pet *pet = _player->GetPet();

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    // can't place in stable dead pet
    if(!pet||!pet->isAlive()||pet->getPetType()!=HUNTER_PET)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    uint32 free_slot = 1;

    QueryResult *result = sDatabase.PQuery("SELECT `owner`,`slot`,`id` FROM `character_pet` WHERE `owner` = '%u'  AND `slot` > 0 AND `slot` < 3 ORDER BY `slot` ",_player->GetGUIDLow());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 slot = fields[1].GetUInt32();

            if(slot==free_slot)                             // this slot not free
                ++free_slot;
        }while( result->NextRow() );
    }
    delete result;

    if( free_slot > 0 && free_slot <= GetPlayer()->m_stableSlots)
    {
        _player->RemovePet(pet,PetSaveMode(free_slot));
        data << uint8(0x08);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleUnstablePet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: Recv CMSG_UNSTABLE_PET.");
    uint64 npcGUID;
    uint32 petnumber;

    recv_data >> npcGUID >> petnumber;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleUnstablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    Pet* pet = _player->GetPet();
    if(pet && pet->isAlive())
    {
        uint8 i = 0x06;
        data << uint8(i);
        SendPacket(&data);
        return;
    }

    // delete dead pet
    if(pet)
        _player->RemovePet(pet,PET_SAVE_AS_DELETED);

    Pet *newpet = NULL;

    QueryResult *result = sDatabase.PQuery("SELECT `entry` FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u' AND `slot` > 0 AND `slot` < 3",_player->GetGUIDLow(),petnumber);
    if(result)
    {
        Field *fields = result->Fetch();
        uint32 petentry = fields[0].GetUInt32();

        newpet = new Pet(_player, HUNTER_PET);
        if(!newpet->LoadPetFromDB(_player,petentry,petnumber))
        {
            delete newpet;
            newpet = NULL;
        }
        delete result;
    }

    if(newpet)
        data << uint8(0x09);
    else
        data << uint8(0x06);
    SendPacket(&data);
}

void WorldSession::HandleBuyStableSlot( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Recv CMSG_BUY_STABLE_SLOT.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleBuyStableSlot - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);

    if(GetPlayer()->m_stableSlots < 2)                      // max slots amount = 2
    {
        StableSlotPricesEntry const *SlotPrice = sStableSlotPricesStore.LookupEntry(GetPlayer()->m_stableSlots+1);
        if(_player->GetMoney() >= SlotPrice->Price)
        {
            ++GetPlayer()->m_stableSlots;
            _player->SetMoney(_player->GetMoney() - SlotPrice->Price);
            data << uint8(0x0A);                            // success buy
        }
        else
            data << uint8(0x06);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleStableRevivePet( WorldPacket & recv_data )
{
}

void WorldSession::HandleStableSwapPet( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    sLog.outDetail("WORLD: Recv CMSG_STABLE_SWAP_PET.");
    uint64 npcGUID;
    uint32 pet_number;

    recv_data >> npcGUID >> pet_number;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_STABLE);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStableSwapPet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    Pet* pet = _player->GetPet();

    if(!pet || pet->getPetType()!=HUNTER_PET)
        return;

    // find swapped pet slot in stable
    QueryResult *result = sDatabase.PQuery("SELECT `slot`,`entry` FROM `character_pet` WHERE `owner` = '%u' AND `id` = '%u'",_player->GetGUIDLow(),pet_number);
    if(!result)
        return;

    Field *fields = result->Fetch();

    uint32 slot     = fields[0].GetUInt32();
    uint32 petentry = fields[1].GetUInt32();
    delete result;

    // move alive pet to slot or delele dead pet
    _player->RemovePet(pet,pet->isAlive() ? PetSaveMode(slot) : PET_SAVE_AS_DELETED);

    // summon unstabled pet
    Pet *newpet = new Pet(_player, _player->getClass()==CLASS_HUNTER?HUNTER_PET:SUMMON_PET);
    if(!newpet->LoadPetFromDB(_player,petentry,pet_number))
    {
        delete newpet;
        data << uint8(0x06);
    }
    else
        data << uint8(0x09);

    SendPacket(&data);
}

void WorldSession::HandleRepairItemOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE(recv_data,8+8);

    sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");

    uint64 npcGUID, itemGUID;

    recv_data >> npcGUID >> itemGUID;

    Creature *unit = ObjectAccessor::Instance().GetNPCIfCanInteractWith(*_player, npcGUID,UNIT_NPC_FLAG_ARMORER);
    if (!unit)
    {
        sLog.outDebug( "WORLD: HandleStableSwapPet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)) );
        return;
    }

    // 10% reputation discount
    FactionTemplateEntry const* vendor_faction = unit->getFactionTemplateEntry();
    bool discount = vendor_faction && _player->GetReputationRank(vendor_faction->faction) >= REP_HONORED;

    if (itemGUID)
    {
        sLog.outDetail("ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        uint16 pos = _player->GetPosByGuid(itemGUID);

        _player->DurabilityRepair(pos,true,discount);

    }
    else
    {
        sLog.outDetail("ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));

        _player->DurabilityRepairAll(true,discount);
    }
}
