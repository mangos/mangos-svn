-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:17 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `inventory`
-- 
-- Creation: Aug 26, 2005 at 12:03 AM
-- Last update: Aug 26, 2005 at 12:03 AM
-- 

DROP TABLE IF EXISTS `inventory`;
CREATE TABLE `inventory` (
  `player_guid` bigint(20) NOT NULL default '0',
  `slot` int(11) NOT NULL default '0',
  `item_guid` bigint(20) NOT NULL default '0',
  PRIMARY KEY  (`item_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;
