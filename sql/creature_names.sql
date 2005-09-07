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
-- Table structure for table `creature_names`
-- 
-- Creation: Aug 21, 2005 at 10:34 AM
-- Last update: Aug 21, 2005 at 10:38 AM
-- 

DROP TABLE IF EXISTS `creature_names`;
CREATE TABLE `creature_names` (
  `name_id` bigint(20) NOT NULL default '0',
  `creature_name` varchar(100) collate latin1_general_ci NOT NULL default '',
  `Subname` varchar(100) collate latin1_general_ci NOT NULL default '',
  `unk1` int(30) default '0',
  `type` int(30) default '0',
  `unk2` int(30) default '0',
  `unk3` int(30) default '0',
  `unk4` int(30) default '0',
  `displayid` int(30) default '0',
  PRIMARY KEY  (`name_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
