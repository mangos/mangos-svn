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
-- Table structure for table `npc_gossip`
-- 
-- Creation: Aug 26, 2005 at 12:23 AM
-- Last update: Aug 26, 2005 at 09:48 AM
-- 

DROP TABLE IF EXISTS `npc_gossip`;
CREATE TABLE `npc_gossip` (
  `ID` int(11) NOT NULL default '0',
  `NPC_GUID` int(11) NOT NULL default '0',
  `GOSSIP_TYPE` int(11) NOT NULL default '0',
  `TEXTID` int(30) NOT NULL default '0',
  `OPTION_COUNT` int(30) default NULL,
  PRIMARY KEY  (`ID`,`NPC_GUID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
