-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:22 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creature_names`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `creature_names`;
CREATE TABLE IF NOT EXISTS `creature_names` (
  `name_id` bigint(20) NOT NULL default '0',
  `creature_name` varchar(100) NOT NULL default '',
  `Subname` varchar(100) NOT NULL default '',
  `unk1` int(30) default '0',
  `type` int(30) default '0',
  `unk2` int(30) default '0',
  `unk3` int(30) default '0',
  `unk4` int(30) default '0',
  `displayid` int(30) default '0',
  PRIMARY KEY  (`name_id`)
) TYPE=MyISAM;
