DROP TABLE IF EXISTS `reputation`;
CREATE TABLE IF NOT EXISTS `reputation` (
  `ID` int(32) NOT NULL default '0',
  `playerID` int(32) NOT NULL default '0',
  `factionID` int(32) NOT NULL default '0',
  `standing` int(32) NOT NULL default '0',
  `flags` int(32) NOT NULL default '0'
) TYPE=MyISAM;
