

#include "CreatureAIRegistry.h"
#include "NullCreatureAI.h"
#include "ReactorAI.h"
#include "AggressorAI.h"


namespace AIRegistry
{
    static void Initialize()
    {
	(new CreatureAIFactory<NullCreatureAI>("NullAI"))->RegisterSelf();
	(new CreatureAIFactory<AggressorAI>("AggressorAI"))->RegisterSelf();
	(new CreatureAIFactory<ReactorAI>("ReactorAI"))->RegisterSelf();
    }
}
