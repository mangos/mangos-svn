-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Sep 19, 2005 at 03:40 PM
-- Server version: 4.0.24
-- PHP Version: 4.4.0-2
-- 
-- Database: `mangos`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `activation`
-- 

CREATE TABLE `activation` (
  `acct` bigint(20) NOT NULL auto_increment,
  `login` varchar(255) NOT NULL default '',
  `password` varchar(40) NOT NULL default '',
  `s` longtext NOT NULL,
  `v` longtext NOT NULL,
  `email` varchar(50) NOT NULL default '',
  `joindate` timestamp(14) NOT NULL,
  `activated` enum('0','1') NOT NULL default '0',
  PRIMARY KEY  (`acct`),
  UNIQUE KEY `acct` (`acct`)
) TYPE=MyISAM COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=16 ;

-- 
-- Dumping data for table `activation`
-- 

