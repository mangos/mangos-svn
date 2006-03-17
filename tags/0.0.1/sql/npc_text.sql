-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:25 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `npc_text`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `npc_text`;
CREATE TABLE IF NOT EXISTS `npc_text` (
  `ID` int(11) NOT NULL default '0',
  `TYPE_UNUSED` int(11) NOT NULL default '0',
  `TEXT` longtext NOT NULL,
  PRIMARY KEY  (`ID`)
) TYPE=MyISAM;
