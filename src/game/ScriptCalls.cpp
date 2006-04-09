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

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "Platform/Define.h"
#include "ScriptCalls.h"

ScriptsSet Script=NULL;

#define CLOSE_LIB {MANGOS_CLOSE_LIBRARY(testScript->hScriptsLib);delete testScript;return false;}
bool LoadScriptingModule()
{
    ScriptsSet testScript=new _ScriptSet;

    testScript->hScriptsLib=MANGOS_LOAD_LIBRARY(MANGOS_SCRIPT_FILE);

    if(!testScript->hScriptsLib )
    {
        printf("Error loading Scripts Library!\n");
        return false;
    }
    else printf("Scripts Library was successfully loaded.\n");

    if(!(testScript->ScriptsInit=(scriptCallScriptsInit)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"ScriptsInit")))
        CLOSE_LIB

            if(!(testScript->GossipHello=(scriptCallGossipHello)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GossipHello")))
            CLOSE_LIB

                if(!(testScript->GOChooseReward=(scriptCallGOChooseReward)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GOChooseReward")))
                CLOSE_LIB

                    if(!(testScript->QuestAccept=(scriptCallQuestAccept)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"QuestAccept")))
                    CLOSE_LIB

                        if(!(testScript->GossipSelect=(scriptCallGossipSelect)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GossipSelect")))
                        CLOSE_LIB

                            if(!(testScript->GossipSelectWithCode=(scriptCallGossipSelectWithCode)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GossipSelectWithCode")))
                            CLOSE_LIB

                                if(!(testScript->QuestSelect=(scriptCallQuestSelect)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"QuestSelect")))
                                CLOSE_LIB

                                    if(!(testScript->QuestComplete=(scriptCallQuestComplete)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"QuestComplete")))
                                    CLOSE_LIB

                                        if(!(testScript->NPCDialogStatus=(scriptCallNPCDialogStatus)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"NPCDialogStatus")))
                                        CLOSE_LIB

                                            if(!(testScript->ChooseReward=(scriptCallChooseReward)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"ChooseReward")))
                                            CLOSE_LIB

                                                if(!(testScript->ItemHello=(scriptCallItemHello)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"ItemHello")))
                                                CLOSE_LIB

                                                    if(!(testScript->GOHello=(scriptCallGOHello)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GOHello")))
                                                    CLOSE_LIB

                                                        if(!(testScript->scriptAreaTrigger=(scriptCallAreaTrigger)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"AreaTrigger")))
                                                        CLOSE_LIB

                                                            if(!(testScript->ItemQuestAccept=(scriptCallItemQuestAccept)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"ItemQuestAccept")))
                                                            CLOSE_LIB

                                                                if(!(testScript->GOQuestAccept=(scriptCallGOQuestAccept)MANGOS_GET_PROC_ADDR(testScript->hScriptsLib,"GOQuestAccept")))
                                                                CLOSE_LIB

                                                            //heh we are still there :P we have a valid library
                                                            //we reload script
                                                                    if(Script)
    {
        ScriptsSet current =testScript;
        //todo: some check if some func from script library is called right now
        Script=testScript;
        MANGOS_CLOSE_LIBRARY(current->hScriptsLib);
        delete current;
    }else
    Script=testScript;

    return true;

}
