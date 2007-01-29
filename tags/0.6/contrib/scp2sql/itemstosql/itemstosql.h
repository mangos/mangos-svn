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

typedef struct stat {
	char *type;
	char *value;
} stat;

typedef struct dmg {
	char *min;
	char *max;
	char *type;
} dmg;

typedef struct spell {
	char *id;
	char *trigger;
	char *charges;
	char *cooldown;
	char *category;
	char *categorycooldown;
} spell;

extern char *entry, *class, *subclass, *name, *name2, *name3, *name4, *displayid, *quality, *flags, *buyprice, *sellprice, *inventorytype, *allowableclass;
extern char *allowablerace, *itemlevel, *requiredlevel, *requiredskill, *requiredskillrank, *requiredspell, *requiredhonorrank, *field21, *field22, *field23, *maxcount;
extern char *stackable, *containerslots, *armor, *holy_res, *fire_res, *nature_res, *frost_res, *shadow_res, *arcane_res, *delay, *ammo_type, *bonding;
extern char *description, *pagetext, *languageid, *pagematerial, *startquest, *lockid, *material, *sheath, *extra, *block, *itemset, *maxdurability, *area;
extern stat stats[10];
extern dmg dmgs[5];
extern spell spells[5];

void write_table(FILE *sql) {
	fprintf(sql,"DROP TABLE IF EXISTS `items`;\n");
	fprintf(sql,"CREATE TABLE IF NOT EXISTS `items` (\n");
	fprintf(sql,"  `entry` int(255) NOT NULL default '0',\n");
	fprintf(sql,"  `class` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `subclass` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `name` varchar(255) NOT NULL default '',\n");
	fprintf(sql,"  `name2` varchar(255) NOT NULL default '',\n");
	fprintf(sql,"  `name3` varchar(255) NOT NULL default '',\n");
	fprintf(sql,"  `name4` varchar(255) NOT NULL default '',\n");
	fprintf(sql,"  `displayid` int(70) NOT NULL default '0',\n");
	fprintf(sql,"  `quality` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `flags` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `buyprice` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `sellprice` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `inventorytype` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `allowableclass` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `allowablerace` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `itemlevel` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `requiredlevel` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `requiredskill` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `requiredskillrank` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `requiredspell` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `requiredhonorrank` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `field21` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `field22` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `field23` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `maxcount` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stackable` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `containerslots` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type6` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value6` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type7` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value7` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type8` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value8` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type9` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value9` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_type10` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `stat_value10` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_min1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_max1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_type1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_min2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_max2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_type2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_min3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_max3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_type3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_min4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_max4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_type4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_min5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_max5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `dmg_type5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `armor` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `holy_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `fire_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `nature_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `frost_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `shadow_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `arcane_res` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `delay` int(11) NOT NULL default '1000',\n");
	fprintf(sql,"  `ammo_type` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellid_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spelltrigger_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcharges_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcooldown_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategory_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategorycooldown_1` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellid_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spelltrigger_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcharges_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcooldown_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategory_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategorycooldown_2` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellid_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spelltrigger_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcharges_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcooldown_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategory_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategorycooldown_3` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellid_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spelltrigger_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcharges_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcooldown_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategory_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategorycooldown_4` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellid_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spelltrigger_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcharges_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcooldown_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategory_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `spellcategorycooldown_5` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `bonding` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `description` varchar(255) NOT NULL default '',\n");
	fprintf(sql,"  `pagetext` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `languageid` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `pagematerial` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `startquest` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `lockid` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `material` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `sheath` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `extra` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `block` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `itemset` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `maxdurability` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  `area` int(30) NOT NULL default '0',\n");
	fprintf(sql,"  PRIMARY KEY  (`entry`),\n");
	fprintf(sql,"  KEY `items_index` (`class`,`itemlevel`,`requiredlevel`)\n");
	fprintf(sql,") TYPE=MyISAM COMMENT='new item 011';\n\n");
	fprintf(sql,"insert into items values\n");
}

void init_vars() {
	int x;
	entry = malloc(128);
	class = malloc(6);
	subclass = malloc(6);
	name = malloc(1024);
	name2 = malloc(1024);
	name3 = malloc(1024);
	name4 = malloc(1024);
	displayid = malloc(8);
	quality = malloc(3);
	flags = malloc(8);
	buyprice = malloc(24);
	sellprice = malloc(24);
	inventorytype = malloc(3);
	allowableclass = malloc(9);
	allowablerace = malloc(9);
	itemlevel = malloc(5);
	requiredlevel = malloc(5);
	requiredskill = malloc(8);
	requiredskillrank = malloc(8);
	requiredspell = malloc(8);
	requiredhonorrank = malloc(8);
	field21 = malloc(8);
	field22 = malloc(8);
	field23 = malloc(8);
	maxcount = malloc(8);
	stackable = malloc(8);
	containerslots = malloc(8);
	for (x=0;x<10;x++) {
		stats[x].type = malloc(12);
		stats[x].value = malloc(12);
	}
	for (x=0;x<5;x++) {
		dmgs[x].min = malloc(12);
		dmgs[x].max = malloc(12);
		dmgs[x].type = malloc(12);
	}
	armor = malloc(8);
	holy_res = malloc(8);
	fire_res = malloc(8);
	nature_res = malloc(8);
	frost_res = malloc(8);
	shadow_res = malloc(8);
	arcane_res = malloc(8);
	delay = malloc(8);
	ammo_type = malloc(3);
	for (x=0;x<5;x++) {
		spells[x].id = malloc(16);
		spells[x].trigger = malloc(16);
		spells[x].charges = malloc(16);
		spells[x].cooldown = malloc(16);
		spells[x].category = malloc(16);
		spells[x].categorycooldown = malloc(16);
	}
	bonding = malloc(8);
	description = malloc(1024);
	pagetext = malloc(8);
	languageid = malloc(8);
	pagematerial = malloc(8);
	startquest = malloc(8);
	lockid = malloc(8);
	material = malloc(8);
	sheath = malloc(8);
	extra = malloc(8);
	block = malloc(8);
	itemset = malloc(8);
	maxdurability = malloc(8);
    area = malloc(8);
}

void free_vars() {
	int x;
	free(entry);
	free(class);
	free(subclass);
	free(name);
	free(name2);
	free(name3);
	free(name4);
	free(displayid);
	free(quality);
	free(flags);
	free(buyprice);
	free(sellprice);
	free(inventorytype);
	free(allowableclass);
	free(allowablerace);
	free(itemlevel);
	free(requiredlevel);
	free(requiredskill);
	free(requiredskillrank);
	free(requiredspell);
	free(requiredhonorrank);
	free(field21);
	free(field22);
	free(field23);
	free(maxcount);
	free(stackable);
	free(containerslots);
	for (x=0;x<10;x++) {
		free(stats[x].type);
		free(stats[x].value);
	}
	for (x=0;x<5;x++) {
		free(dmgs[x].min);
		free(dmgs[x].max);
		free(dmgs[x].type);
	}
	free(armor);
	free(holy_res);
	free(fire_res);
	free(nature_res);
	free(frost_res);
	free(shadow_res);
	free(arcane_res);
	free(delay);
	free(ammo_type);
	for (x=0;x<5;x++) {
		free(spells[x].id);
		free(spells[x].trigger);
		free(spells[x].charges);
		free(spells[x].cooldown);
		free(spells[x].category);
		free(spells[x].categorycooldown);
	}
	free(bonding);
	free(description);
	free(pagetext);
	free(languageid);
	free(pagematerial);
	free(startquest);
	free(lockid);
	free(material);
	free(sheath);
	free(extra);
	free(block);
	free(itemset);
	free(maxdurability);
    free(area);
}
void default_vars() {
	int x;
	sprintf(entry,"0");
	sprintf(class,"0");
	sprintf(subclass,"0");
	sprintf(name,"");
	sprintf(name2,"");
	sprintf(name3,"");
	sprintf(name4,"");
	sprintf(displayid,"0");
	sprintf(quality,"0");
	sprintf(flags,"0");
	sprintf(buyprice,"0");
	sprintf(sellprice,"0");
	sprintf(inventorytype,"0");
	sprintf(allowableclass,"0");
	sprintf(allowablerace,"0");
	sprintf(itemlevel,"0");
	sprintf(requiredlevel,"0");
	sprintf(requiredskill,"0");
	sprintf(requiredskillrank,"0");
	sprintf(requiredspell,"0");
	sprintf(requiredhonorrank,"0");
	sprintf(field21,"0");
	sprintf(field22,"0");
	sprintf(field23,"0");
	sprintf(maxcount,"0");
	sprintf(stackable,"0");
	sprintf(containerslots,"0");
	for (x=0;x<10;x++) {
		sprintf(stats[x].type,"0");
		sprintf(stats[x].value,"0");
	}
	for (x=0;x<5;x++) {
		sprintf(dmgs[x].min,"0");
		sprintf(dmgs[x].max,"0");
		sprintf(dmgs[x].type,"0");
	}
	sprintf(armor,"0");
	sprintf(holy_res,"0");
	sprintf(fire_res,"0");
	sprintf(nature_res,"0");
	sprintf(frost_res,"0");
	sprintf(shadow_res,"0");
	sprintf(arcane_res,"0");
	sprintf(delay,"0");
	sprintf(ammo_type,"0");
	for (x=0;x<5;x++) {
		sprintf(spells[x].id,"0");
		sprintf(spells[x].trigger,"0");
		sprintf(spells[x].charges,"0");
		sprintf(spells[x].cooldown,"0");
		sprintf(spells[x].category,"0");
		sprintf(spells[x].categorycooldown,"0");
	}
	sprintf(bonding,"0");
	sprintf(description,"");
	sprintf(pagetext,"0");
	sprintf(languageid,"0");
	sprintf(pagematerial,"0");
	sprintf(startquest,"0");
	sprintf(lockid,"0");
	sprintf(material,"0");
	sprintf(sheath,"0");
	sprintf(extra,"0");
	sprintf(block,"0");
	sprintf(itemset,"0");
	sprintf(maxdurability,"0");
    sprintf(area,"0");
}
