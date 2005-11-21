/* ScriptCalls.cpp
 *
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

#include "Platform/Define.h"
#include <string>

class Player;
class Creature;
class Quest;
class Item;
class GameObject;

void scriptCallGossipHello( Player *_Player, Creature *_Creature )
{
}

void scriptCallQuestAccept( Player *_Player, Creature *_Creature, Quest *_Quest )
{
}

void scriptCallGossipSelect( Player *_Player, Creature *_Creature, uint32 opt, uint64 data )
{
}

void scriptCallGossipSelectWithCode( Player *_Player, Creature *_Creature, uint32 opt, uint64 data, char* sCode )
{
}

void scriptCallQuestSelect( Player *_Player, Creature *_Creature, Quest *_Quest )
{
}

void scriptCallQuestComplete( Player *_Player, Creature *_Creature, Quest *_Quest )
{
}

uint32 scriptCallNPCDialogStatus( Player *pPlayer, Creature *_Creature )
{
	return 0;
}

void scriptCallChooseReward( Player *_Player, Creature *_Creature, Quest *_Quest, uint32 opt )
{
}

void scriptCallItemHello( Player *_Player, Item *_Item, Quest *_Quest )
{
}

void scriptCallGOHello( Player *_Player, GameObject *_GO )
{
}

void scriptCallAreaTrigger( Player *_Player, Quest *_Quest, uint32 triggerID )
{
}

void scriptCallItemQuestAccept( Player *_Player, Item *_Item, Quest *_Quest )
{
}

void scriptCallGOQuestAccept( Player *_Player, GameObject *_GO, Quest *_Quest )
{
}

void scriptCallGOChooseReward( Player *_Player, GameObject *_GameObject, Quest *_Quest, uint32 opt )
{
}
