-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:21 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `creatures_mov`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `creatures_mov`;
CREATE TABLE IF NOT EXISTS `creatures_mov` (
  `id` bigint(20) NOT NULL auto_increment,
  `creatureId` bigint(20) NOT NULL default '0',
  `X` float NOT NULL default '0',
  `Y` float NOT NULL default '0',
  `Z` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
