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
-- Table structure for table `creaturespells`
-- 
-- Creation: Aug 25, 2005 at 11:48 PM
-- Last update: Aug 25, 2005 at 11:48 PM
-- 

DROP TABLE IF EXISTS `creaturespells`;
CREATE TABLE `creaturespells` (
  `entryid` int(11) NOT NULL default '0',
  `spellid` int(11) NOT NULL default '0',
  PRIMARY KEY  (`entryid`,`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
