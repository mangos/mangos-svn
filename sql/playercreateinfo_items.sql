-----------------------------
-- Table structure for playercreateinfo_items
-----------------------------
DROP TABLE IF EXISTS `playercreateinfo_items`;
CREATE TABLE IF NOT EXISTS `playercreateinfo_items` (
  `createId` tinyint(3) unsigned NOT NULL default '0',
  `item` mediumint(8) unsigned NOT NULL default '0',
  `item_slot` tinyint(3) unsigned NOT NULL default '0'
) TYPE=MyISAM;

