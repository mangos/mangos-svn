-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:10 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `char_spells`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `char_spells`;
CREATE TABLE IF NOT EXISTS `char_spells` (
  `id` bigint(20) unsigned zerofill NOT NULL auto_increment,
  `charId` bigint(20) unsigned NOT NULL default '0',
  `spellId` int(20) unsigned NOT NULL default '0',
  `slotId` int(11) unsigned default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
