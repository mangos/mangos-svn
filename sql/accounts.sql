-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 10, 2005 at 12:08 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `accounts`
-- 
-- Creation: Sep 10, 2005 at 12:02 PM
-- Last update: Sep 10, 2005 at 12:02 PM
-- 

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE IF NOT EXISTS `accounts` (
  `acct` bigint(20) NOT NULL auto_increment,
  `login` varchar(255) NOT NULL default '',
  `password` varchar(28) NOT NULL default '',
  `s` longtext NOT NULL,
  `v` longtext NOT NULL,
  `gm` tinyint(1) NOT NULL default '0',
  `sessionkey` longtext NOT NULL,
  `email` varchar(50) NOT NULL default '',
  `joindate` timestamp NOT NULL,
  `banned` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`acct`),
  UNIQUE KEY `acct` (`acct`)
) TYPE=MyISAM COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=1 ;
