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
-- Table structure for table `taxipathnodes`
-- 
-- Creation: Aug 26, 2005 at 12:26 AM
-- Last update: Aug 26, 2005 at 12:28 AM
-- 

DROP TABLE IF EXISTS `taxipathnodes`;
CREATE TABLE `taxipathnodes` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `path` smallint(5) unsigned default NULL,
  `index` tinyint(3) unsigned default NULL,
  `continent` tinyint(3) unsigned default NULL,
  `X` float default NULL,
  `Y` float default NULL,
  `Z` float default NULL,
  `unknown1` mediumint(8) unsigned default NULL,
  `unknown2` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
