-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:18 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `npc_options`
-- 
-- Creation: Aug 26, 2005 at 12:24 AM
-- Last update: Aug 26, 2005 at 09:48 AM
-- 

DROP TABLE IF EXISTS `npc_options`;
CREATE TABLE `npc_options` (
  `ID` int(11) NOT NULL default '0',
  `GOSSIP_ID` int(11) NOT NULL default '0',
  `TYPE` int(5) default NULL,
  `OPTION` text collate latin1_general_ci NOT NULL,
  `NPC_TEXT_NEXTID` int(11) default '0',
  `SPECIAL` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
