-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 21, 2005 at 09:02 AM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `playercreateinfo`
-- 
-- Creation: Sep 21, 2005 at 08:58 AM
-- Last update: Sep 21, 2005 at 09:01 AM
-- 

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE IF NOT EXISTS `playercreateinfo` (
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
  PRIMARY KEY  (`Index`)
) TYPE=MyISAM AUTO_INCREMENT=44 ;
