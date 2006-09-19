/*
 * PagesToSQL - Written by oak
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
#include "pagestosql.h"

char *page, *text, *text2, *nextpage;

void write_table(FILE *sql);
void init_vars(void);
void free_vars(void);
void default_vars(void);

void gethex(char* from, char* to) {
	if (((int)*(from) == '0') && ((int)*(from+1) != '\0')) sprintf(to,"%d",strtol(from, (char **)NULL, 16));
	else sprintf(to,"%s",from);
}

int main(int argc,char *argv[]) {
	if (argc <= 2) {
		printf("Usage: pagestosql.exe pages.scp pages.sql\n");
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
	buff = malloc(1*1024*1024); //1MB
	buff2 = buff2b = malloc(50*1024); //10KB
	buff3 = buff3b = malloc(50*1024); //5KB
	key = malloc(2*1024); //2KB
	value = malloc(30*1024); //2KB
	value2 = malloc(30*1024); //2KB

	write_table(sql);
	init_vars();

	long buff_size = fread(buff,1,1*1024*1024,scp);
	sprintf(buff+buff_size,"%c",EOF);

	long end_of_file, end_of_section, line_count, item_count, a, b, c, d, cx, cy;
	end_of_file = item_count = d = 0;

	cx = wherex(); cy = wherey();

	buff = strstr(buff,"[page");

	while (!end_of_file) {
		a = 0;
		a = (int) strstr(buff+1,"[page");
		if (!a) {
			a = (int) strchr(buff+1,EOF);
			end_of_file = 1;
		}
		a -= (int)buff;

		sprintf(buff+a-1,"");

		b = 1;
		while (((int)*(buff+a-(++b)) == '\r') || ((int)*(buff+a-(++b)) == '\n')) {
			sprintf(buff+a-b,"");
		}

		sprintf(buff2,"%s",buff);

		line_count = 1;
		end_of_section = 0;
		default_vars();

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
				while (((int)*(cmt_pos-(++x)) == ' ') || ((int)*(cmt_pos-(++x)) == '\t')) { sprintf(cmt_pos-x,""); }
				sprintf(cmt_pos,"");
			}

			if (!strcmp(buff3,"")) { goto next; }
			if (!strcmp(buff3,"\n")) { goto next; }
			if (!strcmp(buff3,"\r")) { goto next; }

			if (line_count == 1) {
				buff3 += 6;
				sprintf(buff3+strlen(buff3)-1,"\0");
				sprintf(page,"%s",buff3);
				gotoxy(cx,cy); clreol();
				printf("page %s...",page);
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
				if (!strcmp(value,"(null)")) { sprintf(value,""); }
				if (!strcmp(key,"text")) { sprintf(text,"%s$B%s",text2,value); sprintf(text2,"%s",text); }
				if (!strcmp(key,"next_page")) { sprintf(nextpage,"%s",value); }
				//add more fields here
			}

			next:
			buff2 += c+1;
			buff3 = buff3b;
			line_count++;
		}

		fprintf(sql,"(%s,'%s',%s)",page,text+2,nextpage);

		buff += a;
		buff2 = buff2b;

		if (end_of_file) {
			fprintf(sql,";");
			gotoxy(cx,cy); clreol();
			printf("File %s converted (%d pages)\nPress any key to exit...\n",argv[1],item_count);
			getch();
			break;
		}

		d++;
		if (d > 20) {
			fprintf(sql,";\n\ninsert into item_pages values\n");
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
