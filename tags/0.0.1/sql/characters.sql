-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:09 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `characters`
-- 

DROP TABLE IF EXISTS `characters`;
CREATE TABLE IF NOT EXISTS `characters` (
  `guid` int(6) unsigned NOT NULL default '0',
  `acct` bigint(20) unsigned NOT NULL default '0',
  `data` longtext NOT NULL,
  `name` varchar(21) NOT NULL default '',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `mapId` mediumint(8) unsigned NOT NULL default '0',
  `zoneId` mediumint(8) unsigned NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `taximask` longtext NOT NULL,
  PRIMARY KEY  (`guid`)
) TYPE=MyISAM;
