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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "GameObject.h"
#include "Chat.h"
#include "Log.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "SpellAuras.h"
#include "ScriptCalls.h"
#include "Language.h"

bool ChatHandler::HandleReloadCommand(const char* args)
{
    return true;
}

bool ChatHandler::HandleLoadScriptsCommand(const char* args)
{
    if(!LoadScriptingModule(args)) return true;

    sWorld.SendWorldText(LANG_SCRITPS_RELOADED, NULL);

    return true;
}

bool ChatHandler::HandleSecurityCommand(const char* args)
{
    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* pgm = strtok(NULL, " ");
    if (!pgm)
        return false;

    int8 gm = (uint8) atoi(pgm);
    if ( gm < 0 || gm > 5)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return true;
    }

    Player* chr = ObjectAccessor::Instance().FindPlayerByName(args);

    if (chr)
    {

        PSendSysMessage(LANG_YOU_CHANGE_SECURITY, chr->GetName(), gm);

        WorldPacket data;
        char buf[256];
        sprintf((char*)buf,LANG_YOURS_SECURITY_CHANGED, m_session->GetPlayer()->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);

        chr->GetSession()->SendPacket(&data);
        chr->GetSession()->SetSecurity(gm);

        sDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%i' WHERE `id` = '%u'", gm, chr->GetSession()->GetAccountId());

    }
    else
        PSendSysMessage(LANG_NO_PLAYER, pName);

    return true;
}

bool ChatHandler::HandleWorldPortCommand(const char* args)
{
    char* pContinent = strtok((char*)args, " ");
    if (!pContinent)
        return false;

    char* px = strtok(NULL, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    float x = atof(px);
    float y = atof(py);
    float z = atof(pz);
    uint32 mapid = atoi(pContinent);

    if(!MapManager::ExistMAP(mapid,x,y))
    {
        PSendSysMessage(".worldport target map not exist (X: %f Y: %f MapId: %u)",x,y,mapid);
        return true;
    }

    m_session->GetPlayer()->TeleportTo(mapid, x, y, z,0.0f);

    return true;
}

bool ChatHandler::HandleAllowMovementCommand(const char* args)
{
    if(sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
    }
    else
    {
        sWorld.SetAllowMovement(true);
        SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
    }
    return true;
}

bool ChatHandler::HandleAddSpiritCommand(const char* args)
{
    sLog.outDetail(LANG_SPAWNING_SPIRIT_HEAL);
    /*
        Creature* pCreature;
        UpdateMask unitMask;
        WorldPacket data;

        QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`F`,`name_id`,`map`,`zone`,`faction` FROM `npc_spirithealer`");

        if(!result)
        {
            PSendSysMessage(LANG_NO_SPIRIT_HEAL_DB);
            return true;
        }

        uint32 name;

        do
        {
            Field* fields = result->Fetch();

            name = fields[4].GetUInt32();
            sLog.outDetail("%s name is %d\n", fields[4].GetString(), name);

            pCreature = new Creature();

            if(!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), objmgr.GetCreatureTemplate(name)->Name, fields[5].GetUInt16(),
                fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), name))
            {
                delete pCreature;
                return false;
            }

            pCreature->SetZoneId( fields[6].GetUInt16() );
            pCreature->SetUInt32Value( OBJECT_FIELD_ENTRY, name );
            pCreature->SetFloatValue( OBJECT_FIELD_SCALE_X, 1.0f );
            pCreature->SetUInt32Value( UNIT_FIELD_DISPLAYID, 5233 );
            pCreature->SetUInt32Value( UNIT_NPC_FLAGS , 1 );
            pCreature->SetUInt32Value( UNIT_FIELD_FACTIONTEMPLATE , fields[7].GetUInt32() );
            pCreature->SetHealth( 100 + 30*(60) );
            pCreature->SetMaxHealth( 100 + 30*(60) );
            pCreature->SetLevel( 60 );
            pCreature->SetFloatValue( UNIT_FIELD_COMBATREACH , 1.5f );
            pCreature->SetFloatValue( UNIT_FIELD_MAXDAMAGE ,  5.0f );
            pCreature->SetFloatValue( UNIT_FIELD_MINDAMAGE , 8.0f );
            pCreature->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME, 1900 );
            pCreature->SetUInt32Value( UNIT_FIELD_RANGEDATTACKTIME, 2000 );
            pCreature->SetFloatValue( UNIT_FIELD_BOUNDINGRADIUS, 2.0f );

            sLog.outError("AddObject at Level3.cpp line 185");

            pCreature->AIM_Initialize();

            MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);

            //pCreature->SaveToDB();
        }
        while( result->NextRow() );

        delete result;
    */
    return true;
}

bool ChatHandler::HandleGoCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py || !pz || !pmapid)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    uint32 mapid = (uint32)atoi(pmapid);

    if(!MapManager::ExistMAP(mapid,x,y))
    {
        PSendSysMessage(".go target map not exist (X: %f Y: %f MapId:%u)",x,y,mapid);
        return true;
    }

    m_session->GetPlayer()->TeleportTo(mapid, x, y, z,0.0f);

    return true;
}

bool ChatHandler::HandleLearnSkillCommand (const char* args)
{
    bool syntax_error = false;

    if (!*args) syntax_error = true;

    uint32 skill = 0;
    uint16 level = 1;
    uint16 max = 1;
    char args1[512];
    strcpy (args1, args);

    if (!syntax_error)
    {
        char *p = strtok (args1, " ");
        if (p)
        {
            skill = atol (p);
            p = strtok (NULL, " ");
            if (p)
            {
                level = atoi (p);
                p = strtok (NULL, " ");
                if (p)
                {
                    max = atoi (p);
                }
                else
                {
                    syntax_error = true;
                }
            }
            else
            {
                syntax_error = true;
            }
        }
        else
        {
            syntax_error = true;
        }
    }

    if (syntax_error)
    {
        SendSysMessage(LANG_LEARNSK_SYNTAX);
        return true;
    }

    Player * target = getSelectedPlayer();
    if(!target)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if (skill > 0)
    {
        target->SetSkill(skill, level, max);
        PSendSysMessage(LANG_LEARNED_SKILL, skill);
    }
    else
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);

    return true;
}

bool ChatHandler::HandleUnLearnSkillCommand (const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_UNLEARNSK_SYNTAX);
        return true;
    }

    uint32 skill = 0;
    char args1[512];
    strcpy (args1, args);

    skill = atol (strtok(args1, " "));

    if (skill <= 0)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        return true;
    }

    Player * target = getSelectedPlayer();
    if(!target)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if (target->GetSkillValue(skill))
    {
        target->SetSkill(skill, 0, 0);
        PSendSysMessage(LANG_UNLEARNED_SKILL, skill);
    } else
    {
        SendSysMessage(LANG_UNKNOWN_SKILL);
    }
    return true;
}

const char *gmSpellList[] =
{
    "3365",
    "6233",
    "6247",
    "6246",
    "6477",
    "6478",
    "22810",
    "8386",
    "21651",
    "21652",
    "522",
    "7266",
    "8597",
    "2479",
    "22027",
    "6603",
    "5019",
    "133",
    "168",
    "227",
    "5009",
    "9078",
    "668",
    "203",
    "20599",
    "20600",
    "81",
    "20597",
    "20598",
    "20864",
    "1459",
    "5504",
    "587",
    "5143",
    "118",
    "5505",
    "597",
    "604",
    "1449",
    "1460",
    "2855",
    "1008",
    "475",
    "5506",
    "1463",
    "12824",
    "8437",
    "990",
    "5145",
    "8450",
    "1461",
    "759",
    "8494",
    "8455",
    "8438",
    "6127",
    "8416",
    "6129",
    "8451",
    "8495",
    "8439",
    "3552",
    "8417",
    "10138",
    "12825",
    "10169",
    "10156",
    "10144",
    "10191",
    "10201",
    "10211",
    "10053",
    "10173",
    "10139",
    "10145",
    "10192",
    "10170",
    "10202",
    "10054",
    "10174",
    "10193",
    "12826",
    "2136",
    "143",
    "145",
    "2137",
    "2120",
    "3140",
    "543",
    "2138",
    "2948",
    "8400",
    "2121",
    "8444",
    "8412",
    "8457",
    "8401",
    "8422",
    "8445",
    "8402",
    "8413",
    "8458",
    "8423",
    "8446",
    "10148",
    "10197",
    "10205",
    "10149",
    "10215",
    "10223",
    "10206",
    "10199",
    "10150",
    "10216",
    "10207",
    "10225",
    "10151",
    "116",
    "205",
    "7300",
    "122",
    "837",
    "10",
    "7301",
    "7322",
    "6143",
    "120",
    "865",
    "8406",
    "6141",
    "7302",
    "8461",
    "8407",
    "8492",
    "8427",
    "8408",
    "6131",
    "7320",
    "10159",
    "8462",
    "10185",
    "10179",
    "10160",
    "10180",
    "10219",
    "10186",
    "10177",
    "10230",
    "10181",
    "10161",
    "10187",
    "10220",
    "2018",
    "2663",
    "12260",
    "2660",
    "3115",
    "3326",
    "2665",
    "3116",
    "2738",
    "3293",
    "2661",
    "3319",
    "2662",
    "9983",
    "8880",
    "2737",
    "2739",
    "7408",
    "3320",
    "2666",
    "3323",
    "3324",
    "3294",
    "22723",
    "23219",
    "23220",
    "23221",
    "23228",
    "23338",
    "10788",
    "10790",
    "5611",
    "5016",
    "5609",
    "2060",
    "10963",
    "10964",
    "10965",
    "22593",
    "22594",
    "596",
    "996",
    "499",
    "768",
    "17002",
    "1448",
    "1082",
    "16979",
    "1079",
    "5215",
    "20484",
    "5221",
    "15590",
    "17007",
    "6795",
    "6807",
    "5487",
    "1446",
    "1066",
    "5421",
    "3139",
    "779",
    "6811",
    "6808",
    "1445",
    "5216",
    "1737",
    "5222",
    "5217",
    "1432",
    "6812",
    "9492",
    "5210",
    "3030",
    "1441",
    "783",
    "6801",
    "20739",
    "8944",
    "9491",
    "22569",
    "5226",
    "6786",
    "1433",
    "8973",
    "1828",
    "9495",
    "9006",
    "6794",
    "8993",
    "5203",
    "16914",
    "6784",
    "9635",
    "22830",
    "20722",
    "9748",
    "6790",
    "9753",
    "9493",
    "9752",
    "9831",
    "9825",
    "9822",
    "5204",
    "5401",
    "22831",
    "6793",
    "9845",
    "17401",
    "9882",
    "9868",
    "20749",
    "9893",
    "9899",
    "9895",
    "9832",
    "9902",
    "9909",
    "22832",
    "9828",
    "9851",
    "9883",
    "9869",
    "17406",
    "17402",
    "9914",
    "20750",
    "9897",
    "9848",
    "3127",
    "107",
    "204",
    "9116",
    "2457",
    "78",
    "18848",
    "331",
    "403",
    "2098",
    "1752",
    "11278",
    "11288",
    "11284",
    "6461",
    "2344",
    "2345",
    "6463",
    "2346",
    "2352",
    "775",
    "1434",
    "1612",
    "71",
    "2468",
    "2458",
    "2467",
    "7164",
    "7178",
    "7367",
    "7376",
    "7381",
    "21156",
    "5209",
    "3029",
    "5201",
    "9849",
    "9850",
    "20719",
    "22568",
    "22827",
    "22828",
    "22829",
    "6809",
    "8972",
    "9005",
    "9823",
    "9827",
    "6783",
    "9913",
    "6785",
    "6787",
    "9866",
    "9867",
    "9894",
    "9896",
    "6800",
    "8992",
    "9829",
    "9830",
    "780",
    "769",
    "6749",
    "6750",
    "9755",
    "9754",
    "9908",
    "20745",
    "20742",
    "20747",
    "20748",
    "9746",
    "9745",
    "9880",
    "9881",
    "5391",
    "842",
    "3025",
    "3031",
    "3287",
    "3329",
    "1945",
    "3559",
    "4933",
    "4934",
    "4935",
    "4936",
    "5142",
    "5390",
    "5392",
    "5404",
    "5420",
    "6405",
    "7293",
    "7965",
    "8041",
    "8153",
    "9033",
    "9034",
    "9036",
    "16421",
    "21653",
    "22660",
    "5225",
    "9846",
    "2426",
    "5916",
    "6634",
    "6718",
    "6719",
    "8822",
    "9591",
    "9590",
    "10032",
    "17746",
    "17747",
    "885",
    "886",
    "8203",
    "10228",
    "11392",
    "12495",
    "16380",
    "23452",
    "4079",
    "4996",
    "4997",
    "4998",
    "4999",
    "5000",
    "6348",
    "6349",
    "6481",
    "6482",
    "6483",
    "6484",
    "11362",
    "11410",
    "11409",
    "12510",
    "12509",
    "12885",
    "13142",
    "21463",
    "23460",
    "11421",
    "11416",
    "11418",
    "1851",
    "10059",
    "11423",
    "11417",
    "11422",
    "11419",
    "11424",
    "11420",
    "27",
    "31",
    "33",
    "34",
    "35",
    "15125",
    "21127",
    "22950",
    "1180",
    "201",
    "12593",
    "12842",
    "16770",
    "6057",
    "12051",
    "18468",
    "12606",
    "12605",
    "18466",
    "12502",
    "12043",
    "15060",
    "12042",
    "12341",
    "12848",
    "12344",
    "12353",
    "18460",
    "11366",
    "12350",
    "12352",
    "13043",
    "11368",
    "11113",
    "12400",
    "11129",
    "16766",
    "12573",
    "15053",
    "12580",
    "12475",
    "12472",
    "12953",
    "12488",
    "11189",
    "12985",
    "12519",
    "16758",
    "11958",
    "12490",
    "11426",
    "3565",
    "3562",
    "18960",
    "3567",
    "3561",
    "3566",
    "3563",
    "1953",
    "2139",
    "12505",
    "13018",
    "12522",
    "12523",
    "5146",
    "5144",
    "5148",
    "8419",
    "8418",
    "10213",
    "10212",
    "10157",
    "12524",
    "13019",
    "12525",
    "13020",
    "12526",
    "13021",
    "18809",
    "13031",
    "13032",
    "13033",
    "4036",
    "3920",
    "3919",
    "3918",
    "7430",
    "3922",
    "3923",
    "3921",
    "7411",
    "7418",
    "7421",
    "13262",
    "7412",
    "7415",
    "7413",
    "7416",
    "13920",
    "13921",
    "7745",
    "7779",
    "7428",
    "7457",
    "7857",
    "7748",
    "7426",
    "13421",
    "7454",
    "13378",
    "7788",
    "14807",
    "14293",
    "7795",
    "6296",
    "20608",
    "755",
    "444",
    "427",
    "428",
    "442",
    "447",
    "3578",
    "3581",
    "19027",
    "3580",
    "665",
    "3579",
    "3577",
    "6755",
    "3576",
    "2575",
    "2577",
    "2578",
    "2579",
    "2580",
    "2656",
    "2657",
    "2576",
    "3564",
    "10248",
    "8388",
    "2659",
    "14891",
    "3308",
    "3307",
    "10097",
    "2658",
    "3569",
    "16153",
    "3304",
    "10098",
    "4037",
    "3929",
    "3931",
    "3926",
    "3924",
    "3930",
    "3977",
    "3925",
    "95",
    "6",
    "8",
    "136",
    "228",
    "415",
    "98",
    "162",
    "164",
    "473",
    "5487",
    "173",
    "43",
    "202",
    "186",
    "0"
};

bool ChatHandler::HandleLearnCommand(const char* args)
{
    if (!*args)
        return false;

    if (!strcmp(args, "all"))
    {
        int loop = 0;

        PSendSysMessage(LANG_LEARNING_GM_SKILLS, m_session->GetPlayer()->GetName());

        while (strcmp(gmSpellList[loop], "0"))
        {
            uint32 spell = atol((char*)gmSpellList[loop]);

            if (m_session->GetPlayer()->HasSpell(spell))
            {
                loop++;
                continue;
            }
            m_session->GetPlayer()->learnSpell((uint16)spell);

            loop++;
        }

        return true;
    }

    if (!strcmp(args, "all_myclass"))
    {
        PSendSysMessage("%s - Learning all spells/skills for its class.", m_session->GetPlayer()->GetName());

        uint32 family = 0;
        switch(m_session->GetPlayer()->getClass())
        {
            case CLASS_WARRIOR: family = SPELLFAMILY_WARRIOR; break;
            case CLASS_PALADIN: family = SPELLFAMILY_PALADIN; break;
            case CLASS_HUNTER: family = SPELLFAMILY_HUNTER; break;
            case CLASS_ROGUE: family = SPELLFAMILY_ROGUE; break;
            case CLASS_PRIEST: family = SPELLFAMILY_PRIEST; break;
            case CLASS_SHAMAN: family = SPELLFAMILY_SHAMAN; break;
            case CLASS_MAGE: family = SPELLFAMILY_MAGE; break;
            case CLASS_WARLOCK: family = SPELLFAMILY_WARLOCK; break;
            case CLASS_DRUID: family = SPELLFAMILY_DRUID; break;
            default: return true;
        }

        for (uint32 i = 0; i < sSpellStore.GetNumRows(); i++)
        {
            SpellEntry *spellInfo = sSpellStore.LookupEntry(i);
            if (spellInfo && spellInfo->SpellFamilyName == family && !m_session->GetPlayer()->HasSpell(i))
                m_session->GetPlayer()->learnSpell((uint16)i);
        }

        return true;
    }

    Player* target = getSelectedPlayer();
    if(!target)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    uint32 spell = atol((char*)args);

    if (target->HasSpell(spell))
    {
        SendSysMessage(LANG_KNOWN_SPELL);
        return true;
    }

    target->learnSpell((uint16)spell);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 minS;
    uint32 maxS;
    uint32 tmp;

    char* startS = strtok((char*)args, " ");
    char* endS = strtok(NULL, " ");

    if (!endS)
    {
        minS = (uint32)atol(startS);
        maxS =  minS+1;
    }
    else
    {
        minS = (uint32)atol(startS);
        maxS = (uint32)atol(endS);
        if (maxS >= minS)
        {
            maxS=maxS+1;
        }
        else
        {
            tmp=maxS;
            maxS=minS+1;
            tmp=maxS;
        }
    }

    Player* target = getSelectedPlayer();
    if(!target)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    for(uint32 spell=minS;spell<maxS;spell++)
    {
        if (target->HasSpell(spell))
        {
            WorldPacket data;
            data.Initialize(SMSG_REMOVED_SPELL);
            data << (uint32)spell;
            target->GetSession()->SendPacket( &data );
            target->removeSpell(spell);
        }
        else
            SendSysMessage(LANG_FORGET_SPELL);
    }

    return true;
}

bool ChatHandler::HandleAddItemCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_ADDITEM_SYNTAX);
        return true;
    }

    char* citemId = strtok((char*)args, " ");
    char* ccount = strtok(NULL, " ");
    uint32 itemId = atol(citemId);
    uint32 count = 1;

    if (itemId == 0)
    {
        char *citemName = strtok(NULL, "");
        QueryResult *result = sDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", citemName);
        if (!result)
        {
            WorldPacket data;
            FillSystemMessageData(&data, m_session, fmtstring("Could not find '%s'", citemName));
            m_session->SendPacket(&data);
            return true;
        }
        itemId = result->Fetch()->GetUInt16();
        delete result;
    }

    if (ccount) { count = atol(ccount); }
    if (count < 1) { count = 1; }

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if(!plTarget)
        plTarget = pl;

    sLog.outDetail(LANG_ADDITEM, itemId, count);
    uint16 dest;
    uint8 msg = plTarget->CanStoreNewItem( 0, NULL_SLOT, dest, itemId, count, false );
    if( msg == EQUIP_ERR_OK )
    {
        plTarget->StoreNewItem( dest, itemId, count, true);

        // remove binding (let GM give it to another player later)
        if(pl==plTarget)
        {
            Item* item = pl->GetItemByPos(dest);
            if(item)
                item->SetBinding( false );
        }

        PSendSysMessage(LANG_ITEM_CREATED, itemId, count);
    }
    else
    {
        pl->SendEquipError( msg, NULL, NULL );
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, count);
    }

    return true;
}

bool ChatHandler::HandleAddItemSetCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_ADDITEMSET_SYNTAX);
        return true;
    }

    char* citemsetId = strtok((char*)args, " ");
    uint32 itemsetId = atol(citemsetId);

    // prevent generation all items with itemset field value '0'
    if (itemsetId == 0)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);
        return true;
    }

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if(!plTarget)
        plTarget = pl;

    sLog.outDetail(LANG_ADDITEMSET, itemsetId);

    QueryResult *result = sDatabase.PQuery("SELECT `entry` FROM `item_template` WHERE `itemset` = %u",itemsetId);

    if(!result)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);

        return true;
    }

    do
    {
        WorldPacket data;

        Field *fields = result->Fetch();
        uint32 itemId = fields[0].GetUInt32();

        uint16 dest;

        uint8 msg = plTarget->CanStoreNewItem( 0, NULL_SLOT, dest, itemId, 1, false );
        if( msg == EQUIP_ERR_OK )
        {
            plTarget->StoreNewItem( dest, itemId, 1, true);

            // remove binding (let GM give it to another player later)
            if(pl==plTarget)
            {
                Item* item = pl->GetItemByPos(dest);
                if(item)
                    item->SetBinding( false );
            }

            PSendSysMessage(LANG_ITEM_CREATED, itemId, 1);
        }
        else
        {
            pl->SendEquipError( msg, NULL, NULL );
            PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, 1);
        }

    }while( result->NextRow() );

    delete result;

    return true;
}

bool ChatHandler::HandleCreateGuildCommand(const char* args)
{
    Guild *guild;
    Player * player;
    char *lname,*gname;
    std::string guildname;

    if (!*args)
        return false;

    gname = strtok((char*)args, " ");
    lname = strtok(NULL, " ");
    if(!lname)
        return false;
    else if(!gname)
    {
        SendSysMessage(LANG_INSERT_GUILD_NAME);
        return true;
    }

    guildname = gname;
    player = ObjectAccessor::Instance().FindPlayerByName(lname);

    if(!player)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return true;
    }

    if(!player->GetGuildId())
    {
        guild = new Guild;
        guild->create(player->GetGUID(),guildname);
        objmgr.AddGuild(guild);
    }
    else
        SendSysMessage(LANG_PLAYER_IN_GUILD);

    return true;
}

//float max_creature_distance = 160;

bool ChatHandler::HandleGetDistanceCommand(const char* args)
{
    Unit* pUnit = getSelectedUnit();

    if(!pUnit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    PSendSysMessage(LANG_DISTANCE, m_session->GetPlayer()->GetDistanceSq(pUnit));

    return true;
}

bool ChatHandler::HandleObjectCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 display_id = atoi((char*)args);

    char* safe = strtok((char*)args, " ");

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    GameObject* pGameObj = new GameObject();
    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id, chr->GetMapId(), x, y, z, o, 0, 0, 0, 0))
    {
        delete pGameObj;
        return false;
    }
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 19);
    sLog.outDebug(LANG_ADD_OBJ_LV3);
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

    if(strcmp(safe,"true") == 0)
        pGameObj->SaveToDB();

    return true;
}

// FIX-ME!!!

bool ChatHandler::HandleAddWeaponCommand(const char* args)
{
    /*if (!*args)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SendSysMessage(LANG_NO_SELECTION);
        return true;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    char* pSlotID = strtok((char*)args, " ");
    if (!pSlotID)
        return false;

    char* pItemID = strtok(NULL, " ");
    if (!pItemID)
        return false;

    uint32 ItemID = atoi(pItemID);
    uint32 SlotID = atoi(pSlotID);

    ItemPrototype* tmpItem = objmgr.GetItemPrototype(ItemID);

    bool added = false;
    if(tmpItem)
    {
        switch(SlotID)
        {
            case 1:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
                added = true;
                break;
            case 2:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
                added = true;
                break;
            case 3:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
                added = true;
                break;
            default:
                PSendSysMessage(LANG_ITEM_SLOT_NOT_EXIST,SlotID);
                added = false;
                break;
        }
        if(added)
        {
            PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,ItemID,tmpItem->Name1,SlotID);
        }
    }
    else
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND,ItemID);
        return true;
    }
    */
    return true;
}

bool ChatHandler::HandleGameObjectCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;

    uint32 id = atoi((char*)args);
    if(!id)
    {
        SendSysMessage(LANG_GAMEOBJECT_SYNTAX);
        return false;
    }

    const GameObjectInfo *goI = objmgr.GetGameObjectInfo(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        return false;
    }

    Player *chr = m_session->GetPlayer();
    float x = float(chr->GetPositionX());
    float y = float(chr->GetPositionY());
    float z = float(chr->GetPositionZ());
    float o = float(chr->GetOrientation());

    GameObject* pGameObj = new GameObject();
    uint32 lowGUID = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);

    if(!pGameObj->Create(lowGUID, goI->id, chr->GetMapId(), x, y, z, o, 0, 0, 0, 0))
    {
        delete pGameObj;
        return false;
    }
    //pGameObj->SetZoneId(chr->GetZoneId());
    pGameObj->SetMapId(chr->GetMapId());
    //pGameObj->SetNameId(id);
    sLog.outError(LANG_GAMEOBJECT_CURRENT, goI->name, lowGUID, x, y, z, o);

    pGameObj->SaveToDB();
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

    PSendSysMessage(LANG_GAMEOBJECT_ADD,id,goI->name,x,y,z);

    return true;
}

bool ChatHandler::HandleAnimCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 anim_id = atoi((char*)args);

    data.Initialize( SMSG_EMOTE );
    data << anim_id << m_session->GetPlayer( )->GetGUID();
    WPAssert(data.size() == 12);
    MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId())->MessageBoardcast(m_session->GetPlayer(), &data, true);
    return true;
}

bool ChatHandler::HandleStandStateCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 anim_id = atoi((char*)args);
    m_session->GetPlayer( )->SetUInt32Value( UNIT_NPC_EMOTESTATE , anim_id );

    return true;
}

bool ChatHandler::HandleDieCommand(const char* args)
{
    Unit* target = getSelectedUnit();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    if( target->isAlive() )
    {
        uint32 health = target->GetHealth();
        m_session->GetPlayer()->DealDamage(target, target->GetHealth(), 0, false);
    }

    return true;
}

bool ChatHandler::HandleReviveCommand(const char* args)
{
    Player* SelectedPlayer = getSelectedPlayer();
    if(!SelectedPlayer)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    SelectedPlayer->ResurrectPlayer();
    SelectedPlayer->SetHealth( SelectedPlayer->GetMaxHealth()/2 );
    SelectedPlayer->SpawnCorpseBones();
    return true;
}

bool ChatHandler::HandleMorphCommand(const char* args)
{
    if (!*args)
        return false;

    uint16 display_id = (uint16)atoi((char*)args);

    m_session->GetPlayer()->SetUInt32Value(UNIT_FIELD_DISPLAYID, display_id);

    return true;
}

bool ChatHandler::HandleAuraCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 spellID = (uint32)atoi(px);
    SpellEntry *spellInfo = sSpellStore.LookupEntry( spellID );
    if(spellInfo)
    {
        for(uint32 i = 0;i<3;i++)
        {
            uint8 eff = spellInfo->Effect[i];
            if (eff>=TOTAL_SPELL_EFFECTS)
                continue;
            if (eff == 6)
            {
                Aura *Aur = new Aura(spellInfo, i, m_session->GetPlayer());
                m_session->GetPlayer()->AddAura(Aur);
            }
        }
    }

    return true;
}

bool ChatHandler::HandleUnAuraCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 spellID = (uint32)atoi(px);
    m_session->GetPlayer()->RemoveAurasDueToSpell(spellID);

    return true;
}

// Get graveyard 'id' near (50f) from current player position, true if found
bool ChatHandler::GetCurrentGraveId(uint32& id )
{
    Player* player = m_session->GetPlayer();

    QueryResult *result = sDatabase.PQuery(
        "SELECT `id` FROM `game_graveyard` "
        "WHERE  `map` = %u AND (POW('%f'-`position_x`,2)+POW('%f'-`position_y`,2)+POW('%f'-`position_z`,2)) < 50*50",
        player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

    if(result)
    {
        Field *fields = result->Fetch();
        id = fields[0].GetUInt32();

        delete result;

        return true;
    }
    else
        return false;
}

// Link graveyard 'g_id' with current zone
void ChatHandler::LinkGraveIfNeed(uint32 g_id)
{
    Player* player = m_session->GetPlayer();

    QueryResult *result = sDatabase.PQuery(
        "SELECT `id` FROM `game_graveyard_zone` WHERE `id` = %u AND `ghost_map` = %u AND `ghost_zone` = '%u'",
        g_id,player->GetMapId(),player->GetZoneId());

    if(result)
    {
        delete result;

        PSendSysMessage("Graveyard #%u already linked to zone #%u (current).", g_id,player->GetZoneId());
    }
    else
    {
        sDatabase.PExecute("INSERT INTO `game_graveyard_zone` ( `id`,`ghost_map`,`ghost_zone`) VALUES ('%u', '%u', '%u')",
            g_id,player->GetMapId(),player->GetZoneId());

        PSendSysMessage("Graveyard #%u linked to zone #%u (current).", g_id,player->GetZoneId());
    }
}

bool ChatHandler::HandleAddGraveCommand(const char* args)
{

    if(!*args)
        return false;

    uint32 g_team;
    uint32 g_id;

    if (strncmp((char*)args,"any",4)==0)
        g_team = 0;
    else if (strncmp((char*)args,"horde",6)==0)
        g_team = HORDE;
    else if (strncmp((char*)args,"alliance",9)==0)
        g_team = ALLIANCE;
    else
        return false;

    WorldPacket data;

    Player* player = m_session->GetPlayer();

    // Is this update request?
    if(GetCurrentGraveId(g_id))
    {
        sDatabase.PExecute(
            "UPDATE `game_graveyard` "
            "SET `position_x` = '%f',`position_y` = '%f',`position_z` = '%f',`orientation` = '%f', `map` = %u, `faction` = %u "
            "WHERE `id` = '%u'" ,
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), player->GetMapId() , g_team, g_id );

        PSendSysMessage("Graveyard #%u updated (coordinates, orientation and faction).", g_id);
    }
    else
    {
        // new graveyard
        sDatabase.PExecute("INSERT INTO `game_graveyard` ( `position_x`,`position_y`,`position_z`,`orientation`,`map`,`faction`) "
            "VALUES ('%f', '%f', '%f', '%f', '%u', '%u')",
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), player->GetMapId(), g_team );

        if(GetCurrentGraveId(g_id))
            PSendSysMessage("Graveyard #%u added (coordinates, orientation and faction).", g_id);
        else
        {
            SendSysMessage("Graveyard NOT added. Unknown error.");
            return true;
        }
    }

    LinkGraveIfNeed(g_id);

    return true;
}

bool ChatHandler::HandleLinkGraveCommand(const char* args)
{

    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 g_id = (uint32)atoi(px);

    WorldPacket data;

    QueryResult *result = sDatabase.PQuery("SELECT `id` FROM `game_graveyard` WHERE `id` = '%u'",g_id);

    if(!result)
    {
        PSendSysMessage("Graveyard #%u not exist.", g_id);
        return true;
    }
    else
        delete result;

    LinkGraveIfNeed(g_id);

    return true;
}

bool ChatHandler::HandleNearGraveCommand(const char* args)
{
    uint32 g_team;

    QueryResult *result;

    if(!*args)
        g_team = ~uint32(0);
    else if (strncmp((char*)args,"any",4)==0)
        g_team = 0;
    else if (strncmp((char*)args,"horde",6)==0)
        g_team = HORDE;
    else if (strncmp((char*)args,"alliance",9)==0)
        g_team = ALLIANCE;
    else
        return false;

    Player* player = m_session->GetPlayer();

    WorldPacket data;

    if(g_team == ~uint32(0))
    {
        result = sDatabase.PQuery(
            "SELECT (POW('%f'-`position_x`,2)+POW('%f'-`position_y`,2)+POW('%f'-`position_z`,2)) AS `distance`,`game_graveyard`.`id`,`faction` "
            "FROM `game_graveyard`, `game_graveyard_zone` "
            "WHERE  `game_graveyard`.`id` = `game_graveyard_zone`.`id` AND `ghost_map` = %u AND `ghost_zone` = %u "
            "ORDER BY `distance` ASC LIMIT 1",
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetZoneId());
    }
    else
    {
        result = sDatabase.PQuery(
            "SELECT (POW('%f'-`position_x`,2)+POW('%f'-`position_y`,2)+POW('%f'-`position_z`,2)) AS `distance`,`game_graveyard`.`id`,`faction` "
            "FROM `game_graveyard`, `game_graveyard_zone` "
            "WHERE  `game_graveyard`.`id` = `game_graveyard_zone`.`id` AND `ghost_map` = %u AND `ghost_zone` = %u "
            "        AND (`faction` = %u OR `faction` = 0 ) "
            "ORDER BY `distance` ASC LIMIT 1",
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetZoneId(),g_team);
    }

    if(result)
    {
        Field *fields = result->Fetch();
        uint32 g_id = fields[1].GetUInt32();
        uint32 g_team = fields[2].GetUInt32();

        std::string team_name = "invalid team, please fix DB";

        if(g_team == 0)
            team_name = "any";
        else if(g_team == HORDE)
            team_name = "horde";
        else if(g_team == ALLIANCE)
            team_name = "alliance";

        delete result;

        PSendSysMessage("Graveyard #%u (faction: %s) is nearest from linked to zone #%u.", g_id,team_name.c_str(),player->GetZoneId());
    }
    else
    {

        std::string team_name;

        if(g_team == 0)
            team_name = "any";
        else if(g_team == HORDE)
            team_name = "horde";
        else if(g_team == ALLIANCE)
            team_name = "alliance";

        if(g_team == ~uint32(0))
            PSendSysMessage("Zone #%u not have linked graveyards.", player->GetZoneId());
        else
            PSendSysMessage("Zone #%u not have linked graveyards for faction: %s.", player->GetZoneId(),team_name.c_str());
    }

    return true;
}

bool ChatHandler::HandleAddSHCommand(const char *args)
{
    WorldPacket data;

    /* this code is wrong, SH is just an NPC
    and should be spawned normally as any NPC
    (c) Phantomas
    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    Creature* pCreature = new Creature();

    if(!pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), "Spirit Healer", chr->GetMapId(), x, y, z, o, objmgr.AddCreatureTemplate(pCreature->GetName(), 5233)))
    {
        delete pCreature;
        return false;
    }
    pCreature->SetZoneId(chr->GetZoneId());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureTemplate(pCreature->GetName(), 5233));
    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 5233);
    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, 33);
    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , 35);
    pCreature->SetHealth(100);
    pCreature->SetMaxHealth(100);
    pCreature->SetLevel(60);
    pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, 0x0300);
    pCreature->SetUInt32Value(UNIT_FIELD_AURA+0, 10848);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS+0, 0xEEEEEE3C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+0, 0xEEEEEE00);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS+0, 0x00000009);
    pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH , 1.5f);
    pCreature->SetFloatValue(UNIT_FIELD_MAXDAMAGE ,  5.0f);
    pCreature->SetFloatValue(UNIT_FIELD_MINDAMAGE , 8.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 1900);
    pCreature->SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, 2000);
    pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2.0f);

    sLog.outError("AddObject at Level3.cpp line 1470");
    pCreature->AIM_Initialize();
    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);

    pCreature->SaveToDB();

    std::stringstream ss,ss2,ss3;
    QueryResult *result;

    result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_gossip`" );
    if( result )
    {
        sDatabase.PExecute("INSERT INTO `npc_gossip` (`id`,`npc_guid`,`gossip_type`,`textid`,`option_count`) VALUES ('%u', '%u', '%u', '%u', '%u')", (*result)[0].GetUInt32()+1, pCreature->GetGUIDLow(), 1, 1, 1);
        delete result;
        result = NULL;

        result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_option`" );
        if( result )
        {
            sDatabase.PExecute("INSERT INTO `npc_option` (`id`,`gossip_id`,`type`,`option`,`npc_text_nextid`,`special`) VALUES ('%u', '%u', '%u', '%s', '%u', '%u')", (*result)[0].GetUInt32()+1, (*result)[0].GetUInt32()+2, 0, "Return me to life.", 0, 2);
            delete result;
            result = NULL;
        }
        result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_text`" );
        if( result )
        {
            sDatabase.PExecute("INSERT INTO `npc_text` (`id`,`text0_0`) VALUES ('%u', '%s')", (*result)[0].GetUInt32()+1, "It is not yet your time. I shall aid your journey back to the realm of the living... For a price.");

            delete result;
            result = NULL;
        }
    }
    */
    return true;
}

bool ChatHandler::HandleSpawnTransportCommand(const char* args)
{

    return true;
}

bool ChatHandler::HandleEmoteCommand(const char* args)
{
    uint32 emote = atoi((char*)args);

    Unit* target = getSelectedCreature();
    if(!target)
    {
        PSendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);

    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char* args)
{
    uint32 faction = 0, npcflags = 0, skinid = 0, Entry = 0;

    Unit* target = getSelectedCreature();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    faction = target->getFaction();
    npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    skinid = target->GetUInt32Value(UNIT_FIELD_DISPLAYID);
    Entry = target->GetUInt32Value(OBJECT_FIELD_ENTRY);

    PSendSysMessage(LANG_NPCINFO_CHAR,  target->GetGUIDLow(), faction, npcflags, Entry, skinid);
    PSendSysMessage(LANG_NPCINFO_LEVEL, target->getLevel());
    PSendSysMessage(LANG_NPCINFO_HEALTH,target->GetUInt32Value(UNIT_FIELD_BASE_HEALTH), target->GetMaxHealth(), target->GetHealth());
    PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->getFaction());

    PSendSysMessage(LANG_NPCINFO_POSITION,float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));

    if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
    {
        SendSysMessage(LANG_NPCINFO_VENDOR);
    }
    if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
    {
        SendSysMessage(LANG_NPCINFO_TRAINER);
    }

    return true;
}

bool ChatHandler::HandleNpcInfoSetCommand(const char* args)
{
    uint32 entry = 0, testvalue = 0;

    Unit* target = getSelectedCreature();
    if(!target)
    {
        PSendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(!args)
        return true;

    //m_session->GetPlayer( )->SetUInt32Value(PLAYER_FLAGS, (uint32)8);

    testvalue = uint32(atoi((char*)args));

    entry = target->GetUInt32Value( OBJECT_FIELD_ENTRY );

    m_session->SendTestCreatureQueryOpcode( entry, target->GetGUID(), testvalue );

    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    if (flag != 0)
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName());
    }
    else
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName());
    }

    WorldPacket data;
    char buf[256];

    if (flag != 0)
    {
        sprintf((char*)buf,LANG_YOURS_EXPLORE_SET_ALL,
            m_session->GetPlayer()->GetName());
    }
    else
    {
        sprintf((char*)buf,LANG_YOURS_EXPLORE_SET_NOTHING,
            m_session->GetPlayer()->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    chr->GetSession()->SendPacket(&data);

    for (uint8 i=0; i<64; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0);
        }
    }

    return true;
}

bool ChatHandler::HandleHoverCommand(const char* args)
{
    WorldPacket data;

    char* px = strtok((char*)args, " ");
    uint32 flag;
    if (!px)
        flag = 1;
    else
        flag = atoi(px);

    //SMSG_MOVE_UNSET_HOVER

    if (flag)
    {
        data.Initialize(SMSG_MOVE_SET_HOVER);
        data << (uint8)0xFF <<m_session->GetPlayer()->GetGUID();
    }
    else
    {
        data.Initialize(SMSG_MOVE_UNSET_HOVER);
        data << (uint8)0xFF <<m_session->GetPlayer()->GetGUID();
    }

    m_session->SendPacket( &data );

    if (flag)
        SendSysMessage(LANG_HOVER_ENABLED);
    else
        PSendSysMessage(LANG_HOVER_DISABLED);

    return true;
}

bool ChatHandler::HandleLevelUpCommand(const char* args)
{
    int nrlvl = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    for(int i=0;i<nrlvl || i==0;i++)
    {
        chr->GiveLevel();

        WorldPacket data;
        FillSystemMessageData(&data, chr->GetSession(), LANG_YOURS_LEVEL_UP );
        chr->GetSession()->SendPacket( &data );
    }
    chr->SetUInt32Value(PLAYER_XP,0);

    return true;
}

bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    SendSysMessage(LANG_EXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleHideAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    SendSysMessage(LANG_UNEXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleUpdate(const char* args)
{
    uint32 updateIndex;
    uint32 value;

    char* pUpdateIndex = strtok((char*)args, " ");

    Unit* chr = getSelectedUnit();
    if (chr == NULL)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    if(!pUpdateIndex)
    {
        return true;
    }
    updateIndex = atoi(pUpdateIndex);
    //check updateIndex
    if(chr->GetTypeId() == TYPEID_PLAYER)
    {
        if (updateIndex>=PLAYER_END) return true;
    }
    else
    {
        if (updateIndex>=UNIT_END) return true;
    }

    char*  pvalue = strtok(NULL, " ");
    if (!pvalue)
    {
        value=chr->GetUInt32Value(updateIndex);

        PSendSysMessage(LANG_UPDATE, chr->GetGUIDLow(),updateIndex,value);
        return true;
    }

    value=atoi(pvalue);

    PSendSysMessage(LANG_UPDATE_CHANGE, chr->GetGUIDLow(),updateIndex,value);

    chr->SetUInt32Value(updateIndex,value);

    return true;
}

bool ChatHandler::HandleBankCommand(const char* args)
{
    WorldPacket data;
    uint64 guid;

    guid = m_session->GetPlayer()->GetGUID();

    data.Initialize( SMSG_SHOW_BANK );
    data << guid;
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleChangeWeather(const char* args)
{
    //*Change the weather of a cell
    WorldPacket data;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 type = (uint32)atoi(px);                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
    float grade = (float)atof(py);                          //0 to 1, sending -1 is instand good weather

    Player *player = m_session->GetPlayer();
    uint32 zoneid = player->GetZoneId();
    Weather *wth = sWorld.FindWeather(zoneid);
    if(!wth)
    {
        Weather *wth = new Weather(player);
        sWorld.AddWeather(wth);
    }

    wth->SetWeather(type, grade);

    return true;
}

bool ChatHandler::HandleSetValue(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py)
        return false;

    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    uint64 guid = target->GetGUID();

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if(pz)
        isint32 = (bool)atoi(pz);
    if(isint32)
    {
        iValue = (uint32)atoi(py);
        sLog.outDebug( LANG_SET_UINT, GUID_LOPART(guid), Opcode, iValue);
        target->SetUInt32Value( Opcode , iValue );
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), Opcode,iValue);
    }
    else
    {
        fValue = (float)atof(py);
        sLog.outDebug( LANG_SET_FLOAT, GUID_LOPART(guid), Opcode, fValue);
        target->SetFloatValue( Opcode , fValue );
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), Opcode,fValue);
    }

    return true;
}

bool ChatHandler::HandleGetValue(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
        return false;

    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    uint64 guid = target->GetGUID();

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if(pz)
        isint32 = (bool)atoi(pz);

    if(isint32)
    {
        iValue = target->GetUInt32Value( Opcode );
        sLog.outDebug( LANG_GET_UINT, GUID_LOPART(guid), Opcode, iValue);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), Opcode,    iValue);
    }
    else
    {
        fValue = target->GetFloatValue( Opcode );
        sLog.outDebug( LANG_GET_FLOAT, GUID_LOPART(guid), Opcode, fValue);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), Opcode, fValue);
    }

    return true;
}

bool ChatHandler::HandleSet32Bit(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    uint32 Value = (uint32)atoi(py);
    if (Value > 32)                                         //uint32 = 32 bits
        return false;

    sLog.outDebug( LANG_SET_32BIT , Opcode, Value);

    m_session->GetPlayer( )->SetUInt32Value( Opcode , 2^Value );

    PSendSysMessage(LANG_SET_32BIT_FIELD, Opcode,1);
    return true;
}

bool ChatHandler::HandleMod32Value(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    int Value = atoi(py);

    sLog.outDebug( LANG_CHANGE_32BIT , Opcode, Value);

    int CurrentValue = (int)m_session->GetPlayer( )->GetUInt32Value( Opcode );

    CurrentValue += Value;
    m_session->GetPlayer( )->SetUInt32Value( Opcode , (uint32)CurrentValue );

    PSendSysMessage(LANG_CHANGE_32BIT_FIELD, Opcode,CurrentValue);

    return true;
}

bool ChatHandler::HandleSendMailNotice(const char* args)
{
    WorldPacket data;

    char* px = strtok((char*)args, " ");
    uint32 flag;
    if (!px)
        flag = 0;
    else
        flag = atoi(px);

    data.Initialize(SMSG_RECEIVED_MAIL);

    data << uint32(flag);
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleQueryNextMailTime(const char* args)
{
    WorldPacket Data;

    char* px = strtok((char*)args, " ");
    uint32 flag;
    if (!px)
        flag = 0;
    else
        flag = atoi(px);

    Data.Initialize(MSG_QUERY_NEXT_MAIL_TIME);
    Data << uint32(flag);
    m_session->SendPacket(&Data);
    return true;
}

bool ChatHandler::HandleAddTeleCommand(const char * args)
{
    if(!*args)
        return false;
    QueryResult *result;
    Player *player=m_session->GetPlayer();
    if (!player) return false;
    char *name=(char*)args;

    result = sDatabase.PQuery("SELECT `id` FROM `game_tele` WHERE `name` = '%s'",name);
    if (result)
    {
        SendSysMessage("Teleport location already exists!");
        delete result;
        return true;
    }

    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    float ort = player->GetOrientation();
    int mapid = player->GetMapId();

    if(sDatabase.PExecute("INSERT INTO `game_tele` (`position_x`,`position_y`,`position_z`,`orientation`,`map`,`name`) VALUES (%f,%f,%f,%f,%d,'%s')",x,y,z,ort,mapid,name))
    {
        SendSysMessage("Teleport location added.");
    }
    return true;
}

bool ChatHandler::HandleDelTeleCommand(const char * args)
{
    if(!*args)
        return false;

    char *name=(char*)args;
    QueryResult *result=sDatabase.PQuery("SELECT `id` FROM `game_tele` WHERE `name` = '%s'",name);
    if (!result)
    {
        SendSysMessage("Teleport location not found!");
        return true;
    }
    delete result;

    if(sDatabase.PExecute("DELETE FROM `game_tele` WHERE `name` = '%s'",name))
    {
        SendSysMessage("Teleport location deleted.");
    }
    return true;
}

bool ChatHandler::HandleResetCommand (const char * args)
{
    if(!*args)
        return false;
    Player* player = getSelectedPlayer();
    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "stats")
    {
        PlayerCreateInfo *info = objmgr.GetPlayerCreateInfo((uint32)player->getRace(), (uint32)player->getClass());
        if(!info) return false;

        for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            if(player->GetItemByPos(INVENTORY_SLOT_BAG_0,i) != NULL)
            {
                SendSysMessage("The player must unequip all items before resetting stats!");
                return true;
            }
        }

        Player::AuraMap p_Auras = player->GetAuras();
        for(Player::AuraMap::iterator itr = p_Auras.begin(); itr != p_Auras.end(); ++itr)
        {
            if (itr->second && !itr->second->IsPassive())
            {
                SendSysMessage("The player must debuff all non passive auras before resetting stats!");
                return true;
            }
        }

        uint8 powertype = 0;
        uint32 unitfield = 0;
        switch(player->getClass())
        {
            case WARRIOR: powertype = 1; unitfield = 0x1100EE00; break;
            case PALADIN: powertype = 0; unitfield = 0x0000EE00; break;
            case HUNTER: powertype = 0; unitfield = 0x0000EE00; break;
            case ROGUE: powertype = 3; unitfield = 0x00000000; break;
            case PRIEST: powertype = 0; unitfield = 0x0000EE00; break;
            case SHAMAN: powertype = 0; unitfield = 0x0000EE00; break;
            case MAGE: powertype = 0; unitfield = 0x0000EE00; break;
            case WARLOCK: powertype = 0; unitfield = 0x0000EE00; break;
            case DRUID: powertype = 0; unitfield = 0x0000EE00; break;
        }

        if ( player->getRace() == TAUREN )
        {
            player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.35f);
        }
        else player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
        player->SetStat(STAT_STRENGTH,info->strength );
        player->SetStat(STAT_AGILITY,info->agility );
        player->SetStat(STAT_STAMINA,info->stamina );
        player->SetStat(STAT_INTELLECT,info->intellect );
        player->SetStat(STAT_SPIRIT,info->spirit );
        player->SetArmor(info->basearmor );
        player->SetUInt32Value(UNIT_FIELD_ATTACK_POWER, info->attackpower );

        player->SetHealth(info->health);
        player->SetMaxHealth(info->health);

        player->SetPower(   POWER_MANA, info->mana );
        player->SetMaxPower(POWER_MANA, info->mana );
        player->SetPower(   POWER_RAGE, 0 );
        player->SetMaxPower(POWER_RAGE, info->rage );
        player->SetPower(   POWER_FOCUS, info->focus );
        player->SetMaxPower(POWER_FOCUS, info->focus );
        player->SetPower(   POWER_ENERGY, info->energy );
        player->SetMaxPower(POWER_ENERGY, info->energy );

        player->SetFloatValue(UNIT_FIELD_MINDAMAGE, info->mindmg );
        player->SetFloatValue(UNIT_FIELD_MAXDAMAGE, info->maxdmg );
        player->SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, info->ranmindmg );
        player->SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, info->ranmaxdmg );

        player->SetAttackTime(BASE_ATTACK,   2000 );        // melee attack time
        player->SetAttackTime(RANGED_ATTACK, 2000 );        // ranged attack time

        player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.388999998569489f );
        player->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f   );

        player->SetUInt32Value(UNIT_FIELD_DISPLAYID, info->displayId + player->getGender());
        player->SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, info->displayId + player->getGender() );

        player->SetLevel( 1 );

        player->setFactionForRace(player->getRace());

        player->SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( player->getRace() ) | ( player->getClass() << 8 ) | ( player->getGender() << 16 ) | ( powertype << 24 ) ) );
        player->SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield );
        player->SetUInt32Value(UNIT_FIELD_BYTES_2, 0xEEEEEE00 );
        player->SetUInt32Value(UNIT_FIELD_FLAGS , UNIT_FLAG_NONE | UNIT_FLAG_NOT_IN_PVP );

        player->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0x10);
                                                            //-1 is default value
        player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

        player->SetUInt32Value(PLAYER_NEXT_LEVEL_XP, 400);
        player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );

        player->SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, 1.00);
        player->SetLevel(1);
        player->SetUInt32Value(PLAYER_FIELD_POSSTAT0, 0);
        player->SetUInt32Value(PLAYER_FIELD_POSSTAT1, 0);
        player->SetUInt32Value(PLAYER_FIELD_POSSTAT2, 0);
        player->SetUInt32Value(PLAYER_FIELD_POSSTAT3, 0);
        player->SetUInt32Value(PLAYER_FIELD_POSSTAT4, 0);

        //+5% HP if has skill Endurance
        if (player->HasSpell(SPELL_PASSIVE_ENDURENCE))
        {
                                                            // only integer part
            player->SetMaxHealth( uint32(player->GetMaxHealth() * 1.05));
        }

        // school resistances
        if (player->HasSpell(SPELL_PASSIVE_FROST_RESISTANCE))
        {
            player->SetResistance(SPELL_SCHOOL_FROST, 10 );
        }
        if (player->HasSpell(SPELL_PASSIVE_NATURE_RESISTANCE))
        {
            player->SetResistance(SPELL_SCHOOL_NATURE, 10 );
        }
        if (player->HasSpell(SPELL_PASSIVE_SHADOW_RESISTANCE))
        {
            player->SetResistance(SPELL_SCHOOL_SHADOW, 10 );
        }
        if (player->HasSpell(SPELL_PASSIVE_ARCANE_RESISTANCE))
        {
            player->SetResistance(SPELL_SCHOOL_ARCANE, 10 );
        }

        // reinitilize potential block chance (used if item with Block value equiped)
        player->SetFloatValue(PLAYER_BLOCK_PERCENTAGE, 5 + (float(player->GetDefenceSkillValue()) - player->getLevel()*5)*0.04);
        return true;
    }
    if (argstr == "talents")
    {
        for (int i = 0; i < sTalentStore.GetNumRows(); i++)
        {
            TalentEntry *talentInfo = sTalentStore.LookupEntry(i);
            if (!talentInfo) continue;
            for (int j = 0; j < 5; j++)
            {
                SpellEntry *spellInfo = sSpellStore.LookupEntry(talentInfo->RankID[j]);
                if (!spellInfo) continue;
                PlayerSpellList s_list = player->getSpellList();
                for(PlayerSpellList::iterator itr = s_list.begin(); itr != s_list.end(); ++itr)
                {
                    if ((*itr)->spellId == spellInfo->Id)
                    {
                        WorldPacket data;
                        data.Initialize(SMSG_REMOVED_SPELL);
                        data << (*itr)->spellId;
                        player->GetSession()->SendPacket(&data);
                        player->removeSpell((*itr)->spellId);
                        player->RemoveAurasDueToSpell((*itr)->spellId);
                        break;
                    }
                }
            }
        }
        uint32 tp = player->getLevel() < 10 ? 0 : player->getLevel() - 9;
        player->SetUInt32Value(PLAYER_CHARACTER_POINTS1, tp);
        return true;
    }
    return false;
}

bool ChatHandler::HandleMaxSkillCommand(const char* args)
{
    Player* SelectedPlayer = getSelectedPlayer();
    if(!SelectedPlayer)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    // each skills that have max skill value dependent from level seted to current level max skill value
    SelectedPlayer->UpdateSkillsToMaxSkillsForLevel();
    return true;
}

bool ChatHandler::ShutDown (const char* args)
{
    if(!*args)
    {
        SendSysMessage(LANG_SHUTTDOWN);
        return false;
    }

    uint32 time = atoi(args);
    sWorld.ShutdownServ(time);
    return true;
}

bool ChatHandler::CancelShutdown (const char* args)
{
    sWorld.ShutdownServ(0);
    return true;
}

// TODO Add a commando "Illegal name" to set playerflag |= 32;
// maybe do'able with a playerclass m_Illegal_name = false
