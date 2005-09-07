-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:15 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creatureloot`
-- 
-- Creation: Aug 21, 2005 at 10:38 AM
-- Last update: Aug 25, 2005 at 11:43 PM
-- 

DROP TABLE IF EXISTS `creatureloot`;
CREATE TABLE `creatureloot` (
  `entryid` int(11) NOT NULL default '0',
  `itemid` int(11) NOT NULL default '0',
  `percentchance` float default NULL,
  PRIMARY KEY  (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
