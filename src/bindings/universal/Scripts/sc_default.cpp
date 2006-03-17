

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













