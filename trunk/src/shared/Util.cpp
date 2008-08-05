/* 
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
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

#include "Util.h"

#include "sockets/socket_include.h"
#include "utf8cpp/utf8.h"
#include "mersennetwister/MersenneTwister.h"
#include "zthread/ThreadLocal.h"

typedef ZThread::ThreadLocal<MTRand> MTRandTSS;

/* NOTE: Not sure if static initialization is ok for TSS objects ,
 * as I see zthread uses custom implementation of the TSS 
 * ,and in the consturctor there is no code ,so I suppose its ok
 * If its not ok ,change it to use singleton.
 */
static MTRandTSS mtRand;

int32 irand (int32 min, int32 max)
{
  return int32 (mtRand.get ().randInt (max - min)) + min;
}

uint32 urand (uint32 min, uint32 max)
{
  return mtRand.get ().randInt (max - min) + min;
}

int32 rand32 ()
{
  return mtRand.get ().randInt ();
}

double rand_norm(void)
{
  return mtRand.get ().randExc ();
}

double rand_chance (void)
{
  return mtRand.get ().randExc (100.0);
}

Tokens StrSplit(const std::string &src, const std::string &sep)
{
    Tokens r;
    std::string s;
    for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            if (s.length()) r.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (s.length()) r.push_back(s);
    return r;
}

void stripLineInvisibleChars(std::string &str)
{
    static std::string invChars = " \t\7";

    size_t wpos = 0;

    bool space = false;
    for(size_t pos = 0; pos < str.size(); ++pos)
    {
        if(invChars.find(str[pos])!=std::string::npos)
        {
            if(!space)
            {
                str[wpos++] = ' ';
                space = true;
            }
        }
        else
        {
            if(wpos!=pos)
                str[wpos++] = str[pos];
            else
                ++wpos;
            space = false;
        }
    }

    if(wpos < str.size())
        str.erase(wpos,str.size());
}

std::string secsToTimeString(uint32 timeInSecs, bool shortText, bool hoursOnly)
{
    uint32 secs    = timeInSecs % MINUTE;
    uint32 minutes = timeInSecs % HOUR / MINUTE;
    uint32 hours   = timeInSecs % DAY  / HOUR;
    uint32 days    = timeInSecs / DAY;

    std::ostringstream ss;
    if(days)
        ss << days << (shortText ? "d" : " Day(s) ");
    if(hours || hoursOnly)
        ss << hours << (shortText ? "h" : " Hour(s) ");
    if(!hoursOnly)
    {
        if(minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if(secs || !days && !hours && !minutes)
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

uint32 TimeStringToSecs(std::string timestring)
{
    uint32 secs       = 0;
    uint32 buffer     = 0;
    uint32 multiplier = 0;

    for(std::string::iterator itr = timestring.begin(); itr != timestring.end(); itr++ )
    {
        if(isdigit(*itr))
        {
            std::string str;                                //very complicated typecast char->const char*; is there no better way?
            str += *itr;
            const char* tmp = str.c_str();

            buffer*=10;
            buffer+=atoi(tmp);
        }
        else
        {
            switch(*itr)
            {
                case 'd': multiplier = DAY;     break;
                case 'h': multiplier = HOUR;    break;
                case 'm': multiplier = MINUTE;  break;
                case 's': multiplier = 1;       break;
                default : return 0;                         //bad format
            }
            buffer*=multiplier;
            secs+=buffer;
            buffer=0;
        }
    }

    return secs;
}

std::string TimeToTimestampStr(time_t t)
{
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf,20,"%04d-%02d-%02d_%02d-%02d-%02d",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
    return std::string(buf);
}

/// Check if the string is a valid ip address representation
bool IsIPAddress(char const* ipaddress)
{
    if(!ipaddress)
        return false;

    // Let the big boys do it.
    // Drawback: all valid ip address formats are recognized e.g.: 12.23,121234,0xABCD)
    return inet_addr(ipaddress) != INADDR_NONE;
}

/// create PID file
uint32 CreatePIDFile(std::string filename)
{
    FILE * pid_file = fopen (filename.c_str(), "w" );
    if (pid_file == NULL)
        return 0;

#ifdef WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    fprintf(pid_file, "%d", pid );
    fclose(pid_file);

    return (uint32)pid;
}

size_t utf8length(std::string& utf8str)
{
    try
    {
        return utf8::distance(utf8str.c_str(),utf8str.c_str()+utf8str.size());
    }
    catch(std::exception)
    {
        utf8str = "";
        return 0;
    }
}

bool Utf8toWStr(std::string utf8str, std::wstring& wstr)
{
    try
    {
        size_t len = utf8::distance(utf8str.c_str(),utf8str.c_str()+utf8str.size());
        wstr.resize(len);

        utf8::utf8to16(utf8str.c_str(),utf8str.c_str()+utf8str.size(),&wstr[0]);
    }
    catch(std::exception)
    {
        wstr = L"";
        return false;
    }

    return true;
}

bool WStrToUtf8(std::wstring wstr, std::string& utf8str)
{
    try
    {
        utf8str.resize(wstr.size()*2);                              // allocate for most long case

        char* oend = utf8::utf16to8(wstr.c_str(),wstr.c_str()+wstr.size(),&utf8str[0]);
        utf8str.resize(oend-(&utf8str[0]));                             // remove unused tail
    }
    catch(std::exception)
    {
        utf8str = "";
        return false;
    }

    return true;
}

typedef wchar_t const* const* wstrlist;

std::wstring GetMainPartOfName(std::wstring wname, uint32 declension)
{
    // supported only Cyrillic cases
    if(wname.size() < 1 || !isCyrillicCharacter(wname[0]) || declension > 5)
        return wname;

    static wchar_t const aEnd[]    = { wchar_t(1), wchar_t(0x0430),wchar_t(0x0000)};
    static wchar_t const oEnd[]    = { wchar_t(1), wchar_t(0x043E),wchar_t(0x0000)};
    static wchar_t const yaEnd[]   = { wchar_t(1), wchar_t(0x044F),wchar_t(0x0000)};
    static wchar_t const ieEnd[]   = { wchar_t(1), wchar_t(0x0435),wchar_t(0x0000)};
    static wchar_t const iEnd[]    = { wchar_t(1), wchar_t(0x0438),wchar_t(0x0000)};
    static wchar_t const yeruEnd[] = { wchar_t(1), wchar_t(0x044B),wchar_t(0x0000)};
    static wchar_t const uEnd[]    = { wchar_t(1), wchar_t(0x0443),wchar_t(0x0000)};
    static wchar_t const yuEnd[]   = { wchar_t(1), wchar_t(0x044E),wchar_t(0x0000)};
    static wchar_t const ojEnd[]   = { wchar_t(2), wchar_t(0x043E),wchar_t(0x0439),wchar_t(0x0000)};
    static wchar_t const iejEnd[]  = { wchar_t(2), wchar_t(0x0435),wchar_t(0x0439),wchar_t(0x0000)};
    static wchar_t const iojEnd[]  = { wchar_t(2), wchar_t(0x0451),wchar_t(0x0439),wchar_t(0x0000)};
    static wchar_t const omEnd[]   = { wchar_t(2), wchar_t(0x043E),wchar_t(0x043C),wchar_t(0x0000)};
    static wchar_t const iomEnd[]  = { wchar_t(2), wchar_t(0x0451),wchar_t(0x043C),wchar_t(0x0000)};
    static wchar_t const softEnd[] = { wchar_t(1), wchar_t(0x044C),wchar_t(0x0000)};

    static wchar_t const* const dropEnds[6][7] = {
        { &aEnd[1], &oEnd[1],  &yaEnd[1],  &ieEnd[1], &softEnd[1], NULL,      NULL },
        { &aEnd[1], &yaEnd[1], &yeruEnd[1],&iEnd[1],  NULL,        NULL,      NULL },
        { &ieEnd[1],&uEnd[1],  &yuEnd[1],  &iEnd[1],  NULL,        NULL,      NULL },
        { &uEnd[1], &yuEnd[1], &oEnd[1],   &ieEnd[1], &softEnd[1], NULL,      NULL },
        { &ojEnd[1],&iojEnd[1],&iejEnd[1], &omEnd[1], &iomEnd[1],  &yuEnd[1], NULL },
        { &ieEnd[1],&iEnd[1],  NULL,       NULL,      NULL,        NULL,      NULL }
    };

    for(wchar_t const * const* itr = &dropEnds[declension][0]; *itr; ++itr)
    {
        size_t len = size_t((*itr)[-1]);                    // get length from string size field

        if(wname.substr(wname.size()-len,len)==*itr)
            return wname.substr(0,wname.size()-len);
    }

    return wname;
}
