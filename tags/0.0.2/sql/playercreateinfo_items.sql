-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 21, 2005 at 09:03 AM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `playercreateinfo_items`
-- 
-- Creation: Sep 20, 2005 at 12:14 PM
-- Last update: Sep 20, 2005 at 12:14 PM
-- 

DROP TABLE IF EXISTS `playercreateinfo_items`;
CREATE TABLE IF NOT EXISTS `playercreateinfo_items` (
  `createId` tinyint(3) unsigned NOT NULL default '0',
  `item` mediumint(8) unsigned NOT NULL default '0',
  `item_slot` tinyint(3) unsigned NOT NULL default '0'
) TYPE=MyISAM;
