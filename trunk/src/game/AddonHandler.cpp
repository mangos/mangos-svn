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

#include "AddonHandler.h"
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "zlib/zlib.h"

//TODO: Make level3 commands for this. .Loadaddons .Saveaddons .loadaddondefault

INSTANTIATE_SINGLETON_1( AddonHandler );

AddonHandler::AddonHandler()
{
}

AddonHandler::~AddonHandler()
{
    //clean the addon data when close
    m_Addon_data.clear();
}

uint8 AddonHandler::_removeAddon(std::string* Name)
{
    sLog.outDebug("Remove addon:%s",(*Name).c_str());
    for(std::list<AddOns*>::iterator i = m_Addon_data.begin();i!=m_Addon_data.end();++i)
    {
        //if name exists remove it and return 0
        if ((*i)->Name == *Name)
        {
            //remove addon, from mem and from sql
            sDatabase.PExecute("DELETE FROM `game_addons` WHERE `addonname` = '%s'",(*i)->Name.c_str());
            m_Addon_data.remove(*i);
            return 1;
        }
    }
    //if name does not exists return 0
    return 0;
}

void AddonHandler::_AddAddon(AddOns* newaddon)
{
    m_Addon_data.push_back( newaddon );
}

void AddonHandler::_SaveToDB()
{

    sLog.outDebug("Size of Addon:%d",m_Addon_data.size());
    for(std::list<AddOns*>::iterator i = m_Addon_data.begin();i!=m_Addon_data.end();++i)
    {
        sDatabase.PExecute("DELETE FROM `game_addons` WHERE `addonname` = '%s'",(*i)->Name.c_str());
        //TODO FIX THE (uint32)
        sDatabase.PExecute("INSERT INTO `game_addons` (`addonname`,`crc`,`enabled`) VALUES('%s','%lu','%d');", (*i)->Name.c_str(),(uint32)(*i)->CRC,(*i)->Enabled);
    }
}

void AddonHandler::_LoadFromDB()
{
    //clean the addon data before use
    m_Addon_data.clear();

    QueryResult *result = sDatabase.PQuery( "SELECT * FROM `game_addons`;" );
    if( !result )
        return;

    AddOns *newaddon;

    do
    {
        Field *fields = result->Fetch();

        newaddon = new AddOns;
        newaddon->Name = fields[0].GetString();
        newaddon->CRC = fields[1].GetUInt64();
        newaddon->Enabled = fields[2].GetUInt8();           //fields[2].GetBool();

        sLog.outDebug("LoadAddons from DB:%s State:%s", newaddon->Name.c_str(), newaddon->Enabled ? "Enabled" : "Disabled" );
        _AddAddon(newaddon);
    } while (result->NextRow());
    delete result;
}

bool AddonHandler::GetAddonStatus(AddOns* Target, bool* Allowed)
{
    //sLog.outDebug("Get Addon Status");
    uint8 state = 0;
    for(std::list<AddOns*>::iterator i = m_Addon_data.begin();i!=m_Addon_data.end();++i)
    {
        if (!strcmp((*i)->Name.c_str(), Target->Name.c_str()))
        {
            *Allowed = (*i)->Enabled;
            return false;
        }
    }
    return true;
}

void AddonHandler::BuildAddonPacket(WorldPacket* Source, WorldPacket* Target, uint32 Packetoffset)
{
    ByteBuffer AddOnPacked;
    uLongf AddonRealSize;
    uint32 CurrentPosition;
    uint32 TempValue;

    *Source >> TempValue;                                   //get real size of the packed structure

    AddonRealSize = TempValue;                              //temp value becouse ZLIB only excepts uLongf

    CurrentPosition = Source->rpos();                       //get the position of the pointer in the structure

    AddOnPacked.resize(AddonRealSize);                      //resize target for zlib action

    if (!uncompress((uint8*)AddOnPacked.contents(), &AddonRealSize, (uint8*)(*Source).contents() + CurrentPosition, (*Source).size() - CurrentPosition)!= Z_OK)
    {
        bool* AddonAllowed = new bool;                      //handle addon check and enable-ing

        uint32 NumberofAddons = 0;
        uint32 Unknown1;
        uint8 Unknown0;

        AddOnPacked >> Unknown0;
        AddOnPacked >> Unknown1;

        Target->Initialize(SMSG_ADDON_INFO);

        uint32 i = 5;                                       //offset for addon extraction
        while(i != AddOnPacked.size())
        {
            std::string AddonNames;
            AddOns* Addonstr = new AddOns;
            uint8 unk6;
            uint64 CRCCHECK;
            AddOnPacked >> AddonNames >> CRCCHECK >> unk6;

            //sLog.outDebug("ADDON:    Name:%s CRC:%x Unknown:%x",AddonNames.c_str(), CRCCHECK,unk6);

            Addonstr->Name = AddonNames;
            Addonstr->CRC = CRCCHECK;

            //if not allowed but unknown added to list
            if (GetAddonStatus(Addonstr, AddonAllowed))     // If addon is new
            {
                Addonstr->Enabled = m_Addon_Default;        // by default new addons are set from Config file
                *AddonAllowed = m_Addon_Default;            // Set addon allowed on default value
                _AddAddon(Addonstr);
                sLog.outString("Found new Addon, Name:%s CRC:%x Unknown:%x",AddonNames.c_str(), CRCCHECK, unk6);
            }

            if (CRCCHECK == 0x4C1C776D01LL)                 //If addon is Standard addon CRC
            {
                                                            //value's standard Addons
                *Target << uint8(0) << uint8(2) << uint8(1) << uint8(0) << uint32(0);
            }
            else if (*AddonAllowed)                         //if addon is Custom addons
                                                            //value's enable addon
                *Target << uint8(0x00) << uint8(0x01) << uint8(0x00) << uint8(0x01);
            else
                                                            //value's disable addom
                *Target << uint8(0x00) << uint8(0x0) << uint8(0x00) << uint8(0x0);

            i += AddonNames.size() + 10;
        }
        *Target << uint8(0x0);

        //delete mem allocation
        delete AddonAllowed;
    }
    else
    {
        //handle uncompress error
    }
}
