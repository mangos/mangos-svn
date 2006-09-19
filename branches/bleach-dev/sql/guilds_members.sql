DROP TABLE IF EXISTS `guilds_members`;
CREATE TABLE IF NOT EXISTS `guilds_members` (
  `guildId` int(6) unsigned NOT NULL default 0,
  `memguid`   int(6) NOT NULL default 0,
  `rank` tinyint(2) unsigned NOT NULL default 0,
  
  PRIMARY KEY  (`guildId`)
) TYPE=MyISAM AUTO_INCREMENT=44 ;