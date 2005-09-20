#----------------------------
# Table structure for playercreateinfo_skills
#----------------------------
CREATE TABLE `playercreateinfo_skills` (
  `createId` tinyint(3) unsigned NOT NULL default '0',
  `skill` smallint(5) unsigned NOT NULL default '0',
  `skillCuVal` smallint(5) unsigned NOT NULL default '0',
  `skillMaxVal` smallint(5) unsigned NOT NULL default '0',

) TYPE=MyISAM;