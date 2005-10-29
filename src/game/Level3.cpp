/* Level3.cpp
 *
 * Copyright (C) 2004 Wow Daemon
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

/////////////////////////////////////////////////
//  Admin Chat Commands
//

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

#ifdef ENABLE_GRID_SYSTEM
#include "ObjectAccessor.h"
#include "MapManager.h"
#endif

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
        FillSystemMessageData(&data, m_session, "Incorrect value.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

#ifndef ENABLE_GRID_SYSTEM
    Player *chr = objmgr.GetPlayer(args);
#else
    Player* chr = ObjectAccessor::Instance().FindPlayerByName(args);
#endif
    if (chr)
    {
        // send message to user
        sprintf((char*)buf,"You change security level of %s to %i.", chr->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);
        m_session->SendPacket( &data );

        // send message to player
        sprintf((char*)buf,"%s changed your security level to %i.", m_session->GetPlayer()->GetName(), gm);
        FillSystemMessageData(&data, m_session, buf);

        chr->GetSession()->SendPacket(&data);
        chr->GetSession()->SetSecurity(gm);

        char sql[512];
        sprintf(sql, "UPDATE accounts SET gm = '%i' WHERE acct = '%u'", gm, chr->GetSession()->GetAccountId());
        sDatabase.Execute( sql );
    }
    else
    {
        sprintf((char*)buf,"Player (%s) does not exist or is not logged in.", pName);
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

    // LINA
    smsg_NewWorld(m_session, atoi(pContinent), (float)atof(px), (float)atof(py), (float)atof(pz));

    return true;
}


bool ChatHandler::HandleAllowMovementCommand(const char* args)
{
    WorldPacket data;
    if(sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        FillSystemMessageData(&data, m_session, "Creature Movement Disabled.");
    }
    else
    {

        sWorld.SetAllowMovement(true);
        FillSystemMessageData(&data, m_session, "Creature Movement Enabled.");
    }

    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleAddSpiritCommand(const char* args)
{
    Log::getSingleton( ).outDetail("Spawning Spirit Healers\n");

    std::stringstream query;
    Creature* pCreature;
    UpdateMask unitMask;
    WorldPacket data;

    query << "select X,Y,Z,F,name_id,mapId,zoneId,faction_id from spirithealers";
    QueryResult *result = sDatabase.Query( query.str( ).c_str( ) );

    if(!result)
    {
        FillSystemMessageData(&data, m_session, "No spirit healers in db, exiting.");
        m_session->SendPacket( &data );

        return true;
    }

    uint32 name;
    do
    {
        Field* fields = result->Fetch();

        name = fields[4].GetUInt32();
        Log::getSingleton( ).outDetail("%s name is %d\n", fields[4].GetString(), name);

        pCreature = new Creature();

        pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), objmgr.GetCreatureName(name)->Name.c_str(), fields[5].GetUInt16(),
            fields[0].GetFloat(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat());

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
        Log::getSingleton( ).outError("AddObject at Level3.cpp line 172");
#ifndef ENABLE_GRID_SYSTEM
        objmgr.AddObject(pCreature);
        pCreature->PlaceOnMap();
#else
	MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);
#endif

        pCreature->SaveToDB();
    }
    while( result->NextRow() );

    delete result;

    return true;
}


bool ChatHandler::HandleMoveCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);

    MovePlayer(m_session, x, y, z);

    return true;
}

// Game master learn spells/skills list.. CHECKME: Move into the SQL database for easy editing by admins???
const char *gmSpellList[] = {
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
	{// UQ1: .learn all command. Teach dm's some good default skills/spells...
		int loop = 0;

		FillSystemMessageData(&data, m_session, fmtstring("%s - Learning default GM spells/skills.", m_session->GetPlayer()->GetName()));
		m_session->SendPacket(&data);

		while (strcmp(gmSpellList[loop], "0"))
		{// While not at end of the list ("0"), add each spell from the list...
			uint32 spell = atol((char*)gmSpellList[loop]);

			//FillSystemMessageData(&data, m_session, fmtstring("Learning spell/skill %i.", spell));
			//m_session->SendPacket(&data);

			if (m_session->GetPlayer()->HasSpell(spell))  // check to see if char already learned spell
			{
				//FillSystemMessageData(&data, m_session, "You already know that spell.");
				//m_session->SendPacket(&data);
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

    if (m_session->GetPlayer()->HasSpell(spell))  // check to see if char already learned spell
    {
        FillSystemMessageData(&data, m_session, "You already know that spell.");
        m_session->SendPacket(&data);
        return true;
    }

    data.Initialize( SMSG_LEARNED_SPELL );
    data << (uint32)spell;
    m_session->SendPacket( &data );
    m_session->GetPlayer()->addSpell((uint16)spell);

    return true;
}

//add by vendy for unlearn spell 2005/10/9 23:26
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

    if (!endS){       
        minS = (uint32)atol(startS);
        maxS =  minS+1;
    }else{       
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
        if (m_session->GetPlayer()->HasSpell(spell)) // check to see if char already learned spell
        {
            data.Initialize(SMSG_REMOVED_SPELL);
            data << (uint32)spell; 
            m_session->SendPacket( &data ); 
            m_session->GetPlayer()->removeSpell(spell);
        }else{
            FillSystemMessageData(&data, m_session, "You already forget that spell.");
            m_session->SendPacket(&data);
        }
    }

    return true;
}

//add by vendy for add item to slot 2005/10/16 20:32
bool ChatHandler::HandleAddItemCommand(const char* args)
{
	
    WorldPacket data;

    if (!*args)  
        return false;

	char* citemid = strtok((char*)args, " ");
    char* cPos = strtok(NULL, " ");
    char* cVal = strtok(NULL, " ");

	uint32 itemid=atol(citemid);

	Player*	pl = m_session->GetPlayer();
	bool   slotfree=false;
	uint8  i,slot;
    uint32 Pos=5,Val=1;
	

	for(i =	INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END;	i++)
	{
		if (pl->GetItemBySlot(i) == NULL)
		{
			slot = i;
			slotfree=true;
			break;
		}
	}
	if (slotfree)
	{
		Item *item = new Item();	
		item->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM),	itemid, pl);
		
		//添加物品属性
		if ((cPos) && (cVal)){
		    Pos=(uint32)atol(cPos);
            Val=(uint32)atol(cVal);
			//for(int j=Pos;j<Val;j++)
			item->SetUInt32Value( Pos, Val );
		}

		pl->AddItemToSlot(	slot, item );
	}else{
        FillSystemMessageData(&data, m_session, "Bag is full.");
        m_session->SendPacket(&data);
	}

    return true;
}

float max_creature_distance = 160;

bool ChatHandler::HandleCreatureDistanceCommand(const char* args)
{
	WorldPacket data;

    if (!*args)
        return false;

	max_creature_distance = (float)atof((char*)args);

	FillSystemMessageData(&data, m_session, fmtstring("Creature max think distance set to %f (units from nearest player).", max_creature_distance));
    m_session->SendPacket(&data);

	return true;
}

bool ChatHandler::HandleObjectCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    uint32 display_id = atoi((char*)args);
    //char* name = strtok((char*)args, " ");
    char* safe = strtok((char*)args, " ");

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    GameObject* pGameObj = new GameObject();
    pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), display_id, chr->GetMapId(), x, y, z, o);	
    pGameObj->SetZoneId(chr->GetZoneId());
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, 19);
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 252");
#ifndef ENABLE_GRID_SYSTEM
    objmgr.AddObject(pGameObj);
    pGameObj->PlaceOnMap();
#else
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
#endif
    
    if(strcmp(safe,"true") == 0)
	pGameObj->SaveToDB();
    
    return true;
}


bool ChatHandler::HandleAddWeaponCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        FillSystemMessageData(&data, m_session, "No selection.");
        m_session->SendPacket( &data );
        return true;
    }

#ifndef ENABLE_GRID_SYSTEM
    Creature * pCreature = objmgr.GetCreature(guid);
#else
    Creature *pCreature = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), guid);
#endif
    if(!pCreature)
    {
        FillSystemMessageData(&data, m_session, "You should select a creature.");
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
                sstext << "Item Slot '" << SlotID << "' doesn't exist." << '\0';
                added = false;
                break;
        }
        if(added)
        {
            sstext << "Item '" << ItemID << "' '" << tmpItem->Name1 << "' Added to Slot " << SlotID << '\0';
        }
    }
    else
    {
        sstext << "Item '" << ItemID << "' Not Found in Database." << '\0';
        return true;
    }
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleGameObjectCommand(const char* args)
{
    if (!*args)
        return false;

    WorldPacket data;

    uint32 display_id = atoi((char*)args);
    if(!display_id) return false;

    uint16 typesid = atoi((char*)args);
    if(!typesid) return false;

    uint16 factionid = atoi((char*)args);
    if(!factionid) return false;

    uint32 fieldentry = atoi((char*)args);
    if(!fieldentry) return false;

    // char* name = strtok((char*)args, " ");
    // char* safe = strtok((char*)args, " ");

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();
    uint32 guidlow = objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT);
    Log::getSingleton( ).outError("GameObjectGUIDlow %u",guidlow);

    GameObject* pGameObj = new GameObject();

    // uint32 guidlow, uint16 display_id, uint8 state, uint32 obj_field_entry, uint8 scale, uint16 type, uint16 faction,  float x, float y, float z, float ang
    pGameObj->Create(guidlow, display_id, chr->GetMapId(), x, y, z, o);
    pGameObj->SetUInt32Value(OBJECT_FIELD_ENTRY, fieldentry);
    pGameObj->SetUInt32Value(GAMEOBJECT_TYPE_ID, typesid);
    pGameObj->SetZoneId(chr->GetZoneId());
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 252");
#ifndef ENABLE_GRID_SYSTEM
    objmgr.AddObject(pGameObj);
    pGameObj->PlaceOnMap();
#else
    MapManager::Instance().GetMap(pGameObj->GetMapId())->Add(pGameObj);
#endif

    //if(strcmp(safe,"true") == 0)
    pGameObj->SaveToDB();

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
#ifndef ENABLE_GRID_SYSTEM
    m_session->GetPlayer()->SendMessageToSet(&data, true);
#else
    MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId())->MessageBoardcast(m_session->GetPlayer(), &data, true);
#endif

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

    SelectedPlayer->SetMovement(MOVE_LAND_WALK);
    SelectedPlayer->SetMovement(MOVE_UNROOT);
    SelectedPlayer->SetPlayerSpeed(RUN, (float)7.5, true);
    SelectedPlayer->SetPlayerSpeed(SWIM, (float)4.9, true);

    SelectedPlayer->SetUInt32Value(CONTAINER_FIELD_SLOT_1+29, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURA+32, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURALEVELS+8, 0xeeeeeeee);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeeeee);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURAFLAGS+4, 0);
    SelectedPlayer->SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

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
    // m_session->GetPlayer()->UpdateObject( );
    // m_session->GetPlayer()->SendMessageToSet(&data, true);

    return true;
}


bool ChatHandler::HandleAuraCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 aura_id = atoi((char*)args);

    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURA, aura_id );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAFLAGS, 0x0000000d );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURA+32, aura_id );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURALEVELS+8, 0xeeeeee00 );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAAPPLICATIONS+8, 0xeeeeee00 );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURAFLAGS+4, 0x0000000d );
    m_session->GetPlayer( )->SetUInt32Value( UNIT_FIELD_AURASTATE, 0x00000002 );
    // m_session->GetPlayer()->UpdateObject( );

    return true;
}


bool ChatHandler::HandleAddGraveCommand(const char* args)
{
    //QueryResult *result;
    std::stringstream ss;
  //  GraveyardTeleport *pGrave;

   // ss.rdbuf()->str("");
  //  pGrave = new GraveyardTeleport;

   // result = sDatabase.Query( "SELECT MAX(ID) FROM graveyards" );

  /*  if( result )
    {
        pGrave->ID = (*result)[0].GetUInt32()+1;

        delete result;
    }*/
   // pGrave->X = m_session->GetPlayer()->GetPositionX();
  //  pGrave->Y = m_session->GetPlayer()->GetPositionY();
  //  pGrave->Z = m_session->GetPlayer()->GetPositionZ();
    //pGrave->O = m_session->GetPlayer()->GetOrientation();
  //  pGrave->ZoneId = m_session->GetPlayer()->GetZoneId();
 //   pGrave->MapId = m_session->GetPlayer()->GetMapId();
  //  pGrave->FactionID = 0;

/*    ss << "INSERT INTO graveyards ( X, Y, Z, O, zoneId, mapId) VALUES ("
        << pGrave->X << ", "
        << pGrave->Y << ", "
        << pGrave->Z << ", "
        << pGrave->O<< ", "
        << pGrave->ZoneId << ", "
        << pGrave->MapId << ")";*/
	
	 ss << "INSERT INTO graveyards ( X, Y, Z, mapId) VALUES ("
        << m_session->GetPlayer()->GetPositionX() << ", "
        << m_session->GetPlayer()->GetPositionY() << ", "
        << m_session->GetPlayer()->GetPositionZ() << ", "
        << m_session->GetPlayer()->GetMapId() << ")";

    sDatabase.Execute( ss.str( ).c_str( ) );

    //objmgr.AddGraveyard(pGrave);
    return true;
}


bool ChatHandler::HandleAddSHCommand(const char *args)
{
    WorldPacket data;

    // Create the requested monster
    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();

    Creature* pCreature = new Creature();

    pCreature->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT), "Spirit Healer", chr->GetMapId(), x, y, z, o);
    pCreature->SetZoneId(chr->GetZoneId());
    pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, objmgr.AddCreatureName(pCreature->GetName(), 5233));
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
    Log::getSingleton( ).outError("AddObject at Level3.cpp line 455");
#ifndef ENABLE_GRID_SYSTEM
    objmgr.AddObject(pCreature);
    pCreature->PlaceOnMap();
#else
    MapManager::Instance().GetMap(pCreature->GetMapId())->Add(pCreature);
#endif
    pCreature->SaveToDB();

    std::stringstream ss,ss2,ss3;
    QueryResult *result;

    result = sDatabase.Query( "SELECT MAX(ID) FROM npc_gossip" );
    if( result )
    {
        ss2 << "INSERT INTO npc_gossip ( ID , NPC_GUID, GOSSIP_TYPE, TEXTID, OPTION_COUNT) VALUES ("
            << (*result)[0].GetUInt32()+1 << ", "
            << pCreature->GetGUIDLow() << ", "
            << 1 << ", "
            << 1 << ", "
            << 1 << ")";

        sDatabase.Execute( ss2.str( ).c_str( ) );
        delete result;
        result = NULL;

        result = sDatabase.Query( "SELECT MAX(ID) FROM npc_options" );
        if( result )
        {
            ss << "INSERT INTO npc_options ( `ID` , `GOSSIP_ID`, `TYPE`, `OPTION`, `NPC_TEXT_NEXTID`, `SPECIAL`) VALUES ("
                << (*result)[0].GetUInt32()+1 << ", "
                << (*result)[0].GetUInt32()+2 << ", "
                << 0 << ", '"
                << "Return me to life." << "', "
                << 0 << ", "
                << 2 << ")";

            sDatabase.Execute( ss.str( ).c_str( ) );
            delete result;
            result = NULL;
        }
        result = sDatabase.Query( "SELECT MAX(ID) FROM npc_text" );
        if( result )
        {
            ss3 << "INSERT INTO npc_text ( ID , TYPE_UNUSED, TEXT) VALUES ("
                << (*result)[0].GetUInt32()+1 << ", "
                << 0 << ", '"
                << "It is not yet your time. I shall aid your journey back to the realm of the living... For a price." << "')";
            sDatabase.Execute( ss3.str( ).c_str( ) );
            delete result;
            result = NULL;
        }
    }

    return true;
}


bool ChatHandler::HandleSpawnTransportCommand(const char* args)
{
    // don't need this anymore cuz its in the world.save or sql table
    return true;
}


bool ChatHandler::HandleEmoteCommand(const char* args)
{
    uint32 emote = atoi((char*)args);
    Player* chr = m_session->GetPlayer();
    if(chr->GetSelection() == 0)
        return false;

#ifndef ENABLE_GRID_SYSTEM
    Unit* target = objmgr.GetObject<Creature>(chr->GetSelection());
#else
    Unit* target = ObjectAccessor::Instance().GetCreature(*chr, chr->GetSelection());
#endif
    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);

    return true;
}


bool ChatHandler::HandleNpcInfoCommand(const char* args)
{
    WorldPacket data;
    char buf[256];
    uint32 guid = m_session->GetPlayer()->GetSelection();
    uint32 factionid = 0, npcflags = 0, skinid = 0;
#ifndef ENABLE_GRID_SYSTEM
    Unit* target = objmgr.GetObject<Creature>(m_session->GetPlayer()->GetSelection());
#else
    Unit* target = ObjectAccessor::Instance().GetCreature(*m_session->GetPlayer(), m_session->GetPlayer()->GetSelection());
#endif

    if(!target)
    {
        sChatHandler.FillSystemMessageData(&data, m_session, "Select something first.");
        m_session->SendPacket(&data);
        return true;
    }

    factionid = target->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE);
    npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    skinid = target->GetUInt32Value(UNIT_FIELD_DISPLAYID);

    std::stringstream ss;
    ss << "SELECT * FROM creatures WHERE id = " << target->GetGUIDLow() << '\0';

    QueryResult *result = sDatabase.Query( ss.str().c_str() );

    Field *fields = result->Fetch();

    sprintf(buf,"Player selected guid: %d. faction %d. flag %d. name_id %d. skin_id %d.",guid,factionid,npcflags,fields[8].GetUInt32(),skinid);
    sChatHandler.FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket(&data);

    return true;
}


bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    char buf[256];

    // send message to user
    if (flag != 0)
    {
        sprintf((char*)buf,"%s has explored all zones now.", chr->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has no more explored zones.", chr->GetName());
    }
    FillSystemMessageData(&data, m_session, buf);
    m_session->SendPacket( &data );

    // send message to player
    if (flag != 0)
    {
        sprintf((char*)buf,"%s has explored all zones for you.",
            m_session->GetPlayer()->GetName());
    }
    else
    {
        sprintf((char*)buf,"%s has hidden all zones from you.",
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

    data.Initialize(SMSG_MOVE_SET_HOVER);
    data << m_session->GetPlayer()->GetGUID();
    m_session->SendPacket( &data );

    WorldPacket data1;
    std::stringstream sstext;
    sstext << "Hover Enabled" << '\0';
    FillSystemMessageData(&data1, m_session, sstext.str().c_str());
    m_session->SendPacket( &data1 );

/*
SMSG_MOVE_SET_HOVER = 244,
SMSG_MOVE_UNSET_HOVER = 245,
*/

    return true;
}


bool ChatHandler::HandleLevelUpCommand(const char* args)
{
    uint32 curXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = m_session->GetPlayer()->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 givexp = nextLvlXP - curXP;

    uint32 points2 = m_session->GetPlayer()->GetUInt32Value(PLAYER_CHARACTER_POINTS2);
    m_session->GetPlayer()->SetUInt32Value(PLAYER_CHARACTER_POINTS2,points2+2);

    m_session->GetPlayer()->GiveXP(givexp,m_session->GetPlayer()->GetGUID());

    WorldPacket data;
    std::stringstream sstext;
    sstext << "You have been leveled Up" << '\0';
    FillSystemMessageData(&data, m_session, sstext.str().c_str());
    m_session->SendPacket( &data );
    return true;
}


bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    WorldPacket data;

    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedChar(m_session);
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    FillSystemMessageData(&data, m_session, "The area has been set as explored.");
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
    if (chr == NULL)                              // Ignatich: what should NOT happen but just in case...
    {
        FillSystemMessageData(&data, m_session, "No character selected.");
        m_session->SendPacket( &data );
        return true;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    FillSystemMessageData(&data, m_session, "The area has been set as not explored.");
    m_session->SendPacket( &data );
    return true;
}
