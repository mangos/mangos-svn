-- MySQL dump 10.10
--
-- Host: localhost    Database: mangos
-- ------------------------------------------------------
-- Server version	5.0.21

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `id` bigint(20) unsigned NOT NULL auto_increment COMMENT 'Identifier',
  `username` varchar(16) NOT NULL default '',
  `password` varchar(28) NOT NULL default '',
  `gmlevel` tinyint(3) unsigned NOT NULL default '0',
  `sessionkey` longtext,
  `email` varchar(50) NOT NULL default '',
  `joindate` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `banned` tinyint(3) unsigned NOT NULL default '0',
  `last_ip` varchar(30) NOT NULL default '127.0.0.1',
  `failed_logins` int(11) unsigned NOT NULL default '0',
  `locked` tinyint(3) unsigned NOT NULL default '0',
  `last_login` timestamp NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `idx_username` (`username`),
  KEY `idx_banned` (`banned`),
  KEY `idx_gmlevel` (`gmlevel`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Account System';

--
-- Dumping data for table `account`
--


/*!40000 ALTER TABLE `account` DISABLE KEYS */;
LOCK TABLES `account` WRITE;
INSERT INTO `account` VALUES (1,'administrator','administrator',3,'','','2006-04-25 12:18:56',0,'127.0.0.1',0,0,'0000-00-00 00:00:00'),(2,'gamemaster','gamemaster',2,'','','2006-04-25 12:18:56',0,'127.0.0.1',0,0,'0000-00-00 00:00:00'),(3,'moderator','moderator',1,'','','2006-04-25 12:19:35',0,'127.0.0.1',0,0,'0000-00-00 00:00:00'),(4,'player','player',0,'','','2006-04-25 12:19:35',0,'127.0.0.1',0,0,'0000-00-00 00:00:00');
UNLOCK TABLES;
/*!40000 ALTER TABLE `account` ENABLE KEYS */;

--
-- Table structure for table `areatrigger_city`
--

DROP TABLE IF EXISTS `areatrigger_city`;
CREATE TABLE `areatrigger_city` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `name` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_city`
--


/*!40000 ALTER TABLE `areatrigger_city` DISABLE KEYS */;
LOCK TABLES `areatrigger_city` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger_city` ENABLE KEYS */;

--
-- Table structure for table `areatrigger_graveyard`
--

DROP TABLE IF EXISTS `areatrigger_graveyard`;
CREATE TABLE `areatrigger_graveyard` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `zone` int(11) unsigned NOT NULL default '0' COMMENT 'Zone Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `faction` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_graveyard`
--


/*!40000 ALTER TABLE `areatrigger_graveyard` DISABLE KEYS */;
LOCK TABLES `areatrigger_graveyard` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger_graveyard` ENABLE KEYS */;

--
-- Table structure for table `areatrigger_involvedrelation`
--

DROP TABLE IF EXISTS `areatrigger_involvedrelation`;
CREATE TABLE `areatrigger_involvedrelation` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_involvedrelation`
--


/*!40000 ALTER TABLE `areatrigger_involvedrelation` DISABLE KEYS */;
LOCK TABLES `areatrigger_involvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger_involvedrelation` ENABLE KEYS */;

--
-- Table structure for table `areatrigger_tavern`
--

DROP TABLE IF EXISTS `areatrigger_tavern`;
CREATE TABLE `areatrigger_tavern` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `name` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_tavern`
--


/*!40000 ALTER TABLE `areatrigger_tavern` DISABLE KEYS */;
LOCK TABLES `areatrigger_tavern` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger_tavern` ENABLE KEYS */;

--
-- Table structure for table `areatrigger_template`
--

DROP TABLE IF EXISTS `areatrigger_template`;
CREATE TABLE `areatrigger_template` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `target_position_x` float NOT NULL default '0',
  `target_position_y` float NOT NULL default '0',
  `target_position_z` float NOT NULL default '0',
  `target_orientation` float NOT NULL default '0',
  `target_map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `name` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_template`
--


/*!40000 ALTER TABLE `areatrigger_template` DISABLE KEYS */;
LOCK TABLES `areatrigger_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `areatrigger_template` ENABLE KEYS */;

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
-- Table structure for table `auctionhouse_bid`
--

DROP TABLE IF EXISTS `auctionhouse_bid`;
CREATE TABLE `auctionhouse_bid` (
  `bidder` int(32) NOT NULL default '0',
  `id` int(32) NOT NULL default '0',
  `amount` int(32) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `auctionhouse_bid`
--


/*!40000 ALTER TABLE `auctionhouse_bid` DISABLE KEYS */;
LOCK TABLES `auctionhouse_bid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `auctionhouse_bid` ENABLE KEYS */;

--
-- Table structure for table `auctionhouse_item`
--

DROP TABLE IF EXISTS `auctionhouse_item`;
CREATE TABLE `auctionhouse_item` (
  `guid` bigint(20) NOT NULL default '0',
  `data` longtext,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `auctionhouse_item`
--


/*!40000 ALTER TABLE `auctionhouse_item` DISABLE KEYS */;
LOCK TABLES `auctionhouse_item` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `auctionhouse_item` ENABLE KEYS */;

--
-- Table structure for table `bugreport`
--

DROP TABLE IF EXISTS `bugreport`;
CREATE TABLE `bugreport` (
  `id` int(11) NOT NULL auto_increment COMMENT 'Identifier',
  `type` varchar(255) NOT NULL default '',
  `content` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Debug System';

--
-- Dumping data for table `bugreport`
--


/*!40000 ALTER TABLE `bugreport` DISABLE KEYS */;
LOCK TABLES `bugreport` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `bugreport` ENABLE KEYS */;

--
-- Table structure for table `character`
--

DROP TABLE IF EXISTS `character`;
CREATE TABLE `character` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `realm` int(11) unsigned NOT NULL default '0' COMMENT 'Realm Identifier',
  `account` bigint(20) unsigned NOT NULL default '0' COMMENT 'Account Identifier',
  `data` longtext,
  `name` varchar(12) NOT NULL default '',
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `orientation` float NOT NULL default '0',
  `taximask` longtext,
  `online` tinyint(3) unsigned NOT NULL default '0',
  `honor` int(11) unsigned NOT NULL default '0',
  `last_week_honor` int(11) unsigned NOT NULL default '0',
  `cinematic` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`),
  KEY `idx_account` (`account`),
  KEY `idx_online` (`online`),
  FULLTEXT KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character`
--


/*!40000 ALTER TABLE `character` DISABLE KEYS */;
LOCK TABLES `character` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character` ENABLE KEYS */;

--
-- Table structure for table `character_action`
--

DROP TABLE IF EXISTS `character_action`;
CREATE TABLE `character_action` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `button` int(11) unsigned NOT NULL default '0',
  `action` int(11) unsigned NOT NULL default '0',
  `type` int(11) unsigned NOT NULL default '0',
  `misc` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`,`button`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_action`
--


/*!40000 ALTER TABLE `character_action` DISABLE KEYS */;
LOCK TABLES `character_action` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_action` ENABLE KEYS */;

--
-- Table structure for table `character_aura`
--

DROP TABLE IF EXISTS `character_aura`;
CREATE TABLE `character_aura` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `spell` int(11) unsigned NOT NULL default '0',
  `effect_index` int(11) unsigned NOT NULL default '0',
  `remaintime` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_aura`
--


/*!40000 ALTER TABLE `character_aura` DISABLE KEYS */;
LOCK TABLES `character_aura` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_aura` ENABLE KEYS */;

--
-- Table structure for table `character_homebind`
--

DROP TABLE IF EXISTS `character_homebind`;
CREATE TABLE `character_homebind` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `zone` int(11) unsigned NOT NULL default '0' COMMENT 'Zone Identifier',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_homebind`
--


/*!40000 ALTER TABLE `character_homebind` DISABLE KEYS */;
LOCK TABLES `character_homebind` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_homebind` ENABLE KEYS */;

--
-- Table structure for table `character_inventory`
--

DROP TABLE IF EXISTS `character_inventory`;
CREATE TABLE `character_inventory` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `bag` tinyint(3) unsigned NOT NULL default '0',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `item` bigint(20) unsigned NOT NULL default '0' COMMENT 'Item Global Unique Identifier',
  `item_template` int(11) unsigned NOT NULL default '0' COMMENT 'Item Identifier',
  PRIMARY KEY  (`guid`,`bag`,`slot`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_inventory`
--


/*!40000 ALTER TABLE `character_inventory` DISABLE KEYS */;
LOCK TABLES `character_inventory` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_inventory` ENABLE KEYS */;

--
-- Table structure for table `character_kill`
--

DROP TABLE IF EXISTS `character_kill`;
CREATE TABLE `character_kill` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `creature_template` int(11) unsigned NOT NULL default '0' COMMENT 'Creature Identifier',
  `honor` float NOT NULL default '0',
  `date` int(11) unsigned NOT NULL default '0',
  `type` smallint(9) unsigned NOT NULL default '0',
  KEY `idx_guid` (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_kill`
--


/*!40000 ALTER TABLE `character_kill` DISABLE KEYS */;
LOCK TABLES `character_kill` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_kill` ENABLE KEYS */;

--
-- Table structure for table `character_pet`
--

DROP TABLE IF EXISTS `character_pet`;
CREATE TABLE `character_pet` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `entry` int(11) unsigned NOT NULL default '0',
  `owner` int(11) unsigned NOT NULL default '0',
  `level` int(11) unsigned NOT NULL default '1',
  `exp` int(11) unsigned NOT NULL default '0',
  `nextlvlexp` int(11) unsigned NOT NULL default '100',
  `spell1` int(11) unsigned NOT NULL default '0',
  `spell2` int(11) unsigned NOT NULL default '0',
  `spell3` int(11) unsigned NOT NULL default '0',
  `spell4` int(11) unsigned NOT NULL default '0',
  `action` int(11) unsigned NOT NULL default '0',
  `fealty` int(11) unsigned NOT NULL default '0',
  `name` varchar(50) NOT NULL default 'pet',
  `current` tinyint(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`id`),
  KEY `owner` (`owner`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_pet`
--


/*!40000 ALTER TABLE `character_pet` DISABLE KEYS */;
LOCK TABLES `character_pet` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_pet` ENABLE KEYS */;

--
-- Table structure for table `character_queststatus`
--

DROP TABLE IF EXISTS `character_queststatus`;
CREATE TABLE `character_queststatus` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  `status` int(11) unsigned NOT NULL default '0',
  `rewarded` int(11) unsigned NOT NULL default '0',
  `explored` int(11) unsigned NOT NULL default '0',
  `timer` bigint(20) unsigned NOT NULL default '0',
  `mobcount1` int(11) unsigned NOT NULL default '0',
  `mobcount2` int(11) unsigned NOT NULL default '0',
  `mobcount3` int(11) unsigned NOT NULL default '0',
  `mobcount4` int(11) unsigned NOT NULL default '0',
  `itemcount1` int(11) unsigned NOT NULL default '0',
  `itemcount2` int(11) unsigned NOT NULL default '0',
  `itemcount3` int(11) unsigned NOT NULL default '0',
  `itemcount4` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_queststatus`
--


/*!40000 ALTER TABLE `character_queststatus` DISABLE KEYS */;
LOCK TABLES `character_queststatus` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_queststatus` ENABLE KEYS */;

--
-- Table structure for table `character_reputation`
--

DROP TABLE IF EXISTS `character_reputation`;
CREATE TABLE `character_reputation` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `faction` int(11) unsigned NOT NULL default '0',
  `reputation` int(11) unsigned NOT NULL default '0' COMMENT 'Reputation Identifier',
  `standing` int(11) NOT NULL default '0',
  `flags` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`,`faction`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_reputation`
--


/*!40000 ALTER TABLE `character_reputation` DISABLE KEYS */;
LOCK TABLES `character_reputation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_reputation` ENABLE KEYS */;

--
-- Table structure for table `character_social`
--

DROP TABLE IF EXISTS `character_social`;
CREATE TABLE `character_social` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `name` varchar(21) NOT NULL default '',
  `friend` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character Global Unique Identifier',
  `flags` varchar(21) NOT NULL default '',
  PRIMARY KEY  (`guid`,`friend`,`flags`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_social`
--


/*!40000 ALTER TABLE `character_social` DISABLE KEYS */;
LOCK TABLES `character_social` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_social` ENABLE KEYS */;

--
-- Table structure for table `character_spell`
--

DROP TABLE IF EXISTS `character_spell`;
CREATE TABLE `character_spell` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `spell` int(11) unsigned NOT NULL default '0' COMMENT 'Spell Identifier',
  `slot` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_spell`
--


/*!40000 ALTER TABLE `character_spell` DISABLE KEYS */;
LOCK TABLES `character_spell` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_spell` ENABLE KEYS */;

--
-- Table structure for table `character_ticket`
--

DROP TABLE IF EXISTS `character_ticket`;
CREATE TABLE `character_ticket` (
  `ticket_id` int(11) NOT NULL auto_increment,
  `guid` int(6) unsigned NOT NULL default '0',
  `ticket_text` varchar(255) NOT NULL default '',
  `ticket_category` int(1) NOT NULL default '0',
  PRIMARY KEY  (`ticket_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_ticket`
--


/*!40000 ALTER TABLE `character_ticket` DISABLE KEYS */;
LOCK TABLES `character_ticket` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_ticket` ENABLE KEYS */;

--
-- Table structure for table `character_tutorial`
--

DROP TABLE IF EXISTS `character_tutorial`;
CREATE TABLE `character_tutorial` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `tut0` bigint(20) unsigned NOT NULL default '0',
  `tut1` bigint(20) unsigned NOT NULL default '0',
  `tut2` bigint(20) unsigned NOT NULL default '0',
  `tut3` bigint(20) unsigned NOT NULL default '0',
  `tut4` bigint(20) unsigned NOT NULL default '0',
  `tut5` bigint(20) unsigned NOT NULL default '0',
  `tut6` bigint(20) unsigned NOT NULL default '0',
  `tut7` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_tutorial`
--


/*!40000 ALTER TABLE `character_tutorial` DISABLE KEYS */;
LOCK TABLES `character_tutorial` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_tutorial` ENABLE KEYS */;

--
-- Table structure for table `command`
--

DROP TABLE IF EXISTS `command`;
CREATE TABLE `command` (
  `name` varchar(100) NOT NULL default '',
  `security` int(11) unsigned NOT NULL default '0',
  `help` longtext,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Chat System';

--
-- Dumping data for table `command`
--


/*!40000 ALTER TABLE `command` DISABLE KEYS */;
LOCK TABLES `command` WRITE;
INSERT INTO `command` VALUES ('modify',1,'Syntax: .modify parameter value\r\n\r\nAllows to modify the value of various parameters. Supported parameters include hp, mana, rage, energy, gold, level, speed, swim, scale, bit, bwalk, aspeed, faction, and spell.\r\n\r\nUse .help modify parameter to get help on specific parameter usage.'),('acct',0,'Syntax: .acct\r\n\r\nDisplay the access level of your account.'),('addmove',2,'Syntax: .move\r\n\r\nAdd your current location as a waypoint for the selected creature.'),('addspirit',3,'Syntax: .addspirit\r\n\r\nSpawns the spirit healer for the current location, if there is one defined for the current location.'),('anim',3,'Syntax: .anim\r\n\r\n'),('announce',1,'Syntax: .announce $MessageToBroadcast\r\n\r\nSend a global message to all characters online.'),('go',3,'Syntax: .go position_x position_y position_z mapid\r\n\r\nTeleport to the given coordinates on the specified map.'),('goname',1,'Syntax: .goname charactername\r\n\r\nTeleport to the given character. Either specify the charactername in the chat or click on the characters symbol, e.g. when you\'re in a group.'),('namego',1,'Syntax: .namego charactername\r\n\r\nTeleport the given character to you. Either specify the charactername in the chat or click on the characters symbol, e.g. when you\'\'re in a group.'),('aura',3,'Syntax: .aura #auraid\r\n\r\nAdd the aura with id #auraid to the selected creature.'),('changelevel',2,'Syntax: .changelevel #level\r\n\r\nChange the level of the selected creature to #level. #level may range from 1 to 63.'),('commands',0,'Syntax: .commands\r\n\r\nDisplay a list of available commands for your account level.'),('delete',2,'Syntax: .delete\r\n\r\nDelete the selected creature from the world.'),('demorph',2,'Syntax: .demorph\r\n\r\nDemorph the selected player.'),('die',3,'Syntax: .die\r\n\r\nKill the selected player. If no player is selected, it will kill you.'),('revive',3,'Syntax: .revive\r\n\r\nRevive the selected player. If no player is selected, it will revive you.'),('dismount',0,'Syntax: .dismount\r\n\r\nDismount you, if you are mounted.'),('displayid',2,'Syntax: .displayid #displayid\r\n\r\nChange the model id of the selected creature to #displayid.'),('factionid',2,'Syntax: .factionid #factionid\r\n\r\nSet the faction of the selected creature to #factionid.'),('gmlist',0,'Syntax: .gmlist\r\n\r\nDisplay a list a available Game Masters.'),('gmoff',1,'Syntax: .gmoff\r\n\r\nDisable the <GM> prefix for your character.'),('gmon',1,'Syntax: .gmon\r\n\r\nEnable the <GM> prefix for your character.'),('gps',1,'Syntax: .gps\r\n\r\nDisplay the position information for a selected character or creature. Position information includes X, Y, Z, and orientation.'),('guid',2,'Syntax: .guid\r\n\r\nDisplay the GUID for the selected player.'),('help',0,'Syntax: .help $command\r\n\r\nDisplay usage instructions for the given $command.'),('info',0,'Syntax: .info\r\n\r\nDisplay the number of connected characters.'),('npcinfo',3,'Syntax: .npcinfo\r\n\r\nDisplay a list of details for the selected creature.\r\n\r\nThe list includes:\r\n- GUID, Faction, NPC flags, Entry ID, Model ID,\r\n- Level,\r\n- Health (current/maximum),\r\n- Field flags, dynamic flags, faction template, \r\n- Position information,\r\n- and the creature type, e.g. if the creature is a vendor.'),('npcinfoset',3,'Syntax: .npcinfoset\r\n\r\nTODO: Write me.'),('item',2,'Syntax: .item #guid #amount\r\n\r\nAdd the given amount #amount of the item with a GUID of #guid to the selected vendor. '),('itemrmv',2,'Syntax: .itemrmv #guid\r\n\r\nRemove the given item with a GUID of #guid from the selected vendor. '),('kick',2,'Syntax: .kick\r\n\r\nKick the selected character from the world.\r\n\r\nNot yet implemented.'),('learn',3,'Syntax: .learn #parameter\r\n\r\nLearn a spell of id #parameter. If you want to learn all default spells for Game Masters, use the syntax .learn all'),('unlearn',3,'Syntax: .unlearn #parameter\r\n\r\nUnlearn a spell of id #parameter. '),('learnsk',3,'Syntax: .learnsk #skillId #level #max\r\n\r\nLearn a skill of id #skill with a current skill value of #level and a maximum value of #max. '),('unleask',3,'Syntax: .unlearnsk #parameter\r\n\r\nUnlearn a skill of id #parameter. '),('morph',3,'Syntax: .morph #displayid\r\n\r\nChange your current model id to #displayid.'),('name',2,'Syntax: .name $Name\r\n\r\nChange the name of the selected creature or player to $Name.\r\n\r\nCommand disabled.'),('subname',2,'Syntax: .subname $Name\r\n\r\nChange the subname of the selected creature or player to $Name.\r\n\r\nCommand disabled.'),('npcflag',2,'Syntax: .npcflag #npcflag\r\n\r\nSet the NPC flags of the selected creature to #npcflag.'),('cdist',3,'Syntax: .cdist #units\r\n\r\nSet Creature max think distance to #units (from nearest player).'),('object',3,'Syntax: .object #displayid\r\n\r\nAdd a new object of type mailbox with the display id of #displayid to your current position.'),('gameobject',3,'Syntax: .gameobject #id\r\n\r\nAdd a game object from game object templates to the world using the #id.'),('addgo',3,'Syntax: .addgo #id\r\n\r\nAdd a game object from game object templates to the world using the #id.\r\nNote: this is just a copy of .gameobject.'),('prog',2,'Syntax: .prog\r\n\r\nTeleports you to Programmers Island.'),('random',2,'Syntax: .moverandom #flag\r\n\r\nEnable or disable random movement for a selected creature. Use a #flag of value 1 to enable, use a #flag value of 0 to disable random movement.\r\n\r\nNot yet implemented.'),('recall',1,'Syntax: .recall $place\r\n\r\nTeleport you to various towns around the world. $place defines the target location. Available targets include sunr, thun, ogri, neth, thel, storm, iron, under, and darr.'),('run',2,'Syntax: .run #flag\r\n\r\nEnable or disable running movement for a selected creature. Use a #flag of value 1 to enable, use a #flag value of 0 to disable running.\r\n\r\nNot yet implemented.'),('save',0,'Syntax: .save\r\n\r\nSaves your character.'),('security',3,'Syntax: .security $name #level\r\n\r\nSet the security level of character $name to a level of #level.'),('AddSpawn',2,'Not yet implemented.'),('standstate',3,'Syntax: .standstate #emoteid\r\n\r\nMake the selected creature use the emote of id #emoteid.'),('start',0,'Syntax: .start\r\n\r\nTeleports you to the starting area of your character.'),('taxicheat',1,'Syntax: .taxicheat\r\n\r\nReveal all taxi routes for the selected character.'),('worldport',3,'Syntax: .worldport position_x position_y position_z\r\n\r\nTeleport to the given coordinates on the current continent.'),('addweapon',3,'Not yet implemented.'),('allowmove',3,'Syntax: .allowmove\r\n\r\nEnable or disable movement for the selected creature.'),('addgrave',3,'Syntax: .addgrave\r\n\r\nAdd a graveyard at your current location.'),('addsh',3,'Syntax: .addsh\r\n\r\nAdd a spirit healer to your current location.\r\n\r\nNot yet implemented.'),('transport',3,'Not yet implemented.'),('explorecheat',3,'Syntax: .explorecheat\r\n\r\nReveal all maps for the selected character.'),('hover',3,'Syntax: .hover\r\n\r\nEnable hover mode for the selected player.'),('levelup',3,'Syntax: .levelup #numberoflevels\r\n\r\nIncrease the level of the selected player by #numberoflevels. if #numberoflevels is omitted, the level will be increase by 1.'),('emote',3,'Syntax: .emote #emoteid\r\n\r\nMake the selected creature emote with an emote of id #emoteid.'),('showarea',3,'Syntax: .showarea #areaid\r\n\r\nReveal the area of #areaid for the selected player.'),('hidearea',3,'Syntax: .hidearea #areaid\r\n\r\nHide the area of #areaid for the selected player.'),('addspw',2,'Syntax: .addspw #creatureid\r\n\r\nSpawn a creature by the given template id of #creatureid.\r\n'),('additem',3,'Syntax: .additem #itemid #itemcount\r\n\r\nAdd an item of id #itemid to your inventory. Will add a number of #itemcount. If #itemcount is omitted, only one item will be added.'),('createguild',3,'Syntax: .createguild $GuildName $GuildLeaderName\r\n\r\nCreate a guild name $GuildName with the character $GuildLeaderName as the leader.'),('showhonor',0,'Syntax: .showhonor\r\n\r\nDisplay your honor ranking.'),('update',3,'Syntax: .update\r\n'),('bank',3,'Syntax: .bank\r\n\r\nShow your bank inventory.'),('wchange',3,'Syntax: .wchange #weathertype #status\r\n\r\nSet current weather to #weathertype with an intensitiy of #status. #weathertype can be 1 for rain, 2 for snow, and 3 for sand. #status can be 0 for disabled, and 1 for enabled.'),('reload',3,'Not yet implemented.'),('loadscripts',3,'Syntax: .loadscripts\r\n\r\nWill reload the script library, in case you change it while the server was running.'),('set32value',3,''),('Set32Bit',3,''),('Mod32Value',3,''),('modify hp',1,''),('modify mana',1,''),('modify energy',1,''),('modify gold',1,''),('modify level',1,''),('modify speed',1,''),('modify swim',1,''),('modify scale',1,''),('modify bit',1,''),('modify bwalk',1,''),('modify aspeed',1,''),('modify faction',1,''),('modify spell',1,'');
UNLOCK TABLES;
/*!40000 ALTER TABLE `command` ENABLE KEYS */;

--
-- Table structure for table `creature`
--

DROP TABLE IF EXISTS `creature`;
CREATE TABLE `creature` (
  `guid` bigint(20) unsigned NOT NULL auto_increment COMMENT 'Global Unique Identifier',
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Creature Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `spawntimemin` int(11) unsigned NOT NULL default '10',
  `spawntimemax` int(11) unsigned NOT NULL default '10',
  `spawndist` float NOT NULL default '5',
  `currentwaypoint` int(11) unsigned NOT NULL default '0',
  `spawn_position_x` float NOT NULL default '0',
  `spawn_position_y` float NOT NULL default '0',
  `spawn_position_z` float NOT NULL default '0',
  `spawn_orientation` float NOT NULL default '0',
  `curhealth` int(11) unsigned NOT NULL default '1',
  `curmana` int(11) unsigned NOT NULL default '0',
  `respawntimer` int(11) unsigned NOT NULL default '0',
  `state` int(11) unsigned NOT NULL default '0',
  `npcflags` int(11) unsigned NOT NULL default '0',
  `faction` int(11) unsigned NOT NULL default '0',
  `auras` longtext,
  PRIMARY KEY  (`guid`),
  KEY `idx_map` (`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature`
--


/*!40000 ALTER TABLE `creature` DISABLE KEYS */;
LOCK TABLES `creature` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature` ENABLE KEYS */;

--
-- Table structure for table `creature_grid`
--

DROP TABLE IF EXISTS `creature_grid`;
CREATE TABLE `creature_grid` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `position_x` int(11) NOT NULL default '0',
  `position_y` int(11) NOT NULL default '0',
  `cell_position_x` int(11) NOT NULL default '0',
  `cell_position_y` int(11) NOT NULL default '0',
  `grid` int(11) unsigned NOT NULL default '0' COMMENT 'Grid Identifier',
  `cell` int(11) unsigned NOT NULL default '0' COMMENT 'Cell Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  UNIQUE KEY `idx_search` (`grid`,`cell`,`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Exploration System';

--
-- Dumping data for table `creature_grid`
--


/*!40000 ALTER TABLE `creature_grid` DISABLE KEYS */;
LOCK TABLES `creature_grid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_grid` ENABLE KEYS */;

--
-- Table structure for table `creature_involvedrelation`
--

DROP TABLE IF EXISTS `creature_involvedrelation`;
CREATE TABLE `creature_involvedrelation` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Creature System';

--
-- Dumping data for table `creature_involvedrelation`
--


/*!40000 ALTER TABLE `creature_involvedrelation` DISABLE KEYS */;
LOCK TABLES `creature_involvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_involvedrelation` ENABLE KEYS */;

--
-- Table structure for table `creature_movement`
--

DROP TABLE IF EXISTS `creature_movement`;
CREATE TABLE `creature_movement` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `point` int(11) unsigned NOT NULL default '0',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `waittime` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`point`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Creature System';

--
-- Dumping data for table `creature_movement`
--


/*!40000 ALTER TABLE `creature_movement` DISABLE KEYS */;
LOCK TABLES `creature_movement` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_movement` ENABLE KEYS */;

--
-- Table structure for table `creature_questrelation`
--

DROP TABLE IF EXISTS `creature_questrelation`;
CREATE TABLE `creature_questrelation` (
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Creature System';

--
-- Dumping data for table `creature_questrelation`
--


/*!40000 ALTER TABLE `creature_questrelation` DISABLE KEYS */;
LOCK TABLES `creature_questrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_questrelation` ENABLE KEYS */;

--
-- Table structure for table `creature_template`
--

DROP TABLE IF EXISTS `creature_template`;
CREATE TABLE `creature_template` (
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
  `spell1` int(11) unsigned NOT NULL default '0',
  `spell2` int(11) unsigned NOT NULL default '0',
  `spell3` int(11) unsigned NOT NULL default '0',
  `spell4` int(11) unsigned NOT NULL default '0',
  `AIName` varchar(128) NOT NULL default '',
  `MoveName` varchar(128) NOT NULL default '',
  `ScriptName` varchar(128) NOT NULL default '',
  PRIMARY KEY  (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Creature System';

--
-- Dumping data for table `creature_template`
--


/*!40000 ALTER TABLE `creature_template` DISABLE KEYS */;
LOCK TABLES `creature_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_template` ENABLE KEYS */;

--
-- Table structure for table `game_corpse`
--

DROP TABLE IF EXISTS `game_corpse`;
CREATE TABLE `game_corpse` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `player` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character Global Unique Identifier',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `zone` int(11) unsigned NOT NULL default '38' COMMENT 'Zone Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `data` longtext,
  `time` timestamp NOT NULL default '0000-00-00 00:00:00',
  `bones_flag` tinyint(3) NOT NULL default '0',
  PRIMARY KEY  (`guid`),
  UNIQUE KEY `idx_player` (`player`),
  KEY `idx_bones_flag` (`bones_flag`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Death System';

--
-- Dumping data for table `game_corpse`
--


/*!40000 ALTER TABLE `game_corpse` DISABLE KEYS */;
LOCK TABLES `game_corpse` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_corpse` ENABLE KEYS */;

--
-- Table structure for table `game_spell`
--

DROP TABLE IF EXISTS `game_spell`;
CREATE TABLE `game_spell` (
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
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Spell System';

--
-- Dumping data for table `game_spell`
--


/*!40000 ALTER TABLE `game_spell` DISABLE KEYS */;
LOCK TABLES `game_spell` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_spell` ENABLE KEYS */;

--
-- Table structure for table `game_talent`
--

DROP TABLE IF EXISTS `game_talent`;
CREATE TABLE `game_talent` (
  `id` int(10) NOT NULL auto_increment,
  `t_id` int(10) NOT NULL default '0',
  `maxrank` int(7) NOT NULL default '0',
  `class` int(10) NOT NULL default '0',
  `rank1` longtext,
  `rank2` longtext,
  `rank3` longtext,
  `rank4` longtext,
  `rank5` longtext,
  PRIMARY KEY  (`id`),
  KEY `talents_index` (`t_id`,`maxrank`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Spell System';

--
-- Dumping data for table `game_talent`
--


/*!40000 ALTER TABLE `game_talent` DISABLE KEYS */;
LOCK TABLES `game_talent` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_talent` ENABLE KEYS */;

--
-- Table structure for table `game_weather`
--

DROP TABLE IF EXISTS `game_weather`;
CREATE TABLE `game_weather` (
  `zone` int(11) unsigned NOT NULL default '0',
  `spring_rain_chance` tinyint(3) unsigned NOT NULL default '25',
  `spring_snow_chance` tinyint(3) unsigned NOT NULL default '25',
  `spring_storm_chance` tinyint(3) unsigned NOT NULL default '25',
  `summer_rain_chance` tinyint(3) unsigned NOT NULL default '25',
  `summer_snow_chance` tinyint(3) unsigned NOT NULL default '25',
  `summer_storm_chance` tinyint(3) unsigned NOT NULL default '25',
  `fall_rain_chance` tinyint(3) unsigned NOT NULL default '25',
  `fall_snow_chance` tinyint(3) unsigned NOT NULL default '25',
  `fall_storm_chance` tinyint(3) unsigned NOT NULL default '25',
  `winter_rain_chance` tinyint(3) unsigned NOT NULL default '25',
  `winter_snow_chance` tinyint(3) unsigned NOT NULL default '25',
  `winter_storm_chance` tinyint(3) unsigned NOT NULL default '25',
  PRIMARY KEY  (`zone`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Weather System';

--
-- Dumping data for table `game_weather`
--


/*!40000 ALTER TABLE `game_weather` DISABLE KEYS */;
LOCK TABLES `game_weather` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_weather` ENABLE KEYS */;

--
-- Table structure for table `gameobject`
--

DROP TABLE IF EXISTS `gameobject`;
CREATE TABLE `gameobject` (
  `guid` bigint(20) unsigned NOT NULL auto_increment COMMENT 'Global Unique Identifier',
  `id` int(11) unsigned NOT NULL default '0' COMMENT 'Gameobject Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `rotation0` float NOT NULL default '0',
  `rotation1` float NOT NULL default '0',
  `rotation2` float NOT NULL default '0',
  `rotation3` float NOT NULL default '0',
  `loot` int(11) unsigned NOT NULL default '0',
  `respawntimer` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Gameobject System';

--
-- Dumping data for table `gameobject`
--


/*!40000 ALTER TABLE `gameobject` DISABLE KEYS */;
LOCK TABLES `gameobject` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject` ENABLE KEYS */;

--
-- Table structure for table `gameobject_grid`
--

DROP TABLE IF EXISTS `gameobject_grid`;
CREATE TABLE `gameobject_grid` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `position_x` int(11) NOT NULL default '0',
  `position_y` int(11) NOT NULL default '0',
  `cell_position_x` int(11) NOT NULL default '0',
  `cell_position_y` int(11) NOT NULL default '0',
  `grid` int(11) unsigned NOT NULL default '0' COMMENT 'Grid Identifier',
  `cell` int(11) unsigned NOT NULL default '0' COMMENT 'Cell Identifier',
  `map` int(11) unsigned NOT NULL default '0' COMMENT 'Map Identifier',
  UNIQUE KEY `idx_search` (`grid`,`cell`,`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Grid System';

--
-- Dumping data for table `gameobject_grid`
--


/*!40000 ALTER TABLE `gameobject_grid` DISABLE KEYS */;
LOCK TABLES `gameobject_grid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject_grid` ENABLE KEYS */;

--
-- Table structure for table `gameobject_template`
--

DROP TABLE IF EXISTS `gameobject_template`;
CREATE TABLE `gameobject_template` (
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
  PRIMARY KEY  (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Gameobject System';

--
-- Dumping data for table `gameobject_template`
--


/*!40000 ALTER TABLE `gameobject_template` DISABLE KEYS */;
LOCK TABLES `gameobject_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject_template` ENABLE KEYS */;

--
-- Table structure for table `guild`
--

DROP TABLE IF EXISTS `guild`;
CREATE TABLE `guild` (
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
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild`
--


/*!40000 ALTER TABLE `guild` DISABLE KEYS */;
LOCK TABLES `guild` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild` ENABLE KEYS */;

--
-- Table structure for table `guild_member`
--

DROP TABLE IF EXISTS `guild_member`;
CREATE TABLE `guild_member` (
  `guildid` int(6) unsigned NOT NULL default '0',
  `guid` int(6) NOT NULL default '0',
  `rank` tinyint(2) unsigned NOT NULL default '0',
  `Pnote` varchar(255) NOT NULL default '',
  `OFFnote` varchar(255) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild_member`
--


/*!40000 ALTER TABLE `guild_member` DISABLE KEYS */;
LOCK TABLES `guild_member` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild_member` ENABLE KEYS */;

--
-- Table structure for table `guild_rank`
--

DROP TABLE IF EXISTS `guild_rank`;
CREATE TABLE `guild_rank` (
  `guildid` int(6) unsigned NOT NULL default '0',
  `rname` varchar(255) NOT NULL default '',
  `rights` int(3) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild_rank`
--


/*!40000 ALTER TABLE `guild_rank` DISABLE KEYS */;
LOCK TABLES `guild_rank` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild_rank` ENABLE KEYS */;

--
-- Table structure for table `ip_banned`
--

DROP TABLE IF EXISTS `ip_banned`;
CREATE TABLE `ip_banned` (
  `ip` varchar(32) NOT NULL default '127.0.0.1',
  PRIMARY KEY  (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm System';

--
-- Dumping data for table `ip_banned`
--


/*!40000 ALTER TABLE `ip_banned` DISABLE KEYS */;
LOCK TABLES `ip_banned` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `ip_banned` ENABLE KEYS */;

--
-- Table structure for table `item_instance`
--

DROP TABLE IF EXISTS `item_instance`;
CREATE TABLE `item_instance` (
  `guid` bigint(20) NOT NULL default '0',
  `data` longtext,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item System';

--
-- Dumping data for table `item_instance`
--


/*!40000 ALTER TABLE `item_instance` DISABLE KEYS */;
LOCK TABLES `item_instance` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_instance` ENABLE KEYS */;

--
-- Table structure for table `item_page`
--

DROP TABLE IF EXISTS `item_page`;
CREATE TABLE `item_page` (
  `id` int(11) NOT NULL default '0',
  `text` longtext,
  `next_page` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `item_pages_index` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Item System';

--
-- Dumping data for table `item_page`
--


/*!40000 ALTER TABLE `item_page` DISABLE KEYS */;
LOCK TABLES `item_page` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_page` ENABLE KEYS */;

--
-- Table structure for table `item_template`
--

DROP TABLE IF EXISTS `item_template`;
CREATE TABLE `item_template` (
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
  `RequiredReputationRank` int(30) unsigned NOT NULL default '0',
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
  `dmg_min1` float NOT NULL default '0',
  `dmg_max1` float NOT NULL default '0',
  `dmg_type1` int(30) unsigned NOT NULL default '0',
  `dmg_min2` float NOT NULL default '0',
  `dmg_max2` float NOT NULL default '0',
  `dmg_type2` int(30) unsigned NOT NULL default '0',
  `dmg_min3` float NOT NULL default '0',
  `dmg_max3` float NOT NULL default '0',
  `dmg_type3` int(30) unsigned NOT NULL default '0',
  `dmg_min4` float NOT NULL default '0',
  `dmg_max4` float NOT NULL default '0',
  `dmg_type4` int(30) unsigned NOT NULL default '0',
  `dmg_min5` float NOT NULL default '0',
  `dmg_max5` float NOT NULL default '0',
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
  `RangedModRange` float NOT NULL default '0',
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
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Item System';

--
-- Dumping data for table `item_template`
--


/*!40000 ALTER TABLE `item_template` DISABLE KEYS */;
LOCK TABLES `item_template` WRITE;
INSERT INTO `item_template` VALUES (65020,0,0,'Tough Jerky','Tough Jerky','Tough Jerky','Tough Jerky',2473,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65021,0,0,'Refreshing Spring Water','Refreshing Spring Water','Refreshing Spring Water','Refreshing Spring Water',18084,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,430,0,0,0,59,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65022,0,0,'Darnassian Bleu','Darnassian Bleu','Darnassian Bleu','Darnassian Bleu',6353,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65023,2,16,'Small Throwing Knife','Small Throwing Knife','Small Throwing Knife','Small Throwing Knife',16754,1,0,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,'internalItemHandler'),(65024,2,16,'Crude Throwing Axe','Crude Throwing Axe','Crude Throwing Axe','Crude Throwing Axe',20777,1,0,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,'internalItemHandler'),(65025,0,0,'Shiny Red Apple','Shiny Red Apple','Shiny Red Apple','Shiny Red Apple',6410,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65026,0,0,'Tough Hunk of Bread','Tough Hunk of Bread','Tough Hunk of Bread','Tough Hunk of Bread',6399,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65027,0,0,'Forest Mushroom Cap','Forest Mushroom Cap','Forest Mushroom Cap','Forest Mushroom Cap',15852,1,0,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(25,2,7,'Worn Shortsword','Worn Shortsword','Worn Shortsword','Worn Shortsword',1542,1,0,35,7,21,32767,511,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,1,0,0,0,1,3,0,0,0,20,0,'internalItemHandler'),(39,4,1,'Recruit\'s Pants','Recruit\'s Pants','Recruit\'s Pants','Recruit\'s Pants',9892,0,0,5,1,7,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(40,4,0,'Recruit\'s Boots','Recruit\'s Boots','Recruit\'s Boots','Recruit\'s Boots',10141,1,0,4,1,8,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(38,4,0,'Recruit\'s Shirt','Recruit\'s Shirt','Recruit\'s Shirt','Recruit\'s Shirt',9891,1,0,1,1,4,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(2362,4,6,'Worn Wooden Shield','Worn Wooden Shield','Worn Wooden Shield','Worn Wooden Shield',18730,0,0,7,1,14,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,4,0,1,0,20,0,'internalItemHandler'),(6948,15,0,'Hearthstone','Hearthstone','Hearthstone','Hearthstone',6418,1,64,0,0,0,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14646,12,0,'Northshire Gift Voucher','Northshire Gift Voucher','Northshire Gift Voucher','Northshire Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5805,0,0,0,0,0,0,0,0,'internalItemHandler'),(14647,12,0,'Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5841,0,0,0,0,0,0,0,0,'internalItemHandler'),(14648,12,0,'Shadowglen Gift Voucher','Shadowglen Gift Voucher','Shadowglen Gift Voucher','Shadowglen Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5842,0,0,0,0,0,0,0,0,'internalItemHandler'),(14649,12,0,'Valley of Trials Gift Voucher','Valley of Trials Gift Voucher','Valley of Trials Gift Voucher','Valley of Trials Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5843,0,0,0,0,0,0,0,0,'internalItemHandler'),(14650,12,0,'Camp Narache Gift Voucher','Camp Narache Gift Voucher','Camp Narache Gift Voucher','Camp Narache Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5844,0,0,0,0,0,0,0,0,'internalItemHandler'),(14651,12,0,'Deathknell Gift Voucher','Deathknell Gift Voucher','Deathknell Gift Voucher','Deathknell Gift Voucher',18499,1,0,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5847,0,0,0,0,0,0,0,0,'internalItemHandler'),(43,4,0,'Squire\'s Boots','Squire\'s Boots','Squire\'s Boots','Squire\'s Boots',10272,1,0,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(44,4,1,'Squire\'s Pants','Squire\'s Pants','Squire\'s Pants','Squire\'s Pants',9937,0,0,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(45,4,0,'Squire\'s Shirt','Squire\'s Shirt','Squire\'s Shirt','Squire\'s Shirt',3265,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(47,4,0,'Footpad\'s Shoes','Footpad\'s Shoes','Footpad\'s Shoes','Footpad\'s Shoes',9915,1,0,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(48,4,1,'Footpad\'s Pants','Footpad\'s Pants','Footpad\'s Pants','Footpad\'s Pants',9913,0,0,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(49,4,0,'Footpad\'s Shirt','Footpad\'s Shirt','Footpad\'s Shirt','Footpad\'s Shirt',9906,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(51,4,0,'Neophyte\'s Boots','Neophyte\'s Boots','Neophyte\'s Boots','Neophyte\'s Boots',9946,1,0,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(52,4,1,'Neophyte\'s Pants','Neophyte\'s Pants','Neophyte\'s Pants','Neophyte\'s Pants',9945,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(53,4,0,'Neophyte\'s Shirt','Neophyte\'s Shirt','Neophyte\'s Shirt','Neophyte\'s Shirt',9944,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(55,4,0,'Apprentice\'s Boots','Apprentice\'s Boots','Apprentice\'s Boots','Apprentice\'s Boots',9929,1,0,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(56,4,1,'Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe',12647,0,0,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(57,4,1,'Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe',12645,0,0,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(59,4,0,'Acolyte\'s Shoes','Acolyte\'s Shoes','Acolyte\'s Shoes','Acolyte\'s Shoes',3261,1,0,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(35,2,10,'Bent Staff','Bent Staff','Bent Staff','Bent Staff',472,1,0,47,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,'internalItemHandler'),(36,2,4,'Worn Mace','Worn Mace','Worn Mace','Worn Mace',5194,1,0,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,3,0,0,0,20,0,'internalItemHandler'),(37,2,0,'Worn Axe','Worn Axe','Worn Axe','Worn Axe',14029,1,0,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,20,0,'internalItemHandler'),(2361,2,5,'Battleworn Hammer','Battleworn Hammer','Battleworn Hammer','Battleworn Hammer',8690,1,0,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,1,0,0,0,25,0,'internalItemHandler'),(2092,2,15,'Worn Dagger','Worn Dagger','Worn Dagger','Worn Dagger',6442,1,0,35,7,13,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1600,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,16,0,'internalItemHandler'),(6096,4,0,'Apprentice\'s Shirt','Apprentice\'s Shirt','Apprentice\'s Shirt','Apprentice\'s Shirt',2163,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(6097,4,0,'Acolyte\'s Shirt','Acolyte\'s Shirt','Acolyte\'s Shirt','Acolyte\'s Shirt',2470,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(6098,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12679,0,0,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(1395,4,1,'Apprentice\'s Pants','Apprentice\'s Pants','Apprentice\'s Pants','Apprentice\'s Pants',9924,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(1396,4,1,'Acolyte\'s Pants','Acolyte\'s Pants','Acolyte\'s Pants','Acolyte\'s Pants',3260,0,0,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(6125,4,0,'Brawler\'s Harness','Brawler\'s Harness','Brawler\'s Harness','Brawler\'s Harness',9995,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(6126,4,1,'Trapper\'s Pants','Trapper\'s Pants','Trapper\'s Pants','Trapper\'s Pants',10002,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(6127,4,0,'Trapper\'s Boots','Trapper\'s Boots','Trapper\'s Boots','Trapper\'s Boots',10003,1,0,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(6129,4,1,'Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe',12646,0,0,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(139,4,1,'Brawler\'s Pants','Brawler\'s Pants','Brawler\'s Pants','Brawler\'s Pants',9988,0,0,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(140,4,0,'Brawler\'s Boots','Brawler\'s Boots','Brawler\'s Boots','Brawler\'s Boots',9992,1,0,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(127,4,0,'Trapper\'s Shirt','Trapper\'s Shirt','Trapper\'s Shirt','Trapper\'s Shirt',9996,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(120,4,1,'Thug Pants','Thug Pants','Thug Pants','Thug Pants',10006,0,0,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(121,4,0,'Thug Boots','Thug Boots','Thug Boots','Thug Boots',10008,1,0,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(147,4,1,'Rugged Trapper\'s Pants','Rugged Trapper\'s Pants','Rugged Trapper\'s Pants','Rugged Trapper\'s Pants',9975,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(148,4,0,'Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt',9976,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(153,4,2,'Primitive Kilt','Primitive Kilt','Primitive Kilt','Primitive Kilt',10050,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,'internalItemHandler'),(154,4,0,'Primitive Mantle','Primitive Mantle','Primitive Mantle','Primitive Mantle',10058,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(2101,1,2,'Light Quiver','Light Quiver','Light Quiver','Light Quiver',21328,1,0,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(2102,1,3,'Small Ammo Pouch','Small Ammo Pouch','Small Ammo Pouch','Small Ammo Pouch',1816,1,0,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(2105,4,0,'Thug Shirt','Thug Shirt','Thug Shirt','Thug Shirt',10005,1,0,5,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(2504,2,2,'Worn Shortbow','Worn Shortbow','Worn Shortbow','Worn Shortbow',8106,1,0,29,5,15,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,20,0,'internalItemHandler'),(2508,2,3,'Old Blunderbuss','Old Blunderbuss','Old Blunderbuss','Old Blunderbuss',6606,1,0,27,5,26,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,20,0,'internalItemHandler'),(2512,6,2,'Rough Arrow','Rough Arrow','Rough Arrow','Rough Arrow',5996,1,0,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,'internalItemHandler'),(2516,6,3,'Light Shot','Light Shot','Light Shot','Light Shot',5998,1,0,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,'internalItemHandler'),(3661,2,10,'Handcrafted Staff','Handcrafted Staff','Handcrafted Staff','Handcrafted Staff',18530,1,0,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,'internalItemHandler'),(6119,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12681,0,0,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(6123,4,1,'Novice\'s Robe','Novice\'s Robe','Novice\'s Robe','Novice\'s Robe',12683,0,0,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(6124,4,1,'Novice\'s Pants','Novice\'s Pants','Novice\'s Pants','Novice\'s Pants',9987,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,'internalItemHandler'),(6134,4,0,'Primitive Mantle','Primitive Mantle','Primitive Mantle','Primitive Mantle',10108,1,0,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,'internalItemHandler'),(6135,4,2,'Primitive Kilt','Primitive Kilt','Primitive Kilt','Primitive Kilt',10109,0,0,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,'internalItemHandler'),(6139,4,1,'Novice\'s Robe','Novice\'s Robe','Novice\'s Robe','Novice\'s Robe',12684,0,0,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(6140,4,1,'Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe',12649,0,0,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(6144,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12680,0,0,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,'internalItemHandler'),(12282,2,1,'Worn Battleaxe','Worn Battleaxe','Worn Battleaxe','Worn Battleaxe',22291,1,0,43,8,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,1,0,0,0,25,0,'internalItemHandler');
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_template` ENABLE KEYS */;

--
-- Table structure for table `loot_template`
--

DROP TABLE IF EXISTS `loot_template`;
CREATE TABLE `loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `chance` float NOT NULL default '100',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `loot_template`
--


/*!40000 ALTER TABLE `loot_template` DISABLE KEYS */;
LOCK TABLES `loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `loot_template` ENABLE KEYS */;

--
-- Table structure for table `mail`
--

DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `id` bigint(20) unsigned NOT NULL default '0' COMMENT 'Identifier',
  `sender` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character Global Unique Identifier',
  `receiver` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character Global Unique Identifier',
  `subject` longtext,
  `body` longtext,
  `item` bigint(20) unsigned NOT NULL default '0' COMMENT 'Mail Item Global Unique Identifier',
  `time` int(11) unsigned NOT NULL default '0',
  `money` int(11) unsigned NOT NULL default '0',
  `cod` bigint(20) unsigned NOT NULL default '0',
  `checked` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_receiver` (`receiver`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Mail System';

--
-- Dumping data for table `mail`
--


/*!40000 ALTER TABLE `mail` DISABLE KEYS */;
LOCK TABLES `mail` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `mail` ENABLE KEYS */;

--
-- Table structure for table `mail_item`
--

DROP TABLE IF EXISTS `mail_item`;
CREATE TABLE `mail_item` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `data` longtext,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Mail System';

--
-- Dumping data for table `mail_item`
--


/*!40000 ALTER TABLE `mail_item` DISABLE KEYS */;
LOCK TABLES `mail_item` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `mail_item` ENABLE KEYS */;

--
-- Table structure for table `npc_gossip`
--

DROP TABLE IF EXISTS `npc_gossip`;
CREATE TABLE `npc_gossip` (
  `id` int(11) NOT NULL default '0',
  `npc_guid` int(11) NOT NULL default '0',
  `gossip_type` int(11) NOT NULL default '0',
  `textid` int(30) NOT NULL default '0',
  `option_count` int(30) default NULL,
  PRIMARY KEY  (`id`,`npc_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_gossip`
--


/*!40000 ALTER TABLE `npc_gossip` DISABLE KEYS */;
LOCK TABLES `npc_gossip` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_gossip` ENABLE KEYS */;

--
-- Table structure for table `npc_gossip_textid`
--

DROP TABLE IF EXISTS `npc_gossip_textid`;
CREATE TABLE `npc_gossip_textid` (
  `zoneid` int(11) unsigned NOT NULL default '0',
  `action` int(3) unsigned NOT NULL default '0',
  `textid` int(11) unsigned NOT NULL default '0',
  KEY `zoneid` (`zoneid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_gossip_textid`
--


/*!40000 ALTER TABLE `npc_gossip_textid` DISABLE KEYS */;
LOCK TABLES `npc_gossip_textid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_gossip_textid` ENABLE KEYS */;

--
-- Table structure for table `npc_option`
--

DROP TABLE IF EXISTS `npc_option`;
CREATE TABLE `npc_option` (
  `id` int(11) unsigned NOT NULL default '0',
  `gossip_id` int(11) unsigned NOT NULL default '0',
  `npcflag` int(11) unsigned NOT NULL default '0',
  `icon` int(11) unsigned NOT NULL default '0',
  `action` int(11) unsigned NOT NULL default '0',
  `option` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_option`
--


/*!40000 ALTER TABLE `npc_option` DISABLE KEYS */;
LOCK TABLES `npc_option` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_option` ENABLE KEYS */;

--
-- Table structure for table `npc_spirithealer`
--

DROP TABLE IF EXISTS `npc_spirithealer`;
CREATE TABLE `npc_spirithealer` (
  `position_x` float default NULL,
  `position_y` float default NULL,
  `position_z` float default NULL,
  `F` float default NULL,
  `name_id` int(8) default NULL,
  `zone` int(16) default NULL,
  `map` int(16) default NULL,
  `faction` int(32) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_spirithealer`
--


/*!40000 ALTER TABLE `npc_spirithealer` DISABLE KEYS */;
LOCK TABLES `npc_spirithealer` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_spirithealer` ENABLE KEYS */;

--
-- Table structure for table `npc_text`
--

DROP TABLE IF EXISTS `npc_text`;
CREATE TABLE `npc_text` (
  `ID` int(11) NOT NULL default '0',
  `text0_0` longtext,
  `text0_1` longtext,
  `lang0` bigint(20) unsigned NOT NULL default '0',
  `prob0` float NOT NULL default '0',
  `em0_0` bigint(20) unsigned NOT NULL default '0',
  `em0_1` bigint(20) unsigned NOT NULL default '0',
  `em0_2` bigint(20) unsigned NOT NULL default '0',
  `em0_3` bigint(20) unsigned NOT NULL default '0',
  `em0_4` bigint(20) unsigned NOT NULL default '0',
  `em0_5` bigint(20) unsigned NOT NULL default '0',
  `text1_0` longtext,
  `text1_1` longtext,
  `lang1` bigint(20) unsigned NOT NULL default '0',
  `prob1` float NOT NULL default '0',
  `em1_0` bigint(20) unsigned NOT NULL default '0',
  `em1_1` bigint(20) unsigned NOT NULL default '0',
  `em1_2` bigint(20) unsigned NOT NULL default '0',
  `em1_3` bigint(20) unsigned NOT NULL default '0',
  `em1_4` bigint(20) unsigned NOT NULL default '0',
  `em1_5` bigint(20) unsigned NOT NULL default '0',
  `text2_0` longtext,
  `text2_1` longtext,
  `lang2` bigint(20) unsigned NOT NULL default '0',
  `prob2` float NOT NULL default '0',
  `em2_0` bigint(20) unsigned NOT NULL default '0',
  `em2_1` bigint(20) unsigned NOT NULL default '0',
  `em2_2` bigint(20) unsigned NOT NULL default '0',
  `em2_3` bigint(20) unsigned NOT NULL default '0',
  `em2_4` bigint(20) unsigned NOT NULL default '0',
  `em2_5` bigint(20) unsigned NOT NULL default '0',
  `text3_0` longtext,
  `text3_1` longtext,
  `lang3` bigint(20) unsigned NOT NULL default '0',
  `prob3` float NOT NULL default '0',
  `em3_0` bigint(20) unsigned NOT NULL default '0',
  `em3_1` bigint(20) unsigned NOT NULL default '0',
  `em3_2` bigint(20) unsigned NOT NULL default '0',
  `em3_3` bigint(20) unsigned NOT NULL default '0',
  `em3_4` bigint(20) unsigned NOT NULL default '0',
  `em3_5` bigint(20) unsigned NOT NULL default '0',
  `text4_0` longtext,
  `text4_1` longtext,
  `lang4` bigint(20) unsigned NOT NULL default '0',
  `prob4` float NOT NULL default '0',
  `em4_0` bigint(20) unsigned NOT NULL default '0',
  `em4_1` bigint(20) unsigned NOT NULL default '0',
  `em4_2` bigint(20) unsigned NOT NULL default '0',
  `em4_3` bigint(20) unsigned NOT NULL default '0',
  `em4_4` bigint(20) unsigned NOT NULL default '0',
  `em4_5` bigint(20) unsigned NOT NULL default '0',
  `text5_0` longtext,
  `text5_1` longtext,
  `lang5` bigint(20) unsigned NOT NULL default '0',
  `prob5` float NOT NULL default '0',
  `em5_0` bigint(20) unsigned NOT NULL default '0',
  `em5_1` bigint(20) unsigned NOT NULL default '0',
  `em5_2` bigint(20) unsigned NOT NULL default '0',
  `em5_3` bigint(20) unsigned NOT NULL default '0',
  `em5_4` bigint(20) unsigned NOT NULL default '0',
  `em5_5` bigint(20) unsigned NOT NULL default '0',
  `text6_0` longtext,
  `text6_1` longtext,
  `lang6` bigint(20) unsigned NOT NULL default '0',
  `prob6` float NOT NULL default '0',
  `em6_0` bigint(20) unsigned NOT NULL default '0',
  `em6_1` bigint(20) unsigned NOT NULL default '0',
  `em6_2` bigint(20) unsigned NOT NULL default '0',
  `em6_3` bigint(20) unsigned NOT NULL default '0',
  `em6_4` bigint(20) unsigned NOT NULL default '0',
  `em6_5` bigint(20) unsigned NOT NULL default '0',
  `text7_0` longtext,
  `text7_1` longtext,
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

--
-- Table structure for table `npc_vendor`
--

DROP TABLE IF EXISTS `npc_vendor`;
CREATE TABLE `npc_vendor` (
  `entry` bigint(20) unsigned NOT NULL default '0',
  `itemguid` bigint(20) unsigned NOT NULL default '0',
  `amount` bigint(20) NOT NULL default '5',
  `index_id` bigint(20) NOT NULL auto_increment,
  PRIMARY KEY  (`index_id`),
  UNIQUE KEY `index_id` (`index_id`),
  KEY `vendor_id` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 18432 kB';

--
-- Dumping data for table `npc_vendor`
--


/*!40000 ALTER TABLE `npc_vendor` DISABLE KEYS */;
LOCK TABLES `npc_vendor` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_vendor` ENABLE KEYS */;

--
-- Table structure for table `object_involvedrelation`
--

DROP TABLE IF EXISTS `object_involvedrelation`;
CREATE TABLE `object_involvedrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `questId` bigint(20) unsigned NOT NULL default '0',
  `objectId` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `object_involvedrelation`
--


/*!40000 ALTER TABLE `object_involvedrelation` DISABLE KEYS */;
LOCK TABLES `object_involvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `object_involvedrelation` ENABLE KEYS */;

--
-- Table structure for table `object_questrelation`
--

DROP TABLE IF EXISTS `object_questrelation`;
CREATE TABLE `object_questrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `questId` bigint(20) unsigned NOT NULL default '0',
  `objectId` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `object_questrelation`
--


/*!40000 ALTER TABLE `object_questrelation` DISABLE KEYS */;
LOCK TABLES `object_questrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `object_questrelation` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo`
--

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE `playercreateinfo` (
  `createId` tinyint(3) unsigned NOT NULL auto_increment,
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `map` mediumint(8) unsigned NOT NULL default '0',
  `zone` mediumint(8) unsigned NOT NULL default '0',
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
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
INSERT INTO `playercreateinfo` VALUES (1,1,1,0,12,-8949,-132,84,49,23,20,22,20,21,40,60,0,1000,0,0,29,5,6,0,0),(2,1,2,0,12,-8949,-132,84,49,22,20,22,20,22,40,68,79,0,0,0,27,4,5,0,0),(3,1,4,0,12,-8949,-132,84,49,21,23,21,20,20,40,55,0,0,0,100,27,10,13,0,0),(4,1,5,0,12,-8949,-132,84,49,20,20,20,22,24,40,61,128,0,0,0,30,4,4,0,0),(5,1,8,0,12,-8949,-132,84,49,20,20,20,23,22,40,61,119,0,0,0,30,4,4,0,0),(6,1,9,0,12,-8949,-132,84,49,20,20,21,22,22,40,53,109,0,0,0,30,4,4,0,0),(7,2,1,1,14,-618,-4251,39,51,26,17,24,17,23,0,80,0,1000,0,0,35,6,6,0,0),(8,2,3,1,14,-618,-4251,39,51,23,20,23,17,24,0,76,80,0,0,0,25,4,5,0,0),(9,2,4,1,14,-618,-4251,39,51,24,20,23,17,23,0,75,0,0,0,100,30,4,4,0,0),(10,2,7,1,14,-618,-4251,39,51,24,17,23,18,25,0,97,71,0,0,0,30,4,4,0,0),(11,2,9,1,14,-618,-4251,39,51,23,17,23,19,25,0,73,109,0,0,0,30,4,4,0,0),(12,3,1,0,1,-6240,331,383,53,25,16,25,19,19,0,90,0,1000,0,0,33,5,6,0,0),(13,3,2,0,1,-6240,331,383,53,24,16,25,19,20,0,88,79,0,0,0,31,5,6,0,0),(14,3,3,0,1,-6240,331,383,53,22,19,24,19,20,0,86,80,0,0,0,24,4,5,0,0),(15,3,4,0,1,-6240,331,383,53,23,19,24,19,19,0,85,0,0,0,100,30,4,4,0,0),(16,3,5,0,1,-6240,331,383,53,22,16,23,21,22,0,91,128,0,0,0,30,4,4,0,0),(17,4,1,1,141,10311,832,1327,55,20,25,21,20,20,0,50,0,1000,0,0,21,4,4,0,0),(18,4,3,1,141,10311,832,1327,55,17,28,20,20,21,0,46,80,0,0,0,26,4,5,0,0),(19,4,4,1,141,10311,832,1327,55,18,28,20,20,20,0,45,0,0,0,100,30,4,4,0,0),(20,4,5,1,141,10311,832,1327,55,17,25,19,22,23,0,51,128,0,0,0,30,4,4,0,0),(21,4,11,1,141,10311,832,1327,55,18,25,19,22,22,0,53,67,0,0,0,30,4,4,0,0),(22,5,1,0,85,1676,1677,122,57,22,18,23,18,25,0,70,0,1000,0,0,27,4,5,0,0),(23,5,4,0,85,1676,1677,122,57,20,21,22,18,25,0,65,0,0,0,100,30,4,4,0,0),(24,5,5,0,85,1676,1677,122,57,19,18,21,20,28,0,71,128,0,0,0,30,4,4,0,0),(25,5,8,0,85,1676,1677,122,57,19,18,21,21,27,0,71,119,0,0,0,30,4,4,0,0),(26,5,9,0,85,1676,1677,122,57,19,18,22,20,27,0,63,109,0,0,0,30,4,4,0,0),(27,6,1,1,215,-2917,-257,53,59,28,15,24,15,22,0,80,0,1000,0,0,39,6,7,0,0),(28,6,3,1,215,-2917,-257,53,59,25,18,23,15,23,0,76,80,0,0,0,16,5,7,0,0),(29,6,7,1,215,-2917,-257,53,59,26,15,23,16,24,0,97,71,0,0,0,30,4,4,0,0),(30,6,11,1,215,-2917,-257,53,59,26,15,22,17,24,0,97,67,0,0,0,30,4,4,0,0),(31,7,1,0,1,-6240,331,383,1563,18,23,21,23,20,0,50,0,1000,0,0,10,3,4,0,0),(32,7,4,0,1,-6340,331,383,1563,16,26,20,23,20,0,45,0,0,0,100,30,4,4,0,0),(33,7,8,0,1,-6340,331,383,1563,15,23,19,26,22,0,51,119,0,0,0,30,4,4,0,0),(34,7,9,0,1,-6340,331,383,1563,15,23,20,25,22,0,43,109,0,0,0,30,4,4,0,0),(35,8,1,1,14,-618,-4251,39,1478,24,22,23,16,21,0,70,0,1000,0,0,29,5,7,0,0),(36,8,3,1,14,-618,-4251,39,1478,21,25,22,16,22,0,66,80,0,0,0,16,5,7,0,0),(37,8,4,1,14,-618,-4251,39,1478,22,25,22,16,21,0,65,0,0,0,100,30,4,4,0,0),(38,8,5,1,14,-618,-4251,39,1478,21,22,21,18,24,0,71,128,0,0,0,30,4,4,0,0),(39,8,7,1,14,-618,-4251,39,1478,22,22,22,17,23,0,87,71,0,0,0,30,4,4,0,0),(40,8,8,1,14,-618,-4251,39,1478,21,22,21,19,23,0,71,119,0,0,0,30,4,4,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_action`
--

DROP TABLE IF EXISTS `playercreateinfo_action`;
CREATE TABLE `playercreateinfo_action` (
  `createid` tinyint(3) unsigned NOT NULL default '0',
  `button` smallint(2) unsigned NOT NULL default '0',
  `action` smallint(6) unsigned NOT NULL default '0',
  `type` smallint(3) unsigned NOT NULL default '0',
  `misc` smallint(3) unsigned NOT NULL default '0',
  KEY `playercreateinfo_actions_index` (`button`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_action`
--


/*!40000 ALTER TABLE `playercreateinfo_action` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_action` WRITE;
INSERT INTO `playercreateinfo_action` VALUES (1,1,78,0,0),(1,0,6603,0,0),(1,11,65020,0,128),(2,2,635,0,0),(2,0,6603,0,0),(2,1,20154,0,0),(2,10,65021,0,128),(2,11,65022,0,128),(3,1,1752,0,0),(3,2,2098,0,0),(3,3,2764,0,0),(3,0,6603,0,0),(3,11,65022,0,128),(4,1,585,0,0),(4,2,2050,0,0),(4,0,6603,0,0),(4,10,65021,0,128),(4,11,65022,0,128),(5,1,133,0,0),(5,2,168,0,0),(5,0,6603,0,0),(5,10,65021,0,128),(5,11,65022,0,128),(6,1,686,0,0),(6,2,687,0,0),(6,0,6603,0,0),(6,10,65021,0,128),(6,11,65027,0,128),(7,1,78,0,0),(7,0,6603,0,0),(7,11,65020,0,128),(8,2,75,0,0),(8,1,2973,0,0),(8,0,6603,0,0),(8,11,65020,0,128),(8,10,65021,0,128),(9,10,0,0,128),(9,1,1752,0,0),(9,2,2098,0,0),(9,0,6603,0,0),(9,11,65020,0,128),(10,2,331,0,0),(10,1,403,0,0),(10,0,6603,0,0),(10,11,65020,0,128),(10,10,65021,0,128),(11,1,686,0,0),(11,2,687,0,0),(11,0,6603,0,0),(11,11,65020,0,128),(11,10,65021,0,128),(12,1,78,0,0),(12,0,6603,0,0),(12,11,65020,0,128),(13,2,635,0,0),(13,0,6603,0,0),(13,1,20154,0,0),(13,10,65021,0,128),(13,11,65026,0,128),(14,2,75,0,0),(14,1,2973,0,0),(14,0,6603,0,0),(14,11,65020,0,128),(14,10,65021,0,128),(15,1,1752,0,0),(15,2,2098,0,0),(15,3,2764,0,0),(15,0,6603,0,0),(15,11,65026,0,128),(16,1,585,0,0),(16,2,2050,0,0),(16,0,6603,0,0),(16,10,65021,0,128),(16,11,65026,0,128),(17,1,78,0,0),(17,0,6603,0,0),(17,11,65020,0,128),(18,2,75,0,0),(18,1,2973,0,0),(18,0,6603,0,0),(18,11,65020,0,128),(18,10,65021,0,128),(19,1,1752,0,0),(19,2,2098,0,0),(19,3,2764,0,0),(19,0,6603,0,0),(19,11,65026,0,128),(20,1,585,0,0),(20,2,2050,0,0),(20,0,6603,0,0),(20,10,65021,0,128),(20,11,65022,0,128),(21,1,5176,0,0),(21,2,5185,0,0),(21,0,6603,0,0),(21,10,65021,0,128),(21,11,65025,0,128),(22,11,65027,0,128),(22,0,6603,0,0),(22,1,78,0,0),(23,11,65027,0,128),(23,3,2764,0,0),(23,2,2098,0,0),(23,1,1752,0,0),(23,0,6603,0,0),(24,10,65021,0,128),(24,2,2050,0,0),(24,1,585,0,0),(24,11,65027,0,128),(24,0,6603,0,0),(25,11,65027,0,128),(25,10,65021,0,128),(25,2,168,0,0),(25,1,133,0,0),(25,0,6603,0,0),(26,1,686,0,0),(26,10,65021,0,128),(26,2,687,0,0),(26,11,65027,0,128),(26,0,6603,0,0),(27,1,78,0,0),(27,2,20549,0,0),(27,11,65026,0,128),(27,0,6603,0,0),(28,1,2973,0,0),(28,10,65021,0,128),(28,2,75,0,0),(28,3,20549,0,0),(28,11,65020,0,128),(28,0,6603,0,0),(29,1,403,0,0),(29,10,65021,0,128),(29,2,331,0,0),(29,3,20549,0,0),(29,11,65027,0,128),(29,0,6603,0,0),(30,1,5176,0,0),(30,10,65021,0,128),(30,2,5185,0,0),(30,3,20549,0,0),(30,11,65025,0,128),(30,0,6603,0,0),(31,11,65020,0,128),(31,1,78,0,0),(31,0,6603,0,0),(32,11,65020,0,128),(32,3,2764,0,0),(32,1,1752,0,0),(32,2,2098,0,0),(32,0,6603,0,0),(33,11,65025,0,128),(33,1,133,0,0),(33,2,168,0,0),(33,10,65021,0,128),(33,0,6603,0,0),(34,11,65027,0,128),(34,1,686,0,0),(34,2,687,0,0),(34,10,65021,0,128),(34,0,6603,0,0),(35,11,65020,0,128),(35,1,78,0,0),(35,3,2764,0,0),(35,0,6603,0,0),(36,10,65021,0,128),(36,11,65027,0,128),(36,1,2973,0,0),(36,2,75,0,0),(36,0,6603,0,0),(37,1,1752,0,0),(37,3,2764,0,0),(37,2,2098,0,0),(37,11,65020,0,128),(37,0,6603,0,0),(38,1,585,0,0),(38,10,65021,0,128),(38,2,2050,0,0),(38,11,65026,0,128),(38,0,6603,0,0),(39,1,403,0,0),(39,10,65021,0,128),(39,2,331,0,0),(39,11,65020,0,128),(39,0,6603,0,0),(40,1,133,0,0),(40,10,65021,0,128),(40,2,168,0,0),(40,11,65020,0,128),(40,0,6603,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_action` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_item`
--

DROP TABLE IF EXISTS `playercreateinfo_item`;
CREATE TABLE `playercreateinfo_item` (
  `createid` tinyint(3) unsigned NOT NULL default '0',
  `itemid` mediumint(8) unsigned NOT NULL default '0',
  `bagIndex` tinyint(3) unsigned NOT NULL default '255',
  `slot` tinyint(3) unsigned NOT NULL default '0',
  `amount` tinyint(8) unsigned NOT NULL default '1',
  KEY `playercreateinfo_items_index` (`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_item`
--


/*!40000 ALTER TABLE `playercreateinfo_item` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_item` WRITE;
INSERT INTO `playercreateinfo_item` VALUES (1,38,255,3,1),(1,39,255,6,1),(1,40,255,7,1),(1,25,255,15,1),(1,2362,255,16,1),(1,65020,255,23,4),(1,6948,255,24,1),(1,14646,255,25,1),(2,45,255,3,1),(2,44,255,6,1),(2,43,255,7,1),(2,2361,255,15,1),(2,6948,255,23,1),(2,65021,255,24,2),(2,65022,255,25,4),(2,14646,255,26,1),(3,49,255,3,1),(3,48,255,6,1),(3,47,255,7,1),(3,2092,255,15,1),(3,65023,255,17,100),(3,65022,255,23,4),(3,6948,255,24,1),(3,14646,255,25,1),(4,53,255,3,1),(4,6098,255,4,1),(4,52,255,6,1),(4,51,255,7,1),(4,36,255,15,1),(4,65021,255,23,2),(4,65022,255,24,4),(4,6948,255,25,1),(4,14646,255,26,1),(5,6096,255,3,1),(5,56,255,4,1),(5,1395,255,6,1),(5,55,255,7,1),(5,35,255,15,1),(5,65022,255,23,4),(5,65021,255,24,2),(5,6948,255,25,1),(5,14646,255,26,1),(6,6097,255,3,1),(6,57,255,4,1),(6,1396,255,6,1),(6,59,255,7,1),(6,2092,255,15,1),(6,65027,255,23,4),(6,65021,255,24,2),(6,6948,255,25,1),(6,14646,255,26,1),(7,6125,255,3,1),(7,139,255,6,1),(7,140,255,7,1),(7,12282,255,15,1),(7,6948,255,23,1),(7,65020,255,24,4),(7,14649,255,25,1),(8,127,255,3,1),(8,6126,255,6,1),(8,6127,255,7,1),(8,37,255,15,1),(8,2504,255,17,1),(8,65021,255,23,2),(8,65020,255,24,4),(8,6948,255,25,1),(8,14649,255,26,1),(8,2512,255,27,200),(8,2101,255,28,1),(9,2105,255,3,1),(9,120,255,6,1),(9,121,255,7,1),(9,2092,255,15,1),(9,65024,255,17,100),(9,65020,255,23,4),(9,6948,255,24,1),(9,14649,255,25,1),(10,154,255,3,1),(10,153,255,6,1),(10,36,255,15,1),(10,6948,255,23,1),(10,65020,255,24,4),(10,65021,255,25,2),(10,14649,255,26,1),(11,6129,255,4,1),(11,1396,255,6,1),(11,59,255,7,1),(11,2092,255,15,1),(11,6948,255,23,1),(11,65020,255,24,4),(11,65021,255,25,2),(11,14649,255,26,1),(12,38,255,3,1),(12,39,255,6,1),(12,40,255,7,1),(12,12282,255,15,1),(12,6948,255,23,1),(12,65020,255,24,4),(12,14647,255,25,1),(13,45,255,3,1),(13,44,255,6,1),(13,43,255,7,1),(13,2361,255,15,1),(13,65026,255,23,4),(13,65021,255,24,2),(13,6948,255,25,1),(13,14647,255,26,1),(14,148,255,3,1),(14,147,255,6,1),(14,129,255,7,1),(14,37,255,15,1),(14,2508,255,17,1),(14,65021,255,23,2),(14,65020,255,24,4),(14,6948,255,25,1),(14,14647,255,26,1),(14,2516,255,27,200),(14,2102,255,28,1),(15,49,255,3,1),(15,48,255,6,1),(15,47,255,7,1),(15,2092,255,15,1),(15,65024,255,17,100),(15,65026,255,23,4),(15,6948,255,24,1),(15,14647,255,25,1),(16,53,255,3,1),(16,6098,255,4,1),(16,52,255,6,1),(16,51,255,7,1),(16,36,255,15,1),(16,65021,255,23,2),(16,65026,255,24,4),(16,6948,255,25,1),(16,14647,255,26,1),(17,38,255,3,1),(17,39,255,6,1),(17,40,255,7,1),(17,25,255,15,1),(17,2362,255,16,1),(17,65020,255,23,4),(17,6948,255,24,1),(17,14648,255,25,1),(18,148,255,3,1),(18,147,255,6,1),(18,129,255,7,1),(18,2092,255,15,1),(18,2504,255,17,1),(18,65021,255,23,2),(18,65020,255,24,4),(18,6948,255,25,1),(18,14648,255,26,1),(18,2512,255,27,200),(18,2101,255,28,1),(19,49,255,3,1),(19,48,255,6,1),(19,47,255,7,1),(19,2092,255,15,1),(19,65023,255,17,100),(19,65026,255,23,4),(19,6948,255,24,1),(19,14648,255,25,1),(20,53,255,3,1),(20,6119,255,4,1),(20,52,255,6,1),(20,51,255,7,1),(20,36,255,15,1),(20,65022,255,23,4),(20,65021,255,24,2),(20,6948,255,25,1),(20,14648,255,26,1),(21,6123,255,4,1),(21,44,255,6,1),(21,3661,255,15,1),(21,65021,255,23,2),(21,65025,255,24,4),(21,6948,255,25,1),(21,14648,255,26,1),(22,6125,255,3,1),(22,139,255,6,1),(22,140,255,7,1),(22,25,255,15,1),(22,2362,255,16,1),(22,65027,255,23,4),(22,6948,255,24,1),(22,14651,255,25,1),(23,2105,255,3,1),(23,120,255,6,1),(23,121,255,7,1),(23,2092,255,15,1),(23,65023,255,17,100),(23,65027,255,23,4),(23,6948,255,24,1),(23,14651,255,25,1),(24,53,255,3,1),(24,6144,255,4,1),(24,52,255,6,1),(24,51,255,7,1),(24,36,255,15,1),(24,65027,255,23,4),(24,65021,255,24,2),(24,6948,255,25,1),(24,14651,255,26,1),(25,6096,255,3,1),(25,6140,255,4,1),(25,1395,255,6,1),(25,55,255,7,1),(25,35,255,15,1),(25,65027,255,23,4),(25,65021,255,24,2),(25,6948,255,25,1),(25,14651,255,26,1),(26,6129,255,4,1),(26,1396,255,6,1),(26,59,255,7,1),(26,2092,255,15,1),(26,65027,255,23,4),(26,65021,255,24,2),(26,6948,255,25,1),(26,14651,255,26,1),(27,6125,255,3,1),(27,139,255,6,1),(27,2361,255,15,1),(27,6948,255,23,1),(27,65026,255,24,4),(27,14650,255,25,1),(28,127,255,3,1),(28,6126,255,6,1),(28,37,255,15,1),(28,2508,255,17,1),(28,65021,255,23,2),(28,65020,255,24,4),(28,6948,255,25,1),(28,14650,255,26,1),(28,2516,255,27,200),(28,2102,255,28,1),(29,154,255,3,1),(29,153,255,6,1),(29,36,255,15,1),(29,6948,255,23,1),(29,65027,255,24,4),(29,65021,255,25,2),(29,14650,255,26,1),(30,6139,255,4,1),(30,6124,255,6,1),(30,35,255,15,1),(30,65021,255,23,2),(30,65025,255,24,4),(30,6948,255,25,1),(30,14650,255,26,1),(31,38,255,4,1),(31,39,255,6,1),(31,40,255,7,1),(31,25,255,15,1),(31,2362,255,16,1),(31,65020,255,23,4),(31,6948,255,24,1),(31,14647,255,25,1),(32,49,255,3,1),(32,48,255,6,1),(32,47,255,7,1),(32,2092,255,15,1),(32,65023,255,17,100),(32,65020,255,23,4),(32,6948,255,24,1),(32,14647,255,25,1),(33,6096,255,3,1),(33,56,255,4,1),(33,1395,255,6,1),(33,55,255,7,1),(33,35,255,15,1),(33,65025,255,23,4),(33,65021,255,24,2),(33,6948,255,25,1),(33,14647,255,26,1),(34,6097,255,3,1),(34,57,255,4,1),(34,1396,255,6,1),(34,59,255,7,1),(34,2092,255,15,1),(34,65021,255,23,2),(34,65027,255,24,4),(34,6948,255,25,1),(34,14647,255,26,1),(35,6125,255,3,1),(35,139,255,6,1),(35,140,255,7,1),(35,37,255,15,1),(35,2362,255,16,1),(35,65024,255,17,100),(35,65020,255,23,4),(35,6948,255,24,1),(35,14649,255,25,1),(36,127,255,3,1),(36,6126,255,6,1),(36,6127,255,7,1),(36,37,255,15,1),(36,2504,255,17,1),(36,65027,255,23,4),(36,65021,255,24,2),(36,2512,255,27,200),(36,2101,255,28,1),(36,14649,255,26,1),(36,6948,255,25,1),(37,2105,255,3,1),(37,120,255,6,1),(37,121,255,7,1),(37,2092,255,15,1),(37,65024,255,17,100),(37,65020,255,23,4),(37,6948,255,24,1),(37,14649,255,25,1),(38,53,255,3,1),(38,6144,255,4,1),(38,52,255,6,1),(38,36,255,15,1),(38,65026,255,23,4),(38,65021,255,24,2),(38,6948,255,25,1),(38,14649,255,26,1),(39,6134,255,3,1),(39,6135,255,6,1),(39,36,255,15,1),(39,65020,255,23,4),(39,65021,255,24,2),(39,6948,255,25,1),(39,14649,255,26,1),(40,6096,255,3,1),(40,6140,255,4,1),(40,1395,255,6,1),(40,55,255,7,1),(40,35,255,15,1),(40,65020,255,23,4),(40,65021,255,24,2),(40,6948,255,25,1),(40,14649,255,26,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_item` ENABLE KEYS */;

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
INSERT INTO `playercreateinfo_reputation` VALUES (1,0,0,0),(2,0,0,0),(3,0,0,0),(4,0,0,0),(5,0,0,0),(6,0,0,0),(7,0,0,0),(8,0,0,0),(9,0,0,0),(10,0,0,0),(15,0,0,0),(11,0,0,0),(12,0,0,0),(16,0,0,0),(17,0,0,0),(18,0,0,0),(19,0,0,0),(20,0,0,0),(21,0,0,0),(22,0,0,0),(23,0,0,0),(24,0,0,0),(25,0,0,0),(26,0,0,0),(27,0,0,0),(28,0,0,0),(29,0,0,0),(30,0,0,0),(31,0,0,0),(32,0,0,0),(34,0,0,0),(35,0,0,0),(36,0,0,0),(37,0,0,0),(38,0,0,0),(39,0,0,0),(40,0,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_reputation` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_skill`
--

DROP TABLE IF EXISTS `playercreateinfo_skill`;
CREATE TABLE `playercreateinfo_skill` (
  `createid` smallint(5) unsigned NOT NULL default '0',
  `Skill` mediumint(8) unsigned NOT NULL default '0',
  `SkillMin` smallint(5) unsigned NOT NULL default '0',
  `SkillMax` smallint(5) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_skill`
--


/*!40000 ALTER TABLE `playercreateinfo_skill` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_skill` WRITE;
INSERT INTO `playercreateinfo_skill` VALUES (1,148,0,0,'Horse Riding'),(1,150,0,0,'Tiger Riding'),(1,164,0,0,'Blacksmithing'),(1,172,0,5,'Two-Handed Axes'),(1,173,0,5,'Daggers'),(1,183,5,5,'GENERIC (DND)'),(1,197,0,0,'Tailoring'),(1,226,0,5,'Crossbows'),(1,754,5,5,'Racial - Human'),(1,393,0,0,'Skinning'),(1,109,0,0,'Language: Orcish'),(1,433,1,1,'Shield'),(1,137,0,0,'Language: Thalassian'),(1,473,0,1,'Fist Weapons'),(1,140,0,0,'Language: Titan'),(1,149,0,0,'Wolf Riding'),(1,229,0,0,'Polearms'),(1,257,0,5,'Protection'),(1,293,0,0,'Plate Mail'),(1,98,300,300,'Language: Common'),(1,315,0,0,'Language: Troll'),(1,139,0,0,'Language: Demon Tongue'),(1,152,0,0,'Ram Riding'),(1,165,0,0,'Leatherworking'),(1,415,1,1,'Cloth'),(1,256,0,5,'Fury'),(1,673,0,0,'Language: Gutterspeak'),(1,95,1,5,'Defense'),(1,333,0,0,'Enchanting'),(1,186,0,0,'Mining'),(1,43,1,5,'Swords'),(1,55,0,5,'Two-Handed Swords'),(1,713,0,0,'Kodo Riding'),(1,176,0,5,'Thrown'),(1,26,5,5,'Arms'),(1,44,1,5,'Axes'),(1,111,0,0,'Language: Dwarven'),(1,113,0,0,'Language: Darnassian'),(1,115,0,0,'Language: Taurahe'),(1,118,0,0,'Dual Wield'),(1,136,0,5,'Staves'),(1,138,0,0,'Language: Draconic'),(1,141,0,0,'Language: Old Tongue'),(1,142,0,1,'Survival'),(1,160,0,5,'Two-Handed Maces'),(1,313,0,0,'Language: Gnomish'),(1,413,1,1,'Mail'),(1,414,1,1,'Leather'),(1,45,0,5,'Bows'),(1,46,0,5,'Guns'),(1,533,0,0,'Raptor Riding'),(1,554,0,0,'Undead Horsemanship'),(1,202,0,0,'Engineering'),(1,356,0,0,'Fishing'),(1,171,0,0,'Alchemy'),(1,54,1,5,'Maces'),(1,182,0,0,'Herbalism'),(1,129,0,0,'First Aid'),(1,185,0,0,'Cooking'),(1,162,1,5,'Unarmed'),(2,148,0,0,'Horse Riding'),(2,150,0,0,'Tiger Riding'),(2,164,0,0,'Blacksmithing'),(2,172,0,5,'Two-Handed Axes'),(2,183,5,5,'GENERIC (DND)'),(2,184,0,5,'Retribution'),(2,197,0,0,'Tailoring'),(2,754,5,5,'Racial - Human'),(2,393,0,0,'Skinning'),(2,109,0,0,'Language: Orcish'),(2,433,1,1,'Shield'),(2,137,0,0,'Language: Thalassian'),(2,140,0,0,'Language: Titan'),(2,149,0,0,'Wolf Riding'),(2,229,0,0,'Polearms'),(2,293,0,0,'Plate Mail'),(2,98,300,300,'Language: Common'),(2,315,0,0,'Language: Troll'),(2,139,0,0,'Language: Demon Tongue'),(2,152,0,0,'Ram Riding'),(2,165,0,0,'Leatherworking'),(2,415,1,1,'Cloth'),(2,594,5,5,'Holy'),(2,673,0,0,'Language: Gutterspeak'),(2,95,1,5,'Defense'),(2,333,0,0,'Enchanting'),(2,186,0,0,'Mining'),(2,43,0,5,'Swords'),(2,55,0,5,'Two-Handed Swords'),(2,713,0,0,'Kodo Riding'),(2,44,0,5,'Axes'),(2,111,0,0,'Language: Dwarven'),(2,113,0,0,'Language: Darnassian'),(2,115,0,0,'Language: Taurahe'),(2,138,0,0,'Language: Draconic'),(2,141,0,0,'Language: Old Tongue'),(2,142,0,1,'Survival'),(2,160,1,5,'Two-Handed Maces'),(2,313,0,0,'Language: Gnomish'),(2,413,1,1,'Mail'),(2,414,1,1,'Leather'),(2,533,0,0,'Raptor Riding'),(2,554,0,0,'Undead Horsemanship'),(2,202,0,0,'Engineering'),(2,356,0,0,'Fishing'),(2,171,0,0,'Alchemy'),(2,54,1,5,'Maces'),(2,182,0,0,'Herbalism'),(2,129,0,0,'First Aid'),(2,185,0,0,'Cooking'),(2,267,0,5,'Protection'),(2,162,1,5,'Unarmed'),(3,148,0,0,'Horse Riding'),(3,150,0,0,'Tiger Riding'),(3,164,0,0,'Blacksmithing'),(3,173,1,5,'Daggers'),(3,183,5,5,'GENERIC (DND)'),(3,197,0,0,'Tailoring'),(3,226,0,5,'Crossbows'),(3,754,5,5,'Racial - Human'),(3,393,0,0,'Skinning'),(3,109,0,0,'Language: Orcish'),(3,137,0,0,'Language: Thalassian'),(3,473,0,1,'Fist Weapons'),(3,140,0,0,'Language: Titan'),(3,149,0,0,'Wolf Riding'),(3,98,300,300,'Language: Common'),(3,315,0,0,'Language: Troll'),(3,139,0,0,'Language: Demon Tongue'),(3,152,0,0,'Ram Riding'),(3,165,0,0,'Leatherworking'),(3,415,1,1,'Cloth'),(3,633,0,5,'Lockpicking'),(3,673,0,0,'Language: Gutterspeak'),(3,95,1,5,'Defense'),(3,333,0,0,'Enchanting'),(3,186,0,0,'Mining'),(3,43,0,5,'Swords'),(3,713,0,0,'Kodo Riding'),(3,176,1,5,'Thrown'),(3,38,5,5,'Combat'),(3,39,0,5,'Subtlety'),(3,111,0,0,'Language: Dwarven'),(3,113,0,0,'Language: Darnassian'),(3,115,0,0,'Language: Taurahe'),(3,118,0,0,'Dual Wield'),(3,138,0,0,'Language: Draconic'),(3,141,0,0,'Language: Old Tongue'),(3,142,0,1,'Survival'),(3,253,5,5,'Assassination'),(3,313,0,0,'Language: Gnomish'),(3,414,1,1,'Leather'),(3,45,0,5,'Bows'),(3,46,0,5,'Guns'),(3,533,0,0,'Raptor Riding'),(3,554,0,0,'Undead Horsemanship'),(3,202,0,0,'Engineering'),(3,356,0,0,'Fishing'),(3,171,0,0,'Alchemy'),(3,54,0,5,'Maces'),(3,182,0,0,'Herbalism'),(3,129,0,0,'First Aid'),(3,185,0,0,'Cooking'),(3,40,0,5,'Poisons'),(3,162,1,5,'Unarmed'),(4,148,0,0,'Horse Riding'),(4,150,0,0,'Tiger Riding'),(4,164,0,0,'Blacksmithing'),(4,173,0,5,'Daggers'),(4,183,5,5,'GENERIC (DND)'),(4,197,0,0,'Tailoring'),(4,228,1,5,'Wands'),(4,754,5,5,'Racial - Human'),(4,393,0,0,'Skinning'),(4,109,0,0,'Language: Orcish'),(4,137,0,0,'Language: Thalassian'),(4,140,0,0,'Language: Titan'),(4,149,0,0,'Wolf Riding'),(4,98,300,300,'Language: Common'),(4,315,0,0,'Language: Troll'),(4,139,0,0,'Language: Demon Tongue'),(4,152,0,0,'Ram Riding'),(4,165,0,0,'Leatherworking'),(4,415,1,1,'Cloth'),(4,673,0,0,'Language: Gutterspeak'),(4,95,1,5,'Defense'),(4,333,0,0,'Enchanting'),(4,186,0,0,'Mining'),(4,713,0,0,'Kodo Riding'),(4,56,5,5,'Holy'),(4,78,0,5,'Shadow Magic'),(4,111,0,0,'Language: Dwarven'),(4,113,0,0,'Language: Darnassian'),(4,115,0,0,'Language: Taurahe'),(4,136,0,5,'Staves'),(4,138,0,0,'Language: Draconic'),(4,141,0,0,'Language: Old Tongue'),(4,142,0,1,'Survival'),(4,313,0,0,'Language: Gnomish'),(4,613,0,5,'Discipline'),(4,533,0,0,'Raptor Riding'),(4,554,0,0,'Undead Horsemanship'),(4,202,0,0,'Engineering'),(4,356,0,0,'Fishing'),(4,171,0,0,'Alchemy'),(4,54,1,5,'Maces'),(4,182,0,0,'Herbalism'),(4,129,0,0,'First Aid'),(4,185,0,0,'Cooking'),(4,162,1,5,'Unarmed'),(5,148,0,0,'Horse Riding'),(5,150,0,0,'Tiger Riding'),(5,164,0,0,'Blacksmithing'),(5,173,0,5,'Daggers'),(5,183,5,5,'GENERIC (DND)'),(5,197,0,0,'Tailoring'),(5,228,1,5,'Wands'),(5,754,5,5,'Racial - Human'),(5,393,0,0,'Skinning'),(5,109,0,0,'Language: Orcish'),(5,137,0,0,'Language: Thalassian'),(5,140,0,0,'Language: Titan'),(5,149,0,0,'Wolf Riding'),(5,8,5,5,'Fire'),(5,98,300,300,'Language: Common'),(5,315,0,0,'Language: Troll'),(5,139,0,0,'Language: Demon Tongue'),(5,152,0,0,'Ram Riding'),(5,165,0,0,'Leatherworking'),(5,415,1,1,'Cloth'),(5,673,0,0,'Language: Gutterspeak'),(5,95,1,5,'Defense'),(5,333,0,0,'Enchanting'),(5,186,0,0,'Mining'),(5,43,0,5,'Swords'),(5,713,0,0,'Kodo Riding'),(5,6,5,5,'Frost'),(5,111,0,0,'Language: Dwarven'),(5,113,0,0,'Language: Darnassian'),(5,115,0,0,'Language: Taurahe'),(5,136,1,5,'Staves'),(5,138,0,0,'Language: Draconic'),(5,141,0,0,'Language: Old Tongue'),(5,142,0,1,'Survival'),(5,313,0,0,'Language: Gnomish'),(5,533,0,0,'Raptor Riding'),(5,554,0,0,'Undead Horsemanship'),(5,202,0,0,'Engineering'),(5,356,0,0,'Fishing'),(5,171,0,0,'Alchemy'),(5,182,0,0,'Herbalism'),(5,129,0,0,'First Aid'),(5,185,0,0,'Cooking'),(5,237,0,5,'Arcane'),(5,162,1,5,'Unarmed'),(6,148,0,0,'Horse Riding'),(6,150,0,0,'Tiger Riding'),(6,164,0,0,'Blacksmithing'),(6,173,1,5,'Daggers'),(6,183,5,5,'GENERIC (DND)'),(6,197,0,0,'Tailoring'),(6,228,1,5,'Wands'),(6,754,5,5,'Racial - Human'),(6,393,0,0,'Skinning'),(6,109,0,0,'Language: Orcish'),(6,137,0,0,'Language: Thalassian'),(6,140,0,0,'Language: Titan'),(6,149,0,0,'Wolf Riding'),(6,593,5,5,'Destruction'),(6,98,300,300,'Language: Common'),(6,315,0,0,'Language: Troll'),(6,139,0,0,'Language: Demon Tongue'),(6,152,0,0,'Ram Riding'),(6,354,5,5,'Demonology'),(6,355,0,5,'Affliction'),(6,165,0,0,'Leatherworking'),(6,415,1,1,'Cloth'),(6,673,0,0,'Language: Gutterspeak'),(6,95,1,5,'Defense'),(6,333,0,0,'Enchanting'),(6,186,0,0,'Mining'),(6,43,0,5,'Swords'),(6,713,0,0,'Kodo Riding'),(6,111,0,0,'Language: Dwarven'),(6,113,0,0,'Language: Darnassian'),(6,115,0,0,'Language: Taurahe'),(6,136,0,5,'Staves'),(6,138,0,0,'Language: Draconic'),(6,141,0,0,'Language: Old Tongue'),(6,142,0,1,'Survival'),(6,313,0,0,'Language: Gnomish'),(6,533,0,0,'Raptor Riding'),(6,554,0,0,'Undead Horsemanship'),(6,202,0,0,'Engineering'),(6,356,0,0,'Fishing'),(6,171,0,0,'Alchemy'),(6,182,0,0,'Herbalism'),(6,129,0,0,'First Aid'),(6,185,0,0,'Cooking'),(6,162,1,5,'Unarmed'),(7,148,0,0,'Horse Riding'),(7,150,0,0,'Tiger Riding'),(7,164,0,0,'Blacksmithing'),(7,172,1,5,'Two-Handed Axes'),(7,173,0,5,'Daggers'),(7,183,5,5,'GENERIC (DND)'),(7,197,0,0,'Tailoring'),(7,226,0,5,'Crossbows'),(7,393,0,0,'Skinning'),(7,109,300,300,'Language: Orcish'),(7,433,1,1,'Shield'),(7,137,0,0,'Language: Thalassian'),(7,473,0,1,'Fist Weapons'),(7,140,0,0,'Language: Titan'),(7,149,0,0,'Wolf Riding'),(7,229,0,0,'Polearms'),(7,257,0,5,'Protection'),(7,293,0,0,'Plate Mail'),(7,98,0,0,'Language: Common'),(7,125,5,5,'Orc Racial'),(7,315,0,0,'Language: Troll'),(7,139,0,0,'Language: Demon Tongue'),(7,152,0,0,'Ram Riding'),(7,165,0,0,'Leatherworking'),(7,415,1,1,'Cloth'),(7,256,0,5,'Fury'),(7,673,0,0,'Language: Gutterspeak'),(7,95,1,5,'Defense'),(7,333,0,0,'Enchanting'),(7,186,0,0,'Mining'),(7,43,1,5,'Swords'),(7,55,0,5,'Two-Handed Swords'),(7,713,0,0,'Kodo Riding'),(7,176,0,5,'Thrown'),(7,26,5,5,'Arms'),(7,44,1,5,'Axes'),(7,111,0,0,'Language: Dwarven'),(7,113,0,0,'Language: Darnassian'),(7,115,0,0,'Language: Taurahe'),(7,118,0,0,'Dual Wield'),(7,136,0,5,'Staves'),(7,138,0,0,'Language: Draconic'),(7,141,0,0,'Language: Old Tongue'),(7,142,0,1,'Survival'),(7,160,0,5,'Two-Handed Maces'),(7,313,0,0,'Language: Gnomish'),(7,413,1,1,'Mail'),(7,414,1,1,'Leather'),(7,45,0,5,'Bows'),(7,46,0,5,'Guns'),(7,533,0,0,'Raptor Riding'),(7,554,0,0,'Undead Horsemanship'),(7,202,0,0,'Engineering'),(7,356,0,0,'Fishing'),(7,171,0,0,'Alchemy'),(7,54,0,5,'Maces'),(7,182,0,0,'Herbalism'),(7,129,0,0,'First Aid'),(7,185,0,0,'Cooking'),(7,162,1,5,'Unarmed'),(8,148,0,0,'Horse Riding'),(8,150,0,0,'Tiger Riding'),(8,164,0,0,'Blacksmithing'),(8,172,0,5,'Two-Handed Axes'),(8,173,0,5,'Daggers'),(8,183,5,5,'GENERIC (DND)'),(8,197,0,0,'Tailoring'),(8,226,0,5,'Crossbows'),(8,393,0,0,'Skinning'),(8,109,300,300,'Language: Orcish'),(8,137,0,0,'Language: Thalassian'),(8,473,0,1,'Fist Weapons'),(8,140,0,0,'Language: Titan'),(8,149,0,0,'Wolf Riding'),(8,229,0,0,'Polearms'),(8,98,0,0,'Language: Common'),(8,125,5,5,'Orc Racial'),(8,315,0,0,'Language: Troll'),(8,139,0,0,'Language: Demon Tongue'),(8,152,0,0,'Ram Riding'),(8,165,0,0,'Leatherworking'),(8,415,1,1,'Cloth'),(8,673,0,0,'Language: Gutterspeak'),(8,95,1,5,'Defense'),(8,333,0,0,'Enchanting'),(8,186,0,0,'Mining'),(8,43,0,5,'Swords'),(8,55,0,5,'Two-Handed Swords'),(8,713,0,0,'Kodo Riding'),(8,176,0,5,'Thrown'),(8,44,1,5,'Axes'),(8,50,0,5,'Beast Mastery'),(8,51,5,5,'Survival'),(8,111,0,0,'Language: Dwarven'),(8,113,0,0,'Language: Darnassian'),(8,115,0,0,'Language: Taurahe'),(8,118,0,0,'Dual Wield'),(8,136,0,5,'Staves'),(8,138,0,0,'Language: Draconic'),(8,141,0,0,'Language: Old Tongue'),(8,142,0,1,'Survival'),(8,313,0,0,'Language: Gnomish'),(8,413,0,0,'Mail'),(8,414,1,1,'Leather'),(8,45,1,5,'Bows'),(8,163,5,5,'Marksmanship'),(8,46,0,5,'Guns'),(8,533,0,0,'Raptor Riding'),(8,554,0,0,'Undead Horsemanship'),(8,202,0,0,'Engineering'),(8,356,0,0,'Fishing'),(8,171,0,0,'Alchemy'),(8,182,0,0,'Herbalism'),(8,129,0,0,'First Aid'),(8,185,0,0,'Cooking'),(8,261,0,5,'Beast Training'),(8,162,1,5,'Unarmed'),(9,148,0,0,'Horse Riding'),(9,150,0,0,'Tiger Riding'),(9,164,0,0,'Blacksmithing'),(9,173,1,5,'Daggers'),(9,183,5,5,'GENERIC (DND)'),(9,197,0,0,'Tailoring'),(9,226,0,5,'Crossbows'),(9,393,0,0,'Skinning'),(9,109,300,300,'Language: Orcish'),(9,137,0,0,'Language: Thalassian'),(9,473,0,1,'Fist Weapons'),(9,140,0,0,'Language: Titan'),(9,149,0,0,'Wolf Riding'),(9,98,0,0,'Language: Common'),(9,125,5,5,'Orc Racial'),(9,315,0,0,'Language: Troll'),(9,139,0,0,'Language: Demon Tongue'),(9,152,0,0,'Ram Riding'),(9,165,0,0,'Leatherworking'),(9,415,1,1,'Cloth'),(9,633,0,5,'Lockpicking'),(9,673,0,0,'Language: Gutterspeak'),(9,95,1,5,'Defense'),(9,333,0,0,'Enchanting'),(9,186,0,0,'Mining'),(9,43,0,5,'Swords'),(9,713,0,0,'Kodo Riding'),(9,176,1,5,'Thrown'),(9,38,5,5,'Combat'),(9,39,0,5,'Subtlety'),(9,111,0,0,'Language: Dwarven'),(9,113,0,0,'Language: Darnassian'),(9,115,0,0,'Language: Taurahe'),(9,118,0,0,'Dual Wield'),(9,138,0,0,'Language: Draconic'),(9,141,0,0,'Language: Old Tongue'),(9,142,0,1,'Survival'),(9,253,5,5,'Assassination'),(9,313,0,0,'Language: Gnomish'),(9,414,1,1,'Leather'),(9,45,0,5,'Bows'),(9,46,0,5,'Guns'),(9,533,0,0,'Raptor Riding'),(9,554,0,0,'Undead Horsemanship'),(9,202,0,0,'Engineering'),(9,356,0,0,'Fishing'),(9,171,0,0,'Alchemy'),(9,54,0,5,'Maces'),(9,182,0,0,'Herbalism'),(9,129,0,0,'First Aid'),(9,185,0,0,'Cooking'),(9,40,0,5,'Poisons'),(9,162,1,5,'Unarmed'),(10,148,0,0,'Horse Riding'),(10,150,0,0,'Tiger Riding'),(10,164,0,0,'Blacksmithing'),(10,172,0,5,'Two-Handed Axes'),(10,173,0,5,'Daggers'),(10,183,5,5,'GENERIC (DND)'),(10,197,0,0,'Tailoring'),(10,393,0,0,'Skinning'),(10,109,300,300,'Language: Orcish'),(10,433,1,1,'Shield'),(10,137,0,0,'Language: Thalassian'),(10,473,0,1,'Fist Weapons'),(10,140,0,0,'Language: Titan'),(10,149,0,0,'Wolf Riding'),(10,98,0,0,'Language: Common'),(10,125,5,5,'Orc Racial'),(10,315,0,0,'Language: Troll'),(10,139,0,0,'Language: Demon Tongue'),(10,152,0,0,'Ram Riding'),(10,165,0,0,'Leatherworking'),(10,373,0,5,'Enhancement'),(10,374,5,5,'Restoration'),(10,375,5,5,'Elemental Combat'),(10,415,1,1,'Cloth'),(10,673,0,0,'Language: Gutterspeak'),(10,95,1,5,'Defense'),(10,333,0,0,'Enchanting'),(10,186,0,0,'Mining'),(10,713,0,0,'Kodo Riding'),(10,44,0,5,'Axes'),(10,111,0,0,'Language: Dwarven'),(10,113,0,0,'Language: Darnassian'),(10,115,0,0,'Language: Taurahe'),(10,136,1,5,'Staves'),(10,138,0,0,'Language: Draconic'),(10,141,0,0,'Language: Old Tongue'),(10,142,0,1,'Survival'),(10,160,0,5,'Two-Handed Maces'),(10,313,0,0,'Language: Gnomish'),(10,413,0,0,'Mail'),(10,414,1,1,'Leather'),(10,533,0,0,'Raptor Riding'),(10,554,0,0,'Undead Horsemanship'),(10,202,0,0,'Engineering'),(10,356,0,0,'Fishing'),(10,171,0,0,'Alchemy'),(10,54,1,5,'Maces'),(10,182,0,0,'Herbalism'),(10,129,0,0,'First Aid'),(10,185,0,0,'Cooking'),(10,162,1,5,'Unarmed'),(11,148,0,0,'Horse Riding'),(11,150,0,0,'Tiger Riding'),(11,164,0,0,'Blacksmithing'),(11,173,1,5,'Daggers'),(11,183,5,5,'GENERIC (DND)'),(11,197,0,0,'Tailoring'),(11,228,1,5,'Wands'),(11,393,0,0,'Skinning'),(11,109,300,300,'Language: Orcish'),(11,137,0,0,'Language: Thalassian'),(11,140,0,0,'Language: Titan'),(11,149,0,0,'Wolf Riding'),(11,593,5,5,'Destruction'),(11,98,0,0,'Language: Common'),(11,125,5,5,'Orc Racial'),(11,315,0,0,'Language: Troll'),(11,139,0,0,'Language: Demon Tongue'),(11,152,0,0,'Ram Riding'),(11,354,5,5,'Demonology'),(11,355,0,5,'Affliction'),(11,165,0,0,'Leatherworking'),(11,415,1,1,'Cloth'),(11,673,0,0,'Language: Gutterspeak'),(11,95,1,5,'Defense'),(11,333,0,0,'Enchanting'),(11,186,0,0,'Mining'),(11,43,0,5,'Swords'),(11,713,0,0,'Kodo Riding'),(11,111,0,0,'Language: Dwarven'),(11,113,0,0,'Language: Darnassian'),(11,115,0,0,'Language: Taurahe'),(11,136,0,5,'Staves'),(11,138,0,0,'Language: Draconic'),(11,141,0,0,'Language: Old Tongue'),(11,142,0,1,'Survival'),(11,313,0,0,'Language: Gnomish'),(11,533,0,0,'Raptor Riding'),(11,554,0,0,'Undead Horsemanship'),(11,202,0,0,'Engineering'),(11,356,0,0,'Fishing'),(11,171,0,0,'Alchemy'),(11,182,0,0,'Herbalism'),(11,129,0,0,'First Aid'),(11,185,0,0,'Cooking'),(11,162,1,5,'Unarmed'),(12,148,0,0,'Horse Riding'),(12,150,0,0,'Tiger Riding'),(12,164,0,0,'Blacksmithing'),(12,172,1,5,'Two-Handed Axes'),(12,173,0,5,'Daggers'),(12,183,5,5,'GENERIC (DND)'),(12,197,0,0,'Tailoring'),(12,226,0,5,'Crossbows'),(12,393,0,0,'Skinning'),(12,109,0,0,'Language: Orcish'),(12,433,1,1,'Shield'),(12,137,0,0,'Language: Thalassian'),(12,473,0,1,'Fist Weapons'),(12,140,0,0,'Language: Titan'),(12,149,0,0,'Wolf Riding'),(12,229,0,0,'Polearms'),(12,257,0,5,'Protection'),(12,293,0,0,'Plate Mail'),(12,98,300,300,'Language: Common'),(12,315,0,0,'Language: Troll'),(12,139,0,0,'Language: Demon Tongue'),(12,152,0,0,'Ram Riding'),(12,165,0,0,'Leatherworking'),(12,415,1,1,'Cloth'),(12,256,0,5,'Fury'),(12,673,0,0,'Language: Gutterspeak'),(12,95,1,5,'Defense'),(12,333,0,0,'Enchanting'),(12,186,0,0,'Mining'),(12,43,0,5,'Swords'),(12,55,0,5,'Two-Handed Swords'),(12,713,0,0,'Kodo Riding'),(12,176,0,5,'Thrown'),(12,26,5,5,'Arms'),(12,44,1,5,'Axes'),(12,101,5,5,'Dwarven Racial'),(12,111,300,300,'Language: Dwarven'),(12,113,0,0,'Language: Darnassian'),(12,115,0,0,'Language: Taurahe'),(12,118,0,0,'Dual Wield'),(12,136,0,5,'Staves'),(12,138,0,0,'Language: Draconic'),(12,141,0,0,'Language: Old Tongue'),(12,142,0,1,'Survival'),(12,160,0,5,'Two-Handed Maces'),(12,313,0,0,'Language: Gnomish'),(12,413,1,1,'Mail'),(12,414,1,1,'Leather'),(12,45,0,5,'Bows'),(12,46,0,5,'Guns'),(12,533,0,0,'Raptor Riding'),(12,553,0,0,'Mechanostrider Piloting'),(12,554,0,0,'Undead Horsemanship'),(12,202,0,0,'Engineering'),(12,356,0,0,'Fishing'),(12,171,0,0,'Alchemy'),(12,54,1,5,'Maces'),(12,182,0,0,'Herbalism'),(12,129,0,0,'First Aid'),(12,185,0,0,'Cooking'),(12,162,1,5,'Unarmed'),(13,148,0,0,'Horse Riding'),(13,150,0,0,'Tiger Riding'),(13,164,0,0,'Blacksmithing'),(13,172,0,5,'Two-Handed Axes'),(13,183,5,5,'GENERIC (DND)'),(13,184,0,5,'Retribution'),(13,197,0,0,'Tailoring'),(13,393,0,0,'Skinning'),(13,109,0,0,'Language: Orcish'),(13,433,1,1,'Shield'),(13,137,0,0,'Language: Thalassian'),(13,140,0,0,'Language: Titan'),(13,149,0,0,'Wolf Riding'),(13,229,0,0,'Polearms'),(13,293,0,0,'Plate Mail'),(13,98,300,300,'Language: Common'),(13,315,0,0,'Language: Troll'),(13,139,0,0,'Language: Demon Tongue'),(13,152,0,0,'Ram Riding'),(13,165,0,0,'Leatherworking'),(13,415,1,1,'Cloth'),(13,594,5,5,'Holy'),(13,673,0,0,'Language: Gutterspeak'),(13,95,1,5,'Defense'),(13,333,0,0,'Enchanting'),(13,186,0,0,'Mining'),(13,43,0,5,'Swords'),(13,55,0,5,'Two-Handed Swords'),(13,713,0,0,'Kodo Riding'),(13,44,0,5,'Axes'),(13,101,5,5,'Dwarven Racial'),(13,111,300,300,'Language: Dwarven'),(13,113,0,0,'Language: Darnassian'),(13,115,0,0,'Language: Taurahe'),(13,138,0,0,'Language: Draconic'),(13,141,0,0,'Language: Old Tongue'),(13,142,0,1,'Survival'),(13,160,1,5,'Two-Handed Maces'),(13,313,0,0,'Language: Gnomish'),(13,413,1,1,'Mail'),(13,414,1,1,'Leather'),(13,533,0,0,'Raptor Riding'),(13,553,0,0,'Mechanostrider Piloting'),(13,554,0,0,'Undead Horsemanship'),(13,202,0,0,'Engineering'),(13,356,0,0,'Fishing'),(13,171,0,0,'Alchemy'),(13,54,1,5,'Maces'),(13,182,0,0,'Herbalism'),(13,129,0,0,'First Aid'),(13,185,0,0,'Cooking'),(13,267,0,5,'Protection'),(13,162,1,5,'Unarmed'),(14,148,0,0,'Horse Riding'),(14,150,0,0,'Tiger Riding'),(14,164,0,0,'Blacksmithing'),(14,172,0,5,'Two-Handed Axes'),(14,173,0,5,'Daggers'),(14,183,5,5,'GENERIC (DND)'),(14,197,0,0,'Tailoring'),(14,226,0,5,'Crossbows'),(14,393,0,0,'Skinning'),(14,109,0,0,'Language: Orcish'),(14,137,0,0,'Language: Thalassian'),(14,473,0,1,'Fist Weapons'),(14,140,0,0,'Language: Titan'),(14,149,0,0,'Wolf Riding'),(14,229,0,0,'Polearms'),(14,98,300,300,'Language: Common'),(14,315,0,0,'Language: Troll'),(14,139,0,0,'Language: Demon Tongue'),(14,152,0,0,'Ram Riding'),(14,165,0,0,'Leatherworking'),(14,415,1,1,'Cloth'),(14,673,0,0,'Language: Gutterspeak'),(14,95,1,5,'Defense'),(14,333,0,0,'Enchanting'),(14,186,0,0,'Mining'),(14,43,0,5,'Swords'),(14,55,0,5,'Two-Handed Swords'),(14,713,0,0,'Kodo Riding'),(14,176,0,5,'Thrown'),(14,44,1,5,'Axes'),(14,50,0,5,'Beast Mastery'),(14,51,5,5,'Survival'),(14,101,5,5,'Dwarven Racial'),(14,111,300,300,'Language: Dwarven'),(14,113,0,0,'Language: Darnassian'),(14,115,0,0,'Language: Taurahe'),(14,118,0,0,'Dual Wield'),(14,136,0,5,'Staves'),(14,138,0,0,'Language: Draconic'),(14,141,0,0,'Language: Old Tongue'),(14,142,0,1,'Survival'),(14,313,0,0,'Language: Gnomish'),(14,413,0,0,'Mail'),(14,414,1,1,'Leather'),(14,45,0,5,'Bows'),(14,163,5,5,'Marksmanship'),(14,46,1,5,'Guns'),(14,533,0,0,'Raptor Riding'),(14,553,0,0,'Mechanostrider Piloting'),(14,554,0,0,'Undead Horsemanship'),(14,202,0,0,'Engineering'),(14,356,0,0,'Fishing'),(14,171,0,0,'Alchemy'),(14,182,0,0,'Herbalism'),(14,129,0,0,'First Aid'),(14,185,0,0,'Cooking'),(14,261,0,5,'Beast Training'),(14,162,1,5,'Unarmed'),(15,148,0,0,'Horse Riding'),(15,150,0,0,'Tiger Riding'),(15,164,0,0,'Blacksmithing'),(15,173,1,5,'Daggers'),(15,183,5,5,'GENERIC (DND)'),(15,197,0,0,'Tailoring'),(15,226,0,5,'Crossbows'),(15,393,0,0,'Skinning'),(15,109,0,0,'Language: Orcish'),(15,137,0,0,'Language: Thalassian'),(15,473,0,1,'Fist Weapons'),(15,140,0,0,'Language: Titan'),(15,149,0,0,'Wolf Riding'),(15,98,300,300,'Language: Common'),(15,315,0,0,'Language: Troll'),(15,139,0,0,'Language: Demon Tongue'),(15,152,0,0,'Ram Riding'),(15,165,0,0,'Leatherworking'),(15,415,1,1,'Cloth'),(15,633,0,5,'Lockpicking'),(15,673,0,0,'Language: Gutterspeak'),(15,95,1,5,'Defense'),(15,333,0,0,'Enchanting'),(15,186,0,0,'Mining'),(15,43,0,5,'Swords'),(15,713,0,0,'Kodo Riding'),(15,176,1,5,'Thrown'),(15,38,5,5,'Combat'),(15,39,0,5,'Subtlety'),(15,101,5,5,'Dwarven Racial'),(15,111,300,300,'Language: Dwarven'),(15,113,0,0,'Language: Darnassian'),(15,115,0,0,'Language: Taurahe'),(15,118,0,0,'Dual Wield'),(15,138,0,0,'Language: Draconic'),(15,141,0,0,'Language: Old Tongue'),(15,142,0,1,'Survival'),(15,253,5,5,'Assassination'),(15,313,0,0,'Language: Gnomish'),(15,414,1,1,'Leather'),(15,45,0,5,'Bows'),(15,46,0,5,'Guns'),(15,533,0,0,'Raptor Riding'),(15,553,0,0,'Mechanostrider Piloting'),(15,554,0,0,'Undead Horsemanship'),(15,202,0,0,'Engineering'),(15,356,0,0,'Fishing'),(15,171,0,0,'Alchemy'),(15,54,0,5,'Maces'),(15,182,0,0,'Herbalism'),(15,129,0,0,'First Aid'),(15,185,0,0,'Cooking'),(15,40,0,5,'Poisons'),(15,162,1,5,'Unarmed'),(16,148,0,0,'Horse Riding'),(16,150,0,0,'Tiger Riding'),(16,164,0,0,'Blacksmithing'),(16,173,0,5,'Daggers'),(16,183,5,5,'GENERIC (DND)'),(16,197,0,0,'Tailoring'),(16,228,1,5,'Wands'),(16,393,0,0,'Skinning'),(16,109,0,0,'Language: Orcish'),(16,137,0,0,'Language: Thalassian'),(16,140,0,0,'Language: Titan'),(16,149,0,0,'Wolf Riding'),(16,98,300,300,'Language: Common'),(16,315,0,0,'Language: Troll'),(16,139,0,0,'Language: Demon Tongue'),(16,152,0,0,'Ram Riding'),(16,165,0,0,'Leatherworking'),(16,415,1,1,'Cloth'),(16,673,0,0,'Language: Gutterspeak'),(16,95,1,5,'Defense'),(16,333,0,0,'Enchanting'),(16,186,0,0,'Mining'),(16,713,0,0,'Kodo Riding'),(16,56,5,5,'Holy'),(16,78,0,5,'Shadow Magic'),(16,101,5,5,'Dwarven Racial'),(16,111,300,300,'Language: Dwarven'),(16,113,0,0,'Language: Darnassian'),(16,115,0,0,'Language: Taurahe'),(16,136,0,5,'Staves'),(16,138,0,0,'Language: Draconic'),(16,141,0,0,'Language: Old Tongue'),(16,142,0,1,'Survival'),(16,313,0,0,'Language: Gnomish'),(16,613,0,5,'Discipline'),(16,533,0,0,'Raptor Riding'),(16,553,0,0,'Mechanostrider Piloting'),(16,554,0,0,'Undead Horsemanship'),(16,202,0,0,'Engineering'),(16,356,0,0,'Fishing'),(16,171,0,0,'Alchemy'),(16,54,1,5,'Maces'),(16,182,0,0,'Herbalism'),(16,129,0,0,'First Aid'),(16,185,0,0,'Cooking'),(16,162,1,5,'Unarmed'),(17,148,0,0,'Horse Riding'),(17,150,0,0,'Tiger Riding'),(17,164,0,0,'Blacksmithing'),(17,172,0,5,'Two-Handed Axes'),(17,173,1,5,'Daggers'),(17,183,5,5,'GENERIC (DND)'),(17,197,0,0,'Tailoring'),(17,226,0,5,'Crossbows'),(17,393,0,0,'Skinning'),(17,109,0,0,'Language: Orcish'),(17,433,1,1,'Shield'),(17,137,0,0,'Language: Thalassian'),(17,473,0,1,'Fist Weapons'),(17,140,0,0,'Language: Titan'),(17,149,0,0,'Wolf Riding'),(17,229,0,0,'Polearms'),(17,257,0,5,'Protection'),(17,293,0,0,'Plate Mail'),(17,98,300,300,'Language: Common'),(17,315,0,0,'Language: Troll'),(17,139,0,0,'Language: Demon Tongue'),(17,152,0,0,'Ram Riding'),(17,165,0,0,'Leatherworking'),(17,415,1,1,'Cloth'),(17,256,0,5,'Fury'),(17,673,0,0,'Language: Gutterspeak'),(17,95,1,5,'Defense'),(17,333,0,0,'Enchanting'),(17,186,0,0,'Mining'),(17,43,1,5,'Swords'),(17,55,0,5,'Two-Handed Swords'),(17,713,0,0,'Kodo Riding'),(17,176,0,5,'Thrown'),(17,26,5,5,'Arms'),(17,44,0,5,'Axes'),(17,111,0,0,'Language: Dwarven'),(17,113,300,300,'Language: Darnassian'),(17,115,0,0,'Language: Taurahe'),(17,118,0,0,'Dual Wield'),(17,136,0,5,'Staves'),(17,138,0,0,'Language: Draconic'),(17,141,0,0,'Language: Old Tongue'),(17,142,0,1,'Survival'),(17,160,0,5,'Two-Handed Maces'),(17,313,0,0,'Language: Gnomish'),(17,413,1,1,'Mail'),(17,414,1,1,'Leather'),(17,45,0,5,'Bows'),(17,46,0,5,'Guns'),(17,533,0,0,'Raptor Riding'),(17,554,0,0,'Undead Horsemanship'),(17,202,0,0,'Engineering'),(17,356,0,0,'Fishing'),(17,171,0,0,'Alchemy'),(17,54,1,5,'Maces'),(17,182,0,0,'Herbalism'),(17,129,0,0,'First Aid'),(17,185,0,0,'Cooking'),(17,126,5,5,'Night Elf Racial'),(17,162,1,5,'Unarmed'),(21,148,0,0,'Horse Riding'),(21,150,0,0,'Tiger Riding'),(21,164,0,0,'Blacksmithing'),(21,173,1,5,'Daggers'),(21,183,5,5,'GENERIC (DND)'),(21,197,0,0,'Tailoring'),(21,574,5,5,'Balance'),(21,393,0,0,'Skinning'),(21,109,0,0,'Language: Orcish'),(21,137,0,0,'Language: Thalassian'),(21,473,0,1,'Fist Weapons'),(21,140,0,0,'Language: Titan'),(21,149,0,0,'Wolf Riding'),(21,573,5,5,'Restoration'),(21,98,300,300,'Language: Common'),(21,315,0,0,'Language: Troll'),(21,139,0,0,'Language: Demon Tongue'),(21,152,0,0,'Ram Riding'),(21,165,0,0,'Leatherworking'),(21,134,0,0,'Feral Combat'),(21,415,1,1,'Cloth'),(21,673,0,0,'Language: Gutterspeak'),(21,95,1,5,'Defense'),(21,333,0,0,'Enchanting'),(21,186,0,0,'Mining'),(21,713,0,0,'Kodo Riding'),(21,111,0,0,'Language: Dwarven'),(21,113,300,300,'Language: Darnassian'),(21,115,0,0,'Language: Taurahe'),(21,136,1,5,'Staves'),(21,138,0,0,'Language: Draconic'),(21,141,0,0,'Language: Old Tongue'),(21,142,0,1,'Survival'),(21,160,0,5,'Two-Handed Maces'),(21,313,0,0,'Language: Gnomish'),(21,414,1,1,'Leather'),(21,533,0,0,'Raptor Riding'),(21,554,0,0,'Undead Horsemanship'),(21,202,0,0,'Engineering'),(21,356,0,0,'Fishing'),(21,171,0,0,'Alchemy'),(21,54,0,5,'Maces'),(21,182,0,0,'Herbalism'),(21,129,0,0,'First Aid'),(21,185,0,0,'Cooking'),(21,126,5,5,'Night Elf Racial'),(21,162,1,5,'Unarmed'),(18,148,0,0,'Horse Riding'),(18,150,0,0,'Tiger Riding'),(18,164,0,0,'Blacksmithing'),(18,172,0,5,'Two-Handed Axes'),(18,173,1,5,'Daggers'),(18,183,5,5,'GENERIC (DND)'),(18,197,0,0,'Tailoring'),(18,226,0,5,'Crossbows'),(18,393,0,0,'Skinning'),(18,109,0,0,'Language: Orcish'),(18,137,0,0,'Language: Thalassian'),(18,473,0,1,'Fist Weapons'),(18,140,0,0,'Language: Titan'),(18,149,0,0,'Wolf Riding'),(18,229,0,0,'Polearms'),(18,98,300,300,'Language: Common'),(18,315,0,0,'Language: Troll'),(18,139,0,0,'Language: Demon Tongue'),(18,152,0,0,'Ram Riding'),(18,165,0,0,'Leatherworking'),(18,415,1,1,'Cloth'),(18,673,0,0,'Language: Gutterspeak'),(18,95,1,5,'Defense'),(18,333,0,0,'Enchanting'),(18,186,0,0,'Mining'),(18,43,0,5,'Swords'),(18,55,0,5,'Two-Handed Swords'),(18,713,0,0,'Kodo Riding'),(18,176,0,5,'Thrown'),(18,44,0,5,'Axes'),(18,50,0,5,'Beast Mastery'),(18,51,5,5,'Survival'),(18,111,0,0,'Language: Dwarven'),(18,113,300,300,'Language: Darnassian'),(18,115,0,0,'Language: Taurahe'),(18,118,0,0,'Dual Wield'),(18,136,0,5,'Staves'),(18,138,0,0,'Language: Draconic'),(18,141,0,0,'Language: Old Tongue'),(18,142,0,1,'Survival'),(18,313,0,0,'Language: Gnomish'),(18,413,0,0,'Mail'),(18,414,1,1,'Leather'),(18,45,1,5,'Bows'),(18,163,5,5,'Marksmanship'),(18,46,0,5,'Guns'),(18,533,0,0,'Raptor Riding'),(18,554,0,0,'Undead Horsemanship'),(18,202,0,0,'Engineering'),(18,356,0,0,'Fishing'),(18,171,0,0,'Alchemy'),(18,182,0,0,'Herbalism'),(18,129,0,0,'First Aid'),(18,185,0,0,'Cooking'),(18,126,5,5,'Night Elf Racial'),(18,261,0,5,'Beast Training'),(18,162,1,5,'Unarmed'),(19,148,0,0,'Horse Riding'),(19,150,0,0,'Tiger Riding'),(19,164,0,0,'Blacksmithing'),(19,173,1,5,'Daggers'),(19,183,5,5,'GENERIC (DND)'),(19,197,0,0,'Tailoring'),(19,226,0,5,'Crossbows'),(19,393,0,0,'Skinning'),(19,109,0,0,'Language: Orcish'),(19,137,0,0,'Language: Thalassian'),(19,473,0,1,'Fist Weapons'),(19,140,0,0,'Language: Titan'),(19,149,0,0,'Wolf Riding'),(19,98,300,300,'Language: Common'),(19,315,0,0,'Language: Troll'),(19,139,0,0,'Language: Demon Tongue'),(19,152,0,0,'Ram Riding'),(19,165,0,0,'Leatherworking'),(19,415,1,1,'Cloth'),(19,633,0,5,'Lockpicking'),(19,673,0,0,'Language: Gutterspeak'),(19,95,1,5,'Defense'),(19,333,0,0,'Enchanting'),(19,186,0,0,'Mining'),(19,43,0,5,'Swords'),(19,713,0,0,'Kodo Riding'),(19,176,1,5,'Thrown'),(19,38,5,5,'Combat'),(19,39,0,5,'Subtlety'),(19,111,0,0,'Language: Dwarven'),(19,113,300,300,'Language: Darnassian'),(19,115,0,0,'Language: Taurahe'),(19,118,0,0,'Dual Wield'),(19,138,0,0,'Language: Draconic'),(19,141,0,0,'Language: Old Tongue'),(19,142,0,1,'Survival'),(19,253,5,5,'Assassination'),(19,313,0,0,'Language: Gnomish'),(19,414,1,1,'Leather'),(19,45,0,5,'Bows'),(19,46,0,5,'Guns'),(19,533,0,0,'Raptor Riding'),(19,554,0,0,'Undead Horsemanship'),(19,202,0,0,'Engineering'),(19,356,0,0,'Fishing'),(19,171,0,0,'Alchemy'),(19,54,0,5,'Maces'),(19,182,0,0,'Herbalism'),(19,129,0,0,'First Aid'),(19,185,0,0,'Cooking'),(19,126,5,5,'Night Elf Racial'),(19,40,0,5,'Poisons'),(19,162,1,5,'Unarmed'),(20,148,0,0,'Horse Riding'),(20,150,0,0,'Tiger Riding'),(20,164,0,0,'Blacksmithing'),(20,173,0,5,'Daggers'),(20,183,5,5,'GENERIC (DND)'),(20,197,0,0,'Tailoring'),(20,228,1,5,'Wands'),(20,393,0,0,'Skinning'),(20,109,0,0,'Language: Orcish'),(20,137,0,0,'Language: Thalassian'),(20,140,0,0,'Language: Titan'),(20,149,0,0,'Wolf Riding'),(20,98,300,300,'Language: Common'),(20,315,0,0,'Language: Troll'),(20,139,0,0,'Language: Demon Tongue'),(20,152,0,0,'Ram Riding'),(20,165,0,0,'Leatherworking'),(20,415,1,1,'Cloth'),(20,673,0,0,'Language: Gutterspeak'),(20,95,1,5,'Defense'),(20,333,0,0,'Enchanting'),(20,186,0,0,'Mining'),(20,713,0,0,'Kodo Riding'),(20,56,5,5,'Holy'),(20,78,0,5,'Shadow Magic'),(20,111,0,0,'Language: Dwarven'),(20,113,300,300,'Language: Darnassian'),(20,115,0,0,'Language: Taurahe'),(20,136,0,5,'Staves'),(20,138,0,0,'Language: Draconic'),(20,141,0,0,'Language: Old Tongue'),(20,142,0,1,'Survival'),(20,313,0,0,'Language: Gnomish'),(20,613,0,5,'Discipline'),(20,533,0,0,'Raptor Riding'),(20,554,0,0,'Undead Horsemanship'),(20,202,0,0,'Engineering'),(20,356,0,0,'Fishing'),(20,171,0,0,'Alchemy'),(20,54,1,5,'Maces'),(20,182,0,0,'Herbalism'),(20,129,0,0,'First Aid'),(20,185,0,0,'Cooking'),(20,126,5,5,'Night Elf Racial'),(20,162,1,5,'Unarmed'),(22,148,0,0,'Horse Riding'),(22,150,0,0,'Tiger Riding'),(22,164,0,0,'Blacksmithing'),(22,172,0,5,'Two-Handed Axes'),(22,173,1,5,'Daggers'),(22,183,5,5,'GENERIC (DND)'),(22,197,0,0,'Tailoring'),(22,220,5,5,'Racial - Undead'),(22,226,0,5,'Crossbows'),(22,393,0,0,'Skinning'),(22,109,300,300,'Language: Orcish'),(22,433,1,1,'Shield'),(22,137,0,0,'Language: Thalassian'),(22,473,0,1,'Fist Weapons'),(22,140,0,0,'Language: Titan'),(22,149,0,0,'Wolf Riding'),(22,229,0,0,'Polearms'),(22,257,0,5,'Protection'),(22,293,0,0,'Plate Mail'),(22,315,0,0,'Language: Troll'),(22,139,0,0,'Language: Demon Tongue'),(22,152,0,0,'Ram Riding'),(22,165,0,0,'Leatherworking'),(22,415,1,1,'Cloth'),(22,256,0,5,'Fury'),(22,673,300,300,'Language: Gutterspeak'),(22,95,1,5,'Defense'),(22,333,0,0,'Enchanting'),(22,186,0,0,'Mining'),(22,43,1,5,'Swords'),(22,55,1,5,'Two-Handed Swords'),(22,713,0,0,'Kodo Riding'),(22,176,0,5,'Thrown'),(22,26,5,5,'Arms'),(22,44,0,5,'Axes'),(22,111,0,0,'Language: Dwarven'),(22,113,0,0,'Language: Darnassian'),(22,115,0,0,'Language: Taurahe'),(22,118,0,0,'Dual Wield'),(22,136,0,5,'Staves'),(22,138,0,0,'Language: Draconic'),(22,141,0,0,'Language: Old Tongue'),(22,142,0,1,'Survival'),(22,160,0,5,'Two-Handed Maces'),(22,313,0,0,'Language: Gnomish'),(22,413,1,1,'Mail'),(22,414,1,1,'Leather'),(22,45,0,5,'Bows'),(22,46,0,5,'Guns'),(22,533,0,0,'Raptor Riding'),(22,554,0,0,'Undead Horsemanship'),(22,202,0,0,'Engineering'),(22,356,0,0,'Fishing'),(22,171,0,0,'Alchemy'),(22,54,0,5,'Maces'),(22,182,0,0,'Herbalism'),(22,129,0,0,'First Aid'),(22,185,0,0,'Cooking'),(22,162,1,5,'Unarmed'),(23,148,0,0,'Horse Riding'),(23,150,0,0,'Tiger Riding'),(23,164,0,0,'Blacksmithing'),(23,173,1,5,'Daggers'),(23,183,5,5,'GENERIC (DND)'),(23,197,0,0,'Tailoring'),(23,220,5,5,'Racial - Undead'),(23,226,0,5,'Crossbows'),(23,393,0,0,'Skinning'),(23,109,300,300,'Language: Orcish'),(23,137,0,0,'Language: Thalassian'),(23,473,0,1,'Fist Weapons'),(23,140,0,0,'Language: Titan'),(23,149,0,0,'Wolf Riding'),(23,315,0,0,'Language: Troll'),(23,139,0,0,'Language: Demon Tongue'),(23,152,0,0,'Ram Riding'),(23,165,0,0,'Leatherworking'),(23,415,1,1,'Cloth'),(23,633,0,5,'Lockpicking'),(23,673,300,300,'Language: Gutterspeak'),(23,95,1,5,'Defense'),(23,333,0,0,'Enchanting'),(23,186,0,0,'Mining'),(23,43,0,5,'Swords'),(23,713,0,0,'Kodo Riding'),(23,176,1,5,'Thrown'),(23,38,5,5,'Combat'),(23,39,0,5,'Subtlety'),(23,111,0,0,'Language: Dwarven'),(23,113,0,0,'Language: Darnassian'),(23,115,0,0,'Language: Taurahe'),(23,118,0,0,'Dual Wield'),(23,138,0,0,'Language: Draconic'),(23,141,0,0,'Language: Old Tongue'),(23,142,0,1,'Survival'),(23,253,5,5,'Assassination'),(23,313,0,0,'Language: Gnomish'),(23,414,1,1,'Leather'),(23,45,0,5,'Bows'),(23,46,0,5,'Guns'),(23,533,0,0,'Raptor Riding'),(23,554,0,0,'Undead Horsemanship'),(23,202,0,0,'Engineering'),(23,356,0,0,'Fishing'),(23,171,0,0,'Alchemy'),(23,54,0,5,'Maces'),(23,182,0,0,'Herbalism'),(23,129,0,0,'First Aid'),(23,185,0,0,'Cooking'),(23,40,0,5,'Poisons'),(23,162,1,5,'Unarmed'),(24,148,0,0,'Horse Riding'),(24,150,0,0,'Tiger Riding'),(24,164,0,0,'Blacksmithing'),(24,173,0,5,'Daggers'),(24,183,5,5,'GENERIC (DND)'),(24,197,0,0,'Tailoring'),(24,220,5,5,'Racial - Undead'),(24,228,1,5,'Wands'),(24,393,0,0,'Skinning'),(24,109,300,300,'Language: Orcish'),(24,137,0,0,'Language: Thalassian'),(24,140,0,0,'Language: Titan'),(24,149,0,0,'Wolf Riding'),(24,315,0,0,'Language: Troll'),(24,139,0,0,'Language: Demon Tongue'),(24,152,0,0,'Ram Riding'),(24,165,0,0,'Leatherworking'),(24,415,1,1,'Cloth'),(24,673,300,300,'Language: Gutterspeak'),(24,95,1,5,'Defense'),(24,333,0,0,'Enchanting'),(24,186,0,0,'Mining'),(24,713,0,0,'Kodo Riding'),(24,56,5,5,'Holy'),(24,78,0,5,'Shadow Magic'),(24,111,0,0,'Language: Dwarven'),(24,113,0,0,'Language: Darnassian'),(24,115,0,0,'Language: Taurahe'),(24,136,0,5,'Staves'),(24,138,0,0,'Language: Draconic'),(24,141,0,0,'Language: Old Tongue'),(24,142,0,1,'Survival'),(24,313,0,0,'Language: Gnomish'),(24,613,0,5,'Discipline'),(24,533,0,0,'Raptor Riding'),(24,554,0,0,'Undead Horsemanship'),(24,202,0,0,'Engineering'),(24,356,0,0,'Fishing'),(24,171,0,0,'Alchemy'),(24,54,1,5,'Maces'),(24,182,0,0,'Herbalism'),(24,129,0,0,'First Aid'),(24,185,0,0,'Cooking'),(24,162,1,5,'Unarmed'),(25,148,0,0,'Horse Riding'),(25,150,0,0,'Tiger Riding'),(25,164,0,0,'Blacksmithing'),(25,173,0,5,'Daggers'),(25,183,5,5,'GENERIC (DND)'),(25,197,0,0,'Tailoring'),(25,220,5,5,'Racial - Undead'),(25,228,1,5,'Wands'),(25,393,0,0,'Skinning'),(25,109,300,300,'Language: Orcish'),(25,137,0,0,'Language: Thalassian'),(25,140,0,0,'Language: Titan'),(25,149,0,0,'Wolf Riding'),(25,8,5,5,'Fire'),(25,315,0,0,'Language: Troll'),(25,139,0,0,'Language: Demon Tongue'),(25,152,0,0,'Ram Riding'),(25,165,0,0,'Leatherworking'),(25,415,1,1,'Cloth'),(25,673,300,300,'Language: Gutterspeak'),(25,95,1,5,'Defense'),(25,333,0,0,'Enchanting'),(25,186,0,0,'Mining'),(25,43,0,5,'Swords'),(25,713,0,0,'Kodo Riding'),(25,6,5,5,'Frost'),(25,111,0,0,'Language: Dwarven'),(25,113,0,0,'Language: Darnassian'),(25,115,0,0,'Language: Taurahe'),(25,136,1,5,'Staves'),(25,138,0,0,'Language: Draconic'),(25,141,0,0,'Language: Old Tongue'),(25,142,0,1,'Survival'),(25,313,0,0,'Language: Gnomish'),(25,533,0,0,'Raptor Riding'),(25,554,0,0,'Undead Horsemanship'),(25,202,0,0,'Engineering'),(25,356,0,0,'Fishing'),(25,171,0,0,'Alchemy'),(25,182,0,0,'Herbalism'),(25,129,0,0,'First Aid'),(25,185,0,0,'Cooking'),(25,237,0,5,'Arcane'),(25,162,1,5,'Unarmed'),(26,148,0,0,'Horse Riding'),(26,150,0,0,'Tiger Riding'),(26,164,0,0,'Blacksmithing'),(26,173,1,5,'Daggers'),(26,183,5,5,'GENERIC (DND)'),(26,197,0,0,'Tailoring'),(26,220,5,5,'Racial - Undead'),(26,228,1,5,'Wands'),(26,393,0,0,'Skinning'),(26,109,300,300,'Language: Orcish'),(26,137,0,0,'Language: Thalassian'),(26,140,0,0,'Language: Titan'),(26,149,0,0,'Wolf Riding'),(26,593,5,5,'Destruction'),(26,315,0,0,'Language: Troll'),(26,139,0,0,'Language: Demon Tongue'),(26,152,0,0,'Ram Riding'),(26,354,5,5,'Demonology'),(26,355,0,5,'Affliction'),(26,165,0,0,'Leatherworking'),(26,415,1,1,'Cloth'),(26,673,300,300,'Language: Gutterspeak'),(26,95,1,5,'Defense'),(26,333,0,0,'Enchanting'),(26,186,0,0,'Mining'),(26,43,0,5,'Swords'),(26,713,0,0,'Kodo Riding'),(26,111,0,0,'Language: Dwarven'),(26,113,0,0,'Language: Darnassian'),(26,115,0,0,'Language: Taurahe'),(26,136,0,5,'Staves'),(26,138,0,0,'Language: Draconic'),(26,141,0,0,'Language: Old Tongue'),(26,142,0,1,'Survival'),(26,313,0,0,'Language: Gnomish'),(26,533,0,0,'Raptor Riding'),(26,554,0,0,'Undead Horsemanship'),(26,202,0,0,'Engineering'),(26,356,0,0,'Fishing'),(26,171,0,0,'Alchemy'),(26,182,0,0,'Herbalism'),(26,129,0,0,'First Aid'),(26,185,0,0,'Cooking'),(26,162,1,5,'Unarmed'),(27,164,0,0,'Blacksmithing'),(27,172,0,5,'Two-Handed Axes'),(27,173,0,5,'Daggers'),(27,183,5,5,'GENERIC (DND)'),(27,197,0,0,'Tailoring'),(27,226,0,5,'Crossbows'),(27,393,0,0,'Skinning'),(27,109,300,300,'Language: Orcish'),(27,124,5,5,'Tauren Racial'),(27,433,1,1,'Shield'),(27,137,0,0,'Language: Thalassian'),(27,473,0,1,'Fist Weapons'),(27,140,0,0,'Language: Titan'),(27,149,0,0,'Wolf Riding'),(27,229,0,0,'Polearms'),(27,257,0,5,'Protection'),(27,293,0,0,'Plate Mail'),(27,98,0,0,'Language: Common'),(27,315,0,0,'Language: Troll'),(27,139,0,0,'Language: Demon Tongue'),(27,165,0,0,'Leatherworking'),(27,415,1,1,'Cloth'),(27,256,0,5,'Fury'),(27,673,0,0,'Language: Gutterspeak'),(27,95,1,5,'Defense'),(27,333,0,0,'Enchanting'),(27,186,0,0,'Mining'),(27,43,0,5,'Swords'),(27,55,0,5,'Two-Handed Swords'),(27,713,0,0,'Kodo Riding'),(27,176,0,5,'Thrown'),(27,26,5,5,'Arms'),(27,44,1,5,'Axes'),(27,111,0,0,'Language: Dwarven'),(27,113,0,0,'Language: Darnassian'),(27,115,300,300,'Language: Taurahe'),(27,118,0,0,'Dual Wield'),(27,136,0,5,'Staves'),(27,138,0,0,'Language: Draconic'),(27,141,0,0,'Language: Old Tongue'),(27,142,0,1,'Survival'),(27,160,1,5,'Two-Handed Maces'),(27,313,0,0,'Language: Gnomish'),(27,413,1,1,'Mail'),(27,414,1,1,'Leather'),(27,45,0,5,'Bows'),(27,46,0,5,'Guns'),(27,202,0,0,'Engineering'),(27,356,0,0,'Fishing'),(27,171,0,0,'Alchemy'),(27,54,1,5,'Maces'),(27,182,0,0,'Herbalism'),(27,129,0,0,'First Aid'),(27,185,0,0,'Cooking'),(27,162,1,5,'Unarmed'),(30,164,0,0,'Blacksmithing'),(30,173,0,5,'Daggers'),(30,183,5,5,'GENERIC (DND)'),(30,197,0,0,'Tailoring'),(30,574,5,5,'Balance'),(30,393,0,0,'Skinning'),(30,109,300,300,'Language: Orcish'),(30,124,5,5,'Tauren Racial'),(30,137,0,0,'Language: Thalassian'),(30,473,0,1,'Fist Weapons'),(30,140,0,0,'Language: Titan'),(30,149,0,0,'Wolf Riding'),(30,573,5,5,'Restoration'),(30,98,0,0,'Language: Common'),(30,315,0,0,'Language: Troll'),(30,139,0,0,'Language: Demon Tongue'),(30,165,0,0,'Leatherworking'),(30,134,0,0,'Feral Combat'),(30,415,1,1,'Cloth'),(30,673,0,0,'Language: Gutterspeak'),(30,95,1,5,'Defense'),(30,333,0,0,'Enchanting'),(30,186,0,0,'Mining'),(30,713,0,0,'Kodo Riding'),(30,111,0,0,'Language: Dwarven'),(30,113,0,0,'Language: Darnassian'),(30,115,300,300,'Language: Taurahe'),(30,136,1,5,'Staves'),(30,138,0,0,'Language: Draconic'),(30,141,0,0,'Language: Old Tongue'),(30,142,0,1,'Survival'),(30,160,0,5,'Two-Handed Maces'),(30,313,0,0,'Language: Gnomish'),(30,414,1,1,'Leather'),(30,202,0,0,'Engineering'),(30,356,0,0,'Fishing'),(30,171,0,0,'Alchemy'),(30,54,1,5,'Maces'),(30,182,0,0,'Herbalism'),(30,129,0,0,'First Aid'),(30,185,0,0,'Cooking'),(30,162,1,5,'Unarmed'),(28,164,0,0,'Blacksmithing'),(28,172,0,5,'Two-Handed Axes'),(28,173,0,5,'Daggers'),(28,183,5,5,'GENERIC (DND)'),(28,197,0,0,'Tailoring'),(28,226,0,5,'Crossbows'),(28,393,0,0,'Skinning'),(28,109,300,300,'Language: Orcish'),(28,124,5,5,'Tauren Racial'),(28,137,0,0,'Language: Thalassian'),(28,473,0,1,'Fist Weapons'),(28,140,0,0,'Language: Titan'),(28,149,0,0,'Wolf Riding'),(28,229,0,0,'Polearms'),(28,98,0,0,'Language: Common'),(28,315,0,0,'Language: Troll'),(28,139,0,0,'Language: Demon Tongue'),(28,165,0,0,'Leatherworking'),(28,415,1,1,'Cloth'),(28,673,0,0,'Language: Gutterspeak'),(28,95,1,5,'Defense'),(28,333,0,0,'Enchanting'),(28,186,0,0,'Mining'),(28,43,0,5,'Swords'),(28,55,0,5,'Two-Handed Swords'),(28,713,0,0,'Kodo Riding'),(28,176,0,5,'Thrown'),(28,44,1,5,'Axes'),(28,50,0,5,'Beast Mastery'),(28,51,5,5,'Survival'),(28,111,0,0,'Language: Dwarven'),(28,113,0,0,'Language: Darnassian'),(28,115,300,300,'Language: Taurahe'),(28,118,0,0,'Dual Wield'),(28,136,0,5,'Staves'),(28,138,0,0,'Language: Draconic'),(28,141,0,0,'Language: Old Tongue'),(28,142,0,1,'Survival'),(28,313,0,0,'Language: Gnomish'),(28,413,0,0,'Mail'),(28,414,1,1,'Leather'),(28,45,0,5,'Bows'),(28,163,5,5,'Marksmanship'),(28,46,1,5,'Guns'),(28,202,0,0,'Engineering'),(28,356,0,0,'Fishing'),(28,171,0,0,'Alchemy'),(28,182,0,0,'Herbalism'),(28,129,0,0,'First Aid'),(28,185,0,0,'Cooking'),(28,261,0,5,'Beast Training'),(28,162,1,5,'Unarmed'),(29,164,0,0,'Blacksmithing'),(29,172,0,5,'Two-Handed Axes'),(29,173,0,5,'Daggers'),(29,183,5,5,'GENERIC (DND)'),(29,197,0,0,'Tailoring'),(29,393,0,0,'Skinning'),(29,109,300,300,'Language: Orcish'),(29,124,5,5,'Tauren Racial'),(29,433,1,1,'Shield'),(29,137,0,0,'Language: Thalassian'),(29,473,0,1,'Fist Weapons'),(29,140,0,0,'Language: Titan'),(29,149,0,0,'Wolf Riding'),(29,98,0,0,'Language: Common'),(29,315,0,0,'Language: Troll'),(29,139,0,0,'Language: Demon Tongue'),(29,165,0,0,'Leatherworking'),(29,373,0,5,'Enhancement'),(29,374,5,5,'Restoration'),(29,375,5,5,'Elemental Combat'),(29,415,1,1,'Cloth'),(29,673,0,0,'Language: Gutterspeak'),(29,95,1,5,'Defense'),(29,333,0,0,'Enchanting'),(29,186,0,0,'Mining'),(29,713,0,0,'Kodo Riding'),(29,44,0,5,'Axes'),(29,111,0,0,'Language: Dwarven'),(29,113,0,0,'Language: Darnassian'),(29,115,300,300,'Language: Taurahe'),(29,136,1,5,'Staves'),(29,138,0,0,'Language: Draconic'),(29,141,0,0,'Language: Old Tongue'),(29,142,0,1,'Survival'),(29,160,0,5,'Two-Handed Maces'),(29,313,0,0,'Language: Gnomish'),(29,413,0,0,'Mail'),(29,414,1,1,'Leather'),(29,202,0,0,'Engineering'),(29,356,0,0,'Fishing'),(29,171,0,0,'Alchemy'),(29,54,1,5,'Maces'),(29,182,0,0,'Herbalism'),(29,129,0,0,'First Aid'),(29,185,0,0,'Cooking'),(29,162,1,5,'Unarmed'),(31,148,0,0,'Horse Riding'),(31,150,0,0,'Tiger Riding'),(31,164,0,0,'Blacksmithing'),(31,172,0,5,'Two-Handed Axes'),(31,173,1,5,'Daggers'),(31,183,5,5,'GENERIC (DND)'),(31,197,0,0,'Tailoring'),(31,226,0,5,'Crossbows'),(31,393,0,0,'Skinning'),(31,109,0,0,'Language: Orcish'),(31,433,1,1,'Shield'),(31,137,0,0,'Language: Thalassian'),(31,473,0,1,'Fist Weapons'),(31,140,0,0,'Language: Titan'),(31,149,0,0,'Wolf Riding'),(31,229,0,0,'Polearms'),(31,257,0,5,'Protection'),(31,293,0,0,'Plate Mail'),(31,98,300,300,'Language: Common'),(31,315,0,0,'Language: Troll'),(31,139,0,0,'Language: Demon Tongue'),(31,152,0,0,'Ram Riding'),(31,165,0,0,'Leatherworking'),(31,415,1,1,'Cloth'),(31,256,0,5,'Fury'),(31,673,0,0,'Language: Gutterspeak'),(31,95,1,5,'Defense'),(31,333,0,0,'Enchanting'),(31,186,0,0,'Mining'),(31,43,1,5,'Swords'),(31,55,0,5,'Two-Handed Swords'),(31,713,0,0,'Kodo Riding'),(31,753,5,5,'Racial - Gnome'),(31,176,0,5,'Thrown'),(31,26,5,5,'Arms'),(31,44,0,5,'Axes'),(31,111,0,0,'Language: Dwarven'),(31,113,0,0,'Language: Darnassian'),(31,115,0,0,'Language: Taurahe'),(31,118,0,0,'Dual Wield'),(31,136,0,5,'Staves'),(31,138,0,0,'Language: Draconic'),(31,141,0,0,'Language: Old Tongue'),(31,142,0,1,'Survival'),(31,160,0,5,'Two-Handed Maces'),(31,313,300,300,'Language: Gnomish'),(31,413,1,1,'Mail'),(31,414,1,1,'Leather'),(31,45,0,5,'Bows'),(31,46,0,5,'Guns'),(31,533,0,0,'Raptor Riding'),(31,553,0,0,'Mechanostrider Piloting'),(31,554,0,0,'Undead Horsemanship'),(31,202,0,0,'Engineering'),(31,356,0,0,'Fishing'),(31,171,0,0,'Alchemy'),(31,54,1,5,'Maces'),(31,182,0,0,'Herbalism'),(31,129,0,0,'First Aid'),(31,185,0,0,'Cooking'),(31,162,1,5,'Unarmed'),(32,148,0,0,'Horse Riding'),(32,150,0,0,'Tiger Riding'),(32,164,0,0,'Blacksmithing'),(32,173,1,5,'Daggers'),(32,183,5,5,'GENERIC (DND)'),(32,197,0,0,'Tailoring'),(32,226,0,5,'Crossbows'),(32,393,0,0,'Skinning'),(32,109,0,0,'Language: Orcish'),(32,137,0,0,'Language: Thalassian'),(32,473,0,1,'Fist Weapons'),(32,140,0,0,'Language: Titan'),(32,149,0,0,'Wolf Riding'),(32,98,300,300,'Language: Common'),(32,315,0,0,'Language: Troll'),(32,139,0,0,'Language: Demon Tongue'),(32,152,0,0,'Ram Riding'),(32,165,0,0,'Leatherworking'),(32,415,1,1,'Cloth'),(32,633,0,5,'Lockpicking'),(32,673,0,0,'Language: Gutterspeak'),(32,95,1,5,'Defense'),(32,333,0,0,'Enchanting'),(32,186,0,0,'Mining'),(32,43,0,5,'Swords'),(32,713,0,0,'Kodo Riding'),(32,753,5,5,'Racial - Gnome'),(32,176,1,5,'Thrown'),(32,38,5,5,'Combat'),(32,39,0,5,'Subtlety'),(32,111,0,0,'Language: Dwarven'),(32,113,0,0,'Language: Darnassian'),(32,115,0,0,'Language: Taurahe'),(32,118,0,0,'Dual Wield'),(32,138,0,0,'Language: Draconic'),(32,141,0,0,'Language: Old Tongue'),(32,142,0,1,'Survival'),(32,253,5,5,'Assassination'),(32,313,300,300,'Language: Gnomish'),(32,414,1,1,'Leather'),(32,45,0,5,'Bows'),(32,46,0,5,'Guns'),(32,533,0,0,'Raptor Riding'),(32,553,0,0,'Mechanostrider Piloting'),(32,554,0,0,'Undead Horsemanship'),(32,202,0,0,'Engineering'),(32,356,0,0,'Fishing'),(32,171,0,0,'Alchemy'),(32,54,0,5,'Maces'),(32,182,0,0,'Herbalism'),(32,129,0,0,'First Aid'),(32,185,0,0,'Cooking'),(32,40,0,5,'Poisons'),(32,162,1,5,'Unarmed'),(33,148,0,0,'Horse Riding'),(33,150,0,0,'Tiger Riding'),(33,164,0,0,'Blacksmithing'),(33,173,0,5,'Daggers'),(33,183,5,5,'GENERIC (DND)'),(33,197,0,0,'Tailoring'),(33,228,1,5,'Wands'),(33,393,0,0,'Skinning'),(33,109,0,0,'Language: Orcish'),(33,137,0,0,'Language: Thalassian'),(33,140,0,0,'Language: Titan'),(33,149,0,0,'Wolf Riding'),(33,8,5,5,'Fire'),(33,98,300,300,'Language: Common'),(33,315,0,0,'Language: Troll'),(33,139,0,0,'Language: Demon Tongue'),(33,152,0,0,'Ram Riding'),(33,165,0,0,'Leatherworking'),(33,415,1,1,'Cloth'),(33,673,0,0,'Language: Gutterspeak'),(33,95,1,5,'Defense'),(33,333,0,0,'Enchanting'),(33,186,0,0,'Mining'),(33,43,0,5,'Swords'),(33,713,0,0,'Kodo Riding'),(33,753,5,5,'Racial - Gnome'),(33,6,5,5,'Frost'),(33,111,0,0,'Language: Dwarven'),(33,113,0,0,'Language: Darnassian'),(33,115,0,0,'Language: Taurahe'),(33,136,1,5,'Staves'),(33,138,0,0,'Language: Draconic'),(33,141,0,0,'Language: Old Tongue'),(33,142,0,1,'Survival'),(33,313,300,300,'Language: Gnomish'),(33,533,0,0,'Raptor Riding'),(33,553,0,0,'Mechanostrider Piloting'),(33,554,0,0,'Undead Horsemanship'),(33,202,0,0,'Engineering'),(33,356,0,0,'Fishing'),(33,171,0,0,'Alchemy'),(33,182,0,0,'Herbalism'),(33,129,0,0,'First Aid'),(33,185,0,0,'Cooking'),(33,237,0,5,'Arcane'),(33,162,1,5,'Unarmed'),(34,148,0,0,'Horse Riding'),(34,150,0,0,'Tiger Riding'),(34,164,0,0,'Blacksmithing'),(34,173,1,5,'Daggers'),(34,183,5,5,'GENERIC (DND)'),(34,197,0,0,'Tailoring'),(34,228,1,5,'Wands'),(34,393,0,0,'Skinning'),(34,109,0,0,'Language: Orcish'),(34,137,0,0,'Language: Thalassian'),(34,140,0,0,'Language: Titan'),(34,149,0,0,'Wolf Riding'),(34,593,5,5,'Destruction'),(34,98,300,300,'Language: Common'),(34,315,0,0,'Language: Troll'),(34,139,0,0,'Language: Demon Tongue'),(34,152,0,0,'Ram Riding'),(34,354,5,5,'Demonology'),(34,355,0,5,'Affliction'),(34,165,0,0,'Leatherworking'),(34,415,1,1,'Cloth'),(34,673,0,0,'Language: Gutterspeak'),(34,95,1,5,'Defense'),(34,333,0,0,'Enchanting'),(34,186,0,0,'Mining'),(34,43,0,5,'Swords'),(34,713,0,0,'Kodo Riding'),(34,753,5,5,'Racial - Gnome'),(34,111,0,0,'Language: Dwarven'),(34,113,0,0,'Language: Darnassian'),(34,115,0,0,'Language: Taurahe'),(34,136,0,5,'Staves'),(34,138,0,0,'Language: Draconic'),(34,141,0,0,'Language: Old Tongue'),(34,142,0,1,'Survival'),(34,313,300,300,'Language: Gnomish'),(34,533,0,0,'Raptor Riding'),(34,553,0,0,'Mechanostrider Piloting'),(34,554,0,0,'Undead Horsemanship'),(34,202,0,0,'Engineering'),(34,356,0,0,'Fishing'),(34,171,0,0,'Alchemy'),(34,182,0,0,'Herbalism'),(34,129,0,0,'First Aid'),(34,185,0,0,'Cooking'),(34,162,1,5,'Unarmed'),(35,148,0,0,'Horse Riding'),(35,150,0,0,'Tiger Riding'),(35,164,0,0,'Blacksmithing'),(35,172,0,5,'Two-Handed Axes'),(35,173,1,5,'Daggers'),(35,183,5,5,'GENERIC (DND)'),(35,197,0,0,'Tailoring'),(35,226,0,5,'Crossbows'),(35,393,0,0,'Skinning'),(35,109,300,300,'Language: Orcish'),(35,433,1,1,'Shield'),(35,137,0,0,'Language: Thalassian'),(35,473,0,1,'Fist Weapons'),(35,140,0,0,'Language: Titan'),(35,149,0,0,'Wolf Riding'),(35,229,0,0,'Polearms'),(35,257,0,5,'Protection'),(35,293,0,0,'Plate Mail'),(35,98,0,0,'Language: Common'),(35,315,300,300,'Language: Troll'),(35,139,0,0,'Language: Demon Tongue'),(35,152,0,0,'Ram Riding'),(35,165,0,0,'Leatherworking'),(35,415,1,1,'Cloth'),(35,256,0,5,'Fury'),(35,673,0,0,'Language: Gutterspeak'),(35,95,1,5,'Defense'),(35,333,0,0,'Enchanting'),(35,186,0,0,'Mining'),(35,43,0,5,'Swords'),(35,55,0,5,'Two-Handed Swords'),(35,713,0,0,'Kodo Riding'),(35,733,5,5,'Racial - Troll'),(35,176,1,5,'Thrown'),(35,26,5,5,'Arms'),(35,44,1,5,'Axes'),(35,111,0,0,'Language: Dwarven'),(35,113,0,0,'Language: Darnassian'),(35,115,0,0,'Language: Taurahe'),(35,118,0,0,'Dual Wield'),(35,136,0,5,'Staves'),(35,138,0,0,'Language: Draconic'),(35,141,0,0,'Language: Old Tongue'),(35,142,0,1,'Survival'),(35,160,0,5,'Two-Handed Maces'),(35,313,0,0,'Language: Gnomish'),(35,413,1,1,'Mail'),(35,414,1,1,'Leather'),(35,45,0,5,'Bows'),(35,46,0,5,'Guns'),(35,533,0,0,'Raptor Riding'),(35,554,0,0,'Undead Horsemanship'),(35,202,0,0,'Engineering'),(35,356,0,0,'Fishing'),(35,171,0,0,'Alchemy'),(35,54,0,5,'Maces'),(35,182,0,0,'Herbalism'),(35,129,0,0,'First Aid'),(35,185,0,0,'Cooking'),(35,162,1,5,'Unarmed'),(36,148,0,0,'Horse Riding'),(36,150,0,0,'Tiger Riding'),(36,164,0,0,'Blacksmithing'),(36,172,0,5,'Two-Handed Axes'),(36,173,0,5,'Daggers'),(36,183,5,5,'GENERIC (DND)'),(36,197,0,0,'Tailoring'),(36,226,0,5,'Crossbows'),(36,393,0,0,'Skinning'),(36,109,300,300,'Language: Orcish'),(36,137,0,0,'Language: Thalassian'),(36,473,0,1,'Fist Weapons'),(36,140,0,0,'Language: Titan'),(36,149,0,0,'Wolf Riding'),(36,229,0,0,'Polearms'),(36,98,0,0,'Language: Common'),(36,315,300,300,'Language: Troll'),(36,139,0,0,'Language: Demon Tongue'),(36,152,0,0,'Ram Riding'),(36,165,0,0,'Leatherworking'),(36,415,1,1,'Cloth'),(36,673,0,0,'Language: Gutterspeak'),(36,95,1,5,'Defense'),(36,333,0,0,'Enchanting'),(36,186,0,0,'Mining'),(36,43,0,5,'Swords'),(36,55,0,5,'Two-Handed Swords'),(36,713,0,0,'Kodo Riding'),(36,733,5,5,'Racial - Troll'),(36,176,0,5,'Thrown'),(36,44,1,5,'Axes'),(36,50,0,5,'Beast Mastery'),(36,51,5,5,'Survival'),(36,111,0,0,'Language: Dwarven'),(36,113,0,0,'Language: Darnassian'),(36,115,0,0,'Language: Taurahe'),(36,118,0,0,'Dual Wield'),(36,136,0,5,'Staves'),(36,138,0,0,'Language: Draconic'),(36,141,0,0,'Language: Old Tongue'),(36,142,0,1,'Survival'),(36,313,0,0,'Language: Gnomish'),(36,413,0,0,'Mail'),(36,414,1,1,'Leather'),(36,45,1,5,'Bows'),(36,163,5,5,'Marksmanship'),(36,46,0,5,'Guns'),(36,533,0,0,'Raptor Riding'),(36,554,0,0,'Undead Horsemanship'),(36,202,0,0,'Engineering'),(36,356,0,0,'Fishing'),(36,171,0,0,'Alchemy'),(36,182,0,0,'Herbalism'),(36,129,0,0,'First Aid'),(36,185,0,0,'Cooking'),(36,261,0,5,'Beast Training'),(36,162,1,5,'Unarmed'),(37,148,0,0,'Horse Riding'),(37,150,0,0,'Tiger Riding'),(37,164,0,0,'Blacksmithing'),(37,173,1,5,'Daggers'),(37,183,5,5,'GENERIC (DND)'),(37,197,0,0,'Tailoring'),(37,226,0,5,'Crossbows'),(37,393,0,0,'Skinning'),(37,109,300,300,'Language: Orcish'),(37,137,0,0,'Language: Thalassian'),(37,473,0,1,'Fist Weapons'),(37,140,0,0,'Language: Titan'),(37,149,0,0,'Wolf Riding'),(37,98,0,0,'Language: Common'),(37,315,300,300,'Language: Troll'),(37,139,0,0,'Language: Demon Tongue'),(37,152,0,0,'Ram Riding'),(37,165,0,0,'Leatherworking'),(37,415,1,1,'Cloth'),(37,633,0,5,'Lockpicking'),(37,673,0,0,'Language: Gutterspeak'),(37,95,1,5,'Defense'),(37,333,0,0,'Enchanting'),(37,186,0,0,'Mining'),(37,43,0,5,'Swords'),(37,713,0,0,'Kodo Riding'),(37,733,5,5,'Racial - Troll'),(37,176,1,5,'Thrown'),(37,38,5,5,'Combat'),(37,39,0,5,'Subtlety'),(37,111,0,0,'Language: Dwarven'),(37,113,0,0,'Language: Darnassian'),(37,115,0,0,'Language: Taurahe'),(37,118,0,0,'Dual Wield'),(37,138,0,0,'Language: Draconic'),(37,141,0,0,'Language: Old Tongue'),(37,142,0,1,'Survival'),(37,253,5,5,'Assassination'),(37,313,0,0,'Language: Gnomish'),(37,414,1,1,'Leather'),(37,45,0,5,'Bows'),(37,46,0,5,'Guns'),(37,533,0,0,'Raptor Riding'),(37,554,0,0,'Undead Horsemanship'),(37,202,0,0,'Engineering'),(37,356,0,0,'Fishing'),(37,171,0,0,'Alchemy'),(37,54,0,5,'Maces'),(37,182,0,0,'Herbalism'),(37,129,0,0,'First Aid'),(37,185,0,0,'Cooking'),(37,40,0,5,'Poisons'),(37,162,1,5,'Unarmed'),(38,148,0,0,'Horse Riding'),(38,150,0,0,'Tiger Riding'),(38,164,0,0,'Blacksmithing'),(38,173,0,5,'Daggers'),(38,183,5,5,'GENERIC (DND)'),(38,197,0,0,'Tailoring'),(38,228,1,5,'Wands'),(38,393,0,0,'Skinning'),(38,109,300,300,'Language: Orcish'),(38,137,0,0,'Language: Thalassian'),(38,140,0,0,'Language: Titan'),(38,149,0,0,'Wolf Riding'),(38,98,0,0,'Language: Common'),(38,315,300,300,'Language: Troll'),(38,139,0,0,'Language: Demon Tongue'),(38,152,0,0,'Ram Riding'),(38,165,0,0,'Leatherworking'),(38,415,1,1,'Cloth'),(38,673,0,0,'Language: Gutterspeak'),(38,95,1,5,'Defense'),(38,333,0,0,'Enchanting'),(38,186,0,0,'Mining'),(38,713,0,0,'Kodo Riding'),(38,733,5,5,'Racial - Troll'),(38,56,5,5,'Holy'),(38,78,0,5,'Shadow Magic'),(38,111,0,0,'Language: Dwarven'),(38,113,0,0,'Language: Darnassian'),(38,115,0,0,'Language: Taurahe'),(38,136,0,5,'Staves'),(38,138,0,0,'Language: Draconic'),(38,141,0,0,'Language: Old Tongue'),(38,142,0,1,'Survival'),(38,313,0,0,'Language: Gnomish'),(38,613,0,5,'Discipline'),(38,533,0,0,'Raptor Riding'),(38,554,0,0,'Undead Horsemanship'),(38,202,0,0,'Engineering'),(38,356,0,0,'Fishing'),(38,171,0,0,'Alchemy'),(38,54,1,5,'Maces'),(38,182,0,0,'Herbalism'),(38,129,0,0,'First Aid'),(38,185,0,0,'Cooking'),(38,162,1,5,'Unarmed'),(39,148,0,0,'Horse Riding'),(39,150,0,0,'Tiger Riding'),(39,164,0,0,'Blacksmithing'),(39,172,0,5,'Two-Handed Axes'),(39,173,0,5,'Daggers'),(39,183,5,5,'GENERIC (DND)'),(39,197,0,0,'Tailoring'),(39,393,0,0,'Skinning'),(39,109,300,300,'Language: Orcish'),(39,433,1,1,'Shield'),(39,137,0,0,'Language: Thalassian'),(39,473,0,1,'Fist Weapons'),(39,140,0,0,'Language: Titan'),(39,149,0,0,'Wolf Riding'),(39,98,0,0,'Language: Common'),(39,315,300,300,'Language: Troll'),(39,139,0,0,'Language: Demon Tongue'),(39,152,0,0,'Ram Riding'),(39,165,0,0,'Leatherworking'),(39,373,0,5,'Enhancement'),(39,374,5,5,'Restoration'),(39,375,5,5,'Elemental Combat'),(39,415,1,1,'Cloth'),(39,673,0,0,'Language: Gutterspeak'),(39,95,1,5,'Defense'),(39,333,0,0,'Enchanting'),(39,186,0,0,'Mining'),(39,713,0,0,'Kodo Riding'),(39,733,5,5,'Racial - Troll'),(39,44,0,5,'Axes'),(39,111,0,0,'Language: Dwarven'),(39,113,0,0,'Language: Darnassian'),(39,115,0,0,'Language: Taurahe'),(39,136,1,5,'Staves'),(39,138,0,0,'Language: Draconic'),(39,141,0,0,'Language: Old Tongue'),(39,142,0,1,'Survival'),(39,160,0,5,'Two-Handed Maces'),(39,313,0,0,'Language: Gnomish'),(39,413,0,0,'Mail'),(39,414,1,1,'Leather'),(39,533,0,0,'Raptor Riding'),(39,554,0,0,'Undead Horsemanship'),(39,202,0,0,'Engineering'),(39,356,0,0,'Fishing'),(39,171,0,0,'Alchemy'),(39,54,1,5,'Maces'),(39,182,0,0,'Herbalism'),(39,129,0,0,'First Aid'),(39,185,0,0,'Cooking'),(39,162,1,5,'Unarmed'),(40,148,0,0,'Horse Riding'),(40,150,0,0,'Tiger Riding'),(40,164,0,0,'Blacksmithing'),(40,173,0,5,'Daggers'),(40,183,5,5,'GENERIC (DND)'),(40,197,0,0,'Tailoring'),(40,228,1,5,'Wands'),(40,393,0,0,'Skinning'),(40,109,300,300,'Language: Orcish'),(40,137,0,0,'Language: Thalassian'),(40,140,0,0,'Language: Titan'),(40,149,0,0,'Wolf Riding'),(40,8,5,5,'Fire'),(40,98,0,0,'Language: Common'),(40,315,300,300,'Language: Troll'),(40,139,0,0,'Language: Demon Tongue'),(40,152,0,0,'Ram Riding'),(40,165,0,0,'Leatherworking'),(40,415,1,1,'Cloth'),(40,673,0,0,'Language: Gutterspeak'),(40,95,1,5,'Defense'),(40,333,0,0,'Enchanting'),(40,186,0,0,'Mining'),(40,43,0,5,'Swords'),(40,713,0,0,'Kodo Riding'),(40,733,5,5,'Racial - Troll'),(40,6,5,5,'Frost'),(40,111,0,0,'Language: Dwarven'),(40,113,0,0,'Language: Darnassian'),(40,115,0,0,'Language: Taurahe'),(40,136,1,5,'Staves'),(40,138,0,0,'Language: Draconic'),(40,141,0,0,'Language: Old Tongue'),(40,142,0,1,'Survival'),(40,313,0,0,'Language: Gnomish'),(40,533,0,0,'Raptor Riding'),(40,554,0,0,'Undead Horsemanship'),(40,202,0,0,'Engineering'),(40,356,0,0,'Fishing'),(40,171,0,0,'Alchemy'),(40,182,0,0,'Herbalism'),(40,129,0,0,'First Aid'),(40,185,0,0,'Cooking'),(40,237,0,5,'Arcane'),(40,162,1,5,'Unarmed');
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_skill` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_spell`
--

DROP TABLE IF EXISTS `playercreateinfo_spell`;
CREATE TABLE `playercreateinfo_spell` (
  `createid` smallint(5) unsigned NOT NULL default '0',
  `Spell` bigint(20) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_spell`
--


/*!40000 ALTER TABLE `playercreateinfo_spell` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_spell` WRITE;
INSERT INTO `playercreateinfo_spell` VALUES (1,2382,'Generic'),(1,3365,'Opening'),(1,3050,'Detect'),(1,6233,'Closing'),(1,6246,'Closing'),(1,6247,'Opening'),(1,9125,'Generic'),(1,2479,'Honorless Target'),(1,6477,'Opening'),(1,6478,'Opening'),(1,6603,'Attack'),(1,7266,'Duel'),(1,7267,'Grovel'),(1,7355,'Stuck'),(1,8386,'Attacking'),(1,21651,'Opening'),(1,21652,'Closing'),(1,22027,'Remove Insignia'),(1,22810,'Opening - No Text'),(1,20599,'Diplomacy'),(1,20600,'Perception'),(1,20597,'Sword Specialization'),(1,20598,'The Human Spirit'),(1,20864,'Mace Specialization'),(1,9116,'Shield'),(1,668,'Language Common'),(1,9078,'Cloth'),(1,204,'Defense'),(1,81,'Dodge'),(1,522,'SPELLDEFENSE (DND)'),(1,107,'Block'),(1,5301,'Defensive State (DND)'),(1,201,'One-Handed Swords'),(1,78,'Heroic Strike'),(1,2457,'Battle Stance'),(1,196,'One-Handed Axes'),(1,8737,'Mail'),(1,9077,'Leather'),(1,198,'One-Handed Maces'),(1,203,'Unarmed'),(2,2382,'Generic'),(2,3365,'Opening'),(2,3050,'Detect'),(2,6233,'Closing'),(2,6246,'Closing'),(2,6247,'Opening'),(2,9125,'Generic'),(2,2479,'Honorless Target'),(2,6477,'Opening'),(2,6478,'Opening'),(2,6603,'Attack'),(2,7266,'Duel'),(2,7267,'Grovel'),(2,7355,'Stuck'),(2,8386,'Attacking'),(2,21651,'Opening'),(2,21652,'Closing'),(2,22027,'Remove Insignia'),(2,22810,'Opening - No Text'),(2,20599,'Diplomacy'),(2,20600,'Perception'),(2,20597,'Sword Specialization'),(2,20598,'The Human Spirit'),(2,20864,'Mace Specialization'),(2,9116,'Shield'),(2,668,'Language Common'),(2,9078,'Cloth'),(2,635,'Holy Light'),(2,20154,'Seal of Righteousness'),(2,204,'Defense'),(2,81,'Dodge'),(2,522,'SPELLDEFENSE (DND)'),(2,107,'Block'),(2,199,'Two-Handed Maces'),(2,8737,'Mail'),(2,9077,'Leather'),(2,198,'One-Handed Maces'),(2,203,'Unarmed'),(3,1180,'Daggers'),(3,2382,'Generic'),(3,3365,'Opening'),(3,3050,'Detect'),(3,6233,'Closing'),(3,6246,'Closing'),(3,6247,'Opening'),(3,9125,'Generic'),(3,2479,'Honorless Target'),(3,6477,'Opening'),(3,6478,'Opening'),(3,6603,'Attack'),(3,7266,'Duel'),(3,7267,'Grovel'),(3,7355,'Stuck'),(3,8386,'Attacking'),(3,21651,'Opening'),(3,21652,'Closing'),(3,22027,'Remove Insignia'),(3,22810,'Opening - No Text'),(3,20599,'Diplomacy'),(3,20600,'Perception'),(3,20597,'Sword Specialization'),(3,20598,'The Human Spirit'),(3,20864,'Mace Specialization'),(3,668,'Language Common'),(3,9078,'Cloth'),(3,204,'Defense'),(3,81,'Dodge'),(3,522,'SPELLDEFENSE (DND)'),(3,16092,'Defensive State (DND)'),(3,2567,'Thrown'),(3,2764,'Throw'),(3,1752,'Sinister Strike'),(3,21184,'Rogue Passive (DND)'),(3,2098,'Eviscerate'),(3,9077,'Leather'),(3,203,'Unarmed'),(4,2382,'Generic'),(4,3365,'Opening'),(4,3050,'Detect'),(4,6233,'Closing'),(4,6246,'Closing'),(4,6247,'Opening'),(4,9125,'Generic'),(4,2479,'Honorless Target'),(4,6477,'Opening'),(4,6478,'Opening'),(4,6603,'Attack'),(4,7266,'Duel'),(4,7267,'Grovel'),(4,7355,'Stuck'),(4,8386,'Attacking'),(4,21651,'Opening'),(4,21652,'Closing'),(4,22027,'Remove Insignia'),(4,22810,'Opening - No Text'),(4,5009,'Wands'),(4,5019,'Shoot'),(4,20599,'Diplomacy'),(4,20600,'Perception'),(4,20597,'Sword Specialization'),(4,20598,'The Human Spirit'),(4,20864,'Mace Specialization'),(4,668,'Language Common'),(4,9078,'Cloth'),(4,204,'Defense'),(4,81,'Dodge'),(4,522,'SPELLDEFENSE (DND)'),(4,2050,'Lesser Heal'),(4,585,'Smite'),(4,198,'One-Handed Maces'),(4,203,'Unarmed'),(5,2382,'Generic'),(5,3365,'Opening'),(5,3050,'Detect'),(5,6233,'Closing'),(5,6246,'Closing'),(5,6247,'Opening'),(5,9125,'Generic'),(5,2479,'Honorless Target'),(5,6477,'Opening'),(5,6478,'Opening'),(5,6603,'Attack'),(5,7266,'Duel'),(5,7267,'Grovel'),(5,7355,'Stuck'),(5,8386,'Attacking'),(5,21651,'Opening'),(5,21652,'Closing'),(5,22027,'Remove Insignia'),(5,22810,'Opening - No Text'),(5,5009,'Wands'),(5,5019,'Shoot'),(5,20599,'Diplomacy'),(5,20600,'Perception'),(5,20597,'Sword Specialization'),(5,20598,'The Human Spirit'),(5,20864,'Mace Specialization'),(5,133,'Fireball'),(5,668,'Language Common'),(5,9078,'Cloth'),(5,204,'Defense'),(5,81,'Dodge'),(5,522,'SPELLDEFENSE (DND)'),(5,168,'Frost Armor'),(5,227,'Staves'),(5,203,'Unarmed'),(6,1180,'Daggers'),(6,2382,'Generic'),(6,3365,'Opening'),(6,3050,'Detect'),(6,6233,'Closing'),(6,6246,'Closing'),(6,6247,'Opening'),(6,9125,'Generic'),(6,2479,'Honorless Target'),(6,6477,'Opening'),(6,6478,'Opening'),(6,6603,'Attack'),(6,7266,'Duel'),(6,7267,'Grovel'),(6,7355,'Stuck'),(6,8386,'Attacking'),(6,21651,'Opening'),(6,21652,'Closing'),(6,22027,'Remove Insignia'),(6,22810,'Opening - No Text'),(6,5009,'Wands'),(6,5019,'Shoot'),(6,20599,'Diplomacy'),(6,20600,'Perception'),(6,20597,'Sword Specialization'),(6,20598,'The Human Spirit'),(6,20864,'Mace Specialization'),(6,686,'Shadow Bolt'),(6,668,'Language Common'),(6,687,'Demon Skin'),(6,9078,'Cloth'),(6,204,'Defense'),(6,81,'Dodge'),(6,522,'SPELLDEFENSE (DND)'),(6,203,'Unarmed'),(7,197,'Two-Handed Axes'),(7,2382,'Generic'),(7,3365,'Opening'),(7,3050,'Detect'),(7,6233,'Closing'),(7,6246,'Closing'),(7,6247,'Opening'),(7,9125,'Generic'),(7,2479,'Honorless Target'),(7,6477,'Opening'),(7,6478,'Opening'),(7,6603,'Attack'),(7,7266,'Duel'),(7,7267,'Grovel'),(7,7355,'Stuck'),(7,8386,'Attacking'),(7,21651,'Opening'),(7,21652,'Closing'),(7,22027,'Remove Insignia'),(7,22810,'Opening - No Text'),(7,669,'Language Orcish'),(7,9116,'Shield'),(7,21563,'Command'),(7,20572,'Blood Fury'),(7,20573,'Hardiness'),(7,20574,'Axe Specialization'),(7,9078,'Cloth'),(7,204,'Defense'),(7,81,'Dodge'),(7,522,'SPELLDEFENSE (DND)'),(7,107,'Block'),(7,5301,'Defensive State (DND)'),(7,201,'One-Handed Swords'),(7,78,'Heroic Strike'),(7,2457,'Battle Stance'),(7,196,'One-Handed Axes'),(7,8737,'Mail'),(7,9077,'Leather'),(7,203,'Unarmed'),(8,2382,'Generic'),(8,3365,'Opening'),(8,3050,'Detect'),(8,6233,'Closing'),(8,6246,'Closing'),(8,6247,'Opening'),(8,9125,'Generic'),(8,2479,'Honorless Target'),(8,6477,'Opening'),(8,6478,'Opening'),(8,6603,'Attack'),(8,7266,'Duel'),(8,7267,'Grovel'),(8,7355,'Stuck'),(8,8386,'Attacking'),(8,21651,'Opening'),(8,21652,'Closing'),(8,22027,'Remove Insignia'),(8,22810,'Opening - No Text'),(8,669,'Language Orcish'),(8,20572,'Blood Fury'),(8,20573,'Hardiness'),(8,20574,'Axe Specialization'),(8,20576,'Command'),(8,9078,'Cloth'),(8,204,'Defense'),(8,81,'Dodge'),(8,522,'SPELLDEFENSE (DND)'),(8,13358,'Defensive State (DND)'),(8,24949,'Defensive State 2 (DND)'),(8,196,'One-Handed Axes'),(8,2973,'Raptor Strike'),(8,9077,'Leather'),(8,264,'Bows'),(8,75,'Auto Shot'),(8,203,'Unarmed'),(9,1180,'Daggers'),(9,2382,'Generic'),(9,3365,'Opening'),(9,3050,'Detect'),(9,6233,'Closing'),(9,6246,'Closing'),(9,6247,'Opening'),(9,9125,'Generic'),(9,2479,'Honorless Target'),(9,6477,'Opening'),(9,6478,'Opening'),(9,6603,'Attack'),(9,7266,'Duel'),(9,7267,'Grovel'),(9,7355,'Stuck'),(9,8386,'Attacking'),(9,21651,'Opening'),(9,21652,'Closing'),(9,22027,'Remove Insignia'),(9,22810,'Opening - No Text'),(9,669,'Language Orcish'),(9,21563,'Command'),(9,20572,'Blood Fury'),(9,20573,'Hardiness'),(9,20574,'Axe Specialization'),(9,9078,'Cloth'),(9,204,'Defense'),(9,81,'Dodge'),(9,522,'SPELLDEFENSE (DND)'),(9,16092,'Defensive State (DND)'),(9,2567,'Thrown'),(9,2764,'Throw'),(9,1752,'Sinister Strike'),(9,21184,'Rogue Passive (DND)'),(9,2098,'Eviscerate'),(9,9077,'Leather'),(9,203,'Unarmed'),(10,2382,'Generic'),(10,3365,'Opening'),(10,3050,'Detect'),(10,6233,'Closing'),(10,6246,'Closing'),(10,6247,'Opening'),(10,9125,'Generic'),(10,2479,'Honorless Target'),(10,6477,'Opening'),(10,6478,'Opening'),(10,6603,'Attack'),(10,7266,'Duel'),(10,7267,'Grovel'),(10,7355,'Stuck'),(10,8386,'Attacking'),(10,21651,'Opening'),(10,21652,'Closing'),(10,22027,'Remove Insignia'),(10,22810,'Opening - No Text'),(10,669,'Language Orcish'),(10,9116,'Shield'),(10,21563,'Command'),(10,20572,'Blood Fury'),(10,20573,'Hardiness'),(10,20574,'Axe Specialization'),(10,331,'Healing Wave'),(10,403,'Lightning Bolt'),(10,9078,'Cloth'),(10,204,'Defense'),(10,81,'Dodge'),(10,522,'SPELLDEFENSE (DND)'),(10,107,'Block'),(10,227,'Staves'),(10,9077,'Leather'),(10,198,'One-Handed Maces'),(10,203,'Unarmed'),(11,1180,'Daggers'),(11,2382,'Generic'),(11,3365,'Opening'),(11,3050,'Detect'),(11,6233,'Closing'),(11,6246,'Closing'),(11,6247,'Opening'),(11,9125,'Generic'),(11,2479,'Honorless Target'),(11,6477,'Opening'),(11,6478,'Opening'),(11,6603,'Attack'),(11,7266,'Duel'),(11,7267,'Grovel'),(11,7355,'Stuck'),(11,8386,'Attacking'),(11,21651,'Opening'),(11,21652,'Closing'),(11,22027,'Remove Insignia'),(11,22810,'Opening - No Text'),(11,5009,'Wands'),(11,5019,'Shoot'),(11,669,'Language Orcish'),(11,686,'Shadow Bolt'),(11,20572,'Blood Fury'),(11,20573,'Hardiness'),(11,20574,'Axe Specialization'),(11,20575,'Command'),(11,687,'Demon Skin'),(11,9078,'Cloth'),(11,204,'Defense'),(11,81,'Dodge'),(11,522,'SPELLDEFENSE (DND)'),(11,203,'Unarmed'),(12,197,'Two-Handed Axes'),(12,2382,'Generic'),(12,3365,'Opening'),(12,3050,'Detect'),(12,6233,'Closing'),(12,6246,'Closing'),(12,6247,'Opening'),(12,9125,'Generic'),(12,2479,'Honorless Target'),(12,6477,'Opening'),(12,6478,'Opening'),(12,6603,'Attack'),(12,7266,'Duel'),(12,7267,'Grovel'),(12,7355,'Stuck'),(12,8386,'Attacking'),(12,21651,'Opening'),(12,21652,'Closing'),(12,22027,'Remove Insignia'),(12,22810,'Opening - No Text'),(12,9116,'Shield'),(12,668,'Language Common'),(12,9078,'Cloth'),(12,204,'Defense'),(12,81,'Dodge'),(12,522,'SPELLDEFENSE (DND)'),(12,107,'Block'),(12,5301,'Defensive State (DND)'),(12,78,'Heroic Strike'),(12,2457,'Battle Stance'),(12,196,'One-Handed Axes'),(12,2481,'Find Treasure'),(12,20596,'Frost Resistance'),(12,20595,'Gun Specialization'),(12,20594,'Stoneform'),(12,672,'Language Dwarven'),(12,8737,'Mail'),(12,9077,'Leather'),(12,198,'One-Handed Maces'),(12,203,'Unarmed'),(13,2382,'Generic'),(13,3365,'Opening'),(13,3050,'Detect'),(13,6233,'Closing'),(13,6246,'Closing'),(13,6247,'Opening'),(13,9125,'Generic'),(13,2479,'Honorless Target'),(13,6477,'Opening'),(13,6478,'Opening'),(13,6603,'Attack'),(13,7266,'Duel'),(13,7267,'Grovel'),(13,7355,'Stuck'),(13,8386,'Attacking'),(13,21651,'Opening'),(13,21652,'Closing'),(13,22027,'Remove Insignia'),(13,22810,'Opening - No Text'),(13,9116,'Shield'),(13,668,'Language Common'),(13,9078,'Cloth'),(13,635,'Holy Light'),(13,20154,'Seal of Righteousness'),(13,204,'Defense'),(13,81,'Dodge'),(13,522,'SPELLDEFENSE (DND)'),(13,107,'Block'),(13,2481,'Find Treasure'),(13,20596,'Frost Resistance'),(13,20595,'Gun Specialization'),(13,20594,'Stoneform'),(13,672,'Language Dwarven'),(13,199,'Two-Handed Maces'),(13,8737,'Mail'),(13,9077,'Leather'),(13,198,'One-Handed Maces'),(13,203,'Unarmed'),(14,2382,'Generic'),(14,3365,'Opening'),(14,3050,'Detect'),(14,6233,'Closing'),(14,6246,'Closing'),(14,6247,'Opening'),(14,9125,'Generic'),(14,2479,'Honorless Target'),(14,6477,'Opening'),(14,6478,'Opening'),(14,6603,'Attack'),(14,7266,'Duel'),(14,7267,'Grovel'),(14,7355,'Stuck'),(14,8386,'Attacking'),(14,21651,'Opening'),(14,21652,'Closing'),(14,22027,'Remove Insignia'),(14,22810,'Opening - No Text'),(14,668,'Language Common'),(14,9078,'Cloth'),(14,204,'Defense'),(14,81,'Dodge'),(14,522,'SPELLDEFENSE (DND)'),(14,13358,'Defensive State (DND)'),(14,24949,'Defensive State 2 (DND)'),(14,196,'One-Handed Axes'),(14,2973,'Raptor Strike'),(14,2481,'Find Treasure'),(14,20596,'Frost Resistance'),(14,20595,'Gun Specialization'),(14,20594,'Stoneform'),(14,672,'Language Dwarven'),(14,9077,'Leather'),(14,75,'Auto Shot'),(14,266,'Guns'),(14,203,'Unarmed'),(15,1180,'Daggers'),(15,2382,'Generic'),(15,3365,'Opening'),(15,3050,'Detect'),(15,6233,'Closing'),(15,6246,'Closing'),(15,6247,'Opening'),(15,9125,'Generic'),(15,2479,'Honorless Target'),(15,6477,'Opening'),(15,6478,'Opening'),(15,6603,'Attack'),(15,7266,'Duel'),(15,7267,'Grovel'),(15,7355,'Stuck'),(15,8386,'Attacking'),(15,21651,'Opening'),(15,21652,'Closing'),(15,22027,'Remove Insignia'),(15,22810,'Opening - No Text'),(15,668,'Language Common'),(15,9078,'Cloth'),(15,204,'Defense'),(15,81,'Dodge'),(15,522,'SPELLDEFENSE (DND)'),(15,16092,'Defensive State (DND)'),(15,2567,'Thrown'),(15,2764,'Throw'),(15,1752,'Sinister Strike'),(15,21184,'Rogue Passive (DND)'),(15,2481,'Find Treasure'),(15,20596,'Frost Resistance'),(15,20595,'Gun Specialization'),(15,20594,'Stoneform'),(15,672,'Language Dwarven'),(15,2098,'Eviscerate'),(15,9077,'Leather'),(15,203,'Unarmed'),(16,2382,'Generic'),(16,3365,'Opening'),(16,3050,'Detect'),(16,6233,'Closing'),(16,6246,'Closing'),(16,6247,'Opening'),(16,9125,'Generic'),(16,2479,'Honorless Target'),(16,6477,'Opening'),(16,6478,'Opening'),(16,6603,'Attack'),(16,7266,'Duel'),(16,7267,'Grovel'),(16,7355,'Stuck'),(16,8386,'Attacking'),(16,21651,'Opening'),(16,21652,'Closing'),(16,22027,'Remove Insignia'),(16,22810,'Opening - No Text'),(16,5009,'Wands'),(16,5019,'Shoot'),(16,668,'Language Common'),(16,9078,'Cloth'),(16,204,'Defense'),(16,81,'Dodge'),(16,522,'SPELLDEFENSE (DND)'),(16,2050,'Lesser Heal'),(16,585,'Smite'),(16,2481,'Find Treasure'),(16,20596,'Frost Resistance'),(16,20595,'Gun Specialization'),(16,20594,'Stoneform'),(16,672,'Language Dwarven'),(16,198,'One-Handed Maces'),(16,203,'Unarmed'),(17,1180,'Daggers'),(17,2382,'Generic'),(17,3365,'Opening'),(17,3050,'Detect'),(17,6233,'Closing'),(17,6246,'Closing'),(17,6247,'Opening'),(17,9125,'Generic'),(17,2479,'Honorless Target'),(17,6477,'Opening'),(17,6478,'Opening'),(17,6603,'Attack'),(17,7266,'Duel'),(17,7267,'Grovel'),(17,7355,'Stuck'),(17,8386,'Attacking'),(17,21651,'Opening'),(17,21652,'Closing'),(17,22027,'Remove Insignia'),(17,22810,'Opening - No Text'),(17,9116,'Shield'),(17,668,'Language Common'),(17,9078,'Cloth'),(17,204,'Defense'),(17,81,'Dodge'),(17,522,'SPELLDEFENSE (DND)'),(17,107,'Block'),(17,5301,'Defensive State (DND)'),(17,201,'One-Handed Swords'),(17,78,'Heroic Strike'),(17,2457,'Battle Stance'),(17,671,'Language Darnassian'),(17,8737,'Mail'),(17,9077,'Leather'),(17,198,'One-Handed Maces'),(17,20580,'Shadowmeld'),(17,20583,'Nature Resistance'),(17,20582,'Quickness'),(17,20585,'Wisp Spirit'),(17,21009,'Shadowmeld Passive'),(17,203,'Unarmed'),(21,1180,'Daggers'),(21,2382,'Generic'),(21,3365,'Opening'),(21,3050,'Detect'),(21,6233,'Closing'),(21,6246,'Closing'),(21,6247,'Opening'),(21,9125,'Generic'),(21,2479,'Honorless Target'),(21,6477,'Opening'),(21,6478,'Opening'),(21,6603,'Attack'),(21,7266,'Duel'),(21,7267,'Grovel'),(21,7355,'Stuck'),(21,8386,'Attacking'),(21,21651,'Opening'),(21,21652,'Closing'),(21,22027,'Remove Insignia'),(21,22810,'Opening - No Text'),(21,5176,'Wrath'),(21,5185,'Healing Touch'),(21,668,'Language Common'),(21,9078,'Cloth'),(21,204,'Defense'),(21,81,'Dodge'),(21,522,'SPELLDEFENSE (DND)'),(21,671,'Language Darnassian'),(21,227,'Staves'),(21,9077,'Leather'),(21,20580,'Shadowmeld'),(21,20583,'Nature Resistance'),(21,20582,'Quickness'),(21,20585,'Wisp Spirit'),(21,21009,'Shadowmeld Passive'),(21,203,'Unarmed'),(18,1180,'Daggers'),(18,2382,'Generic'),(18,3365,'Opening'),(18,3050,'Detect'),(18,6233,'Closing'),(18,6246,'Closing'),(18,6247,'Opening'),(18,9125,'Generic'),(18,2479,'Honorless Target'),(18,6477,'Opening'),(18,6478,'Opening'),(18,6603,'Attack'),(18,7266,'Duel'),(18,7267,'Grovel'),(18,7355,'Stuck'),(18,8386,'Attacking'),(18,21651,'Opening'),(18,21652,'Closing'),(18,22027,'Remove Insignia'),(18,22810,'Opening - No Text'),(18,668,'Language Common'),(18,9078,'Cloth'),(18,204,'Defense'),(18,81,'Dodge'),(18,522,'SPELLDEFENSE (DND)'),(18,13358,'Defensive State (DND)'),(18,24949,'Defensive State 2 (DND)'),(18,2973,'Raptor Strike'),(18,671,'Language Darnassian'),(18,9077,'Leather'),(18,264,'Bows'),(18,75,'Auto Shot'),(18,20580,'Shadowmeld'),(18,20583,'Nature Resistance'),(18,20582,'Quickness'),(18,20585,'Wisp Spirit'),(18,21009,'Shadowmeld Passive'),(18,203,'Unarmed'),(19,1180,'Daggers'),(19,2382,'Generic'),(19,3365,'Opening'),(19,3050,'Detect'),(19,6233,'Closing'),(19,6246,'Closing'),(19,6247,'Opening'),(19,9125,'Generic'),(19,2479,'Honorless Target'),(19,6477,'Opening'),(19,6478,'Opening'),(19,6603,'Attack'),(19,7266,'Duel'),(19,7267,'Grovel'),(19,7355,'Stuck'),(19,8386,'Attacking'),(19,21651,'Opening'),(19,21652,'Closing'),(19,22027,'Remove Insignia'),(19,22810,'Opening - No Text'),(19,668,'Language Common'),(19,9078,'Cloth'),(19,204,'Defense'),(19,81,'Dodge'),(19,522,'SPELLDEFENSE (DND)'),(19,16092,'Defensive State (DND)'),(19,2567,'Thrown'),(19,2764,'Throw'),(19,1752,'Sinister Strike'),(19,21184,'Rogue Passive (DND)'),(19,671,'Language Darnassian'),(19,2098,'Eviscerate'),(19,9077,'Leather'),(19,20580,'Shadowmeld'),(19,20583,'Nature Resistance'),(19,20582,'Quickness'),(19,20585,'Wisp Spirit'),(19,21009,'Shadowmeld Passive'),(19,203,'Unarmed'),(20,2382,'Generic'),(20,3365,'Opening'),(20,3050,'Detect'),(20,6233,'Closing'),(20,6246,'Closing'),(20,6247,'Opening'),(20,9125,'Generic'),(20,2479,'Honorless Target'),(20,6477,'Opening'),(20,6478,'Opening'),(20,6603,'Attack'),(20,7266,'Duel'),(20,7267,'Grovel'),(20,7355,'Stuck'),(20,8386,'Attacking'),(20,21651,'Opening'),(20,21652,'Closing'),(20,22027,'Remove Insignia'),(20,22810,'Opening - No Text'),(20,5009,'Wands'),(20,5019,'Shoot'),(20,668,'Language Common'),(20,9078,'Cloth'),(20,204,'Defense'),(20,81,'Dodge'),(20,522,'SPELLDEFENSE (DND)'),(20,2050,'Lesser Heal'),(20,585,'Smite'),(20,671,'Language Darnassian'),(20,198,'One-Handed Maces'),(20,20580,'Shadowmeld'),(20,20583,'Nature Resistance'),(20,20582,'Quickness'),(20,20585,'Wisp Spirit'),(20,21009,'Shadowmeld Passive'),(20,203,'Unarmed'),(22,1180,'Daggers'),(22,2382,'Generic'),(22,3365,'Opening'),(22,3050,'Detect'),(22,6233,'Closing'),(22,6246,'Closing'),(22,6247,'Opening'),(22,9125,'Generic'),(22,2479,'Honorless Target'),(22,6477,'Opening'),(22,6478,'Opening'),(22,6603,'Attack'),(22,7266,'Duel'),(22,7267,'Grovel'),(22,7355,'Stuck'),(22,8386,'Attacking'),(22,21651,'Opening'),(22,21652,'Closing'),(22,22027,'Remove Insignia'),(22,22810,'Opening - No Text'),(22,5227,'Underwater Breathing'),(22,7744,'Will of the Forsaken'),(22,20577,'Cannibalize'),(22,20579,'Shadow Resistance'),(22,669,'Language Orcish'),(22,9116,'Shield'),(22,9078,'Cloth'),(22,17737,'Language Gutterspeak'),(22,204,'Defense'),(22,81,'Dodge'),(22,522,'SPELLDEFENSE (DND)'),(22,107,'Block'),(22,5301,'Defensive State (DND)'),(22,201,'One-Handed Swords'),(22,202,'Two-Handed Swords'),(22,78,'Heroic Strike'),(22,2457,'Battle Stance'),(22,8737,'Mail'),(22,9077,'Leather'),(22,203,'Unarmed'),(23,1180,'Daggers'),(23,2382,'Generic'),(23,3365,'Opening'),(23,3050,'Detect'),(23,6233,'Closing'),(23,6246,'Closing'),(23,6247,'Opening'),(23,9125,'Generic'),(23,2479,'Honorless Target'),(23,6477,'Opening'),(23,6478,'Opening'),(23,6603,'Attack'),(23,7266,'Duel'),(23,7267,'Grovel'),(23,7355,'Stuck'),(23,8386,'Attacking'),(23,21651,'Opening'),(23,21652,'Closing'),(23,22027,'Remove Insignia'),(23,22810,'Opening - No Text'),(23,5227,'Underwater Breathing'),(23,7744,'Will of the Forsaken'),(23,20577,'Cannibalize'),(23,20579,'Shadow Resistance'),(23,669,'Language Orcish'),(23,9078,'Cloth'),(23,17737,'Language Gutterspeak'),(23,204,'Defense'),(23,81,'Dodge'),(23,522,'SPELLDEFENSE (DND)'),(23,16092,'Defensive State (DND)'),(23,2567,'Thrown'),(23,2764,'Throw'),(23,1752,'Sinister Strike'),(23,21184,'Rogue Passive (DND)'),(23,2098,'Eviscerate'),(23,9077,'Leather'),(23,203,'Unarmed'),(24,2382,'Generic'),(24,3365,'Opening'),(24,3050,'Detect'),(24,6233,'Closing'),(24,6246,'Closing'),(24,6247,'Opening'),(24,9125,'Generic'),(24,2479,'Honorless Target'),(24,6477,'Opening'),(24,6478,'Opening'),(24,6603,'Attack'),(24,7266,'Duel'),(24,7267,'Grovel'),(24,7355,'Stuck'),(24,8386,'Attacking'),(24,21651,'Opening'),(24,21652,'Closing'),(24,22027,'Remove Insignia'),(24,22810,'Opening - No Text'),(24,5227,'Underwater Breathing'),(24,7744,'Will of the Forsaken'),(24,20577,'Cannibalize'),(24,20579,'Shadow Resistance'),(24,5009,'Wands'),(24,5019,'Shoot'),(24,669,'Language Orcish'),(24,9078,'Cloth'),(24,17737,'Language Gutterspeak'),(24,204,'Defense'),(24,81,'Dodge'),(24,522,'SPELLDEFENSE (DND)'),(24,2050,'Lesser Heal'),(24,585,'Smite'),(24,198,'One-Handed Maces'),(24,203,'Unarmed'),(25,2382,'Generic'),(25,3365,'Opening'),(25,3050,'Detect'),(25,6233,'Closing'),(25,6246,'Closing'),(25,6247,'Opening'),(25,9125,'Generic'),(25,2479,'Honorless Target'),(25,6477,'Opening'),(25,6478,'Opening'),(25,6603,'Attack'),(25,7266,'Duel'),(25,7267,'Grovel'),(25,7355,'Stuck'),(25,8386,'Attacking'),(25,21651,'Opening'),(25,21652,'Closing'),(25,22027,'Remove Insignia'),(25,22810,'Opening - No Text'),(25,5227,'Underwater Breathing'),(25,7744,'Will of the Forsaken'),(25,20577,'Cannibalize'),(25,20579,'Shadow Resistance'),(25,5009,'Wands'),(25,5019,'Shoot'),(25,669,'Language Orcish'),(25,133,'Fireball'),(25,9078,'Cloth'),(25,17737,'Language Gutterspeak'),(25,204,'Defense'),(25,81,'Dodge'),(25,522,'SPELLDEFENSE (DND)'),(25,168,'Frost Armor'),(25,227,'Staves'),(25,203,'Unarmed'),(26,1180,'Daggers'),(26,2382,'Generic'),(26,3365,'Opening'),(26,3050,'Detect'),(26,6233,'Closing'),(26,6246,'Closing'),(26,6247,'Opening'),(26,9125,'Generic'),(26,2479,'Honorless Target'),(26,6477,'Opening'),(26,6478,'Opening'),(26,6603,'Attack'),(26,7266,'Duel'),(26,7267,'Grovel'),(26,7355,'Stuck'),(26,8386,'Attacking'),(26,21651,'Opening'),(26,21652,'Closing'),(26,22027,'Remove Insignia'),(26,22810,'Opening - No Text'),(26,5227,'Underwater Breathing'),(26,7744,'Will of the Forsaken'),(26,20577,'Cannibalize'),(26,20579,'Shadow Resistance'),(26,5009,'Wands'),(26,5019,'Shoot'),(26,669,'Language Orcish'),(26,686,'Shadow Bolt'),(26,687,'Demon Skin'),(26,9078,'Cloth'),(26,17737,'Language Gutterspeak'),(26,204,'Defense'),(26,81,'Dodge'),(26,522,'SPELLDEFENSE (DND)'),(26,203,'Unarmed'),(27,2382,'Generic'),(27,3365,'Opening'),(27,3050,'Detect'),(27,6233,'Closing'),(27,6246,'Closing'),(27,6247,'Opening'),(27,9125,'Generic'),(27,2479,'Honorless Target'),(27,6477,'Opening'),(27,6478,'Opening'),(27,6603,'Attack'),(27,7266,'Duel'),(27,7267,'Grovel'),(27,7355,'Stuck'),(27,8386,'Attacking'),(27,21651,'Opening'),(27,21652,'Closing'),(27,22027,'Remove Insignia'),(27,22810,'Opening - No Text'),(27,669,'Language Orcish'),(27,20549,'War Stomp'),(27,20550,'Endurance'),(27,20551,'Nature Resistance'),(27,20552,'Cultivation'),(27,9116,'Shield'),(27,9078,'Cloth'),(27,204,'Defense'),(27,81,'Dodge'),(27,522,'SPELLDEFENSE (DND)'),(27,107,'Block'),(27,5301,'Defensive State (DND)'),(27,78,'Heroic Strike'),(27,2457,'Battle Stance'),(27,196,'One-Handed Axes'),(27,670,'Language Taurahe'),(27,199,'Two-Handed Maces'),(27,8737,'Mail'),(27,9077,'Leather'),(27,198,'One-Handed Maces'),(27,203,'Unarmed'),(30,2382,'Generic'),(30,3365,'Opening'),(30,3050,'Detect'),(30,6233,'Closing'),(30,6246,'Closing'),(30,6247,'Opening'),(30,9125,'Generic'),(30,2479,'Honorless Target'),(30,6477,'Opening'),(30,6478,'Opening'),(30,6603,'Attack'),(30,7266,'Duel'),(30,7267,'Grovel'),(30,7355,'Stuck'),(30,8386,'Attacking'),(30,21651,'Opening'),(30,21652,'Closing'),(30,22027,'Remove Insignia'),(30,22810,'Opening - No Text'),(30,5176,'Wrath'),(30,669,'Language Orcish'),(30,20549,'War Stomp'),(30,20550,'Endurance'),(30,20551,'Nature Resistance'),(30,20552,'Cultivation'),(30,5185,'Healing Touch'),(30,9078,'Cloth'),(30,204,'Defense'),(30,81,'Dodge'),(30,522,'SPELLDEFENSE (DND)'),(30,670,'Language Taurahe'),(30,227,'Staves'),(30,9077,'Leather'),(30,198,'One-Handed Maces'),(30,203,'Unarmed'),(28,2382,'Generic'),(28,3365,'Opening'),(28,3050,'Detect'),(28,6233,'Closing'),(28,6246,'Closing'),(28,6247,'Opening'),(28,9125,'Generic'),(28,2479,'Honorless Target'),(28,6477,'Opening'),(28,6478,'Opening'),(28,6603,'Attack'),(28,7266,'Duel'),(28,7267,'Grovel'),(28,7355,'Stuck'),(28,8386,'Attacking'),(28,21651,'Opening'),(28,21652,'Closing'),(28,22027,'Remove Insignia'),(28,22810,'Opening - No Text'),(28,669,'Language Orcish'),(28,20549,'War Stomp'),(28,20550,'Endurance'),(28,20551,'Nature Resistance'),(28,20552,'Cultivation'),(28,9078,'Cloth'),(28,204,'Defense'),(28,81,'Dodge'),(28,522,'SPELLDEFENSE (DND)'),(28,13358,'Defensive State (DND)'),(28,24949,'Defensive State 2 (DND)'),(28,196,'One-Handed Axes'),(28,2973,'Raptor Strike'),(28,670,'Language Taurahe'),(28,9077,'Leather'),(28,75,'Auto Shot'),(28,266,'Guns'),(28,203,'Unarmed'),(29,2382,'Generic'),(29,3365,'Opening'),(29,3050,'Detect'),(29,6233,'Closing'),(29,6246,'Closing'),(29,6247,'Opening'),(29,9125,'Generic'),(29,2479,'Honorless Target'),(29,6477,'Opening'),(29,6478,'Opening'),(29,6603,'Attack'),(29,7266,'Duel'),(29,7267,'Grovel'),(29,7355,'Stuck'),(29,8386,'Attacking'),(29,21651,'Opening'),(29,21652,'Closing'),(29,22027,'Remove Insignia'),(29,22810,'Opening - No Text'),(29,669,'Language Orcish'),(29,20549,'War Stomp'),(29,20550,'Endurance'),(29,20551,'Nature Resistance'),(29,20552,'Cultivation'),(29,9116,'Shield'),(29,331,'Healing Wave'),(29,403,'Lightning Bolt'),(29,9078,'Cloth'),(29,204,'Defense'),(29,81,'Dodge'),(29,522,'SPELLDEFENSE (DND)'),(29,107,'Block'),(29,670,'Language Taurahe'),(29,227,'Staves'),(29,9077,'Leather'),(29,198,'One-Handed Maces'),(29,203,'Unarmed'),(31,1180,'Daggers'),(31,2382,'Generic'),(31,3365,'Opening'),(31,3050,'Detect'),(31,6233,'Closing'),(31,6246,'Closing'),(31,6247,'Opening'),(31,9125,'Generic'),(31,2479,'Honorless Target'),(31,6477,'Opening'),(31,6478,'Opening'),(31,6603,'Attack'),(31,7266,'Duel'),(31,7267,'Grovel'),(31,7355,'Stuck'),(31,8386,'Attacking'),(31,21651,'Opening'),(31,21652,'Closing'),(31,22027,'Remove Insignia'),(31,22810,'Opening - No Text'),(31,9116,'Shield'),(31,668,'Language Common'),(31,9078,'Cloth'),(31,204,'Defense'),(31,81,'Dodge'),(31,522,'SPELLDEFENSE (DND)'),(31,107,'Block'),(31,5301,'Defensive State (DND)'),(31,201,'One-Handed Swords'),(31,20589,'Escape Artist'),(31,20591,'Expansive Mind'),(31,20593,'Engineering Specialization'),(31,20592,'Arcane Resistance'),(31,78,'Heroic Strike'),(31,2457,'Battle Stance'),(31,7340,'Language Gnomish'),(31,8737,'Mail'),(31,9077,'Leather'),(31,198,'One-Handed Maces'),(31,203,'Unarmed'),(32,1180,'Daggers'),(32,2382,'Generic'),(32,3365,'Opening'),(32,3050,'Detect'),(32,6233,'Closing'),(32,6246,'Closing'),(32,6247,'Opening'),(32,9125,'Generic'),(32,2479,'Honorless Target'),(32,6477,'Opening'),(32,6478,'Opening'),(32,6603,'Attack'),(32,7266,'Duel'),(32,7267,'Grovel'),(32,7355,'Stuck'),(32,8386,'Attacking'),(32,21651,'Opening'),(32,21652,'Closing'),(32,22027,'Remove Insignia'),(32,22810,'Opening - No Text'),(32,668,'Language Common'),(32,9078,'Cloth'),(32,204,'Defense'),(32,81,'Dodge'),(32,522,'SPELLDEFENSE (DND)'),(32,16092,'Defensive State (DND)'),(32,20589,'Escape Artist'),(32,20591,'Expansive Mind'),(32,20593,'Engineering Specialization'),(32,20592,'Arcane Resistance'),(32,2567,'Thrown'),(32,2764,'Throw'),(32,1752,'Sinister Strike'),(32,21184,'Rogue Passive (DND)'),(32,2098,'Eviscerate'),(32,7340,'Language Gnomish'),(32,9077,'Leather'),(32,203,'Unarmed'),(33,2382,'Generic'),(33,3365,'Opening'),(33,3050,'Detect'),(33,6233,'Closing'),(33,6246,'Closing'),(33,6247,'Opening'),(33,9125,'Generic'),(33,2479,'Honorless Target'),(33,6477,'Opening'),(33,6478,'Opening'),(33,6603,'Attack'),(33,7266,'Duel'),(33,7267,'Grovel'),(33,7355,'Stuck'),(33,8386,'Attacking'),(33,21651,'Opening'),(33,21652,'Closing'),(33,22027,'Remove Insignia'),(33,22810,'Opening - No Text'),(33,5009,'Wands'),(33,5019,'Shoot'),(33,133,'Fireball'),(33,668,'Language Common'),(33,9078,'Cloth'),(33,204,'Defense'),(33,81,'Dodge'),(33,522,'SPELLDEFENSE (DND)'),(33,20589,'Escape Artist'),(33,20591,'Expansive Mind'),(33,20593,'Engineering Specialization'),(33,20592,'Arcane Resistance'),(33,168,'Frost Armor'),(33,227,'Staves'),(33,7340,'Language Gnomish'),(33,203,'Unarmed'),(34,1180,'Daggers'),(34,2382,'Generic'),(34,3365,'Opening'),(34,3050,'Detect'),(34,6233,'Closing'),(34,6246,'Closing'),(34,6247,'Opening'),(34,9125,'Generic'),(34,2479,'Honorless Target'),(34,6477,'Opening'),(34,6478,'Opening'),(34,6603,'Attack'),(34,7266,'Duel'),(34,7267,'Grovel'),(34,7355,'Stuck'),(34,8386,'Attacking'),(34,21651,'Opening'),(34,21652,'Closing'),(34,22027,'Remove Insignia'),(34,22810,'Opening - No Text'),(34,5009,'Wands'),(34,5019,'Shoot'),(34,686,'Shadow Bolt'),(34,668,'Language Common'),(34,687,'Demon Skin'),(34,9078,'Cloth'),(34,204,'Defense'),(34,81,'Dodge'),(34,522,'SPELLDEFENSE (DND)'),(34,20589,'Escape Artist'),(34,20591,'Expansive Mind'),(34,20593,'Engineering Specialization'),(34,20592,'Arcane Resistance'),(34,7340,'Language Gnomish'),(34,203,'Unarmed'),(35,1180,'Daggers'),(35,2382,'Generic'),(35,3365,'Opening'),(35,3050,'Detect'),(35,6233,'Closing'),(35,6246,'Closing'),(35,6247,'Opening'),(35,9125,'Generic'),(35,2479,'Honorless Target'),(35,6477,'Opening'),(35,6478,'Opening'),(35,6603,'Attack'),(35,7266,'Duel'),(35,7267,'Grovel'),(35,7355,'Stuck'),(35,8386,'Attacking'),(35,21651,'Opening'),(35,21652,'Closing'),(35,22027,'Remove Insignia'),(35,22810,'Opening - No Text'),(35,669,'Language Orcish'),(35,9116,'Shield'),(35,7341,'Language Troll'),(35,9078,'Cloth'),(35,204,'Defense'),(35,81,'Dodge'),(35,522,'SPELLDEFENSE (DND)'),(35,107,'Block'),(35,5301,'Defensive State (DND)'),(35,23301,'Berserking'),(35,20555,'Regeneration'),(35,20557,'Beast Slaying'),(35,20558,'Throwing Specialization'),(35,26290,'Bow Specialization'),(35,26296,'Berserking'),(35,2567,'Thrown'),(35,2764,'Throw'),(35,78,'Heroic Strike'),(35,2457,'Battle Stance'),(35,196,'One-Handed Axes'),(35,8737,'Mail'),(35,9077,'Leather'),(35,203,'Unarmed'),(36,2382,'Generic'),(36,3365,'Opening'),(36,3050,'Detect'),(36,6233,'Closing'),(36,6246,'Closing'),(36,6247,'Opening'),(36,9125,'Generic'),(36,2479,'Honorless Target'),(36,6477,'Opening'),(36,6478,'Opening'),(36,6603,'Attack'),(36,7266,'Duel'),(36,7267,'Grovel'),(36,7355,'Stuck'),(36,8386,'Attacking'),(36,21651,'Opening'),(36,21652,'Closing'),(36,22027,'Remove Insignia'),(36,22810,'Opening - No Text'),(36,669,'Language Orcish'),(36,7341,'Language Troll'),(36,9078,'Cloth'),(36,204,'Defense'),(36,81,'Dodge'),(36,522,'SPELLDEFENSE (DND)'),(36,13358,'Defensive State (DND)'),(36,24949,'Defensive State 2 (DND)'),(36,23301,'Berserking'),(36,20554,'Berserking'),(36,20555,'Regeneration'),(36,20557,'Beast Slaying'),(36,20558,'Throwing Specialization'),(36,26290,'Bow Specialization'),(36,196,'One-Handed Axes'),(36,2973,'Raptor Strike'),(36,9077,'Leather'),(36,264,'Bows'),(36,75,'Auto Shot'),(36,203,'Unarmed'),(37,1180,'Daggers'),(37,2382,'Generic'),(37,3365,'Opening'),(37,3050,'Detect'),(37,6233,'Closing'),(37,6246,'Closing'),(37,6247,'Opening'),(37,9125,'Generic'),(37,2479,'Honorless Target'),(37,6477,'Opening'),(37,6478,'Opening'),(37,6603,'Attack'),(37,7266,'Duel'),(37,7267,'Grovel'),(37,7355,'Stuck'),(37,8386,'Attacking'),(37,21651,'Opening'),(37,21652,'Closing'),(37,22027,'Remove Insignia'),(37,22810,'Opening - No Text'),(37,669,'Language Orcish'),(37,7341,'Language Troll'),(37,9078,'Cloth'),(37,204,'Defense'),(37,81,'Dodge'),(37,522,'SPELLDEFENSE (DND)'),(37,16092,'Defensive State (DND)'),(37,23301,'Berserking'),(37,20555,'Regeneration'),(37,20557,'Beast Slaying'),(37,20558,'Throwing Specialization'),(37,26290,'Bow Specialization'),(37,26297,'Berserking'),(37,2567,'Thrown'),(37,2764,'Throw'),(37,1752,'Sinister Strike'),(37,21184,'Rogue Passive (DND)'),(37,2098,'Eviscerate'),(37,9077,'Leather'),(37,203,'Unarmed'),(38,2382,'Generic'),(38,3365,'Opening'),(38,3050,'Detect'),(38,6233,'Closing'),(38,6246,'Closing'),(38,6247,'Opening'),(38,9125,'Generic'),(38,2479,'Honorless Target'),(38,6477,'Opening'),(38,6478,'Opening'),(38,6603,'Attack'),(38,7266,'Duel'),(38,7267,'Grovel'),(38,7355,'Stuck'),(38,8386,'Attacking'),(38,21651,'Opening'),(38,21652,'Closing'),(38,22027,'Remove Insignia'),(38,22810,'Opening - No Text'),(38,5009,'Wands'),(38,5019,'Shoot'),(38,669,'Language Orcish'),(38,7341,'Language Troll'),(38,9078,'Cloth'),(38,204,'Defense'),(38,81,'Dodge'),(38,522,'SPELLDEFENSE (DND)'),(38,23301,'Berserking'),(38,20554,'Berserking'),(38,20555,'Regeneration'),(38,20557,'Beast Slaying'),(38,20558,'Throwing Specialization'),(38,26290,'Bow Specialization'),(38,2050,'Lesser Heal'),(38,585,'Smite'),(38,198,'One-Handed Maces'),(38,203,'Unarmed'),(39,2382,'Generic'),(39,3365,'Opening'),(39,3050,'Detect'),(39,6233,'Closing'),(39,6246,'Closing'),(39,6247,'Opening'),(39,9125,'Generic'),(39,2479,'Honorless Target'),(39,6477,'Opening'),(39,6478,'Opening'),(39,6603,'Attack'),(39,7266,'Duel'),(39,7267,'Grovel'),(39,7355,'Stuck'),(39,8386,'Attacking'),(39,21651,'Opening'),(39,21652,'Closing'),(39,22027,'Remove Insignia'),(39,22810,'Opening - No Text'),(39,669,'Language Orcish'),(39,9116,'Shield'),(39,7341,'Language Troll'),(39,331,'Healing Wave'),(39,403,'Lightning Bolt'),(39,9078,'Cloth'),(39,204,'Defense'),(39,81,'Dodge'),(39,522,'SPELLDEFENSE (DND)'),(39,107,'Block'),(39,23301,'Berserking'),(39,20554,'Berserking'),(39,20555,'Regeneration'),(39,20557,'Beast Slaying'),(39,20558,'Throwing Specialization'),(39,26290,'Bow Specialization'),(39,227,'Staves'),(39,9077,'Leather'),(39,198,'One-Handed Maces'),(39,203,'Unarmed'),(40,2382,'Generic'),(40,3365,'Opening'),(40,3050,'Detect'),(40,6233,'Closing'),(40,6246,'Closing'),(40,6247,'Opening'),(40,9125,'Generic'),(40,2479,'Honorless Target'),(40,6477,'Opening'),(40,6478,'Opening'),(40,6603,'Attack'),(40,7266,'Duel'),(40,7267,'Grovel'),(40,7355,'Stuck'),(40,8386,'Attacking'),(40,21651,'Opening'),(40,21652,'Closing'),(40,22027,'Remove Insignia'),(40,22810,'Opening - No Text'),(40,5009,'Wands'),(40,5019,'Shoot'),(40,669,'Language Orcish'),(40,133,'Fireball'),(40,7341,'Language Troll'),(40,9078,'Cloth'),(40,204,'Defense'),(40,81,'Dodge'),(40,522,'SPELLDEFENSE (DND)'),(40,23301,'Berserking'),(40,20554,'Berserking'),(40,20555,'Regeneration'),(40,20557,'Beast Slaying'),(40,20558,'Throwing Specialization'),(40,26290,'Bow Specialization'),(40,168,'Frost Armor'),(40,227,'Staves'),(40,203,'Unarmed');
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_spell` ENABLE KEYS */;

--
-- Table structure for table `quest_template`
--

DROP TABLE IF EXISTS `quest_template`;
CREATE TABLE `quest_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `ZoneId` int(11) unsigned NOT NULL default '0',
  `QuestFlags` int(11) unsigned NOT NULL default '0',
  `MinLevel` int(11) unsigned NOT NULL default '0',
  `MaxLevel` int(11) unsigned NOT NULL default '0',
  `Type` int(11) unsigned NOT NULL default '0',
  `RequiredRaces` int(11) unsigned NOT NULL default '0',
  `RequiredClass` int(11) unsigned NOT NULL default '0',
  `RequiredTradeskill` int(11) unsigned NOT NULL default '0',
  `LimitTime` int(11) unsigned NOT NULL default '0',
  `SpecialFlags` int(11) unsigned NOT NULL default '0',
  `PrevQuestId` int(11) unsigned NOT NULL default '0',
  `NextQuestId` int(11) unsigned NOT NULL default '0',
  `srcItem` int(11) unsigned NOT NULL default '0',
  `SrcItemCount` int(11) unsigned NOT NULL default '0',
  `Title` varchar(50) default NULL,
  `Details` varchar(250) default NULL,
  `Objectives` varchar(150) default NULL,
  `CompletionText` varchar(250) default NULL,
  `IncompleteText` varchar(200) default NULL,
  `EndText` varchar(200) default NULL,
  `ObjectiveText1` varchar(250) default NULL,
  `ObjectiveText2` varchar(250) default NULL,
  `ObjectiveText3` varchar(250) default NULL,
  `ObjectiveText4` varchar(250) default NULL,
  `ReqItemId1` int(11) unsigned NOT NULL default '0',
  `ReqItemId2` int(11) unsigned NOT NULL default '0',
  `ReqItemId3` int(11) unsigned NOT NULL default '0',
  `ReqItemId4` int(11) unsigned NOT NULL default '0',
  `ReqItemCount1` int(11) unsigned NOT NULL default '0',
  `ReqItemCount2` int(11) unsigned NOT NULL default '0',
  `ReqItemCount3` int(11) unsigned NOT NULL default '0',
  `ReqItemCount4` int(11) unsigned NOT NULL default '0',
  `ReqKillMobId1` int(11) unsigned NOT NULL default '0',
  `ReqKillMobId2` int(11) unsigned NOT NULL default '0',
  `ReqKillMobId3` int(11) unsigned NOT NULL default '0',
  `ReqKillMobId4` int(11) unsigned NOT NULL default '0',
  `ReqKillMobCount1` int(11) unsigned NOT NULL default '0',
  `ReqKillMobCount2` int(11) unsigned NOT NULL default '0',
  `ReqKillMobCount3` int(11) unsigned NOT NULL default '0',
  `ReqKillMobCount4` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId1` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId2` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId3` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId4` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId5` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemId6` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount1` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount2` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount3` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount4` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount5` int(11) unsigned NOT NULL default '0',
  `RewChoiceItemCount6` int(11) unsigned NOT NULL default '0',
  `RewItemId1` int(11) unsigned NOT NULL default '0',
  `RewItemId2` int(11) unsigned NOT NULL default '0',
  `RewItemId3` int(11) unsigned NOT NULL default '0',
  `RewItemId4` int(11) unsigned NOT NULL default '0',
  `RewItemCount1` int(11) unsigned NOT NULL default '0',
  `RewItemCount2` int(11) unsigned NOT NULL default '0',
  `RewItemCount3` int(11) unsigned NOT NULL default '0',
  `RewItemCount4` int(11) unsigned NOT NULL default '0',
  `RewRepFaction1` int(11) unsigned NOT NULL default '0',
  `RewRepFaction2` int(11) unsigned NOT NULL default '0',
  `RewRepValue1` int(11) unsigned NOT NULL default '0',
  `RewRepValue2` int(11) unsigned NOT NULL default '0',
  `RewMoney` int(11) unsigned NOT NULL default '0',
  `RewXP` int(11) unsigned NOT NULL default '0',
  `RewSpell` int(11) unsigned NOT NULL default '0',
  `PointMapId` int(11) unsigned NOT NULL default '0',
  `PointX` float NOT NULL default '0',
  `PointY` float NOT NULL default '0',
  `PointOpt` int(2) unsigned NOT NULL default '0',
  PRIMARY KEY  (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Quest System';

--
-- Dumping data for table `quest_template`
--


/*!40000 ALTER TABLE `quest_template` DISABLE KEYS */;
LOCK TABLES `quest_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `quest_template` ENABLE KEYS */;

--
-- Table structure for table `realmlist`
--

DROP TABLE IF EXISTS `realmlist`;
CREATE TABLE `realmlist` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `name` varchar(32) NOT NULL default '',
  `address` varchar(32) NOT NULL default '127.0.0.1',
  `icon` tinyint(3) unsigned NOT NULL default '0',
  `color` tinyint(3) unsigned NOT NULL default '0',
  `timezone` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm System';

--
-- Dumping data for table `realmlist`
--


/*!40000 ALTER TABLE `realmlist` DISABLE KEYS */;
LOCK TABLES `realmlist` WRITE;
INSERT INTO `realmlist` VALUES (1,'MaNGOS','127.0.0.1',1,0,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `realmlist` ENABLE KEYS */;

--
-- Table structure for table `taxi_node`
--

DROP TABLE IF EXISTS `taxi_node`;
CREATE TABLE `taxi_node` (
  `id` tinyint(3) unsigned NOT NULL auto_increment,
  `continent` tinyint(3) unsigned NOT NULL default '0',
  `position_x` float default NULL,
  `position_y` float default NULL,
  `position_z` float default NULL,
  `name` varchar(255) default NULL,
  `flags` mediumint(11) unsigned default NULL,
  `mount` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `taxinodes_index` (`continent`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `taxi_node`
--


/*!40000 ALTER TABLE `taxi_node` DISABLE KEYS */;
LOCK TABLES `taxi_node` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxi_node` ENABLE KEYS */;

--
-- Table structure for table `taxi_path`
--

DROP TABLE IF EXISTS `taxi_path`;
CREATE TABLE `taxi_path` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `source` tinyint(3) unsigned default NULL,
  `destination` tinyint(3) unsigned default NULL,
  `price` mediumint(8) unsigned default NULL,
  PRIMARY KEY  (`id`),
  KEY `taxipath_index` (`source`,`destination`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

--
-- Dumping data for table `taxi_path`
--


/*!40000 ALTER TABLE `taxi_path` DISABLE KEYS */;
LOCK TABLES `taxi_path` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxi_path` ENABLE KEYS */;

--
-- Table structure for table `taxi_pathnode`
--

DROP TABLE IF EXISTS `taxi_pathnode`;
CREATE TABLE `taxi_pathnode` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `path` smallint(5) unsigned default NULL,
  `index` tinyint(3) unsigned default NULL,
  `continent` tinyint(3) unsigned default NULL,
  `position_x` float default NULL,
  `position_y` float default NULL,
  `position_z` float default NULL,
  `unknown1` mediumint(8) unsigned default NULL,
  `unknown2` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `taxi_pathnode`
--


/*!40000 ALTER TABLE `taxi_pathnode` DISABLE KEYS */;
LOCK TABLES `taxi_pathnode` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `taxi_pathnode` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

