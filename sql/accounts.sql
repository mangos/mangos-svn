-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:13 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `accounts`
-- 
-- Creation: Aug 21, 2005 at 10:30 AM
-- Last update: Aug 21, 2005 at 10:30 AM
-- 

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `acct` bigint(20) NOT NULL auto_increment,
  `login` varchar(255) collate latin1_general_ci NOT NULL default '',
  `password` varchar(28) collate latin1_general_ci NOT NULL default '',
  `s` longtext collate latin1_general_ci NOT NULL,
  `v` longtext collate latin1_general_ci NOT NULL,
  `gm` tinyint(1) NOT NULL default '0',
  `sessionkey` longtext collate latin1_general_ci NOT NULL,
  `email` varchar(50) collate latin1_general_ci NOT NULL default '',
  `joindate` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `banned` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`acct`),
  UNIQUE KEY `acct` (`acct`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=2 ;
