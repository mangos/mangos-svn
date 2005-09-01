-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:16 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creatures_mov`
-- 
-- Creation: Aug 25, 2005 at 11:47 PM
-- Last update: Aug 25, 2005 at 11:47 PM
-- 

DROP TABLE IF EXISTS `creatures_mov`;
CREATE TABLE `creatures_mov` (
  `id` bigint(20) NOT NULL auto_increment,
  `creatureId` bigint(20) NOT NULL default '0',
  `X` float NOT NULL default '0',
  `Y` float NOT NULL default '0',
  `Z` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1 ;
