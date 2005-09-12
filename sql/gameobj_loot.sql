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
-- Table structure for table `gameobj_loot`
-- 
-- Creation: Sep 12, 2005 at 10:17 AM
-- Last update: Sep 12, 2005 at 10:17 AM
-- Last check: Sep 12, 2005 at 10:19 AM
-- 

DROP TABLE IF EXISTS `gameobj_loot`;
CREATE TABLE IF NOT EXISTS `gameobj_loot` (
  `entryid` int(11) NOT NULL default '0',
  `itemid` int(11) NOT NULL default '0',
  `percentchance` float default NULL,
  PRIMARY KEY  (`entryid`,`itemid`)
) TYPE=MyISAM;
