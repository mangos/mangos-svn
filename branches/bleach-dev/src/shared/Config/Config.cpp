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

#include "Config/ConfigEnv.h"

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Singleton<Config, ACE_Recursive_Thread_Mutex>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Singleton<Config, ACE_Recursive_Thread_Mutex>
#elif defined (__GNUC__) && (defined (_AIX) || defined (__hpux))
template ACE_Singleton<Config, ACE_Recursive_Thread_Mutex> * ACE_Singleton<Config, ACE_Recursive_Thread_Mutex>::singleton_;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

Config::Config() : mConf(0)
{
}


Config::~Config()
{
    if (mConf)
        delete mConf;
}


bool Config::SetSource(const ACE_TCHAR *file, bool ignorecase)
{
    mConf = new DOTCONFDocument(ignorecase ?
        DOTCONFDocument::CASEINSENSETIVE :
    DOTCONFDocument::CASESENSETIVE);

    if (mConf->setContent(file) == -1)
    {
        delete mConf;
        mConf = 0;
        return false;
    }

    return true;
}


bool Config::GetString(const ACE_TCHAR *name, std::string *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

	*value = node->getValue();

    return true;
}

std::string Config::GetStringDefault(const ACE_TCHAR *name, const ACE_TCHAR *def)
{
    if(!mConf)
        return std::string(def);

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return std::string(def);

    return std::string(node->getValue());
}

bool Config::GetBool(const ACE_TCHAR *name, bool *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    const ACE_TCHAR* str = node->getValue();
    if(strcmp(str, "true") == 0 || strcmp(str, "TRUE") == 0 ||
        strcmp(str, "yes") == 0 || strcmp(str, "YES") == 0 ||
        strcmp(str, "1") == 0)
    {
        *value = true;
    }
    else
        *value = false;

    return true;
}


bool Config::GetBoolDefault(const ACE_TCHAR* name, const bool def)
{
    bool val;
    return GetBool(name, &val) ? val : def;
}


bool Config::GetInt(const ACE_TCHAR* name, int *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    *value = atoi(node->getValue());

    return true;
}


bool Config::GetFloat(const ACE_TCHAR* name, float *value)
{
    if(!mConf)
        return false;

    DOTCONFDocumentNode *node = (DOTCONFDocumentNode *)mConf->findNode(name);
    if(!node || !node->getValue())
        return false;

    *value = atof(node->getValue());

    return true;
}


int Config::GetIntDefault(const ACE_TCHAR* name, const int def)
{
    int val;
    return GetInt(name, &val) ? val : def;
}


float Config::GetFloatDefault(const ACE_TCHAR* name, const float def)
{
    float val;
    return (GetFloat(name, &val) ? val : def);
}
