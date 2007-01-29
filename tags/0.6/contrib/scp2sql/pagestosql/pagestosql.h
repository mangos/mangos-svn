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

extern char *page, *text, *text2, *nextpage;

void write_table(FILE *sql) {
	fprintf(sql,"DROP TABLE IF EXISTS `item_pages`;\n");
	fprintf(sql,"CREATE TABLE `item_pages` (\n");
	fprintf(sql,"  `ID` int(11) NOT NULL default '0',\n");
	fprintf(sql,"  `text` longtext NOT NULL,\n");
	fprintf(sql,"  `next_page` bigint(20) unsigned NOT NULL default '0',\n");
	fprintf(sql,"  PRIMARY KEY  (`ID`),\n");
	fprintf(sql,"  KEY `item_pages_index` (`ID`)\n");
	fprintf(sql,") ENGINE=MyISAM DEFAULT CHARSET=latin1;\n\n");
	fprintf(sql,"insert into item_pages values\n");
}

void init_vars() {
	page = malloc(8);
	text = malloc(10*1024);
	text2 = malloc(10*1024);
	nextpage = malloc(8);
}

void free_vars() {
	free(page);
	free(text);
	free(text2);
	free(nextpage);
}

void default_vars() {
	sprintf(page,"");
	sprintf(text,"");
	sprintf(text2,"");
	sprintf(nextpage,"0");
}
