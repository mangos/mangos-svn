-- Table structure for table `gameobjecttemplate`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `gameobjecttemplate`;
CREATE TABLE IF NOT EXISTS `gameobjecttemaplate` (
  `id` bigint(20) unsigned NOT NULL default '0',
  `type` int(11) NOT NULL default '0',
  `displayId` int(11) NOT NULL default '0',
  `name` varchar(100) NOT NULL default '0',
  `sound0` int(11) NOT NULL default '0',
  `sound1` int(11) NOT NULL default '0',
  `sound2` int(11) NOT NULL default '0',
  `sound3` int(11) NOT NULL default '0',
  `sound4` int(11) NOT NULL default '0',
  `sound5` int(11) NOT NULL default '0',
  `sound6` int(11) NOT NULL default '0',
  `sound7` int(11) NOT NULL default '0',
  `sound8` int(11) NOT NULL default '0',
  `sound9` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM COMMENT='InnoDB free: 18432 kB';
