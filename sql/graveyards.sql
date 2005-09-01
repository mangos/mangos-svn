-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:17 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `graveyards`
-- 
-- Creation: Aug 26, 2005 at 12:03 AM
-- Last update: Aug 26, 2005 at 12:03 AM
-- 

DROP TABLE IF EXISTS `graveyards`;
CREATE TABLE `graveyards` (
  `ID` int(60) NOT NULL auto_increment,
  `X` float default NULL,
  `Y` float default NULL,
  `Z` float default NULL,
  `O` float default NULL,
  `zoneId` int(16) default NULL,
  `mapId` int(16) default NULL,
  `faction_id` int(32) unsigned default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1 ;
