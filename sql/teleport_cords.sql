-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:29 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `teleport_cords`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `teleport_cords`;
CREATE TABLE IF NOT EXISTS `teleport_cords` (
  `id` int(16) NOT NULL default '0',
  `name` char(255) NOT NULL default '',
  `mapId` int(16) NOT NULL default '0',
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `totrigger` int(16) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;
