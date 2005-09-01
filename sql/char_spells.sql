-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:15 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `char_spells`
-- 
-- Creation: Aug 21, 2005 at 10:31 AM
-- Last update: Aug 21, 2005 at 10:32 AM
-- 

DROP TABLE IF EXISTS `char_spells`;
CREATE TABLE `char_spells` (
  `id` bigint(20) unsigned zerofill NOT NULL auto_increment,
  `charId` bigint(20) unsigned NOT NULL default '0',
  `spellId` int(20) unsigned NOT NULL default '0',
  `slotId` int(11) unsigned default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=166 ;
