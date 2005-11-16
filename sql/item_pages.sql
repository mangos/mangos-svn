-- --------------------------------------------------------

-- 
-- Table structure for table `item_pages`
-- 
-- Creation: Sep 11, 2005 at 08:21 PM
-- Last update: Sep 11, 2005 at 08:23 PM
-- 

DROP TABLE IF EXISTS `item_pages`;
CREATE TABLE IF NOT EXISTS `item_pages` (
  `pageid` bigint(20) unsigned NOT NULL auto_increment,
  `text` longtext NOT NULL,
  `nextpage` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`pageid`)
) TYPE=MyISAM;
