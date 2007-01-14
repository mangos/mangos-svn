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

#ifndef __SCRIPT_CALLS_H
#define __SCRIPT_CALLS_H

#include "GossipDef.h"
#include "Player.h"

bool LoadScriptingModule(char const* libName = "");

typedef void(MANGOS_IMPORT * scriptCallScriptsInit) ();
typedef void(MANGOS_IMPORT * scriptCallScriptsFree) ();

typedef bool(MANGOS_IMPORT * scriptCallGossipHello) (Player *player, Creature *_Creature );
typedef bool(MANGOS_IMPORT * scriptCallQuestAccept) (Player *player, Creature *_Creature, Quest *);
typedef bool(MANGOS_IMPORT * scriptCallGossipSelect)(Player *player, Creature *_Creature, uint32 sender, uint32 action);
typedef bool(MANGOS_IMPORT * scriptCallGossipSelectWithCode)( Player *player, Creature *_Creature, uint32 sender, uint64 action, char* sCode );
typedef bool(MANGOS_IMPORT * scriptCallQuestSelect)( Player *player, Creature *_Creature, Quest * );
typedef bool(MANGOS_IMPORT * scriptCallQuestComplete)(Player *player, Creature *_Creature, Quest *);
typedef uint32(MANGOS_IMPORT * scriptCallNPCDialogStatus)( Player *player, Creature *_Creature);
typedef bool(MANGOS_IMPORT * scriptCallChooseReward)( Player *player, Creature *_Creature, Quest *, uint32 opt );
typedef bool(MANGOS_IMPORT * scriptCallItemHello)( Player *player, Item *, Quest *);
typedef bool(MANGOS_IMPORT * scriptCallGOHello)( Player *player, GameObject * );
typedef bool(MANGOS_IMPORT * scriptCallAreaTrigger)( Player *player, Quest *, uint32 triggerID );
typedef bool(MANGOS_IMPORT * scriptCallItemQuestAccept)(Player *player, Item *, Quest *);
typedef bool(MANGOS_IMPORT * scriptCallGOQuestAccept)(Player *player, GameObject *, Quest *);
typedef bool(MANGOS_IMPORT * scriptCallGOChooseReward)(Player *player, GameObject *, Quest *, uint32 opt );
typedef bool(MANGOS_IMPORT * scriptCallReciveEmote) ( Player *player, Creature *_Creature, uint32 emote );
typedef CreatureAI* (MANGOS_IMPORT * scriptCallGetAI)  ( Creature *_Creature );

typedef struct
{
    scriptCallScriptsInit ScriptsInit;
    scriptCallScriptsFree ScriptsFree;

    scriptCallGossipHello GossipHello;
    scriptCallGOChooseReward GOChooseReward;
    scriptCallQuestAccept QuestAccept;
    scriptCallGossipSelect GossipSelect;
    scriptCallGossipSelectWithCode GossipSelectWithCode;
    scriptCallQuestSelect QuestSelect;
    scriptCallQuestComplete QuestComplete;
    scriptCallNPCDialogStatus NPCDialogStatus;
    scriptCallChooseReward ChooseReward;
    scriptCallItemHello ItemHello;
    scriptCallGOHello GOHello;
    scriptCallAreaTrigger scriptAreaTrigger;
    scriptCallItemQuestAccept ItemQuestAccept;
    scriptCallGOQuestAccept GOQuestAccept;
    scriptCallReciveEmote ReciveEmote;
    scriptCallGetAI GetAI;

    MANGOS_LIBRARY_HANDLE hScriptsLib;
}_ScriptSet,*ScriptsSet;

extern ScriptsSet Script;
#endif
