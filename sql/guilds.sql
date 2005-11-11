DROP TABLE IF EXISTS `guilds`;
CREATE TABLE IF NOT EXISTS `guilds` (
  `guildId` int(6) unsigned NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `leaderguid` tinyint(3) unsigned NOT NULL default 0,
  `rank1` varchar(255) NOT NULL default 'Unused',
  `rank2` varchar(255) NOT NULL default 'Unused',
  `rank3` varchar(255) NOT NULL default 'Unused',
  `rank4` varchar(255) NOT NULL default 'Unused',
  `rank5` varchar(255) NOT NULL default 'Unused',
  `rank6` varchar(255) NOT NULL default 'Unused',
  `rank7` varchar(255) NOT NULL default 'Unused',
  `rank8` varchar(255) NOT NULL default 'Unused',
  `rank9` varchar(255) NOT NULL default 'Unused',
  `rank10` varchar(255) NOT NULL default 'Unused',
  `EmblemStyle` int(5) unsigned NOT NULL default 0,
  `EmblemColor` int(5) unsigned NOT NULL default 0,
  `BorderStyle` int(5) unsigned NOT NULL default 0,
  `BorderColor` int(5) unsigned NOT NULL default 0,
  `BackgroundColor` int(5) unsigned NOT NULL default 0,
  `MOTD` longtext NOT NULL default '',

  PRIMARY KEY  (`guildId`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;