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

#ifndef _DOTCONFIG_H
#define _DOTCONFIG_H

#include <ace/Singleton.h>

class Config 
{
    public:
        Config();
        ~Config();

        bool SetSource(const ACE_TCHAR *file, bool ignorecase = true);

        bool GetString(const ACE_TCHAR* name, std::string *value);
        std::string GetStringDefault(const ACE_TCHAR* name, const ACE_TCHAR* def);

        bool GetBool(const ACE_TCHAR* name, bool *value);
        bool GetBoolDefault(const ACE_TCHAR* name, const bool def = false);

        bool GetInt(const ACE_TCHAR* name, int *value);
        int GetIntDefault(const ACE_TCHAR* name, const int def);

        bool GetFloat(const ACE_TCHAR* name, float *value);
        float GetFloatDefault(const ACE_TCHAR* name, const float def);

    private:
        DOTCONFDocument *mConf;
};

typedef ACE_Singleton<Config, ACE_Recursive_Thread_Mutex> ConfigSingleton;
#define sConfig ConfigSingleton::instance()

#endif
