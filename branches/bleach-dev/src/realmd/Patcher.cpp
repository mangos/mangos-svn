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
#include "Patcher.h"
#include "SystemConfig.h"

#include <ace/OS.h>


#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Singleton<Patcher, ACE_Recursive_Thread_Mutex>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Singleton<Patcher, ACE_Recursive_Thread_Mutex>
#elif defined (__GNUC__) && (defined (_AIX) || defined (__hpux))
template ACE_Singleton<Patcher, ACE_Recursive_Thread_Mutex> * ACE_Singleton<Patcher, ACE_Recursive_Thread_Mutex>::singleton_;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

Patcher::Patcher()
{
    LoadPatchesInfo();
}

Patcher::~Patcher()
{
    for(Patches::iterator i = _patches.begin(); i != _patches.end(); i++ )
        delete i->second;
}

#ifndef _WIN32
#include <sys/dir.h>
#include <errno.h>
void Patcher ::LoadPatchesInfo()
{
    DIR * dirp;
    //int errno;
    struct dirent * dp;
    dirp = opendir("./patches/");
    if(!dirp)
        return;
    while (dirp)
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            int l=strlen(dp->d_name);
            if(l<8)continue;
            if(!memcmp(&dp->d_name[l-4],".mpq",4))
                LoadPatchMD5(dp->d_name);
        }
        else
        {
            if(errno != 0)
            {
                closedir(dirp);
                return;
            }
            break;
        }
    }

    if(dirp)
        closedir(dirp);
}

#else

void Patcher :: LoadPatchesInfo()
{
    WIN32_FIND_DATA fil;
    HANDLE hFil=FindFirstFile("./patches/*.mpq",&fil);
    if(hFil==INVALID_HANDLE_VALUE)return;                   //no pathces were found
    LoadPatchMD5(fil.cFileName);

    while(FindNextFile(hFil,&fil))
        LoadPatchMD5(fil.cFileName);
}
#endif

void Patcher::LoadPatchMD5(char * szFileName)
{
    char path[128]="./patches/";
    strcat(path,szFileName);
    FILE * pPatch=fopen(path,"rb");
    printf("\nLoading patch info from %s\n",path);
    if(!pPatch)
    {
        printf("Error loading patch.");
        return;
    }
    MD5_CTX ctx;
    MD5_Init(&ctx);
    uint8 * buf= new uint8 [512*1024];

    while (!feof(pPatch))
    {
        size_t read = fread(buf, 1, 512*1024, pPatch);
        MD5_Update(&ctx, buf, read);
    }
    delete [] buf;
    fclose(pPatch);
    _patches[ path] = new PATCH_INFO;
    MD5_Final((uint8 *)&_patches[path]->md5 , &ctx);
}

bool Patcher::GetHash(char * pat,uint8 mymd5[16])
{
    for( Patches::iterator i = _patches.begin(); i != _patches.end(); i++ )
        if(!stricmp(pat,i->first.c_str () ))
    {
        ACE_OS::memcpy(mymd5,i->second->md5,16);
        return true;
    }

    return false;
}
