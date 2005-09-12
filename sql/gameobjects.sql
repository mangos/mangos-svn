-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 12, 2005 at 10:37 AM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `gameobjects`
-- 
-- Creation: Sep 12, 2005 at 10:14 AM
-- Last update: Sep 12, 2005 at 10:15 AM
-- Last check: Sep 12, 2005 at 10:19 AM
-- 

DROP TABLE IF EXISTS `gameobjects`;
CREATE TABLE IF NOT EXISTS `gameobjects` (
  `id` bigint(20) unsigned NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `zoneId` int(11) NOT NULL default '38',
  `mapId` int(11) NOT NULL default '0',
  `data` longtext NOT NULL,
  `name_id` bigint(20) NOT NULL default '0',
  `moverandom` int(11) NOT NULL default '1',
  `running` int(11) NOT NULL default '0',
  `name` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM COMMENT='InnoDB free: 18432 kB';
