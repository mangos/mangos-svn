-- phpMyAdmin SQL Dump
-- version 2.6.2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Aug 30, 2005 at 08:17 PM
-- Server version: 4.1.12
-- PHP Version: 5.0.4
-- 
-- Database: `wowd`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `ipbantable`
-- 
-- Creation: Aug 26, 2005 at 12:03 AM
-- Last update: Aug 26, 2005 at 12:03 AM
-- 

DROP TABLE IF EXISTS `ipbantable`;
CREATE TABLE `ipbantable` (
  `ip` varchar(32) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY  (`ip`),
  UNIQUE KEY `ip` (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';
