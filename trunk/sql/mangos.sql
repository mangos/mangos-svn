-- MySQL dump 10.10
--
-- Host: localhost    Database: mangos
-- ------------------------------------------------------
-- Server version	5.0.18
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO,MYSQL40' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `acct` bigint(20) NOT NULL auto_increment,
  `login` varchar(255) NOT NULL default '',
  `password` varchar(28) NOT NULL default '',
  `gm` tinyint(1) NOT NULL default '0',
  `sessionkey` longtext NOT NULL,
  `email` varchar(50) NOT NULL default '',
  `joindate` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `banned` tinyint(1) NOT NULL default '0',
  `last_ip` varchar(30) NOT NULL default '0',
  `failed_logins` int(6) default '0',
  `locked` int(1) default '0',
  PRIMARY KEY  (`acct`),
  UNIQUE KEY `acct` (`acct`),
  KEY `accounts` (`gm`,`banned`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `accounts`
--


/*!40000 ALTER TABLE `accounts` DISABLE KEYS */;
LOCK TABLES `accounts` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `accounts` ENABLE KEYS */;

--
-- Table structure for table `areatrigger`
--

DROP TABLE IF EXISTS `areatrigger`;
CREATE TABLE `areatrigger` (
  `triggerID` int(20) NOT NULL auto_increment,
  `TargetPosX` float NOT NULL default '0',
  `TargetPosY` float NOT NULL default '0',
  `TargetPosZ` float NOT NULL default '0',
  `TargetOrientation` float NOT NULL default '0',
  `TargetMapID` int(11) unsigned NOT NULL default '0',
  `Triggername` text,
  PRIMARY KEY  (`triggerID`),
  KEY `areatrigger_index` (`TargetMapID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `areatrigger`
--


/*!40000 ALTER TABLE `areatrigger` DISABLE KEYS */;
LOCK TABLES `areatrigger` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger` ENABLE KEYS */;

--
-- Table structure for table `auctioned_items`
--

DROP TABLE IF EXISTS `auctioned_items`;
CREATE TABLE `auctioned_items` (
  `guid` bigint(20) NOT NULL default '0',
  `data` longtext NOT NULL,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `auctioned_items`
--


/*!40000 ALTER TABLE `auctioned_items` DISABLE KEYS */;
LOCK TABLES `auctioned_items` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `auctioned_items` ENABLE KEYS */;

--
-- Table structure for table `auctionhouse`
--

DROP TABLE IF EXISTS `auctionhouse`;
CREATE TABLE `auctionhouse` (
  `auctioneerguid` int(32) NOT NULL default '0',
  `itemguid` int(32) NOT NULL default '0',
  `itemowner` int(32) NOT NULL default '0',
  `buyoutprice` int(32) NOT NULL default '0',
  `time` bigint(40) NOT NULL default '0',
  `buyguid` int(32) NOT NULL default '0',
  `lastbid` int(32) NOT NULL default '0',
  `id` int(32) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `auctionhouse`
--


/*!40000 ALTER TABLE `auctionhouse` DISABLE KEYS */;
LOCK TABLES `auctionhouse` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `auctionhouse` ENABLE KEYS */;

--
-- Table structure for table `bag`
--

DROP TABLE IF EXISTS `bag`;
CREATE TABLE `bag` (
  `bag_guid` bigint(20) NOT NULL default '0',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `item_guid` bigint(20) NOT NULL default '0',
  `item_id` int(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`item_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `bag`
--


/*!40000 ALTER TABLE `bag` DISABLE KEYS */;
LOCK TABLES `bag` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `bag` ENABLE KEYS */;

--
-- Table structure for table `bids`
--

DROP TABLE IF EXISTS `bids`;
CREATE TABLE `bids` (
  `bidder` int(32) NOT NULL default '0',
  `id` int(32) NOT NULL default '0',
  `amt` int(32) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `bids`
--


/*!40000 ALTER TABLE `bids` DISABLE KEYS */;
LOCK TABLES `bids` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `bids` ENABLE KEYS */;

--
-- Table structure for table `bugreport`
--

DROP TABLE IF EXISTS `bugreport`;
CREATE TABLE `bugreport` (
  `bug_id` int(11) NOT NULL auto_increment,
  `rep_type` varchar(255) NOT NULL default '',
  `rep_content` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`bug_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `bugreport`
--


/*!40000 ALTER TABLE `bugreport` DISABLE KEYS */;
LOCK TABLES `bugreport` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `bugreport` ENABLE KEYS */;

--
-- Table structure for table `char_actions`
--

DROP TABLE IF EXISTS `char_actions`;
CREATE TABLE `char_actions` (
  `charId` int(6) NOT NULL default '0',
  `button` int(2) unsigned NOT NULL default '0',
  `action` int(6) unsigned NOT NULL default '0',
  `type` int(3) unsigned NOT NULL default '0',
  `misc` int(3) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `char_actions`
--


/*!40000 ALTER TABLE `char_actions` DISABLE KEYS */;
LOCK TABLES `char_actions` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `char_actions` ENABLE KEYS */;

--
-- Table structure for table `char_spells`
--

DROP TABLE IF EXISTS `char_spells`;
CREATE TABLE `char_spells` (
  `id` bigint(20) unsigned zerofill NOT NULL auto_increment,
  `charId` bigint(20) unsigned NOT NULL default '0',
  `spellId` int(20) unsigned NOT NULL default '0',
  `slotId` int(11) unsigned default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `char_spells`
--


/*!40000 ALTER TABLE `char_spells` DISABLE KEYS */;
LOCK TABLES `char_spells` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `char_spells` ENABLE KEYS */;

--
-- Table structure for table `characters`
--

DROP TABLE IF EXISTS `characters`;
CREATE TABLE `characters` (
  `guid` int(6) unsigned NOT NULL default '0',
  `acct` bigint(20) unsigned NOT NULL default '0',
  `data` longtext NOT NULL,
  `name` varchar(21) NOT NULL default '',
  `race` tinyint(2) NOT NULL default '0',
  `class` tinyint(2) NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `mapId` mediumint(8) unsigned NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `taximask` longtext NOT NULL,
  `online` tinyint(1) NOT NULL default '0',
  `highest_rank` int(11) NOT NULL default '0',
  `last_week_rank` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `characters`
--


/*!40000 ALTER TABLE `characters` DISABLE KEYS */;
LOCK TABLES `characters` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `characters` ENABLE KEYS */;

--
-- Table structure for table `commands`
--

DROP TABLE IF EXISTS `commands`;
CREATE TABLE `commands` (
  `name` varchar(100) NOT NULL default '',
  `security` int(11) NOT NULL default '0',
  `help` longtext NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `commands`
--


/*!40000 ALTER TABLE `commands` DISABLE KEYS */;
LOCK TABLES `commands` WRITE;
INSERT INTO `commands` VALUES ('help',0,'Syntax: .help <command name>\r\nDisplays help on a command.'),('acct',0,'Syntax: .acct\r\nDisplays the level of your account.'),('mount',4,'Syntax: .mount <mount number>\r\nMount from the mount number # \r\n(max=3)  lvl10=1 lvl15=2 lvl20=3'),('start',4,'Syntax: .start\r\nWarp to your start.'),('save',0,'Syntax: .save\r\nSave your character.'),('gps',0,'Syntax: .gps\r\nWill display the coordinates for your current position in the world.'),('modify',4,'Syntax: .modify # <new value>\r\n#  gold\r\n    mana\r\n    hp\r\n    level\r\n    speed\r\n    scale\r\n    mount'),('announce',4,'Syntax: .announce <Message to announce>\r\nSends a global message to all characters.'),('aura',4,'Syntax: .aura <aura number>\r\nTo test aura\'s, can be unstable'),('learn',4,'Syntax: .learn <spell number>\r\nLearn a spell to your character'),('summon',4,'Syntax: .summon <character name>\r\nTeleport the user <character name> to you.'),('appear',4,'Syntax: .appear <character name>\r\nTeleport you to the user'),('kick',4,'Syntax: .kick <character name>\r\nForce to disconnect user (you can\'t kick a >gm).'),('prog',4,'Syntax: .prog\r\nTeleports you to Programer\'s Island.'),('guid',4,'Syntax: .guid\r\nWill display the GUID for the selected NPC.'),('AddSpawn',4,'Syntax: .AddSpawn <model number> <npc flag> <faction id> <level> <name>\r\nAllows you to spawn a NPC.\r\n<model number> = decimal model number'),('spawntaxi',4,'Syntax: .spawntaxi\r\nSpawns a taxi vendor at your current location.'),('delete',4,'Syntax: .delete\r\nDelete selected NPC.'),('name',4,'Syntax: .name <new name>\r\nChanges the name of the selected NPC. (Max 75 chars)'),('changelevel',4,'Syntax: .changelevel <new level>\r\nChange the level of selected NPC (max 99)'),('item',4,'Syntax: .item \r\nAllows you to assign an item to a vendor.'),('itemmove',4,'Syntax: .itemmove\r\nNOT WORKING'),('addmove',4,'Syntax: .move\r\nAdd your current location for move.'),('random',4,'Syntax: .random #\r\nSet random movement! 1=ranom(default), 0=path'),('run',4,'Syntax: .run #\r\nSet run or walk! 1=run, 0=walk(default)'),('anim',4,''),('animfreq',4,''),('commands',4,'Syntax: .commands\r\nWill display a list of available GM commands.'),('die',4,'Syntax: .die\r\nKills your character.'),('dismount',0,'Syntax: .dismount\r\nDismounts you, if you are mounted.'),('displayid',4,'Syntax: .displayid\r\nChanges the skin ID of a NPC.'),('factionid',4,'Syntax: .factionid\r\nChanges the faction ID of a NPC.'),('gmlist',0,'Syntax: .gmlist\r\nWill display a list of Game Masters online.'),('gmoff',4,'Syntax: .gmoff\r\nSwitches off <GM> prefix for your character.'),('gmon',4,'Syntax: .gmon\r\nSwitches on <GM> prefix for your character.'),('info',0,'Syntax: .info\r\nWill display the number of connected users.'),('morph',4,'Syntax: .morph\r\nChanges your skin.'),('go',4,'Syntax: .go X Y Z\r\nTeleports you to the coordinates given as X Y Z.'),('npcflag',4,'Syntax: .npcflag\r\nChanges the flag of the selected NPC.'),('security',4,''),('worldport',4,'Syntax: .worldport\r\nTeleports you around the world without the loading screen.'),('update',4,''),('addgrave',4,'Syntax: .addgrave\r\nWill add a graveyour location with the current position to the database.'),('addsh',4,'Syntax: .addsh\r\nSpawns a spirit healer on your current location. You wont see it, if you are not dead.'),('npcinfo',4,''),('demorph',4,'Syntax: .demorph\r\nWill change your skin back to the default skin.\r\n'),('revive',4,'Syntax: .revive\r\nWill revive the selected character.'),('addspw',4,'Syntax: .addspw <entry id>\r\nAllows to spawn a creature from a creature template using the given template id.\r\n<entry id> = decimal template id\r\n');
UNLOCK TABLES;
/*!40000 ALTER TABLE `commands` ENABLE KEYS */;

--
-- Table structure for table `corpses`
--

DROP TABLE IF EXISTS `corpses`;
CREATE TABLE `corpses` (
  `guid` bigint(20) unsigned NOT NULL default '0',
  `player_guid` bigint(20) unsigned NOT NULL default '0',
  `positionx` float NOT NULL default '0',
  `positiony` float NOT NULL default '0',
  `positionz` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `zoneid` int(11) NOT NULL default '38',
  `mapid` int(11) NOT NULL default '0',
  `data` longtext NOT NULL,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `corpses`
--


/*!40000 ALTER TABLE `corpses` DISABLE KEYS */;
LOCK TABLES `corpses` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `corpses` ENABLE KEYS */;

--
-- Table structure for table `creatureinvolvedrelation`
--

DROP TABLE IF EXISTS `creatureinvolvedrelation`;
CREATE TABLE `creatureinvolvedrelation` (
  `id` int(6) unsigned NOT NULL auto_increment,
  `questid` bigint(20) unsigned NOT NULL default '0',
  `creatureid` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creatureinvolvedrelation`
--


/*!40000 ALTER TABLE `creatureinvolvedrelation` DISABLE KEYS */;
LOCK TABLES `creatureinvolvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creatureinvolvedrelation` ENABLE KEYS */;

--
-- Table structure for table `creaturequestrelation`
--

DROP TABLE IF EXISTS `creaturequestrelation`;
CREATE TABLE `creaturequestrelation` (
  `id` int(6) unsigned NOT NULL auto_increment,
  `questid` bigint(20) unsigned NOT NULL default '0',
  `creatureid` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creaturequestrelation`
--


/*!40000 ALTER TABLE `creaturequestrelation` DISABLE KEYS */;
LOCK TABLES `creaturequestrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creaturequestrelation` ENABLE KEYS */;

--
-- Table structure for table `creatures`
--

DROP TABLE IF EXISTS `creatures`;
CREATE TABLE `creatures` (
  `guid` int(10) unsigned NOT NULL default '0',
  `entry` int(10) unsigned NOT NULL default '0',
  `mapId` int(10) unsigned NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `spawnTime1` int(5) unsigned NOT NULL default '10',
  `spawnTime2` int(5) unsigned NOT NULL default '10',
  `spawnDist` float NOT NULL default '5',
  `currentWaypoint` int(10) unsigned NOT NULL default '0',
  `spawnX` float NOT NULL default '0',
  `spawnY` float NOT NULL default '0',
  `spawnZ` float NOT NULL default '0',
  `spawnOrient` float NOT NULL default '0',
  `curhealth` int(10) unsigned NOT NULL default '1',
  `curmana` int(10) unsigned NOT NULL default '0',
  `respawntimer` int(10) unsigned NOT NULL default '0',
  `state` int(10) unsigned NOT NULL default '0',
  `npcflags` int(10) unsigned NOT NULL default '0',
  `faction` int(10) unsigned NOT NULL default '0',
  `auras` longtext NOT NULL,
  UNIQUE KEY `guid` (`guid`),
  KEY `map` (`mapId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creatures`
--


/*!40000 ALTER TABLE `creatures` DISABLE KEYS */;
LOCK TABLES `creatures` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creatures` ENABLE KEYS */;

--
-- Table structure for table `creatures_grid`
--

DROP TABLE IF EXISTS `creatures_grid`;
CREATE TABLE `creatures_grid` (
  `guid` bigint(20) unsigned NOT NULL default '0',
  `x` int(11) NOT NULL default '0',
  `y` int(11) NOT NULL default '0',
  `cell_x` int(11) NOT NULL default '0',
  `cell_y` int(11) NOT NULL default '0',
  `grid_id` int(11) NOT NULL default '0',
  `cell_id` int(11) NOT NULL default '0',
  `mapid` int(11) NOT NULL default '0',
  KEY `srch_grid` (`grid_id`,`cell_id`,`mapid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creatures_grid`
--


/*!40000 ALTER TABLE `creatures_grid` DISABLE KEYS */;
LOCK TABLES `creatures_grid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creatures_grid` ENABLE KEYS */;

--
-- Table structure for table `creatures_mov`
--

DROP TABLE IF EXISTS `creatures_mov`;
CREATE TABLE `creatures_mov` (
  `id` int(11) NOT NULL auto_increment,
  `creatureid` int(11) NOT NULL default '0',
  `positionx` float NOT NULL default '0',
  `positiony` float NOT NULL default '0',
  `positionz` float NOT NULL default '0',
  `WaitTime1` tinyint(3) unsigned NOT NULL default '0',
  `WaitTime2` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `i_creatures_mov_cid` (`creatureid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creatures_mov`
--


/*!40000 ALTER TABLE `creatures_mov` DISABLE KEYS */;
LOCK TABLES `creatures_mov` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creatures_mov` ENABLE KEYS */;

--
-- Table structure for table `creaturetemplate`
--

DROP TABLE IF EXISTS `creaturetemplate`;
CREATE TABLE `creaturetemplate` (
  `entry` int(11) unsigned NOT NULL default '0',
  `modelid` int(11) unsigned default '0',
  `name` varchar(100) NOT NULL default '0',
  `subname` varchar(100) default NULL,
  `maxhealth` int(5) unsigned default '0',
  `maxmana` int(5) unsigned default '0',
  `level` int(3) unsigned default '1',
  `armor` int(10) unsigned NOT NULL default '0',
  `faction` int(4) unsigned default '0',
  `npcflag` int(4) unsigned default '0',
  `speed` float default '0',
  `rank` int(1) unsigned default '0',
  `mindmg` float default '0',
  `maxdmg` float default '0',
  `attackpower` int(10) unsigned NOT NULL default '0',
  `baseattacktime` int(4) unsigned default '0',
  `rangeattacktime` int(4) unsigned default '0',
  `flags` int(11) unsigned default '0',
  `mount` int(5) unsigned default '0',
  `level_max` int(11) default '0',
  `dynamicflags` int(11) unsigned default '0',
  `size` float default '0',
  `family` int(11) default '0',
  `bounding_radius` float default '0',
  `trainer_type` int(11) default '0',
  `class` int(11) unsigned default '0',
  `minrangedmg` float NOT NULL default '0',
  `maxrangedmg` float NOT NULL default '0',
  `rangedattackpower` int(10) unsigned NOT NULL default '0',
  `combat_reach` float NOT NULL default '0',
  `type` int(2) unsigned default '0',
  `civilian` int(4) unsigned NOT NULL default '0',
  `flag1` int(11) unsigned default '0',
  `equipmodel1` int(10) unsigned NOT NULL default '0',
  `equipmodel2` int(10) unsigned NOT NULL default '0',
  `equipmodel3` int(10) unsigned NOT NULL default '0',
  `equipinfo1` int(10) unsigned NOT NULL default '0',
  `equipinfo2` int(10) unsigned NOT NULL default '0',
  `equipinfo3` int(10) unsigned NOT NULL default '0',
  `equipslot1` int(10) unsigned NOT NULL default '0',
  `equipslot2` int(10) unsigned NOT NULL default '0',
  `equipslot3` int(10) unsigned NOT NULL default '0',
  `lootid` int(10) unsigned NOT NULL default '0',
  `skinloot` int(10) unsigned NOT NULL default '0',
  `resistance1` int(10) unsigned NOT NULL default '0',
  `resistance2` int(10) unsigned NOT NULL default '0',
  `resistance3` int(10) unsigned NOT NULL default '0',
  `resistance4` int(10) unsigned NOT NULL default '0',
  `resistance5` int(10) unsigned NOT NULL default '0',
  `resistance6` int(10) unsigned NOT NULL default '0',
  `AIName` varchar(128) NOT NULL default '',
  `MoveName` varchar(128) NOT NULL default '',
  `ScriptName` varchar(128) NOT NULL default '',
  UNIQUE KEY `entry` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creaturetemplate`
--


/*!40000 ALTER TABLE `creaturetemplate` DISABLE KEYS */;
LOCK TABLES `creaturetemplate` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creaturetemplate` ENABLE KEYS */;

--
-- Table structure for table `gameobjects`
--

DROP TABLE IF EXISTS `gameobjects`;
CREATE TABLE `gameobjects` (
  `guid` int(20) unsigned NOT NULL default '0',
  `entry` int(10) unsigned NOT NULL default '0',
  `mapid` int(10) unsigned NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `rotation0` float NOT NULL default '0',
  `rotation1` float NOT NULL default '0',
  `rotation2` float NOT NULL default '0',
  `rotation3` float NOT NULL default '0',
  `lootid` int(11) unsigned NOT NULL default '0',
  `respawntimer` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `guid` (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `gameobjects`
--


/*!40000 ALTER TABLE `gameobjects` DISABLE KEYS */;
LOCK TABLES `gameobjects` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobjects` ENABLE KEYS */;

--
-- Table structure for table `gameobjects_grid`
--

DROP TABLE IF EXISTS `gameobjects_grid`;
CREATE TABLE `gameobjects_grid` (
  `guid` bigint(20) unsigned NOT NULL default '0',
  `x` int(11) NOT NULL default '0',
  `y` int(11) NOT NULL default '0',
  `cell_x` int(11) NOT NULL default '0',
  `cell_y` int(11) NOT NULL default '0',
  `grid_id` int(11) NOT NULL default '0',
  `cell_id` int(11) NOT NULL default '0',
  `mapid` int(11) NOT NULL default '0',
  KEY `srch_grid` (`grid_id`,`cell_id`,`mapid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gameobjects_grid`
--


/*!40000 ALTER TABLE `gameobjects_grid` DISABLE KEYS */;
LOCK TABLES `gameobjects_grid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobjects_grid` ENABLE KEYS */;

--
-- Table structure for table `gameobjecttemplate`
--

DROP TABLE IF EXISTS `gameobjecttemplate`;
CREATE TABLE `gameobjecttemplate` (
  `entry` int(20) unsigned NOT NULL default '0',
  `type` int(11) unsigned NOT NULL default '0',
  `displayId` int(11) unsigned NOT NULL default '0',
  `name` varchar(100) NOT NULL default '0',
  `faction` int(4) unsigned NOT NULL default '0',
  `flags` int(4) unsigned NOT NULL default '0',
  `size` float NOT NULL default '1',
  `sound0` int(11) unsigned NOT NULL default '0',
  `sound1` int(11) unsigned NOT NULL default '0',
  `sound2` int(11) unsigned NOT NULL default '0',
  `sound3` int(11) unsigned NOT NULL default '0',
  `sound4` int(11) unsigned NOT NULL default '0',
  `sound5` int(11) unsigned NOT NULL default '0',
  `sound6` int(11) unsigned NOT NULL default '0',
  `sound7` int(11) unsigned NOT NULL default '0',
  `sound8` int(11) unsigned NOT NULL default '0',
  `sound9` int(11) unsigned NOT NULL default '0',
  `ScriptName` varchar(100) NOT NULL default '',
  UNIQUE KEY `id` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `gameobjecttemplate`
--


/*!40000 ALTER TABLE `gameobjecttemplate` DISABLE KEYS */;
LOCK TABLES `gameobjecttemplate` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobjecttemplate` ENABLE KEYS */;

--
-- Table structure for table `gmtickets`
--

DROP TABLE IF EXISTS `gmtickets`;
CREATE TABLE `gmtickets` (
  `ticket_id` int(11) NOT NULL auto_increment,
  `guid` int(6) unsigned NOT NULL default '0',
  `ticket_text` varchar(255) NOT NULL default '',
  `ticket_category` int(1) NOT NULL default '0',
  PRIMARY KEY  (`ticket_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gmtickets`
--


/*!40000 ALTER TABLE `gmtickets` DISABLE KEYS */;
LOCK TABLES `gmtickets` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gmtickets` ENABLE KEYS */;

--
-- Table structure for table `graveyards`
--

DROP TABLE IF EXISTS `graveyards`;
CREATE TABLE `graveyards` (
  `id` int(60) NOT NULL auto_increment,
  `positionx` float default NULL,
  `positiony` float default NULL,
  `positionz` float default NULL,
  `orientation` float default NULL,
  `zoneid` int(16) default NULL,
  `mapid` int(16) default NULL,
  `faction` int(32) unsigned default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `graveyards`
--


/*!40000 ALTER TABLE `graveyards` DISABLE KEYS */;
LOCK TABLES `graveyards` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `graveyards` ENABLE KEYS */;

--
-- Table structure for table `guilds`
--

DROP TABLE IF EXISTS `guilds`;
CREATE TABLE `guilds` (
  `guildid` int(6) unsigned NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `leaderguid` int(6) unsigned NOT NULL default '0',
  `EmblemStyle` int(5) unsigned NOT NULL default '0',
  `EmblemColor` int(5) unsigned NOT NULL default '0',
  `BorderStyle` int(5) unsigned NOT NULL default '0',
  `BorderColor` int(5) unsigned NOT NULL default '0',
  `BackgroundColor` int(5) unsigned NOT NULL default '0',
  `MOTD` varchar(255) NOT NULL default '',
  `createdate` datetime default NULL,
  PRIMARY KEY  (`guildid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `guilds`
--


/*!40000 ALTER TABLE `guilds` DISABLE KEYS */;
LOCK TABLES `guilds` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guilds` ENABLE KEYS */;

--
-- Table structure for table `guilds_members`
--

DROP TABLE IF EXISTS `guilds_members`;
CREATE TABLE `guilds_members` (
  `guildid` int(6) unsigned NOT NULL default '0',
  `guid` int(6) NOT NULL default '0',
  `rank` tinyint(2) unsigned NOT NULL default '0',
  `Pnote` varchar(255) NOT NULL default '',
  `OFFnote` varchar(255) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `guilds_members`
--


/*!40000 ALTER TABLE `guilds_members` DISABLE KEYS */;
LOCK TABLES `guilds_members` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guilds_members` ENABLE KEYS */;

--
-- Table structure for table `guilds_ranks`
--

DROP TABLE IF EXISTS `guilds_ranks`;
CREATE TABLE `guilds_ranks` (
  `guildid` int(6) unsigned NOT NULL default '0',
  `rname` varchar(255) NOT NULL default '',
  `rights` int(3) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `guilds_ranks`
--


/*!40000 ALTER TABLE `guilds_ranks` DISABLE KEYS */;
LOCK TABLES `guilds_ranks` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guilds_ranks` ENABLE KEYS */;

--
-- Table structure for table `homebind`
--

DROP TABLE IF EXISTS `homebind`;
CREATE TABLE `homebind` (
  `id` int(255) NOT NULL auto_increment,
  `guid` int(6) unsigned NOT NULL default '0',
  `mapid` int(11) NOT NULL default '0',
  `zoneid` int(11) NOT NULL default '0',
  `positionx` float NOT NULL default '0',
  `positiony` float NOT NULL default '0',
  `positionz` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `homebind`
--


/*!40000 ALTER TABLE `homebind` DISABLE KEYS */;
LOCK TABLES `homebind` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `homebind` ENABLE KEYS */;

--
-- Table structure for table `inventory`
--

DROP TABLE IF EXISTS `inventory`;
CREATE TABLE `inventory` (
  `player_guid` bigint(20) NOT NULL default '0',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `item_guid` bigint(20) NOT NULL default '0',
  `item_id` int(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`item_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `inventory`
--


/*!40000 ALTER TABLE `inventory` DISABLE KEYS */;
LOCK TABLES `inventory` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `inventory` ENABLE KEYS */;

--
-- Table structure for table `ipbantable`
--

DROP TABLE IF EXISTS `ipbantable`;
CREATE TABLE `ipbantable` (
  `ip` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`ip`),
  UNIQUE KEY `ip` (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `ipbantable`
--


/*!40000 ALTER TABLE `ipbantable` DISABLE KEYS */;
LOCK TABLES `ipbantable` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `ipbantable` ENABLE KEYS */;

--
-- Table structure for table `item_instances`
--

DROP TABLE IF EXISTS `item_instances`;
CREATE TABLE `item_instances` (
  `guid` bigint(20) NOT NULL default '0',
  `data` longtext NOT NULL,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `item_instances`
--


/*!40000 ALTER TABLE `item_instances` DISABLE KEYS */;
LOCK TABLES `item_instances` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_instances` ENABLE KEYS */;

--
-- Table structure for table `item_pages`
--

DROP TABLE IF EXISTS `item_pages`;
CREATE TABLE `item_pages` (
  `id` int(11) NOT NULL default '0',
  `text` longtext NOT NULL,
  `next_page` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `item_pages_index` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `item_pages`
--


/*!40000 ALTER TABLE `item_pages` DISABLE KEYS */;
LOCK TABLES `item_pages` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_pages` ENABLE KEYS */;

--
-- Table structure for table `itemstemplate`
--

DROP TABLE IF EXISTS `itemstemplate`;
CREATE TABLE `itemstemplate` (
  `entry` int(255) unsigned NOT NULL default '0',
  `class` int(30) unsigned NOT NULL default '0',
  `subclass` int(30) unsigned NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `name2` varchar(255) NOT NULL default '',
  `name3` varchar(255) NOT NULL default '',
  `name4` varchar(255) NOT NULL default '',
  `displayid` int(70) unsigned NOT NULL default '0',
  `Quality` int(30) unsigned NOT NULL default '0',
  `Flags` int(30) unsigned NOT NULL default '0',
  `BuyPrice` int(30) unsigned NOT NULL default '0',
  `SellPrice` int(30) unsigned NOT NULL default '0',
  `InventoryType` int(30) unsigned NOT NULL default '0',
  `AllowableClass` int(30) unsigned NOT NULL default '0',
  `AllowableRace` int(30) unsigned NOT NULL default '0',
  `ItemLevel` int(30) unsigned NOT NULL default '0',
  `RequiredLevel` int(30) unsigned NOT NULL default '0',
  `RequiredSkill` int(30) unsigned NOT NULL default '0',
  `RequiredSkillRank` int(30) unsigned NOT NULL default '0',
  `requiredspell` int(30) unsigned NOT NULL default '0',
  `requiredhonorrank` int(30) unsigned NOT NULL default '0',
  `RequiredCityRank` int(30) unsigned NOT NULL default '0',
  `RequiredReputationFaction` int(30) unsigned NOT NULL default '0',
  `RequiredRaputationRank` int(30) unsigned NOT NULL default '0',
  `maxcount` int(30) unsigned NOT NULL default '0',
  `stackable` int(30) unsigned NOT NULL default '0',
  `ContainerSlots` int(30) unsigned NOT NULL default '0',
  `stat_type1` int(30) unsigned NOT NULL default '0',
  `stat_value1` int(30) unsigned NOT NULL default '0',
  `stat_type2` int(30) unsigned NOT NULL default '0',
  `stat_value2` int(30) unsigned NOT NULL default '0',
  `stat_type3` int(30) unsigned NOT NULL default '0',
  `stat_value3` int(30) unsigned NOT NULL default '0',
  `stat_type4` int(30) unsigned NOT NULL default '0',
  `stat_value4` int(30) unsigned NOT NULL default '0',
  `stat_type5` int(30) unsigned NOT NULL default '0',
  `stat_value5` int(30) unsigned NOT NULL default '0',
  `stat_type6` int(30) unsigned NOT NULL default '0',
  `stat_value6` int(30) unsigned NOT NULL default '0',
  `stat_type7` int(30) unsigned NOT NULL default '0',
  `stat_value7` int(30) unsigned NOT NULL default '0',
  `stat_type8` int(30) unsigned NOT NULL default '0',
  `stat_value8` int(30) unsigned NOT NULL default '0',
  `stat_type9` int(30) unsigned NOT NULL default '0',
  `stat_value9` int(30) unsigned NOT NULL default '0',
  `stat_type10` int(30) unsigned NOT NULL default '0',
  `stat_value10` int(30) unsigned NOT NULL default '0',
  `dmg_min1` int(30) unsigned NOT NULL default '0',
  `dmg_max1` int(30) unsigned NOT NULL default '0',
  `dmg_type1` int(30) unsigned NOT NULL default '0',
  `dmg_min2` int(30) unsigned NOT NULL default '0',
  `dmg_max2` int(30) unsigned NOT NULL default '0',
  `dmg_type2` int(30) unsigned NOT NULL default '0',
  `dmg_min3` int(30) unsigned NOT NULL default '0',
  `dmg_max3` int(30) unsigned NOT NULL default '0',
  `dmg_type3` int(30) unsigned NOT NULL default '0',
  `dmg_min4` int(30) unsigned NOT NULL default '0',
  `dmg_max4` int(30) unsigned NOT NULL default '0',
  `dmg_type4` int(30) unsigned NOT NULL default '0',
  `dmg_min5` int(30) unsigned NOT NULL default '0',
  `dmg_max5` int(30) unsigned NOT NULL default '0',
  `dmg_type5` int(30) unsigned NOT NULL default '0',
  `armor` int(30) unsigned NOT NULL default '0',
  `holy_res` int(30) unsigned NOT NULL default '0',
  `fire_res` int(30) unsigned NOT NULL default '0',
  `nature_res` int(30) unsigned NOT NULL default '0',
  `frost_res` int(30) unsigned NOT NULL default '0',
  `shadow_res` int(30) unsigned NOT NULL default '0',
  `arcane_res` int(30) unsigned NOT NULL default '0',
  `delay` int(11) unsigned NOT NULL default '1000',
  `ammo_type` int(30) unsigned NOT NULL default '0',
  `spellid_1` int(30) unsigned NOT NULL default '0',
  `spelltrigger_1` int(30) unsigned NOT NULL default '0',
  `spellcharges_1` int(30) unsigned NOT NULL default '0',
  `spellcooldown_1` int(30) unsigned NOT NULL default '0',
  `spellcategory_1` int(30) unsigned NOT NULL default '0',
  `spellcategorycooldown_1` int(30) unsigned NOT NULL default '0',
  `spellid_2` int(30) unsigned NOT NULL default '0',
  `spelltrigger_2` int(30) unsigned NOT NULL default '0',
  `spellcharges_2` int(30) unsigned NOT NULL default '0',
  `spellcooldown_2` int(30) unsigned NOT NULL default '0',
  `spellcategory_2` int(30) unsigned NOT NULL default '0',
  `spellcategorycooldown_2` int(30) unsigned NOT NULL default '0',
  `spellid_3` int(30) unsigned NOT NULL default '0',
  `spelltrigger_3` int(30) unsigned NOT NULL default '0',
  `spellcharges_3` int(30) unsigned NOT NULL default '0',
  `spellcooldown_3` int(30) unsigned NOT NULL default '0',
  `spellcategory_3` int(30) unsigned NOT NULL default '0',
  `spellcategorycooldown_3` int(30) unsigned NOT NULL default '0',
  `spellid_4` int(30) unsigned NOT NULL default '0',
  `spelltrigger_4` int(30) unsigned NOT NULL default '0',
  `spellcharges_4` int(30) unsigned NOT NULL default '0',
  `spellcooldown_4` int(30) unsigned NOT NULL default '0',
  `spellcategory_4` int(30) unsigned NOT NULL default '0',
  `spellcategorycooldown_4` int(30) unsigned NOT NULL default '0',
  `spellid_5` int(30) unsigned NOT NULL default '0',
  `spelltrigger_5` int(30) unsigned NOT NULL default '0',
  `spellcharges_5` int(30) unsigned NOT NULL default '0',
  `spellcooldown_5` int(30) unsigned NOT NULL default '0',
  `spellcategory_5` int(30) unsigned NOT NULL default '0',
  `spellcategorycooldown_5` int(30) unsigned NOT NULL default '0',
  `bonding` int(30) unsigned NOT NULL default '0',
  `description` varchar(255) NOT NULL default '',
  `PageText` int(30) unsigned NOT NULL default '0',
  `LanguageID` int(30) unsigned NOT NULL default '0',
  `PageMaterial` int(30) unsigned NOT NULL default '0',
  `startquest` int(30) unsigned NOT NULL default '0',
  `lockid` int(30) unsigned NOT NULL default '0',
  `Material` int(30) unsigned NOT NULL default '0',
  `sheath` int(30) unsigned NOT NULL default '0',
  `Extra` int(30) unsigned NOT NULL default '0',
  `block` int(30) unsigned NOT NULL default '0',
  `itemset` int(30) unsigned NOT NULL default '0',
  `MaxDurability` int(30) unsigned NOT NULL default '0',
  `area` int(30) unsigned NOT NULL default '0',
  `ScriptName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`entry`),
  KEY `items_index` (`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `itemstemplate`
--


/*!40000 ALTER TABLE `itemstemplate` DISABLE KEYS */;
LOCK TABLES `itemstemplate` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `itemstemplate` ENABLE KEYS */;

--
-- Table structure for table `kills`
--

DROP TABLE IF EXISTS `kills`;
CREATE TABLE `kills` (
  `killerID` int(32) NOT NULL default '0',
  `victimID` int(32) NOT NULL default '0',
  `honor_pts` float NOT NULL default '0',
  `date` int(32) NOT NULL default '0',
  `type` smallint(5) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `kills`
--


/*!40000 ALTER TABLE `kills` DISABLE KEYS */;
LOCK TABLES `kills` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `kills` ENABLE KEYS */;

--
-- Table structure for table `loottemplate`
--

DROP TABLE IF EXISTS `loottemplate`;
CREATE TABLE `loottemplate` (
  `entry` int(11) unsigned NOT NULL default '0',
  `itemid` int(11) unsigned NOT NULL default '0',
  `percentchance` float NOT NULL default '100',
  KEY `i_creature_loot_creatureid` (`entry`),
  KEY `creatureloot_index` (`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `loottemplate`
--


/*!40000 ALTER TABLE `loottemplate` DISABLE KEYS */;
LOCK TABLES `loottemplate` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `loottemplate` ENABLE KEYS */;

--
-- Table structure for table `mail`
--

DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `mailid` bigint(20) unsigned NOT NULL default '0',
  `sender` bigint(20) unsigned NOT NULL default '0',
  `reciever` bigint(20) unsigned NOT NULL default '0',
  `subject` longtext,
  `body` longtext,
  `item` bigint(20) unsigned NOT NULL default '0',
  `time` bigint(20) unsigned NOT NULL default '0',
  `money` bigint(20) unsigned NOT NULL default '0',
  `COD` bigint(20) unsigned NOT NULL default '0',
  `checked` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`mailid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `mail`
--


/*!40000 ALTER TABLE `mail` DISABLE KEYS */;
LOCK TABLES `mail` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `mail` ENABLE KEYS */;

--
-- Table structure for table `mailed_items`
--

DROP TABLE IF EXISTS `mailed_items`;
CREATE TABLE `mailed_items` (
  `guid` bigint(20) NOT NULL default '0',
  `data` longtext NOT NULL,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `mailed_items`
--


/*!40000 ALTER TABLE `mailed_items` DISABLE KEYS */;
LOCK TABLES `mailed_items` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `mailed_items` ENABLE KEYS */;

--
-- Table structure for table `npc_gossip`
--

DROP TABLE IF EXISTS `npc_gossip`;
CREATE TABLE `npc_gossip` (
  `ID` int(11) NOT NULL default '0',
  `NPC_GUID` int(11) NOT NULL default '0',
  `GOSSIP_TYPE` int(11) NOT NULL default '0',
  `TEXTID` int(30) NOT NULL default '0',
  `OPTION_COUNT` int(30) default NULL,
  PRIMARY KEY  (`ID`,`NPC_GUID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_gossip`
--


/*!40000 ALTER TABLE `npc_gossip` DISABLE KEYS */;
LOCK TABLES `npc_gossip` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_gossip` ENABLE KEYS */;

--
-- Table structure for table `npc_options`
--

DROP TABLE IF EXISTS `npc_options`;
CREATE TABLE `npc_options` (
  `ID` int(11) NOT NULL default '0',
  `GOSSIP_ID` int(11) NOT NULL default '0',
  `TYPE` int(5) default NULL,
  `OPTION` text NOT NULL,
  `NPC_TEXT_NEXTID` int(11) default '0',
  `SPECIAL` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_options`
--


/*!40000 ALTER TABLE `npc_options` DISABLE KEYS */;
LOCK TABLES `npc_options` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_options` ENABLE KEYS */;

--
-- Table structure for table `npc_text`
--

DROP TABLE IF EXISTS `npc_text`;
CREATE TABLE `npc_text` (
  `ID` int(11) NOT NULL default '0',
  `text0_0` longtext NOT NULL,
  `text0_1` longtext NOT NULL,
  `lang0` bigint(20) unsigned NOT NULL default '0',
  `prob0` float NOT NULL default '0',
  `em0_0` bigint(20) unsigned NOT NULL default '0',
  `em0_1` bigint(20) unsigned NOT NULL default '0',
  `em0_2` bigint(20) unsigned NOT NULL default '0',
  `em0_3` bigint(20) unsigned NOT NULL default '0',
  `em0_4` bigint(20) unsigned NOT NULL default '0',
  `em0_5` bigint(20) unsigned NOT NULL default '0',
  `text1_0` longtext NOT NULL,
  `text1_1` longtext NOT NULL,
  `lang1` bigint(20) unsigned NOT NULL default '0',
  `prob1` float NOT NULL default '0',
  `em1_0` bigint(20) unsigned NOT NULL default '0',
  `em1_1` bigint(20) unsigned NOT NULL default '0',
  `em1_2` bigint(20) unsigned NOT NULL default '0',
  `em1_3` bigint(20) unsigned NOT NULL default '0',
  `em1_4` bigint(20) unsigned NOT NULL default '0',
  `em1_5` bigint(20) unsigned NOT NULL default '0',
  `text2_0` longtext NOT NULL,
  `text2_1` longtext NOT NULL,
  `lang2` bigint(20) unsigned NOT NULL default '0',
  `prob2` float NOT NULL default '0',
  `em2_0` bigint(20) unsigned NOT NULL default '0',
  `em2_1` bigint(20) unsigned NOT NULL default '0',
  `em2_2` bigint(20) unsigned NOT NULL default '0',
  `em2_3` bigint(20) unsigned NOT NULL default '0',
  `em2_4` bigint(20) unsigned NOT NULL default '0',
  `em2_5` bigint(20) unsigned NOT NULL default '0',
  `text3_0` longtext NOT NULL,
  `text3_1` longtext NOT NULL,
  `lang3` bigint(20) unsigned NOT NULL default '0',
  `prob3` float NOT NULL default '0',
  `em3_0` bigint(20) unsigned NOT NULL default '0',
  `em3_1` bigint(20) unsigned NOT NULL default '0',
  `em3_2` bigint(20) unsigned NOT NULL default '0',
  `em3_3` bigint(20) unsigned NOT NULL default '0',
  `em3_4` bigint(20) unsigned NOT NULL default '0',
  `em3_5` bigint(20) unsigned NOT NULL default '0',
  `text4_0` longtext NOT NULL,
  `text4_1` longtext NOT NULL,
  `lang4` bigint(20) unsigned NOT NULL default '0',
  `prob4` float NOT NULL default '0',
  `em4_0` bigint(20) unsigned NOT NULL default '0',
  `em4_1` bigint(20) unsigned NOT NULL default '0',
  `em4_2` bigint(20) unsigned NOT NULL default '0',
  `em4_3` bigint(20) unsigned NOT NULL default '0',
  `em4_4` bigint(20) unsigned NOT NULL default '0',
  `em4_5` bigint(20) unsigned NOT NULL default '0',
  `text5_0` longtext NOT NULL,
  `text5_1` longtext NOT NULL,
  `lang5` bigint(20) unsigned NOT NULL default '0',
  `prob5` float NOT NULL default '0',
  `em5_0` bigint(20) unsigned NOT NULL default '0',
  `em5_1` bigint(20) unsigned NOT NULL default '0',
  `em5_2` bigint(20) unsigned NOT NULL default '0',
  `em5_3` bigint(20) unsigned NOT NULL default '0',
  `em5_4` bigint(20) unsigned NOT NULL default '0',
  `em5_5` bigint(20) unsigned NOT NULL default '0',
  `text6_0` longtext NOT NULL,
  `text6_1` longtext NOT NULL,
  `lang6` bigint(20) unsigned NOT NULL default '0',
  `prob6` float NOT NULL default '0',
  `em6_0` bigint(20) unsigned NOT NULL default '0',
  `em6_1` bigint(20) unsigned NOT NULL default '0',
  `em6_2` bigint(20) unsigned NOT NULL default '0',
  `em6_3` bigint(20) unsigned NOT NULL default '0',
  `em6_4` bigint(20) unsigned NOT NULL default '0',
  `em6_5` bigint(20) unsigned NOT NULL default '0',
  `text7_0` longtext NOT NULL,
  `text7_1` longtext NOT NULL,
  `lang7` bigint(20) unsigned NOT NULL default '0',
  `prob7` float NOT NULL default '0',
  `em7_0` bigint(20) unsigned NOT NULL default '0',
  `em7_1` bigint(20) unsigned NOT NULL default '0',
  `em7_2` bigint(20) unsigned NOT NULL default '0',
  `em7_3` bigint(20) unsigned NOT NULL default '0',
  `em7_4` bigint(20) unsigned NOT NULL default '0',
  `em7_5` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_text`
--


/*!40000 ALTER TABLE `npc_text` DISABLE KEYS */;
LOCK TABLES `npc_text` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_text` ENABLE KEYS */;

--
-- Table structure for table `objectinvolvedrelation`
--

DROP TABLE IF EXISTS `objectinvolvedrelation`;
CREATE TABLE `objectinvolvedrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `questId` bigint(20) unsigned NOT NULL default '0',
  `objectId` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `objectinvolvedrelation`
--


/*!40000 ALTER TABLE `objectinvolvedrelation` DISABLE KEYS */;
LOCK TABLES `objectinvolvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `objectinvolvedrelation` ENABLE KEYS */;

--
-- Table structure for table `objectquestrelation`
--

DROP TABLE IF EXISTS `objectquestrelation`;
CREATE TABLE `objectquestrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `questId` bigint(20) unsigned NOT NULL default '0',
  `objectId` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `objectquestrelation`
--


/*!40000 ALTER TABLE `objectquestrelation` DISABLE KEYS */;
LOCK TABLES `objectquestrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `objectquestrelation` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo`
--

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE `playercreateinfo` (
  `createId` tinyint(3) unsigned NOT NULL auto_increment,
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `mapid` mediumint(8) unsigned NOT NULL default '0',
  `zoneid` mediumint(8) unsigned NOT NULL default '0',
  `positionx` float NOT NULL default '0',
  `positiony` float NOT NULL default '0',
  `positionz` float NOT NULL default '0',
  `displayID` smallint(5) unsigned NOT NULL default '0',
  `BaseStrength` tinyint(3) unsigned NOT NULL default '0',
  `BaseAgility` tinyint(3) unsigned NOT NULL default '0',
  `BaseStamina` tinyint(3) unsigned NOT NULL default '0',
  `BaseIntellect` tinyint(3) unsigned NOT NULL default '0',
  `BaseSpirit` tinyint(3) unsigned NOT NULL default '0',
  `BaseArmor` mediumint(8) unsigned NOT NULL default '0',
  `BaseHealth` mediumint(8) unsigned NOT NULL default '0',
  `BaseMana` mediumint(8) unsigned NOT NULL default '0',
  `BaseRage` mediumint(8) unsigned NOT NULL default '0',
  `BaseFocus` mediumint(8) unsigned NOT NULL default '0',
  `BaseEnergy` mediumint(8) unsigned NOT NULL default '0',
  `attackpower` mediumint(8) unsigned NOT NULL default '0',
  `mindmg` float NOT NULL default '0',
  `maxdmg` float NOT NULL default '0',
  `ranmindmg` float NOT NULL default '0',
  `ranmaxdmg` float NOT NULL default '0',
  PRIMARY KEY  (`createId`),
  KEY `playercreateinfo_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo`
--


/*!40000 ALTER TABLE `playercreateinfo` DISABLE KEYS */;
LOCK TABLES `playercreateinfo` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_actions`
--

DROP TABLE IF EXISTS `playercreateinfo_actions`;
CREATE TABLE `playercreateinfo_actions` (
  `createid` tinyint(3) unsigned NOT NULL default '0',
  `button` smallint(2) unsigned NOT NULL default '0',
  `action` smallint(6) unsigned NOT NULL default '0',
  `type` smallint(3) unsigned NOT NULL default '0',
  `misc` smallint(3) unsigned NOT NULL default '0',
  KEY `playercreateinfo_actions_index` (`button`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_actions`
--


/*!40000 ALTER TABLE `playercreateinfo_actions` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_actions` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_actions` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_items`
--

DROP TABLE IF EXISTS `playercreateinfo_items`;
CREATE TABLE `playercreateinfo_items` (
  `createid` tinyint(3) unsigned NOT NULL default '0',
  `itemid` mediumint(8) unsigned NOT NULL default '0',
  `bagIndex` tinyint(3) unsigned NOT NULL default '255',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `amount` tinyint(8) unsigned NOT NULL default '1',
  KEY `playercreateinfo_items_index` (`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_items`
--


/*!40000 ALTER TABLE `playercreateinfo_items` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_items` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_items` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_reputation`
--

DROP TABLE IF EXISTS `playercreateinfo_reputation`;
CREATE TABLE `playercreateinfo_reputation` (
  `createid` tinyint(3) unsigned NOT NULL default '0',
  `slot` smallint(2) unsigned NOT NULL default '0',
  `faction` smallint(6) unsigned NOT NULL default '0',
  `reputation` smallint(3) unsigned NOT NULL default '0',
  KEY `playercreateinfo_reputation_index` (`faction`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_reputation`
--


/*!40000 ALTER TABLE `playercreateinfo_reputation` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_reputation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_reputation` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_skills`
--

DROP TABLE IF EXISTS `playercreateinfo_skills`;
CREATE TABLE `playercreateinfo_skills` (
  `createid` smallint(5) unsigned NOT NULL default '0',
  `Skill` mediumint(8) unsigned NOT NULL default '0',
  `SkillMin` smallint(5) unsigned NOT NULL default '0',
  `SkillMax` smallint(5) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_skills`
--


/*!40000 ALTER TABLE `playercreateinfo_skills` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_skills` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_skills` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_spells`
--

DROP TABLE IF EXISTS `playercreateinfo_spells`;
CREATE TABLE `playercreateinfo_spells` (
  `createid` smallint(5) unsigned NOT NULL default '0',
  `Spell` bigint(20) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_spells`
--


/*!40000 ALTER TABLE `playercreateinfo_spells` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_spells` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_spells` ENABLE KEYS */;

--
-- Table structure for table `quests`
--

DROP TABLE IF EXISTS `quests`;
CREATE TABLE `quests` (
  `questId` bigint(20) NOT NULL default '0',
  `zoneId` bigint(20) NOT NULL default '0',
  `questFlags` bigint(20) NOT NULL default '0',
  `title` longtext,
  `details` longtext,
  `objectives` longtext,
  `completedText` longtext,
  `incompleteText` longtext,
  `secondText` longtext,
  `partText1` longtext,
  `partText2` longtext,
  `partText3` longtext,
  `partText4` longtext,
  `reqLevel` bigint(20) NOT NULL default '0',
  `questLevel` bigint(20) NOT NULL default '0',
  `prevQuests` bigint(20) NOT NULL default '0',
  `previousQuest1` bigint(20) NOT NULL default '0',
  `previousQuest2` bigint(20) NOT NULL default '0',
  `previousQuest3` bigint(20) NOT NULL default '0',
  `previousQuest4` bigint(20) NOT NULL default '0',
  `previousQuest5` bigint(20) NOT NULL default '0',
  `previousQuest6` bigint(20) NOT NULL default '0',
  `previousQuest7` bigint(20) NOT NULL default '0',
  `previousQuest8` bigint(20) NOT NULL default '0',
  `previousQuest9` bigint(20) NOT NULL default '0',
  `previousQuest10` bigint(20) NOT NULL default '0',
  `lprevQuests` bigint(20) NOT NULL default '0',
  `lpreviousQuest1` bigint(20) NOT NULL default '0',
  `lpreviousQuest2` bigint(20) NOT NULL default '0',
  `lpreviousQuest3` bigint(20) NOT NULL default '0',
  `lpreviousQuest4` bigint(20) NOT NULL default '0',
  `lpreviousQuest5` bigint(20) NOT NULL default '0',
  `lpreviousQuest6` bigint(20) NOT NULL default '0',
  `lpreviousQuest7` bigint(20) NOT NULL default '0',
  `lpreviousQuest8` bigint(20) NOT NULL default '0',
  `lpreviousQuest9` bigint(20) NOT NULL default '0',
  `lpreviousQuest10` bigint(20) NOT NULL default '0',
  `lockQuests` bigint(20) NOT NULL default '0',
  `lockQuest1` bigint(20) NOT NULL default '0',
  `lockQuest2` bigint(20) NOT NULL default '0',
  `lockQuest3` bigint(20) NOT NULL default '0',
  `lockQuest4` bigint(20) NOT NULL default '0',
  `lockQuest5` bigint(20) NOT NULL default '0',
  `lockQuest6` bigint(20) NOT NULL default '0',
  `lockQuest7` bigint(20) NOT NULL default '0',
  `lockQuest8` bigint(20) NOT NULL default '0',
  `lockQuest9` bigint(20) NOT NULL default '0',
  `lockQuest10` bigint(20) NOT NULL default '0',
  `questItemId1` bigint(20) NOT NULL default '0',
  `questItemId2` bigint(20) NOT NULL default '0',
  `questItemId3` bigint(20) NOT NULL default '0',
  `questItemId4` bigint(20) NOT NULL default '0',
  `questItemCount1` bigint(20) NOT NULL default '0',
  `questItemCount2` bigint(20) NOT NULL default '0',
  `questItemCount3` bigint(20) NOT NULL default '0',
  `questItemCount4` bigint(20) NOT NULL default '0',
  `questMobId1` bigint(20) NOT NULL default '0',
  `questMobId2` bigint(20) NOT NULL default '0',
  `questMobId3` bigint(20) NOT NULL default '0',
  `questMobId4` bigint(20) NOT NULL default '0',
  `questMobCount1` bigint(20) NOT NULL default '0',
  `questMobCount2` bigint(20) NOT NULL default '0',
  `questMobCount3` bigint(20) NOT NULL default '0',
  `questMobCount4` bigint(20) NOT NULL default '0',
  `choiceRewards` bigint(20) NOT NULL default '0',
  `choiceItemId1` bigint(20) NOT NULL default '0',
  `choiceItemId2` bigint(20) NOT NULL default '0',
  `choiceItemId3` bigint(20) NOT NULL default '0',
  `choiceItemId4` bigint(20) NOT NULL default '0',
  `choiceItemId5` bigint(20) NOT NULL default '0',
  `choiceItemId6` bigint(20) NOT NULL default '0',
  `choiceItemCount1` bigint(20) NOT NULL default '0',
  `choiceItemCount2` bigint(20) NOT NULL default '0',
  `choiceItemCount3` bigint(20) NOT NULL default '0',
  `choiceItemCount4` bigint(20) NOT NULL default '0',
  `choiceItemCount5` bigint(20) NOT NULL default '0',
  `choiceItemCount6` bigint(20) NOT NULL default '0',
  `itemRewards` bigint(20) NOT NULL default '0',
  `rewardItemId1` bigint(20) NOT NULL default '0',
  `rewardItemId2` bigint(20) NOT NULL default '0',
  `rewardItemId3` bigint(20) NOT NULL default '0',
  `rewardItemId4` bigint(20) NOT NULL default '0',
  `rewardItemCount1` bigint(20) NOT NULL default '0',
  `rewardItemCount2` bigint(20) NOT NULL default '0',
  `rewardItemCount3` bigint(20) NOT NULL default '0',
  `rewardItemCount4` bigint(20) NOT NULL default '0',
  `rewardGold` bigint(20) NOT NULL default '0',
  `repFaction1` bigint(20) NOT NULL default '0',
  `repFaction2` bigint(20) NOT NULL default '0',
  `repValue1` bigint(20) NOT NULL default '0',
  `repValue2` bigint(20) NOT NULL default '0',
  `srcItem` bigint(20) NOT NULL default '0',
  `nextQuest` bigint(20) NOT NULL default '0',
  `learnSpell` bigint(20) NOT NULL default '0',
  `timeMinutes` bigint(20) NOT NULL default '0',
  `questType` bigint(20) NOT NULL default '0',
  `questRaces` bigint(20) NOT NULL default '0',
  `questClass` bigint(20) NOT NULL default '0',
  `questTrSkill` bigint(20) NOT NULL default '0',
  `questBehavior` bigint(20) NOT NULL default '0',
  `locationid` bigint(20) NOT NULL default '0',
  `locationx` float NOT NULL default '0',
  `locationy` float NOT NULL default '0',
  `locationz` float NOT NULL default '0',
  PRIMARY KEY  (`questId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `quests`
--


/*!40000 ALTER TABLE `quests` DISABLE KEYS */;
LOCK TABLES `quests` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `quests` ENABLE KEYS */;

--
-- Table structure for table `queststatus`
--

DROP TABLE IF EXISTS `queststatus`;
CREATE TABLE `queststatus` (
  `playerId` bigint(20) unsigned NOT NULL default '0',
  `questId` bigint(22) unsigned NOT NULL default '0',
  `status` bigint(20) unsigned NOT NULL default '0',
  `rewarded` bigint(20) unsigned NOT NULL default '0',
  `questMobCount1` bigint(20) unsigned NOT NULL default '0',
  `questMobCount2` bigint(20) unsigned NOT NULL default '0',
  `questMobCount3` bigint(20) unsigned NOT NULL default '0',
  `questMobCount4` bigint(20) unsigned NOT NULL default '0',
  `questItemCount1` bigint(20) unsigned NOT NULL default '0',
  `questItemCount2` bigint(20) unsigned NOT NULL default '0',
  `questItemCount3` bigint(20) unsigned NOT NULL default '0',
  `questItemCount4` bigint(20) unsigned NOT NULL default '0',
  `timer` bigint(20) unsigned NOT NULL default '0',
  `explored` bigint(20) unsigned NOT NULL default '0',
  `id` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `queststatus`
--


/*!40000 ALTER TABLE `queststatus` DISABLE KEYS */;
LOCK TABLES `queststatus` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `queststatus` ENABLE KEYS */;

--
-- Table structure for table `realms`
--

DROP TABLE IF EXISTS `realms`;
CREATE TABLE `realms` (
  `id` bigint(20) NOT NULL auto_increment,
  `name` varchar(32) NOT NULL default '',
  `address` varchar(32) NOT NULL default '',
  `icon` int(10) default '0',
  `color` int(10) default '0',
  `timezone` int(10) default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `realms`
--


/*!40000 ALTER TABLE `realms` DISABLE KEYS */;
LOCK TABLES `realms` WRITE;
INSERT INTO `realms` VALUES (1,'MaNGOS','127.0.0.1:8129',1,0,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `realms` ENABLE KEYS */;

--
-- Table structure for table `reputation`
--

DROP TABLE IF EXISTS `reputation`;
CREATE TABLE `reputation` (
  `ID` int(32) NOT NULL auto_increment,
  `playerID` int(32) NOT NULL default '0',
  `factionID` int(32) NOT NULL default '0',
  `reputationID` int(32) NOT NULL default '0',
  `standing` int(32) NOT NULL default '0',
  `flags` int(32) NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `reputation`
--


/*!40000 ALTER TABLE `reputation` DISABLE KEYS */;
LOCK TABLES `reputation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `reputation` ENABLE KEYS */;

--
-- Table structure for table `social`
--

DROP TABLE IF EXISTS `social`;
CREATE TABLE `social` (
  `charname` varchar(21) NOT NULL default '',
  `guid` int(6) NOT NULL default '0',
  `friendid` int(6) NOT NULL default '0',
  `flags` varchar(21) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `social`
--


/*!40000 ALTER TABLE `social` DISABLE KEYS */;
LOCK TABLES `social` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `social` ENABLE KEYS */;

--
-- Table structure for table `spells`
--

DROP TABLE IF EXISTS `spells`;
CREATE TABLE `spells` (
  `Id` int(11) NOT NULL auto_increment,
  `learn` int(11) unsigned NOT NULL default '0',
  `trigger_spell` int(11) unsigned NOT NULL default '0',
  `create_item` int(11) unsigned NOT NULL default '0',
  `craft_skill` int(11) unsigned NOT NULL default '0',
  `name` char(255) default NULL,
  `rank` char(64) default NULL,
  `description` char(255) default NULL,
  PRIMARY KEY  (`Id`),
  KEY `spell_index` (`name`,`rank`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spells`
--


/*!40000 ALTER TABLE `spells` DISABLE KEYS */;
LOCK TABLES `spells` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `spells` ENABLE KEYS */;

--
-- Table structure for table `spirithealers`
--

DROP TABLE IF EXISTS `spirithealers`;
CREATE TABLE `spirithealers` (
  `X` float default NULL,
  `Y` float default NULL,
  `Z` float default NULL,
  `F` float default NULL,
  `name_id` int(8) default NULL,
  `zoneId` int(16) default NULL,
  `mapId` int(16) default NULL,
  `faction_id` int(32) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spirithealers`
--


/*!40000 ALTER TABLE `spirithealers` DISABLE KEYS */;
LOCK TABLES `spirithealers` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `spirithealers` ENABLE KEYS */;

--
-- Table structure for table `talents`
--

DROP TABLE IF EXISTS `talents`;
CREATE TABLE `talents` (
  `id` int(10) NOT NULL auto_increment,
  `t_id` int(10) NOT NULL default '0',
  `maxrank` int(7) NOT NULL default '0',
  `class` int(10) NOT NULL default '0',
  `rank1` longtext NOT NULL,
  `rank2` longtext NOT NULL,
  `rank3` longtext NOT NULL,
  `rank4` longtext NOT NULL,
  `rank5` longtext NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `talents_index` (`t_id`,`maxrank`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `talents`
--


/*!40000 ALTER TABLE `talents` DISABLE KEYS */;
LOCK TABLES `talents` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `talents` ENABLE KEYS */;

--
-- Table structure for table `tavern`
--

DROP TABLE IF EXISTS `tavern`;
CREATE TABLE `tavern` (
  `triggerid` int(20) NOT NULL auto_increment,
  `Triggername` text NOT NULL,
  PRIMARY KEY  (`triggerid`),
  UNIQUE KEY `acct` (`triggerid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `tavern`
--


/*!40000 ALTER TABLE `tavern` DISABLE KEYS */;
LOCK TABLES `tavern` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `tavern` ENABLE KEYS */;

--
-- Table structure for table `taxinodes`
--

DROP TABLE IF EXISTS `taxinodes`;
CREATE TABLE `taxinodes` (
  `ID` tinyint(3) unsigned NOT NULL auto_increment,
  `continent` tinyint(3) unsigned NOT NULL default '0',
  `x` float default NULL,
  `y` float default NULL,
  `z` float default NULL,
  `name` varchar(255) default NULL,
  `flags` mediumint(11) unsigned default NULL,
  `mount` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`),
  KEY `taxinodes_index` (`continent`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `taxinodes`
--


/*!40000 ALTER TABLE `taxinodes` DISABLE KEYS */;
LOCK TABLES `taxinodes` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxinodes` ENABLE KEYS */;

--
-- Table structure for table `taxipath`
--

DROP TABLE IF EXISTS `taxipath`;
CREATE TABLE `taxipath` (
  `ID` smallint(5) unsigned NOT NULL default '0',
  `source` tinyint(3) unsigned default NULL,
  `destination` tinyint(3) unsigned default NULL,
  `price` mediumint(8) unsigned default NULL,
  PRIMARY KEY  (`ID`),
  KEY `taxipath_index` (`source`,`destination`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `taxipath`
--


/*!40000 ALTER TABLE `taxipath` DISABLE KEYS */;
LOCK TABLES `taxipath` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxipath` ENABLE KEYS */;

--
-- Table structure for table `taxipathnodes`
--

DROP TABLE IF EXISTS `taxipathnodes`;
CREATE TABLE `taxipathnodes` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `path` smallint(5) unsigned default NULL,
  `index` tinyint(3) unsigned default NULL,
  `continent` tinyint(3) unsigned default NULL,
  `X` float default NULL,
  `Y` float default NULL,
  `Z` float default NULL,
  `unknown1` mediumint(8) unsigned default NULL,
  `unknown2` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `taxipathnodes`
--


/*!40000 ALTER TABLE `taxipathnodes` DISABLE KEYS */;
LOCK TABLES `taxipathnodes` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxipathnodes` ENABLE KEYS */;

--
-- Table structure for table `trainers`
--

DROP TABLE IF EXISTS `trainers`;
CREATE TABLE `trainers` (
  `rowid` int(11) NOT NULL default '0',
  `guid` int(11) NOT NULL default '0',
  `spell` int(11) NOT NULL default '0',
  `spellcost` int(11) default '0',
  `reqspell` int(11) default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `trainers`
--


/*!40000 ALTER TABLE `trainers` DISABLE KEYS */;
LOCK TABLES `trainers` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `trainers` ENABLE KEYS */;

--
-- Table structure for table `triggerquestrelation`
--

DROP TABLE IF EXISTS `triggerquestrelation`;
CREATE TABLE `triggerquestrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `triggerID` bigint(20) unsigned NOT NULL default '0',
  `questID` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `triggerquestrelation`
--


/*!40000 ALTER TABLE `triggerquestrelation` DISABLE KEYS */;
LOCK TABLES `triggerquestrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `triggerquestrelation` ENABLE KEYS */;

--
-- Table structure for table `tutorials`
--

DROP TABLE IF EXISTS `tutorials`;
CREATE TABLE `tutorials` (
  `playerId` bigint(20) unsigned NOT NULL default '0',
  `tut0` bigint(20) unsigned NOT NULL default '0',
  `tut1` bigint(20) unsigned NOT NULL default '0',
  `tut2` bigint(20) unsigned NOT NULL default '0',
  `tut3` bigint(20) unsigned NOT NULL default '0',
  `tut4` bigint(20) unsigned NOT NULL default '0',
  `tut5` bigint(20) unsigned NOT NULL default '0',
  `tut6` bigint(20) unsigned NOT NULL default '0',
  `tut7` bigint(20) unsigned NOT NULL default '0',
  `id` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `tutorials`
--


/*!40000 ALTER TABLE `tutorials` DISABLE KEYS */;
LOCK TABLES `tutorials` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `tutorials` ENABLE KEYS */;

--
-- Table structure for table `vendors`
--

DROP TABLE IF EXISTS `vendors`;
CREATE TABLE `vendors` (
  `entry` bigint(20) unsigned NOT NULL default '0',
  `itemguid` bigint(20) unsigned NOT NULL default '0',
  `amount` bigint(20) NOT NULL default '5',
  `index_id` bigint(20) NOT NULL auto_increment,
  PRIMARY KEY  (`index_id`),
  UNIQUE KEY `index_id` (`index_id`),
  KEY `vendor_id` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `vendors`
--


/*!40000 ALTER TABLE `vendors` DISABLE KEYS */;
LOCK TABLES `vendors` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `vendors` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

