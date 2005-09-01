-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:18 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `playercreateinfo`
-- 
-- Creation: Aug 26, 2005 at 12:30 AM
-- Last update: Aug 26, 2005 at 12:30 AM
-- 

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE `playercreateinfo` (
  `Index` tinyint(3) unsigned NOT NULL auto_increment,
  `race` tinyint(3) unsigned NOT NULL default '0',
  `class` tinyint(3) unsigned NOT NULL default '0',
  `mapID` mediumint(8) unsigned NOT NULL default '0',
  `zoneID` mediumint(8) unsigned NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `displayID` smallint(5) unsigned NOT NULL default '0',
  `BaseStrength` tinyint(3) unsigned NOT NULL default '0',
  `BaseAgility` tinyint(3) unsigned NOT NULL default '0',
  `BaseStamina` tinyint(3) unsigned NOT NULL default '0',
  `BaseIntellect` tinyint(3) unsigned NOT NULL default '0',
  `BaseSpirit` tinyint(3) unsigned NOT NULL default '0',
  `BaseHealth` mediumint(8) unsigned NOT NULL default '0',
  `BaseMana` mediumint(8) unsigned NOT NULL default '0',
  `BaseRage` mediumint(8) unsigned NOT NULL default '0',
  `BaseFocus` mediumint(8) unsigned NOT NULL default '0',
  `BaseEnergy` mediumint(8) unsigned NOT NULL default '0',
  `attackpower` mediumint(8) unsigned NOT NULL default '0',
  `mindmg` float NOT NULL default '0',
  `maxdmg` float NOT NULL default '0',
  `item1` mediumint(8) unsigned NOT NULL default '0',
  `item1_slot` tinyint(3) unsigned NOT NULL default '0',
  `item2` mediumint(8) unsigned NOT NULL default '0',
  `item2_slot` tinyint(3) unsigned NOT NULL default '0',
  `item3` mediumint(8) unsigned NOT NULL default '0',
  `item3_slot` tinyint(3) unsigned NOT NULL default '0',
  `item4` mediumint(8) unsigned NOT NULL default '0',
  `item4_slot` tinyint(3) unsigned NOT NULL default '0',
  `item5` mediumint(8) unsigned NOT NULL default '0',
  `item5_slot` tinyint(3) unsigned NOT NULL default '0',
  `item6` mediumint(8) unsigned NOT NULL default '0',
  `item6_slot` tinyint(3) unsigned NOT NULL default '0',
  `item7` mediumint(8) unsigned NOT NULL default '0',
  `item7_slot` tinyint(3) unsigned NOT NULL default '0',
  `item8` mediumint(8) unsigned NOT NULL default '0',
  `item8_slot` tinyint(3) unsigned NOT NULL default '0',
  `item9` mediumint(8) unsigned NOT NULL default '0',
  `item9_slot` tinyint(3) unsigned NOT NULL default '0',
  `item10` mediumint(8) unsigned NOT NULL default '0',
  `item10_slot` tinyint(3) unsigned NOT NULL default '0',
  `spell1` smallint(5) unsigned NOT NULL default '0',
  `spell2` smallint(5) unsigned NOT NULL default '0',
  `spell3` smallint(5) unsigned NOT NULL default '0',
  `spell4` smallint(5) unsigned NOT NULL default '0',
  `spell5` smallint(5) unsigned NOT NULL default '0',
  `spell6` smallint(5) unsigned NOT NULL default '0',
  `spell7` smallint(5) unsigned NOT NULL default '0',
  `spell8` smallint(5) unsigned NOT NULL default '0',
  `spell9` smallint(5) unsigned NOT NULL default '0',
  `spell10` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=44 ;
