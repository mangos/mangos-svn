DROP TABLE IF EXISTS `guilds_ranks`;
CREATE TABLE IF NOT EXISTS `guilds_ranks` (
  `guildId` int(6) unsigned NOT NULL default 0,
  `rname` varchar(255) NOT NULL default '',
  `rights` int(3) unsigned NOT NULL default '0'
) TYPE=MyISAM;