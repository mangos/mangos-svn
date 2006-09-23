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

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Singleton<Configuration, ACE_Recursive_Thread_Mutex>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Singleton<Configuration, ACE_Recursive_Thread_Mutex>
#elif defined (__GNUC__) && (defined (_AIX) || defined (__hpux))
template ACE_Singleton<Configuration, ACE_Recursive_Thread_Mutex> * ACE_Singleton<Configuration, ACE_Recursive_Thread_Mutex>::singleton_;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

#include "Configuration.h"
#include <ace/Configuration_Import_Export.h>
#include <ace/OS_NS_ctype.h>
#include <ace/OS_NS_errno.h>
#include <ace/OS_NS_stdio.h>
#include <ace/OS_NS_string.h>
#include <ace/OS_NS_unistd.h>

Configuration::Configuration()
{

}

Configuration::~Configuration()
{

}


int
Configuration::SetSource(const char *file)
{
	int status;
	ACE_Configuration_Heap cf;
	if ((status = cf.open ()) != 0)
		ACE_ERROR ((LM_ERROR, ACE_TEXT ("ACE_Configuration_Heap::open returned %d\n"), status));
	
	ACE_Ini_ImpExp import (cf);
    status = import.import_config (file);
	if (status != 0)
	{
		ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("Config_Test_Import_1.ini failed")));
    }
    else
	{
		ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("mangosd.conf imported\n")));
		int mangosd_seen = 0, section2_seen = 0;
		int version_seen = 0, realmdid_seen = 0;
		int gametype_seen = 0, datadir_seen = 0;
		int index;
		ACE_TString sect_name;
		const ACE_Configuration_Section_Key &root = cf.root_section ();
		for (index = 0; (status = cf.enumerate_sections (root, index, sect_name)) == 0; ++index)
		{
			if (index > 9)
				ACE_ERROR ((LM_ERROR, ACE_TEXT ("Enumerated %d sections; expected 9\n"), index + 1));
			else
			{
				ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Enumerated to section %s\n"), sect_name.c_str ()));
				if (sect_name == ACE_TEXT ("mangos"))
				{
					if (mangosd_seen)
						ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %s multiple times!\n"), sect_name.c_str ()));
					mangosd_seen = 1;
					ACE_Configuration_Section_Key sect1;
					if (cf.open_section (root, sect_name.c_str (), 0, sect1) != 0)
						ACE_ERROR ((LM_ERROR, ACE_TEXT ("Failed to open section: %s\n"), sect_name.c_str ()));
					else
					{
						int val_index = 0, val_status;
						ACE_TString val_name, value;
						ACE_Configuration::VALUETYPE val_type;
						while ((val_status = cf.enumerate_values (sect1, val_index, val_name, val_type)) == 0)
						{
							ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Enumerated %s, type %d\n"), val_name.c_str (), val_type));

							if ( val_name == ACE_TEXT("version"))
							{
								if (version_seen)
									ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %s more than once\n"), val_name.c_str ()));
								else
								{
									this->GetInt(cf, sect1, val_name.c_str(), &m_version, -1);
									if ( m_version == -1)
									{
										ACE_ERROR ((LM_ERROR, ACE_TEXT ("wrong m_version value\n")));
									}
								}
								version_seen = 1;
							}
							else if (val_name == ACE_TEXT ("realmdid"))
							{
								if (realmdid_seen)
									ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %s more than once\n"), val_name.c_str ()));
								else
								{
									this->GetInt(cf, sect1, val_name.c_str(), &m_realmid, -1);
									if ( m_realmid == -1)
									{
										ACE_ERROR ((LM_ERROR, ACE_TEXT ("wrong m_realmid value\n")));
									}
								}
								realmdid_seen = 1;
							}
							else if (val_name == ACE_TEXT ("gametype"))
							{
								if (gametype_seen)
									ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %s more than once\n"), val_name.c_str ()));
								else
								{
									this->GetInt(cf, sect1, val_name.c_str(), &m_gametype, -1);
									if ( m_gametype == -1)
									{
										ACE_ERROR ((LM_ERROR, ACE_TEXT ("wrong m_gametype value\n")));
									}
								}
								gametype_seen = 1;
							}
							else if (val_name == ACE_TEXT ("datadir"))
							{
								if (datadir_seen)
									ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %s more than once\n"), val_name.c_str ()));
								else
								{
								}
								datadir_seen = 1;
							}
							//ACE_ERROR ((LM_ERROR, ACE_TEXT ("Unexpected key %s\n"), val_name.c_str ()));

							/*if ((val_status = cf.get_string_value (sect1, val_name.c_str (), value)) != 0) 
								ACE_ERROR ((LM_ERROR, ACE_TEXT ("Can't get value of %s\n"), val_name.c_str ()));
							else
							{
								ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("%s value: %s\n"), val_name.c_str (), value.c_str ()));
								if (value != ACE_TEXT ("SomeValue")) 
								{ 
									ACE_ERROR ((LM_ERROR, ACE_TEXT ("SomeKey: %s; expected SomeValue\n")));
								}
							}*/
							++val_index;
						}
						if (val_status == 1)
						{
							if (val_index != 4)
								ACE_ERROR ((LM_ERROR, ACE_TEXT ("Expected 4 values; saw %d\n"), index));
						}
						else
							ACE_ERROR ((LM_ERROR, ACE_TEXT ("Error enumerating %s; status %d\n"), sect_name.c_str (), val_status));
					}
				}
				else
				{
				}
			}
		}
		if (status == 1)
		{
			if (index != 9)
				ACE_ERROR ((LM_ERROR, ACE_TEXT ("Saw %d sections; expected 9\n"), index));
		}
		else
			ACE_ERROR ((LM_ERROR, ACE_TEXT ("Error enumerating sections; status %d\n"), status));
	}

	ACE_DEBUG ((LM_DEBUG, "Configuration\n"));
	ACE_DEBUG ((LM_DEBUG, "[mangos]\n"));
	ACE_DEBUG ((LM_DEBUG, "version = %d\n", m_version));
	ACE_DEBUG ((LM_DEBUG, "realmid = %d\n", m_realmid));
	ACE_DEBUG ((LM_DEBUG, "gametype = %d\n", m_realmid));
	return 1;
}

/*
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
*/
// Reads a string value from a configuration object.
/*
void
Config_Test::get_section_string (ACE_Configuration&             config,
                                 ACE_Configuration_Section_Key& SectionKey,
                                 const ACE_TCHAR*               pszName,
                                 ACE_TCHAR*                     pszVariable,
                                 int                            nMaxLength)
{
  ACE_TString StringValue;

  if (config.get_string_value (SectionKey,
                               pszName,
                               StringValue) == 0)
    {
      ACE_OS::strncpy (pszVariable,
                       StringValue.c_str (),
                       nMaxLength);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s = %s\n"),
                  pszName,
                  pszVariable));
    }
}
*/

// Reads an integer value from a configuration object (when it's
// stored as a string)

void
Configuration::GetInt(ACE_Configuration& config, ACE_Configuration_Section_Key& SectionKey,
	const ACE_TCHAR* pszName, int* nVariable, int nDefault)
{
	ACE_TString StringValue;
	ACE_TCHAR pszString[30];
	ACE_OS::strcpy (pszString, ACE_TEXT ("0"));
	int IntegerValue = 0;
	if (config.get_string_value (SectionKey, pszName, StringValue) == 0)
	{
		ACE_OS::strncpy (pszString, StringValue.c_str (), 30);
		ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("%s = %s\n"), pszName, pszString));
		IntegerValue = ACE_OS::atoi (pszString);
		*nVariable = IntegerValue;
    }
	else
	{
		*nVariable = nDefault;
	}
}

// Reads a boolean value from a configuration object (when it's stored as a string).
/*
void
Config_Test::get_section_boolean (ACE_Configuration&             config,
                                  ACE_Configuration_Section_Key& SectionKey,
                                  const ACE_TCHAR*               pszName,
                                  int*                           pVariable)
{
  ACE_TString StringValue;
  ACE_TCHAR pszString[10];
  ACE_OS::strcpy (pszString, ACE_TEXT ("0"));

  if (config.get_string_value (SectionKey,
                               pszName,
                               StringValue) == 0)
    {
      ACE_OS::strncpy (pszString,
                       StringValue.c_str (),
                       10);
      for (ACE_TCHAR* pSrc = pszString;
           *pSrc != ACE_TEXT ('\0');
           pSrc++)
        // Convert to uppercase
        if (ACE_OS::ace_islower (*pSrc))
          *pSrc = ACE_OS::ace_tolower (*pSrc);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s = %s\n"),
                  pszName,
                  pszString));

      if (ACE_OS::strcmp (pszString,
                          ACE_TEXT ("TRUE")) == 0)
        *pVariable = 1;
      else if (ACE_OS::strcmp (pszString,
                               ACE_TEXT ("FALSE")) == 0)
        *pVariable = 0;
    }
}
*/
