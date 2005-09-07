-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:16 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creatures`
-- 
-- Creation: Aug 25, 2005 at 11:45 PM
-- Last update: Aug 25, 2005 at 11:47 PM
-- 

DROP TABLE IF EXISTS `creatures`;
CREATE TABLE `creatures` (
  `id` bigint(20) unsigned NOT NULL default '0',
  `positionX` float NOT NULL default '0',
  `positionY` float NOT NULL default '0',
  `positionZ` float NOT NULL default '0',
  `orientation` float NOT NULL default '0',
  `zoneId` int(11) NOT NULL default '38',
  `mapId` int(11) NOT NULL default '0',
  `data` longtext collate latin1_general_ci NOT NULL,
  `name_id` bigint(20) NOT NULL default '0',
  `moverandom` int(11) NOT NULL default '1',
  `running` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
