--
-- Table structure for table `npc_trainer`
--

DROP TABLE IF EXISTS `npc_trainer`;
CREATE TABLE `npc_trainer` (
  `rowid` int(11) NOT NULL default '0',
  `entry` int(11) NOT NULL default '0',
  `spell` int(11) NOT NULL default '0',
  `spellcost` int(11) default '0',
  `reqspell` int(11) default '0',
  `reqskill` int(11) default '0',
  `reqskillvalue` int(11) default '0',
  PRIMARY KEY  (`rowid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_trainer`
--


/*!40000 ALTER TABLE `npc_trainer` DISABLE KEYS */;
LOCK TABLES `npc_trainer` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_trainer` ENABLE KEYS */;
