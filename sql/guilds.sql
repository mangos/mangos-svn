DROP TABLE IF EXISTS `guilds`;
CREATE TABLE IF NOT EXISTS `guilds` (
  `guildId` int(6) unsigned NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `leaderguid` int(6) unsigned NOT NULL default '0',
  `rank1` varchar(255) NOT NULL default '',
  `rank2` varchar(255) NOT NULL default '',
  `rank3` varchar(255) NOT NULL default '',
  `rank4` varchar(255) NOT NULL default '',
  `rank5` varchar(255) NOT NULL default '',
  `rank6` varchar(255) NOT NULL default '',
  `rank7` varchar(255) NOT NULL default '',
  `rank8` varchar(255) NOT NULL default '',
  `rank9` varchar(255) NOT NULL default '',
  `rank10` varchar(255) NOT NULL default '',
  `EmblemStyle` int(5) unsigned NOT NULL default '0',
  `EmblemColor` int(5) unsigned NOT NULL default '0',
  `BorderStyle` int(5) unsigned NOT NULL default '0',
  `BorderColor` int(5) unsigned NOT NULL default '0',
  `BackgroundColor` int(5) unsigned NOT NULL default '0',
  `MOTD` varchar(255) NOT NULL default '',
  `createdate` timestamp NOT NULL,

  PRIMARY KEY  (`guildId`)
) TYPE=MyISAM COMMENT='InnoDB free: 18432 kB';