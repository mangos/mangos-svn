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

#include "sc_defines.h"

void GossipHello_default(Player *player, Creature *_Creature)
{}

void GossipSelect_default(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{}

void GossipSelectWithCode_default( Player *player, Creature *_Creature, uint32 sender, uint32 action, char* sCode )
{}
void QuestAccept_default(Player *player, Creature *_Creature, Quest *_Quest )
{}

void QuestSelect_default(Player *player, Creature *_Creature, Quest *_Quest )
{}

void QuestComplete_default(Player *player, Creature *_Creature, Quest *_Quest )
{}

void ChooseReward_default(Player *player, Creature *_Creature, Quest *_Quest, uint32 opt )
{}

uint32 NPCDialogStatus_default(Player *player, Creature *_Creature )
{
    return 0;
}

void ItemHello_default(Player *player, Item *_Item, Quest *_Quest )
{}

void ItemQuestAccept_default(Player *player, Item *_Item, Quest *_Quest )
{}

void GOHello_default(Player *player, GameObject *_GO )
{}

void GOQuestAccept_default(Player *player, GameObject *_GO, Quest *_Quest )
{}
void GOChooseReward_default(Player *player, GameObject *_GO, Quest *_Quest, uint32 opt )
{}

void AreaTrigger_default(Player *player, Quest *_Quest, uint32 triggerID )
{}

void AddSC_default()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="default";
    newscript->pGossipHello          = &GossipHello_default;
    newscript->pQuestAccept          = &QuestAccept_default;
    newscript->pGossipSelect         = &GossipSelect_default;
    newscript->pGossipSelectWithCode = &GossipSelectWithCode_default;
    newscript->pQuestSelect          = &QuestSelect_default;
    newscript->pQuestComplete        = &QuestComplete_default;
    newscript->pNPCDialogStatus      = &NPCDialogStatus_default;
    newscript->pChooseReward         = &ChooseReward_default;
    newscript->pItemHello            = &ItemHello_default;
    newscript->pGOHello              = &GOHello_default;
    newscript->pAreaTrigger          = &AreaTrigger_default;
    newscript->pItemQuestAccept      = &ItemQuestAccept_default;
    newscript->pGOQuestAccept        = &GOQuestAccept_default;
    newscript->pGOChooseReward       = &GOChooseReward_default;

    m_scripts[nrscripts++] = newscript;
}
