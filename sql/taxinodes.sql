-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:19 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `taxinodes`
-- 
-- Creation: Aug 26, 2005 at 12:26 AM
-- Last update: Aug 26, 2005 at 12:26 AM
-- 

DROP TABLE IF EXISTS `taxinodes`;
CREATE TABLE `taxinodes` (
  `ID` tinyint(3) unsigned NOT NULL auto_increment,
  `continent` tinyint(3) unsigned NOT NULL default '0',
  `x` float default NULL,
  `y` float default NULL,
  `z` float default NULL,
  `name` varchar(255) collate latin1_general_ci default NULL,
  `flags` mediumint(11) unsigned default NULL,
  `mount` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=59 ;
