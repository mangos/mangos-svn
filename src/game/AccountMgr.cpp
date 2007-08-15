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

#include "AccountMgr.h"
#include "Database/DatabaseEnv.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Policies/SingletonImp.h"

#ifdef DO_POSTGRESQL
extern DatabasePostgre loginDatabase;
#else
extern DatabaseMysql loginDatabase;
#endif

INSTANTIATE_SINGLETON_1(AccountMgr);

AccountMgr::AccountMgr()
{}

AccountMgr::~AccountMgr()
{}

int AccountMgr::CreateAccount(std::string username, std::string password)
{
    if(username.length() > 16)
        return 1;   // username's too long

    loginDatabase.escape_string(username);
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `username`='%s'", username.c_str());
    if(result)
    {
        delete result;
        return 2;   // username does already exist
    }

    loginDatabase.escape_string(password);

    if(!loginDatabase.PExecute("INSERT INTO `account`(`username`,`I`,`joindate`) VALUES('%s',SHA1(CONCAT(UPPER('%s'),':',UPPER('%s'))),NOW())", username.c_str(), username.c_str(), password.c_str()))
        return -1;  // unexpected error
    loginDatabase.Execute("INSERT INTO `realmcharacters` (`realmid`, `acctid`, `numchars`) SELECT `realmlist`.`id`, `account`.`id`, 0 FROM `account`, `realmlist` WHERE `account`.`id` NOT IN (SELECT `acctid` FROM `realmcharacters`)");

    return 0;   // everything's fine
}

int AccountMgr::DeleteAccount(uint32 accid)
{
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `id`='%d'", accid);
    if(!result)
        return 1;   // account doesn't exist
    delete result;

    result = sDatabase.PQuery("SELECT `guid` FROM `character` WHERE `account`='%d'",accid);
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guidlo = fields[0].GetUInt32();
            uint64 guid = MAKE_GUID(guidlo, HIGHGUID_PLAYER);

            // kick if player currently
            if(Player* p = objmgr.GetPlayer(guid))
                p->GetSession()->KickPlayer();

            Player::DeleteFromDB(guid, accid);
        } while (result->NextRow());

        delete result;
    }

    sDatabase.BeginTransaction();

    bool res =  loginDatabase.PExecute("DELETE FROM `account` WHERE `id`='%d'", accid) &&
                loginDatabase.PExecute("DELETE FROM `realmcharacters` WHERE `acctid`='%d'", accid);

    sDatabase.CommitTransaction();

    if(!res)
        return -1;   // unexpected error;


    return 0;
}

int AccountMgr::ChangeUsername(uint32 accid, std::string new_uname, std::string new_passwd)
{
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `id`='%d'", accid);
    if(!result)
        return 1;   // account doesn't exist
    delete result;

    loginDatabase.escape_string(new_uname);
    loginDatabase.escape_string(new_passwd);
    if(!loginDatabase.PExecute("UPDATE `account` SET `username`='%s',`I`=SHA1(CONCAT(UPPER('%s'),':',UPPER('%s'))) WHERE `id`='%d'", new_uname.c_str(), new_uname.c_str(), new_passwd.c_str(), accid))
        return -1;  // unexpected error

    return 0;
}

int AccountMgr::ChangePassword(uint32 accid, std::string new_passwd)
{
    QueryResult *result = loginDatabase.PQuery("SELECT 1 FROM `account` WHERE `id`='%d'", accid);
    if(!result)
        return 1;   // account doesn't exist
    delete result;

    loginDatabase.escape_string(new_passwd);
    if(!loginDatabase.PExecute("UPDATE `account` SET `I`=SHA1(CONCAT(UPPER(`username`),':',UPPER('%s'))) WHERE `id`='%d'", new_passwd.c_str(), accid))
        return -1;  // unexpected error

    return 0;
}

uint32 AccountMgr::GetId(std::string username)
{
    loginDatabase.escape_string(username);

    QueryResult *result = loginDatabase.PQuery("SELECT `id` FROM `account` WHERE UPPER(`username`)=UPPER('%s')", username.c_str());
    if(!result)
        return 0;
    else
    {
        uint32 id = (*result)[0].GetUInt32();
        delete result;
        return id;
    }
}
