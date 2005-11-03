DROP TABLE IF EXISTS `reputation`;
CREATE TABLE `reputation` (
  `ID` int(32) NOT NULL auto_increment,
  `playerID` int(32) NOT NULL default '0',
  `factionID` int(32) NOT NULL default '0',
  `standing` int(32) NOT NULL default '0',
  `flags` int(32) NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

