-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:16 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creaturequestrelation`
-- 
-- Creation: Aug 25, 2005 at 11:43 PM
-- Last update: Aug 25, 2005 at 11:44 PM
-- 

DROP TABLE IF EXISTS `creaturequestrelation`;
CREATE TABLE `creaturequestrelation` (
  `Id` int(6) unsigned NOT NULL auto_increment,
  `questId` bigint(20) unsigned NOT NULL default '0',
  `creatureId` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='Maps which creatures have which quests; InnoDB free: 18432 k' AUTO_INCREMENT=5410 ;
