-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:20 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `taxipath`
-- 
-- Creation: Aug 26, 2005 at 12:26 AM
-- Last update: Aug 26, 2005 at 12:26 AM
-- 

DROP TABLE IF EXISTS `taxipath`;
CREATE TABLE `taxipath` (
  `ID` smallint(5) unsigned NOT NULL default '0',
  `source` tinyint(3) unsigned default NULL,
  `destination` tinyint(3) unsigned default NULL,
  `price` mediumint(8) unsigned default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';
