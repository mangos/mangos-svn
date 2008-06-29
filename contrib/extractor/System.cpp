#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <deque>
#include <set>
#include "direct.h"

#include "dbcfile.h"
#include "mpq_libmpq.h"

extern unsigned int iRes;
extern ArchiveSet gOpenArchives;

bool ConvertADT(char*,char*);

typedef struct{
    char name[64];
    unsigned int id;
}map_id;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
map_id * map_ids;
uint16 * areas;
char output_path[128]=".";
char input_path[128]=".";
uint32 map_count;

static char* const langs[]={"deDE", "enUS", "enGB", "frFR", "esES", "zhCN", "zhTW" };
#define LANG_COUNT 7

void CreateDir( const std::string& Path )
{
    #ifdef WIN32
     _mkdir( Path.c_str());
    #else
    mkdir( Path.c_str(), 0777 );
    #endif
}

bool FileExists( const char* FileName )
{
    if( FILE* fp = fopen( FileName, "rb" ) )
    {
        fclose( fp );
        return true;
    }

    return false;
}

void Usage(char* prg)
{
    printf("Usage:\n%s -[var] [value]\n-i set input path\n-o set output path\n-r set resolution\nExample: %s -r 256 -i \"c:\\games\\game\"",
    prg,prg);
    exit(1);
}

void HandleArgs(int argc, char * arg[])
{
    for(int c=1;c<argc;c++)
    {
        //i - input path
        //o - output path
        //r - resolution, array of (r * r) heights will be created 
        if(arg[c][0] != '-')
            Usage(arg[0]);

        switch(arg[c][1])
        {
            case 'i':
                if(c+1<argc)//all ok
                    strcpy(input_path,arg[(c++) +1]);
                else
                    Usage(arg[0]);
                break;
            case 'o':
                if(c+1<argc)//all ok
                    strcpy(output_path,arg[(c++) +1]);
                else
                    Usage(arg[0]);
                break;
            case 'r':
                if(c+1<argc)//all ok
                    iRes=atoi(arg[(c++) +1]);
                else
                    Usage(arg[0]);
                break;
        }
    }
}

void ExtractMapsFromMpq()
{
    char mpq_filename[1024];
    char output_filename[1024];

    unsigned int total=map_count*64*64;
    unsigned int done=0;
    
    std::string path = output_path;
    path += "/maps/";
    CreateDir(path);

    for(unsigned int x=0;x<64;x++)
    {
        for(unsigned int z=0;z<map_count;z++)
        {
            for(unsigned int y=0;y<64;y++)
            {
                sprintf(mpq_filename,"World\\Maps\\%s\\%s_%u_%u.adt",map_ids[z].name,map_ids[z].name,x,y);
                sprintf(output_filename,"%s/maps/%03u%02u%02u.map",output_path,map_ids[z].id,y,x);
                ConvertADT(mpq_filename,output_filename);
                done++;
            }
            //draw progess bar
            printf("Processing........................%d%%\r",(100*done)/total);
        }
    }
}

//bool WMO(char* filename);

void ReadMapDBC()
{
    DBCFile dbc("DBFilesClient\\Map.dbc");
    dbc.open();

    map_count=dbc.getRecordCount();
    map_ids=new map_id[map_count];
    for(unsigned int x=0;x<map_count;x++)
    {
        map_ids[x].id=dbc.getRecord(x).getUInt(0);
        strcpy(map_ids[x].name,dbc.getRecord(x).getString(1));
    }
}

void ReadAreaTableDBC()
{
    DBCFile dbc("DBFilesClient\\AreaTable.dbc");
    dbc.open();

    unsigned int area_count=dbc.getRecordCount();
    uint32 maxi=0;
    for(unsigned int x=0;x<area_count;x++)
    {
        if(maxi<dbc.getRecord(x).getUInt(0))
            maxi=dbc.getRecord(x).getUInt(0);
            //printf("\n%d %d",dbc->getRecord(x).getUInt(0),dbc->getRecord(x).getUInt(3));
    }
    maxi++;//not needed actually
    areas=new uint16[maxi];
    memset(areas,0xff,maxi*2);
    for(unsigned int x=0;x<area_count;x++)
        areas[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);
}

void ExtractDBCFiles()
{
    printf("Extracting dbc files...\n");
    
    set<string> dbcfiles;

    // get DBC file list
    for(ArchiveSet::iterator i = gOpenArchives.begin(); i != gOpenArchives.end();++i)
    {
        vector<string> files = (*i)->GetFileList();
        for (vector<string>::iterator iter = files.begin(); iter != files.end(); ++iter) 
            if (iter->rfind(".dbc") == iter->length() - strlen(".dbc"))
                    dbcfiles.insert(*iter);
    }

    std::string path = output_path;
    path += "/dbc/";
    CreateDir(path);
    
    // extract DBCs
    int count = 0;
    for (set<string>::iterator iter = dbcfiles.begin(); iter != dbcfiles.end(); ++iter) 
    {
        string filename = output_path;
        filename += "/dbc/";
        filename += (iter->c_str() + strlen("DBFilesClient\\"));

        //cout << filename << endl;

        FILE *output=fopen(filename.c_str(),"wb");
        if(!output)
        {
            printf("Can't create the output file '%s'\n",filename.c_str());
            continue;
        }
        MPQFile m(iter->c_str());
        if(!m.isEof())
            fwrite(m.getPointer(),1,m.getSize(),output);

        fclose(output);
        ++count;
    }
    printf("Extracted %u DBC files\n", count);
}

int GetLocale()
{
    for (int i = 0; i < LANG_COUNT; i++)
    {
        char tmp1[512];
        sprintf(tmp1, "%s/Data/%s/locale-%s.MPQ", input_path, langs[i], langs[i]);
        if (FileExists(tmp1))
        {
            printf("Detected locale: %s\n", langs[i]);
            return i;
        }
    }

    printf("Could not detect locale.\n");
    return -1;
}

void LoadMPQFiles(int const locale)
{
    char filename[512];

    sprintf(filename,"%s/Data/%s/locale-%s.MPQ",input_path,langs[locale],langs[locale]);
    new MPQArchive(filename);

    for(int i = 1; i < 5; ++i)
    {
        char ext[2] = "";
        if(i > 1)
            sprintf(ext, "-%i", i);

        sprintf(filename,"%s/Data/%s/patch-%s%s.MPQ",input_path,langs[locale],langs[locale],ext);
        if(!FileExists(filename))
            break;
        new MPQArchive(filename);
    }
    
    sprintf(filename,"%s/Data/common.MPQ",input_path);
    new MPQArchive(filename);
    sprintf(filename,"%s/Data/expansion.MPQ",input_path);
    new MPQArchive(filename);

    for(int i = 1; i < 5; ++i)
    {
        char ext[2] = "";
        if(i > 1)
            sprintf(ext, "-%i", i);

        sprintf(filename,"%s/Data/patch%s.MPQ",input_path,ext);
        if(!FileExists(filename))
            break;
        new MPQArchive(filename);
    }
}

int main(int argc, char * arg[])
{
    HandleArgs(argc, arg);

    int const locale = GetLocale();
    if(locale < 0)
        return 1;

    LoadMPQFiles(locale);

    ReadMapDBC();
    ReadAreaTableDBC();

    ExtractMapsFromMpq();

    delete [] areas;
    delete [] map_ids;

    ExtractDBCFiles();

    return 0;
}
