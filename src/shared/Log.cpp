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

#include "Common.h"
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "Config/ConfigEnv.h"

#include <stdarg.h>

INSTANTIATE_SINGLETON_1( Log );

enum LogType
{
    LogNormal = 0,
    LogDetails,
    LogDebug,
    LogError
};

const int LogType_count = int(LogError) +1;

void Log::InitColors(std::string str)
{
    if(str=="")
    {
        m_colored = false;
        return;
    }

    int color[4];

    std::istringstream ss(str);

    for(int i = 0; i < LogType_count; ++i)
    {
        ss >> color[i];

        if(!ss)
            return;

        if(color[i] < 0 || color[i] >= Color_count)
            return;
    }

    for(int i = 0; i < LogType_count; ++i)
        m_colors[i] = Color(color[i]);

    m_colored = true;
}

void Log::SetColor(bool stdout_stream, Color color)
{
    #if PLATFORM == PLATFORM_WIN32

    static WORD WinColorFG[Color_count] =
    {
        0,                                                  // BLACK
        FOREGROUND_RED,                                     // RED
        FOREGROUND_GREEN,                                   // GREEN
        FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
        FOREGROUND_BLUE,                                    // BLUE
        FOREGROUND_RED |                    FOREGROUND_BLUE,// MAGENTA
        FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,// WHITE
                                                            // YELLOW
        FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
                                                            // RED_BOLD
        FOREGROUND_RED |                                      FOREGROUND_INTENSITY,
                                                            // GREEN_BOLD
        FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
                                                            // MAGENTA_BOLD
        FOREGROUND_RED |                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
                                                            // CYAN_BOLD
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
                                                            // WHITE_BOLD
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
    #else

    enum ANSITextAttr
    {
        TA_NORMAL=0,
        TA_BOLD=1,
        TA_BLINK=5,
        TA_REVERSE=7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK=30, FG_RED,  FG_GREEN, FG_BROWN, FG_BLUE,
        FG_MAGENTA,  FG_CYAN, FG_WHITE, FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK=40, BG_RED,  BG_GREEN, BG_BROWN, BG_BLUE,
        BG_MAGENTA,  BG_CYAN, BG_WHITE
    };

    static uint8 UnixColorFG[Color_count] =
    {
        FG_BLACK,                                           // BLACK
        FG_RED,                                             // RED
        FG_GREEN,                                           // GREEN
        FG_BROWN,                                           // BROWN
        FG_BLUE,                                            // BLUE
        FG_MAGENTA,                                         // MAGENTA
        FG_CYAN,                                            // CYAN
        FG_WHITE,                                           // WHITE
        FG_YELLOW,                                          // YELLOW
        FG_RED,                                             // LRED
        FG_GREEN,                                           // LGREEN
        FG_BLUE,                                            // LBLUE
        FG_MAGENTA,                                         // LMAGENTA
        FG_CYAN,                                            // LCYAN
        FG_WHITE                                            // LWHITE
    };

    fprintf((stdout_stream? stdout : stderr), "\x1b[%d%sm",UnixColorFG[color],(color>=YELLOW&&color<Color_count ?";1":""));
    #endif
}

void Log::ResetColor(bool stdout_stream)
{
    #if PLATFORM == PLATFORM_WIN32
    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
    #else
    fprintf(( stdout_stream ? stdout : stderr ), "\x1b[0m");
    #endif
}

void Log::SetLogLevel(char *Level)
{
    int32 NewLevel =atoi((char*)Level);
    if ( NewLevel <0 )
        NewLevel = 0;
    m_logLevel = NewLevel;

    printf( "LogLevel is %u\n",m_logLevel );
}

void Log::SetLogFileLevel(char *Level)
{
    int32 NewLevel =atoi((char*)Level);
    if ( NewLevel <0 )
        NewLevel = 0;
    m_logFileLevel = NewLevel;

    printf( "LogFileLevel is %u\n",m_logFileLevel );
}

void Log::Initialize()
{
    std::string logsDir = sConfig.GetStringDefault("LogsDir","");

    if(!logsDir.empty())
    {
        if((logsDir.at(logsDir.length()-1)!='/') && (logsDir.at(logsDir.length()-1)!='\\'))
            logsDir.append("/");
    }

    std::string logfn=sConfig.GetStringDefault("LogFile", "");
    if(logfn!="")
    {
        if(sConfig.GetIntDefault("LogTimestamp",0))
        {
            std::string logTimestamp = sConfig.GetIntDefault("LogTimestamp",0) ? GetTimestampStr() : "";
            logTimestamp.insert(0,"_");
            size_t dot_pos = logfn.find_last_of(".");
            if(dot_pos!=logfn.npos)
                logfn.insert(dot_pos,logTimestamp);
            else
                logfn += logTimestamp;
        }

        logfile = fopen((logsDir+logfn).c_str(), "w");
    }

    std::string gmlogname = sConfig.GetStringDefault("GMLogFile", "");
    if(gmlogname!="")
    {
        gmLogfile = fopen((logsDir+gmlogname).c_str(), "a");
    }

    std::string dberlogname = sConfig.GetStringDefault("DBErrorLogFile", "");
    if(dberlogname!="")
    {
        dberLogfile = fopen((logsDir+dberlogname).c_str(), "a");
    }
    std::string ralogname = sConfig.GetStringDefault("RaLogFile", "");
    if(ralogname!="")
    {
        raLogfile = fopen((logsDir+ralogname).c_str(), "a");
    }
    m_logLevel     = sConfig.GetIntDefault("LogLevel", 0);
    m_logFileLevel = sConfig.GetIntDefault("LogFileLevel", 0);
    InitColors(sConfig.GetStringDefault("LogColors", ""));

    m_logFilter = 0;

    if(sConfig.GetIntDefault("LogFilter_TransportMoves", 0)!=0)
        m_logFilter |= LOG_FILTER_TRANSPORT_MOVES;
    if(sConfig.GetIntDefault("LogFilter_CreatureMoves", 0)!=0)
        m_logFilter |= LOG_FILTER_CREATURE_MOVES;
    if(sConfig.GetIntDefault("LogFilter_VisibilityChanges", 0)!=0)
        m_logFilter |= LOG_FILTER_VISIBILITY_CHANGES;
}

void Log::outTimestamp(FILE* file)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    fprintf(file,"%-4d-%02d-%02d %02d:%02d:%02d ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
}

std::string Log::GetTimestampStr() const
{
    time_t t = time(NULL);
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

void Log::outTitle( const char * str)
{
    if( !str ) return;

    if(m_colored)
        SetColor(true,WHITE);

    printf( str );

    if(m_colored)
        ResetColor(true);

    printf( "\n" );
    if(logfile)
    {
        fprintf(logfile, str);
        fprintf(logfile, "\n" );
        fflush(logfile);
    }

    fflush(stdout);
}

void Log::outString()
{
    printf( "\n" );
    if(logfile)
    {
        outTimestamp(logfile);
        fprintf(logfile, "\n" );
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outString( const char * str, ... )
{
    if( !str ) return;

    if(m_colored)
        SetColor(true,m_colors[LogNormal]);

    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);

    if(m_colored)
        ResetColor(true);

    printf( "\n" );
    if(logfile)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outError( const char * err, ... )
{
    if( !err ) return;

    if(m_colored)
        SetColor(false,m_colors[LogError]);

    va_list ap;
    va_start(ap, err);
    vfprintf( stderr, err, ap );
    va_end(ap);

    if(m_colored)
        ResetColor(false);

    fprintf( stderr, "\n" );
    if(logfile)
    {
        outTimestamp(logfile);
        fprintf(logfile, "ERROR:" );
        va_start(ap, err);
        vfprintf(logfile, err, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stderr);
}

void Log::outErrorDb( const char * err, ... )
{
    if( !err ) return;

    if(m_colored)
        SetColor(false,m_colors[LogError]);

    va_list ap;
    va_start(ap, err);
    vfprintf( stderr, err, ap );
    va_end(ap);

    if(m_colored)
        ResetColor(false);

    fprintf( stderr, "\n" );

    if(logfile)
    {
        outTimestamp(logfile);
        fprintf(logfile, "ERROR:" );
        va_start(ap, err);
        vfprintf(logfile, err, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }

    if(dberLogfile)
    {
        outTimestamp(dberLogfile);
        va_start(ap, err);
        vfprintf(dberLogfile, err, ap);
        fprintf(dberLogfile, "\n" );
        va_end(ap);
        fflush(dberLogfile);
    }
    fflush(stderr);
}

void Log::outBasic( const char * str, ... )
{
    if( !str ) return;
    va_list ap;

    if( m_logLevel > 0 )
    {
        if(m_colored)
            SetColor(true,m_colors[LogDetails]);

        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);

        if(m_colored)
            ResetColor(true);

        printf( "\n" );
    }

    if(logfile && m_logFileLevel > 0)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outDetail( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 1 )
    {

        if(m_colored)
            SetColor(true,m_colors[LogDetails]);

        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);

        if(m_colored)
            ResetColor(true);

        printf( "\n" );
    }
    if(logfile && m_logFileLevel > 1)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }

    fflush(stdout);
}

void Log::outDebugInLine( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 2 )
    {
        if(m_colored)
            SetColor(true,m_colors[LogDebug]);

        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);

        if(m_colored)
            ResetColor(true);
    }
    if(logfile && m_logFileLevel > 2)
    {
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        va_end(ap);
    }
}

void Log::outDebug( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 2 )
    {
        if(m_colored)
            SetColor(true,m_colors[LogDebug]);

        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);

        if(m_colored)
            ResetColor(true);

        printf( "\n" );
    }
    if(logfile && m_logFileLevel > 2)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void Log::outCommand( const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if( m_logLevel > 1 )
    {
        if(m_colored)
            SetColor(true,m_colors[LogDetails]);

        va_start(ap, str);
        vprintf( str, ap );
        va_end(ap);

        if(m_colored)
            ResetColor(true);

        printf( "\n" );
    }
    if(logfile && m_logFileLevel > 1)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    if(gmLogfile)
    {
        outTimestamp(gmLogfile);
        va_start(ap, str);
        vfprintf(gmLogfile, str, ap);
        fprintf(gmLogfile, "\n" );
        va_end(ap);
        fflush(gmLogfile);
    }
    fflush(stdout);
}

void Log::outMenu( const char * str, ... )
{
    if( !str ) return;

    SetColor(true,m_colors[LogNormal]);

    va_list ap;
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);

    ResetColor(true);

    if(logfile)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
    fflush(stdout);
}

void debug_log(const char * str, ...)
{
    if( !str ) return;

    char buf[100];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buf,100, str, ap);
    va_end(ap);

    MaNGOS::Singleton<Log>::Instance().outDebug(buf);
}

void error_log(const char * str, ...)
{
    if( !str ) return;

    char buf[100];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buf,100, str, ap);
    va_end(ap);

    MaNGOS::Singleton<Log>::Instance().outError(buf);
}

void Log::outRALog(    const char * str, ... )
{
    if( !str ) return;
    va_list ap;
    if (raLogfile)
    {
        outTimestamp(raLogfile);
        va_start(ap, str);
        vfprintf(raLogfile, str, ap);
        fprintf(raLogfile, "\n" );
        va_end(ap);
        fflush(raLogfile);
    }
     fflush(stdout);
}
