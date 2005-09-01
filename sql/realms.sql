-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:19 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `realms`
-- 
-- Creation: Aug 26, 2005 at 12:25 AM
-- Last update: Aug 26, 2005 at 12:25 AM
-- 

DROP TABLE IF EXISTS `realms`;
CREATE TABLE `realms` (
  `id` bigint(20) NOT NULL auto_increment,
  `name` varchar(32) collate latin1_general_ci NOT NULL default '',
  `address` varchar(32) collate latin1_general_ci NOT NULL default '',
  `icon` int(10) default '0',
  `color` int(10) default '0',
  `timezone` int(10) default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB' AUTO_INCREMENT=3 ;
