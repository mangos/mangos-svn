#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <vector>
#include "dbcfile.h"
#include "adt.h"
#include "mpq_libmpq.h"
//#include <io.h>
#include <fcntl.h>

extern unsigned int iRes;
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

typedef bool extr_map[64][64];
//extr_map * maps_extr;

void Usage(char* prg)
{
    printf("Usage:\n%s -[var] [value]\n-i set input path\n-o set output path\n-r set resolution\nExample: %s -r 256 -i \"c:\\games\\game\"",
    prg,prg);
    exit(1);
}

void ExtractMapsFromMpq()
{
    char mpq_filename[1024];
    char output_filename[1024];
    //  char tmp[256];
    //  sprintf(tmp,"%s/Data/%s",input_path,filename);

    //MPQArchive* p=new MPQArchive(tmp);
    //map_count=1;
    unsigned int total=map_count*64*64;
    unsigned int done=0;

    for(unsigned int x=0;x<64;x++)
    {
        for(unsigned int z=0;z<map_count;z++)
        {
            for(unsigned int y=0;y<64;y++)
            {
                sprintf(mpq_filename,"World\\Maps\\%s\\%s_%u_%u.adt",map_ids[z].name,map_ids[z].name,x,y);
                //maps_extr[z][x][y]=true;
                sprintf(output_filename,"%s/maps/%03u%02u%02u.map",output_path,map_ids[z].id,y,x);
                //maps_extr[z][x][y]=
                ConvertADT(mpq_filename,output_filename);
                done++;
            }
            //draw progess bar
            printf("Processing........................%d%%\r",(100*done)/total);
        }
    }
}

//bool WMO(char* filename);

int main(int argc, char * arg[])
{
    FILE* pDatei;
    char tmp[512];
    char tmp1[512];
    //char tmp2[512];
    char tmp3[512];
    char tmp4[512];
    for(int c=1;c<argc;c++)
    {
        //i - input path
        //o - output path
        //r - resolution, array of (r * r) heights will be created 
        if(arg[c][0]!='-')

        Usage(arg[0]);

        switch(arg[c][1])
        {
        case 'i':
            if(c+1<argc)//all ok
                strcpy(input_path,arg[(c++) +1]);
            else Usage(arg[0]);
            break;
        case 'o':
            if(c+1<argc)//all ok
                strcpy(output_path,arg[(c++) +1]);
            else Usage(arg[0]);
            break;
        case 'r':
            if(c+1<argc)//all ok
                iRes=atoi(arg[(c++) +1]);
            else Usage(arg[0]);
            break;
        }
    }

    std::vector<MPQArchive*> archives;

    char* LANG;
    LANG="ERro";

    char* langs[]={"deDE", "enUS", "enGB", "frFR", "esES", "zhCN", "zhTW" };
    for (size_t i = 0; i < 7; i++) {
        sprintf(tmp1, "%s/Data/%s/locale-%s.MPQ", input_path, langs[i], langs[i]);
        pDatei=fopen(tmp1, "rb");
        if (pDatei!=NULL) {
            LANG = langs[i];
            fclose(pDatei);
            break;
        }
    }

    if(LANG == "ERro") {
        printf("Error can't find the files in: %s/Data/\n", input_path);
        exit(1);
    }

    sprintf(tmp1,"%s/locale-%s.MPQ",LANG,LANG);
    //sprintf(tmp2,"%s/expansion-locale-%s.MPQ",LANG,LANG);
    sprintf(tmp3,"%s/patch-%s.MPQ",LANG,LANG);
    sprintf(tmp4,"%s/patch-%s-2.MPQ",LANG,LANG);

    //char* archiveNames[]={"expansion.MPQ", "common.MPQ", "patch-2.MPQ", "patch.MPQ", tmp4, tmp3, tmp2, tmp1};
    char* archiveNames[]={"patch-2.MPQ", "patch.MPQ", "common.MPQ", "expansion.MPQ", tmp4, tmp3, tmp1};
    //char* archiveNames[]={"patch.MPQ", "common.MPQ", "expansion.MPQ", tmp3, tmp1};

    //for (size_t i=0; i<8; i++)
    for (size_t i=0; i<7; i++)
    //for (size_t i=0; i<5; i++)
    {
        sprintf(tmp,"%s/Data/%s",input_path,archiveNames[i]);
        archives.push_back(new MPQArchive(tmp));
    }

    //map.dbc
    DBCFile * dbc= new DBCFile("DBFilesClient\\Map.dbc");
    if(dbc)
        dbc->open();
    else
        return (1);

    map_count=dbc->getRecordCount();
    map_ids=new map_id[map_count];
    //maps_extr = new extr_map [map_count];
    //memset(maps_extr,false, sizeof(extr_map) *map_count);
    for(unsigned int x=0;x<map_count;x++)
    {
        map_ids[x].id=dbc->getRecord (x).getUInt(0);
        strcpy(map_ids[x].name,dbc->getRecord(x).getString(1));
    }
    delete dbc;
    //map.dbc

    //areatable.dbc
    dbc = new DBCFile("DBFilesClient\\AreaTable.dbc");
    dbc->open();

    unsigned int area_count=dbc->getRecordCount ();
    uint32 maxi=0;
    for(unsigned int x=0;x<area_count;x++)
    {
        if(maxi<dbc->getRecord(x).getUInt(0))
            maxi=dbc->getRecord(x).getUInt(0);
            //printf("\n%d %d",dbc->getRecord(x).getUInt(0),dbc->getRecord(x).getUInt(3));
    }
    maxi++;//not needed actually
    areas=new uint16[maxi];
    memset(areas,0xff,maxi*2);
    for(unsigned int x=0;x<area_count;x++)
        areas[dbc->getRecord(x).getUInt(0)]  =dbc->getRecord(x).getUInt(3);

    delete dbc;

    //areatable.dbc

    ExtractMapsFromMpq();

    delete [] areas;
    delete [] map_ids;

/*  // This would be nice someday (don't have time to get it to work right now):
    cout << "Extracting dbc files..." << endl;
    for (size_t i=3; i>0; i--)
    {
        vector<string> files = archives[i]->GetFileList();
        for (vector<string>::iterator iter = files.begin(); iter != files.end(); iter++) {
            if (memcmp((void *)(iter->c_str()), "DBFilesClient\\", strlen("DBFilesClient\\")) == 0) {
                string tmp = "./dbc/";
                tmp += (iter->c_str() + strlen("DBFilesClient\\"));
                //mpq_hash hash = archives[i]->GetHashEntry(iter->c_str());
                //if ((hash.blockindex != 0xFFFFFFFF) && (hash.blockindex != 0)) {
                    MPQFile m(iter->c_str());
                    if(!m.isEof ()) {
                        int fd = _open(tmp.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
                        _write(fd, m.getBuffer(), m.getSize());
                        _close(fd);
                    }
                    //if (libmpq_file_extract(&archives[i]->mpq_a, hash.blockindex, tmp.c_str())) {
                    //    cout << "Error writing " << tmp.c_str() << endl;
                    //}
            }
                //cout << iter->c_str() << endl;
        }
    }
*/
    return (0); // Exit The Program
}
