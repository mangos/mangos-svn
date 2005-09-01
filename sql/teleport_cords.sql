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
-- Table structure for table `teleport_cords`
-- 
-- Creation: Aug 26, 2005 at 12:31 AM
-- Last update: Aug 26, 2005 at 12:31 AM
-- 

DROP TABLE IF EXISTS `teleport_cords`;
CREATE TABLE `teleport_cords` (
  `id` int(16) NOT NULL default '0',
  `name` char(255) NOT NULL default '',
  `mapId` int(16) NOT NULL default '0',
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `totrigger` int(16) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
