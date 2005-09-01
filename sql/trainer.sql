-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:20 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `trainer`
-- 
-- Creation: Aug 26, 2005 at 12:29 AM
-- Last update: Aug 26, 2005 at 12:29 AM
-- 

DROP TABLE IF EXISTS `trainer`;
CREATE TABLE `trainer` (
  `GUID` int(32) unsigned NOT NULL default '0',
  `skillline1` int(32) unsigned NOT NULL default '0',
  `skillline2` int(32) unsigned NOT NULL default '0',
  `skillline3` int(32) unsigned NOT NULL default '0',
  `maxlvl` int(32) unsigned NOT NULL default '0',
  `class` int(32) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
