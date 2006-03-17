

#include "sc_defines.h"

	
uint32 GetSkillLevel(Player *player,uint32 trskill)
{
    // Returns the level of some tradetrskill known by player
    // Need to add missing spells

    uint32 spell_apprentice = 0;
    uint32 spell_journeyman = 0;
    uint32 spell_expert     = 0;
    uint32 spell_artisan    = 0;

		switch(trskill)
		{
			case TRADESKILL_ALCHEMY:
				spell_apprentice = 2259;
        spell_journeyman = 3101;
        spell_expert     = 3464;
        spell_artisan    = 11611;
        break;
      case TRADESKILL_BLACKSMITHING:
				spell_apprentice = 2018;
        spell_journeyman = 3100;
        spell_expert     = 8768;
        spell_artisan    = 11454;
        break;
      case TRADESKILL_COOKING:
      	spell_apprentice = 2550;
        spell_journeyman = 3102;
        spell_expert     = 3413;
        spell_artisan    = 18260;
        break;
      case TRADESKILL_ENCHANTING:
      	spell_apprentice = 7411;
        spell_journeyman = 7412;
        spell_expert     = 7413;
        spell_artisan    = 13920;
        break;
      case TRADESKILL_ENGINEERING:
      	spell_apprentice = 4036;
        spell_journeyman = 4037;
        spell_expert     = 4038;
        spell_artisan    = 12656;
      	break;
     	case TRADESKILL_FIRSTAID:
      	spell_apprentice = 3273;
        spell_journeyman = 3274;
        spell_expert     = 7924;
        spell_artisan    = 10846;
        break;
      case TRADESKILL_HERBALISM:
        spell_apprentice = 2372;
        spell_journeyman = 2373;
        spell_expert     = 3571;
        spell_artisan    = 11994;
        break;
      case TRADESKILL_LEATHERWORKING:
      	spell_apprentice = 2108;
        spell_journeyman = 3104;
        spell_expert     = 20649;
        spell_artisan    = 10662;
        break;
      case TRADESKILL_POISONS:
      	spell_apprentice = 0;
        spell_journeyman = 0;
        spell_expert     = 0;
        spell_artisan    = 0;
        break;
      case TRADESKILL_TAILORING:
      	spell_apprentice = 3908;
        spell_journeyman = 3909;
        spell_expert     = 3910;
        spell_artisan    = 12180;
        break;
      case TRADESKILL_MINING:
        spell_apprentice = 2581;
        spell_journeyman = 2582;
        spell_expert     = 3568;
        spell_artisan    = 10249;
        break;
      case TRADESKILL_FISHING:  
        spell_apprentice = 7733;
        spell_journeyman = 7734;
        spell_expert     = 7736;
        spell_artisan    = 18249;
        break;
      case TRADESKILL_SKINNING:
        spell_apprentice = 8615;
        spell_journeyman = 8619;
        spell_expert     = 8620;
        spell_artisan    = 10769;
        break;
		}
        
    if (player->HasSpell(spell_artisan))
        return TRADESKILL_LEVEL_ARTISAN;

    if (player->HasSpell(spell_expert))
        return TRADESKILL_LEVEL_EXPERT;

    if (player->HasSpell(spell_journeyman))
        return TRADESKILL_LEVEL_JOURNEYMAN;

    if (player->HasSpell(spell_apprentice))
        return TRADESKILL_LEVEL_APPRENTICE;

    return TRADESKILL_LEVEL_NONE;
}
