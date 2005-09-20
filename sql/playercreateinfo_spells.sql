-----------------------------
-- Table structure for playercreateinfo_spells
-----------------------------
DROP TABLE IF EXISTS `playercreateinfo_spells`;
CREATE TABLE IF NOT EXISTS `playercreateinfo_spells` (
  `createId` tinyint(3) unsigned NOT NULL default '0',
  `spell` mediumint(8) unsigned NOT NULL default '0'
) TYPE=MyISAM;

