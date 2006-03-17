DROP TABLE IF EXISTS `kills`;
CREATE TABLE `kills` (
  `killerID` INT(32) NOT NULL, 
  `victimID` INT(32) NOT NULL, 
  `honor_pts` INT(32) NOT NULL, 
  `date`  INT(32) NOT NULL,
  `type` smallint(5) unsigned NOT NULL default '0'
);