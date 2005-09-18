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
-- Table structure for table `npc_options`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `npc_options`;
CREATE TABLE IF NOT EXISTS `npc_options` (
  `ID` int(11) NOT NULL auto_increment,
  `NPC_GUID` int(11) NOT NULL default '0',
  `ICON` int(5) default NULL,
  `OPTIONTEXT` text NOT NULL,
  `NPC_TEXT_ID` int(11) default '0',
  `SPECIAL` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) TYPE=MyISAM;
