DROP TABLE IF EXISTS `honor`;
CREATE TABLE `honor` (
  `guid` bigint(20) NOT NULL default '0',
  `SESSION_KILLS` int(11) default NULL,
  `YESTERDAY_KILLS` int(11) default NULL,
  `LAST_WEEK_KILLS` int(11) default NULL,
  `THIS_WEEK_KILLS` int(11) default NULL,
  `THIS_WEEK_CONTRIBUTION` int(11) default NULL,
  `LIFETIME_HONORBALE_KILLS` int(11) default NULL,
  `LIFETIME_DISHONORBALE_KILLS` int(11) default NULL,
  `YESTERDAY_CONTRIBUTION` int(11) default NULL,
  `LAST_WEEK_CONTRIBUTION` int(11) default NULL,
  `LAST_WEEK_RANK` int(11) default NULL,
  `PLAYER_FIELD_PVP_MEDALS` int(11) default NULL,
  PRIMARY KEY  (`guid`)
);


