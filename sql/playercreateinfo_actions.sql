#----------------------------
# Table structure for playercreateinfo
#----------------------------
CREATE TABLE `playercreateinfo_actions` (
  `createId` tinyint(3) unsigned NOT NULL default '0',
  `button` smallint(2) unsigned NOT NULL default '0',
  `action` smallint(6) unsigned NOT NULL default '0',
  `type` smallint(3) unsigned NOT NULL default '0',
  `misc` smallint(3) unsigned NOT NULL default '0',

) TYPE=MyISAM;