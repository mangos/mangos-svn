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
    WorldPacket data;

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* pgm = strtok(NULL, " ");
    if (!pgm)
        return false;

    int8 gm = (uint8) atoi(pgm);
    if ( gm < 0 || gm > 5)
    {
        FillSystemMessageData(&data, m_session, LANG_BAD_VALUE);
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    Player* chr = ObjectAccessor::Instance().FindPlayerByName(args);

    if (chr)
    {

        sprintf((char*)buf,LANG_YOU_CHANGE_SECURITY, chr->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        sprintf((char*)buf,LANG_YOURS_SECURITY_CHANGED, m_session->GetPlayer()->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);

        chr->GetSession()->SendPacket(&data);
        chr->GetSession()->SetSecurity(gm);

        sDatabase.PExecute("UPDATE `account` SET `gmlevel` = '%i' WHERE `id` = '%u';", gm, chr->GetSession()->GetAccountId());

    }
    else
    {
        sprintf((char*)buf,LANG_NO_PLAYER, pName);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
    }

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

    m_session->GetPlayer()->SendNewWorld(atoi(pContinent), (float)atof(px), (float)atof(py), (float)atof(pz),0.0f);

    return true;
}

bool ChatHandler::HandleAllowMovementCommand(const char* args)
{
    WorldPacket data;
    if(sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        FillSystemMessageData(&data, m_session, LANG_CREATURE_MOVE_DISABLED);
    }
    else
    {

        sWorld.SetAllowMovement(true);
        FillSystemMessageData(&data, m_session, LANG_CREATURE_MOVE_ENABLED);
    }

    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleAddSpiritCommand(const char* args)
{
    sLog.outDetail(LANG_SPAWNING_SPIRIT_HEAL);
    /*
        Creature* pCreature;
        UpdateMask unitMask;
        WorldPacket data;

        QueryResult *result = sDatabase.PQuery("SELECT `position_x`,`position_y`,`position_z`,`F`,`name_id`,`map`,`zone`,`faction` FROM `npc_spirithealer`;");

        if(!result)
        {
            FillSystemMessageData(&data, m_session, LANG_NO_SPIRIT_HEAL_DB);
            m_session->SendPacket( &data );

            return true;
        }

        uint32 name;

        do
        {
            Field* fields = result->Fetch();

            name = fields[4].GetUInt32();
            sLog.outDetail("%s name is %d\n", fields[4].GetString(), name);

            pCreature = new Creature();

            pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), objmgr.GetCreatureTemplate(name)->Name, fields[5].GetUInt16(),
                fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), name);

            pCreature->SetZoneId( fields[6].GetUInt16() );
            pCreature->SetUInt32Value( OBJECT_FIELD_ENTRY, name );
            pCreature->SetFloatValue( OBJECT_FIELD_SCALE_X, 1.0f );
            pCreature->SetUInt32Value( UNIT_FIELD_DISPLAYID, 5233 );
            pCreature->SetUInt32Value( UNIT_NPC_FLAGS , 1 );
            pCreature->SetUInt32Value( UNIT_FIELD_FACTIONTEMPLATE , fields[7].GetUInt32() );
            pCreature->SetUInt32Value( UNIT_FIELD_HEALTH, 100 + 30*(60) );
            pCreature->SetUInt32Value( UNIT_FIELD_MAXHEALTH, 100 + 30*(60) );
            pCreature->SetUInt32Value( UNIT_FIELD_LEVEL , 60 );
            pCreature->SetFloatValue( UNIT_FIELD_COMBATREACH , 1.5f );
            pCreature->SetFloatValue( UNIT_FIELD_MAXDAMAGE ,  5.0f );
            pCreature->SetFloatValue( UNIT_FIELD_MINDAMAGE , 8.0f );
            pCreature->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME, 1900 );
            pCreature->SetUInt32Value( UNIT_FIELD_BASEATTACKTIME+1, 2000 );
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

    m_session->GetPlayer()->SendNewWorld(mapid, x, y, z,0.0f);

    return true;
}

bool ChatHandler::HandleLearnSkillCommand (const char* args)
{
    WorldPacket data;
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
        FillSystemMessageData(&data, m_session, LANG_LEARNSK_SYNTAX);
        m_session->SendPacket(&data);
        return true;
    }

    Player * player = m_session->GetPlayer();
    Player * target = objmgr.GetPlayer(player->GetSelection());

    if (!target) target = player;

    if (skill > 0)
    {
        target->SetSkill(skill, level, max);
        FillSystemMessageData(&data, m_session, fmtstring(LANG_LEARNED_SKILL, skill));
        m_session->SendPacket(&data);
    } else
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_INVALID_SKILL_ID, skill));
        m_session->SendPacket(&data);
    }
    return true;
}

bool ChatHandler::HandleUnLearnSkillCommand (const char* args)
{
    WorldPacket data;

    if (!*args)
    {
        FillSystemMessageData(&data, m_session, LANG_UNLEARNSK_SYNTAX);
        m_session->SendPacket(&data);
        return true;
    }

    uint32 skill = 0;
    uint16 level = 1;
    char args1[512];
    strcpy (args1, args);

    skill = atol (strtok(args1, " "));

    if (skill <= 0)
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_INVALID_SKILL_ID, skill));
        m_session->SendPacket(&data);
        return true;
    }

    Player * player = m_session->GetPlayer();
    Player * target = objmgr.GetPlayer(player->GetSelection());

    if (!target) target = player;

    if (target->GetSkillValue(skill))
    {
        target->SetSkill(skill, 0, 0);
        FillSystemMessageData(&data, m_session, fmtstring(LANG_UNLEARNED_SKILL, skill));
        m_session->SendPacket(&data);
    } else
    {
        FillSystemMessageData(&data, m_session, LANG_UNKNOWN_SKILL);
        m_session->SendPacket(&data);
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
    WorldPacket data;

    if (!*args)
        return false;

    if (!strcmp(args, "all"))
    {
        int loop = 0;

        FillSystemMessageData(&data, m_session, fmtstring(LANG_LEARNING_GM_SKILLS, m_session->GetPlayer()->GetName()));
        m_session->SendPacket(&data);

        while (strcmp(gmSpellList[loop], "0"))
        {
            uint32 spell = atol((char*)gmSpellList[loop]);

            if (m_session->GetPlayer()->HasSpell(spell))
            {
                loop++;
                continue;
            }

            data.Initialize( SMSG_LEARNED_SPELL );
            data << (uint32)spell;
            m_session->SendPacket( &data );
            m_session->GetPlayer()->addSpell((uint16)spell);

            loop++;
        }

        return true;
    }

    uint32 spell = atol((char*)args);

    if (m_session->GetPlayer()->HasSpell(spell))
    {
        FillSystemMessageData(&data, m_session, LANG_KNOWN_SPELL);
        m_session->SendPacket(&data);
        return true;
    }

    data.Initialize( SMSG_LEARNED_SPELL );
    data << (uint32)spell;
    m_session->SendPacket( &data );
    m_session->GetPlayer()->addSpell((uint16)spell);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    WorldPacket data;

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

    for(uint32 spell=minS;spell<maxS;spell++)
    {
        if (m_session->GetPlayer()->HasSpell(spell))
        {
            data.Initialize(SMSG_REMOVED_SPELL);
            data << (uint32)spell;
            m_session->SendPacket( &data );
            m_session->GetPlayer()->removeSpell(spell);
        }
        else
        {
            FillSystemMessageData(&data, m_session, LANG_FORGET_SPELL);
            m_session->SendPacket(&data);
        }
    }

    return true;
}

bool ChatHandler::HandleAddItemCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
    {
        FillSystemMessageData(&data, m_session, LANG_ADDITEM_SYNTAX);
        m_session->SendPacket(&data);
        return true;
    }

    char* citemId = strtok((char*)args, " ");
    char* ccount = strtok(NULL, " ");
    uint32 itemId = atol(citemId);
    uint32 count = 1;

    if (ccount) { count = atol(ccount); }
    if (count < 1) { count = 1; }

    Player* pl = m_session->GetPlayer();

    sLog.outDetail(LANG_ADDITEM, itemId, count);
    if( uint16 dst = pl->CanStoreNewItem( NULL, NULL_SLOT, itemId, count, false, true ) )
    {
        pl->StoreNewItem( dst, itemId, count);
        FillSystemMessageData(&data, m_session, fmtstring(LANG_ITEM_CREATED, itemId, count));
    }
    else
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_ITEM_CANNOT_CREATE, itemId, count));
    }
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleCreateGuildCommand(const char* args)
{
    WorldPacket data;
    Guild *guild;
    Player * player;
    char *lname,*gname;
    std::string guildname;

    if (!*args)
        return false;

    lname = strtok((char*)args, " ");
    gname = strtok(NULL, " ");
    if(!lname)
        return false;
    else if(!gname)
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_INSERT_GUILD_NAME));
        m_session->SendPacket(&data);
        return true;
    }

    guildname = gname;
    player = ObjectAccessor::Instance().FindPlayerByName(lname);

    if(!player)
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_PLAYER_NOT_FOUND));
        m_session->SendPacket(&data);
        return true;
    }

    if(!player->GetGuildId())
    {
        guild = new Guild;
        guild->create(player->GetGUID(),guildname);
        objmgr.AddGuild(guild);
    }
    else
    {
        FillSystemMessageData(&data, m_session, fmtstring(LANG_PLAYER_IN_GUILD));
        m_session->SendPacket(&data);
    }

    return true;
}

//float max_creature_distance = 160;

bool ChatHandler::HandleGetDistanceCommand(const char* args)
{
    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_SELECTION);
        m_session->SendPacket( &data );
        return true;
    }

    Creature* pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, LANG_SELECT_CREATURE);
        m_session->SendPacket( &data );
        return true;
    }

    FillSystemMessageData(&data, m_session, fmtstring(LANG_DISTANCE, m_session->GetPlayer()->GetDistanceSq(pCreature)));
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleObjectCommand(const char* args)
{
    WorldPacket data;

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
        return false;
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 19);
    sLog.outError(LANG_ADD_OBJ_LV3);
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

    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_SELECTION);
        m_session->SendPacket( &data );
        return true;
    }

    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, LANG_SELECT_CREATURE);
        m_session->SendPacket( &data );
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
    std::stringstream sstext;
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
                sstext << LANG_ITEM_SLOT << SlotID << LANG_ITEM_SLOT_NOT_EXIST << '\0';
                added = false;
                break;
        }
        if(added)
        {
            sstext << LANG_ITEM << ItemID << "' '" << tmpItem->Name1 << LANG_ITEM_ADDED_TO_SLOT << SlotID << '\0';
        }
    }
    else
    {
        sstext << LANG_ITEM << ItemID << LANG_ITEM_NOT_FOUND << '\0';
        return true;
    }
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    */
    return true;
}

bool ChatHandler::HandleGameObjectCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;
    std::stringstream sstext;

    uint32 id = atoi((char*)args);
    if(!id)
    {
        sstext << LANG_GAMEOBJECT_SYNTAX << '\0';
        FillSystemMessageData(&data, m_session, sstext.str().c_str());
        m_session->SendPacket( &data );
        return false;
    }

    const GameObjectInfo *goI = objmgr.GetGameObjectInfo(id);

    if (!goI)
    {
        sstext << LANG_GAMEOBJECT << id << LANG_GAMEOBJECT_NOT_EXIST << '\0';
        FillSystemMessageData(&data, m_session, sstext.str().c_str());
        m_session->SendPacket( &data );
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
        return false;
    //    pGameObj->SetZoneId(chr->GetZoneId());
    pGameObj->SetMapId(chr->GetMapId());
    //    pGameObj->SetNameId(id);
    sLog.outError(LANG_GAMEOBJECT_CURRENT, goI->name, lowGUID, x, y, z, o);

    pGameObj->SaveToDB();
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);

    sstext << LANG_GAMEOBJECT_ADD << id << "' (" << goI->name << LANG_GAMEOBJECT_ADD_AT << x << " " << y << " " << z <<"'." << '\0';
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );

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
    Player* SelectedPlayer=NULL;
    Player *  player=m_session->GetPlayer();
    uint64 guid = player->GetSelection();
    if(guid)
        SelectedPlayer = objmgr.GetPlayer(guid);

    if(!SelectedPlayer)
        SelectedPlayer = player;

    SelectedPlayer->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
    SelectedPlayer->setDeathState(JUST_DIED);
    return true;
}

bool ChatHandler::HandleReviveCommand(const char* args)
{
    Player* SelectedPlayer;
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if(guid == 0)
    {
        SelectedPlayer = m_session->GetPlayer();
    }
    else
    {
        SelectedPlayer = objmgr.GetPlayer(guid);
    }
    if(!SelectedPlayer)
    {
        SelectedPlayer = m_session->GetPlayer();
    }

    SelectedPlayer->ResurrectPlayer();
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_HEALTH, (uint32)(SelectedPlayer->GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.50) );
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
                Aura *Aur = new Aura(spellInfo, i, NULL, m_session->GetPlayer());
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

bool ChatHandler::HandleAddGraveCommand(const char* args)
{

    sDatabase.PExecute("INSERT INTO `areatrigger_graveyard` ( `position_x`,`position_y`,`position_z`,`map`) VALUES ('%f', '%f', '%f', '%u');", m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetMapId() );

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

    pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), "Spirit Healer", chr->GetMapId(), x, y, z, o, objmgr.AddCreatureTemplate(pCreature->GetName(), 5233));
    pCreature->SetZoneId(chr->GetZoneId());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureTemplate(pCreature->GetName(), 5233));
    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 5233);
    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, 33);
    pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE , 35);
    pCreature->SetUInt32Value(UNIT_FIELD_HEALTH, 100);
    pCreature->SetUInt32Value(UNIT_FIELD_MAXHEALTH, 100);
    pCreature->SetUInt32Value(UNIT_FIELD_LEVEL, 60);
    pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, 768);
    pCreature->SetUInt32Value(UNIT_FIELD_AURA+0, 10848);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS+0, 0xEEEEEE3C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+0, 0xEEEEEE00);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS+0, 0x00000009);
    pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH , 1.5f);
    pCreature->SetFloatValue(UNIT_FIELD_MAXDAMAGE ,  5.0f);
    pCreature->SetFloatValue(UNIT_FIELD_MINDAMAGE , 8.0f);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, 1900);
    pCreature->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME+1, 2000);
    pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2.0f);

    sLog.outError("AddObject at Level3.cpp line 1470");
    pCreature->AIM_Initialize();
    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);

    pCreature->SaveToDB();

    std::stringstream ss,ss2,ss3;
    QueryResult *result;

    result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_gossip`;" );
    if( result )
    {
        sDatabase.PExecute("INSERT INTO `npc_gossip` (`id`,`npc_guid`,`gossip_type`,`textid`,`option_count`) VALUES ('%u', '%u', '%u', '%u', '%u');", (*result)[0].GetUInt32()+1, pCreature->GetGUIDLow(), 1, 1, 1);
        delete result;
        result = NULL;

        result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_option`;" );
        if( result )
        {
            sDatabase.PExecute("INSERT INTO `npc_option` (`id`,`gossip_id`,`type`,`option`,`npc_text_nextid`,`special`) VALUES ('%u', '%u', '%u', '%s', '%u', '%u');", (*result)[0].GetUInt32()+1, (*result)[0].GetUInt32()+2, 0, "Return me to life.", 0, 2);
            delete result;
            result = NULL;
        }
        result = sDatabase.PQuery( "SELECT MAX(`id`) FROM `npc_text`;" );
        if( result )
        {
            sDatabase.PExecute("INSERT INTO `npc_text` (`id`,`text0_0`) VALUES ('%u', '%s');", (*result)[0].GetUInt32()+1, "It is not yet your time. I shall aid your journey back to the realm of the living... For a price.");

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
    Player* chr = m_session->GetPlayer();
    if(chr->GetSelection() == 0)
        return false;

    Unit* target = ObjectAccessor::Instance().GetCreature(*chr, chr->GetSelection());
    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);

    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char* args)
{
    WorldPacket data;

    char buf[512];
    uint64 guid = m_session->GetPlayer()->GetSelection();
    uint32 faction = 0, npcflags = 0, skinid = 0, Entry = 0;

    Unit* target = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);

    if(!target)
    {
        sChatHandler.FillSystemMessageData(&data, m_session, LANG_SELECT_CREATURE);
        m_session->SendPacket(&data);
        return true;
    }

    faction = target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE);
    npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    skinid = target->GetUInt32Value(UNIT_FIELD_DISPLAYID);
    Entry = target->GetUInt32Value(OBJECT_FIELD_ENTRY);

    // 'id' changed to 'guid'
    QueryResult *result = sDatabase.PQuery("SELECT * FROM `creature` WHERE `guid` = '%u';", target->GetGUIDLow());

    Field *fields = result->Fetch();

    sprintf(buf,LANG_NPCINFO_CHAR, GUID_LOPART(guid), faction, npcflags, Entry, skinid);
    sprintf(buf,LANG_NPCINFO_LEVEL, buf, target->GetUInt32Value(UNIT_FIELD_LEVEL));
    sprintf(buf,LANG_NPCINFO_HEALTH, buf, target->GetUInt32Value(UNIT_FIELD_BASE_HEALTH), target->GetUInt32Value(UNIT_FIELD_MAXHEALTH), target->GetUInt32Value(UNIT_FIELD_HEALTH));
    sprintf(buf,LANG_NPCINFO_FLAGS, buf, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));

    sprintf(buf,LANG_NPCINFO_POSITION, buf, float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));

    if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
    {
        sprintf(buf,LANG_NPCINFO_VENDOR, buf);
    }
    if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
    {
        sprintf(buf,LANG_NPCINFO_TRAINER, buf);
    }

    sChatHandler.FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket(&data);
    delete result;
    return true;
}

bool ChatHandler::HandleNpcInfoSetCommand(const char* args)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    uint32 entry = 0, testvalue = 0;

    Unit* target = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), m_session->GetPlayer()->GetSelection());

    if(!target || !args)
    {
        return true;
    }

    //m_session->GetPlayer( )->SetUInt32Value(PLAYER_FLAGS, (uint32)8);

    testvalue = uint32(atoi((char*)args));

    entry = target->GetUInt32Value( OBJECT_FIELD_ENTRY );

    m_session->SendTestCreatureQueryOpcode( entry, guid, testvalue );

    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_CHAR_SELECTED);
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    if (flag != 0)
    {
        sprintf((char*)buf,LANG_YOU_SET_EXPLORE_ALL, chr->GetName());
    }
    else
    {
        sprintf((char*)buf,LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

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

    WorldPacket data1;
    std::stringstream sstext;
    if (flag)
        sstext << LANG_HOVER_ENABLED << '\0';
    else
        sstext << LANG_HOVER_DISABLED << '\0';

    FillSystemMessageData(&data1, m_session, sstext.str().c_str());
    m_session->SendPacket( &data1 );

    return true;
}

bool ChatHandler::HandleLevelUpCommand(const char* args)
{

    int nrlvl = atoi((char*)args);

    for(int i=0;i<nrlvl || i==0;i++)
    {
        uint32 curXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_XP);
        uint32 nextLvlXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
        uint32 givexp = nextLvlXP - curXP;

        //        uint32 points2 = m_session->GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS2);
        //        m_session->GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS2,points2+2);

        m_session->GetPlayer()->GiveXP(givexp,m_session->GetPlayer()->GetGUID());

        WorldPacket data;
        std::stringstream sstext;
        sstext << LANG_YOURS_LEVEL_UP << '\0';
        FillSystemMessageData(&data, m_session, sstext.str().c_str());
        m_session->SendPacket( &data );
    }
    return true;
}

bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_CHAR_SELECTED);
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    FillSystemMessageData(&data, m_session, LANG_EXPLORE_AREA);
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleHideAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_CHAR_SELECTED);
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    FillSystemMessageData(&data, m_session, LANG_UNEXPLORE_AREA);
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleUpdate(const char* args)
{
    WorldPacket data;

    uint32 updateIndex;
    uint32 value;
    char buf[256];

    char* pUpdateIndex = strtok((char*)args, " ");
    Unit* chr =NULL;
    chr = ObjectAccessor::Instance().GetUnit(*m_session->GetPlayer(), m_session->GetPlayer()->GetSelection());

    if (chr == NULL)
    {
        FillSystemMessageData(&data, m_session, LANG_NO_CHAR_SELECTED);
        m_session->SendPacket( &data );
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

        sprintf((char*)buf,LANG_UPDATE, chr->GetGUIDLow(),updateIndex,value);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        return true;
    }

    value=atoi(pvalue);

    sprintf((char*)buf,LANG_UPDATE_CHANGE, chr->GetGUIDLow(),updateIndex,value);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

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
    WorldPacket data;

    char buf[256];
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    Unit* target = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);
    if(!target)
    {
        target = m_session->GetPlayer();
        guid = target->GetGUID();
    }

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        sprintf((char*)buf,LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
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
        sprintf((char*)buf,LANG_SET_UINT_FIELD, GUID_LOPART(guid), Opcode,iValue);
    }
    else
    {
        fValue = (float)atof(py);
        sLog.outDebug( LANG_SET_FLOAT, GUID_LOPART(guid), Opcode, fValue);
        target->SetFloatValue( Opcode , fValue );
        sprintf((char*)buf,LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), Opcode,fValue);
    }

    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleGetValue(const char* args)
{
    WorldPacket data;

    char buf[256];
    char* px = strtok((char*)args, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    Unit* target = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), m_session->GetPlayer()->GetSelection());
    if(!target)
    {
        target = m_session->GetPlayer();
        guid = target->GetGUID();
    }

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        sprintf((char*)buf,LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );
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
        sprintf((char*)buf,LANG_GET_UINT_FIELD, GUID_LOPART(guid), Opcode,    iValue);
    }
    else
    {
        fValue = target->GetFloatValue( Opcode );
        sLog.outDebug( LANG_GET_FLOAT, GUID_LOPART(guid), Opcode, fValue);
        sprintf((char*)buf,LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), Opcode, fValue);
    }
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    return true;
}

bool ChatHandler::HandleSet32Bit(const char* args)
{
    WorldPacket data;

    char buf[256];
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

    sprintf((char*)buf,LANG_SET_32BIT_FIELD, Opcode,1);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleMod32Value(const char* args)
{
    WorldPacket data;

    char buf[256];
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

    sprintf((char*)buf,LANG_CHANGE_32BIT_FIELD, Opcode,CurrentValue);
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

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
    bool checkmail=false;

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

// TODO Add a commando "Illegal name" to set playerflag |= 32;
// maybe do'able with a playerclass m_Illegal_name = false
