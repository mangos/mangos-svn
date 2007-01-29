/*
 * ItemsToSQL - Written by oak
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <tcconio.h>
#include "itemstosql.h"

void write_table(FILE *sql);
void init_vars(void);
void free_vars(void);
void default_vars(void);

char *entry, *class, *subclass, *name, *name2, *name3, *name4, *displayid, *quality, *flags, *buyprice, *sellprice, *inventorytype, *allowableclass;
char *allowablerace, *itemlevel, *requiredlevel, *requiredskill, *requiredskillrank, *requiredspell, *requiredhonorrank, *field21, *field22, *field23, *maxcount;
char *stackable, *containerslots, *armor, *holy_res, *fire_res, *nature_res, *frost_res, *shadow_res, *arcane_res, *delay, *ammo_type, *bonding;
char *description, *pagetext, *languageid, *pagematerial, *startquest, *lockid, *material, *sheath, *extra, *block, *itemset, *maxdurability, *area;
stat stats[10];
dmg dmgs[5];
spell spells[5];

void gethex(char* from, char* to) {
	if (((int)*(from) == '0') && ((int)*(from+1) != '\0')) sprintf(to,"%d",strtol(from, (char **)NULL, 16));
	else sprintf(to,"%s",from);
}

int main(int argc,char *argv[]) {
	if (argc <= 2) {
		printf("Usage: itemstosql.exe items.scp items.sql\n");
		return 0;
	}

	FILE *scp = fopen(argv[1],"rb");
	if (!scp) {
		printf("Cannot open file: %s\n",argv[1]);
		return 0;
	}

	FILE *sql = fopen(argv[2],"wb");
	if (!sql) {
		printf("Cannot open file: %s\n",argv[2]);
		fclose(scp);
		return 0;
	}

	char *buff, *buff2b, *buff2, *buff3b, *buff3, *line, *key, *value, *value2;
	buff = malloc(6*1024*1024); //6MB
	buff2 = buff2b = malloc(10*1024); //10KB
	buff3 = buff3b = malloc(5*1024); //5KB
	key = malloc(2*1024); //2KB
	value = malloc(2*1024); //2KB
	value2 = malloc(2*1024); //2KB

	write_table(sql);
	init_vars();

	long buff_size = fread(buff,1,6*1024*1024,scp);
	sprintf(buff+buff_size,"%c",EOF);

	long end_of_file, end_of_section, line_count, item_count, stat_count, dmg_count, spell_count, a, b, c, d, cx, cy;
	end_of_file = item_count = d = 0;

	cx = wherex(); cy = wherey();

	buff = strstr(buff,"[item");
	while (!end_of_file) {
		a = 0;
		a = (int) strstr(buff+1,"[item");
		if (!a) {
			a = (int) strchr(buff+1,EOF);
			end_of_file = 1;
		}
		a -= (int)buff;

		sprintf(buff+a-1,"\0");

		b = 1;
		while (((int)*(buff+a-(++b)) == '\r') || ((int)*(buff+a-(++b)) == '\n')) {
			sprintf(buff+a-b,"");
		}

		sprintf(buff2,"%s",buff);

		line_count = 1;
		end_of_section = 0;
		default_vars();
		stat_count = dmg_count = spell_count = 0;

		while (!end_of_section) {
			c=1;
			c = (long) strchr(buff2+1,'\n');
			if (!c) {
				c = (long) strchr(buff2+1,'\0');
				end_of_section = 1;
			}
			c = c-(long)buff2;

			sprintf(buff2+c,"");
			if ((int)*(buff2+c-1) == '\r') { sprintf(buff2+c-1,""); }

			sprintf(buff3,"%s",buff2);

			if (strstr(buff3,"//")) {
				char* cmt_pos = strstr(buff3,"//");
				int x=0;
				while (((int)*(cmt_pos-(++x)) == ' ') || ((int)*(cmt_pos-(++x)) == '\t')) { sprintf(cmt_pos-x,"\0"); }
				sprintf(cmt_pos,"\0");
			}
			if (!strcmp(buff3,"")) { goto next; }
			if (!strcmp(buff3,"\n")) { goto next; }
			if (!strcmp(buff3,"\r")) { goto next; }

			if (line_count == 1) {
				buff3 += 6;
				sprintf(buff3+strlen(buff3)-1,"\0");
				sprintf(entry,"%s",buff3);
				gotoxy(cx,cy);
				clreol();
				printf("item %s...",entry);
				item_count++;
			} else {
				sprintf(value,strchr(buff3,'=')+1);
				sprintf(strchr(buff3,'='),"\0");
				sprintf(key,"%s",buff3);

				int x=0;
				while ((int)*(value+x) != '\0') {
					if (((int)*(value+x) == '\'') && ((int)*(value+x-1) != '\\')) {
						sprintf(value2,"%s",value+x);
						sprintf(value+x,"\\");
						sprintf(value+x+1,"%s",value2);
						x++;
					}
					x++;
				}

				if (!strcmp(key,"class")) { sprintf(class,"%s",value); }
				else if (!strcmp(key,"subclass")) { sprintf(subclass,"%s",value); }
				else if (!strcmp(key,"name")) { sprintf(name,"%s",value); sprintf(name2,"%s",value); sprintf(name3,"%s",value); sprintf(name4,"%s",value); }
				else if (!strcmp(key,"model")) { gethex(value,displayid); }
				else if (!strcmp(key,"quality")) { gethex(value,quality); }
				else if (!strcmp(key,"flags")) { gethex(value,flags); }
				else if (!strcmp(key,"buyprice")) { sprintf(buyprice,"%s",value); }
				else if (!strcmp(key,"sellprice")) { sprintf(sellprice,"%s",value); }
				else if (!strcmp(key,"inventorytype")) { gethex(value,inventorytype); }
				else if (!strcmp(key,"classes")) { gethex(value,allowableclass); }
				else if (!strcmp(key,"races")) { gethex(value,allowablerace); }
				else if (!strcmp(key,"level")) { sprintf(itemlevel,"%s",value); }
				else if (!strcmp(key,"reqlevel")) { sprintf(requiredlevel,"%s",value); }
				else if (!strcmp(key,"skill")) { sprintf(requiredskill,"%s",value); }
				else if (!strcmp(key,"skillrank")) { sprintf(requiredskillrank,"%s",value); }
				else if (!strcmp(key,"spellreq")) { sprintf(requiredspell,"%s",value); }
				else if (!strcmp(key,"pvprankreq")) { sprintf(requiredhonorrank,"%s",value); }
				else if (!strcmp(key,"maxcount")) { sprintf(maxcount,"%s",value); }
				else if (!strcmp(key,"stackable")) { sprintf(stackable,"%s",value); }
				else if (!strcmp(key,"containerslots")) { sprintf(containerslots,"%s",value); }
				else if (!strcmp(key,"bonus")) {
					sprintf(stats[stat_count].type,"%s",strtok(value," "));
					sprintf(stats[stat_count].value,"%s",strtok(NULL," "));
					stat_count++;
				}
				else if (!strcmp(key,"damage")) {
					sprintf(dmgs[dmg_count].min,"%s",strtok(value," "));
					sprintf(dmgs[dmg_count].max,"%s",strtok(NULL," "));
					sprintf(dmgs[dmg_count].type,"%s",strtok(NULL," "));
					dmg_count++;
				}
				else if (!strcmp(key,"resistance1")) { sprintf(armor,"%s",value); }
				else if (!strcmp(key,"resistance2")) { sprintf(holy_res,"%s",value); }
				else if (!strcmp(key,"resistance3")) { sprintf(fire_res,"%s",value); }
				else if (!strcmp(key,"resistance4")) { sprintf(nature_res,"%s",value); }
				else if (!strcmp(key,"resistance5")) { sprintf(frost_res,"%s",value); }
				else if (!strcmp(key,"resistance6")) { sprintf(shadow_res,"%s",value); }
				else if (!strcmp(key,"resistance7")) { sprintf(arcane_res,"%s",value); }
				else if (!strcmp(key,"delay")) { gethex(value,delay); }
				else if (!strcmp(key,"ammo_type")) { sprintf(ammo_type,"%s",value); }
				else if (!strcmp(key,"spell")) {
					sprintf(spells[spell_count].id,"%s",strtok(value," "));
					sprintf(spells[spell_count].trigger,"%s",strtok(NULL," "));
					sprintf(spells[spell_count].charges,"%s",strtok(NULL," "));
					sprintf(spells[spell_count].cooldown,"%s",strtok(NULL," "));
					sprintf(spells[spell_count].category,"%s",strtok(NULL," "));
					sprintf(spells[spell_count].categorycooldown,"%s",strtok(NULL," "));
					spell_count++;
				}
				else if (!strcmp(key,"bonding")) { sprintf(bonding,"%s",value); }
				else if (!strcmp(key,"description")) { sprintf(description,"%s",value); }
				else if (!strcmp(key,"pagetext")) { sprintf(pagetext,"%s",value); }
				else if (!strcmp(key,"language")) { sprintf(languageid,"%s",value); }
				else if (!strcmp(key,"pagematerial")) { sprintf(pagematerial,"%s",value); }
				else if (!strcmp(key,"startquest")) { sprintf(startquest,"%s",value); }
				else if (!strcmp(key,"lockid")) { sprintf(lockid,"%s",value); }
				else if (!strcmp(key,"material")) { sprintf(material,"%s",value); }
				else if (!strcmp(key,"sheath")) { sprintf(sheath,"%s",value); }
				else if (!strcmp(key,"extra")) { sprintf(extra,"%s",value); }
				else if (!strcmp(key,"block")) { sprintf(block,"%s",value); }
				else if (!strcmp(key,"set")) { sprintf(itemset,"%s",value); }
				else if (!strcmp(key,"durability")) { sprintf(maxdurability,"%s",value); }
				//add more fields here
			}
			next:
			buff2 += c+1;
			buff3 = buff3b;
			line_count++;
		}
		fprintf(sql,"(%s,%s,%s,'%s','%s','%s','%s'",entry,class,subclass,name,name,name,name);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",displayid,quality,flags,buyprice,sellprice,inventorytype,allowableclass,allowablerace,itemlevel,requiredlevel);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",requiredskill,requiredskillrank,requiredspell,requiredhonorrank,field21,field22,field23,maxcount,stackable,containerslots);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",stats[0].type,stats[0].value,stats[1].type,stats[1].value,stats[2].type,stats[2].value,stats[3].type,stats[3].value,stats[4].type,stats[4].value);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",stats[5].type,stats[5].value,stats[6].type,stats[6].value,stats[7].type,stats[7].value,stats[8].type,stats[8].value,stats[9].type,stats[9].value);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s",dmgs[0].min,dmgs[0].max,dmgs[0].type,dmgs[1].min,dmgs[1].max,dmgs[1].type,dmgs[2].min,dmgs[2].max,dmgs[2].type);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",dmgs[3].min,dmgs[3].max,dmgs[3].type,dmgs[4].min,dmgs[4].max,dmgs[4].type);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s,%s,%s",armor,holy_res,fire_res,nature_res,frost_res,shadow_res,arcane_res,delay,ammo_type);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",spells[0].id,spells[0].trigger,spells[0].charges,spells[0].cooldown,spells[0].category,spells[0].categorycooldown);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",spells[1].id,spells[1].trigger,spells[1].charges,spells[1].cooldown,spells[1].category,spells[1].categorycooldown);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",spells[2].id,spells[2].trigger,spells[2].charges,spells[2].cooldown,spells[2].category,spells[2].categorycooldown);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",spells[3].id,spells[3].trigger,spells[3].charges,spells[3].cooldown,spells[3].category,spells[3].categorycooldown);
		fprintf(sql,",%s,%s,%s,%s,%s,%s",spells[4].id,spells[4].trigger,spells[4].charges,spells[4].cooldown,spells[4].category,spells[4].categorycooldown);
		fprintf(sql,",%s,'%s',%s,%s,%s,%s,%s",bonding,description,pagetext,languageid,pagematerial,startquest,lockid);
		fprintf(sql,",%s,%s,%s,%s,%s,%s,%s)",material,sheath,extra,block,itemset,maxdurability,area);

		buff += a;
		buff2 = buff2b;

		if (end_of_file) {
			fprintf(sql,";");
			gotoxy(cx,cy); clreol();
			printf("File %s converted (%d items)\nPress any key to exit...\n",argv[1],item_count);
			getch();
			break;
		}

		d++;
		if (d > 39) {
			fprintf(sql,";\n\ninsert into items values\n");
			d = 0;
		} else {
			fprintf(sql,",\n");
		}

	}
	free_vars();
	free(key);
	free(value);
	free(buff);
	free(buff2);
	free(buff3);
	fclose(sql);
	fclose(scp);
	return 1;
}
