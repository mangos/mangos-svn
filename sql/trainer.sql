-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:30 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `trainer`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `trainer`;
CREATE TABLE IF NOT EXISTS `trainer` (
  `GUID` int(32) unsigned NOT NULL default '0',
  `skillline1` int(32) unsigned NOT NULL default '0',
  `skillline2` int(32) unsigned NOT NULL default '0',
  `skillline3` int(32) unsigned NOT NULL default '0',
  `skillline4` int(32) unsigned NOT NULL default '0',
  `skillline5` int(32) unsigned NOT NULL default '0',
  `skillline6` int(32) unsigned NOT NULL default '0',
  `skillline7` int(32) unsigned NOT NULL default '0',
  `skillline8` int(32) unsigned NOT NULL default '0',
  `skillline9` int(32) unsigned NOT NULL default '0',
  `skillline10` int(32) unsigned NOT NULL default '0',
  `skillline11` int(32) unsigned NOT NULL default '0',
  `skillline12` int(32) unsigned NOT NULL default '0',
  `skillline13` int(32) unsigned NOT NULL default '0',
  `skillline14` int(32) unsigned NOT NULL default '0',
  `skillline15` int(32) unsigned NOT NULL default '0',
  `skillline16` int(32) unsigned NOT NULL default '0',
  `skillline17` int(32) unsigned NOT NULL default '0',
  `skillline18` int(32) unsigned NOT NULL default '0',
  `skillline19` int(32) unsigned NOT NULL default '0',
  `skillline20` int(32) unsigned NOT NULL default '0',
  `maxlvl` int(32) unsigned NOT NULL default '0',
  `class` int(32) unsigned NOT NULL default '0'
) TYPE=MyISAM;
