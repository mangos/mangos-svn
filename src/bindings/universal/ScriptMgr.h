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

#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "../../game/Player.h"
#include "../../game/GameObject.h"
#include "../../game/SharedDefines.h"
#include "../../game/GossipDef.h"
#include "../../game/QuestDef.h"
#include "../../game/WorldSession.h"
#include "../../game/CreatureAI.h"

#define MAX_SCRIPTS 1000

struct Script
{
    Script() :
    pGossipHello(NULL), pQuestAccept(NULL), pGossipSelect(NULL), pGossipSelectWithCode(NULL),
        pQuestSelect(NULL), pQuestComplete(NULL), pNPCDialogStatus(NULL), pChooseReward(NULL),
        pItemHello(NULL), pGOHello(NULL), pAreaTrigger(NULL), pItemQuestAccept(NULL), pGOQuestAccept(NULL),
        pGOChooseReward(NULL), pReceiveEmote(NULL), GetAI(NULL)
        {}

    std::string Name;

    // -- Quest/gossip Methods to be scripted --
    bool (*pGossipHello         )(Player *player, Creature *_Creature);
    bool (*pQuestAccept         )(Player *player, Creature *_Creature, Quest *_Quest );
    bool (*pGossipSelect        )(Player *player, Creature *_Creature, uint32 sender, uint32 action );
    bool (*pGossipSelectWithCode)(Player *player, Creature *_Creature, uint32 sender, uint32 action, char* sCode );
    bool (*pQuestSelect         )(Player *player, Creature *_Creature, Quest *_Quest );
    bool (*pQuestComplete       )(Player *player, Creature *_Creature, Quest *_Quest );
    uint32 (*pNPCDialogStatus   )(Player *player, Creature *_Creature );
    bool (*pChooseReward        )(Player *player, Creature *_Creature, Quest *_Quest, uint32 opt );
    bool (*pItemHello           )(Player *player, Item *_Item, Quest *_Quest );
    bool (*pGOHello             )(Player *player, GameObject *_GO );
    bool (*pAreaTrigger         )(Player *player, Quest *_Quest, uint32 triggerID );
    bool (*pItemQuestAccept     )(Player *player, Item *_Item, Quest *_Quest );
    bool (*pGOQuestAccept       )(Player *player, GameObject *_GO, Quest *_Quest );
    bool (*pGOChooseReward      )(Player *player, GameObject *_GO, Quest *_Quest, uint32 opt );
    bool (*pReceiveEmote        )(Player *player, Creature *_Creature, uint32 emote );

    CreatureAI* (*GetAI)(Creature *_Creature);
    // -----------------------------------------

};

extern int nrscripts;
extern Script *m_scripts[MAX_SCRIPTS];

#define VISIBLE_RANGE (26.46f)

struct MANGOS_DLL_DECL ScriptedAI : public CreatureAI
{
    ScriptedAI(Creature* creature) : m_creature(creature) {}
    ~ScriptedAI() {}

    // Called if IsVisible(Unit *who) is true at each *who move
    void MoveInLineOfSight(Unit *) {}

    // Called at each attack of m_creature by any victim
    void AttackStart(Unit *) {}

    // Called at stoping attack by any attacker
    void EnterEvadeMode();

    // Called at any heal cast/item used (call non implemented in mangos)
    void HealBy(Unit *healer, uint32 amount_healed) {}

    // Called at any Damage from any attacker
    void DamageInflict(Unit *healer, uint32 amount_healed) {}

    // Is unit visibale for MoveInLineOfSight
    bool IsVisible(Unit *who) const
    {
        return !who->HasStealthAura() && m_creature->GetDistanceSq(who) <= VISIBLE_RANGE;
    }

    // Called at World update tick
    void UpdateAI(const uint32);

    // Called when the creature is killed
    void JustDied(Unit *){}

    // Called when the creature kills a unit
    void KilledUnit(Unit *){}

    // Called when hit by a spell
    void SpellHit(Unit *, const SpellEntry*){}

    Creature* m_creature;

    // Check condition for attack stop
    virtual bool needToStop() const;

    //= Some useful helpers =========================

    // Start attack of victim and go to him
    void DoStartAttack(Unit* victim);

    // Stop attack of current victim
    void DoStopAttack();

    // Cast spell
    void DoCast(Unit* victim, uint32 spelId)
    {
        m_creature->CastSpell(victim,spelId,true);
    }

    void DoCastSpell(Unit* who,SpellEntry *spellInfo)
    {
        m_creature->CastSpell(who,spellInfo,true);
    }

    void DoSay(char const* text, uint32 language)
    {
        m_creature->Say(text,language,0);
    }

    void DoGoHome();
};
#endif
