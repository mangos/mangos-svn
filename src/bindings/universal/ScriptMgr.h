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

#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "../../game/Player.h"
#include "../../game/GameObject.h"
#include "../../game/SharedDefines.h"
#include "../../game/GossipDef.h"
#include "../../game/WorldSession.h"

#define MAX_SCRIPTS 1000

struct Script
{
    std::string Name;

    // -- Quest/gossip Methods to be scripted --
    void (*pGossipHello)(Player *player, Creature *_Creature);
    void (*pQuestAccept)(Player *player, Creature *_Creature, Quest *_Quest );
    void (*pGossipSelect)(Player *player, Creature *_Creature, uint32 sender, uint32 action );
    void (*pGossipSelectWithCode)(Player *player, Creature *_Creature, uint32 sender, uint32 action, char* sCode );
    void (*pQuestSelect)(Player *player, Creature *_Creature, Quest *_Quest );
    void (*pQuestComplete)(Player *player, Creature *_Creature, Quest *_Quest );
    uint32 (*pNPCDialogStatus)(Player *player, Creature *_Creature );
    void (*pChooseReward)(Player *player, Creature *_Creature, Quest *_Quest, uint32 opt );
    void (*pItemHello)(Player *player, Item *_Item, Quest *_Quest );
    void (*pGOHello)(Player *player, GameObject *_GO );
    void (*pAreaTrigger)(Player *player, Quest *_Quest, uint32 triggerID );
    void (*pItemQuestAccept)(Player *player, Item *_Item, Quest *_Quest );
    void (*pGOQuestAccept)(Player *player, GameObject *_GO, Quest *_Quest );
    void (*pGOChooseReward)(Player *player, GameObject *_GO, Quest *_Quest, uint32 opt );
    // ----------------------------

};

extern int nrscripts;
extern Script *m_scripts[MAX_SCRIPTS];
#endif
