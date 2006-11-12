-- MySQL dump 10.10
--
-- Host: localhost    Database: mangos
-- ------------------------------------------------------
-- Server version	5.0.22-Debian_0ubuntu6.06.2-log

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
  `name` text,
  `trigger_map` int(11) unsigned NOT NULL DEFAULT '0', 
  `trigger_position_x` FLOAT NOT NULL DEFAULT '0', 
  `trigger_position_y` FLOAT NOT NULL DEFAULT '0', 
  `trigger_position_z` FLOAT NOT NULL DEFAULT '0', 
  `target_map` int(11) unsigned NOT NULL DEFAULT '0',
  `target_position_x` float NOT NULL default '0',
  `target_position_y` float NOT NULL default '0',
  `target_position_z` float NOT NULL default '0',
  `target_orientation` float NOT NULL default '0',
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
  `id` bigint(20) unsigned NOT NULL default '0',
  `auctioneerguid` int(32) NOT NULL default '0',
  `itemguid` int(32) NOT NULL default '0',
  `item_template` int(11) unsigned NOT NULL default '0' COMMENT 'Item Identifier',
  `itemowner` int(32) NOT NULL default '0',
  `buyoutprice` int(32) NOT NULL default '0',
  `time` bigint(40) NOT NULL default '0',
  `buyguid` int(32) NOT NULL default '0',
  `lastbid` int(32) NOT NULL default '0',
  `location` tinyint(3) unsigned NOT NULL default '3'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `auctionhouse`
--


/*!40000 ALTER TABLE `auctionhouse` DISABLE KEYS */;
LOCK TABLES `auctionhouse` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `auctionhouse` ENABLE KEYS */;

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
  `highest_rank` int(11) NOT NULL default '0',
  `standing` int(11) NOT NULL default '0',
  `rating` float NOT NULL default '0',
  `cinematic` tinyint(3) unsigned NOT NULL default '0',
  `totaltime` int(11) unsigned NOT NULL default '0',
  `leveltime` int(11) unsigned NOT NULL default '0',
  `logout_time` int(11) NOT NULL DEFAULT '0',
  `is_logout_resting` int(11) NOT NULL DEFAULT '0',
  `rest_bonus` FLOAT NOT NULL DEFAULT '0',
  `resettalents_cost` int(11) unsigned NOT NULL default '0',
  `resettalents_time` bigint(20) unsigned NOT NULL default '0',
  `trans_x` float NOT NULL default '0',
  `trans_y` float NOT NULL default '0',
  `trans_z` float NOT NULL default '0',
  `trans_o` float NOT NULL default '0',
  `transguid` bigint(20) unsigned NOT NULL default '0',
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
  `type` smallint(3) unsigned NOT NULL default '0',
  `misc` smallint(3) unsigned NOT NULL default '0',
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
  PRIMARY KEY  (`guid`,`spell`,`effect_index`)
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
  `loyalty` int(11) unsigned NOT NULL default '1',
  `trainpoint` int(11) unsigned NOT NULL default '0',
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
  `rewarded` tinyint(1) unsigned NOT NULL default '0',
  `explored` tinyint(1) unsigned NOT NULL default '0',
  `completed_once` tinyint(1) unsigned NOT NULL default '0',
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
  `active` int(11) unsigned NOT NULL default '1',
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
-- Table structure for table `character_spell_cooldown`
--

DROP TABLE IF EXISTS `character_spell_cooldown`;
CREATE TABLE `character_spell_cooldown` (
  `guid`  int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier, Low part',
  `spell` int(11) unsigned NOT NULL default '0' COMMENT 'Spell Identifier',
  `time`  bigint(20) unsigned NOT NULL default '0',
   PRIMARY KEY  (`guid`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `character_spell_cooldown`
--

/*!40000 ALTER TABLE `character_spell_cooldown` DISABLE KEYS */;
LOCK TABLES `character_spell_cooldown` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_spell_cooldown` ENABLE KEYS */;


--
-- Table structure for table `character_stable`
--

DROP TABLE IF EXISTS `character_stable`;
CREATE TABLE `character_stable` (
  `owner` int(11) unsigned NOT NULL default '0',
  `slot` int(11) unsigned NOT NULL default '0',
  `petnumber` int(11) unsigned NOT NULL default '0',
  `entry` int(11) unsigned NOT NULL default '0',
  `level` int(11) unsigned NOT NULL default '0',
  `loyalty` int(11) unsigned NOT NULL default '1',
  `trainpoint` int(11) unsigned NOT NULL default '0',
  KEY `petnumber` (`petnumber`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

--
-- Dumping data for table `character_stable`
--


/*!40000 ALTER TABLE `character_stable` DISABLE KEYS */;
LOCK TABLES `character_stable` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `character_stable` ENABLE KEYS */;

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
INSERT INTO `command` VALUES ('acct',0,'Syntax: .acct\r\n\r\nDisplay the access level of your account.'),('addgo',3,'Syntax: .addgo #id\r\n\r\nAdd a game object from game object templates to the world at your current location using the #id.\r\n\r\nNote: this is a copy of .gameobject.'),('additem',3,'Syntax: .additem #itemid [#itemcount] or .additem 0 #itemcount $itemname\r\n\r\nAdds the specified number of items of id #itemid to your or selected character inventory. If #itemcount is omitted, only one item will be added.\r\nWith the second syntax you can add an item with exact (!) name $itemname.'),('additemset',3,'Syntax: .additemset #itemsetid\r\n\r\nAdd items from itemset of id #itemsetid to your or selected character inventory. Will add by one example each item from itemset.'),('addmove',2,'Syntax: .addmove #creature_guid [#waittime]\r\n\r\nAdd your current location as a waypoint for creature with guid #creature_guid. And optional add wait time.'),('addtele',3,'Syntax: .addtele $name\r\n\r\nAdd current your position to .tele command target locations list with name $name.'),('addsh',3,'Syntax: .addsh\r\n\r\nAdd a spirit healer to your current location.\r\n\r\nNot yet implemented.'),('AddSpawn',2,'Not yet implemented.'),('addspirit',3,'Syntax: .addspirit\r\n\r\nSpawns the spirit healer for the current location, if there is one defined for the current location.\r\n\r\nCommand disabled.'),('addspw',2,'Syntax: .addspw #creatureid\r\n\r\nSpawn a creature by the given template id of #creatureid.'),('addweapon',3,'Not yet implemented.'),('allowmove',3,'Syntax: .allowmove\r\n\r\nEnable or disable movement for the selected creature.'),('anim',3,'Syntax: .anim #emoteid\r\n\r\nPlay emote #emoteid for your character.'),('announce',1,'Syntax: .announce $MessageToBroadcast\r\n\r\nSend a global message to all players online.'),('aura',3,'Syntax: .aura #spellid\r\n\r\nAdd the aura from spell #spellid to your character.'),('bank',3,'Syntax: .bank\r\n\r\nShow your bank inventory.'),('changelevel',2,'Syntax: .changelevel #level\r\n\r\nChange the level of the selected creature to #level.\r\n\r\n#level may range from 1 to 63.'),('commands',0,'Syntax: .commands\r\n\r\nDisplay a list of available commands for your account level.'),('createguild',3,'Syntax: .createguild $GuildLeaderName $GuildName\r\n\r\nCreate a guild named $GuildName with the player $GuildLeaderName as leader.'),('cshuttdown',3,'Syntax: .cshuttdown Cancels shuttdown'),('delete',2,'Syntax: .delete\r\n\r\nDelete the selected creature from the world.'),('delobject',2,'Usage: .delobject #go_guid\r\nDelete gameobject with guid #go_guid.'),('deltele',3,'Syntax: .deltele $name\r\n\r\nRemove location with name $name for .tele command locations list.'),('delticket',2,'Syntax: .delticket all\r\n        .delticket #num\r\n        .delticket $character_name\r\n\rall to dalate all tickets at server, $character_name to delete ticket of this character, #num to delete ticket #num.'),('demorph',2,'Syntax: .demorph\r\n\r\nDemorph the selected player.'),('die',3,'Syntax: .die\r\n\r\nKill the selected player. If no player is selected, it will kill you.'),('dismount',0,'Syntax: .dismount\r\n\r\nDismount you, if you are mounted.'),('displayid',2,'Syntax: .displayid #displayid\r\n\r\nChange the model id of the selected creature to #displayid.'),('distance',3,'Syntax: .distance\r\n\r\nDisplay the distance from your character to the selected creature.'),('emote',3,'Syntax: .emote #emoteid\r\n\r\nMake the selected creature emote with an emote of id #emoteid.'),('explorecheat',3,'Syntax: .explorecheat #flag\r\n\r\nReveal  or hide all maps for the selected player. If no player is selected, hide or reveal maps to you.\r\n\r\nUse a #flag of value 1 to reveal, use a #flag value of 0 to hide all maps.'),('factionid',2,'Syntax: .factionid #factionid\r\n\r\nSet the faction of the selected creature to #factionid.'),('gameobject',3,'Syntax: .gameobject #id\r\n\r\nAdd a game object from game object templates to the world at your current position using the #id.'),('getvalue',3,'Syntax: .getvalue #field #isInt\r\n\r\nGet the field #field of the selected creature. If no creature is selected, get the content of your field.\r\n\r\nUse a #isInt of value 1 if the expected field content is an integer.'),('gmlist',0,'Syntax: .gmlist\r\n\r\nDisplay a list of available Game Masters.'),('gmoff',1,'Syntax: .gmoff\r\n\r\nDisable the <GM> prefix for your character.'),('gmon',1,'Syntax: .gmon\r\n\r\nEnable the <GM> prefix for your character.'),('go',3,'Syntax: .go #position_x #position_y #position_z #mapid\r\n\r\nTeleport to the given coordinates on the specified map.'),('gocreature',2,'Usage: .gocreature #creature_guid\r\nTeleport your character to creature with guid #creature_guid.'),('goobject',1,'Usage: .goobject #object_guid\r\nTeleport your character to gameobject with guid #creature_guid'),('goname',1,'Syntax: .goname $charactername\r\n\r\nTeleport to the given character. Either specify the character name or click on the character\'s portait, e.g. when you are in a group.'),('goxy','3','Syntax: .goxy #x #y [#mapid]\r\n\r\nTeleport player to point with (#x,#y) coordinates at ground(water) level at map #mapid or same map if #mapid not provided.'),('gps',1,'Syntax: .gps\r\n\r\nDisplay the position information for a selected character or creature. Position information includes X, Y, Z, and orientation, map Id and zone Id'),('guid',2,'Syntax: .guid\r\n\r\nDisplay the GUID for the selected character.'),('help',0,'Syntax: .help $command\r\n\r\nDisplay usage instructions for the given $command.'),('hidearea',3,'Syntax: .hidearea #areaid\r\n\r\nHide the area of #areaid to the selected character. If no character is selected, hide this area to you.'),('hover',3,'Syntax: .hover #flag\r\n\r\nEnable or disable hover mode for your character.\r\n\r\nUse a #flag of value 1 to enable, use a #flag value of 0 to disable hover.'),('info',0,'Syntax: .info\r\n\r\nDisplay the number of connected players.'),('item',2,'Syntax: .item #guid #amount\r\n\r\nAdd the given amount #amount of the item with a GUID of #guid to the selected vendor. '),('itemmove',2,'Syntax: .itemmove #sourceslotid #destinationslotid\r\n\r\nMove an item from slots #sourceslotid to #destinationslotid in your inventory\r\n\r\nNot yet implemented'),('itemrmv',2,'Syntax: .itemrmv #guid\r\n\r\nRemove the given item with a GUID of #guid from the selected vendor. '),('kick',2,'Syntax: .kick $charactername\r\n\r\nKick the given character from the world.'),('learn',3,'Syntax: .learn #parameter\r\n\r\nSelected character learn a spell of id #parameter. A GM can use .learn all if he wants to learn all default spells for Game Masters, .learn all_lang to learn all langauges, and .learn all_myclass to learn all spells available for his class (Character selection in these cases ignored).'),('learnsk',3,'Syntax: .learnsk #skillId #level #max\r\n\r\nLearn a skill of id #skill with a current skill value of #level and a maximum value of #max for the selected character. If no character is selected, you learn the skill.'),('levelup',3,'Syntax: .levelup #numberoflevels\r\n\r\nIncrease the level of the selected character by #numberoflevels. If #numberoflevels is omitted, the level will be increase by 1. If no character is selected, increase your level.'),('linkgrave',3,'Syntax: .linkgrave #graveyard_id [alliance|horde]\r\n\r\nLink current zone to graveyard for any (or alliance/horde faction ghosts). This let character ghost from zone teleport to graveyard after die if graveyard is nearest from linked to zone and accept ghost of this faction. Add only single graveyard at another map and only if no graveyards linked (or planned linked at same map).'),('loadscripts',3,'Syntax: .loadscripts $scriptlibraryname\r\n\r\nUnload current and load the script library $scriptlibraryname or reload current if $scriptlibraryname omitted, in case you changed it while the server was running.'),('lookupitem',3,'Syntax: .lookupitem $itemname\r\n\r\nLooks up an item by $itemname, and returns all matches with their Item ID\'s.'),('lookupmob',3,'Syntax: .lookupmob $namepart\r\n\r\nLooks up a creature by $namepart, and returns all matches with their creature ID\'s.'),('lookupskill',3,'Syntax: .lookupskill $$namepart\r\n\r\nLooks up a skill by $namepart, and returns all matches with their skill ID\'s.'),('maxskill',3,'Usage: .maxskill\r\nSets all skills of the targeted player to their maximum values for its current level.'),('Mod32Value',3,'Syntax: .Mod32Value #field #value\r\n\r\nAdd #value to field #field of your character.'),('modify',1,'Syntax: .modify $parameter $value\r\n\r\nModify the value of various parameters. Use .help modify $parameter to get help on specific parameter usage.\r\n\r\nSupported parameters include hp, mana, rage, energy, gold, speed, swim, scale, bit, bwalk, aspeed, faction, spell and tp.'),('modify aspeed',1,'Syntax: .modify aspeed #speed\r\n\r\nModify all speeds -run,swim,run back- of the selected player. If no player is selected, modify your speed.\r\n\r\n #speed may range from 0 to 50.'),('modify bit',1,'Syntax: .modify bit #field #bit\r\n\r\nToggle the #bit bit of the #field field for the selected player. If no player is selected, modify your character.'),('modify bwalk',1,'Syntax: .modify bwalk #speed\r\n\r\nModify the speed of the selected player while running backwards. If no player is selected, modify your speed.\r\n\r\n #speed may range from 0 to 50.'),('modify energy',1,'Syntax: .modify energy #energy\r\n\r\nModify the energy of the selected player. If no player is selected, modify your energy.'),('modify faction',1,'Syntax: .modify faction #factionid #flagid #npcflagid #dynamicflagid\r\n\r\nModify the faction and flags of the selected creature. Without arguments, display the faction and flags of the selected creature.'),('modify gold',1,'Syntax: .modify gold #money\r\n\r\nAdd or remove money to the selected player. If no player is selected, modify your money.\r\n\r\n #gold can be negative to remove money.'),('modify hp',1,'Syntax: .modify hp #newhp\r\n\r\nModify the hp of the selected player. If no player is selected, modify your hp.'),('modify mana',1,'Syntax: .modify mana #newmana\r\n\r\nModify the mana of the selected player. If no player is selected, modify your mana.'),('modify rage',1,'Syntax: .modify rage #newrage\r\n\r\nModify the rage of the selected player. If no player is selected, modify your rage.'),('modify scale',1,''),('modify speed',1,'Syntax: .modify speed #speed\r\n\r\nModify the running speed of the selected player. If no player is selected, modify your speed.\r\n\r\n #speed may range from 0 to 50.'),('modify spell',1,''),('modify swim',1,'Syntax: .modify swim #speed\r\n\r\nModify the swim speed of the selected player. If no player is selected, modify your speed.\r\n\r\n #speed may range from 0 to 50.'),('morph',3,'Syntax: .morph #displayid\r\n\r\nChange your current model id to #displayid.'),('moveobject','2','Syntax: .moveobject #goguid [#x #y #z]\r\n\r\nMove gameobject #goguid to character coordintes (or to (#x,#y,#z) coordinates if its provide).'),('name',2,'Syntax: .name $name\r\n\r\nChange the name of the selected creature or character to $name.\r\n\r\nCommand disabled.'),('namego',1,'Syntax: .namego $charactername\r\n\r\nTeleport the given character to you. Either specify the character name or click on the player\'s portrait, e.g. when you are in a group.'),('neargrave',3,'Syntax: .neargrave [alliance|horde]\r\n\r\nFind nearest graveyard linked to zone (or only nearest from accepts alliance or horde faction ghosts).'),('NewMail',3,'Syntax: .NewMail #flag\r\n\r\nSend a new mail notification with flag #flag.'),('npcflag',2,'Syntax: .npcflag #npcflag\r\n\r\nSet the NPC flags of the selected creature to #npcflag.'),('npcinfo',3,'Syntax: .npcinfo\r\n\r\nDisplay a list of details for the selected creature.\r\n\r\nThe list includes:\r\n- GUID, Faction, NPC flags, Entry ID, Model ID,\r\n- Level,\r\n- Health (current/maximum),\r\n- Field flags, dynamic flags, faction template, \r\n- Position information,\r\n- and the creature type, e.g. if the creature is a vendor.'),('npcinfoset',3,'Syntax: .npcinfoset\r\n\r\nTODO: Write me.'),('object',3,'Syntax: .object #displayid $save\r\n\r\nAdd a new object of type mailbox with the display id of #displayid to your current position. If $save is set to \'true\', save the object in the database.'),('pinfo','2','Syntax: .pinfo [$player_name]\r\n\r\nOutput account information for selected player or player find by $player_name.'),('playsound',1,'Syntax: .playsound #soundid\r\n\r\nPlay sound with #soundid.\r\nSound will be play only for you. Other players dont hear this.\r\nWarning: client may have more 5000 sounds...'),('prog',2,'Syntax: .prog\r\n\r\nTeleport you to Programmers Island.'),('QNM',3,'Syntax: .QNM #flag\r\n\r\nQuery next mail time with flag #flag.'),('random',2,'Syntax: .random #flag\r\n\r\nEnable or disable random movement for the selected creature.\r\n\r\nUse a #flag of value 1 to enable, use a #flag value of 0 to disable random movement. Not yet implemented.'),('recall',1,'Syntax: .recall $place\r\n\r\nTeleport you to various towns around the world. $place defines the target location.\r\n\r\nAvailable places include sunr, thun, cross, ogri, neth, thel, storm, iron, under, and darr.'),('reload',3,'Not yet implemented.'),('reset',3,'Usage:\r\n.reset stats\r\n  Resets all stats of the targeted player to their orginal values at level 1.\r\n  Please unequip all items and debuff all auras from the player before using.\r\n.reset talents\r\n  Removes all talents of the targeted player.'),('revive',3,'Syntax: .revive\r\n\r\nRevive the selected player. If no player is selected, it will revive you.'),('run',2,'Syntax: .run #flag\r\n\r\nEnable or disable running movement for a selected creature.\r\n\r\nUse a #flag of value 1 to enable, use a #flag value of 0 to disable running. Not yet implemented.'),('save',0,'Syntax: .save\r\n\r\nSaves your character.'),('searchtele',1,'Syntax: .searchtele $substring\r\n\r\nSearch and output all .tele command locations with provide $substring in name.'),('security',3,'Syntax: .security $name #level\r\n\r\nSet the security level of player $name to a level of #level.\r\n\r\n#level may range from 0 to 5.'),('setvalue',3,'Syntax: .setvalue #field #value #isInt\r\n\r\nSet the field #field of the selected creature with value #value. If no creature is selected, set the content of your field.\r\n\r\nUse a #isInt of value 1 if #value is an integer.'),('showarea',3,'Syntax: .showarea #areaid\r\n\r\nReveal the area of #areaid to the selected character. If no character is selected, reveal this area to you.'),('showhonor',0,'Syntax: .showhonor\r\n\r\nDisplay your honor ranking.'),('shuttdown',3,'Syntax: .shuttdown seconds'),('standstate',3,'Syntax: .standstate #emoteid\r\n\r\nChange the emote of your character while standing to #emoteid.'),('start',0,'Syntax: .start\r\n\r\nTeleport you to the starting area of your character.'),('subname',2,'Syntax: .subname $Name\r\n\r\nChange the subname of the selected creature or player to $Name.\r\n\r\nCommand disabled.'),('shutdown','3','Syntax: .shutdown #delay|stop\r\n\r\nShutting down server after #delay seconds or stop shutting down if stop value used.'),('taxicheat',1,'Syntax: .taxicheat #flag\r\n\r\nTemporary grant access or remove to all taxi routes for the selected character. If no character is selected, hide or reveal all routes to you.\r\n\r\nUse a #flag of value 1 to add access, use a #flag value of 0 to remove access. Visited taxi nodes sill accesaable after removing access.'),('ticket',2,'Syntax: .ticket on\r\n        .ticket off\r\n        .ticket #num\r\n        .ticket $character_name\r\n\r\non/off for GMs to show or not a new ticket directly, $character_name to show ticket of this character, #num to show ticket #num.'),('transport',3,'Not yet implemented.'),('turnobject','2','Syntax: .turnobject #goguid \r\n\r\nSet for gameobject #goguid orientation same as current character orientation.'),('unaura',3,'Syntax: .unaura #spellid\r\n\r\nRemove aura due to spell #spellid from your character.'),('unlearn',3,'Syntax: .unlearn #startspell #endspell\r\n\r\nUnlearn for selected player the range of spells between id #startspell and #endspell. If no #endspell is provided, just unlearn spell of id #startspell.'),('unlearnsk',3,'Syntax: .unlearnsk #parameter\r\n\r\nUnlearn a skill of id #parameter for the selected character. If no character is selected, you unlearn the skill.'),('update',3,'Syntax: .update #field #value\r\n\r\nUpdate the field #field of the selected character or creature with value #value.\r\n\r\nIf no #value is provided, display the content of field #field.'),('visible','1','Syntax: .visible [0||1]\r\n\r\nOutput current visibility state or make GM visible(1) and invisible(0) fot other players.'),('wchange',3,'Syntax: .wchange #weathertype #status\r\n\r\nSet current weather to #weathertype with an intensitiy of #status.\r\n\r\n#weathertype can be 1 for rain, 2 for snow, and 3 for sand. #status can be 0 for disabled, and 1 for enabled.'),('worldport',3,'Syntax: .worldport #map #position_x #position_y #position_z\r\n\r\nTeleport to the given coordinates on the specified continent (map).'),('whispers',1,'Usage: .whispers on|off\r\nEnable/disable accepting whispers by GM from players. By default use mangosd.conf setting.');
UNLOCK TABLES;
/*!40000 ALTER TABLE `command` ENABLE KEYS */;

--
-- Table structure for table `corpse`
--

DROP TABLE IF EXISTS `corpse`;
CREATE TABLE `corpse` (
  `guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `player` bigint(20) unsigned NOT NULL default '0' COMMENT 'Character Global Identifier',
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
  KEY `idx_bones_flag` (`bones_flag`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Death System';

--
-- Dumping data for table `corpse`
--


/*!40000 ALTER TABLE `corpse` DISABLE KEYS */;
LOCK TABLES `corpse` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `corpse` ENABLE KEYS */;

--
-- Table structure for table `corpse_grid`
--

DROP TABLE IF EXISTS `corpse_grid`;
CREATE TABLE `corpse_grid` (
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
-- Dumping data for table `corpse_grid`
--


/*!40000 ALTER TABLE `corpse_grid` DISABLE KEYS */;
LOCK TABLES `corpse_grid` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `corpse_grid` ENABLE KEYS */;

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
  `MovementType` int(11) unsigned NOT NULL default '0',
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
-- Table structure for table `creature_loot_template`
--

DROP TABLE IF EXISTS `creature_loot_template`;
CREATE TABLE `creature_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `creature_loot_template`
--


/*!40000 ALTER TABLE `creature_loot_template` DISABLE KEYS */;
LOCK TABLES `creature_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `creature_loot_template` ENABLE KEYS */;

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
  `modelid_m` int(11) unsigned default '0',
  `modelid_f` int(11) unsigned default '0',
  `name` varchar(100) NOT NULL default '0',
  `subname` varchar(100) default NULL,
  `minlevel` int(3) unsigned default '1',
  `maxlevel` int(3) unsigned default '1',
  `minhealth` int(5) unsigned default '0',
  `maxhealth` int(5) unsigned default '0',
  `minmana` int(5) unsigned default '0',
  `maxmana` int(5) unsigned default '0',
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
  `dynamicflags` int(11) unsigned default '0',
  `size` float default '0',
  `family` int(11) default '0',
  `bounding_radius` float default '0',
  `trainer_type` int(11) default '0',
  `trainer_spell` int(11) unsigned default '0',
  `class` int(11) unsigned default '0',
  `race` int(11) unsigned default '0',
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
  `mingold` int(30) unsigned NOT NULL default '0',
  `maxgold` int(30) unsigned NOT NULL default '0',
  `AIName` varchar(128) NOT NULL default '',
  `MovementType` int(11) unsigned NOT NULL default '0',
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
-- Table structure for table `fishing_loot_template`
--

DROP TABLE IF EXISTS `fishing_loot_template`;
CREATE TABLE `fishing_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `fishing_loot_template`
--


/*!40000 ALTER TABLE `fishing_loot_template` DISABLE KEYS */;
LOCK TABLES `fishing_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `fishing_loot_template` ENABLE KEYS */;

--
-- Table structure for table `game_addons`
--

DROP TABLE IF EXISTS `game_addons`;
CREATE TABLE `game_addons` (
  `addonname` char(255) NOT NULL default '',
  `crc` bigint(20) NOT NULL default '0',
  `enabled` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`addonname`),
  KEY `addonname` (`addonname`,`crc`,`enabled`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Addon system';

--
-- Dumping data for table `game_addons`
--


/*!40000 ALTER TABLE `game_addons` DISABLE KEYS */;
LOCK TABLES `game_addons` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_addons` ENABLE KEYS */;

--
-- Table structure for table `game_graveyard_zone`
--

DROP TABLE IF EXISTS `game_graveyard_zone`;
CREATE TABLE `game_graveyard_zone` (
  `id` int(11) unsigned NOT NULL default '0',
  `ghost_map` int(11) unsigned NOT NULL default '0',
  `ghost_zone` int(11) unsigned NOT NULL default '0',
  `faction` int(11) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Trigger System';

--
-- Dumping data for table `game_graveyard_zone`
--


/*!40000 ALTER TABLE `game_graveyard_zone` DISABLE KEYS */;
LOCK TABLES `game_graveyard_zone` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_graveyard_zone` ENABLE KEYS */;

--
-- Table structure for table `game_tele`
--

DROP TABLE IF EXISTS `game_tele`;
CREATE TABLE `game_tele` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `position_x` float NOT NULL default '0',
  `position_y` float NOT NULL default '0',
  `position_z` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `map` int(11) unsigned NOT NULL default '0',
  `name` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Tele Command';

--
-- Dumping data for table `game_tele`
--


/*!40000 ALTER TABLE `game_tele` DISABLE KEYS */;
LOCK TABLES `game_tele` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `game_tele` ENABLE KEYS */;

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
  `animprogress` int(11) unsigned NOT NULL default '0',
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
-- Table structure for table `gameobject_involvedrelation`
--

DROP TABLE IF EXISTS `gameobject_involvedrelation`;
CREATE TABLE `gameobject_involvedrelation` (
  `id` int(11) unsigned NOT NULL default '0',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gameobject_involvedrelation`
--


/*!40000 ALTER TABLE `gameobject_involvedrelation` DISABLE KEYS */;
LOCK TABLES `gameobject_involvedrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject_involvedrelation` ENABLE KEYS */;

--
-- Table structure for table `gameobject_loot_template`
--

DROP TABLE IF EXISTS `gameobject_loot_template`;
CREATE TABLE `gameobject_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `gameobject_loot_template`
--


/*!40000 ALTER TABLE `gameobject_loot_template` DISABLE KEYS */;
LOCK TABLES `gameobject_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject_loot_template` ENABLE KEYS */;

--
-- Table structure for table `gameobject_questrelation`
--

DROP TABLE IF EXISTS `gameobject_questrelation`;
CREATE TABLE `gameobject_questrelation` (
  `id` int(11) unsigned NOT NULL default '0',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gameobject_questrelation`
--


/*!40000 ALTER TABLE `gameobject_questrelation` DISABLE KEYS */;
LOCK TABLES `gameobject_questrelation` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gameobject_questrelation` ENABLE KEYS */;

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
  `castsSpell` int(11) NOT NULL default '0',
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
  `info` TEXT NOT NULL,
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
-- Table structure for table `guild_charter`
--

DROP TABLE IF EXISTS `guild_charter`;
CREATE TABLE `guild_charter` (
  `ownerguid` int(10) unsigned NOT NULL,
  `charterguid` int(10) unsigned default '0', 
  `guildname` varchar(255) NOT NULL default '',
  PRIMARY KEY (`ownerguid`),
  UNIQUE KEY `index_ownerguid_charterguid` (`ownerguid`,`charterguid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild_charter`
--


/*!40000 ALTER TABLE `guild_charter` DISABLE KEYS */;
LOCK TABLES `guild_charter` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild_charter` ENABLE KEYS */;


--
-- Table structure for table `guild_charter_sign`
--

DROP TABLE IF EXISTS `guild_charter_sign`;
CREATE TABLE `guild_charter_sign` (
  `ownerguid` int(10) unsigned NOT NULL,
  `charterguid` int(11) unsigned default '0', 
  `playerguid` int(11) unsigned default '0',
  PRIMARY KEY (`charterguid`,`playerguid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild_charter_sign`
--


/*!40000 ALTER TABLE `guild_charter_sign` DISABLE KEYS */;
LOCK TABLES `guild_charter_sign` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild_charter_sign` ENABLE KEYS */;


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
  `rid` int(11) unsigned NOT NULL,
  `rname` varchar(255) NOT NULL default '',
  `rights` int(3) unsigned NOT NULL default '0',
  PRIMARY KEY (`guildid`,`rid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Guild System';

--
-- Dumping data for table `guild_rank`
--


/*!40000 ALTER TABLE `guild_rank` DISABLE KEYS */;
LOCK TABLES `guild_rank` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `guild_rank` ENABLE KEYS */;

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
-- Table structure for table `item_loot_template`
--

DROP TABLE IF EXISTS `item_loot_template`;
CREATE TABLE `item_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `item_loot_template`
--


/*!40000 ALTER TABLE `item_loot_template` DISABLE KEYS */;
LOCK TABLES `item_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_loot_template` ENABLE KEYS */;

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
  `BuyCount` tinyint(3) unsigned NOT NULL default '1',
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
  `Unknown1` int(30) unsigned NOT NULL default '0',
  `ScriptName` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`entry`),
  KEY `items_index` (`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Item System';

--
-- Dumping data for table `item_template`
--


/*!40000 ALTER TABLE `item_template` DISABLE KEYS */;
LOCK TABLES `item_template` WRITE;
INSERT INTO `item_template` VALUES (65020,0,0,'Tough Jerky','Tough Jerky','Tough Jerky','Tough Jerky',2473,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65021,0,0,'Refreshing Spring Water','Refreshing Spring Water','Refreshing Spring Water','Refreshing Spring Water',18084,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,430,0,0,0,59,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65022,0,0,'Darnassian Bleu','Darnassian Bleu','Darnassian Bleu','Darnassian Bleu',6353,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65023,2,16,'Small Throwing Knife','Small Throwing Knife','Small Throwing Knife','Small Throwing Knife',16754,1,0,1,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,0,'internalItemHandler'),(65024,2,16,'Crude Throwing Axe','Crude Throwing Axe','Crude Throwing Axe','Crude Throwing Axe',20777,1,0,1,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,0,'internalItemHandler'),(65025,0,0,'Shiny Red Apple','Shiny Red Apple','Shiny Red Apple','Shiny Red Apple',6410,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65026,0,0,'Tough Hunk of Bread','Tough Hunk of Bread','Tough Hunk of Bread','Tough Hunk of Bread',6399,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(65027,0,0,'Forest Mushroom Cap','Forest Mushroom Cap','Forest Mushroom Cap','Forest Mushroom Cap',15852,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,0,0,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(25,2,7,'Worn Shortsword','Worn Shortsword','Worn Shortsword','Worn Shortsword',1542,1,0,1,35,7,21,32767,511,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,1,0,0,0,1,3,0,0,0,20,0,0,'internalItemHandler'),(39,4,1,'Recruit\'s Pants','Recruit\'s Pants','Recruit\'s Pants','Recruit\'s Pants',9892,0,0,1,5,1,7,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(40,4,0,'Recruit\'s Boots','Recruit\'s Boots','Recruit\'s Boots','Recruit\'s Boots',10141,1,0,1,4,1,8,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(38,4,0,'Recruit\'s Shirt','Recruit\'s Shirt','Recruit\'s Shirt','Recruit\'s Shirt',9891,1,0,1,1,1,4,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(2362,4,6,'Worn Wooden Shield','Worn Wooden Shield','Worn Wooden Shield','Worn Wooden Shield',18730,0,0,1,7,1,14,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,4,0,1,0,20,0,0,'internalItemHandler'),(6948,15,0,'Hearthstone','Hearthstone','Hearthstone','Hearthstone',6418,1,64,1,0,0,0,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14646,12,0,'Northshire Gift Voucher','Northshire Gift Voucher','Northshire Gift Voucher','Northshire Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5805,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14647,12,0,'Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher','Coldridge Valley Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5841,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14648,12,0,'Shadowglen Gift Voucher','Shadowglen Gift Voucher','Shadowglen Gift Voucher','Shadowglen Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5842,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14649,12,0,'Valley of Trials Gift Voucher','Valley of Trials Gift Voucher','Valley of Trials Gift Voucher','Valley of Trials Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5843,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14650,12,0,'Camp Narache Gift Voucher','Camp Narache Gift Voucher','Camp Narache Gift Voucher','Camp Narache Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5844,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(14651,12,0,'Deathknell Gift Voucher','Deathknell Gift Voucher','Deathknell Gift Voucher','Deathknell Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5847,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(43,4,0,'Squire\'s Boots','Squire\'s Boots','Squire\'s Boots','Squire\'s Boots',10272,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(44,4,1,'Squire\'s Pants','Squire\'s Pants','Squire\'s Pants','Squire\'s Pants',9937,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(45,4,0,'Squire\'s Shirt','Squire\'s Shirt','Squire\'s Shirt','Squire\'s Shirt',3265,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(47,4,0,'Footpad\'s Shoes','Footpad\'s Shoes','Footpad\'s Shoes','Footpad\'s Shoes',9915,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(48,4,1,'Footpad\'s Pants','Footpad\'s Pants','Footpad\'s Pants','Footpad\'s Pants',9913,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(49,4,0,'Footpad\'s Shirt','Footpad\'s Shirt','Footpad\'s Shirt','Footpad\'s Shirt',9906,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(51,4,0,'Neophyte\'s Boots','Neophyte\'s Boots','Neophyte\'s Boots','Neophyte\'s Boots',9946,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(52,4,1,'Neophyte\'s Pants','Neophyte\'s Pants','Neophyte\'s Pants','Neophyte\'s Pants',9945,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(53,4,0,'Neophyte\'s Shirt','Neophyte\'s Shirt','Neophyte\'s Shirt','Neophyte\'s Shirt',9944,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(55,4,0,'Apprentice\'s Boots','Apprentice\'s Boots','Apprentice\'s Boots','Apprentice\'s Boots',9929,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(56,4,1,'Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe',12647,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(57,4,1,'Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe',12645,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(59,4,0,'Acolyte\'s Shoes','Acolyte\'s Shoes','Acolyte\'s Shoes','Acolyte\'s Shoes',3261,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(35,2,10,'Bent Staff','Bent Staff','Bent Staff','Bent Staff',472,1,0,1,47,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,0,'internalItemHandler'),(36,2,4,'Worn Mace','Worn Mace','Worn Mace','Worn Mace',5194,1,0,1,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,3,0,0,0,20,0,0,'internalItemHandler'),(37,2,0,'Worn Axe','Worn Axe','Worn Axe','Worn Axe',14029,1,0,1,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,20,0,0,'internalItemHandler'),(2361,2,5,'Battleworn Hammer','Battleworn Hammer','Battleworn Hammer','Battleworn Hammer',8690,1,0,1,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,1,0,0,0,25,0,0,'internalItemHandler'),(2092,2,15,'Worn Dagger','Worn Dagger','Worn Dagger','Worn Dagger',6442,1,0,1,35,7,13,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1600,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,16,0,0,'internalItemHandler'),(6096,4,0,'Apprentice\'s Shirt','Apprentice\'s Shirt','Apprentice\'s Shirt','Apprentice\'s Shirt',2163,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(6097,4,0,'Acolyte\'s Shirt','Acolyte\'s Shirt','Acolyte\'s Shirt','Acolyte\'s Shirt',2470,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(6098,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12679,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(1395,4,1,'Apprentice\'s Pants','Apprentice\'s Pants','Apprentice\'s Pants','Apprentice\'s Pants',9924,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(1396,4,1,'Acolyte\'s Pants','Acolyte\'s Pants','Acolyte\'s Pants','Acolyte\'s Pants',3260,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(6125,4,0,'Brawler\'s Harness','Brawler\'s Harness','Brawler\'s Harness','Brawler\'s Harness',9995,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(6126,4,1,'Trapper\'s Pants','Trapper\'s Pants','Trapper\'s Pants','Trapper\'s Pants',10002,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(6127,4,0,'Trapper\'s Boots','Trapper\'s Boots','Trapper\'s Boots','Trapper\'s Boots',10003,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(6129,4,1,'Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe','Acolyte\'s Robe',12646,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(139,4,1,'Brawler\'s Pants','Brawler\'s Pants','Brawler\'s Pants','Brawler\'s Pants',9988,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(140,4,0,'Brawler\'s Boots','Brawler\'s Boots','Brawler\'s Boots','Brawler\'s Boots',9992,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(127,4,0,'Trapper\'s Shirt','Trapper\'s Shirt','Trapper\'s Shirt','Trapper\'s Shirt',9996,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(120,4,1,'Thug Pants','Thug Pants','Thug Pants','Thug Pants',10006,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(121,4,0,'Thug Boots','Thug Boots','Thug Boots','Thug Boots',10008,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(147,4,1,'Rugged Trapper\'s Pants','Rugged Trapper\'s Pants','Rugged Trapper\'s Pants','Rugged Trapper\'s Pants',9975,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(148,4,0,'Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt','Rugged Trapper\'s Shirt',9976,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(153,4,2,'Primitive Kilt','Primitive Kilt','Primitive Kilt','Primitive Kilt',10050,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,0,'internalItemHandler'),(154,4,0,'Primitive Mantle','Primitive Mantle','Primitive Mantle','Primitive Mantle',10058,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(2101,1,2,'Light Quiver','Light Quiver','Light Quiver','Light Quiver',21328,1,0,1,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(2102,1,3,'Small Ammo Pouch','Small Ammo Pouch','Small Ammo Pouch','Small Ammo Pouch',1816,1,0,1,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,'internalItemHandler'),(2105,4,0,'Thug Shirt','Thug Shirt','Thug Shirt','Thug Shirt',10005,1,0,1,5,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(2504,2,2,'Worn Shortbow','Worn Shortbow','Worn Shortbow','Worn Shortbow',8106,1,0,1,29,5,15,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,20,0,0,'internalItemHandler'),(2508,2,3,'Old Blunderbuss','Old Blunderbuss','Old Blunderbuss','Old Blunderbuss',6606,1,0,1,27,5,26,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,20,0,0,'internalItemHandler'),(2512,6,2,'Rough Arrow','Rough Arrow','Rough Arrow','Rough Arrow',5996,1,0,1,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,0,'internalItemHandler'),(2516,6,3,'Light Shot','Light Shot','Light Shot','Light Shot',5998,1,0,1,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,0,'internalItemHandler'),(3661,2,10,'Handcrafted Staff','Handcrafted Staff','Handcrafted Staff','Handcrafted Staff',18530,1,0,1,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,0,'internalItemHandler'),(6119,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12681,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(6123,4,1,'Novice\'s Robe','Novice\'s Robe','Novice\'s Robe','Novice\'s Robe',12683,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(6124,4,1,'Novice\'s Pants','Novice\'s Pants','Novice\'s Pants','Novice\'s Pants',9987,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,'internalItemHandler'),(6134,4,0,'Primitive Mantle','Primitive Mantle','Primitive Mantle','Primitive Mantle',10108,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,'internalItemHandler'),(6135,4,2,'Primitive Kilt','Primitive Kilt','Primitive Kilt','Primitive Kilt',10109,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,0,'internalItemHandler'),(6139,4,1,'Novice\'s Robe','Novice\'s Robe','Novice\'s Robe','Novice\'s Robe',12684,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(6140,4,1,'Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe','Apprentice\'s Robe',12649,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(6144,4,1,'Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe','Neophyte\'s Robe',12680,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,'internalItemHandler'),(12282,2,1,'Worn Battleaxe','Worn Battleaxe','Worn Battleaxe','Worn Battleaxe',22291,1,0,1,43,8,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,1,0,0,0,25,0,0,'internalItemHandler');
UNLOCK TABLES;
/*!40000 ALTER TABLE `item_template` ENABLE KEYS */;

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
  `item_template` int(11) unsigned NOT NULL default '0' COMMENT 'Item Identifier',
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
  `entry` int(11) NOT NULL default '0',
  `spell` int(11) NOT NULL default '0',
  `spellcost` int(11) default '0',
  `reqspell` int(11) unsigned default '0',
  `reqskill` int(11) unsigned default '0',
  `reqskillvalue` int(11) unsigned default '0',
  `reqlevel` int(11) unsigned default '0',
  UNIQUE KEY `entry_spell` (`entry`,`spell`)
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
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '0',
  `incrtime` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Npc System';

--
-- Dumping data for table `npc_vendor`
--


/*!40000 ALTER TABLE `npc_vendor` DISABLE KEYS */;
LOCK TABLES `npc_vendor` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `npc_vendor` ENABLE KEYS */;

--
-- Table structure for table `pickpocketing_loot_template`
--

DROP TABLE IF EXISTS `pickpocketing_loot_template`;
CREATE TABLE `pickpocketing_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `pickpocketing_loot_template`
--


/*!40000 ALTER TABLE `pickpocketing_loot_template` DISABLE KEYS */;
LOCK TABLES `pickpocketing_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `pickpocketing_loot_template` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo`
--

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE `playercreateinfo` (
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
  PRIMARY KEY `playercreateinfo_race_class_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo`
--


/*!40000 ALTER TABLE `playercreateinfo` DISABLE KEYS */;
LOCK TABLES `playercreateinfo` WRITE;
INSERT INTO `playercreateinfo` VALUES (1,1,0,12,-8949,-132,84,49,23,20,22,20,21,40,60,0,1000,0,0),(1,2,0,12,-8949,-132,84,49,22,20,22,20,22,40,68,79,0,0,0),(1,4,0,12,-8949,-132,84,49,21,23,21,20,20,40,55,0,0,0,100),(1,5,0,12,-8949,-132,84,49,20,20,20,22,24,40,61,128,0,0,0),(1,8,0,12,-8949,-132,84,49,20,20,20,23,22,40,61,119,0,0,0),(1,9,0,12,-8949,-132,84,49,20,20,21,22,22,40,53,109,0,0,0),(2,1,1,14,-618,-4251,39,51,26,17,24,17,23,0,80,0,1000,0,0),(2,3,1,14,-618,-4251,39,51,23,20,23,17,24,0,76,80,0,0,0),(2,4,1,14,-618,-4251,39,51,24,20,23,17,23,0,75,0,0,0,100),(2,7,1,14,-618,-4251,39,51,24,17,23,18,25,0,97,71,0,0,0),(2,9,1,14,-618,-4251,39,51,23,17,23,19,25,0,73,109,0,0,0),(3,1,0,1,-6240,331,383,53,25,16,25,19,19,0,90,0,1000,0,0),(3,2,0,1,-6240,331,383,53,24,16,25,19,20,0,88,79,0,0,0),(3,3,0,1,-6240,331,383,53,22,19,24,19,20,0,86,80,0,0,0),(3,4,0,1,-6240,331,383,53,23,19,24,19,19,0,85,0,0,0,100),(3,5,0,1,-6240,331,383,53,22,16,23,21,22,0,91,128,0,0,0),(4,1,1,141,10311,832,1327,55,20,25,21,20,20,0,50,0,1000,0,0),(4,3,1,141,10311,832,1327,55,17,28,20,20,21,0,46,80,0,0,0),(4,4,1,141,10311,832,1327,55,18,28,20,20,20,0,45,0,0,0,100),(4,5,1,141,10311,832,1327,55,17,25,19,22,23,0,51,128,0,0,0),(4,11,1,141,10311,832,1327,55,18,25,19,22,22,0,53,67,0,0,0),(5,1,0,85,1676,1677,122,57,22,18,23,18,25,0,70,0,1000,0,0),(5,4,0,85,1676,1677,122,57,20,21,22,18,25,0,65,0,0,0,100),(5,5,0,85,1676,1677,122,57,19,18,21,20,28,0,71,128,0,0,0),(5,8,0,85,1676,1677,122,57,19,18,21,21,27,0,71,119,0,0,0),(5,9,0,85,1676,1677,122,57,19,18,22,20,27,0,63,109,0,0,0),(6,1,1,215,-2917,-257,53,59,28,15,24,15,22,0,80,0,1000,0,0),(6,3,1,215,-2917,-257,53,59,25,18,23,15,23,0,76,80,0,0,0),(6,7,1,215,-2917,-257,53,59,26,15,23,16,24,0,97,71,0,0,0),(6,11,1,215,-2917,-257,53,59,26,15,22,17,24,0,97,67,0,0,0),(7,1,0,1,-6240,331,383,1563,18,23,21,23,20,0,50,0,1000,0,0),(7,4,0,1,-6340,331,383,1563,16,26,20,23,20,0,45,0,0,0,100),(7,8,0,1,-6340,331,383,1563,15,23,19,26,22,0,51,119,0,0,0),(7,9,0,1,-6340,331,383,1563,15,23,20,25,22,0,43,109,0,0,0),(8,1,1,14,-618,-4251,39,1478,24,22,23,16,21,0,70,0,1000,0,0),(8,3,1,14,-618,-4251,39,1478,21,25,22,16,22,0,66,80,0,0,0),(8,4,1,14,-618,-4251,39,1478,22,25,22,16,21,0,65,0,0,0,100),(8,5,1,14,-618,-4251,39,1478,21,22,21,18,24,0,71,128,0,0,0),(8,7,1,14,-618,-4251,39,1478,22,22,22,17,23,0,87,71,0,0,0),(8,8,1,14,-618,-4251,39,1478,21,22,21,19,23,0,71,119,0,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_action`
--

DROP TABLE IF EXISTS `playercreateinfo_action`;
CREATE TABLE `playercreateinfo_action` (
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `button` smallint(2) unsigned NOT NULL default '0',
  `action` smallint(6) unsigned NOT NULL default '0',
  `type` smallint(3) unsigned NOT NULL default '0',
  `misc` smallint(3) unsigned NOT NULL default '0',
  KEY `playercreateinfo_race_class_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_action`
--


/*!40000 ALTER TABLE `playercreateinfo_action` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_action` WRITE;
INSERT INTO `playercreateinfo_action` (race, class, button, action, type, misc) VALUES (1, 1, 1, 78, 0, 0),(1, 1, 0, 6603, 0, 0),(1, 1, 11, 65020, 128, 0),(1, 2, 2, 635, 0, 0),(1, 2, 0, 6603, 0, 0),(1, 2, 1, 20154, 0, 0),(1, 2, 10, 65021, 128, 0),(1, 2, 11, 65022, 128, 0),(1, 4, 1, 1752, 0, 0),(1, 4, 2, 2098, 0, 0),(1, 4, 3, 2764, 0, 0),(1, 4, 0, 6603, 0, 0),(1, 4, 11, 65022, 128, 0),(1, 5, 1, 585, 0, 0),(1, 5, 2, 2050, 0, 0),(1, 5, 0, 6603, 0, 0),(1, 5, 10, 65021, 128, 0),(1, 5, 11, 65022, 128, 0),(1, 8, 1, 133, 0, 0),(1, 8, 2, 168, 0, 0),(1, 8, 0, 6603, 0, 0),(1, 8, 10, 65021, 128, 0),(1, 8, 11, 65022, 128, 0),(1, 9, 1, 686, 0, 0),(1, 9, 2, 687, 0, 0),(1, 9, 0, 6603, 0, 0),(1, 9, 10, 65021, 128, 0),(1, 9, 11, 65027, 128, 0),(2, 1, 1, 78, 0, 0),(2, 1, 0, 6603, 0, 0),(2, 1, 11, 65020, 128, 0),(2, 3, 2, 75, 0, 0),(2, 3, 1, 2973, 0, 0),(2, 3, 0, 6603, 0, 0),(2, 3, 11, 65020, 128, 0),(2, 3, 10, 65021, 128, 0),(2, 4, 10, 0, 128, 0),(2, 4, 1, 1752, 0, 0),(2, 4, 2, 2098, 0, 0),(2, 4, 0, 6603, 0, 0),(2, 4, 11, 65020, 128, 0),(2, 7, 2, 331, 0, 0),(2, 7, 1, 403, 0, 0),(2, 7, 0, 6603, 0, 0),(2, 7, 11, 65020, 128, 0),(2, 7, 10, 65021, 128, 0),(2, 9, 1, 686, 0, 0),(2, 9, 2, 687, 0, 0),(2, 9, 0, 6603, 0, 0),(2, 9, 11, 65020, 128, 0),(2, 9, 10, 65021, 128, 0),(3, 1, 1, 78, 0, 0),(3, 1, 0, 6603, 0, 0),(3, 1, 11, 65020, 128, 0),(3, 2, 2, 635, 0, 0),(3, 2, 0, 6603, 0, 0),(3, 2, 1, 20154, 0, 0),(3, 2, 10, 65021, 128, 0),(3, 2, 11, 65026, 128, 0),(3, 3, 2, 75, 0, 0),(3, 3, 1, 2973, 0, 0),(3, 3, 0, 6603, 0, 0),(3, 3, 11, 65020, 128, 0),(3, 3, 10, 65021, 128, 0),(3, 4, 1, 1752, 0, 0),(3, 4, 2, 2098, 0, 0),(3, 4, 3, 2764, 0, 0),(3, 4, 0, 6603, 0, 0),(3, 4, 11, 65026, 128, 0),(3, 5, 1, 585, 0, 0),(3, 5, 2, 2050, 0, 0),(3, 5, 0, 6603, 0, 0),(3, 5, 10, 65021, 128, 0),(3, 5, 11, 65026, 128, 0),(4, 1, 1, 78, 0, 0),(4, 1, 0, 6603, 0, 0),(4, 1, 11, 65020, 128, 0),(4, 3, 2, 75, 0, 0),(4, 3, 1, 2973, 0, 0),(4, 3, 0, 6603, 0, 0),(4, 3, 11, 65020, 128, 0),(4, 3, 10, 65021, 128, 0),(4, 4, 1, 1752, 0, 0),(4, 4, 2, 2098, 0, 0),(4, 4, 3, 2764, 0, 0),(4, 4, 0, 6603, 0, 0),(4, 4, 11, 65026, 128, 0),(4, 5, 1, 585, 0, 0),(4, 5, 2, 2050, 0, 0),(4, 5, 0, 6603, 0, 0),(4, 5, 10, 65021, 128, 0),(4, 5, 11, 65022, 128, 0),(4, 11, 1, 5176, 0, 0),(4, 11, 2, 5185, 0, 0),(4, 11, 0, 6603, 0, 0),(4, 11, 10, 65021, 128, 0),(4, 11, 11, 65025, 128, 0),(5, 1, 11, 65027, 128, 0),(5, 1, 0, 6603, 0, 0),(5, 1, 1, 78, 0, 0),(5, 4, 11, 65027, 128, 0),(5, 4, 3, 2764, 0, 0),(5, 4, 2, 2098, 0, 0),(5, 4, 1, 1752, 0, 0),(5, 4, 0, 6603, 0, 0),(5, 5, 10, 65021, 128, 0),(5, 5, 2, 2050, 0, 0),(5, 5, 1, 585, 0, 0),(5, 5, 11, 65027, 128, 0),(5, 5, 0, 6603, 0, 0),(5, 8, 11, 65027, 128, 0),(5, 8, 10, 65021, 128, 0),(5, 8, 2, 168, 0, 0),(5, 8, 1, 133, 0, 0),(5, 8, 0, 6603, 0, 0),(5, 9, 1, 686, 0, 0),(5, 9, 10, 65021, 128, 0),(5, 9, 2, 687, 0, 0),(5, 9, 11, 65027, 128, 0),(5, 9, 0, 6603, 0, 0),(6, 1, 1, 78, 0, 0),(6, 1, 2, 20549, 0, 0),(6, 1, 11, 65026, 128, 0),(6, 1, 0, 6603, 0, 0),(6, 3, 1, 2973, 0, 0),(6, 3, 10, 65021, 128, 0),(6, 3, 2, 75, 0, 0),(6, 3, 3, 20549, 0, 0),(6, 3, 11, 65020, 128, 0),(6, 3, 0, 6603, 0, 0),(6, 7, 1, 403, 0, 0),(6, 7, 10, 65021, 128, 0),(6, 7, 2, 331, 0, 0),(6, 7, 3, 20549, 0, 0),(6, 7, 11, 65027, 128, 0),(6, 7, 0, 6603, 0, 0),(6, 11, 1, 5176, 0, 0),(6, 11, 10, 65021, 128, 0),(6, 11, 2, 5185, 0, 0),(6, 11, 3, 20549, 0, 0),(6, 11, 11, 65025, 128, 0),(6, 11, 0, 6603, 0, 0),(7, 1, 11, 65020, 128, 0),(7, 1, 1, 78, 0, 0),(7, 1, 0, 6603, 0, 0),(7, 4, 11, 65020, 128, 0),(7, 4, 3, 2764, 0, 0),(7, 4, 1, 1752, 0, 0),(7, 4, 2, 2098, 0, 0),(7, 4, 0, 6603, 0, 0),(7, 8, 11, 65025, 128, 0),(7, 8, 1, 133, 0, 0),(7, 8, 2, 168, 0, 0),(7, 8, 10, 65021, 128, 0),(7, 8, 0, 6603, 0, 0),(7, 9, 11, 65027, 128, 0),(7, 9, 1, 686, 0, 0),(7, 9, 2, 687, 0, 0),(7, 9, 10, 65021, 128, 0),(7, 9, 0, 6603, 0, 0),(8, 1, 11, 65020, 128, 0),(8, 1, 1, 78, 0, 0),(8, 1, 3, 2764, 0, 0),(8, 1, 0, 6603, 0, 0),(8, 3, 10, 65021, 128, 0),(8, 3, 11, 65027, 128, 0),(8, 3, 1, 2973, 0, 0),(8, 3, 2, 75, 0, 0),(8, 3, 0, 6603, 0, 0),(8, 4, 1, 1752, 0, 0),(8, 4, 3, 2764, 0, 0),(8, 4, 2, 2098, 0, 0),(8, 4, 11, 65020, 128, 0),(8, 4, 0, 6603, 0, 0),(8, 5, 1, 585, 0, 0),(8, 5, 10, 65021, 128, 0),(8, 5, 2, 2050, 0, 0),(8, 5, 11, 65026, 128, 0),(8, 5, 0, 6603, 0, 0),(8, 7, 1, 403, 0, 0),(8, 7, 10, 65021, 128, 0),(8, 7, 2, 331, 0, 0),(8, 7, 11, 65020, 128, 0),(8, 7, 0, 6603, 0, 0),(8, 8, 1, 133, 0, 0),(8, 8, 10, 65021, 128, 0),(8, 8, 2, 168, 0, 0),(8, 8, 11, 65020, 128, 0),(8, 8, 0, 6603, 0, 0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_action` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_item`
--

DROP TABLE IF EXISTS `playercreateinfo_item`;
CREATE TABLE `playercreateinfo_item` (
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `itemid` mediumint(8) unsigned NOT NULL default '0',
  `amount` tinyint(8) unsigned NOT NULL default '1',
  KEY `playercreateinfo_race_class_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_item`
--


/*!40000 ALTER TABLE `playercreateinfo_item` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_item` WRITE;
INSERT INTO `playercreateinfo_item` (race, class, itemid, amount) VALUES (1, 1, 38, 1),(1, 1, 39, 1),(1, 1, 40, 1),(1, 1, 25, 1),(1, 1, 2362, 1),(1, 1, 65020, 4),(1, 1, 6948, 1),(1, 1, 14646, 1),(1, 2, 45, 1),(1, 2, 44, 1),(1, 2, 43, 1),(1, 2, 2361, 1),(1, 2, 6948, 1),(1, 2, 65021, 2),(1, 2, 65022, 4),(1, 2, 14646, 1),(1, 4, 49, 1),(1, 4, 48, 1),(1, 4, 47, 1),(1, 4, 2092, 1),(1, 4, 65023, 100),(1, 4, 65022, 4),(1, 4, 6948, 1),(1, 4, 14646, 1),(1, 5, 53, 1),(1, 5, 6098, 1),(1, 5, 52, 1),(1, 5, 51, 1),(1, 5, 36, 1),(1, 5, 65021, 2),(1, 5, 65022, 4),(1, 5, 6948, 1),(1, 5, 14646, 1),(1, 8, 6096, 1),(1, 8, 56, 1),(1, 8, 1395, 1),(1, 8, 55, 1),(1, 8, 35, 1),(1, 8, 65022, 4),(1, 8, 65021, 2),(1, 8, 6948, 1),(1, 8, 14646, 1),(1, 9, 6097, 1),(1, 9, 57, 1),(1, 9, 1396, 1),(1, 9, 59, 1),(1, 9, 2092, 1),(1, 9, 65027, 4),(1, 9, 65021, 2),(1, 9, 6948, 1),(1, 9, 14646, 1),(2, 1, 6125, 1),(2, 1, 139, 1),(2, 1, 140, 1),(2, 1, 12282, 1),(2, 1, 6948, 1),(2, 1, 65020, 4),(2, 1, 14649, 1),(2, 3, 127, 1),(2, 3, 6126, 1),(2, 3, 6127, 1),(2, 3, 37, 1),(2, 3, 2504, 1),(2, 3, 65021, 2),(2, 3, 65020, 4),(2, 3, 6948, 1),(2, 3, 14649, 1),(2, 3, 2512, 200),(2, 3, 2101, 1),(2, 4, 2105, 1),(2, 4, 120, 1),(2, 4, 121, 1),(2, 4, 2092, 1),(2, 4, 65024, 100),(2, 4, 65020, 4),(2, 4, 6948, 1),(2, 4, 14649, 1),(2, 7, 154, 1),(2, 7, 153, 1),(2, 7, 36, 1),(2, 7, 6948, 1),(2, 7, 65020, 4),(2, 7, 65021, 2),(2, 7, 14649, 1),(2, 9, 6129, 1),(2, 9, 1396, 1),(2, 9, 59, 1),(2, 9, 2092, 1),(2, 9, 6948, 1),(2, 9, 65020, 4),(2, 9, 65021, 2),(2, 9, 14649, 1),(3, 1, 38, 1),(3, 1, 39, 1),(3, 1, 40, 1),(3, 1, 12282, 1),(3, 1, 6948, 1),(3, 1, 65020, 4),(3, 1, 14647, 1),(3, 2, 45, 1),(3, 2, 44, 1),(3, 2, 43, 1),(3, 2, 2361, 1),(3, 2, 65026, 4),(3, 2, 65021, 2),(3, 2, 6948, 1),(3, 2, 14647, 1),(3, 3, 148, 1),(3, 3, 147, 1),(3, 3, 129, 1),(3, 3, 37, 1),(3, 3, 2508, 1),(3, 3, 65021, 2),(3, 3, 65020, 4),(3, 3, 6948, 1),(3, 3, 14647, 1),(3, 3, 2516, 200),(3, 3, 2102, 1),(3, 4, 49, 1),(3, 4, 48, 1),(3, 4, 47, 1),(3, 4, 2092, 1),(3, 4, 65024, 100),(3, 4, 65026, 4),(3, 4, 6948, 1),(3, 4, 14647, 1),(3, 5, 53, 1),(3, 5, 6098, 1),(3, 5, 52, 1),(3, 5, 51, 1),(3, 5, 36, 1),(3, 5, 65021, 2),(3, 5, 65026, 4),(3, 5, 6948, 1),(3, 5, 14647, 1),(4, 1, 38, 1),(4, 1, 39, 1),(4, 1, 40, 1),(4, 1, 25, 1),(4, 1, 2362, 1),(4, 1, 65020, 4),(4, 1, 6948, 1),(4, 1, 14648, 1),(4, 3, 148, 1),(4, 3, 147, 1),(4, 3, 129, 1),(4, 3, 2092, 1),(4, 3, 2504, 1),(4, 3, 65021, 2),(4, 3, 65020, 4),(4, 3, 6948, 1),(4, 3, 14648, 1),(4, 3, 2512, 200),(4, 3, 2101, 1),(4, 4, 49, 1),(4, 4, 48, 1),(4, 4, 47, 1),(4, 4, 2092, 1),(4, 4, 65023, 100),(4, 4, 65026, 4),(4, 4, 6948, 1),(4, 4, 14648, 1),(4, 5, 53, 1),(4, 5, 6119, 1),(4, 5, 52, 1),(4, 5, 51, 1),(4, 5, 36, 1),(4, 5, 65022, 4),(4, 5, 65021, 2),(4, 5, 6948, 1),(4, 5, 14648, 1),(4, 11, 6123, 1),(4, 11, 44, 1),(4, 11, 3661, 1),(4, 11, 65021, 2),(4, 11, 65025, 4),(4, 11, 6948, 1),(4, 11, 14648, 1),(5, 1, 6125, 1),(5, 1, 139, 1),(5, 1, 140, 1),(5, 1, 25, 1),(5, 1, 2362, 1),(5, 1, 65027, 4),(5, 1, 6948, 1),(5, 1, 14651, 1),(5, 4, 2105, 1),(5, 4, 120, 1),(5, 4, 121, 1),(5, 4, 2092, 1),(5, 4, 65023, 100),(5, 4, 65027, 4),(5, 4, 6948, 1),(5, 4, 14651, 1),(5, 5, 53, 1),(5, 5, 6144, 1),(5, 5, 52, 1),(5, 5, 51, 1),(5, 5, 36, 1),(5, 5, 65027, 4),(5, 5, 65021, 2),(5, 5, 6948, 1),(5, 5, 14651, 1),(5, 8, 6096, 1),(5, 8, 6140, 1),(5, 8, 1395, 1),(5, 8, 55, 1),(5, 8, 35, 1),(5, 8, 65027, 4),(5, 8, 65021, 2),(5, 8, 6948, 1),(5, 8, 14651, 1),(5, 9, 6129, 1),(5, 9, 1396, 1),(5, 9, 59, 1),(5, 9, 2092, 1),(5, 9, 65027, 4),(5, 9, 65021, 2),(5, 9, 6948, 1),(5, 9, 14651, 1),(6, 1, 6125, 1),(6, 1, 139, 1),(6, 1, 2361, 1),(6, 1, 6948, 1),(6, 1, 65026, 4),(6, 1, 14650, 1),(6, 3, 127, 1),(6, 3, 6126, 1),(6, 3, 37, 1),(6, 3, 2508, 1),(6, 3, 65021, 2),(6, 3, 65020, 4),(6, 3, 6948, 1),(6, 3, 14650, 1),(6, 3, 2516, 200),(6, 3, 2102, 1),(6, 7, 154, 1),(6, 7, 153, 1),(6, 7, 36, 1),(6, 7, 6948, 1),(6, 7, 65027, 4),(6, 7, 65021, 2),(6, 7, 14650, 1),(6, 11, 6139, 1),(6, 11, 6124, 1),(6, 11, 35, 1),(6, 11, 65021, 2),(6, 11, 65025, 4),(6, 11, 6948, 1),(6, 11, 14650, 1),(7, 1, 38, 1),(7, 1, 39, 1),(7, 1, 40, 1),(7, 1, 25, 1),(7, 1, 2362, 1),(7, 1, 65020, 4),(7, 1, 6948, 1),(7, 1, 14647, 1),(7, 4, 49, 1),(7, 4, 48, 1),(7, 4, 47, 1),(7, 4, 2092, 1),(7, 4, 65023, 100),(7, 4, 65020, 4),(7, 4, 6948, 1),(7, 4, 14647, 1),(7, 8, 6096, 1),(7, 8, 56, 1),(7, 8, 1395, 1),(7, 8, 55, 1),(7, 8, 35, 1),(7, 8, 65025, 4),(7, 8, 65021, 2),(7, 8, 6948, 1),(7, 8, 14647, 1),(7, 9, 6097, 1),(7, 9, 57, 1),(7, 9, 1396, 1),(7, 9, 59, 1),(7, 9, 2092, 1),(7, 9, 65021, 2),(7, 9, 65027, 4),(7, 9, 6948, 1),(7, 9, 14647, 1),(8, 1, 6125, 1),(8, 1, 139, 1),(8, 1, 140, 1),(8, 1, 37, 1),(8, 1, 2362, 1),(8, 1, 65024, 100),(8, 1, 65020, 4),(8, 1, 6948, 1),(8, 1, 14649, 1),(8, 3, 127, 1),(8, 3, 6126, 1),(8, 3, 6127, 1),(8, 3, 37, 1),(8, 3, 2504, 1),(8, 3, 65027, 4),(8, 3, 65021, 2),(8, 3, 2512, 200),(8, 3, 2101, 1),(8, 3, 14649, 1),(8, 3, 6948, 1),(8, 4, 2105, 1),(8, 4, 120, 1),(8, 4, 121, 1),(8, 4, 2092, 1),(8, 4, 65024, 100),(8, 4, 65020, 4),(8, 4, 6948, 1),(8, 4, 14649, 1),(8, 5, 53, 1),(8, 5, 6144, 1),(8, 5, 52, 1),(8, 5, 36, 1),(8, 5, 65026, 4),(8, 5, 65021, 2),(8, 5, 6948, 1),(8, 5, 14649, 1),(8, 7, 6134, 1),(8, 7, 6135, 1),(8, 7, 36, 1),(8, 7, 65020, 4),(8, 7, 65021, 2),(8, 7, 6948, 1),(8, 7, 14649, 1),(8, 8, 6096, 1),(8, 8, 6140, 1),(8, 8, 1395, 1),(8, 8, 55, 1),(8, 8, 35, 1),(8, 8, 65020, 4),(8, 8, 65021, 2),(8, 8, 6948, 1),(8, 8, 14649, 1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_item` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_reputation`
--

DROP TABLE IF EXISTS `playercreateinfo_reputation`;
CREATE TABLE `playercreateinfo_reputation` (
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `slot` smallint(2) unsigned NOT NULL default '0',
  `faction` smallint(6) unsigned NOT NULL default '0',
  `reputation` smallint(3) unsigned NOT NULL default '0',
  KEY `playercreateinfo_race_class_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_reputation`
--


/*!40000 ALTER TABLE `playercreateinfo_reputation` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_reputation` WRITE;
INSERT INTO `playercreateinfo_reputation` (race, class, slot, faction, reputation) VALUES (1, 1, 0, 0, 0),(1, 2, 0, 0, 0),(1, 4, 0, 0, 0),(1, 5, 0, 0, 0),(1, 8, 0, 0, 0),(1, 9, 0, 0, 0),(2, 1, 0, 0, 0),(2, 3, 0, 0, 0),(2, 4, 0, 0, 0),(2, 7, 0, 0, 0),(3, 4, 0, 0, 0),(2, 9, 0, 0, 0),(3, 1, 0, 0, 0),(3, 5, 0, 0, 0),(4, 1, 0, 0, 0),(4, 3, 0, 0, 0),(4, 4, 0, 0, 0),(4, 5, 0, 0, 0),(4, 11, 0, 0, 0),(5, 1, 0, 0, 0),(5, 4, 0, 0, 0),(5, 5, 0, 0, 0),(5, 8, 0, 0, 0),(5, 9, 0, 0, 0),(6, 1, 0, 0, 0),(6, 3, 0, 0, 0),(6, 7, 0, 0, 0),(6, 11, 0, 0, 0),(7, 1, 0, 0, 0),(7, 4, 0, 0, 0),(7, 9, 0, 0, 0),(8, 1, 0, 0, 0),(8, 3, 0, 0, 0),(8, 4, 0, 0, 0),(8, 5, 0, 0, 0),(8, 7, 0, 0, 0),(8, 8, 0, 0, 0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_reputation` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_skill`
--

DROP TABLE IF EXISTS `playercreateinfo_skill`;
CREATE TABLE `playercreateinfo_skill` (
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `Skill` mediumint(8) unsigned NOT NULL default '0',
  `SkillMin` smallint(5) unsigned NOT NULL default '0',
  `SkillMax` smallint(5) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL,
  PRIMARY KEY  (`race`,`class`,`Skill`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_skill`
--


/*!40000 ALTER TABLE `playercreateinfo_skill` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_skill` WRITE;
INSERT INTO `playercreateinfo_skill` (race, class, Skill, SkillMin, SkillMax, Note) VALUES (1, 1, 148, 0, 0, 'Horse Riding'),(1, 1, 150, 0, 0, 'Tiger Riding'),(1, 1, 164, 0, 0, 'Blacksmithing'),(1, 1, 172, 0, 5, 'Two-Handed Axes'),(1, 1, 173, 0, 5, 'Daggers'),(1, 1, 183, 5, 5, 'GENERIC (DND)'),(1, 1, 197, 0, 0, 'Tailoring'),(1, 1, 226, 0, 5, 'Crossbows'),(1, 1, 754, 5, 5, 'Racial - Human'),(1, 1, 393, 0, 0, 'Skinning'),(1, 1, 109, 0, 0, 'Language: Orcish'),(1, 1, 433, 1, 1, 'Shield'),(1, 1, 137, 0, 0, 'Language: Thalassian'),(1, 1, 473, 0, 1, 'Fist Weapons'),(1, 1, 140, 0, 0, 'Language: Titan'),(1, 1, 149, 0, 0, 'Wolf Riding'),(1, 1, 229, 0, 0, 'Polearms'),(1, 1, 257, 0, 5, 'Protection'),(1, 1, 293, 0, 0, 'Plate Mail'),(1, 1, 98, 300, 300, 'Language: Common'),(1, 1, 315, 0, 0, 'Language: Troll'),(1, 1, 139, 0, 0, 'Language: Demon Tongue'),(1, 1, 152, 0, 0, 'Ram Riding'),(1, 1, 165, 0, 0, 'Leatherworking'),(1, 1, 415, 1, 1, 'Cloth'),(1, 1, 256, 0, 5, 'Fury'),(1, 1, 673, 0, 0, 'Language: Gutterspeak'),(1, 1, 95, 1, 5, 'Defense'),(1, 1, 333, 0, 0, 'Enchanting'),(1, 1, 186, 0, 0, 'Mining'),(1, 1, 43, 1, 5, 'Swords'),(1, 1, 55, 0, 5, 'Two-Handed Swords'),(1, 1, 713, 0, 0, 'Kodo Riding'),(1, 1, 176, 0, 5, 'Thrown'),(1, 1, 26, 5, 5, 'Arms'),(1, 1, 44, 1, 5, 'Axes'),(1, 1, 111, 0, 0, 'Language: Dwarven'),(1, 1, 113, 0, 0, 'Language: Darnassian'),(1, 1, 115, 0, 0, 'Language: Taurahe'),(1, 1, 118, 0, 0, 'Dual Wield'),(1, 1, 136, 0, 5, 'Staves'),(1, 1, 138, 0, 0, 'Language: Draconic'),(1, 1, 141, 0, 0, 'Language: Old Tongue'),(1, 1, 142, 0, 1, 'Survival'),(1, 1, 160, 0, 5, 'Two-Handed Maces'),(1, 1, 313, 0, 0, 'Language: Gnomish'),(1, 1, 413, 1, 1, 'Mail'),(1, 1, 414, 1, 1, 'Leather'),(1, 1, 45, 0, 5, 'Bows'),(1, 1, 46, 0, 5, 'Guns'),(1, 1, 533, 0, 0, 'Raptor Riding'),(1, 1, 554, 0, 0, 'Undead Horsemanship'),(1, 1, 202, 0, 0, 'Engineering'),(1, 1, 356, 0, 0, 'Fishing'),(1, 1, 171, 0, 0, 'Alchemy'),(1, 1, 54, 1, 5, 'Maces'),(1, 1, 182, 0, 0, 'Herbalism'),(1, 1, 129, 0, 0, 'First Aid'),(1, 1, 185, 0, 0, 'Cooking'),(1, 1, 162, 1, 5, 'Unarmed'),(1, 2, 148, 0, 0, 'Horse Riding'),(1, 2, 150, 0, 0, 'Tiger Riding'),(1, 2, 164, 0, 0, 'Blacksmithing'),(1, 2, 172, 0, 5, 'Two-Handed Axes'),(1, 2, 183, 5, 5, 'GENERIC (DND)'),(1, 2, 184, 0, 5, 'Retribution'),(1, 2, 197, 0, 0, 'Tailoring'),(1, 2, 754, 5, 5, 'Racial - Human'),(1, 2, 393, 0, 0, 'Skinning'),(1, 2, 109, 0, 0, 'Language: Orcish'),(1, 2, 433, 1, 1, 'Shield'),(1, 2, 137, 0, 0, 'Language: Thalassian'),(1, 2, 140, 0, 0, 'Language: Titan'),(1, 2, 149, 0, 0, 'Wolf Riding'),(1, 2, 229, 0, 0, 'Polearms'),(1, 2, 293, 0, 0, 'Plate Mail'),(1, 2, 98, 300, 300, 'Language: Common'),(1, 2, 315, 0, 0, 'Language: Troll'),(1, 2, 139, 0, 0, 'Language: Demon Tongue'),(1, 2, 152, 0, 0, 'Ram Riding'),(1, 2, 165, 0, 0, 'Leatherworking'),(1, 2, 415, 1, 1, 'Cloth'),(1, 2, 594, 5, 5, 'Holy'),(1, 2, 673, 0, 0, 'Language: Gutterspeak'),(1, 2, 95, 1, 5, 'Defense'),(1, 2, 333, 0, 0, 'Enchanting'),(1, 2, 186, 0, 0, 'Mining'),(1, 2, 43, 0, 5, 'Swords'),(1, 2, 55, 0, 5, 'Two-Handed Swords'),(1, 2, 713, 0, 0, 'Kodo Riding'),(1, 2, 44, 0, 5, 'Axes'),(1, 2, 111, 0, 0, 'Language: Dwarven'),(1, 2, 113, 0, 0, 'Language: Darnassian'),(1, 2, 115, 0, 0, 'Language: Taurahe'),(1, 2, 138, 0, 0, 'Language: Draconic'),(1, 2, 141, 0, 0, 'Language: Old Tongue'),(1, 2, 142, 0, 1, 'Survival'),(1, 2, 160, 1, 5, 'Two-Handed Maces'),(1, 2, 313, 0, 0, 'Language: Gnomish'),(1, 2, 413, 1, 1, 'Mail'),(1, 2, 414, 1, 1, 'Leather'),(1, 2, 533, 0, 0, 'Raptor Riding'),(1, 2, 554, 0, 0, 'Undead Horsemanship'),(1, 2, 202, 0, 0, 'Engineering'),(1, 2, 356, 0, 0, 'Fishing'),(1, 2, 171, 0, 0, 'Alchemy'),(1, 2, 54, 1, 5, 'Maces'),(1, 2, 182, 0, 0, 'Herbalism'),(1, 2, 129, 0, 0, 'First Aid'),(1, 2, 185, 0, 0, 'Cooking'),(1, 2, 267, 0, 5, 'Protection'),(1, 2, 162, 1, 5, 'Unarmed'),(1, 4, 148, 0, 0, 'Horse Riding'),(1, 4, 150, 0, 0, 'Tiger Riding'),(1, 4, 164, 0, 0, 'Blacksmithing'),(1, 4, 173, 1, 5, 'Daggers'),(1, 4, 183, 5, 5, 'GENERIC (DND)'),(1, 4, 197, 0, 0, 'Tailoring'),(1, 4, 226, 0, 5, 'Crossbows'),(1, 4, 754, 5, 5, 'Racial - Human'),(1, 4, 393, 0, 0, 'Skinning'),(1, 4, 109, 0, 0, 'Language: Orcish'),(1, 4, 137, 0, 0, 'Language: Thalassian'),(1, 4, 473, 0, 1, 'Fist Weapons'),(1, 4, 140, 0, 0, 'Language: Titan'),(1, 4, 149, 0, 0, 'Wolf Riding'),(1, 4, 98, 300, 300, 'Language: Common'),(1, 4, 315, 0, 0, 'Language: Troll'),(1, 4, 139, 0, 0, 'Language: Demon Tongue'),(1, 4, 152, 0, 0, 'Ram Riding'),(1, 4, 165, 0, 0, 'Leatherworking'),(1, 4, 415, 1, 1, 'Cloth'),(1, 4, 633, 0, 5, 'Lockpicking'),(1, 4, 673, 0, 0, 'Language: Gutterspeak'),(1, 4, 95, 1, 5, 'Defense'),(1, 4, 333, 0, 0, 'Enchanting'),(1, 4, 186, 0, 0, 'Mining'),(1, 4, 43, 0, 5, 'Swords'),(1, 4, 713, 0, 0, 'Kodo Riding'),(1, 4, 176, 1, 5, 'Thrown'),(1, 4, 38, 5, 5, 'Combat'),(1, 4, 39, 0, 5, 'Subtlety'),(1, 4, 111, 0, 0, 'Language: Dwarven'),(1, 4, 113, 0, 0, 'Language: Darnassian'),(1, 4, 115, 0, 0, 'Language: Taurahe'),(1, 4, 118, 0, 0, 'Dual Wield'),(1, 4, 138, 0, 0, 'Language: Draconic'),(1, 4, 141, 0, 0, 'Language: Old Tongue'),(1, 4, 142, 0, 1, 'Survival'),(1, 4, 253, 5, 5, 'Assassination'),(1, 4, 313, 0, 0, 'Language: Gnomish'),(1, 4, 414, 1, 1, 'Leather'),(1, 4, 45, 0, 5, 'Bows'),(1, 4, 46, 0, 5, 'Guns'),(1, 4, 533, 0, 0, 'Raptor Riding'),(1, 4, 554, 0, 0, 'Undead Horsemanship'),(1, 4, 202, 0, 0, 'Engineering'),(1, 4, 356, 0, 0, 'Fishing'),(1, 4, 171, 0, 0, 'Alchemy'),(1, 4, 54, 0, 5, 'Maces'),(1, 4, 182, 0, 0, 'Herbalism'),(1, 4, 129, 0, 0, 'First Aid'),(1, 4, 185, 0, 0, 'Cooking'),(1, 4, 40, 0, 5, 'Poisons'),(1, 4, 162, 1, 5, 'Unarmed'),(1, 5, 148, 0, 0, 'Horse Riding'),(1, 5, 150, 0, 0, 'Tiger Riding'),(1, 5, 164, 0, 0, 'Blacksmithing'),(1, 5, 173, 0, 5, 'Daggers'),(1, 5, 183, 5, 5, 'GENERIC (DND)'),(1, 5, 197, 0, 0, 'Tailoring'),(1, 5, 228, 1, 5, 'Wands'),(1, 5, 754, 5, 5, 'Racial - Human'),(1, 5, 393, 0, 0, 'Skinning'),(1, 5, 109, 0, 0, 'Language: Orcish'),(1, 5, 137, 0, 0, 'Language: Thalassian'),(1, 5, 140, 0, 0, 'Language: Titan'),(1, 5, 149, 0, 0, 'Wolf Riding'),(1, 5, 98, 300, 300, 'Language: Common'),(1, 5, 315, 0, 0, 'Language: Troll'),(1, 5, 139, 0, 0, 'Language: Demon Tongue'),(1, 5, 152, 0, 0, 'Ram Riding'),(1, 5, 165, 0, 0, 'Leatherworking'),(1, 5, 415, 1, 1, 'Cloth'),(1, 5, 673, 0, 0, 'Language: Gutterspeak'),(1, 5, 95, 1, 5, 'Defense'),(1, 5, 333, 0, 0, 'Enchanting'),(1, 5, 186, 0, 0, 'Mining'),(1, 5, 713, 0, 0, 'Kodo Riding'),(1, 5, 56, 5, 5, 'Holy'),(1, 5, 78, 0, 5, 'Shadow Magic'),(1, 5, 111, 0, 0, 'Language: Dwarven'),(1, 5, 113, 0, 0, 'Language: Darnassian'),(1, 5, 115, 0, 0, 'Language: Taurahe'),(1, 5, 136, 0, 5, 'Staves'),(1, 5, 138, 0, 0, 'Language: Draconic'),(1, 5, 141, 0, 0, 'Language: Old Tongue'),(1, 5, 142, 0, 1, 'Survival'),(1, 5, 313, 0, 0, 'Language: Gnomish'),(1, 5, 613, 0, 5, 'Discipline'),(1, 5, 533, 0, 0, 'Raptor Riding'),(1, 5, 554, 0, 0, 'Undead Horsemanship'),(1, 5, 202, 0, 0, 'Engineering'),(1, 5, 356, 0, 0, 'Fishing'),(1, 5, 171, 0, 0, 'Alchemy'),(1, 5, 54, 1, 5, 'Maces'),(1, 5, 182, 0, 0, 'Herbalism'),(1, 5, 129, 0, 0, 'First Aid'),(1, 5, 185, 0, 0, 'Cooking'),(1, 5, 162, 1, 5, 'Unarmed'),(1, 8, 148, 0, 0, 'Horse Riding'),(1, 8, 150, 0, 0, 'Tiger Riding'),(1, 8, 164, 0, 0, 'Blacksmithing'),(1, 8, 173, 0, 5, 'Daggers'),(1, 8, 183, 5, 5, 'GENERIC (DND)'),(1, 8, 197, 0, 0, 'Tailoring'),(1, 8, 228, 1, 5, 'Wands'),(1, 8, 754, 5, 5, 'Racial - Human'),(1, 8, 393, 0, 0, 'Skinning'),(1, 8, 109, 0, 0, 'Language: Orcish'),(1, 8, 137, 0, 0, 'Language: Thalassian'),(1, 8, 140, 0, 0, 'Language: Titan'),(1, 8, 149, 0, 0, 'Wolf Riding'),(1, 8, 8, 5, 5, 'Fire'),(1, 8, 98, 300, 300, 'Language: Common'),(1, 8, 315, 0, 0, 'Language: Troll'),(1, 8, 139, 0, 0, 'Language: Demon Tongue'),(1, 8, 152, 0, 0, 'Ram Riding'),(1, 8, 165, 0, 0, 'Leatherworking'),(1, 8, 415, 1, 1, 'Cloth'),(1, 8, 673, 0, 0, 'Language: Gutterspeak'),(1, 8, 95, 1, 5, 'Defense'),(1, 8, 333, 0, 0, 'Enchanting'),(1, 8, 186, 0, 0, 'Mining'),(1, 8, 43, 0, 5, 'Swords'),(1, 8, 713, 0, 0, 'Kodo Riding'),(1, 8, 6, 5, 5, 'Frost'),(1, 8, 111, 0, 0, 'Language: Dwarven'),(1, 8, 113, 0, 0, 'Language: Darnassian'),(1, 8, 115, 0, 0, 'Language: Taurahe'),(1, 8, 136, 1, 5, 'Staves'),(1, 8, 138, 0, 0, 'Language: Draconic'),(1, 8, 141, 0, 0, 'Language: Old Tongue'),(1, 8, 142, 0, 1, 'Survival'),(1, 8, 313, 0, 0, 'Language: Gnomish'),(1, 8, 533, 0, 0, 'Raptor Riding'),(1, 8, 554, 0, 0, 'Undead Horsemanship'),(1, 8, 202, 0, 0, 'Engineering'),(1, 8, 356, 0, 0, 'Fishing'),(1, 8, 171, 0, 0, 'Alchemy'),(1, 8, 182, 0, 0, 'Herbalism'),(1, 8, 129, 0, 0, 'First Aid'),(1, 8, 185, 0, 0, 'Cooking'),(1, 8, 237, 0, 5, 'Arcane'),(1, 8, 162, 1, 5, 'Unarmed'),(1, 9, 148, 0, 0, 'Horse Riding'),(1, 9, 150, 0, 0, 'Tiger Riding'),(1, 9, 164, 0, 0, 'Blacksmithing'),(1, 9, 173, 1, 5, 'Daggers'),(1, 9, 183, 5, 5, 'GENERIC (DND)'),(1, 9, 197, 0, 0, 'Tailoring'),(1, 9, 228, 1, 5, 'Wands'),(1, 9, 754, 5, 5, 'Racial - Human'),(1, 9, 393, 0, 0, 'Skinning'),(1, 9, 109, 0, 0, 'Language: Orcish'),(1, 9, 137, 0, 0, 'Language: Thalassian'),(1, 9, 140, 0, 0, 'Language: Titan'),(1, 9, 149, 0, 0, 'Wolf Riding'),(1, 9, 593, 5, 5, 'Destruction'),(1, 9, 98, 300, 300, 'Language: Common'),(1, 9, 315, 0, 0, 'Language: Troll'),(1, 9, 139, 0, 0, 'Language: Demon Tongue'),(1, 9, 152, 0, 0, 'Ram Riding'),(1, 9, 354, 5, 5, 'Demonology'),(1, 9, 355, 0, 5, 'Affliction'),(1, 9, 165, 0, 0, 'Leatherworking'),(1, 9, 415, 1, 1, 'Cloth'),(1, 9, 673, 0, 0, 'Language: Gutterspeak'),(1, 9, 95, 1, 5, 'Defense'),(1, 9, 333, 0, 0, 'Enchanting'),(1, 9, 186, 0, 0, 'Mining'),(1, 9, 43, 0, 5, 'Swords'),(1, 9, 713, 0, 0, 'Kodo Riding'),(1, 9, 111, 0, 0, 'Language: Dwarven'),(1, 9, 113, 0, 0, 'Language: Darnassian'),(1, 9, 115, 0, 0, 'Language: Taurahe'),(1, 9, 136, 0, 5, 'Staves'),(1, 9, 138, 0, 0, 'Language: Draconic'),(1, 9, 141, 0, 0, 'Language: Old Tongue'),(1, 9, 142, 0, 1, 'Survival'),(1, 9, 313, 0, 0, 'Language: Gnomish'),(1, 9, 533, 0, 0, 'Raptor Riding'),(1, 9, 554, 0, 0, 'Undead Horsemanship'),(1, 9, 202, 0, 0, 'Engineering'),(1, 9, 356, 0, 0, 'Fishing'),(1, 9, 171, 0, 0, 'Alchemy'),(1, 9, 182, 0, 0, 'Herbalism'),(1, 9, 129, 0, 0, 'First Aid'),(1, 9, 185, 0, 0, 'Cooking'),(1, 9, 162, 1, 5, 'Unarmed'),(2, 1, 148, 0, 0, 'Horse Riding'),(2, 1, 150, 0, 0, 'Tiger Riding'),(2, 1, 164, 0, 0, 'Blacksmithing'),(2, 1, 172, 1, 5, 'Two-Handed Axes'),(2, 1, 173, 0, 5, 'Daggers'),(2, 1, 183, 5, 5, 'GENERIC (DND)'),(2, 1, 197, 0, 0, 'Tailoring'),(2, 1, 226, 0, 5, 'Crossbows'),(2, 1, 393, 0, 0, 'Skinning'),(2, 1, 109, 300, 300, 'Language: Orcish'),(2, 1, 433, 1, 1, 'Shield'),(2, 1, 137, 0, 0, 'Language: Thalassian'),(2, 1, 473, 0, 1, 'Fist Weapons'),(2, 1, 140, 0, 0, 'Language: Titan'),(2, 1, 149, 0, 0, 'Wolf Riding'),(2, 1, 229, 0, 0, 'Polearms'),(2, 1, 257, 0, 5, 'Protection'),(2, 1, 293, 0, 0, 'Plate Mail'),(2, 1, 98, 0, 0, 'Language: Common'),(2, 1, 125, 5, 5, 'Orc Racial'),(2, 1, 315, 0, 0, 'Language: Troll'),(2, 1, 139, 0, 0, 'Language: Demon Tongue'),(2, 1, 152, 0, 0, 'Ram Riding'),(2, 1, 165, 0, 0, 'Leatherworking'),(2, 1, 415, 1, 1, 'Cloth'),(2, 1, 256, 0, 5, 'Fury'),(2, 1, 673, 0, 0, 'Language: Gutterspeak'),(2, 1, 95, 1, 5, 'Defense'),(2, 1, 333, 0, 0, 'Enchanting'),(2, 1, 186, 0, 0, 'Mining'),(2, 1, 43, 1, 5, 'Swords'),(2, 1, 55, 0, 5, 'Two-Handed Swords'),(2, 1, 713, 0, 0, 'Kodo Riding'),(2, 1, 176, 0, 5, 'Thrown'),(2, 1, 26, 5, 5, 'Arms'),(2, 1, 44, 1, 5, 'Axes'),(2, 1, 111, 0, 0, 'Language: Dwarven'),(2, 1, 113, 0, 0, 'Language: Darnassian'),(2, 1, 115, 0, 0, 'Language: Taurahe'),(2, 1, 118, 0, 0, 'Dual Wield'),(2, 1, 136, 0, 5, 'Staves'),(2, 1, 138, 0, 0, 'Language: Draconic'),(2, 1, 141, 0, 0, 'Language: Old Tongue'),(2, 1, 142, 0, 1, 'Survival'),(2, 1, 160, 0, 5, 'Two-Handed Maces'),(2, 1, 313, 0, 0, 'Language: Gnomish'),(2, 1, 413, 1, 1, 'Mail'),(2, 1, 414, 1, 1, 'Leather'),(2, 1, 45, 0, 5, 'Bows'),(2, 1, 46, 0, 5, 'Guns'),(2, 1, 533, 0, 0, 'Raptor Riding'),(2, 1, 554, 0, 0, 'Undead Horsemanship'),(2, 1, 202, 0, 0, 'Engineering'),(2, 1, 356, 0, 0, 'Fishing'),(2, 1, 171, 0, 0, 'Alchemy'),(2, 1, 54, 0, 5, 'Maces'),(2, 1, 182, 0, 0, 'Herbalism'),(2, 1, 129, 0, 0, 'First Aid'),(2, 1, 185, 0, 0, 'Cooking'),(2, 1, 162, 1, 5, 'Unarmed'),(2, 3, 148, 0, 0, 'Horse Riding'),(2, 3, 150, 0, 0, 'Tiger Riding'),(2, 3, 164, 0, 0, 'Blacksmithing'),(2, 3, 172, 0, 5, 'Two-Handed Axes'),(2, 3, 173, 0, 5, 'Daggers'),(2, 3, 183, 5, 5, 'GENERIC (DND)'),(2, 3, 197, 0, 0, 'Tailoring'),(2, 3, 226, 0, 5, 'Crossbows'),(2, 3, 393, 0, 0, 'Skinning'),(2, 3, 109, 300, 300, 'Language: Orcish'),(2, 3, 137, 0, 0, 'Language: Thalassian'),(2, 3, 473, 0, 1, 'Fist Weapons'),(2, 3, 140, 0, 0, 'Language: Titan'),(2, 3, 149, 0, 0, 'Wolf Riding'),(2, 3, 229, 0, 0, 'Polearms'),(2, 3, 98, 0, 0, 'Language: Common'),(2, 3, 125, 5, 5, 'Orc Racial'),(2, 3, 315, 0, 0, 'Language: Troll'),(2, 3, 139, 0, 0, 'Language: Demon Tongue'),(2, 3, 152, 0, 0, 'Ram Riding'),(2, 3, 165, 0, 0, 'Leatherworking'),(2, 3, 415, 1, 1, 'Cloth'),(2, 3, 673, 0, 0, 'Language: Gutterspeak'),(2, 3, 95, 1, 5, 'Defense'),(2, 3, 333, 0, 0, 'Enchanting'),(2, 3, 186, 0, 0, 'Mining'),(2, 3, 43, 0, 5, 'Swords'),(2, 3, 55, 0, 5, 'Two-Handed Swords'),(2, 3, 713, 0, 0, 'Kodo Riding'),(2, 3, 176, 0, 5, 'Thrown'),(2, 3, 44, 1, 5, 'Axes'),(2, 3, 50, 0, 5, 'Beast Mastery'),(2, 3, 51, 5, 5, 'Survival'),(2, 3, 111, 0, 0, 'Language: Dwarven'),(2, 3, 113, 0, 0, 'Language: Darnassian'),(2, 3, 115, 0, 0, 'Language: Taurahe'),(2, 3, 118, 0, 0, 'Dual Wield'),(2, 3, 136, 0, 5, 'Staves'),(2, 3, 138, 0, 0, 'Language: Draconic'),(2, 3, 141, 0, 0, 'Language: Old Tongue'),(2, 3, 142, 0, 1, 'Survival'),(2, 3, 313, 0, 0, 'Language: Gnomish'),(2, 3, 413, 0, 0, 'Mail'),(2, 3, 414, 1, 1, 'Leather'),(2, 3, 45, 1, 5, 'Bows'),(2, 3, 163, 5, 5, 'Marksmanship'),(2, 3, 46, 0, 5, 'Guns'),(2, 3, 533, 0, 0, 'Raptor Riding'),(2, 3, 554, 0, 0, 'Undead Horsemanship'),(2, 3, 202, 0, 0, 'Engineering'),(2, 3, 356, 0, 0, 'Fishing'),(2, 3, 171, 0, 0, 'Alchemy'),(2, 3, 182, 0, 0, 'Herbalism'),(2, 3, 129, 0, 0, 'First Aid'),(2, 3, 185, 0, 0, 'Cooking'),(2, 3, 261, 0, 5, 'Beast Training'),(2, 3, 162, 1, 5, 'Unarmed'),(2, 4, 148, 0, 0, 'Horse Riding'),(2, 4, 150, 0, 0, 'Tiger Riding'),(2, 4, 164, 0, 0, 'Blacksmithing'),(2, 4, 173, 1, 5, 'Daggers'),(2, 4, 183, 5, 5, 'GENERIC (DND)'),(2, 4, 197, 0, 0, 'Tailoring'),(2, 4, 226, 0, 5, 'Crossbows'),(2, 4, 393, 0, 0, 'Skinning'),(2, 4, 109, 300, 300, 'Language: Orcish'),(2, 4, 137, 0, 0, 'Language: Thalassian'),(2, 4, 473, 0, 1, 'Fist Weapons'),(2, 4, 140, 0, 0, 'Language: Titan'),(2, 4, 149, 0, 0, 'Wolf Riding'),(2, 4, 98, 0, 0, 'Language: Common'),(2, 4, 125, 5, 5, 'Orc Racial'),(2, 4, 315, 0, 0, 'Language: Troll'),(2, 4, 139, 0, 0, 'Language: Demon Tongue'),(2, 4, 152, 0, 0, 'Ram Riding'),(2, 4, 165, 0, 0, 'Leatherworking'),(2, 4, 415, 1, 1, 'Cloth'),(2, 4, 633, 0, 5, 'Lockpicking'),(2, 4, 673, 0, 0, 'Language: Gutterspeak'),(2, 4, 95, 1, 5, 'Defense'),(2, 4, 333, 0, 0, 'Enchanting'),(2, 4, 186, 0, 0, 'Mining'),(2, 4, 43, 0, 5, 'Swords'),(2, 4, 713, 0, 0, 'Kodo Riding'),(2, 4, 176, 1, 5, 'Thrown'),(2, 4, 38, 5, 5, 'Combat'),(2, 4, 39, 0, 5, 'Subtlety'),(2, 4, 111, 0, 0, 'Language: Dwarven'),(2, 4, 113, 0, 0, 'Language: Darnassian'),(2, 4, 115, 0, 0, 'Language: Taurahe'),(2, 4, 118, 0, 0, 'Dual Wield'),(2, 4, 138, 0, 0, 'Language: Draconic'),(2, 4, 141, 0, 0, 'Language: Old Tongue'),(2, 4, 142, 0, 1, 'Survival'),(2, 4, 253, 5, 5, 'Assassination'),(2, 4, 313, 0, 0, 'Language: Gnomish'),(2, 4, 414, 1, 1, 'Leather'),(2, 4, 45, 0, 5, 'Bows'),(2, 4, 46, 0, 5, 'Guns'),(2, 4, 533, 0, 0, 'Raptor Riding'),(2, 4, 554, 0, 0, 'Undead Horsemanship'),(2, 4, 202, 0, 0, 'Engineering'),(2, 4, 356, 0, 0, 'Fishing'),(2, 4, 171, 0, 0, 'Alchemy'),(2, 4, 54, 0, 5, 'Maces'),(2, 4, 182, 0, 0, 'Herbalism'),(2, 4, 129, 0, 0, 'First Aid'),(2, 4, 185, 0, 0, 'Cooking'),(2, 4, 40, 0, 5, 'Poisons'),(2, 4, 162, 1, 5, 'Unarmed'),(2, 7, 148, 0, 0, 'Horse Riding'),(2, 7, 150, 0, 0, 'Tiger Riding'),(2, 7, 164, 0, 0, 'Blacksmithing'),(2, 7, 172, 0, 5, 'Two-Handed Axes'),(2, 7, 173, 0, 5, 'Daggers'),(2, 7, 183, 5, 5, 'GENERIC (DND)'),(2, 7, 197, 0, 0, 'Tailoring'),(2, 7, 393, 0, 0, 'Skinning'),(2, 7, 109, 300, 300, 'Language: Orcish'),(2, 7, 433, 1, 1, 'Shield'),(2, 7, 137, 0, 0, 'Language: Thalassian'),(2, 7, 473, 0, 1, 'Fist Weapons'),(2, 7, 140, 0, 0, 'Language: Titan'),(2, 7, 149, 0, 0, 'Wolf Riding'),(2, 7, 98, 0, 0, 'Language: Common'),(2, 7, 125, 5, 5, 'Orc Racial'),(2, 7, 315, 0, 0, 'Language: Troll'),(2, 7, 139, 0, 0, 'Language: Demon Tongue'),(2, 7, 152, 0, 0, 'Ram Riding'),(2, 7, 165, 0, 0, 'Leatherworking'),(2, 7, 373, 0, 5, 'Enhancement'),(2, 7, 374, 5, 5, 'Restoration'),(2, 7, 375, 5, 5, 'Elemental Combat'),(2, 7, 415, 1, 1, 'Cloth'),(2, 7, 673, 0, 0, 'Language: Gutterspeak'),(2, 7, 95, 1, 5, 'Defense'),(2, 7, 333, 0, 0, 'Enchanting'),(2, 7, 186, 0, 0, 'Mining'),(2, 7, 713, 0, 0, 'Kodo Riding'),(2, 7, 44, 0, 5, 'Axes'),(2, 7, 111, 0, 0, 'Language: Dwarven'),(2, 7, 113, 0, 0, 'Language: Darnassian'),(2, 7, 115, 0, 0, 'Language: Taurahe'),(2, 7, 136, 1, 5, 'Staves'),(2, 7, 138, 0, 0, 'Language: Draconic'),(2, 7, 141, 0, 0, 'Language: Old Tongue'),(2, 7, 142, 0, 1, 'Survival'),(2, 7, 160, 0, 5, 'Two-Handed Maces'),(2, 7, 313, 0, 0, 'Language: Gnomish'),(2, 7, 413, 0, 0, 'Mail'),(2, 7, 414, 1, 1, 'Leather'),(2, 7, 533, 0, 0, 'Raptor Riding'),(2, 7, 554, 0, 0, 'Undead Horsemanship'),(2, 7, 202, 0, 0, 'Engineering'),(2, 7, 356, 0, 0, 'Fishing'),(2, 7, 171, 0, 0, 'Alchemy'),(2, 7, 54, 1, 5, 'Maces'),(2, 7, 182, 0, 0, 'Herbalism'),(2, 7, 129, 0, 0, 'First Aid'),(2, 7, 185, 0, 0, 'Cooking'),(2, 7, 162, 1, 5, 'Unarmed'),(2, 9, 148, 0, 0, 'Horse Riding'),(2, 9, 150, 0, 0, 'Tiger Riding'),(2, 9, 164, 0, 0, 'Blacksmithing'),(2, 9, 173, 1, 5, 'Daggers'),(2, 9, 183, 5, 5, 'GENERIC (DND)'),(2, 9, 197, 0, 0, 'Tailoring'),(2, 9, 228, 1, 5, 'Wands'),(2, 9, 393, 0, 0, 'Skinning'),(2, 9, 109, 300, 300, 'Language: Orcish'),(2, 9, 137, 0, 0, 'Language: Thalassian'),(2, 9, 140, 0, 0, 'Language: Titan'),(2, 9, 149, 0, 0, 'Wolf Riding'),(2, 9, 593, 5, 5, 'Destruction'),(2, 9, 98, 0, 0, 'Language: Common'),(2, 9, 125, 5, 5, 'Orc Racial'),(2, 9, 315, 0, 0, 'Language: Troll'),(2, 9, 139, 0, 0, 'Language: Demon Tongue'),(2, 9, 152, 0, 0, 'Ram Riding'),(2, 9, 354, 5, 5, 'Demonology'),(2, 9, 355, 0, 5, 'Affliction'),(2, 9, 165, 0, 0, 'Leatherworking'),(2, 9, 415, 1, 1, 'Cloth'),(2, 9, 673, 0, 0, 'Language: Gutterspeak'),(2, 9, 95, 1, 5, 'Defense'),(2, 9, 333, 0, 0, 'Enchanting'),(2, 9, 186, 0, 0, 'Mining'),(2, 9, 43, 0, 5, 'Swords'),(2, 9, 713, 0, 0, 'Kodo Riding'),(2, 9, 111, 0, 0, 'Language: Dwarven'),(2, 9, 113, 0, 0, 'Language: Darnassian'),(2, 9, 115, 0, 0, 'Language: Taurahe'),(2, 9, 136, 0, 5, 'Staves'),(2, 9, 138, 0, 0, 'Language: Draconic'),(2, 9, 141, 0, 0, 'Language: Old Tongue'),(2, 9, 142, 0, 1, 'Survival'),(2, 9, 313, 0, 0, 'Language: Gnomish'),(2, 9, 533, 0, 0, 'Raptor Riding'),(2, 9, 554, 0, 0, 'Undead Horsemanship'),(2, 9, 202, 0, 0, 'Engineering'),(2, 9, 356, 0, 0, 'Fishing'),(2, 9, 171, 0, 0, 'Alchemy'),(2, 9, 182, 0, 0, 'Herbalism'),(2, 9, 129, 0, 0, 'First Aid'),(2, 9, 185, 0, 0, 'Cooking'),(2, 9, 162, 1, 5, 'Unarmed'),(3, 1, 148, 0, 0, 'Horse Riding'),(3, 1, 150, 0, 0, 'Tiger Riding'),(3, 1, 164, 0, 0, 'Blacksmithing'),(3, 1, 172, 1, 5, 'Two-Handed Axes'),(3, 1, 173, 0, 5, 'Daggers'),(3, 1, 183, 5, 5, 'GENERIC (DND)'),(3, 1, 197, 0, 0, 'Tailoring'),(3, 1, 226, 0, 5, 'Crossbows'),(3, 1, 393, 0, 0, 'Skinning'),(3, 1, 109, 0, 0, 'Language: Orcish'),(3, 1, 433, 1, 1, 'Shield'),(3, 1, 137, 0, 0, 'Language: Thalassian'),(3, 1, 473, 0, 1, 'Fist Weapons'),(3, 1, 140, 0, 0, 'Language: Titan'),(3, 1, 149, 0, 0, 'Wolf Riding'),(3, 1, 229, 0, 0, 'Polearms'),(3, 1, 257, 0, 5, 'Protection'),(3, 1, 293, 0, 0, 'Plate Mail'),(3, 1, 98, 300, 300, 'Language: Common'),(3, 1, 315, 0, 0, 'Language: Troll'),(3, 1, 139, 0, 0, 'Language: Demon Tongue'),(3, 1, 152, 0, 0, 'Ram Riding'),(3, 1, 165, 0, 0, 'Leatherworking'),(3, 1, 415, 1, 1, 'Cloth'),(3, 1, 256, 0, 5, 'Fury'),(3, 1, 673, 0, 0, 'Language: Gutterspeak'),(3, 1, 95, 1, 5, 'Defense'),(3, 1, 333, 0, 0, 'Enchanting'),(3, 1, 186, 0, 0, 'Mining'),(3, 1, 43, 0, 5, 'Swords'),(3, 1, 55, 0, 5, 'Two-Handed Swords'),(3, 1, 713, 0, 0, 'Kodo Riding'),(3, 1, 176, 0, 5, 'Thrown'),(3, 1, 26, 5, 5, 'Arms'),(3, 1, 44, 1, 5, 'Axes'),(3, 1, 101, 5, 5, 'Dwarven Racial'),(3, 1, 111, 300, 300, 'Language: Dwarven'),(3, 1, 113, 0, 0, 'Language: Darnassian'),(3, 1, 115, 0, 0, 'Language: Taurahe'),(3, 1, 118, 0, 0, 'Dual Wield'),(3, 1, 136, 0, 5, 'Staves'),(3, 1, 138, 0, 0, 'Language: Draconic'),(3, 1, 141, 0, 0, 'Language: Old Tongue'),(3, 1, 142, 0, 1, 'Survival'),(3, 1, 160, 0, 5, 'Two-Handed Maces'),(3, 1, 313, 0, 0, 'Language: Gnomish'),(3, 1, 413, 1, 1, 'Mail'),(3, 1, 414, 1, 1, 'Leather'),(3, 1, 45, 0, 5, 'Bows'),(3, 1, 46, 0, 5, 'Guns'),(3, 1, 533, 0, 0, 'Raptor Riding'),(3, 1, 553, 0, 0, 'Mechanostrider Piloting'),(3, 1, 554, 0, 0, 'Undead Horsemanship'),(3, 1, 202, 0, 0, 'Engineering'),(3, 1, 356, 0, 0, 'Fishing'),(3, 1, 171, 0, 0, 'Alchemy'),(3, 1, 54, 1, 5, 'Maces'),(3, 1, 182, 0, 0, 'Herbalism'),(3, 1, 129, 0, 0, 'First Aid'),(3, 1, 185, 0, 0, 'Cooking'),(3, 1, 162, 1, 5, 'Unarmed'),(3, 2, 148, 0, 0, 'Horse Riding'),(3, 2, 150, 0, 0, 'Tiger Riding'),(3, 2, 164, 0, 0, 'Blacksmithing'),(3, 2, 172, 0, 5, 'Two-Handed Axes'),(3, 2, 183, 5, 5, 'GENERIC (DND)'),(3, 2, 184, 0, 5, 'Retribution'),(3, 2, 197, 0, 0, 'Tailoring'),(3, 2, 393, 0, 0, 'Skinning'),(3, 2, 109, 0, 0, 'Language: Orcish'),(3, 2, 433, 1, 1, 'Shield'),(3, 2, 137, 0, 0, 'Language: Thalassian'),(3, 2, 140, 0, 0, 'Language: Titan'),(3, 2, 149, 0, 0, 'Wolf Riding'),(3, 2, 229, 0, 0, 'Polearms'),(3, 2, 293, 0, 0, 'Plate Mail'),(3, 2, 98, 300, 300, 'Language: Common'),(3, 2, 315, 0, 0, 'Language: Troll'),(3, 2, 139, 0, 0, 'Language: Demon Tongue'),(3, 2, 152, 0, 0, 'Ram Riding'),(3, 2, 165, 0, 0, 'Leatherworking'),(3, 2, 415, 1, 1, 'Cloth'),(3, 2, 594, 5, 5, 'Holy'),(3, 2, 673, 0, 0, 'Language: Gutterspeak'),(3, 2, 95, 1, 5, 'Defense'),(3, 2, 333, 0, 0, 'Enchanting'),(3, 2, 186, 0, 0, 'Mining'),(3, 2, 43, 0, 5, 'Swords'),(3, 2, 55, 0, 5, 'Two-Handed Swords'),(3, 2, 713, 0, 0, 'Kodo Riding'),(3, 2, 44, 0, 5, 'Axes'),(3, 2, 101, 5, 5, 'Dwarven Racial'),(3, 2, 111, 300, 300, 'Language: Dwarven'),(3, 2, 113, 0, 0, 'Language: Darnassian'),(3, 2, 115, 0, 0, 'Language: Taurahe'),(3, 2, 138, 0, 0, 'Language: Draconic'),(3, 2, 141, 0, 0, 'Language: Old Tongue'),(3, 2, 142, 0, 1, 'Survival'),(3, 2, 160, 1, 5, 'Two-Handed Maces'),(3, 2, 313, 0, 0, 'Language: Gnomish'),(3, 2, 413, 1, 1, 'Mail'),(3, 2, 414, 1, 1, 'Leather'),(3, 2, 533, 0, 0, 'Raptor Riding'),(3, 2, 553, 0, 0, 'Mechanostrider Piloting'),(3, 2, 554, 0, 0, 'Undead Horsemanship'),(3, 2, 202, 0, 0, 'Engineering'),(3, 2, 356, 0, 0, 'Fishing'),(3, 2, 171, 0, 0, 'Alchemy'),(3, 2, 54, 1, 5, 'Maces'),(3, 2, 182, 0, 0, 'Herbalism'),(3, 2, 129, 0, 0, 'First Aid'),(3, 2, 185, 0, 0, 'Cooking'),(3, 2, 267, 0, 5, 'Protection'),(3, 2, 162, 1, 5, 'Unarmed'),(3, 3, 148, 0, 0, 'Horse Riding'),(3, 3, 150, 0, 0, 'Tiger Riding'),(3, 3, 164, 0, 0, 'Blacksmithing'),(3, 3, 172, 0, 5, 'Two-Handed Axes'),(3, 3, 173, 0, 5, 'Daggers'),(3, 3, 183, 5, 5, 'GENERIC (DND)'),(3, 3, 197, 0, 0, 'Tailoring'),(3, 3, 226, 0, 5, 'Crossbows'),(3, 3, 393, 0, 0, 'Skinning'),(3, 3, 109, 0, 0, 'Language: Orcish'),(3, 3, 137, 0, 0, 'Language: Thalassian'),(3, 3, 473, 0, 1, 'Fist Weapons'),(3, 3, 140, 0, 0, 'Language: Titan'),(3, 3, 149, 0, 0, 'Wolf Riding'),(3, 3, 229, 0, 0, 'Polearms'),(3, 3, 98, 300, 300, 'Language: Common'),(3, 3, 315, 0, 0, 'Language: Troll'),(3, 3, 139, 0, 0, 'Language: Demon Tongue'),(3, 3, 152, 0, 0, 'Ram Riding'),(3, 3, 165, 0, 0, 'Leatherworking'),(3, 3, 415, 1, 1, 'Cloth'),(3, 3, 673, 0, 0, 'Language: Gutterspeak'),(3, 3, 95, 1, 5, 'Defense'),(3, 3, 333, 0, 0, 'Enchanting'),(3, 3, 186, 0, 0, 'Mining'),(3, 3, 43, 0, 5, 'Swords'),(3, 3, 55, 0, 5, 'Two-Handed Swords'),(3, 3, 713, 0, 0, 'Kodo Riding'),(3, 3, 176, 0, 5, 'Thrown'),(3, 3, 44, 1, 5, 'Axes'),(3, 3, 50, 0, 5, 'Beast Mastery'),(3, 3, 51, 5, 5, 'Survival'),(3, 3, 101, 5, 5, 'Dwarven Racial'),(3, 3, 111, 300, 300, 'Language: Dwarven'),(3, 3, 113, 0, 0, 'Language: Darnassian'),(3, 3, 115, 0, 0, 'Language: Taurahe'),(3, 3, 118, 0, 0, 'Dual Wield'),(3, 3, 136, 0, 5, 'Staves'),(3, 3, 138, 0, 0, 'Language: Draconic'),(3, 3, 141, 0, 0, 'Language: Old Tongue'),(3, 3, 142, 0, 1, 'Survival'),(3, 3, 313, 0, 0, 'Language: Gnomish'),(3, 3, 413, 0, 0, 'Mail'),(3, 3, 414, 1, 1, 'Leather'),(3, 3, 45, 0, 5, 'Bows'),(3, 3, 163, 5, 5, 'Marksmanship'),(3, 3, 46, 1, 5, 'Guns'),(3, 3, 533, 0, 0, 'Raptor Riding'),(3, 3, 553, 0, 0, 'Mechanostrider Piloting'),(3, 3, 554, 0, 0, 'Undead Horsemanship'),(3, 3, 202, 0, 0, 'Engineering'),(3, 3, 356, 0, 0, 'Fishing'),(3, 3, 171, 0, 0, 'Alchemy'),(3, 3, 182, 0, 0, 'Herbalism'),(3, 3, 129, 0, 0, 'First Aid'),(3, 3, 185, 0, 0, 'Cooking'),(3, 3, 261, 0, 5, 'Beast Training'),(3, 3, 162, 1, 5, 'Unarmed'),(3, 4, 148, 0, 0, 'Horse Riding'),(3, 4, 150, 0, 0, 'Tiger Riding'),(3, 4, 164, 0, 0, 'Blacksmithing'),(3, 4, 173, 1, 5, 'Daggers'),(3, 4, 183, 5, 5, 'GENERIC (DND)'),(3, 4, 197, 0, 0, 'Tailoring'),(3, 4, 226, 0, 5, 'Crossbows'),(3, 4, 393, 0, 0, 'Skinning'),(3, 4, 109, 0, 0, 'Language: Orcish'),(3, 4, 137, 0, 0, 'Language: Thalassian'),(3, 4, 473, 0, 1, 'Fist Weapons'),(3, 4, 140, 0, 0, 'Language: Titan'),(3, 4, 149, 0, 0, 'Wolf Riding'),(3, 4, 98, 300, 300, 'Language: Common'),(3, 4, 315, 0, 0, 'Language: Troll'),(3, 4, 139, 0, 0, 'Language: Demon Tongue'),(3, 4, 152, 0, 0, 'Ram Riding'),(3, 4, 165, 0, 0, 'Leatherworking'),(3, 4, 415, 1, 1, 'Cloth'),(3, 4, 633, 0, 5, 'Lockpicking'),(3, 4, 673, 0, 0, 'Language: Gutterspeak'),(3, 4, 95, 1, 5, 'Defense'),(3, 4, 333, 0, 0, 'Enchanting'),(3, 4, 186, 0, 0, 'Mining'),(3, 4, 43, 0, 5, 'Swords'),(3, 4, 713, 0, 0, 'Kodo Riding'),(3, 4, 176, 1, 5, 'Thrown'),(3, 4, 38, 5, 5, 'Combat'),(3, 4, 39, 0, 5, 'Subtlety'),(3, 4, 101, 5, 5, 'Dwarven Racial'),(3, 4, 111, 300, 300, 'Language: Dwarven'),(3, 4, 113, 0, 0, 'Language: Darnassian'),(3, 4, 115, 0, 0, 'Language: Taurahe'),(3, 4, 118, 0, 0, 'Dual Wield'),(3, 4, 138, 0, 0, 'Language: Draconic'),(3, 4, 141, 0, 0, 'Language: Old Tongue'),(3, 4, 142, 0, 1, 'Survival'),(3, 4, 253, 5, 5, 'Assassination'),(3, 4, 313, 0, 0, 'Language: Gnomish'),(3, 4, 414, 1, 1, 'Leather'),(3, 4, 45, 0, 5, 'Bows'),(3, 4, 46, 0, 5, 'Guns'),(3, 4, 533, 0, 0, 'Raptor Riding'),(3, 4, 553, 0, 0, 'Mechanostrider Piloting'),(3, 4, 554, 0, 0, 'Undead Horsemanship'),(3, 4, 202, 0, 0, 'Engineering'),(3, 4, 356, 0, 0, 'Fishing'),(3, 4, 171, 0, 0, 'Alchemy'),(3, 4, 54, 0, 5, 'Maces'),(3, 4, 182, 0, 0, 'Herbalism'),(3, 4, 129, 0, 0, 'First Aid'),(3, 4, 185, 0, 0, 'Cooking'),(3, 4, 40, 0, 5, 'Poisons'),(3, 4, 162, 1, 5, 'Unarmed'),(3, 5, 148, 0, 0, 'Horse Riding'),(3, 5, 150, 0, 0, 'Tiger Riding'),(3, 5, 164, 0, 0, 'Blacksmithing'),(3, 5, 173, 0, 5, 'Daggers'),(3, 5, 183, 5, 5, 'GENERIC (DND)'),(3, 5, 197, 0, 0, 'Tailoring'),(3, 5, 228, 1, 5, 'Wands'),(3, 5, 393, 0, 0, 'Skinning'),(3, 5, 109, 0, 0, 'Language: Orcish'),(3, 5, 137, 0, 0, 'Language: Thalassian'),(3, 5, 140, 0, 0, 'Language: Titan'),(3, 5, 149, 0, 0, 'Wolf Riding'),(3, 5, 98, 300, 300, 'Language: Common'),(3, 5, 315, 0, 0, 'Language: Troll'),(3, 5, 139, 0, 0, 'Language: Demon Tongue'),(3, 5, 152, 0, 0, 'Ram Riding'),(3, 5, 165, 0, 0, 'Leatherworking'),(3, 5, 415, 1, 1, 'Cloth'),(3, 5, 673, 0, 0, 'Language: Gutterspeak'),(3, 5, 95, 1, 5, 'Defense'),(3, 5, 333, 0, 0, 'Enchanting'),(3, 5, 186, 0, 0, 'Mining'),(3, 5, 713, 0, 0, 'Kodo Riding'),(3, 5, 56, 5, 5, 'Holy'),(3, 5, 78, 0, 5, 'Shadow Magic'),(3, 5, 101, 5, 5, 'Dwarven Racial'),(3, 5, 111, 300, 300, 'Language: Dwarven'),(3, 5, 113, 0, 0, 'Language: Darnassian'),(3, 5, 115, 0, 0, 'Language: Taurahe'),(3, 5, 136, 0, 5, 'Staves'),(3, 5, 138, 0, 0, 'Language: Draconic'),(3, 5, 141, 0, 0, 'Language: Old Tongue'),(3, 5, 142, 0, 1, 'Survival'),(3, 5, 313, 0, 0, 'Language: Gnomish'),(3, 5, 613, 0, 5, 'Discipline'),(3, 5, 533, 0, 0, 'Raptor Riding'),(3, 5, 553, 0, 0, 'Mechanostrider Piloting'),(3, 5, 554, 0, 0, 'Undead Horsemanship'),(3, 5, 202, 0, 0, 'Engineering'),(3, 5, 356, 0, 0, 'Fishing'),(3, 5, 171, 0, 0, 'Alchemy'),(3, 5, 54, 1, 5, 'Maces'),(3, 5, 182, 0, 0, 'Herbalism'),(3, 5, 129, 0, 0, 'First Aid'),(3, 5, 185, 0, 0, 'Cooking'),(3, 5, 162, 1, 5, 'Unarmed'),(4, 1, 148, 0, 0, 'Horse Riding'),(4, 1, 150, 0, 0, 'Tiger Riding'),(4, 1, 164, 0, 0, 'Blacksmithing'),(4, 1, 172, 0, 5, 'Two-Handed Axes'),(4, 1, 173, 1, 5, 'Daggers'),(4, 1, 183, 5, 5, 'GENERIC (DND)'),(4, 1, 197, 0, 0, 'Tailoring'),(4, 1, 226, 0, 5, 'Crossbows'),(4, 1, 393, 0, 0, 'Skinning'),(4, 1, 109, 0, 0, 'Language: Orcish'),(4, 1, 433, 1, 1, 'Shield'),(4, 1, 137, 0, 0, 'Language: Thalassian'),(4, 1, 473, 0, 1, 'Fist Weapons'),(4, 1, 140, 0, 0, 'Language: Titan'),(4, 1, 149, 0, 0, 'Wolf Riding'),(4, 1, 229, 0, 0, 'Polearms'),(4, 1, 257, 0, 5, 'Protection'),(4, 1, 293, 0, 0, 'Plate Mail'),(4, 1, 98, 300, 300, 'Language: Common'),(4, 1, 315, 0, 0, 'Language: Troll'),(4, 1, 139, 0, 0, 'Language: Demon Tongue'),(4, 1, 152, 0, 0, 'Ram Riding'),(4, 1, 165, 0, 0, 'Leatherworking'),(4, 1, 415, 1, 1, 'Cloth'),(4, 1, 256, 0, 5, 'Fury'),(4, 1, 673, 0, 0, 'Language: Gutterspeak'),(4, 1, 95, 1, 5, 'Defense'),(4, 1, 333, 0, 0, 'Enchanting'),(4, 1, 186, 0, 0, 'Mining'),(4, 1, 43, 1, 5, 'Swords'),(4, 1, 55, 0, 5, 'Two-Handed Swords'),(4, 1, 713, 0, 0, 'Kodo Riding'),(4, 1, 176, 0, 5, 'Thrown'),(4, 1, 26, 5, 5, 'Arms'),(4, 1, 44, 0, 5, 'Axes'),(4, 1, 111, 0, 0, 'Language: Dwarven'),(4, 1, 113, 300, 300, 'Language: Darnassian'),(4, 1, 115, 0, 0, 'Language: Taurahe'),(4, 1, 118, 0, 0, 'Dual Wield'),(4, 1, 136, 0, 5, 'Staves'),(4, 1, 138, 0, 0, 'Language: Draconic'),(4, 1, 141, 0, 0, 'Language: Old Tongue'),(4, 1, 142, 0, 1, 'Survival'),(4, 1, 160, 0, 5, 'Two-Handed Maces'),(4, 1, 313, 0, 0, 'Language: Gnomish'),(4, 1, 413, 1, 1, 'Mail'),(4, 1, 414, 1, 1, 'Leather'),(4, 1, 45, 0, 5, 'Bows'),(4, 1, 46, 0, 5, 'Guns'),(4, 1, 533, 0, 0, 'Raptor Riding'),(4, 1, 554, 0, 0, 'Undead Horsemanship'),(4, 1, 202, 0, 0, 'Engineering'),(4, 1, 356, 0, 0, 'Fishing'),(4, 1, 171, 0, 0, 'Alchemy'),(4, 1, 54, 1, 5, 'Maces'),(4, 1, 182, 0, 0, 'Herbalism'),(4, 1, 129, 0, 0, 'First Aid'),(4, 1, 185, 0, 0, 'Cooking'),(4, 1, 126, 5, 5, 'Night Elf Racial'),(4, 1, 162, 1, 5, 'Unarmed'),(4, 11, 148, 0, 0, 'Horse Riding'),(4, 11, 150, 0, 0, 'Tiger Riding'),(4, 11, 164, 0, 0, 'Blacksmithing'),(4, 11, 173, 1, 5, 'Daggers'),(4, 11, 183, 5, 5, 'GENERIC (DND)'),(4, 11, 197, 0, 0, 'Tailoring'),(4, 11, 574, 5, 5, 'Balance'),(4, 11, 393, 0, 0, 'Skinning'),(4, 11, 109, 0, 0, 'Language: Orcish'),(4, 11, 137, 0, 0, 'Language: Thalassian'),(4, 11, 473, 0, 1, 'Fist Weapons'),(4, 11, 140, 0, 0, 'Language: Titan'),(4, 11, 149, 0, 0, 'Wolf Riding'),(4, 11, 573, 5, 5, 'Restoration'),(4, 11, 98, 300, 300, 'Language: Common'),(4, 11, 315, 0, 0, 'Language: Troll'),(4, 11, 139, 0, 0, 'Language: Demon Tongue'),(4, 11, 152, 0, 0, 'Ram Riding'),(4, 11, 165, 0, 0, 'Leatherworking'),(4, 11, 134, 0, 0, 'Feral Combat'),(4, 11, 415, 1, 1, 'Cloth'),(4, 11, 673, 0, 0, 'Language: Gutterspeak'),(4, 11, 95, 1, 5, 'Defense'),(4, 11, 333, 0, 0, 'Enchanting'),(4, 11, 186, 0, 0, 'Mining'),(4, 11, 713, 0, 0, 'Kodo Riding'),(4, 11, 111, 0, 0, 'Language: Dwarven'),(4, 11, 113, 300, 300, 'Language: Darnassian'),(4, 11, 115, 0, 0, 'Language: Taurahe'),(4, 11, 136, 1, 5, 'Staves'),(4, 11, 138, 0, 0, 'Language: Draconic'),(4, 11, 141, 0, 0, 'Language: Old Tongue'),(4, 11, 142, 0, 1, 'Survival'),(4, 11, 160, 0, 5, 'Two-Handed Maces'),(4, 11, 313, 0, 0, 'Language: Gnomish'),(4, 11, 414, 1, 1, 'Leather'),(4, 11, 533, 0, 0, 'Raptor Riding'),(4, 11, 554, 0, 0, 'Undead Horsemanship'),(4, 11, 202, 0, 0, 'Engineering'),(4, 11, 356, 0, 0, 'Fishing'),(4, 11, 171, 0, 0, 'Alchemy'),(4, 11, 54, 0, 5, 'Maces'),(4, 11, 182, 0, 0, 'Herbalism'),(4, 11, 129, 0, 0, 'First Aid'),(4, 11, 185, 0, 0, 'Cooking'),(4, 11, 126, 5, 5, 'Night Elf Racial'),(4, 11, 162, 1, 5, 'Unarmed'),(4, 3, 148, 0, 0, 'Horse Riding'),(4, 3, 150, 0, 0, 'Tiger Riding'),(4, 3, 164, 0, 0, 'Blacksmithing'),(4, 3, 172, 0, 5, 'Two-Handed Axes'),(4, 3, 173, 1, 5, 'Daggers'),(4, 3, 183, 5, 5, 'GENERIC (DND)'),(4, 3, 197, 0, 0, 'Tailoring'),(4, 3, 226, 0, 5, 'Crossbows'),(4, 3, 393, 0, 0, 'Skinning'),(4, 3, 109, 0, 0, 'Language: Orcish'),(4, 3, 137, 0, 0, 'Language: Thalassian'),(4, 3, 473, 0, 1, 'Fist Weapons'),(4, 3, 140, 0, 0, 'Language: Titan'),(4, 3, 149, 0, 0, 'Wolf Riding'),(4, 3, 229, 0, 0, 'Polearms'),(4, 3, 98, 300, 300, 'Language: Common'),(4, 3, 315, 0, 0, 'Language: Troll'),(4, 3, 139, 0, 0, 'Language: Demon Tongue'),(4, 3, 152, 0, 0, 'Ram Riding'),(4, 3, 165, 0, 0, 'Leatherworking'),(4, 3, 415, 1, 1, 'Cloth'),(4, 3, 673, 0, 0, 'Language: Gutterspeak'),(4, 3, 95, 1, 5, 'Defense'),(4, 3, 333, 0, 0, 'Enchanting'),(4, 3, 186, 0, 0, 'Mining'),(4, 3, 43, 0, 5, 'Swords'),(4, 3, 55, 0, 5, 'Two-Handed Swords'),(4, 3, 713, 0, 0, 'Kodo Riding'),(4, 3, 176, 0, 5, 'Thrown'),(4, 3, 44, 0, 5, 'Axes'),(4, 3, 50, 0, 5, 'Beast Mastery'),(4, 3, 51, 5, 5, 'Survival'),(4, 3, 111, 0, 0, 'Language: Dwarven'),(4, 3, 113, 300, 300, 'Language: Darnassian'),(4, 3, 115, 0, 0, 'Language: Taurahe'),(4, 3, 118, 0, 0, 'Dual Wield'),(4, 3, 136, 0, 5, 'Staves'),(4, 3, 138, 0, 0, 'Language: Draconic'),(4, 3, 141, 0, 0, 'Language: Old Tongue'),(4, 3, 142, 0, 1, 'Survival'),(4, 3, 313, 0, 0, 'Language: Gnomish'),(4, 3, 413, 0, 0, 'Mail'),(4, 3, 414, 1, 1, 'Leather'),(4, 3, 45, 1, 5, 'Bows'),(4, 3, 163, 5, 5, 'Marksmanship'),(4, 3, 46, 0, 5, 'Guns'),(4, 3, 533, 0, 0, 'Raptor Riding'),(4, 3, 554, 0, 0, 'Undead Horsemanship'),(4, 3, 202, 0, 0, 'Engineering'),(4, 3, 356, 0, 0, 'Fishing'),(4, 3, 171, 0, 0, 'Alchemy'),(4, 3, 182, 0, 0, 'Herbalism'),(4, 3, 129, 0, 0, 'First Aid'),(4, 3, 185, 0, 0, 'Cooking'),(4, 3, 126, 5, 5, 'Night Elf Racial'),(4, 3, 261, 0, 5, 'Beast Training'),(4, 3, 162, 1, 5, 'Unarmed'),(4, 4, 148, 0, 0, 'Horse Riding'),(4, 4, 150, 0, 0, 'Tiger Riding'),(4, 4, 164, 0, 0, 'Blacksmithing'),(4, 4, 173, 1, 5, 'Daggers'),(4, 4, 183, 5, 5, 'GENERIC (DND)'),(4, 4, 197, 0, 0, 'Tailoring'),(4, 4, 226, 0, 5, 'Crossbows'),(4, 4, 393, 0, 0, 'Skinning'),(4, 4, 109, 0, 0, 'Language: Orcish'),(4, 4, 137, 0, 0, 'Language: Thalassian'),(4, 4, 473, 0, 1, 'Fist Weapons'),(4, 4, 140, 0, 0, 'Language: Titan'),(4, 4, 149, 0, 0, 'Wolf Riding'),(4, 4, 98, 300, 300, 'Language: Common'),(4, 4, 315, 0, 0, 'Language: Troll'),(4, 4, 139, 0, 0, 'Language: Demon Tongue'),(4, 4, 152, 0, 0, 'Ram Riding'),(4, 4, 165, 0, 0, 'Leatherworking'),(4, 4, 415, 1, 1, 'Cloth'),(4, 4, 633, 0, 5, 'Lockpicking'),(4, 4, 673, 0, 0, 'Language: Gutterspeak'),(4, 4, 95, 1, 5, 'Defense'),(4, 4, 333, 0, 0, 'Enchanting'),(4, 4, 186, 0, 0, 'Mining'),(4, 4, 43, 0, 5, 'Swords'),(4, 4, 713, 0, 0, 'Kodo Riding'),(4, 4, 176, 1, 5, 'Thrown'),(4, 4, 38, 5, 5, 'Combat'),(4, 4, 39, 0, 5, 'Subtlety'),(4, 4, 111, 0, 0, 'Language: Dwarven'),(4, 4, 113, 300, 300, 'Language: Darnassian'),(4, 4, 115, 0, 0, 'Language: Taurahe'),(4, 4, 118, 0, 0, 'Dual Wield'),(4, 4, 138, 0, 0, 'Language: Draconic'),(4, 4, 141, 0, 0, 'Language: Old Tongue'),(4, 4, 142, 0, 1, 'Survival'),(4, 4, 253, 5, 5, 'Assassination'),(4, 4, 313, 0, 0, 'Language: Gnomish'),(4, 4, 414, 1, 1, 'Leather'),(4, 4, 45, 0, 5, 'Bows'),(4, 4, 46, 0, 5, 'Guns'),(4, 4, 533, 0, 0, 'Raptor Riding'),(4, 4, 554, 0, 0, 'Undead Horsemanship'),(4, 4, 202, 0, 0, 'Engineering'),(4, 4, 356, 0, 0, 'Fishing'),(4, 4, 171, 0, 0, 'Alchemy'),(4, 4, 54, 0, 5, 'Maces'),(4, 4, 182, 0, 0, 'Herbalism'),(4, 4, 129, 0, 0, 'First Aid'),(4, 4, 185, 0, 0, 'Cooking'),(4, 4, 126, 5, 5, 'Night Elf Racial'),(4, 4, 40, 0, 5, 'Poisons'),(4, 4, 162, 1, 5, 'Unarmed'),(4, 5, 148, 0, 0, 'Horse Riding'),(4, 5, 150, 0, 0, 'Tiger Riding'),(4, 5, 164, 0, 0, 'Blacksmithing'),(4, 5, 173, 0, 5, 'Daggers'),(4, 5, 183, 5, 5, 'GENERIC (DND)'),(4, 5, 197, 0, 0, 'Tailoring'),(4, 5, 228, 1, 5, 'Wands'),(4, 5, 393, 0, 0, 'Skinning'),(4, 5, 109, 0, 0, 'Language: Orcish'),(4, 5, 137, 0, 0, 'Language: Thalassian'),(4, 5, 140, 0, 0, 'Language: Titan'),(4, 5, 149, 0, 0, 'Wolf Riding'),(4, 5, 98, 300, 300, 'Language: Common'),(4, 5, 315, 0, 0, 'Language: Troll'),(4, 5, 139, 0, 0, 'Language: Demon Tongue'),(4, 5, 152, 0, 0, 'Ram Riding'),(4, 5, 165, 0, 0, 'Leatherworking'),(4, 5, 415, 1, 1, 'Cloth'),(4, 5, 673, 0, 0, 'Language: Gutterspeak'),(4, 5, 95, 1, 5, 'Defense'),(4, 5, 333, 0, 0, 'Enchanting'),(4, 5, 186, 0, 0, 'Mining'),(4, 5, 713, 0, 0, 'Kodo Riding'),(4, 5, 56, 5, 5, 'Holy'),(4, 5, 78, 0, 5, 'Shadow Magic'),(4, 5, 111, 0, 0, 'Language: Dwarven'),(4, 5, 113, 300, 300, 'Language: Darnassian'),(4, 5, 115, 0, 0, 'Language: Taurahe'),(4, 5, 136, 0, 5, 'Staves'),(4, 5, 138, 0, 0, 'Language: Draconic'),(4, 5, 141, 0, 0, 'Language: Old Tongue'),(4, 5, 142, 0, 1, 'Survival'),(4, 5, 313, 0, 0, 'Language: Gnomish'),(4, 5, 613, 0, 5, 'Discipline'),(4, 5, 533, 0, 0, 'Raptor Riding'),(4, 5, 554, 0, 0, 'Undead Horsemanship'),(4, 5, 202, 0, 0, 'Engineering'),(4, 5, 356, 0, 0, 'Fishing'),(4, 5, 171, 0, 0, 'Alchemy'),(4, 5, 54, 1, 5, 'Maces'),(4, 5, 182, 0, 0, 'Herbalism'),(4, 5, 129, 0, 0, 'First Aid'),(4, 5, 185, 0, 0, 'Cooking'),(4, 5, 126, 5, 5, 'Night Elf Racial'),(4, 5, 162, 1, 5, 'Unarmed'),(5, 1, 148, 0, 0, 'Horse Riding'),(5, 1, 150, 0, 0, 'Tiger Riding'),(5, 1, 164, 0, 0, 'Blacksmithing'),(5, 1, 172, 0, 5, 'Two-Handed Axes'),(5, 1, 173, 1, 5, 'Daggers'),(5, 1, 183, 5, 5, 'GENERIC (DND)'),(5, 1, 197, 0, 0, 'Tailoring'),(5, 1, 220, 5, 5, 'Racial - Undead'),(5, 1, 226, 0, 5, 'Crossbows'),(5, 1, 393, 0, 0, 'Skinning'),(5, 1, 109, 300, 300, 'Language: Orcish'),(5, 1, 433, 1, 1, 'Shield'),(5, 1, 137, 0, 0, 'Language: Thalassian'),(5, 1, 473, 0, 1, 'Fist Weapons'),(5, 1, 140, 0, 0, 'Language: Titan'),(5, 1, 149, 0, 0, 'Wolf Riding'),(5, 1, 229, 0, 0, 'Polearms'),(5, 1, 257, 0, 5, 'Protection'),(5, 1, 293, 0, 0, 'Plate Mail'),(5, 1, 315, 0, 0, 'Language: Troll'),(5, 1, 139, 0, 0, 'Language: Demon Tongue'),(5, 1, 152, 0, 0, 'Ram Riding'),(5, 1, 165, 0, 0, 'Leatherworking'),(5, 1, 415, 1, 1, 'Cloth'),(5, 1, 256, 0, 5, 'Fury'),(5, 1, 673, 300, 300, 'Language: Gutterspeak'),(5, 1, 95, 1, 5, 'Defense'),(5, 1, 333, 0, 0, 'Enchanting'),(5, 1, 186, 0, 0, 'Mining'),(5, 1, 43, 1, 5, 'Swords'),(5, 1, 55, 1, 5, 'Two-Handed Swords'),(5, 1, 713, 0, 0, 'Kodo Riding'),(5, 1, 176, 0, 5, 'Thrown'),(5, 1, 26, 5, 5, 'Arms'),(5, 1, 44, 0, 5, 'Axes'),(5, 1, 111, 0, 0, 'Language: Dwarven'),(5, 1, 113, 0, 0, 'Language: Darnassian'),(5, 1, 115, 0, 0, 'Language: Taurahe'),(5, 1, 118, 0, 0, 'Dual Wield'),(5, 1, 136, 0, 5, 'Staves'),(5, 1, 138, 0, 0, 'Language: Draconic'),(5, 1, 141, 0, 0, 'Language: Old Tongue'),(5, 1, 142, 0, 1, 'Survival'),(5, 1, 160, 0, 5, 'Two-Handed Maces'),(5, 1, 313, 0, 0, 'Language: Gnomish'),(5, 1, 413, 1, 1, 'Mail'),(5, 1, 414, 1, 1, 'Leather'),(5, 1, 45, 0, 5, 'Bows'),(5, 1, 46, 0, 5, 'Guns'),(5, 1, 533, 0, 0, 'Raptor Riding'),(5, 1, 554, 0, 0, 'Undead Horsemanship'),(5, 1, 202, 0, 0, 'Engineering'),(5, 1, 356, 0, 0, 'Fishing'),(5, 1, 171, 0, 0, 'Alchemy'),(5, 1, 54, 0, 5, 'Maces'),(5, 1, 182, 0, 0, 'Herbalism'),(5, 1, 129, 0, 0, 'First Aid'),(5, 1, 185, 0, 0, 'Cooking'),(5, 1, 162, 1, 5, 'Unarmed'),(5, 4, 148, 0, 0, 'Horse Riding'),(5, 4, 150, 0, 0, 'Tiger Riding'),(5, 4, 164, 0, 0, 'Blacksmithing'),(5, 4, 173, 1, 5, 'Daggers'),(5, 4, 183, 5, 5, 'GENERIC (DND)'),(5, 4, 197, 0, 0, 'Tailoring'),(5, 4, 220, 5, 5, 'Racial - Undead'),(5, 4, 226, 0, 5, 'Crossbows'),(5, 4, 393, 0, 0, 'Skinning'),(5, 4, 109, 300, 300, 'Language: Orcish'),(5, 4, 137, 0, 0, 'Language: Thalassian'),(5, 4, 473, 0, 1, 'Fist Weapons'),(5, 4, 140, 0, 0, 'Language: Titan'),(5, 4, 149, 0, 0, 'Wolf Riding'),(5, 4, 315, 0, 0, 'Language: Troll'),(5, 4, 139, 0, 0, 'Language: Demon Tongue'),(5, 4, 152, 0, 0, 'Ram Riding'),(5, 4, 165, 0, 0, 'Leatherworking'),(5, 4, 415, 1, 1, 'Cloth'),(5, 4, 633, 0, 5, 'Lockpicking'),(5, 4, 673, 300, 300, 'Language: Gutterspeak'),(5, 4, 95, 1, 5, 'Defense'),(5, 4, 333, 0, 0, 'Enchanting'),(5, 4, 186, 0, 0, 'Mining'),(5, 4, 43, 0, 5, 'Swords'),(5, 4, 713, 0, 0, 'Kodo Riding'),(5, 4, 176, 1, 5, 'Thrown'),(5, 4, 38, 5, 5, 'Combat'),(5, 4, 39, 0, 5, 'Subtlety'),(5, 4, 111, 0, 0, 'Language: Dwarven'),(5, 4, 113, 0, 0, 'Language: Darnassian'),(5, 4, 115, 0, 0, 'Language: Taurahe'),(5, 4, 118, 0, 0, 'Dual Wield'),(5, 4, 138, 0, 0, 'Language: Draconic'),(5, 4, 141, 0, 0, 'Language: Old Tongue'),(5, 4, 142, 0, 1, 'Survival'),(5, 4, 253, 5, 5, 'Assassination'),(5, 4, 313, 0, 0, 'Language: Gnomish'),(5, 4, 414, 1, 1, 'Leather'),(5, 4, 45, 0, 5, 'Bows'),(5, 4, 46, 0, 5, 'Guns'),(5, 4, 533, 0, 0, 'Raptor Riding'),(5, 4, 554, 0, 0, 'Undead Horsemanship'),(5, 4, 202, 0, 0, 'Engineering'),(5, 4, 356, 0, 0, 'Fishing'),(5, 4, 171, 0, 0, 'Alchemy'),(5, 4, 54, 0, 5, 'Maces'),(5, 4, 182, 0, 0, 'Herbalism'),(5, 4, 129, 0, 0, 'First Aid'),(5, 4, 185, 0, 0, 'Cooking'),(5, 4, 40, 0, 5, 'Poisons'),(5, 4, 162, 1, 5, 'Unarmed'),(5, 5, 148, 0, 0, 'Horse Riding'),(5, 5, 150, 0, 0, 'Tiger Riding'),(5, 5, 164, 0, 0, 'Blacksmithing'),(5, 5, 173, 0, 5, 'Daggers'),(5, 5, 183, 5, 5, 'GENERIC (DND)'),(5, 5, 197, 0, 0, 'Tailoring'),(5, 5, 220, 5, 5, 'Racial - Undead'),(5, 5, 228, 1, 5, 'Wands'),(5, 5, 393, 0, 0, 'Skinning'),(5, 5, 109, 300, 300, 'Language: Orcish'),(5, 5, 137, 0, 0, 'Language: Thalassian'),(5, 5, 140, 0, 0, 'Language: Titan'),(5, 5, 149, 0, 0, 'Wolf Riding'),(5, 5, 315, 0, 0, 'Language: Troll'),(5, 5, 139, 0, 0, 'Language: Demon Tongue'),(5, 5, 152, 0, 0, 'Ram Riding'),(5, 5, 165, 0, 0, 'Leatherworking'),(5, 5, 415, 1, 1, 'Cloth'),(5, 5, 673, 300, 300, 'Language: Gutterspeak'),(5, 5, 95, 1, 5, 'Defense'),(5, 5, 333, 0, 0, 'Enchanting'),(5, 5, 186, 0, 0, 'Mining'),(5, 5, 713, 0, 0, 'Kodo Riding'),(5, 5, 56, 5, 5, 'Holy'),(5, 5, 78, 0, 5, 'Shadow Magic'),(5, 5, 111, 0, 0, 'Language: Dwarven'),(5, 5, 113, 0, 0, 'Language: Darnassian'),(5, 5, 115, 0, 0, 'Language: Taurahe'),(5, 5, 136, 0, 5, 'Staves'),(5, 5, 138, 0, 0, 'Language: Draconic'),(5, 5, 141, 0, 0, 'Language: Old Tongue'),(5, 5, 142, 0, 1, 'Survival'),(5, 5, 313, 0, 0, 'Language: Gnomish'),(5, 5, 613, 0, 5, 'Discipline'),(5, 5, 533, 0, 0, 'Raptor Riding'),(5, 5, 554, 0, 0, 'Undead Horsemanship'),(5, 5, 202, 0, 0, 'Engineering'),(5, 5, 356, 0, 0, 'Fishing'),(5, 5, 171, 0, 0, 'Alchemy'),(5, 5, 54, 1, 5, 'Maces'),(5, 5, 182, 0, 0, 'Herbalism'),(5, 5, 129, 0, 0, 'First Aid'),(5, 5, 185, 0, 0, 'Cooking'),(5, 5, 162, 1, 5, 'Unarmed'),(5, 8, 148, 0, 0, 'Horse Riding'),(5, 8, 150, 0, 0, 'Tiger Riding'),(5, 8, 164, 0, 0, 'Blacksmithing'),(5, 8, 173, 0, 5, 'Daggers'),(5, 8, 183, 5, 5, 'GENERIC (DND)'),(5, 8, 197, 0, 0, 'Tailoring'),(5, 8, 220, 5, 5, 'Racial - Undead'),(5, 8, 228, 1, 5, 'Wands'),(5, 8, 393, 0, 0, 'Skinning'),(5, 8, 109, 300, 300, 'Language: Orcish'),(5, 8, 137, 0, 0, 'Language: Thalassian'),(5, 8, 140, 0, 0, 'Language: Titan'),(5, 8, 149, 0, 0, 'Wolf Riding'),(5, 8, 8, 5, 5, 'Fire'),(5, 8, 315, 0, 0, 'Language: Troll'),(5, 8, 139, 0, 0, 'Language: Demon Tongue'),(5, 8, 152, 0, 0, 'Ram Riding'),(5, 8, 165, 0, 0, 'Leatherworking'),(5, 8, 415, 1, 1, 'Cloth'),(5, 8, 673, 300, 300, 'Language: Gutterspeak'),(5, 8, 95, 1, 5, 'Defense'),(5, 8, 333, 0, 0, 'Enchanting'),(5, 8, 186, 0, 0, 'Mining'),(5, 8, 43, 0, 5, 'Swords'),(5, 8, 713, 0, 0, 'Kodo Riding'),(5, 8, 6, 5, 5, 'Frost'),(5, 8, 111, 0, 0, 'Language: Dwarven'),(5, 8, 113, 0, 0, 'Language: Darnassian'),(5, 8, 115, 0, 0, 'Language: Taurahe'),(5, 8, 136, 1, 5, 'Staves'),(5, 8, 138, 0, 0, 'Language: Draconic'),(5, 8, 141, 0, 0, 'Language: Old Tongue'),(5, 8, 142, 0, 1, 'Survival'),(5, 8, 313, 0, 0, 'Language: Gnomish'),(5, 8, 533, 0, 0, 'Raptor Riding'),(5, 8, 554, 0, 0, 'Undead Horsemanship'),(5, 8, 202, 0, 0, 'Engineering'),(5, 8, 356, 0, 0, 'Fishing'),(5, 8, 171, 0, 0, 'Alchemy'),(5, 8, 182, 0, 0, 'Herbalism'),(5, 8, 129, 0, 0, 'First Aid'),(5, 8, 185, 0, 0, 'Cooking'),(5, 8, 237, 0, 5, 'Arcane'),(5, 8, 162, 1, 5, 'Unarmed'),(5, 9, 148, 0, 0, 'Horse Riding'),(5, 9, 150, 0, 0, 'Tiger Riding'),(5, 9, 164, 0, 0, 'Blacksmithing'),(5, 9, 173, 1, 5, 'Daggers'),(5, 9, 183, 5, 5, 'GENERIC (DND)'),(5, 9, 197, 0, 0, 'Tailoring'),(5, 9, 220, 5, 5, 'Racial - Undead'),(5, 9, 228, 1, 5, 'Wands'),(5, 9, 393, 0, 0, 'Skinning'),(5, 9, 109, 300, 300, 'Language: Orcish'),(5, 9, 137, 0, 0, 'Language: Thalassian'),(5, 9, 140, 0, 0, 'Language: Titan'),(5, 9, 149, 0, 0, 'Wolf Riding'),(5, 9, 593, 5, 5, 'Destruction'),(5, 9, 315, 0, 0, 'Language: Troll'),(5, 9, 139, 0, 0, 'Language: Demon Tongue'),(5, 9, 152, 0, 0, 'Ram Riding'),(5, 9, 354, 5, 5, 'Demonology'),(5, 9, 355, 0, 5, 'Affliction'),(5, 9, 165, 0, 0, 'Leatherworking'),(5, 9, 415, 1, 1, 'Cloth'),(5, 9, 673, 300, 300, 'Language: Gutterspeak'),(5, 9, 95, 1, 5, 'Defense'),(5, 9, 333, 0, 0, 'Enchanting'),(5, 9, 186, 0, 0, 'Mining'),(5, 9, 43, 0, 5, 'Swords'),(5, 9, 713, 0, 0, 'Kodo Riding'),(5, 9, 111, 0, 0, 'Language: Dwarven'),(5, 9, 113, 0, 0, 'Language: Darnassian'),(5, 9, 115, 0, 0, 'Language: Taurahe'),(5, 9, 136, 0, 5, 'Staves'),(5, 9, 138, 0, 0, 'Language: Draconic'),(5, 9, 141, 0, 0, 'Language: Old Tongue'),(5, 9, 142, 0, 1, 'Survival'),(5, 9, 313, 0, 0, 'Language: Gnomish'),(5, 9, 533, 0, 0, 'Raptor Riding'),(5, 9, 554, 0, 0, 'Undead Horsemanship'),(5, 9, 202, 0, 0, 'Engineering'),(5, 9, 356, 0, 0, 'Fishing'),(5, 9, 171, 0, 0, 'Alchemy'),(5, 9, 182, 0, 0, 'Herbalism'),(5, 9, 129, 0, 0, 'First Aid'),(5, 9, 185, 0, 0, 'Cooking'),(5, 9, 162, 1, 5, 'Unarmed'),(6, 1, 164, 0, 0, 'Blacksmithing'),(6, 1, 172, 0, 5, 'Two-Handed Axes'),(6, 1, 173, 0, 5, 'Daggers'),(6, 1, 183, 5, 5, 'GENERIC (DND)'),(6, 1, 197, 0, 0, 'Tailoring'),(6, 1, 226, 0, 5, 'Crossbows'),(6, 1, 393, 0, 0, 'Skinning'),(6, 1, 109, 300, 300, 'Language: Orcish'),(6, 1, 124, 5, 5, 'Tauren Racial'),(6, 1, 433, 1, 1, 'Shield'),(6, 1, 137, 0, 0, 'Language: Thalassian'),(6, 1, 473, 0, 1, 'Fist Weapons'),(6, 1, 140, 0, 0, 'Language: Titan'),(6, 1, 149, 0, 0, 'Wolf Riding'),(6, 1, 229, 0, 0, 'Polearms'),(6, 1, 257, 0, 5, 'Protection'),(6, 1, 293, 0, 0, 'Plate Mail'),(6, 1, 98, 0, 0, 'Language: Common'),(6, 1, 315, 0, 0, 'Language: Troll'),(6, 1, 139, 0, 0, 'Language: Demon Tongue'),(6, 1, 165, 0, 0, 'Leatherworking'),(6, 1, 415, 1, 1, 'Cloth'),(6, 1, 256, 0, 5, 'Fury'),(6, 1, 673, 0, 0, 'Language: Gutterspeak'),(6, 1, 95, 1, 5, 'Defense'),(6, 1, 333, 0, 0, 'Enchanting'),(6, 1, 186, 0, 0, 'Mining'),(6, 1, 43, 0, 5, 'Swords'),(6, 1, 55, 0, 5, 'Two-Handed Swords'),(6, 1, 713, 0, 0, 'Kodo Riding'),(6, 1, 176, 0, 5, 'Thrown'),(6, 1, 26, 5, 5, 'Arms'),(6, 1, 44, 1, 5, 'Axes'),(6, 1, 111, 0, 0, 'Language: Dwarven'),(6, 1, 113, 0, 0, 'Language: Darnassian'),(6, 1, 115, 300, 300, 'Language: Taurahe'),(6, 1, 118, 0, 0, 'Dual Wield'),(6, 1, 136, 0, 5, 'Staves'),(6, 1, 138, 0, 0, 'Language: Draconic'),(6, 1, 141, 0, 0, 'Language: Old Tongue'),(6, 1, 142, 0, 1, 'Survival'),(6, 1, 160, 1, 5, 'Two-Handed Maces'),(6, 1, 313, 0, 0, 'Language: Gnomish'),(6, 1, 413, 1, 1, 'Mail'),(6, 1, 414, 1, 1, 'Leather'),(6, 1, 45, 0, 5, 'Bows'),(6, 1, 46, 0, 5, 'Guns'),(6, 1, 202, 0, 0, 'Engineering'),(6, 1, 356, 0, 0, 'Fishing'),(6, 1, 171, 0, 0, 'Alchemy'),(6, 1, 54, 1, 5, 'Maces'),(6, 1, 182, 0, 0, 'Herbalism'),(6, 1, 129, 0, 0, 'First Aid'),(6, 1, 185, 0, 0, 'Cooking'),(6, 1, 162, 1, 5, 'Unarmed'),(6, 11, 164, 0, 0, 'Blacksmithing'),(6, 11, 173, 0, 5, 'Daggers'),(6, 11, 183, 5, 5, 'GENERIC (DND)'),(6, 11, 197, 0, 0, 'Tailoring'),(6, 11, 574, 5, 5, 'Balance'),(6, 11, 393, 0, 0, 'Skinning'),(6, 11, 109, 300, 300, 'Language: Orcish'),(6, 11, 124, 5, 5, 'Tauren Racial'),(6, 11, 137, 0, 0, 'Language: Thalassian'),(6, 11, 473, 0, 1, 'Fist Weapons'),(6, 11, 140, 0, 0, 'Language: Titan'),(6, 11, 149, 0, 0, 'Wolf Riding'),(6, 11, 573, 5, 5, 'Restoration'),(6, 11, 98, 0, 0, 'Language: Common'),(6, 11, 315, 0, 0, 'Language: Troll'),(6, 11, 139, 0, 0, 'Language: Demon Tongue'),(6, 11, 165, 0, 0, 'Leatherworking'),(6, 11, 134, 0, 0, 'Feral Combat'),(6, 11, 415, 1, 1, 'Cloth'),(6, 11, 673, 0, 0, 'Language: Gutterspeak'),(6, 11, 95, 1, 5, 'Defense'),(6, 11, 333, 0, 0, 'Enchanting'),(6, 11, 186, 0, 0, 'Mining'),(6, 11, 713, 0, 0, 'Kodo Riding'),(6, 11, 111, 0, 0, 'Language: Dwarven'),(6, 11, 113, 0, 0, 'Language: Darnassian'),(6, 11, 115, 300, 300, 'Language: Taurahe'),(6, 11, 136, 1, 5, 'Staves'),(6, 11, 138, 0, 0, 'Language: Draconic'),(6, 11, 141, 0, 0, 'Language: Old Tongue'),(6, 11, 142, 0, 1, 'Survival'),(6, 11, 160, 0, 5, 'Two-Handed Maces'),(6, 11, 313, 0, 0, 'Language: Gnomish'),(6, 11, 414, 1, 1, 'Leather'),(6, 11, 202, 0, 0, 'Engineering'),(6, 11, 356, 0, 0, 'Fishing'),(6, 11, 171, 0, 0, 'Alchemy'),(6, 11, 54, 1, 5, 'Maces'),(6, 11, 182, 0, 0, 'Herbalism'),(6, 11, 129, 0, 0, 'First Aid'),(6, 11, 185, 0, 0, 'Cooking'),(6, 11, 162, 1, 5, 'Unarmed'),(6, 3, 164, 0, 0, 'Blacksmithing'),(6, 3, 172, 0, 5, 'Two-Handed Axes'),(6, 3, 173, 0, 5, 'Daggers'),(6, 3, 183, 5, 5, 'GENERIC (DND)'),(6, 3, 197, 0, 0, 'Tailoring'),(6, 3, 226, 0, 5, 'Crossbows'),(6, 3, 393, 0, 0, 'Skinning'),(6, 3, 109, 300, 300, 'Language: Orcish'),(6, 3, 124, 5, 5, 'Tauren Racial'),(6, 3, 137, 0, 0, 'Language: Thalassian'),(6, 3, 473, 0, 1, 'Fist Weapons'),(6, 3, 140, 0, 0, 'Language: Titan'),(6, 3, 149, 0, 0, 'Wolf Riding'),(6, 3, 229, 0, 0, 'Polearms'),(6, 3, 98, 0, 0, 'Language: Common'),(6, 3, 315, 0, 0, 'Language: Troll'),(6, 3, 139, 0, 0, 'Language: Demon Tongue'),(6, 3, 165, 0, 0, 'Leatherworking'),(6, 3, 415, 1, 1, 'Cloth'),(6, 3, 673, 0, 0, 'Language: Gutterspeak'),(6, 3, 95, 1, 5, 'Defense'),(6, 3, 333, 0, 0, 'Enchanting'),(6, 3, 186, 0, 0, 'Mining'),(6, 3, 43, 0, 5, 'Swords'),(6, 3, 55, 0, 5, 'Two-Handed Swords'),(6, 3, 713, 0, 0, 'Kodo Riding'),(6, 3, 176, 0, 5, 'Thrown'),(6, 3, 44, 1, 5, 'Axes'),(6, 3, 50, 0, 5, 'Beast Mastery'),(6, 3, 51, 5, 5, 'Survival'),(6, 3, 111, 0, 0, 'Language: Dwarven'),(6, 3, 113, 0, 0, 'Language: Darnassian'),(6, 3, 115, 300, 300, 'Language: Taurahe'),(6, 3, 118, 0, 0, 'Dual Wield'),(6, 3, 136, 0, 5, 'Staves'),(6, 3, 138, 0, 0, 'Language: Draconic'),(6, 3, 141, 0, 0, 'Language: Old Tongue'),(6, 3, 142, 0, 1, 'Survival'),(6, 3, 313, 0, 0, 'Language: Gnomish'),(6, 3, 413, 0, 0, 'Mail'),(6, 3, 414, 1, 1, 'Leather'),(6, 3, 45, 0, 5, 'Bows'),(6, 3, 163, 5, 5, 'Marksmanship'),(6, 3, 46, 1, 5, 'Guns'),(6, 3, 202, 0, 0, 'Engineering'),(6, 3, 356, 0, 0, 'Fishing'),(6, 3, 171, 0, 0, 'Alchemy'),(6, 3, 182, 0, 0, 'Herbalism'),(6, 3, 129, 0, 0, 'First Aid'),(6, 3, 185, 0, 0, 'Cooking'),(6, 3, 261, 0, 5, 'Beast Training'),(6, 3, 162, 1, 5, 'Unarmed'),(6, 7, 164, 0, 0, 'Blacksmithing'),(6, 7, 172, 0, 5, 'Two-Handed Axes'),(6, 7, 173, 0, 5, 'Daggers'),(6, 7, 183, 5, 5, 'GENERIC (DND)'),(6, 7, 197, 0, 0, 'Tailoring'),(6, 7, 393, 0, 0, 'Skinning'),(6, 7, 109, 300, 300, 'Language: Orcish'),(6, 7, 124, 5, 5, 'Tauren Racial'),(6, 7, 433, 1, 1, 'Shield'),(6, 7, 137, 0, 0, 'Language: Thalassian'),(6, 7, 473, 0, 1, 'Fist Weapons'),(6, 7, 140, 0, 0, 'Language: Titan'),(6, 7, 149, 0, 0, 'Wolf Riding'),(6, 7, 98, 0, 0, 'Language: Common'),(6, 7, 315, 0, 0, 'Language: Troll'),(6, 7, 139, 0, 0, 'Language: Demon Tongue'),(6, 7, 165, 0, 0, 'Leatherworking'),(6, 7, 373, 0, 5, 'Enhancement'),(6, 7, 374, 5, 5, 'Restoration'),(6, 7, 375, 5, 5, 'Elemental Combat'),(6, 7, 415, 1, 1, 'Cloth'),(6, 7, 673, 0, 0, 'Language: Gutterspeak'),(6, 7, 95, 1, 5, 'Defense'),(6, 7, 333, 0, 0, 'Enchanting'),(6, 7, 186, 0, 0, 'Mining'),(6, 7, 713, 0, 0, 'Kodo Riding'),(6, 7, 44, 0, 5, 'Axes'),(6, 7, 111, 0, 0, 'Language: Dwarven'),(6, 7, 113, 0, 0, 'Language: Darnassian'),(6, 7, 115, 300, 300, 'Language: Taurahe'),(6, 7, 136, 1, 5, 'Staves'),(6, 7, 138, 0, 0, 'Language: Draconic'),(6, 7, 141, 0, 0, 'Language: Old Tongue'),(6, 7, 142, 0, 1, 'Survival'),(6, 7, 160, 0, 5, 'Two-Handed Maces'),(6, 7, 313, 0, 0, 'Language: Gnomish'),(6, 7, 413, 0, 0, 'Mail'),(6, 7, 414, 1, 1, 'Leather'),(6, 7, 202, 0, 0, 'Engineering'),(6, 7, 356, 0, 0, 'Fishing'),(6, 7, 171, 0, 0, 'Alchemy'),(6, 7, 54, 1, 5, 'Maces'),(6, 7, 182, 0, 0, 'Herbalism'),(6, 7, 129, 0, 0, 'First Aid'),(6, 7, 185, 0, 0, 'Cooking'),(6, 7, 162, 1, 5, 'Unarmed'),(7, 1, 148, 0, 0, 'Horse Riding'),(7, 1, 150, 0, 0, 'Tiger Riding'),(7, 1, 164, 0, 0, 'Blacksmithing'),(7, 1, 172, 0, 5, 'Two-Handed Axes'),(7, 1, 173, 1, 5, 'Daggers'),(7, 1, 183, 5, 5, 'GENERIC (DND)'),(7, 1, 197, 0, 0, 'Tailoring'),(7, 1, 226, 0, 5, 'Crossbows'),(7, 1, 393, 0, 0, 'Skinning'),(7, 1, 109, 0, 0, 'Language: Orcish'),(7, 1, 433, 1, 1, 'Shield'),(7, 1, 137, 0, 0, 'Language: Thalassian'),(7, 1, 473, 0, 1, 'Fist Weapons'),(7, 1, 140, 0, 0, 'Language: Titan'),(7, 1, 149, 0, 0, 'Wolf Riding'),(7, 1, 229, 0, 0, 'Polearms'),(7, 1, 257, 0, 5, 'Protection'),(7, 1, 293, 0, 0, 'Plate Mail'),(7, 1, 98, 300, 300, 'Language: Common'),(7, 1, 315, 0, 0, 'Language: Troll'),(7, 1, 139, 0, 0, 'Language: Demon Tongue'),(7, 1, 152, 0, 0, 'Ram Riding'),(7, 1, 165, 0, 0, 'Leatherworking'),(7, 1, 415, 1, 1, 'Cloth'),(7, 1, 256, 0, 5, 'Fury'),(7, 1, 673, 0, 0, 'Language: Gutterspeak'),(7, 1, 95, 1, 5, 'Defense'),(7, 1, 333, 0, 0, 'Enchanting'),(7, 1, 186, 0, 0, 'Mining'),(7, 1, 43, 1, 5, 'Swords'),(7, 1, 55, 0, 5, 'Two-Handed Swords'),(7, 1, 713, 0, 0, 'Kodo Riding'),(7, 1, 753, 5, 5, 'Racial - Gnome'),(7, 1, 176, 0, 5, 'Thrown'),(7, 1, 26, 5, 5, 'Arms'),(7, 1, 44, 0, 5, 'Axes'),(7, 1, 111, 0, 0, 'Language: Dwarven'),(7, 1, 113, 0, 0, 'Language: Darnassian'),(7, 1, 115, 0, 0, 'Language: Taurahe'),(7, 1, 118, 0, 0, 'Dual Wield'),(7, 1, 136, 0, 5, 'Staves'),(7, 1, 138, 0, 0, 'Language: Draconic'),(7, 1, 141, 0, 0, 'Language: Old Tongue'),(7, 1, 142, 0, 1, 'Survival'),(7, 1, 160, 0, 5, 'Two-Handed Maces'),(7, 1, 313, 300, 300, 'Language: Gnomish'),(7, 1, 413, 1, 1, 'Mail'),(7, 1, 414, 1, 1, 'Leather'),(7, 1, 45, 0, 5, 'Bows'),(7, 1, 46, 0, 5, 'Guns'),(7, 1, 533, 0, 0, 'Raptor Riding'),(7, 1, 553, 0, 0, 'Mechanostrider Piloting'),(7, 1, 554, 0, 0, 'Undead Horsemanship'),(7, 1, 202, 0, 0, 'Engineering'),(7, 1, 356, 0, 0, 'Fishing'),(7, 1, 171, 0, 0, 'Alchemy'),(7, 1, 54, 1, 5, 'Maces'),(7, 1, 182, 0, 0, 'Herbalism'),(7, 1, 129, 0, 0, 'First Aid'),(7, 1, 185, 0, 0, 'Cooking'),(7, 1, 162, 1, 5, 'Unarmed'),(7, 4, 148, 0, 0, 'Horse Riding'),(7, 4, 150, 0, 0, 'Tiger Riding'),(7, 4, 164, 0, 0, 'Blacksmithing'),(7, 4, 173, 1, 5, 'Daggers'),(7, 4, 183, 5, 5, 'GENERIC (DND)'),(7, 4, 197, 0, 0, 'Tailoring'),(7, 4, 226, 0, 5, 'Crossbows'),(7, 4, 393, 0, 0, 'Skinning'),(7, 4, 109, 0, 0, 'Language: Orcish'),(7, 4, 137, 0, 0, 'Language: Thalassian'),(7, 4, 473, 0, 1, 'Fist Weapons'),(7, 4, 140, 0, 0, 'Language: Titan'),(7, 4, 149, 0, 0, 'Wolf Riding'),(7, 4, 98, 300, 300, 'Language: Common'),(7, 4, 315, 0, 0, 'Language: Troll'),(7, 4, 139, 0, 0, 'Language: Demon Tongue'),(7, 4, 152, 0, 0, 'Ram Riding'),(7, 4, 165, 0, 0, 'Leatherworking'),(7, 4, 415, 1, 1, 'Cloth'),(7, 4, 633, 0, 5, 'Lockpicking'),(7, 4, 673, 0, 0, 'Language: Gutterspeak'),(7, 4, 95, 1, 5, 'Defense'),(7, 4, 333, 0, 0, 'Enchanting'),(7, 4, 186, 0, 0, 'Mining'),(7, 4, 43, 0, 5, 'Swords'),(7, 4, 713, 0, 0, 'Kodo Riding'),(7, 4, 753, 5, 5, 'Racial - Gnome'),(7, 4, 176, 1, 5, 'Thrown'),(7, 4, 38, 5, 5, 'Combat'),(7, 4, 39, 0, 5, 'Subtlety'),(7, 4, 111, 0, 0, 'Language: Dwarven'),(7, 4, 113, 0, 0, 'Language: Darnassian'),(7, 4, 115, 0, 0, 'Language: Taurahe'),(7, 4, 118, 0, 0, 'Dual Wield'),(7, 4, 138, 0, 0, 'Language: Draconic'),(7, 4, 141, 0, 0, 'Language: Old Tongue'),(7, 4, 142, 0, 1, 'Survival'),(7, 4, 253, 5, 5, 'Assassination'),(7, 4, 313, 300, 300, 'Language: Gnomish'),(7, 4, 414, 1, 1, 'Leather'),(7, 4, 45, 0, 5, 'Bows'),(7, 4, 46, 0, 5, 'Guns'),(7, 4, 533, 0, 0, 'Raptor Riding'),(7, 4, 553, 0, 0, 'Mechanostrider Piloting'),(7, 4, 554, 0, 0, 'Undead Horsemanship'),(7, 4, 202, 0, 0, 'Engineering'),(7, 4, 356, 0, 0, 'Fishing'),(7, 4, 171, 0, 0, 'Alchemy'),(7, 4, 54, 0, 5, 'Maces'),(7, 4, 182, 0, 0, 'Herbalism'),(7, 4, 129, 0, 0, 'First Aid'),(7, 4, 185, 0, 0, 'Cooking'),(7, 4, 40, 0, 5, 'Poisons'),(7, 4, 162, 1, 5, 'Unarmed'),(7, 8, 148, 0, 0, 'Horse Riding'),(7, 8, 150, 0, 0, 'Tiger Riding'),(7, 8, 164, 0, 0, 'Blacksmithing'),(7, 8, 173, 0, 5, 'Daggers'),(7, 8, 183, 5, 5, 'GENERIC (DND)'),(7, 8, 197, 0, 0, 'Tailoring'),(7, 8, 228, 1, 5, 'Wands'),(7, 8, 393, 0, 0, 'Skinning'),(7, 8, 109, 0, 0, 'Language: Orcish'),(7, 8, 137, 0, 0, 'Language: Thalassian'),(7, 8, 140, 0, 0, 'Language: Titan'),(7, 8, 149, 0, 0, 'Wolf Riding'),(7, 8, 8, 5, 5, 'Fire'),(7, 8, 98, 300, 300, 'Language: Common'),(7, 8, 315, 0, 0, 'Language: Troll'),(7, 8, 139, 0, 0, 'Language: Demon Tongue'),(7, 8, 152, 0, 0, 'Ram Riding'),(7, 8, 165, 0, 0, 'Leatherworking'),(7, 8, 415, 1, 1, 'Cloth'),(7, 8, 673, 0, 0, 'Language: Gutterspeak'),(7, 8, 95, 1, 5, 'Defense'),(7, 8, 333, 0, 0, 'Enchanting'),(7, 8, 186, 0, 0, 'Mining'),(7, 8, 43, 0, 5, 'Swords'),(7, 8, 713, 0, 0, 'Kodo Riding'),(7, 8, 753, 5, 5, 'Racial - Gnome'),(7, 8, 6, 5, 5, 'Frost'),(7, 8, 111, 0, 0, 'Language: Dwarven'),(7, 8, 113, 0, 0, 'Language: Darnassian'),(7, 8, 115, 0, 0, 'Language: Taurahe'),(7, 8, 136, 1, 5, 'Staves'),(7, 8, 138, 0, 0, 'Language: Draconic'),(7, 8, 141, 0, 0, 'Language: Old Tongue'),(7, 8, 142, 0, 1, 'Survival'),(7, 8, 313, 300, 300, 'Language: Gnomish'),(7, 8, 533, 0, 0, 'Raptor Riding'),(7, 8, 553, 0, 0, 'Mechanostrider Piloting'),(7, 8, 554, 0, 0, 'Undead Horsemanship'),(7, 8, 202, 0, 0, 'Engineering'),(7, 8, 356, 0, 0, 'Fishing'),(7, 8, 171, 0, 0, 'Alchemy'),(7, 8, 182, 0, 0, 'Herbalism'),(7, 8, 129, 0, 0, 'First Aid'),(7, 8, 185, 0, 0, 'Cooking'),(7, 8, 237, 0, 5, 'Arcane'),(7, 8, 162, 1, 5, 'Unarmed'),(7, 9, 148, 0, 0, 'Horse Riding'),(7, 9, 150, 0, 0, 'Tiger Riding'),(7, 9, 164, 0, 0, 'Blacksmithing'),(7, 9, 173, 1, 5, 'Daggers'),(7, 9, 183, 5, 5, 'GENERIC (DND)'),(7, 9, 197, 0, 0, 'Tailoring'),(7, 9, 228, 1, 5, 'Wands'),(7, 9, 393, 0, 0, 'Skinning'),(7, 9, 109, 0, 0, 'Language: Orcish'),(7, 9, 137, 0, 0, 'Language: Thalassian'),(7, 9, 140, 0, 0, 'Language: Titan'),(7, 9, 149, 0, 0, 'Wolf Riding'),(7, 9, 593, 5, 5, 'Destruction'),(7, 9, 98, 300, 300, 'Language: Common'),(7, 9, 315, 0, 0, 'Language: Troll'),(7, 9, 139, 0, 0, 'Language: Demon Tongue'),(7, 9, 152, 0, 0, 'Ram Riding'),(7, 9, 354, 5, 5, 'Demonology'),(7, 9, 355, 0, 5, 'Affliction'),(7, 9, 165, 0, 0, 'Leatherworking'),(7, 9, 415, 1, 1, 'Cloth'),(7, 9, 673, 0, 0, 'Language: Gutterspeak'),(7, 9, 95, 1, 5, 'Defense'),(7, 9, 333, 0, 0, 'Enchanting'),(7, 9, 186, 0, 0, 'Mining'),(7, 9, 43, 0, 5, 'Swords'),(7, 9, 713, 0, 0, 'Kodo Riding'),(7, 9, 753, 5, 5, 'Racial - Gnome'),(7, 9, 111, 0, 0, 'Language: Dwarven'),(7, 9, 113, 0, 0, 'Language: Darnassian'),(7, 9, 115, 0, 0, 'Language: Taurahe'),(7, 9, 136, 0, 5, 'Staves'),(7, 9, 138, 0, 0, 'Language: Draconic'),(7, 9, 141, 0, 0, 'Language: Old Tongue'),(7, 9, 142, 0, 1, 'Survival'),(7, 9, 313, 300, 300, 'Language: Gnomish'),(7, 9, 533, 0, 0, 'Raptor Riding'),(7, 9, 553, 0, 0, 'Mechanostrider Piloting'),(7, 9, 554, 0, 0, 'Undead Horsemanship'),(7, 9, 202, 0, 0, 'Engineering'),(7, 9, 356, 0, 0, 'Fishing'),(7, 9, 171, 0, 0, 'Alchemy'),(7, 9, 182, 0, 0, 'Herbalism'),(7, 9, 129, 0, 0, 'First Aid'),(7, 9, 185, 0, 0, 'Cooking'),(7, 9, 162, 1, 5, 'Unarmed'),(8, 1, 148, 0, 0, 'Horse Riding'),(8, 1, 150, 0, 0, 'Tiger Riding'),(8, 1, 164, 0, 0, 'Blacksmithing'),(8, 1, 172, 0, 5, 'Two-Handed Axes'),(8, 1, 173, 1, 5, 'Daggers'),(8, 1, 183, 5, 5, 'GENERIC (DND)'),(8, 1, 197, 0, 0, 'Tailoring'),(8, 1, 226, 0, 5, 'Crossbows'),(8, 1, 393, 0, 0, 'Skinning'),(8, 1, 109, 300, 300, 'Language: Orcish'),(8, 1, 433, 1, 1, 'Shield'),(8, 1, 137, 0, 0, 'Language: Thalassian'),(8, 1, 473, 0, 1, 'Fist Weapons'),(8, 1, 140, 0, 0, 'Language: Titan'),(8, 1, 149, 0, 0, 'Wolf Riding'),(8, 1, 229, 0, 0, 'Polearms'),(8, 1, 257, 0, 5, 'Protection'),(8, 1, 293, 0, 0, 'Plate Mail'),(8, 1, 98, 0, 0, 'Language: Common'),(8, 1, 315, 300, 300, 'Language: Troll'),(8, 1, 139, 0, 0, 'Language: Demon Tongue'),(8, 1, 152, 0, 0, 'Ram Riding'),(8, 1, 165, 0, 0, 'Leatherworking'),(8, 1, 415, 1, 1, 'Cloth'),(8, 1, 256, 0, 5, 'Fury'),(8, 1, 673, 0, 0, 'Language: Gutterspeak'),(8, 1, 95, 1, 5, 'Defense'),(8, 1, 333, 0, 0, 'Enchanting'),(8, 1, 186, 0, 0, 'Mining'),(8, 1, 43, 0, 5, 'Swords'),(8, 1, 55, 0, 5, 'Two-Handed Swords'),(8, 1, 713, 0, 0, 'Kodo Riding'),(8, 1, 733, 5, 5, 'Racial - Troll'),(8, 1, 176, 1, 5, 'Thrown'),(8, 1, 26, 5, 5, 'Arms'),(8, 1, 44, 1, 5, 'Axes'),(8, 1, 111, 0, 0, 'Language: Dwarven'),(8, 1, 113, 0, 0, 'Language: Darnassian'),(8, 1, 115, 0, 0, 'Language: Taurahe'),(8, 1, 118, 0, 0, 'Dual Wield'),(8, 1, 136, 0, 5, 'Staves'),(8, 1, 138, 0, 0, 'Language: Draconic'),(8, 1, 141, 0, 0, 'Language: Old Tongue'),(8, 1, 142, 0, 1, 'Survival'),(8, 1, 160, 0, 5, 'Two-Handed Maces'),(8, 1, 313, 0, 0, 'Language: Gnomish'),(8, 1, 413, 1, 1, 'Mail'),(8, 1, 414, 1, 1, 'Leather'),(8, 1, 45, 0, 5, 'Bows'),(8, 1, 46, 0, 5, 'Guns'),(8, 1, 533, 0, 0, 'Raptor Riding'),(8, 1, 554, 0, 0, 'Undead Horsemanship'),(8, 1, 202, 0, 0, 'Engineering'),(8, 1, 356, 0, 0, 'Fishing'),(8, 1, 171, 0, 0, 'Alchemy'),(8, 1, 54, 0, 5, 'Maces'),(8, 1, 182, 0, 0, 'Herbalism'),(8, 1, 129, 0, 0, 'First Aid'),(8, 1, 185, 0, 0, 'Cooking'),(8, 1, 162, 1, 5, 'Unarmed'),(8, 3, 148, 0, 0, 'Horse Riding'),(8, 3, 150, 0, 0, 'Tiger Riding'),(8, 3, 164, 0, 0, 'Blacksmithing'),(8, 3, 172, 0, 5, 'Two-Handed Axes'),(8, 3, 173, 0, 5, 'Daggers'),(8, 3, 183, 5, 5, 'GENERIC (DND)'),(8, 3, 197, 0, 0, 'Tailoring'),(8, 3, 226, 0, 5, 'Crossbows'),(8, 3, 393, 0, 0, 'Skinning'),(8, 3, 109, 300, 300, 'Language: Orcish'),(8, 3, 137, 0, 0, 'Language: Thalassian'),(8, 3, 473, 0, 1, 'Fist Weapons'),(8, 3, 140, 0, 0, 'Language: Titan'),(8, 3, 149, 0, 0, 'Wolf Riding'),(8, 3, 229, 0, 0, 'Polearms'),(8, 3, 98, 0, 0, 'Language: Common'),(8, 3, 315, 300, 300, 'Language: Troll'),(8, 3, 139, 0, 0, 'Language: Demon Tongue'),(8, 3, 152, 0, 0, 'Ram Riding'),(8, 3, 165, 0, 0, 'Leatherworking'),(8, 3, 415, 1, 1, 'Cloth'),(8, 3, 673, 0, 0, 'Language: Gutterspeak'),(8, 3, 95, 1, 5, 'Defense'),(8, 3, 333, 0, 0, 'Enchanting'),(8, 3, 186, 0, 0, 'Mining'),(8, 3, 43, 0, 5, 'Swords'),(8, 3, 55, 0, 5, 'Two-Handed Swords'),(8, 3, 713, 0, 0, 'Kodo Riding'),(8, 3, 733, 5, 5, 'Racial - Troll'),(8, 3, 176, 0, 5, 'Thrown'),(8, 3, 44, 1, 5, 'Axes'),(8, 3, 50, 0, 5, 'Beast Mastery'),(8, 3, 51, 5, 5, 'Survival'),(8, 3, 111, 0, 0, 'Language: Dwarven'),(8, 3, 113, 0, 0, 'Language: Darnassian'),(8, 3, 115, 0, 0, 'Language: Taurahe'),(8, 3, 118, 0, 0, 'Dual Wield'),(8, 3, 136, 0, 5, 'Staves'),(8, 3, 138, 0, 0, 'Language: Draconic'),(8, 3, 141, 0, 0, 'Language: Old Tongue'),(8, 3, 142, 0, 1, 'Survival'),(8, 3, 313, 0, 0, 'Language: Gnomish'),(8, 3, 413, 0, 0, 'Mail'),(8, 3, 414, 1, 1, 'Leather'),(8, 3, 45, 1, 5, 'Bows'),(8, 3, 163, 5, 5, 'Marksmanship'),(8, 3, 46, 0, 5, 'Guns'),(8, 3, 533, 0, 0, 'Raptor Riding'),(8, 3, 554, 0, 0, 'Undead Horsemanship'),(8, 3, 202, 0, 0, 'Engineering'),(8, 3, 356, 0, 0, 'Fishing'),(8, 3, 171, 0, 0, 'Alchemy'),(8, 3, 182, 0, 0, 'Herbalism'),(8, 3, 129, 0, 0, 'First Aid'),(8, 3, 185, 0, 0, 'Cooking'),(8, 3, 261, 0, 5, 'Beast Training'),(8, 3, 162, 1, 5, 'Unarmed'),(8, 4, 148, 0, 0, 'Horse Riding'),(8, 4, 150, 0, 0, 'Tiger Riding'),(8, 4, 164, 0, 0, 'Blacksmithing'),(8, 4, 173, 1, 5, 'Daggers'),(8, 4, 183, 5, 5, 'GENERIC (DND)'),(8, 4, 197, 0, 0, 'Tailoring'),(8, 4, 226, 0, 5, 'Crossbows'),(8, 4, 393, 0, 0, 'Skinning'),(8, 4, 109, 300, 300, 'Language: Orcish'),(8, 4, 137, 0, 0, 'Language: Thalassian'),(8, 4, 473, 0, 1, 'Fist Weapons'),(8, 4, 140, 0, 0, 'Language: Titan'),(8, 4, 149, 0, 0, 'Wolf Riding'),(8, 4, 98, 0, 0, 'Language: Common'),(8, 4, 315, 300, 300, 'Language: Troll'),(8, 4, 139, 0, 0, 'Language: Demon Tongue'),(8, 4, 152, 0, 0, 'Ram Riding'),(8, 4, 165, 0, 0, 'Leatherworking'),(8, 4, 415, 1, 1, 'Cloth'),(8, 4, 633, 0, 5, 'Lockpicking'),(8, 4, 673, 0, 0, 'Language: Gutterspeak'),(8, 4, 95, 1, 5, 'Defense'),(8, 4, 333, 0, 0, 'Enchanting'),(8, 4, 186, 0, 0, 'Mining'),(8, 4, 43, 0, 5, 'Swords'),(8, 4, 713, 0, 0, 'Kodo Riding'),(8, 4, 733, 5, 5, 'Racial - Troll'),(8, 4, 176, 1, 5, 'Thrown'),(8, 4, 38, 5, 5, 'Combat'),(8, 4, 39, 0, 5, 'Subtlety'),(8, 4, 111, 0, 0, 'Language: Dwarven'),(8, 4, 113, 0, 0, 'Language: Darnassian'),(8, 4, 115, 0, 0, 'Language: Taurahe'),(8, 4, 118, 0, 0, 'Dual Wield'),(8, 4, 138, 0, 0, 'Language: Draconic'),(8, 4, 141, 0, 0, 'Language: Old Tongue'),(8, 4, 142, 0, 1, 'Survival'),(8, 4, 253, 5, 5, 'Assassination'),(8, 4, 313, 0, 0, 'Language: Gnomish'),(8, 4, 414, 1, 1, 'Leather'),(8, 4, 45, 0, 5, 'Bows'),(8, 4, 46, 0, 5, 'Guns'),(8, 4, 533, 0, 0, 'Raptor Riding'),(8, 4, 554, 0, 0, 'Undead Horsemanship'),(8, 4, 202, 0, 0, 'Engineering'),(8, 4, 356, 0, 0, 'Fishing'),(8, 4, 171, 0, 0, 'Alchemy'),(8, 4, 54, 0, 5, 'Maces'),(8, 4, 182, 0, 0, 'Herbalism'),(8, 4, 129, 0, 0, 'First Aid'),(8, 4, 185, 0, 0, 'Cooking'),(8, 4, 40, 0, 5, 'Poisons'),(8, 4, 162, 1, 5, 'Unarmed'),(8, 5, 148, 0, 0, 'Horse Riding'),(8, 5, 150, 0, 0, 'Tiger Riding'),(8, 5, 164, 0, 0, 'Blacksmithing'),(8, 5, 173, 0, 5, 'Daggers'),(8, 5, 183, 5, 5, 'GENERIC (DND)'),(8, 5, 197, 0, 0, 'Tailoring'),(8, 5, 228, 1, 5, 'Wands'),(8, 5, 393, 0, 0, 'Skinning'),(8, 5, 109, 300, 300, 'Language: Orcish'),(8, 5, 137, 0, 0, 'Language: Thalassian'),(8, 5, 140, 0, 0, 'Language: Titan'),(8, 5, 149, 0, 0, 'Wolf Riding'),(8, 5, 98, 0, 0, 'Language: Common'),(8, 5, 315, 300, 300, 'Language: Troll'),(8, 5, 139, 0, 0, 'Language: Demon Tongue'),(8, 5, 152, 0, 0, 'Ram Riding'),(8, 5, 165, 0, 0, 'Leatherworking'),(8, 5, 415, 1, 1, 'Cloth'),(8, 5, 673, 0, 0, 'Language: Gutterspeak'),(8, 5, 95, 1, 5, 'Defense'),(8, 5, 333, 0, 0, 'Enchanting'),(8, 5, 186, 0, 0, 'Mining'),(8, 5, 713, 0, 0, 'Kodo Riding'),(8, 5, 733, 5, 5, 'Racial - Troll'),(8, 5, 56, 5, 5, 'Holy'),(8, 5, 78, 0, 5, 'Shadow Magic'),(8, 5, 111, 0, 0, 'Language: Dwarven'),(8, 5, 113, 0, 0, 'Language: Darnassian'),(8, 5, 115, 0, 0, 'Language: Taurahe'),(8, 5, 136, 0, 5, 'Staves'),(8, 5, 138, 0, 0, 'Language: Draconic'),(8, 5, 141, 0, 0, 'Language: Old Tongue'),(8, 5, 142, 0, 1, 'Survival'),(8, 5, 313, 0, 0, 'Language: Gnomish'),(8, 5, 613, 0, 5, 'Discipline'),(8, 5, 533, 0, 0, 'Raptor Riding'),(8, 5, 554, 0, 0, 'Undead Horsemanship'),(8, 5, 202, 0, 0, 'Engineering'),(8, 5, 356, 0, 0, 'Fishing'),(8, 5, 171, 0, 0, 'Alchemy'),(8, 5, 54, 1, 5, 'Maces'),(8, 5, 182, 0, 0, 'Herbalism'),(8, 5, 129, 0, 0, 'First Aid'),(8, 5, 185, 0, 0, 'Cooking'),(8, 5, 162, 1, 5, 'Unarmed'),(8, 7, 148, 0, 0, 'Horse Riding'),(8, 7, 150, 0, 0, 'Tiger Riding'),(8, 7, 164, 0, 0, 'Blacksmithing'),(8, 7, 172, 0, 5, 'Two-Handed Axes'),(8, 7, 173, 0, 5, 'Daggers'),(8, 7, 183, 5, 5, 'GENERIC (DND)'),(8, 7, 197, 0, 0, 'Tailoring'),(8, 7, 393, 0, 0, 'Skinning'),(8, 7, 109, 300, 300, 'Language: Orcish'),(8, 7, 433, 1, 1, 'Shield'),(8, 7, 137, 0, 0, 'Language: Thalassian'),(8, 7, 473, 0, 1, 'Fist Weapons'),(8, 7, 140, 0, 0, 'Language: Titan'),(8, 7, 149, 0, 0, 'Wolf Riding'),(8, 7, 98, 0, 0, 'Language: Common'),(8, 7, 315, 300, 300, 'Language: Troll'),(8, 7, 139, 0, 0, 'Language: Demon Tongue'),(8, 7, 152, 0, 0, 'Ram Riding'),(8, 7, 165, 0, 0, 'Leatherworking'),(8, 7, 373, 0, 5, 'Enhancement'),(8, 7, 374, 5, 5, 'Restoration'),(8, 7, 375, 5, 5, 'Elemental Combat'),(8, 7, 415, 1, 1, 'Cloth'),(8, 7, 673, 0, 0, 'Language: Gutterspeak'),(8, 7, 95, 1, 5, 'Defense'),(8, 7, 333, 0, 0, 'Enchanting'),(8, 7, 186, 0, 0, 'Mining'),(8, 7, 713, 0, 0, 'Kodo Riding'),(8, 7, 733, 5, 5, 'Racial - Troll'),(8, 7, 44, 0, 5, 'Axes'),(8, 7, 111, 0, 0, 'Language: Dwarven'),(8, 7, 113, 0, 0, 'Language: Darnassian'),(8, 7, 115, 0, 0, 'Language: Taurahe'),(8, 7, 136, 1, 5, 'Staves'),(8, 7, 138, 0, 0, 'Language: Draconic'),(8, 7, 141, 0, 0, 'Language: Old Tongue'),(8, 7, 142, 0, 1, 'Survival'),(8, 7, 160, 0, 5, 'Two-Handed Maces'),(8, 7, 313, 0, 0, 'Language: Gnomish'),(8, 7, 413, 0, 0, 'Mail'),(8, 7, 414, 1, 1, 'Leather'),(8, 7, 533, 0, 0, 'Raptor Riding'),(8, 7, 554, 0, 0, 'Undead Horsemanship'),(8, 7, 202, 0, 0, 'Engineering'),(8, 7, 356, 0, 0, 'Fishing'),(8, 7, 171, 0, 0, 'Alchemy'),(8, 7, 54, 1, 5, 'Maces'),(8, 7, 182, 0, 0, 'Herbalism'),(8, 7, 129, 0, 0, 'First Aid'),(8, 7, 185, 0, 0, 'Cooking'),(8, 7, 162, 1, 5, 'Unarmed'),(8, 8, 148, 0, 0, 'Horse Riding'),(8, 8, 150, 0, 0, 'Tiger Riding'),(8, 8, 164, 0, 0, 'Blacksmithing'),(8, 8, 173, 0, 5, 'Daggers'),(8, 8, 183, 5, 5, 'GENERIC (DND)'),(8, 8, 197, 0, 0, 'Tailoring'),(8, 8, 228, 1, 5, 'Wands'),(8, 8, 393, 0, 0, 'Skinning'),(8, 8, 109, 300, 300, 'Language: Orcish'),(8, 8, 137, 0, 0, 'Language: Thalassian'),(8, 8, 140, 0, 0, 'Language: Titan'),(8, 8, 149, 0, 0, 'Wolf Riding'),(8, 8, 8, 5, 5, 'Fire'),(8, 8, 98, 0, 0, 'Language: Common'),(8, 8, 315, 300, 300, 'Language: Troll'),(8, 8, 139, 0, 0, 'Language: Demon Tongue'),(8, 8, 152, 0, 0, 'Ram Riding'),(8, 8, 165, 0, 0, 'Leatherworking'),(8, 8, 415, 1, 1, 'Cloth'),(8, 8, 673, 0, 0, 'Language: Gutterspeak'),(8, 8, 95, 1, 5, 'Defense'),(8, 8, 333, 0, 0, 'Enchanting'),(8, 8, 186, 0, 0, 'Mining'),(8, 8, 43, 0, 5, 'Swords'),(8, 8, 713, 0, 0, 'Kodo Riding'),(8, 8, 733, 5, 5, 'Racial - Troll'),(8, 8, 6, 5, 5, 'Frost'),(8, 8, 111, 0, 0, 'Language: Dwarven'),(8, 8, 113, 0, 0, 'Language: Darnassian'),(8, 8, 115, 0, 0, 'Language: Taurahe'),(8, 8, 136, 1, 5, 'Staves'),(8, 8, 138, 0, 0, 'Language: Draconic'),(8, 8, 141, 0, 0, 'Language: Old Tongue'),(8, 8, 142, 0, 1, 'Survival'),(8, 8, 313, 0, 0, 'Language: Gnomish'),(8, 8, 533, 0, 0, 'Raptor Riding'),(8, 8, 554, 0, 0, 'Undead Horsemanship'),(8, 8, 202, 0, 0, 'Engineering'),(8, 8, 356, 0, 0, 'Fishing'),(8, 8, 171, 0, 0, 'Alchemy'),(8, 8, 182, 0, 0, 'Herbalism'),(8, 8, 129, 0, 0, 'First Aid'),(8, 8, 185, 0, 0, 'Cooking'),(8, 8, 237, 0, 5, 'Arcane'),(8, 8, 162, 1, 5, 'Unarmed');
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_skill` ENABLE KEYS */;

--
-- Table structure for table `playercreateinfo_spell`
--

DROP TABLE IF EXISTS `playercreateinfo_spell`;
CREATE TABLE `playercreateinfo_spell` (
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `Spell` bigint(20) unsigned NOT NULL default '0',
  `Note` varchar(255) default NULL,
  `Active` tinyint(3) unsigned NOT NULL default '1',
  PRIMARY KEY  (`race`,`class`,`Spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_spell`
--


/*!40000 ALTER TABLE `playercreateinfo_spell` DISABLE KEYS */;
LOCK TABLES `playercreateinfo_spell` WRITE;
INSERT INTO `playercreateinfo_spell` (race, class, Spell, Note, Active) VALUES (1, 1, 2382, 'Generic', 1),(1, 1, 3365, 'Opening', 1),(1, 1, 3050, 'Detect', 1),(1, 1, 6233, 'Closing', 1),(1, 1, 6246, 'Closing', 1),(1, 1, 6247, 'Opening', 1),(1, 1, 9125, 'Generic', 1),(1, 1, 2479, 'Honorless Target', 1),(1, 1, 6477, 'Opening', 1),(1, 1, 6478, 'Opening', 1),(1, 1, 6603, 'Attack', 1),(1, 1, 7266, 'Duel', 1),(1, 1, 7267, 'Grovel', 1),(1, 1, 7355, 'Stuck', 1),(1, 1, 8386, 'Attacking', 1),(1, 1, 21651, 'Opening', 1),(1, 1, 21652, 'Closing', 1),(1, 1, 22027, 'Remove Insignia', 1),(1, 1, 22810, 'Opening - No Text', 1),(1, 1, 20599, 'Diplomacy', 1),(1, 1, 20600, 'Perception', 1),(1, 1, 20597, 'Sword Specialization', 1),(1, 1, 20598, 'The Human Spirit', 1),(1, 1, 20864, 'Mace Specialization', 1),(1, 1, 9116, 'Shield', 1),(1, 1, 668, 'Language Common', 1),(1, 1, 9078, 'Cloth', 1),(1, 1, 204, 'Defense', 1),(1, 1, 81, 'Dodge', 1),(1, 1, 522, 'SPELLDEFENSE (DND)', 1),(1, 1, 107, 'Block', 1),(1, 1, 5301, 'Defensive State (DND)', 1),(1, 1, 201, 'One-Handed Swords', 1),(1, 1, 78, 'Heroic Strike', 1),(1, 1, 2457, 'Battle Stance', 1),(1, 1, 196, 'One-Handed Axes', 1),(1, 1, 8737, 'Mail', 1),(1, 1, 9077, 'Leather', 1),(1, 1, 198, 'One-Handed Maces', 1),(1, 1, 203, 'Unarmed', 1),(1, 2, 2382, 'Generic', 1),(1, 2, 3365, 'Opening', 1),(1, 2, 3050, 'Detect', 1),(1, 2, 6233, 'Closing', 1),(1, 2, 6246, 'Closing', 1),(1, 2, 6247, 'Opening', 1),(1, 2, 9125, 'Generic', 1),(1, 2, 2479, 'Honorless Target', 1),(1, 2, 6477, 'Opening', 1),(1, 2, 6478, 'Opening', 1),(1, 2, 6603, 'Attack', 1),(1, 2, 7266, 'Duel', 1),(1, 2, 7267, 'Grovel', 1),(1, 2, 7355, 'Stuck', 1),(1, 2, 8386, 'Attacking', 1),(1, 2, 21651, 'Opening', 1),(1, 2, 21652, 'Closing', 1),(1, 2, 22027, 'Remove Insignia', 1),(1, 2, 22810, 'Opening - No Text', 1),(1, 2, 20599, 'Diplomacy', 1),(1, 2, 20600, 'Perception', 1),(1, 2, 20597, 'Sword Specialization', 1),(1, 2, 20598, 'The Human Spirit', 1),(1, 2, 20864, 'Mace Specialization', 1),(1, 2, 9116, 'Shield', 1),(1, 2, 668, 'Language Common', 1),(1, 2, 9078, 'Cloth', 1),(1, 2, 635, 'Holy Light', 1),(1, 2, 20154, 'Seal of Righteousness', 1),(1, 2, 204, 'Defense', 1),(1, 2, 81, 'Dodge', 1),(1, 2, 522, 'SPELLDEFENSE (DND)', 1),(1, 2, 107, 'Block', 1),(1, 2, 199, 'Two-Handed Maces', 1),(1, 2, 8737, 'Mail', 1),(1, 2, 9077, 'Leather', 1),(1, 2, 198, 'One-Handed Maces', 1),(1, 2, 203, 'Unarmed', 1),(1, 4, 1180, 'Daggers', 1),(1, 4, 2382, 'Generic', 1),(1, 4, 3365, 'Opening', 1),(1, 4, 3050, 'Detect', 1),(1, 4, 6233, 'Closing', 1),(1, 4, 6246, 'Closing', 1),(1, 4, 6247, 'Opening', 1),(1, 4, 9125, 'Generic', 1),(1, 4, 2479, 'Honorless Target', 1),(1, 4, 6477, 'Opening', 1),(1, 4, 6478, 'Opening', 1),(1, 4, 6603, 'Attack', 1),(1, 4, 7266, 'Duel', 1),(1, 4, 7267, 'Grovel', 1),(1, 4, 7355, 'Stuck', 1),(1, 4, 8386, 'Attacking', 1),(1, 4, 21651, 'Opening', 1),(1, 4, 21652, 'Closing', 1),(1, 4, 22027, 'Remove Insignia', 1),(1, 4, 22810, 'Opening - No Text', 1),(1, 4, 20599, 'Diplomacy', 1),(1, 4, 20600, 'Perception', 1),(1, 4, 20597, 'Sword Specialization', 1),(1, 4, 20598, 'The Human Spirit', 1),(1, 4, 20864, 'Mace Specialization', 1),(1, 4, 668, 'Language Common', 1),(1, 4, 9078, 'Cloth', 1),(1, 4, 204, 'Defense', 1),(1, 4, 81, 'Dodge', 1),(1, 4, 522, 'SPELLDEFENSE (DND)', 1),(1, 4, 16092, 'Defensive State (DND)', 1),(1, 4, 2567, 'Thrown', 1),(1, 4, 2764, 'Throw', 1),(1, 4, 1752, 'Sinister Strike', 1),(1, 4, 21184, 'Rogue Passive (DND)', 1),(1, 4, 2098, 'Eviscerate', 1),(1, 4, 9077, 'Leather', 1),(1, 4, 203, 'Unarmed', 1),(1, 5, 2382, 'Generic', 1),(1, 5, 3365, 'Opening', 1),(1, 5, 3050, 'Detect', 1),(1, 5, 6233, 'Closing', 1),(1, 5, 6246, 'Closing', 1),(1, 5, 6247, 'Opening', 1),(1, 5, 9125, 'Generic', 1),(1, 5, 2479, 'Honorless Target', 1),(1, 5, 6477, 'Opening', 1),(1, 5, 6478, 'Opening', 1),(1, 5, 6603, 'Attack', 1),(1, 5, 7266, 'Duel', 1),(1, 5, 7267, 'Grovel', 1),(1, 5, 7355, 'Stuck', 1),(1, 5, 8386, 'Attacking', 1),(1, 5, 21651, 'Opening', 1),(1, 5, 21652, 'Closing', 1),(1, 5, 22027, 'Remove Insignia', 1),(1, 5, 22810, 'Opening - No Text', 1),(1, 5, 5009, 'Wands', 1),(1, 5, 5019, 'Shoot', 1),(1, 5, 20599, 'Diplomacy', 1),(1, 5, 20600, 'Perception', 1),(1, 5, 20597, 'Sword Specialization', 1),(1, 5, 20598, 'The Human Spirit', 1),(1, 5, 20864, 'Mace Specialization', 1),(1, 5, 668, 'Language Common', 1),(1, 5, 9078, 'Cloth', 1),(1, 5, 204, 'Defense', 1),(1, 5, 81, 'Dodge', 1),(1, 5, 522, 'SPELLDEFENSE (DND)', 1),(1, 5, 2050, 'Lesser Heal', 1),(1, 5, 585, 'Smite', 1),(1, 5, 198, 'One-Handed Maces', 1),(1, 5, 203, 'Unarmed', 1),(1, 8, 2382, 'Generic', 1),(1, 8, 3365, 'Opening', 1),(1, 8, 3050, 'Detect', 1),(1, 8, 6233, 'Closing', 1),(1, 8, 6246, 'Closing', 1),(1, 8, 6247, 'Opening', 1),(1, 8, 9125, 'Generic', 1),(1, 8, 2479, 'Honorless Target', 1),(1, 8, 6477, 'Opening', 1),(1, 8, 6478, 'Opening', 1),(1, 8, 6603, 'Attack', 1),(1, 8, 7266, 'Duel', 1),(1, 8, 7267, 'Grovel', 1),(1, 8, 7355, 'Stuck', 1),(1, 8, 8386, 'Attacking', 1),(1, 8, 21651, 'Opening', 1),(1, 8, 21652, 'Closing', 1),(1, 8, 22027, 'Remove Insignia', 1),(1, 8, 22810, 'Opening - No Text', 1),(1, 8, 5009, 'Wands', 1),(1, 8, 5019, 'Shoot', 1),(1, 8, 20599, 'Diplomacy', 1),(1, 8, 20600, 'Perception', 1),(1, 8, 20597, 'Sword Specialization', 1),(1, 8, 20598, 'The Human Spirit', 1),(1, 8, 20864, 'Mace Specialization', 1),(1, 8, 133, 'Fireball', 1),(1, 8, 668, 'Language Common', 1),(1, 8, 9078, 'Cloth', 1),(1, 8, 204, 'Defense', 1),(1, 8, 81, 'Dodge', 1),(1, 8, 522, 'SPELLDEFENSE (DND)', 1),(1, 8, 168, 'Frost Armor', 1),(1, 8, 227, 'Staves', 1),(1, 8, 203, 'Unarmed', 1),(1, 9, 1180, 'Daggers', 1),(1, 9, 2382, 'Generic', 1),(1, 9, 3365, 'Opening', 1),(1, 9, 3050, 'Detect', 1),(1, 9, 6233, 'Closing', 1),(1, 9, 6246, 'Closing', 1),(1, 9, 6247, 'Opening', 1),(1, 9, 9125, 'Generic', 1),(1, 9, 2479, 'Honorless Target', 1),(1, 9, 6477, 'Opening', 1),(1, 9, 6478, 'Opening', 1),(1, 9, 6603, 'Attack', 1),(1, 9, 7266, 'Duel', 1),(1, 9, 7267, 'Grovel', 1),(1, 9, 7355, 'Stuck', 1),(1, 9, 8386, 'Attacking', 1),(1, 9, 21651, 'Opening', 1),(1, 9, 21652, 'Closing', 1),(1, 9, 22027, 'Remove Insignia', 1),(1, 9, 22810, 'Opening - No Text', 1),(1, 9, 5009, 'Wands', 1),(1, 9, 5019, 'Shoot', 1),(1, 9, 20599, 'Diplomacy', 1),(1, 9, 20600, 'Perception', 1),(1, 9, 20597, 'Sword Specialization', 1),(1, 9, 20598, 'The Human Spirit', 1),(1, 9, 20864, 'Mace Specialization', 1),(1, 9, 686, 'Shadow Bolt', 1),(1, 9, 668, 'Language Common', 1),(1, 9, 687, 'Demon Skin', 1),(1, 9, 9078, 'Cloth', 1),(1, 9, 204, 'Defense', 1),(1, 9, 81, 'Dodge', 1),(1, 9, 522, 'SPELLDEFENSE (DND)', 1),(1, 9, 203, 'Unarmed', 1),(2, 1, 197, 'Two-Handed Axes', 1),(2, 1, 2382, 'Generic', 1),(2, 1, 3365, 'Opening', 1),(2, 1, 3050, 'Detect', 1),(2, 1, 6233, 'Closing', 1),(2, 1, 6246, 'Closing', 1),(2, 1, 6247, 'Opening', 1),(2, 1, 9125, 'Generic', 1),(2, 1, 2479, 'Honorless Target', 1),(2, 1, 6477, 'Opening', 1),(2, 1, 6478, 'Opening', 1),(2, 1, 6603, 'Attack', 1),(2, 1, 7266, 'Duel', 1),(2, 1, 7267, 'Grovel', 1),(2, 1, 7355, 'Stuck', 1),(2, 1, 8386, 'Attacking', 1),(2, 1, 21651, 'Opening', 1),(2, 1, 21652, 'Closing', 1),(2, 1, 22027, 'Remove Insignia', 1),(2, 1, 22810, 'Opening - No Text', 1),(2, 1, 669, 'Language Orcish', 1),(2, 1, 9116, 'Shield', 1),(2, 1, 21563, 'Command', 1),(2, 1, 20572, 'Blood Fury', 1),(2, 1, 20573, 'Hardiness', 1),(2, 1, 20574, 'Axe Specialization', 1),(2, 1, 9078, 'Cloth', 1),(2, 1, 204, 'Defense', 1),(2, 1, 81, 'Dodge', 1),(2, 1, 522, 'SPELLDEFENSE (DND)', 1),(2, 1, 107, 'Block', 1),(2, 1, 5301, 'Defensive State (DND)', 1),(2, 1, 201, 'One-Handed Swords', 1),(2, 1, 78, 'Heroic Strike', 1),(2, 1, 2457, 'Battle Stance', 1),(2, 1, 196, 'One-Handed Axes', 1),(2, 1, 8737, 'Mail', 1),(2, 1, 9077, 'Leather', 1),(2, 1, 203, 'Unarmed', 1),(2, 3, 2382, 'Generic', 1),(2, 3, 3365, 'Opening', 1),(2, 3, 3050, 'Detect', 1),(2, 3, 6233, 'Closing', 1),(2, 3, 6246, 'Closing', 1),(2, 3, 6247, 'Opening', 1),(2, 3, 9125, 'Generic', 1),(2, 3, 2479, 'Honorless Target', 1),(2, 3, 6477, 'Opening', 1),(2, 3, 6478, 'Opening', 1),(2, 3, 6603, 'Attack', 1),(2, 3, 7266, 'Duel', 1),(2, 3, 7267, 'Grovel', 1),(2, 3, 7355, 'Stuck', 1),(2, 3, 8386, 'Attacking', 1),(2, 3, 21651, 'Opening', 1),(2, 3, 21652, 'Closing', 1),(2, 3, 22027, 'Remove Insignia', 1),(2, 3, 22810, 'Opening - No Text', 1),(2, 3, 669, 'Language Orcish', 1),(2, 3, 20572, 'Blood Fury', 1),(2, 3, 20573, 'Hardiness', 1),(2, 3, 20574, 'Axe Specialization', 1),(2, 3, 20576, 'Command', 1),(2, 3, 9078, 'Cloth', 1),(2, 3, 204, 'Defense', 1),(2, 3, 81, 'Dodge', 1),(2, 3, 522, 'SPELLDEFENSE (DND)', 1),(2, 3, 13358, 'Defensive State (DND)', 1),(2, 3, 24949, 'Defensive State 2 (DND)', 1),(2, 3, 196, 'One-Handed Axes', 1),(2, 3, 2973, 'Raptor Strike', 1),(2, 3, 9077, 'Leather', 1),(2, 3, 264, 'Bows', 1),(2, 3, 75, 'Auto Shot', 1),(2, 3, 203, 'Unarmed', 1),(2, 4, 1180, 'Daggers', 1),(2, 4, 2382, 'Generic', 1),(2, 4, 3365, 'Opening', 1),(2, 4, 3050, 'Detect', 1),(2, 4, 6233, 'Closing', 1),(2, 4, 6246, 'Closing', 1),(2, 4, 6247, 'Opening', 1),(2, 4, 9125, 'Generic', 1),(2, 4, 2479, 'Honorless Target', 1),(2, 4, 6477, 'Opening', 1),(2, 4, 6478, 'Opening', 1),(2, 4, 6603, 'Attack', 1),(2, 4, 7266, 'Duel', 1),(2, 4, 7267, 'Grovel', 1),(2, 4, 7355, 'Stuck', 1),(2, 4, 8386, 'Attacking', 1),(2, 4, 21651, 'Opening', 1),(2, 4, 21652, 'Closing', 1),(2, 4, 22027, 'Remove Insignia', 1),(2, 4, 22810, 'Opening - No Text', 1),(2, 4, 669, 'Language Orcish', 1),(2, 4, 21563, 'Command', 1),(2, 4, 20572, 'Blood Fury', 1),(2, 4, 20573, 'Hardiness', 1),(2, 4, 20574, 'Axe Specialization', 1),(2, 4, 9078, 'Cloth', 1),(2, 4, 204, 'Defense', 1),(2, 4, 81, 'Dodge', 1),(2, 4, 522, 'SPELLDEFENSE (DND)', 1),(2, 4, 16092, 'Defensive State (DND)', 1),(2, 4, 2567, 'Thrown', 1),(2, 4, 2764, 'Throw', 1),(2, 4, 1752, 'Sinister Strike', 1),(2, 4, 21184, 'Rogue Passive (DND)', 1),(2, 4, 2098, 'Eviscerate', 1),(2, 4, 9077, 'Leather', 1),(2, 4, 203, 'Unarmed', 1),(2, 7, 2382, 'Generic', 1),(2, 7, 3365, 'Opening', 1),(2, 7, 3050, 'Detect', 1),(2, 7, 6233, 'Closing', 1),(2, 7, 6246, 'Closing', 1),(2, 7, 6247, 'Opening', 1),(2, 7, 9125, 'Generic', 1),(2, 7, 2479, 'Honorless Target', 1),(2, 7, 6477, 'Opening', 1),(2, 7, 6478, 'Opening', 1),(2, 7, 6603, 'Attack', 1),(2, 7, 7266, 'Duel', 1),(2, 7, 7267, 'Grovel', 1),(2, 7, 7355, 'Stuck', 1),(2, 7, 8386, 'Attacking', 1),(2, 7, 21651, 'Opening', 1),(2, 7, 21652, 'Closing', 1),(2, 7, 22027, 'Remove Insignia', 1),(2, 7, 22810, 'Opening - No Text', 1),(2, 7, 669, 'Language Orcish', 1),(2, 7, 9116, 'Shield', 1),(2, 7, 21563, 'Command', 1),(2, 7, 20572, 'Blood Fury', 1),(2, 7, 20573, 'Hardiness', 1),(2, 7, 20574, 'Axe Specialization', 1),(2, 7, 331, 'Healing Wave', 1),(2, 7, 403, 'Lightning Bolt', 1),(2, 7, 9078, 'Cloth', 1),(2, 7, 204, 'Defense', 1),(2, 7, 81, 'Dodge', 1),(2, 7, 522, 'SPELLDEFENSE (DND)', 1),(2, 7, 107, 'Block', 1),(2, 7, 227, 'Staves', 1),(2, 7, 9077, 'Leather', 1),(2, 7, 198, 'One-Handed Maces', 1),(2, 7, 203, 'Unarmed', 1),(2, 9, 1180, 'Daggers', 1),(2, 9, 2382, 'Generic', 1),(2, 9, 3365, 'Opening', 1),(2, 9, 3050, 'Detect', 1),(2, 9, 6233, 'Closing', 1),(2, 9, 6246, 'Closing', 1),(2, 9, 6247, 'Opening', 1),(2, 9, 9125, 'Generic', 1),(2, 9, 2479, 'Honorless Target', 1),(2, 9, 6477, 'Opening', 1),(2, 9, 6478, 'Opening', 1),(2, 9, 6603, 'Attack', 1),(2, 9, 7266, 'Duel', 1),(2, 9, 7267, 'Grovel', 1),(2, 9, 7355, 'Stuck', 1),(2, 9, 8386, 'Attacking', 1),(2, 9, 21651, 'Opening', 1),(2, 9, 21652, 'Closing', 1),(2, 9, 22027, 'Remove Insignia', 1),(2, 9, 22810, 'Opening - No Text', 1),(2, 9, 5009, 'Wands', 1),(2, 9, 5019, 'Shoot', 1),(2, 9, 669, 'Language Orcish', 1),(2, 9, 686, 'Shadow Bolt', 1),(2, 9, 20572, 'Blood Fury', 1),(2, 9, 20573, 'Hardiness', 1),(2, 9, 20574, 'Axe Specialization', 1),(2, 9, 20575, 'Command', 1),(2, 9, 687, 'Demon Skin', 1),(2, 9, 9078, 'Cloth', 1),(2, 9, 204, 'Defense', 1),(2, 9, 81, 'Dodge', 1),(2, 9, 522, 'SPELLDEFENSE (DND)', 1),(2, 9, 203, 'Unarmed', 1),(3, 1, 197, 'Two-Handed Axes', 1),(3, 1, 2382, 'Generic', 1),(3, 1, 3365, 'Opening', 1),(3, 1, 3050, 'Detect', 1),(3, 1, 6233, 'Closing', 1),(3, 1, 6246, 'Closing', 1),(3, 1, 6247, 'Opening', 1),(3, 1, 9125, 'Generic', 1),(3, 1, 2479, 'Honorless Target', 1),(3, 1, 6477, 'Opening', 1),(3, 1, 6478, 'Opening', 1),(3, 1, 6603, 'Attack', 1),(3, 1, 7266, 'Duel', 1),(3, 1, 7267, 'Grovel', 1),(3, 1, 7355, 'Stuck', 1),(3, 1, 8386, 'Attacking', 1),(3, 1, 21651, 'Opening', 1),(3, 1, 21652, 'Closing', 1),(3, 1, 22027, 'Remove Insignia', 1),(3, 1, 22810, 'Opening - No Text', 1),(3, 1, 9116, 'Shield', 1),(3, 1, 668, 'Language Common', 1),(3, 1, 9078, 'Cloth', 1),(3, 1, 204, 'Defense', 1),(3, 1, 81, 'Dodge', 1),(3, 1, 522, 'SPELLDEFENSE (DND)', 1),(3, 1, 107, 'Block', 1),(3, 1, 5301, 'Defensive State (DND)', 1),(3, 1, 78, 'Heroic Strike', 1),(3, 1, 2457, 'Battle Stance', 1),(3, 1, 196, 'One-Handed Axes', 1),(3, 1, 2481, 'Find Treasure', 1),(3, 1, 20596, 'Frost Resistance', 1),(3, 1, 20595, 'Gun Specialization', 1),(3, 1, 20594, 'Stoneform', 1),(3, 1, 672, 'Language Dwarven', 1),(3, 1, 8737, 'Mail', 1),(3, 1, 9077, 'Leather', 1),(3, 1, 198, 'One-Handed Maces', 1),(3, 1, 203, 'Unarmed', 1),(3, 2, 2382, 'Generic', 1),(3, 2, 3365, 'Opening', 1),(3, 2, 3050, 'Detect', 1),(3, 2, 6233, 'Closing', 1),(3, 2, 6246, 'Closing', 1),(3, 2, 6247, 'Opening', 1),(3, 2, 9125, 'Generic', 1),(3, 2, 2479, 'Honorless Target', 1),(3, 2, 6477, 'Opening', 1),(3, 2, 6478, 'Opening', 1),(3, 2, 6603, 'Attack', 1),(3, 2, 7266, 'Duel', 1),(3, 2, 7267, 'Grovel', 1),(3, 2, 7355, 'Stuck', 1),(3, 2, 8386, 'Attacking', 1),(3, 2, 21651, 'Opening', 1),(3, 2, 21652, 'Closing', 1),(3, 2, 22027, 'Remove Insignia', 1),(3, 2, 22810, 'Opening - No Text', 1),(3, 2, 9116, 'Shield', 1),(3, 2, 668, 'Language Common', 1),(3, 2, 9078, 'Cloth', 1),(3, 2, 635, 'Holy Light', 1),(3, 2, 20154, 'Seal of Righteousness', 1),(3, 2, 204, 'Defense', 1),(3, 2, 81, 'Dodge', 1),(3, 2, 522, 'SPELLDEFENSE (DND)', 1),(3, 2, 107, 'Block', 1),(3, 2, 2481, 'Find Treasure', 1),(3, 2, 20596, 'Frost Resistance', 1),(3, 2, 20595, 'Gun Specialization', 1),(3, 2, 20594, 'Stoneform', 1),(3, 2, 672, 'Language Dwarven', 1),(3, 2, 199, 'Two-Handed Maces', 1),(3, 2, 8737, 'Mail', 1),(3, 2, 9077, 'Leather', 1),(3, 2, 198, 'One-Handed Maces', 1),(3, 2, 203, 'Unarmed', 1),(3, 3, 2382, 'Generic', 1),(3, 3, 3365, 'Opening', 1),(3, 3, 3050, 'Detect', 1),(3, 3, 6233, 'Closing', 1),(3, 3, 6246, 'Closing', 1),(3, 3, 6247, 'Opening', 1),(3, 3, 9125, 'Generic', 1),(3, 3, 2479, 'Honorless Target', 1),(3, 3, 6477, 'Opening', 1),(3, 3, 6478, 'Opening', 1),(3, 3, 6603, 'Attack', 1),(3, 3, 7266, 'Duel', 1),(3, 3, 7267, 'Grovel', 1),(3, 3, 7355, 'Stuck', 1),(3, 3, 8386, 'Attacking', 1),(3, 3, 21651, 'Opening', 1),(3, 3, 21652, 'Closing', 1),(3, 3, 22027, 'Remove Insignia', 1),(3, 3, 22810, 'Opening - No Text', 1),(3, 3, 668, 'Language Common', 1),(3, 3, 9078, 'Cloth', 1),(3, 3, 204, 'Defense', 1),(3, 3, 81, 'Dodge', 1),(3, 3, 522, 'SPELLDEFENSE (DND)', 1),(3, 3, 13358, 'Defensive State (DND)', 1),(3, 3, 24949, 'Defensive State 2 (DND)', 1),(3, 3, 196, 'One-Handed Axes', 1),(3, 3, 2973, 'Raptor Strike', 1),(3, 3, 2481, 'Find Treasure', 1),(3, 3, 20596, 'Frost Resistance', 1),(3, 3, 20595, 'Gun Specialization', 1),(3, 3, 20594, 'Stoneform', 1),(3, 3, 672, 'Language Dwarven', 1),(3, 3, 9077, 'Leather', 1),(3, 3, 75, 'Auto Shot', 1),(3, 3, 266, 'Guns', 1),(3, 3, 203, 'Unarmed', 1),(3, 4, 1180, 'Daggers', 1),(3, 4, 2382, 'Generic', 1),(3, 4, 3365, 'Opening', 1),(3, 4, 3050, 'Detect', 1),(3, 4, 6233, 'Closing', 1),(3, 4, 6246, 'Closing', 1),(3, 4, 6247, 'Opening', 1),(3, 4, 9125, 'Generic', 1),(3, 4, 2479, 'Honorless Target', 1),(3, 4, 6477, 'Opening', 1),(3, 4, 6478, 'Opening', 1),(3, 4, 6603, 'Attack', 1),(3, 4, 7266, 'Duel', 1),(3, 4, 7267, 'Grovel', 1),(3, 4, 7355, 'Stuck', 1),(3, 4, 8386, 'Attacking', 1),(3, 4, 21651, 'Opening', 1),(3, 4, 21652, 'Closing', 1),(3, 4, 22027, 'Remove Insignia', 1),(3, 4, 22810, 'Opening - No Text', 1),(3, 4, 668, 'Language Common', 1),(3, 4, 9078, 'Cloth', 1),(3, 4, 204, 'Defense', 1),(3, 4, 81, 'Dodge', 1),(3, 4, 522, 'SPELLDEFENSE (DND)', 1),(3, 4, 16092, 'Defensive State (DND)', 1),(3, 4, 2567, 'Thrown', 1),(3, 4, 2764, 'Throw', 1),(3, 4, 1752, 'Sinister Strike', 1),(3, 4, 21184, 'Rogue Passive (DND)', 1),(3, 4, 2481, 'Find Treasure', 1),(3, 4, 20596, 'Frost Resistance', 1),(3, 4, 20595, 'Gun Specialization', 1),(3, 4, 20594, 'Stoneform', 1),(3, 4, 672, 'Language Dwarven', 1),(3, 4, 2098, 'Eviscerate', 1),(3, 4, 9077, 'Leather', 1),(3, 4, 203, 'Unarmed', 1),(3, 5, 2382, 'Generic', 1),(3, 5, 3365, 'Opening', 1),(3, 5, 3050, 'Detect', 1),(3, 5, 6233, 'Closing', 1),(3, 5, 6246, 'Closing', 1),(3, 5, 6247, 'Opening', 1),(3, 5, 9125, 'Generic', 1),(3, 5, 2479, 'Honorless Target', 1),(3, 5, 6477, 'Opening', 1),(3, 5, 6478, 'Opening', 1),(3, 5, 6603, 'Attack', 1),(3, 5, 7266, 'Duel', 1),(3, 5, 7267, 'Grovel', 1),(3, 5, 7355, 'Stuck', 1),(3, 5, 8386, 'Attacking', 1),(3, 5, 21651, 'Opening', 1),(3, 5, 21652, 'Closing', 1),(3, 5, 22027, 'Remove Insignia', 1),(3, 5, 22810, 'Opening - No Text', 1),(3, 5, 5009, 'Wands', 1),(3, 5, 5019, 'Shoot', 1),(3, 5, 668, 'Language Common', 1),(3, 5, 9078, 'Cloth', 1),(3, 5, 204, 'Defense', 1),(3, 5, 81, 'Dodge', 1),(3, 5, 522, 'SPELLDEFENSE (DND)', 1),(3, 5, 2050, 'Lesser Heal', 1),(3, 5, 585, 'Smite', 1),(3, 5, 2481, 'Find Treasure', 1),(3, 5, 20596, 'Frost Resistance', 1),(3, 5, 20595, 'Gun Specialization', 1),(3, 5, 20594, 'Stoneform', 1),(3, 5, 672, 'Language Dwarven', 1),(3, 5, 198, 'One-Handed Maces', 1),(3, 5, 203, 'Unarmed', 1),(4, 1, 1180, 'Daggers', 1),(4, 1, 2382, 'Generic', 1),(4, 1, 3365, 'Opening', 1),(4, 1, 3050, 'Detect', 1),(4, 1, 6233, 'Closing', 1),(4, 1, 6246, 'Closing', 1),(4, 1, 6247, 'Opening', 1),(4, 1, 9125, 'Generic', 1),(4, 1, 2479, 'Honorless Target', 1),(4, 1, 6477, 'Opening', 1),(4, 1, 6478, 'Opening', 1),(4, 1, 6603, 'Attack', 1),(4, 1, 7266, 'Duel', 1),(4, 1, 7267, 'Grovel', 1),(4, 1, 7355, 'Stuck', 1),(4, 1, 8386, 'Attacking', 1),(4, 1, 21651, 'Opening', 1),(4, 1, 21652, 'Closing', 1),(4, 1, 22027, 'Remove Insignia', 1),(4, 1, 22810, 'Opening - No Text', 1),(4, 1, 9116, 'Shield', 1),(4, 1, 668, 'Language Common', 1),(4, 1, 9078, 'Cloth', 1),(4, 1, 204, 'Defense', 1),(4, 1, 81, 'Dodge', 1),(4, 1, 522, 'SPELLDEFENSE (DND)', 1),(4, 1, 107, 'Block', 1),(4, 1, 5301, 'Defensive State (DND)', 1),(4, 1, 201, 'One-Handed Swords', 1),(4, 1, 78, 'Heroic Strike', 1),(4, 1, 2457, 'Battle Stance', 1),(4, 1, 671, 'Language Darnassian', 1),(4, 1, 8737, 'Mail', 1),(4, 1, 9077, 'Leather', 1),(4, 1, 198, 'One-Handed Maces', 1),(4, 1, 20580, 'Shadowmeld', 1),(4, 1, 20583, 'Nature Resistance', 1),(4, 1, 20582, 'Quickness', 1),(4, 1, 20585, 'Wisp Spirit', 1),(4, 1, 21009, 'Shadowmeld Passive', 1),(4, 1, 203, 'Unarmed', 1),(4, 11, 1180, 'Daggers', 1),(4, 11, 2382, 'Generic', 1),(4, 11, 3365, 'Opening', 1),(4, 11, 3050, 'Detect', 1),(4, 11, 6233, 'Closing', 1),(4, 11, 6246, 'Closing', 1),(4, 11, 6247, 'Opening', 1),(4, 11, 9125, 'Generic', 1),(4, 11, 2479, 'Honorless Target', 1),(4, 11, 6477, 'Opening', 1),(4, 11, 6478, 'Opening', 1),(4, 11, 6603, 'Attack', 1),(4, 11, 7266, 'Duel', 1),(4, 11, 7267, 'Grovel', 1),(4, 11, 7355, 'Stuck', 1),(4, 11, 8386, 'Attacking', 1),(4, 11, 21651, 'Opening', 1),(4, 11, 21652, 'Closing', 1),(4, 11, 22027, 'Remove Insignia', 1),(4, 11, 22810, 'Opening - No Text', 1),(4, 11, 5176, 'Wrath', 1),(4, 11, 5185, 'Healing Touch', 1),(4, 11, 668, 'Language Common', 1),(4, 11, 9078, 'Cloth', 1),(4, 11, 204, 'Defense', 1),(4, 11, 81, 'Dodge', 1),(4, 11, 522, 'SPELLDEFENSE (DND)', 1),(4, 11, 671, 'Language Darnassian', 1),(4, 11, 227, 'Staves', 1),(4, 11, 9077, 'Leather', 1),(4, 11, 20580, 'Shadowmeld', 1),(4, 11, 20583, 'Nature Resistance', 1),(4, 11, 20582, 'Quickness', 1),(4, 11, 20585, 'Wisp Spirit', 1),(4, 11, 21009, 'Shadowmeld Passive', 1),(4, 11, 203, 'Unarmed', 1),(4, 3, 1180, 'Daggers', 1),(4, 3, 2382, 'Generic', 1),(4, 3, 3365, 'Opening', 1),(4, 3, 3050, 'Detect', 1),(4, 3, 6233, 'Closing', 1),(4, 3, 6246, 'Closing', 1),(4, 3, 6247, 'Opening', 1),(4, 3, 9125, 'Generic', 1),(4, 3, 2479, 'Honorless Target', 1),(4, 3, 6477, 'Opening', 1),(4, 3, 6478, 'Opening', 1),(4, 3, 6603, 'Attack', 1),(4, 3, 7266, 'Duel', 1),(4, 3, 7267, 'Grovel', 1),(4, 3, 7355, 'Stuck', 1),(4, 3, 8386, 'Attacking', 1),(4, 3, 21651, 'Opening', 1),(4, 3, 21652, 'Closing', 1),(4, 3, 22027, 'Remove Insignia', 1),(4, 3, 22810, 'Opening - No Text', 1),(4, 3, 668, 'Language Common', 1),(4, 3, 9078, 'Cloth', 1),(4, 3, 204, 'Defense', 1),(4, 3, 81, 'Dodge', 1),(4, 3, 522, 'SPELLDEFENSE (DND)', 1),(4, 3, 13358, 'Defensive State (DND)', 1),(4, 3, 24949, 'Defensive State 2 (DND)', 1),(4, 3, 2973, 'Raptor Strike', 1),(4, 3, 671, 'Language Darnassian', 1),(4, 3, 9077, 'Leather', 1),(4, 3, 264, 'Bows', 1),(4, 3, 75, 'Auto Shot', 1),(4, 3, 20580, 'Shadowmeld', 1),(4, 3, 20583, 'Nature Resistance', 1),(4, 3, 20582, 'Quickness', 1),(4, 3, 20585, 'Wisp Spirit', 1),(4, 3, 21009, 'Shadowmeld Passive', 1),(4, 3, 203, 'Unarmed', 1),(4, 4, 1180, 'Daggers', 1),(4, 4, 2382, 'Generic', 1),(4, 4, 3365, 'Opening', 1),(4, 4, 3050, 'Detect', 1),(4, 4, 6233, 'Closing', 1),(4, 4, 6246, 'Closing', 1),(4, 4, 6247, 'Opening', 1),(4, 4, 9125, 'Generic', 1),(4, 4, 2479, 'Honorless Target', 1),(4, 4, 6477, 'Opening', 1),(4, 4, 6478, 'Opening', 1),(4, 4, 6603, 'Attack', 1),(4, 4, 7266, 'Duel', 1),(4, 4, 7267, 'Grovel', 1),(4, 4, 7355, 'Stuck', 1),(4, 4, 8386, 'Attacking', 1),(4, 4, 21651, 'Opening', 1),(4, 4, 21652, 'Closing', 1),(4, 4, 22027, 'Remove Insignia', 1),(4, 4, 22810, 'Opening - No Text', 1),(4, 4, 668, 'Language Common', 1),(4, 4, 9078, 'Cloth', 1),(4, 4, 204, 'Defense', 1),(4, 4, 81, 'Dodge', 1),(4, 4, 522, 'SPELLDEFENSE (DND)', 1),(4, 4, 16092, 'Defensive State (DND)', 1),(4, 4, 2567, 'Thrown', 1),(4, 4, 2764, 'Throw', 1),(4, 4, 1752, 'Sinister Strike', 1),(4, 4, 21184, 'Rogue Passive (DND)', 1),(4, 4, 671, 'Language Darnassian', 1),(4, 4, 2098, 'Eviscerate', 1),(4, 4, 9077, 'Leather', 1),(4, 4, 20580, 'Shadowmeld', 1),(4, 4, 20583, 'Nature Resistance', 1),(4, 4, 20582, 'Quickness', 1),(4, 4, 20585, 'Wisp Spirit', 1),(4, 4, 21009, 'Shadowmeld Passive', 1),(4, 4, 203, 'Unarmed', 1),(4, 5, 2382, 'Generic', 1),(4, 5, 3365, 'Opening', 1),(4, 5, 3050, 'Detect', 1),(4, 5, 6233, 'Closing', 1),(4, 5, 6246, 'Closing', 1),(4, 5, 6247, 'Opening', 1),(4, 5, 9125, 'Generic', 1),(4, 5, 2479, 'Honorless Target', 1),(4, 5, 6477, 'Opening', 1),(4, 5, 6478, 'Opening', 1),(4, 5, 6603, 'Attack', 1),(4, 5, 7266, 'Duel', 1),(4, 5, 7267, 'Grovel', 1),(4, 5, 7355, 'Stuck', 1),(4, 5, 8386, 'Attacking', 1),(4, 5, 21651, 'Opening', 1),(4, 5, 21652, 'Closing', 1),(4, 5, 22027, 'Remove Insignia', 1),(4, 5, 22810, 'Opening - No Text', 1),(4, 5, 5009, 'Wands', 1),(4, 5, 5019, 'Shoot', 1),(4, 5, 668, 'Language Common', 1),(4, 5, 9078, 'Cloth', 1),(4, 5, 204, 'Defense', 1),(4, 5, 81, 'Dodge', 1),(4, 5, 522, 'SPELLDEFENSE (DND)', 1),(4, 5, 2050, 'Lesser Heal', 1),(4, 5, 585, 'Smite', 1),(4, 5, 671, 'Language Darnassian', 1),(4, 5, 198, 'One-Handed Maces', 1),(4, 5, 20580, 'Shadowmeld', 1),(4, 5, 20583, 'Nature Resistance', 1),(4, 5, 20582, 'Quickness', 1),(4, 5, 20585, 'Wisp Spirit', 1),(4, 5, 21009, 'Shadowmeld Passive', 1),(4, 5, 203, 'Unarmed', 1),(5, 1, 1180, 'Daggers', 1),(5, 1, 2382, 'Generic', 1),(5, 1, 3365, 'Opening', 1),(5, 1, 3050, 'Detect', 1),(5, 1, 6233, 'Closing', 1),(5, 1, 6246, 'Closing', 1),(5, 1, 6247, 'Opening', 1),(5, 1, 9125, 'Generic', 1),(5, 1, 2479, 'Honorless Target', 1),(5, 1, 6477, 'Opening', 1),(5, 1, 6478, 'Opening', 1),(5, 1, 6603, 'Attack', 1),(5, 1, 7266, 'Duel', 1),(5, 1, 7267, 'Grovel', 1),(5, 1, 7355, 'Stuck', 1),(5, 1, 8386, 'Attacking', 1),(5, 1, 21651, 'Opening', 1),(5, 1, 21652, 'Closing', 1),(5, 1, 22027, 'Remove Insignia', 1),(5, 1, 22810, 'Opening - No Text', 1),(5, 1, 5227, 'Underwater Breathing', 1),(5, 1, 7744, 'Will of the Forsaken', 1),(5, 1, 20577, 'Cannibalize', 1),(5, 1, 20579, 'Shadow Resistance', 1),(5, 1, 669, 'Language Orcish', 1),(5, 1, 9116, 'Shield', 1),(5, 1, 9078, 'Cloth', 1),(5, 1, 17737, 'Language Gutterspeak', 1),(5, 1, 204, 'Defense', 1),(5, 1, 81, 'Dodge', 1),(5, 1, 522, 'SPELLDEFENSE (DND)', 1),(5, 1, 107, 'Block', 1),(5, 1, 5301, 'Defensive State (DND)', 1),(5, 1, 201, 'One-Handed Swords', 1),(5, 1, 202, 'Two-Handed Swords', 1),(5, 1, 78, 'Heroic Strike', 1),(5, 1, 2457, 'Battle Stance', 1),(5, 1, 8737, 'Mail', 1),(5, 1, 9077, 'Leather', 1),(5, 1, 203, 'Unarmed', 1),(5, 4, 1180, 'Daggers', 1),(5, 4, 2382, 'Generic', 1),(5, 4, 3365, 'Opening', 1),(5, 4, 3050, 'Detect', 1),(5, 4, 6233, 'Closing', 1),(5, 4, 6246, 'Closing', 1),(5, 4, 6247, 'Opening', 1),(5, 4, 9125, 'Generic', 1),(5, 4, 2479, 'Honorless Target', 1),(5, 4, 6477, 'Opening', 1),(5, 4, 6478, 'Opening', 1),(5, 4, 6603, 'Attack', 1),(5, 4, 7266, 'Duel', 1),(5, 4, 7267, 'Grovel', 1),(5, 4, 7355, 'Stuck', 1),(5, 4, 8386, 'Attacking', 1),(5, 4, 21651, 'Opening', 1),(5, 4, 21652, 'Closing', 1),(5, 4, 22027, 'Remove Insignia', 1),(5, 4, 22810, 'Opening - No Text', 1),(5, 4, 5227, 'Underwater Breathing', 1),(5, 4, 7744, 'Will of the Forsaken', 1),(5, 4, 20577, 'Cannibalize', 1),(5, 4, 20579, 'Shadow Resistance', 1),(5, 4, 669, 'Language Orcish', 1),(5, 4, 9078, 'Cloth', 1),(5, 4, 17737, 'Language Gutterspeak', 1),(5, 4, 204, 'Defense', 1),(5, 4, 81, 'Dodge', 1),(5, 4, 522, 'SPELLDEFENSE (DND)', 1),(5, 4, 16092, 'Defensive State (DND)', 1),(5, 4, 2567, 'Thrown', 1),(5, 4, 2764, 'Throw', 1),(5, 4, 1752, 'Sinister Strike', 1),(5, 4, 21184, 'Rogue Passive (DND)', 1),(5, 4, 2098, 'Eviscerate', 1),(5, 4, 9077, 'Leather', 1),(5, 4, 203, 'Unarmed', 1),(5, 5, 2382, 'Generic', 1),(5, 5, 3365, 'Opening', 1),(5, 5, 3050, 'Detect', 1),(5, 5, 6233, 'Closing', 1),(5, 5, 6246, 'Closing', 1),(5, 5, 6247, 'Opening', 1),(5, 5, 9125, 'Generic', 1),(5, 5, 2479, 'Honorless Target', 1),(5, 5, 6477, 'Opening', 1),(5, 5, 6478, 'Opening', 1),(5, 5, 6603, 'Attack', 1),(5, 5, 7266, 'Duel', 1),(5, 5, 7267, 'Grovel', 1),(5, 5, 7355, 'Stuck', 1),(5, 5, 8386, 'Attacking', 1),(5, 5, 21651, 'Opening', 1),(5, 5, 21652, 'Closing', 1),(5, 5, 22027, 'Remove Insignia', 1),(5, 5, 22810, 'Opening - No Text', 1),(5, 5, 5227, 'Underwater Breathing', 1),(5, 5, 7744, 'Will of the Forsaken', 1),(5, 5, 20577, 'Cannibalize', 1),(5, 5, 20579, 'Shadow Resistance', 1),(5, 5, 5009, 'Wands', 1),(5, 5, 5019, 'Shoot', 1),(5, 5, 669, 'Language Orcish', 1),(5, 5, 9078, 'Cloth', 1),(5, 5, 17737, 'Language Gutterspeak', 1),(5, 5, 204, 'Defense', 1),(5, 5, 81, 'Dodge', 1),(5, 5, 522, 'SPELLDEFENSE (DND)', 1),(5, 5, 2050, 'Lesser Heal', 1),(5, 5, 585, 'Smite', 1),(5, 5, 198, 'One-Handed Maces', 1),(5, 5, 203, 'Unarmed', 1),(5, 8, 2382, 'Generic', 1),(5, 8, 3365, 'Opening', 1),(5, 8, 3050, 'Detect', 1),(5, 8, 6233, 'Closing', 1),(5, 8, 6246, 'Closing', 1),(5, 8, 6247, 'Opening', 1),(5, 8, 9125, 'Generic', 1),(5, 8, 2479, 'Honorless Target', 1),(5, 8, 6477, 'Opening', 1),(5, 8, 6478, 'Opening', 1),(5, 8, 6603, 'Attack', 1),(5, 8, 7266, 'Duel', 1),(5, 8, 7267, 'Grovel', 1),(5, 8, 7355, 'Stuck', 1),(5, 8, 8386, 'Attacking', 1),(5, 8, 21651, 'Opening', 1),(5, 8, 21652, 'Closing', 1),(5, 8, 22027, 'Remove Insignia', 1),(5, 8, 22810, 'Opening - No Text', 1),(5, 8, 5227, 'Underwater Breathing', 1),(5, 8, 7744, 'Will of the Forsaken', 1),(5, 8, 20577, 'Cannibalize', 1),(5, 8, 20579, 'Shadow Resistance', 1),(5, 8, 5009, 'Wands', 1),(5, 8, 5019, 'Shoot', 1),(5, 8, 669, 'Language Orcish', 1),(5, 8, 133, 'Fireball', 1),(5, 8, 9078, 'Cloth', 1),(5, 8, 17737, 'Language Gutterspeak', 1),(5, 8, 204, 'Defense', 1),(5, 8, 81, 'Dodge', 1),(5, 8, 522, 'SPELLDEFENSE (DND)', 1),(5, 8, 168, 'Frost Armor', 1),(5, 8, 227, 'Staves', 1),(5, 8, 203, 'Unarmed', 1),(5, 9, 1180, 'Daggers', 1),(5, 9, 2382, 'Generic', 1),(5, 9, 3365, 'Opening', 1),(5, 9, 3050, 'Detect', 1),(5, 9, 6233, 'Closing', 1),(5, 9, 6246, 'Closing', 1),(5, 9, 6247, 'Opening', 1),(5, 9, 9125, 'Generic', 1),(5, 9, 2479, 'Honorless Target', 1),(5, 9, 6477, 'Opening', 1),(5, 9, 6478, 'Opening', 1),(5, 9, 6603, 'Attack', 1),(5, 9, 7266, 'Duel', 1),(5, 9, 7267, 'Grovel', 1),(5, 9, 7355, 'Stuck', 1),(5, 9, 8386, 'Attacking', 1),(5, 9, 21651, 'Opening', 1),(5, 9, 21652, 'Closing', 1),(5, 9, 22027, 'Remove Insignia', 1),(5, 9, 22810, 'Opening - No Text', 1),(5, 9, 5227, 'Underwater Breathing', 1),(5, 9, 7744, 'Will of the Forsaken', 1),(5, 9, 20577, 'Cannibalize', 1),(5, 9, 20579, 'Shadow Resistance', 1),(5, 9, 5009, 'Wands', 1),(5, 9, 5019, 'Shoot', 1),(5, 9, 669, 'Language Orcish', 1),(5, 9, 686, 'Shadow Bolt', 1),(5, 9, 687, 'Demon Skin', 1),(5, 9, 9078, 'Cloth', 1),(5, 9, 17737, 'Language Gutterspeak', 1),(5, 9, 204, 'Defense', 1),(5, 9, 81, 'Dodge', 1),(5, 9, 522, 'SPELLDEFENSE (DND)', 1),(5, 9, 203, 'Unarmed', 1),(6, 1, 2382, 'Generic', 1),(6, 1, 3365, 'Opening', 1),(6, 1, 3050, 'Detect', 1),(6, 1, 6233, 'Closing', 1),(6, 1, 6246, 'Closing', 1),(6, 1, 6247, 'Opening', 1),(6, 1, 9125, 'Generic', 1),(6, 1, 2479, 'Honorless Target', 1),(6, 1, 6477, 'Opening', 1),(6, 1, 6478, 'Opening', 1),(6, 1, 6603, 'Attack', 1),(6, 1, 7266, 'Duel', 1),(6, 1, 7267, 'Grovel', 1),(6, 1, 7355, 'Stuck', 1),(6, 1, 8386, 'Attacking', 1),(6, 1, 21651, 'Opening', 1),(6, 1, 21652, 'Closing', 1),(6, 1, 22027, 'Remove Insignia', 1),(6, 1, 22810, 'Opening - No Text', 1),(6, 1, 669, 'Language Orcish', 1),(6, 1, 20549, 'War Stomp', 1),(6, 1, 20550, 'Endurance', 1),(6, 1, 20551, 'Nature Resistance', 1),(6, 1, 20552, 'Cultivation', 1),(6, 1, 9116, 'Shield', 1),(6, 1, 9078, 'Cloth', 1),(6, 1, 204, 'Defense', 1),(6, 1, 81, 'Dodge', 1),(6, 1, 522, 'SPELLDEFENSE (DND)', 1),(6, 1, 107, 'Block', 1),(6, 1, 5301, 'Defensive State (DND)', 1),(6, 1, 78, 'Heroic Strike', 1),(6, 1, 2457, 'Battle Stance', 1),(6, 1, 196, 'One-Handed Axes', 1),(6, 1, 670, 'Language Taurahe', 1),(6, 1, 199, 'Two-Handed Maces', 1),(6, 1, 8737, 'Mail', 1),(6, 1, 9077, 'Leather', 1),(6, 1, 198, 'One-Handed Maces', 1),(6, 1, 203, 'Unarmed', 1),(6, 11, 2382, 'Generic', 1),(6, 11, 3365, 'Opening', 1),(6, 11, 3050, 'Detect', 1),(6, 11, 6233, 'Closing', 1),(6, 11, 6246, 'Closing', 1),(6, 11, 6247, 'Opening', 1),(6, 11, 9125, 'Generic', 1),(6, 11, 2479, 'Honorless Target', 1),(6, 11, 6477, 'Opening', 1),(6, 11, 6478, 'Opening', 1),(6, 11, 6603, 'Attack', 1),(6, 11, 7266, 'Duel', 1),(6, 11, 7267, 'Grovel', 1),(6, 11, 7355, 'Stuck', 1),(6, 11, 8386, 'Attacking', 1),(6, 11, 21651, 'Opening', 1),(6, 11, 21652, 'Closing', 1),(6, 11, 22027, 'Remove Insignia', 1),(6, 11, 22810, 'Opening - No Text', 1),(6, 11, 5176, 'Wrath', 1),(6, 11, 669, 'Language Orcish', 1),(6, 11, 20549, 'War Stomp', 1),(6, 11, 20550, 'Endurance', 1),(6, 11, 20551, 'Nature Resistance', 1),(6, 11, 20552, 'Cultivation', 1),(6, 11, 5185, 'Healing Touch', 1),(6, 11, 9078, 'Cloth', 1),(6, 11, 204, 'Defense', 1),(6, 11, 81, 'Dodge', 1),(6, 11, 522, 'SPELLDEFENSE (DND)', 1),(6, 11, 670, 'Language Taurahe', 1),(6, 11, 227, 'Staves', 1),(6, 11, 9077, 'Leather', 1),(6, 11, 198, 'One-Handed Maces', 1),(6, 11, 203, 'Unarmed', 1),(6, 3, 2382, 'Generic', 1),(6, 3, 3365, 'Opening', 1),(6, 3, 3050, 'Detect', 1),(6, 3, 6233, 'Closing', 1),(6, 3, 6246, 'Closing', 1),(6, 3, 6247, 'Opening', 1),(6, 3, 9125, 'Generic', 1),(6, 3, 2479, 'Honorless Target', 1),(6, 3, 6477, 'Opening', 1),(6, 3, 6478, 'Opening', 1),(6, 3, 6603, 'Attack', 1),(6, 3, 7266, 'Duel', 1),(6, 3, 7267, 'Grovel', 1),(6, 3, 7355, 'Stuck', 1),(6, 3, 8386, 'Attacking', 1),(6, 3, 21651, 'Opening', 1),(6, 3, 21652, 'Closing', 1),(6, 3, 22027, 'Remove Insignia', 1),(6, 3, 22810, 'Opening - No Text', 1),(6, 3, 669, 'Language Orcish', 1),(6, 3, 20549, 'War Stomp', 1),(6, 3, 20550, 'Endurance', 1),(6, 3, 20551, 'Nature Resistance', 1),(6, 3, 20552, 'Cultivation', 1),(6, 3, 9078, 'Cloth', 1),(6, 3, 204, 'Defense', 1),(6, 3, 81, 'Dodge', 1),(6, 3, 522, 'SPELLDEFENSE (DND)', 1),(6, 3, 13358, 'Defensive State (DND)', 1),(6, 3, 24949, 'Defensive State 2 (DND)', 1),(6, 3, 196, 'One-Handed Axes', 1),(6, 3, 2973, 'Raptor Strike', 1),(6, 3, 670, 'Language Taurahe', 1),(6, 3, 9077, 'Leather', 1),(6, 3, 75, 'Auto Shot', 1),(6, 3, 266, 'Guns', 1),(6, 3, 203, 'Unarmed', 1),(6, 7, 2382, 'Generic', 1),(6, 7, 3365, 'Opening', 1),(6, 7, 3050, 'Detect', 1),(6, 7, 6233, 'Closing', 1),(6, 7, 6246, 'Closing', 1),(6, 7, 6247, 'Opening', 1),(6, 7, 9125, 'Generic', 1),(6, 7, 2479, 'Honorless Target', 1),(6, 7, 6477, 'Opening', 1),(6, 7, 6478, 'Opening', 1),(6, 7, 6603, 'Attack', 1),(6, 7, 7266, 'Duel', 1),(6, 7, 7267, 'Grovel', 1),(6, 7, 7355, 'Stuck', 1),(6, 7, 8386, 'Attacking', 1),(6, 7, 21651, 'Opening', 1),(6, 7, 21652, 'Closing', 1),(6, 7, 22027, 'Remove Insignia', 1),(6, 7, 22810, 'Opening - No Text', 1),(6, 7, 669, 'Language Orcish', 1),(6, 7, 20549, 'War Stomp', 1),(6, 7, 20550, 'Endurance', 1),(6, 7, 20551, 'Nature Resistance', 1),(6, 7, 20552, 'Cultivation', 1),(6, 7, 9116, 'Shield', 1),(6, 7, 331, 'Healing Wave', 1),(6, 7, 403, 'Lightning Bolt', 1),(6, 7, 9078, 'Cloth', 1),(6, 7, 204, 'Defense', 1),(6, 7, 81, 'Dodge', 1),(6, 7, 522, 'SPELLDEFENSE (DND)', 1),(6, 7, 107, 'Block', 1),(6, 7, 670, 'Language Taurahe', 1),(6, 7, 227, 'Staves', 1),(6, 7, 9077, 'Leather', 1),(6, 7, 198, 'One-Handed Maces', 1),(6, 7, 203, 'Unarmed', 1),(7, 1, 1180, 'Daggers', 1),(7, 1, 2382, 'Generic', 1),(7, 1, 3365, 'Opening', 1),(7, 1, 3050, 'Detect', 1),(7, 1, 6233, 'Closing', 1),(7, 1, 6246, 'Closing', 1),(7, 1, 6247, 'Opening', 1),(7, 1, 9125, 'Generic', 1),(7, 1, 2479, 'Honorless Target', 1),(7, 1, 6477, 'Opening', 1),(7, 1, 6478, 'Opening', 1),(7, 1, 6603, 'Attack', 1),(7, 1, 7266, 'Duel', 1),(7, 1, 7267, 'Grovel', 1),(7, 1, 7355, 'Stuck', 1),(7, 1, 8386, 'Attacking', 1),(7, 1, 21651, 'Opening', 1),(7, 1, 21652, 'Closing', 1),(7, 1, 22027, 'Remove Insignia', 1),(7, 1, 22810, 'Opening - No Text', 1),(7, 1, 9116, 'Shield', 1),(7, 1, 668, 'Language Common', 1),(7, 1, 9078, 'Cloth', 1),(7, 1, 204, 'Defense', 1),(7, 1, 81, 'Dodge', 1),(7, 1, 522, 'SPELLDEFENSE (DND)', 1),(7, 1, 107, 'Block', 1),(7, 1, 5301, 'Defensive State (DND)', 1),(7, 1, 201, 'One-Handed Swords', 1),(7, 1, 20589, 'Escape Artist', 1),(7, 1, 20591, 'Expansive Mind', 1),(7, 1, 20593, 'Engineering Specialization', 1),(7, 1, 20592, 'Arcane Resistance', 1),(7, 1, 78, 'Heroic Strike', 1),(7, 1, 2457, 'Battle Stance', 1),(7, 1, 7340, 'Language Gnomish', 1),(7, 1, 8737, 'Mail', 1),(7, 1, 9077, 'Leather', 1),(7, 1, 198, 'One-Handed Maces', 1),(7, 1, 203, 'Unarmed', 1),(7, 4, 1180, 'Daggers', 1),(7, 4, 2382, 'Generic', 1),(7, 4, 3365, 'Opening', 1),(7, 4, 3050, 'Detect', 1),(7, 4, 6233, 'Closing', 1),(7, 4, 6246, 'Closing', 1),(7, 4, 6247, 'Opening', 1),(7, 4, 9125, 'Generic', 1),(7, 4, 2479, 'Honorless Target', 1),(7, 4, 6477, 'Opening', 1),(7, 4, 6478, 'Opening', 1),(7, 4, 6603, 'Attack', 1),(7, 4, 7266, 'Duel', 1),(7, 4, 7267, 'Grovel', 1),(7, 4, 7355, 'Stuck', 1),(7, 4, 8386, 'Attacking', 1),(7, 4, 21651, 'Opening', 1),(7, 4, 21652, 'Closing', 1),(7, 4, 22027, 'Remove Insignia', 1),(7, 4, 22810, 'Opening - No Text', 1),(7, 4, 668, 'Language Common', 1),(7, 4, 9078, 'Cloth', 1),(7, 4, 204, 'Defense', 1),(7, 4, 81, 'Dodge', 1),(7, 4, 522, 'SPELLDEFENSE (DND)', 1),(7, 4, 16092, 'Defensive State (DND)', 1),(7, 4, 20589, 'Escape Artist', 1),(7, 4, 20591, 'Expansive Mind', 1),(7, 4, 20593, 'Engineering Specialization', 1),(7, 4, 20592, 'Arcane Resistance', 1),(7, 4, 2567, 'Thrown', 1),(7, 4, 2764, 'Throw', 1),(7, 4, 1752, 'Sinister Strike', 1),(7, 4, 21184, 'Rogue Passive (DND)', 1),(7, 4, 2098, 'Eviscerate', 1),(7, 4, 7340, 'Language Gnomish', 1),(7, 4, 9077, 'Leather', 1),(7, 4, 203, 'Unarmed', 1),(7, 8, 2382, 'Generic', 1),(7, 8, 3365, 'Opening', 1),(7, 8, 3050, 'Detect', 1),(7, 8, 6233, 'Closing', 1),(7, 8, 6246, 'Closing', 1),(7, 8, 6247, 'Opening', 1),(7, 8, 9125, 'Generic', 1),(7, 8, 2479, 'Honorless Target', 1),(7, 8, 6477, 'Opening', 1),(7, 8, 6478, 'Opening', 1),(7, 8, 6603, 'Attack', 1),(7, 8, 7266, 'Duel', 1),(7, 8, 7267, 'Grovel', 1),(7, 8, 7355, 'Stuck', 1),(7, 8, 8386, 'Attacking', 1),(7, 8, 21651, 'Opening', 1),(7, 8, 21652, 'Closing', 1),(7, 8, 22027, 'Remove Insignia', 1),(7, 8, 22810, 'Opening - No Text', 1),(7, 8, 5009, 'Wands', 1),(7, 8, 5019, 'Shoot', 1),(7, 8, 133, 'Fireball', 1),(7, 8, 668, 'Language Common', 1),(7, 8, 9078, 'Cloth', 1),(7, 8, 204, 'Defense', 1),(7, 8, 81, 'Dodge', 1),(7, 8, 522, 'SPELLDEFENSE (DND)', 1),(7, 8, 20589, 'Escape Artist', 1),(7, 8, 20591, 'Expansive Mind', 1),(7, 8, 20593, 'Engineering Specialization', 1),(7, 8, 20592, 'Arcane Resistance', 1),(7, 8, 168, 'Frost Armor', 1),(7, 8, 227, 'Staves', 1),(7, 8, 7340, 'Language Gnomish', 1),(7, 8, 203, 'Unarmed', 1),(7, 9, 1180, 'Daggers', 1),(7, 9, 2382, 'Generic', 1),(7, 9, 3365, 'Opening', 1),(7, 9, 3050, 'Detect', 1),(7, 9, 6233, 'Closing', 1),(7, 9, 6246, 'Closing', 1),(7, 9, 6247, 'Opening', 1),(7, 9, 9125, 'Generic', 1),(7, 9, 2479, 'Honorless Target', 1),(7, 9, 6477, 'Opening', 1),(7, 9, 6478, 'Opening', 1),(7, 9, 6603, 'Attack', 1),(7, 9, 7266, 'Duel', 1),(7, 9, 7267, 'Grovel', 1),(7, 9, 7355, 'Stuck', 1),(7, 9, 8386, 'Attacking', 1),(7, 9, 21651, 'Opening', 1),(7, 9, 21652, 'Closing', 1),(7, 9, 22027, 'Remove Insignia', 1),(7, 9, 22810, 'Opening - No Text', 1),(7, 9, 5009, 'Wands', 1),(7, 9, 5019, 'Shoot', 1),(7, 9, 686, 'Shadow Bolt', 1),(7, 9, 668, 'Language Common', 1),(7, 9, 687, 'Demon Skin', 1),(7, 9, 9078, 'Cloth', 1),(7, 9, 204, 'Defense', 1),(7, 9, 81, 'Dodge', 1),(7, 9, 522, 'SPELLDEFENSE (DND)', 1),(7, 9, 20589, 'Escape Artist', 1),(7, 9, 20591, 'Expansive Mind', 1),(7, 9, 20593, 'Engineering Specialization', 1),(7, 9, 20592, 'Arcane Resistance', 1),(7, 9, 7340, 'Language Gnomish', 1),(7, 9, 203, 'Unarmed', 1),(8, 1, 1180, 'Daggers', 1),(8, 1, 2382, 'Generic', 1),(8, 1, 3365, 'Opening', 1),(8, 1, 3050, 'Detect', 1),(8, 1, 6233, 'Closing', 1),(8, 1, 6246, 'Closing', 1),(8, 1, 6247, 'Opening', 1),(8, 1, 9125, 'Generic', 1),(8, 1, 2479, 'Honorless Target', 1),(8, 1, 6477, 'Opening', 1),(8, 1, 6478, 'Opening', 1),(8, 1, 6603, 'Attack', 1),(8, 1, 7266, 'Duel', 1),(8, 1, 7267, 'Grovel', 1),(8, 1, 7355, 'Stuck', 1),(8, 1, 8386, 'Attacking', 1),(8, 1, 21651, 'Opening', 1),(8, 1, 21652, 'Closing', 1),(8, 1, 22027, 'Remove Insignia', 1),(8, 1, 22810, 'Opening - No Text', 1),(8, 1, 669, 'Language Orcish', 1),(8, 1, 9116, 'Shield', 1),(8, 1, 7341, 'Language Troll', 1),(8, 1, 9078, 'Cloth', 1),(8, 1, 204, 'Defense', 1),(8, 1, 81, 'Dodge', 1),(8, 1, 522, 'SPELLDEFENSE (DND)', 1),(8, 1, 107, 'Block', 1),(8, 1, 5301, 'Defensive State (DND)', 1),(8, 1, 23301, 'Berserking', 1),(8, 1, 20555, 'Regeneration', 1),(8, 1, 20557, 'Beast Slaying', 1),(8, 1, 20558, 'Throwing Specialization', 1),(8, 1, 26290, 'Bow Specialization', 1),(8, 1, 26296, 'Berserking', 1),(8, 1, 2567, 'Thrown', 1),(8, 1, 2764, 'Throw', 1),(8, 1, 78, 'Heroic Strike', 1),(8, 1, 2457, 'Battle Stance', 1),(8, 1, 196, 'One-Handed Axes', 1),(8, 1, 8737, 'Mail', 1),(8, 1, 9077, 'Leather', 1),(8, 1, 203, 'Unarmed', 1),(8, 3, 2382, 'Generic', 1),(8, 3, 3365, 'Opening', 1),(8, 3, 3050, 'Detect', 1),(8, 3, 6233, 'Closing', 1),(8, 3, 6246, 'Closing', 1),(8, 3, 6247, 'Opening', 1),(8, 3, 9125, 'Generic', 1),(8, 3, 2479, 'Honorless Target', 1),(8, 3, 6477, 'Opening', 1),(8, 3, 6478, 'Opening', 1),(8, 3, 6603, 'Attack', 1),(8, 3, 7266, 'Duel', 1),(8, 3, 7267, 'Grovel', 1),(8, 3, 7355, 'Stuck', 1),(8, 3, 8386, 'Attacking', 1),(8, 3, 21651, 'Opening', 1),(8, 3, 21652, 'Closing', 1),(8, 3, 22027, 'Remove Insignia', 1),(8, 3, 22810, 'Opening - No Text', 1),(8, 3, 669, 'Language Orcish', 1),(8, 3, 7341, 'Language Troll', 1),(8, 3, 9078, 'Cloth', 1),(8, 3, 204, 'Defense', 1),(8, 3, 81, 'Dodge', 1),(8, 3, 522, 'SPELLDEFENSE (DND)', 1),(8, 3, 13358, 'Defensive State (DND)', 1),(8, 3, 24949, 'Defensive State 2 (DND)', 1),(8, 3, 23301, 'Berserking', 1),(8, 3, 20554, 'Berserking', 1),(8, 3, 20555, 'Regeneration', 1),(8, 3, 20557, 'Beast Slaying', 1),(8, 3, 20558, 'Throwing Specialization', 1),(8, 3, 26290, 'Bow Specialization', 1),(8, 3, 196, 'One-Handed Axes', 1),(8, 3, 2973, 'Raptor Strike', 1),(8, 3, 9077, 'Leather', 1),(8, 3, 264, 'Bows', 1),(8, 3, 75, 'Auto Shot', 1),(8, 3, 203, 'Unarmed', 1),(8, 4, 1180, 'Daggers', 1),(8, 4, 2382, 'Generic', 1),(8, 4, 3365, 'Opening', 1),(8, 4, 3050, 'Detect', 1),(8, 4, 6233, 'Closing', 1),(8, 4, 6246, 'Closing', 1),(8, 4, 6247, 'Opening', 1),(8, 4, 9125, 'Generic', 1),(8, 4, 2479, 'Honorless Target', 1),(8, 4, 6477, 'Opening', 1),(8, 4, 6478, 'Opening', 1),(8, 4, 6603, 'Attack', 1),(8, 4, 7266, 'Duel', 1),(8, 4, 7267, 'Grovel', 1),(8, 4, 7355, 'Stuck', 1),(8, 4, 8386, 'Attacking', 1),(8, 4, 21651, 'Opening', 1),(8, 4, 21652, 'Closing', 1),(8, 4, 22027, 'Remove Insignia', 1),(8, 4, 22810, 'Opening - No Text', 1),(8, 4, 669, 'Language Orcish', 1),(8, 4, 7341, 'Language Troll', 1),(8, 4, 9078, 'Cloth', 1),(8, 4, 204, 'Defense', 1),(8, 4, 81, 'Dodge', 1),(8, 4, 522, 'SPELLDEFENSE (DND)', 1),(8, 4, 16092, 'Defensive State (DND)', 1),(8, 4, 23301, 'Berserking', 1),(8, 4, 20555, 'Regeneration', 1),(8, 4, 20557, 'Beast Slaying', 1),(8, 4, 20558, 'Throwing Specialization', 1),(8, 4, 26290, 'Bow Specialization', 1),(8, 4, 26297, 'Berserking', 1),(8, 4, 2567, 'Thrown', 1),(8, 4, 2764, 'Throw', 1),(8, 4, 1752, 'Sinister Strike', 1),(8, 4, 21184, 'Rogue Passive (DND)', 1),(8, 4, 2098, 'Eviscerate', 1),(8, 4, 9077, 'Leather', 1),(8, 4, 203, 'Unarmed', 1),(8, 5, 2382, 'Generic', 1),(8, 5, 3365, 'Opening', 1),(8, 5, 3050, 'Detect', 1),(8, 5, 6233, 'Closing', 1),(8, 5, 6246, 'Closing', 1),(8, 5, 6247, 'Opening', 1),(8, 5, 9125, 'Generic', 1),(8, 5, 2479, 'Honorless Target', 1),(8, 5, 6477, 'Opening', 1),(8, 5, 6478, 'Opening', 1),(8, 5, 6603, 'Attack', 1),(8, 5, 7266, 'Duel', 1),(8, 5, 7267, 'Grovel', 1),(8, 5, 7355, 'Stuck', 1),(8, 5, 8386, 'Attacking', 1),(8, 5, 21651, 'Opening', 1),(8, 5, 21652, 'Closing', 1),(8, 5, 22027, 'Remove Insignia', 1),(8, 5, 22810, 'Opening - No Text', 1),(8, 5, 5009, 'Wands', 1),(8, 5, 5019, 'Shoot', 1),(8, 5, 669, 'Language Orcish', 1),(8, 5, 7341, 'Language Troll', 1),(8, 5, 9078, 'Cloth', 1),(8, 5, 204, 'Defense', 1),(8, 5, 81, 'Dodge', 1),(8, 5, 522, 'SPELLDEFENSE (DND)', 1),(8, 5, 23301, 'Berserking', 1),(8, 5, 20554, 'Berserking', 1),(8, 5, 20555, 'Regeneration', 1),(8, 5, 20557, 'Beast Slaying', 1),(8, 5, 20558, 'Throwing Specialization', 1),(8, 5, 26290, 'Bow Specialization', 1),(8, 5, 2050, 'Lesser Heal', 1),(8, 5, 585, 'Smite', 1),(8, 5, 198, 'One-Handed Maces', 1),(8, 5, 203, 'Unarmed', 1),(8, 7, 2382, 'Generic', 1),(8, 7, 3365, 'Opening', 1),(8, 7, 3050, 'Detect', 1),(8, 7, 6233, 'Closing', 1),(8, 7, 6246, 'Closing', 1),(8, 7, 6247, 'Opening', 1),(8, 7, 9125, 'Generic', 1),(8, 7, 2479, 'Honorless Target', 1),(8, 7, 6477, 'Opening', 1),(8, 7, 6478, 'Opening', 1),(8, 7, 6603, 'Attack', 1),(8, 7, 7266, 'Duel', 1),(8, 7, 7267, 'Grovel', 1),(8, 7, 7355, 'Stuck', 1),(8, 7, 8386, 'Attacking', 1),(8, 7, 21651, 'Opening', 1),(8, 7, 21652, 'Closing', 1),(8, 7, 22027, 'Remove Insignia', 1),(8, 7, 22810, 'Opening - No Text', 1),(8, 7, 669, 'Language Orcish', 1),(8, 7, 9116, 'Shield', 1),(8, 7, 7341, 'Language Troll', 1),(8, 7, 331, 'Healing Wave', 1),(8, 7, 403, 'Lightning Bolt', 1),(8, 7, 9078, 'Cloth', 1),(8, 7, 204, 'Defense', 1),(8, 7, 81, 'Dodge', 1),(8, 7, 522, 'SPELLDEFENSE (DND)', 1),(8, 7, 107, 'Block', 1),(8, 7, 23301, 'Berserking', 1),(8, 7, 20554, 'Berserking', 1),(8, 7, 20555, 'Regeneration', 1),(8, 7, 20557, 'Beast Slaying', 1),(8, 7, 20558, 'Throwing Specialization', 1),(8, 7, 26290, 'Bow Specialization', 1),(8, 7, 227, 'Staves', 1),(8, 7, 9077, 'Leather', 1),(8, 7, 198, 'One-Handed Maces', 1),(8, 7, 203, 'Unarmed', 1),(8, 8, 2382, 'Generic', 1),(8, 8, 3365, 'Opening', 1),(8, 8, 3050, 'Detect', 1),(8, 8, 6233, 'Closing', 1),(8, 8, 6246, 'Closing', 1),(8, 8, 6247, 'Opening', 1),(8, 8, 9125, 'Generic', 1),(8, 8, 2479, 'Honorless Target', 1),(8, 8, 6477, 'Opening', 1),(8, 8, 6478, 'Opening', 1),(8, 8, 6603, 'Attack', 1),(8, 8, 7266, 'Duel', 1),(8, 8, 7267, 'Grovel', 1),(8, 8, 7355, 'Stuck', 1),(8, 8, 8386, 'Attacking', 1),(8, 8, 21651, 'Opening', 1),(8, 8, 21652, 'Closing', 1),(8, 8, 22027, 'Remove Insignia', 1),(8, 8, 22810, 'Opening - No Text', 1),(8, 8, 5009, 'Wands', 1),(8, 8, 5019, 'Shoot', 1),(8, 8, 669, 'Language Orcish', 1),(8, 8, 133, 'Fireball', 1),(8, 8, 7341, 'Language Troll', 1),(8, 8, 9078, 'Cloth', 1),(8, 8, 204, 'Defense', 1),(8, 8, 81, 'Dodge', 1),(8, 8, 522, 'SPELLDEFENSE (DND)', 1),(8, 8, 23301, 'Berserking', 1),(8, 8, 20554, 'Berserking', 1),(8, 8, 20555, 'Regeneration', 1),(8, 8, 20557, 'Beast Slaying', 1),(8, 8, 20558, 'Throwing Specialization', 1),(8, 8, 26290, 'Bow Specialization', 1),(8, 8, 168, 'Frost Armor', 1),(8, 8, 227, 'Staves', 1),(8, 8, 203, 'Unarmed', 1),(4, 11, 3025, 'Cat Form Passive', 0),(4, 11, 3122, 'Tree Form Passive', 0),(4, 11, 5419, 'Travel Form Passive', 0),(4, 11, 5421, 'Aqua Form Passive', 0),(4, 11, 1178, 'Bear Form Passive', 0),(4, 11, 9635, 'Dire Bear Form Passive', 0),(4, 11, 24905, 'Moonkin Form Passive', 0),(6, 11, 3025, 'Cat Form Passive', 0),(6, 11, 3122, 'Tree Form Passive', 0),(6, 11, 5419, 'Travel Form Passive', 0),(6, 11, 5421, 'Aqua Form Passive', 0),(6, 11, 1178, 'Bear Form Passive', 0),(6, 11, 9635, 'Dire Bear Form Passive', 0),(6, 11, 24905, 'Moonkin Form Passive', 0),(1, 1, 7376, 'Defensive Stance Passive', 0),(1, 1, 7381, 'Berserker Stance Passive', 0),(2, 1, 7376, 'Defensive Stance Passive', 0),(2, 1, 7381, 'Berserker Stance Passive', 0),(3, 1, 7376, 'Defensive Stance Passive', 0),(3, 1, 7381, 'Berserker Stance Passive', 0),(4, 1, 7376, 'Defensive Stance Passive', 0),(4, 1, 7381, 'Berserker Stance Passive', 0),(5, 1, 7376, 'Defensive Stance Passive', 0),(5, 1, 7381, 'Berserker Stance Passive', 0),(6, 1, 7376, 'Defensive Stance Passive', 0),(6, 1, 7381, 'Berserker Stance Passive', 0),(7, 1, 7376, 'Defensive Stance Passive', 0),(7, 1, 7381, 'Berserker Stance Passive', 0),(8, 1, 7376, 'Defensive Stance Passive', 0),(8, 1, 7381, 'Berserker Stance Passive', 0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `playercreateinfo_spell` ENABLE KEYS */;

--
-- Table structure for table `quest_template`
--

DROP TABLE IF EXISTS `quest_template`;
CREATE TABLE `quest_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `ZoneId` int(11) unsigned NOT NULL default '0',
  `QuestSort` int(11) unsigned NOT NULL default '0',
  `MinLevel` int(11) unsigned NOT NULL default '0',
  `QuestLevel` int(11) unsigned NOT NULL default '0',
  `Type` int(11) unsigned NOT NULL default '0',
  `RequiredRaces` int(11) unsigned NOT NULL default '0',
  `RequiredClass` int(11) unsigned NOT NULL default '0',
  `RequiredSkill` int(11) unsigned NOT NULL default '0',
  `RequiredSkillValue` int(11) unsigned NOT NULL default '1',
  `RequiredRepFaction` int(11) unsigned NOT NULL default '0',
  `RequiredRepValue` int(11) unsigned NOT NULL default '0',
  `LimitTime` int(11) unsigned NOT NULL default '0',
  `SpecialFlags` int(11) unsigned NOT NULL default '0',
  `PrevQuestId` int(11) unsigned NOT NULL default '0',
  `NextQuestId` int(11) unsigned NOT NULL default '0',
  `ExclusiveGroup` int(11) unsigned NOT NULL default '0',
  `SrcItemId` int(11) unsigned NOT NULL default '0',
  `SrcItemCount` int(11) unsigned NOT NULL default '0',
  `SrcSpell` int(11) unsigned NOT NULL default '0',
  `Title` text,
  `Details` text,
  `Objectives` text,
  `CompletionText` text,
  `IncompleteText` text,
  `EndText` text,
  `ObjectiveText1` text,
  `ObjectiveText2` text,
  `ObjectiveText3` text,
  `ObjectiveText4` text,
  `ReqItemId1` int(11) unsigned NOT NULL default '0',
  `ReqItemId2` int(11) unsigned NOT NULL default '0',
  `ReqItemId3` int(11) unsigned NOT NULL default '0',
  `ReqItemId4` int(11) unsigned NOT NULL default '0',
  `ReqItemCount1` int(11) unsigned NOT NULL default '0',
  `ReqItemCount2` int(11) unsigned NOT NULL default '0',
  `ReqItemCount3` int(11) unsigned NOT NULL default '0',
  `ReqItemCount4` int(11) unsigned NOT NULL default '0',
  `ReqSourceId1` int(11) unsigned NOT NULL default '0' COMMENT 'Id of item to be used for ReqItemX creation. X = ReqSourceRef1',
  `ReqSourceId2` int(11) unsigned NOT NULL default '0' COMMENT 'Id of item to be used for ReqItemX creation. X = ReqSourceRef2',
  `ReqSourceId3` int(11) unsigned NOT NULL default '0' COMMENT 'Id of item to be used for ReqItemX creation. X = ReqSourceRef3',
  `ReqSourceId4` int(11) unsigned NOT NULL default '0' COMMENT 'Id of item to be used for ReqItemX creation. X = ReqSourceRef4',
  `ReqSourceRef1` int(11) unsigned NOT NULL default '0',
  `ReqSourceRef2` int(11) unsigned NOT NULL default '0',
  `ReqSourceRef3` int(11) unsigned NOT NULL default '0',
  `ReqSourceRef4` int(11) unsigned NOT NULL default '0', 
  `ReqCreatureOrGOId1` int(11) NOT NULL default '0',
  `ReqCreatureOrGOId2` int(11) NOT NULL default '0',
  `ReqCreatureOrGOId3` int(11) NOT NULL default '0',
  `ReqCreatureOrGOId4` int(11) NOT NULL default '0',
  `ReqCreatureOrGOCount1` int(11) unsigned NOT NULL default '0',
  `ReqCreatureOrGOCount2` int(11) unsigned NOT NULL default '0',
  `ReqCreatureOrGOCount3` int(11) unsigned NOT NULL default '0',
  `ReqCreatureOrGOCount4` int(11) unsigned NOT NULL default '0',
  `ReqSpellCast1` int(11) unsigned NOT NULL default '0',
  `ReqSpellCast2` int(11) unsigned NOT NULL default '0',
  `ReqSpellCast3` int(11) unsigned NOT NULL default '0',
  `ReqSpellCast4` int(11) unsigned NOT NULL default '0',
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
  `RewRepValue1` int(11) NOT NULL default '0',
  `RewRepValue2` int(11) NOT NULL default '0',
  `RewOrReqMoney` int(11) NOT NULL default '0',
  `RewXP` int(11) unsigned NOT NULL default '0',
  `RewSpell` int(11) unsigned NOT NULL default '0',
  `PointMapId` int(11) unsigned NOT NULL default '0',
  `PointX` float NOT NULL default '0',
  `PointY` float NOT NULL default '0',
  `PointOpt` int(2) unsigned NOT NULL default '0',
  `DetailsEmote` int(11) NOT NULL default '0', 
  `IncompleteEmote` int(11) NOT NULL default '0', 
  `CompleteEmote` int(11) NOT NULL default '0',
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
-- Table structure for table `skinning_loot_template`
--

DROP TABLE IF EXISTS `skinning_loot_template`;
CREATE TABLE `skinning_loot_template` (
  `entry` int(11) unsigned NOT NULL default '0',
  `item` int(11) unsigned NOT NULL default '0',
  `ChanceOrRef` float NOT NULL default '100',
  `QuestChanceOrGroup` tinyint(3) NOT NULL default '0',
  `maxcount` int(11) unsigned NOT NULL default '1',
  `quest_freeforall` int(1) unsigned NOT NULL default '1',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Loot System';

--
-- Dumping data for table `skinning_loot_template`
--

/*!40000 ALTER TABLE `skinning_loot_template` DISABLE KEYS */;
LOCK TABLES `skinning_loot_template` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `skinning_loot_template` ENABLE KEYS */;


/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

